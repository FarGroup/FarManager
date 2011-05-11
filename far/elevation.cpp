/*
elevation.cpp

Elevation
*/
/*
Copyright © 2010 Far Group
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "headers.hpp"
#pragma hdrstop

#include "elevation.hpp"
#include "config.hpp"
#include "lang.hpp"
#include "language.hpp"
#include "dialog.hpp"
#include "pathmix.hpp"
#include "colors.hpp"
#include "palette.hpp"
#include "lasterror.hpp"
#include "privilege.hpp"
#include "fileowner.hpp"
#include "imports.hpp"
#include "TaskBar.hpp"
#include "synchro.hpp"
#include "scrbuf.hpp"
#include "event.hpp"
#include "FarGuid.hpp"
#include "strmix.hpp"

const int CallbackMagic= 0xCA11BAC6;

DWORD ParentPID;

class AutoObject:NonCopyable
{
public:
	AutoObject():
		Data(nullptr)
	{
	}

	LPVOID Allocate(size_t Size)
	{
		Free();
		Data=xf_malloc(Size);
		return Data;
	}

	void Free()
	{
		if(Data)
		{
			xf_free(Data);
			Data=nullptr;
		}
	}

	~AutoObject()
	{
		Free();
	}

	LPVOID Get()
	{
		return Data;
	}

	LPCWSTR GetStr()
	{
		return static_cast<LPCWSTR>(Get());
	}

private:
	LPVOID Data;
};

bool RawReadPipe(HANDLE Pipe, LPVOID Data, size_t DataSize)
{
	DWORD n;
	return ReadFile(Pipe, Data, static_cast<DWORD>(DataSize), &n, nullptr) && n==DataSize;
}

bool RawWritePipe(HANDLE Pipe, LPCVOID Data, size_t DataSize)
{
	DWORD n;
	return WriteFile(Pipe, Data, static_cast<DWORD>(DataSize), &n, nullptr) && n==DataSize;
}

bool ReadPipeInt(HANDLE Pipe, int& Data)
{
	return RawReadPipe(Pipe, &Data, sizeof(Data));
}

bool ReadPipeInt64(HANDLE Pipe, INT64& Data)
{
	return RawReadPipe(Pipe, &Data, sizeof(Data));
}

bool WritePipeInt(HANDLE Pipe, int Data)
{
	return RawWritePipe(Pipe, &Data, sizeof(Data));
}

bool WritePipeInt64(HANDLE Pipe, INT64 Data)
{
	return RawWritePipe(Pipe, &Data, sizeof(Data));
}

bool ReadPipeData(HANDLE Pipe, AutoObject& Data)
{
	bool Result=false;
	int DataSize=0;
	if(ReadPipeInt(Pipe, DataSize))
	{
		if(DataSize)
		{
			LPVOID Ptr=Data.Allocate(DataSize);
			if(Ptr)
			{
				if(RawReadPipe(Pipe, Ptr, DataSize))
				{
					Result=true;
				}
			}
		}
		else
		{
			Result=true;
		}
	}
	return Result;
}

bool WritePipeData(HANDLE Pipe, LPCVOID Data, size_t DataSize)
{
	bool Result=false;
	if(WritePipeInt(Pipe, static_cast<int>(DataSize)))
	{
		if(!DataSize || RawWritePipe(Pipe, Data, DataSize))
		{
			Result=true;
		}
	}
	return Result;
}

DisableElevation::DisableElevation()
{
	Value = Opt.ElevationMode;
	Opt.ElevationMode = 0;
}

DisableElevation::~DisableElevation()
{
	Opt.ElevationMode = Value;
}

elevation Elevation;

elevation::elevation():
	Pipe(INVALID_HANDLE_VALUE),
	Process(nullptr),
	PID(0),
	MainThreadID(GetCurrentThreadId()),
	Elevation(false),
	DontAskAgain(false),
	Approve(false),
	AskApprove(true),
	Recurse(false)
{
}

elevation::~elevation()
{
	SendCommand(C_SERVICE_EXIT);
	DisconnectNamedPipe(Pipe);
	PID=0;
	CloseHandle(Pipe);
}

void elevation::ResetApprove()
{
	if(!DontAskAgain)
	{
		Approve=false;
		AskApprove=true;
		if(Elevation)
		{
			Elevation=false;
			ScrBuf.RestoreElevationChar();
		}
	}
}

bool elevation::ReadData(AutoObject& Data) const
{
	return ReadPipeData(Pipe, Data);
}

bool elevation::WriteData(LPCVOID Data,size_t DataSize) const
{
	return WritePipeData(Pipe, Data, DataSize);
}

bool elevation::ReadInt(int& Data) const
{
	return ReadPipeInt(Pipe, Data);
}

bool elevation::ReadInt64(INT64& Data) const
{
	return ReadPipeInt64(Pipe, Data);
}

bool elevation::WriteInt(int Data) const
{
	return WritePipeInt(Pipe, Data);
}

bool elevation::WriteInt64(INT64 Data) const
{
	return WritePipeInt64(Pipe, Data);
}

bool elevation::SendCommand(ELEVATION_COMMAND Command) const
{
	return WritePipeInt(Pipe, Command);
}

bool elevation::ReceiveLastError() const
{
	int LastError = ERROR_SUCCESS;
	bool Result = ReadPipeInt(Pipe, LastError);
	SetLastError(LastError);
	return Result;
}

bool elevation::Initialize()
{
	bool Result=false;
	if(Pipe==INVALID_HANDLE_VALUE)
	{
		GUID Id;
		if(CoCreateGuid(&Id) == S_OK)
		{
			strPipeID = GuidToStr(Id);
			SID_IDENTIFIER_AUTHORITY NtAuthority=SECURITY_NT_AUTHORITY;
			PSID AdminSID;
			if(AllocateAndInitializeSid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &AdminSID))
			{
				PSECURITY_DESCRIPTOR pSD = static_cast<PSECURITY_DESCRIPTOR>(LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH));
				if(pSD)
				{
					if (InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION))
					{
						PACL pACL = nullptr;
						EXPLICIT_ACCESS ea={};
						ea.grfAccessPermissions = GENERIC_READ|GENERIC_WRITE;
						ea.grfAccessMode = SET_ACCESS;
						ea.grfInheritance= NO_INHERITANCE;
						ea.Trustee.TrusteeForm = TRUSTEE_IS_SID;
						ea.Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
						ea.Trustee.ptstrName = static_cast<LPTSTR>(AdminSID);
						if(SetEntriesInAcl(1, &ea, nullptr, &pACL) == ERROR_SUCCESS)
						{
							if(SetSecurityDescriptorDacl(pSD, TRUE, pACL, FALSE))
							{
								SECURITY_ATTRIBUTES sa = {sizeof(SECURITY_ATTRIBUTES), pSD, FALSE};
								string strPipe(L"\\\\.\\pipe\\");
								strPipe+=strPipeID;
								Pipe=CreateNamedPipe(strPipe, PIPE_ACCESS_DUPLEX|FILE_FLAG_OVERLAPPED, PIPE_TYPE_BYTE|PIPE_READMODE_BYTE|PIPE_WAIT, 1, 0, 0, 0, &sa);
							}
							LocalFree(pACL);
						}
					}
					LocalFree(pSD);
				}
				FreeSid(AdminSID);
			}
		}
	}
	if(Pipe!=INVALID_HANDLE_VALUE)
	{
		if(Process)
		{
			if(WaitForSingleObject(Process, 0) == WAIT_TIMEOUT)
			{
				Result = true;
			}
			else
			{
				CloseHandle(Process);
				Process = nullptr;
			}
		}
		if(!Result)
		{
			TaskBar TB;
			DisconnectNamedPipe(Pipe);
			FormatString strParam;
			strParam << L"/elevation " << strPipeID << L" " << GetCurrentProcessId() << L" " << ((Opt.ElevationMode&ELEVATION_USE_PRIVILEGES)? L"1" : L"0");
			SHELLEXECUTEINFO info=
			{
				sizeof(info),
				SEE_MASK_FLAG_NO_UI|SEE_MASK_UNICODE|SEE_MASK_NOASYNC|SEE_MASK_NOCLOSEPROCESS,
				nullptr,
				L"runas",
				g_strFarModuleName,
				strParam,
				g_strFarPath,
			};
			if(ShellExecuteEx(&info))
			{
				Process = info.hProcess;
				OVERLAPPED Overlapped;
				Event AEvent;
				Overlapped.hEvent = AEvent.Handle();
				ConnectNamedPipe(Pipe, &Overlapped);
				if(AEvent.Wait(15000))
				{
					DWORD NumberOfBytesTransferred;
					if(GetOverlappedResult(Pipe, &Overlapped, &NumberOfBytesTransferred, FALSE))
					{
						if(ReadPipeInt(Pipe, PID))
						{
							Result = true;
						}
					}
				}
				if(!Result)
				{
					if(WaitForSingleObject(Process, 0) == WAIT_TIMEOUT)
					{
						TerminateProcess(Process, 0);
						CloseHandle(Process);
						Process = nullptr;
					}
					SetLastError(ERROR_PROCESS_ABORTED);
				}
			}
			else
			{
				ResetApprove();
			}
		}
	}
	Elevation=Result;
	return Result;
}

enum ELEVATIONAPPROVEDLGITEM
{
	AAD_DOUBLEBOX,
	AAD_TEXT_NEEDPERMISSION,
	AAD_TEXT_DETAILS,
	AAD_EDIT_OBJECT,
	AAD_CHECKBOX_DOFORALL,
	AAD_CHECKBOX_DONTASKAGAIN,
	AAD_SEPARATOR,
	AAD_BUTTON_OK,
	AAD_BUTTON_SKIP,
};

INT_PTR WINAPI ElevationApproveDlgProc(HANDLE hDlg,int Msg,int Param1,void* Param2)
{
	switch (Msg)
	{
	case DN_CTLCOLORDLGITEM:
		{
			if(Param1==AAD_EDIT_OBJECT)
			{
				int Color=FarColorToReal(COL_DIALOGTEXT);
				return ((reinterpret_cast<INT_PTR>(Param2)&0xFF00FF00)|(Color<<16)|Color);
			}
		}
		break;
	default:
		break;
	}
	return DefDlgProc(hDlg,Msg,Param1,Param2);
}

struct EAData
{
	Event* pEvent;
	int Why;
	LPCWSTR Object;
	bool& AskApprove;
	bool& Approve;
	bool& DontAskAgain;
	EAData(Event* pEvent, int Why, LPCWSTR Object, bool& AskApprove, bool& Approve, bool& DontAskAgain):
		pEvent(pEvent), Why(Why), Object(Object), AskApprove(AskApprove), Approve(Approve), DontAskAgain(DontAskAgain){}
};

void ElevationApproveDlgSync(LPVOID Param)
{
	EAData* Data=static_cast<EAData*>(Param);
	enum {DlgX=64,DlgY=12};
	FarDialogItem ElevationApproveDlgData[]=
	{
		{DI_DOUBLEBOX,3,1,DlgX-4,DlgY-2,0,nullptr,nullptr,0,MSG(MErrorAccessDenied)},
		{DI_TEXT,5,2,0,2,0,nullptr,nullptr,0,MSG(Opt.IsUserAdmin?MElevationRequiredPrivileges:MElevationRequired)},
		{DI_TEXT,5,3,0,3,0,nullptr,nullptr,0,MSG(Data->Why)},
		{DI_EDIT,5,4,DlgX-6,4,0,nullptr,nullptr,DIF_READONLY|DIF_SETCOLOR|FarColorToReal(COL_DIALOGTEXT),Data->Object},
		{DI_CHECKBOX,5,6,0,6,1,nullptr,nullptr,0,MSG(MElevationDoForAll)},
		{DI_CHECKBOX,5,7,0,7,0,nullptr,nullptr,0,MSG(MElevationDoNotAskAgainInTheCurrentSession)},
		{DI_TEXT,3,DlgY-4,0,DlgY-4,0,nullptr,nullptr,DIF_SEPARATOR,L""},
		{DI_BUTTON,0,DlgY-3,0,DlgY-3,0,nullptr,nullptr,DIF_DEFAULTBUTTON|DIF_FOCUS|DIF_SETSHIELD|DIF_CENTERGROUP,MSG(MOk)},
		{DI_BUTTON,0,DlgY-3,0,DlgY-3,0,nullptr,nullptr,DIF_CENTERGROUP,MSG(MSkip)},
	};
	MakeDialogItemsEx(ElevationApproveDlgData,ElevationApproveDlg);
	Dialog Dlg(ElevationApproveDlg,ARRAYSIZE(ElevationApproveDlg),ElevationApproveDlgProc);
	Dlg.SetHelp(L"ElevationDlg");
	Dlg.SetPosition(-1,-1,DlgX,DlgY);
	Dlg.SetDialogMode(DMODE_FULLSHADOW|DMODE_NOPLUGINS);
	Dlg.Process();
	Data->AskApprove=!ElevationApproveDlg[AAD_CHECKBOX_DOFORALL].Selected;
	Data->Approve=Dlg.GetExitCode()==AAD_BUTTON_OK;
	Data->DontAskAgain=ElevationApproveDlg[AAD_CHECKBOX_DONTASKAGAIN].Selected!=FALSE;
	if(Data->pEvent)
	{
		Data->pEvent->Set();
	}
}

bool elevation::ElevationApproveDlg(int Why, LPCWSTR Object)
{
	if(!(Opt.IsUserAdmin && !(Opt.ElevationMode&ELEVATION_USE_PRIVILEGES)) &&
		AskApprove && !DontAskAgain && !Recurse &&
		FrameManager && !FrameManager->ManagerIsDown())
	{
		Recurse = true;
		GuardLastError error;
		TaskBarPause TBP;
		EAData Data(nullptr, Why, Object, AskApprove, Approve, DontAskAgain);
		if(GetCurrentThreadId()!=MainThreadID)
		{
			Data.pEvent=new Event();
			if(Data.pEvent)
			{
				PluginSynchroManager.Synchro(false, FarGuid, &Data);
				Data.pEvent->Wait();
				delete Data.pEvent;
			}
		}
		else
		{
			ElevationApproveDlgSync(&Data);
		}
		Recurse = false;
	}
	return Approve;
}

bool elevation::fCreateDirectoryEx(LPCWSTR TemplateObject, LPCWSTR Object, LPSECURITY_ATTRIBUTES Attributes)
{
	CriticalSectionLock Lock(CS);
	bool Result=false;
	if(ElevationApproveDlg(MElevationRequiredCreate, Object))
	{
		if(Opt.IsUserAdmin)
		{
			Privilege BackupPrivilege(SE_BACKUP_NAME), RestorePrivilege(SE_RESTORE_NAME);
			Result = (TemplateObject?CreateDirectoryEx(TemplateObject, Object, Attributes):CreateDirectory(Object, Attributes)) != FALSE;
		}
		else
		{
			if(Initialize())
			{
				if(SendCommand(C_FUNCTION_CREATEDIRECTORYEX))
				{
					if(WriteData(TemplateObject,TemplateObject?(StrLength(TemplateObject)+1)*sizeof(WCHAR):0))
					{
						if(WriteData(Object,Object?(StrLength(Object)+1)*sizeof(WCHAR):0))
						{
							// BUGBUG: SecurityAttributes ignored
							int OpResult=0;
							if(ReadInt(OpResult))
							{
								if(ReceiveLastError())
								{
									Result = OpResult != 0;
								}
							}
						}
					}
				}
			}
		}
	}
	return Result;
}

bool elevation::fRemoveDirectory(LPCWSTR Object)
{
	CriticalSectionLock Lock(CS);
	bool Result=false;
	if(ElevationApproveDlg(MElevationRequiredDelete, Object))
	{
		if(Opt.IsUserAdmin)
		{
			Privilege BackupPrivilege(SE_BACKUP_NAME), RestorePrivilege(SE_RESTORE_NAME);
			Result = RemoveDirectory(Object) != FALSE;
		}
		else
		{
			if(Initialize())
			{
				if(SendCommand(C_FUNCTION_REMOVEDIRECTORY))
				{
					if(WriteData(Object,Object?(StrLength(Object)+1)*sizeof(WCHAR):0))
					{
						int OpResult=0;
						if(ReadInt(OpResult))
						{
							if(ReceiveLastError())
							{
								Result = OpResult != 0;
							}
						}
					}
				}
			}
		}
	}
	return Result;
}

bool elevation::fDeleteFile(LPCWSTR Object)
{
	CriticalSectionLock Lock(CS);
	bool Result=false;
	if(ElevationApproveDlg(MElevationRequiredDelete, Object))
	{
		if(Opt.IsUserAdmin)
		{
			Privilege BackupPrivilege(SE_BACKUP_NAME), RestorePrivilege(SE_RESTORE_NAME);
			Result = DeleteFile(Object) != FALSE;
		}
		else
		{
			if(Initialize())
			{
				if(SendCommand(C_FUNCTION_DELETEFILE))
				{
					if(WriteData(Object,Object?(StrLength(Object)+1)*sizeof(WCHAR):0))
					{
						int OpResult=0;
						if(ReadInt(OpResult))
						{
							if(ReceiveLastError())
							{
								Result = OpResult != 0;
							}
						}
					}
				}
			}
		}
	}
	return Result;
}

void elevation::fCallbackRoutine(LPPROGRESS_ROUTINE ProgressRoutine) const
{
	if(ProgressRoutine)
	{
		AutoObject TotalFileSize;
		if (ReadData(TotalFileSize))
		{
			AutoObject TotalBytesTransferred;
			if (ReadData(TotalBytesTransferred))
			{
				AutoObject StreamSize;
				if (ReadData(StreamSize))
				{
					AutoObject StreamBytesTransferred;
					if (ReadData(StreamBytesTransferred))
					{
						int StreamNumber=0;
						if (ReadInt(StreamNumber))
						{
							int CallbackReason=0;
							if (ReadInt(CallbackReason))
							{
								// BUGBUG: SourceFile, DestinationFile ignored
								AutoObject Data;
								if (ReadData(Data))
								{
									int Result=ProgressRoutine(*static_cast<PLARGE_INTEGER>(TotalFileSize.Get()), *static_cast<PLARGE_INTEGER>(TotalBytesTransferred.Get()), *static_cast<PLARGE_INTEGER>(StreamSize.Get()), *static_cast<PLARGE_INTEGER>(StreamBytesTransferred.Get()), StreamNumber, CallbackReason, INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE, Data.Get());
									if(WriteInt(CallbackMagic))
									{
										WriteInt(Result);
									}
								}
							}
						}
					}
				}
			}
		}
	}
}

bool elevation::fCopyFileEx(LPCWSTR From, LPCWSTR To, LPPROGRESS_ROUTINE ProgressRoutine, LPVOID Data, LPBOOL Cancel, DWORD Flags)
{
	CriticalSectionLock Lock(CS);
	bool Result = false;
	if(ElevationApproveDlg(MElevationRequiredCopy, From))
	{
		if(Opt.IsUserAdmin)
		{
			Privilege BackupPrivilege(SE_BACKUP_NAME), RestorePrivilege(SE_RESTORE_NAME);
			Result = CopyFileEx(From, To, ProgressRoutine, Data, Cancel, Flags) != FALSE;
		}
		else
		{
			if(Initialize())
			{
				if(SendCommand(C_FUNCTION_COPYFILEEX))
				{
					if(WriteData(From, From?(StrLength(From)+1)*sizeof(WCHAR):0))
					{
						if(WriteData(To, To?(StrLength(To)+1)*sizeof(WCHAR):0))
						{
							if (WriteData(&ProgressRoutine, sizeof(ProgressRoutine)))
							{
								if (WriteData(&Data, sizeof(Data)))
								{
									// BUGBUG: Cancel ignored
									if(WriteInt(Flags))
									{
										int OpResult=0;
										if(ReadInt(OpResult))
										{
											if (OpResult == CallbackMagic)
											{
												while(OpResult == CallbackMagic)
												{
													fCallbackRoutine(ProgressRoutine);
													ReadInt(OpResult);
												}
											}
											if(OpResult != CallbackMagic)
											{
												Result = OpResult != 0;
												ReceiveLastError();
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	return Result;
}

bool elevation::fMoveFileEx(LPCWSTR From, LPCWSTR To, DWORD Flags)
{
	CriticalSectionLock Lock(CS);
	bool Result=false;
	if(ElevationApproveDlg(MElevationRequiredMove, From))
	{
		if(Opt.IsUserAdmin)
		{
			Privilege BackupPrivilege(SE_BACKUP_NAME), RestorePrivilege(SE_RESTORE_NAME);
			Result = MoveFileEx(From, To, Flags) != FALSE;
		}
		else
		{
			if(Initialize())
			{
				if(SendCommand(C_FUNCTION_MOVEFILEEX))
				{
					if(WriteData(From,From?(StrLength(From)+1)*sizeof(WCHAR):0))
					{
						if(WriteData(To,To?(StrLength(To)+1)*sizeof(WCHAR):0))
						{
							if(WriteInt(Flags))
							{
								int OpResult=0;
								if(ReadInt(OpResult))
								{
									if(ReceiveLastError())
									{
										Result = OpResult != 0;
									}
								}
							}
						}
					}
				}
			}
		}
	}
	return Result;
}

DWORD elevation::fGetFileAttributes(LPCWSTR Object)
{
	CriticalSectionLock Lock(CS);
	DWORD Result = INVALID_FILE_ATTRIBUTES;
	if(ElevationApproveDlg(MElevationRequiredGetAttributes, Object))
	{
		if(Opt.IsUserAdmin)
		{
			Privilege BackupPrivilege(SE_BACKUP_NAME), RestorePrivilege(SE_RESTORE_NAME);
			Result = GetFileAttributes(Object);
		}
		else
		{
			if(Initialize())
			{
				if(SendCommand(C_FUNCTION_GETFILEATTRIBUTES))
				{
					if(WriteData(Object,Object?(StrLength(Object)+1)*sizeof(WCHAR):0))
					{
						int OpResult=0;
						if(ReadInt(OpResult))
						{
							if(ReceiveLastError())
							{
								Result = OpResult;
							}
						}
					}
				}
			}
		}
	}
	return Result;
}

bool elevation::fSetFileAttributes(LPCWSTR Object, DWORD FileAttributes)
{
	CriticalSectionLock Lock(CS);
	bool Result=false;
	if(ElevationApproveDlg(MElevationRequiredSetAttributes, Object))
	{
		if(Opt.IsUserAdmin)
		{
			Privilege BackupPrivilege(SE_BACKUP_NAME), RestorePrivilege(SE_RESTORE_NAME);
			Result = SetFileAttributes(Object, FileAttributes) != FALSE;
		}
		else
		{
			if(Initialize())
			{
				if(SendCommand(C_FUNCTION_SETFILEATTRIBUTES))
				{
					if(WriteData(Object,Object?(StrLength(Object)+1)*sizeof(WCHAR):0))
					{
						if(WriteInt(FileAttributes))
						{
							int OpResult=0;
							if(ReadInt(OpResult))
							{
								if(ReceiveLastError())
								{
									Result = OpResult != 0;
								}
							}
						}
					}
				}
			}
		}
	}
	return Result;
}

bool elevation::fCreateHardLink(LPCWSTR Object, LPCWSTR Target, LPSECURITY_ATTRIBUTES SecurityAttributes)
{
	CriticalSectionLock Lock(CS);
	bool Result=false;
	if(ElevationApproveDlg(MElevationRequiredHardLink, Object))
	{
		if(Opt.IsUserAdmin)
		{
			Privilege BackupPrivilege(SE_BACKUP_NAME), RestorePrivilege(SE_RESTORE_NAME);
			Result = CreateHardLink(Object, Target, SecurityAttributes) != FALSE;
		}
		else
		{
			if(Initialize())
			{
				if(SendCommand(C_FUNCTION_CREATEHARDLINK))
				{
					if(WriteData(Object,Object?(StrLength(Object)+1)*sizeof(WCHAR):0))
					{
						if(WriteData(Target,Target?(StrLength(Target)+1)*sizeof(WCHAR):0))
						{
							// BUGBUG: SecurityAttributes ignored.
							int OpResult=0;
							if(ReadInt(OpResult))
							{
								if(ReceiveLastError())
								{
									Result = OpResult != 0;
								}
							}
						}
					}
				}
			}
		}
	}
	return Result;
}

bool elevation::fCreateSymbolicLink(LPCWSTR Object, LPCWSTR Target, DWORD Flags)
{
	CriticalSectionLock Lock(CS);
	bool Result=false;
	if(ElevationApproveDlg(MElevationRequiredSymLink, Object))
	{
		if(Opt.IsUserAdmin)
		{
			Privilege BackupPrivilege(SE_BACKUP_NAME), RestorePrivilege(SE_RESTORE_NAME);
			Result = CreateSymbolicLinkInternal(Object, Target, Flags);
		}
		else
		{
			if(Initialize())
			{
				if(SendCommand(C_FUNCTION_CREATESYMBOLICLINK))
				{
					if(WriteData(Object,Object?(StrLength(Object)+1)*sizeof(WCHAR):0))
					{
						if(WriteData(Target,Target?(StrLength(Target)+1)*sizeof(WCHAR):0))
						{
							if(WriteInt(Flags))
							{
								int OpResult=0;
								if(ReadInt(OpResult))
								{
									if(ReceiveLastError())
									{
										Result = OpResult != 0;
									}
								}
							}
						}
					}
				}
			}
		}
	}
	return Result;
}


int elevation::fMoveToRecycleBin(SHFILEOPSTRUCT& FileOpStruct)
{
	CriticalSectionLock Lock(CS);
	int Result=0;
	if(ElevationApproveDlg(MElevationRequiredRecycle, FileOpStruct.pFrom))
	{
		if(Opt.IsUserAdmin)
		{
			Privilege BackupPrivilege(SE_BACKUP_NAME), RestorePrivilege(SE_RESTORE_NAME);
			Result = SHFileOperation(&FileOpStruct);
		}
		else
		{
			if(Initialize())
			{
				if(SendCommand(C_FUNCTION_MOVETORECYCLEBIN))
				{
					if(WriteData(&FileOpStruct, sizeof(FileOpStruct)))
					{
						if(WriteData(FileOpStruct.pFrom,FileOpStruct.pFrom?(StrLength(FileOpStruct.pFrom)+1+1)*sizeof(WCHAR):0)) // achtung! +1
						{
							if(WriteData(FileOpStruct.pTo,FileOpStruct.pTo?(StrLength(FileOpStruct.pTo)+1+1)*sizeof(WCHAR):0)) // achtung! +1
							{
								int OpResult=0;
								if(ReadInt(OpResult))
								{
									// achtung! no "last error" here
									if(ReadInt(FileOpStruct.fAnyOperationsAborted))
									{
										Result = OpResult;
									}
								}
							}
						}
					}
				}
			}
		}
	}
	return Result;
}

bool elevation::fSetOwner(LPCWSTR Object, LPCWSTR Owner)
{
	CriticalSectionLock Lock(CS);
	bool Result=false;
	if(ElevationApproveDlg(MElevationRequiredSetOwner, Object))
	{
		if(Opt.IsUserAdmin)
		{
			Privilege BackupPrivilege(SE_BACKUP_NAME), RestorePrivilege(SE_RESTORE_NAME);
			Result = SetOwnerInternal(Object, Owner);
		}
		else
		{
			if(Initialize())
			{
				if(SendCommand(C_FUNCTION_SETOWNER))
				{
					if(WriteData(Object,Object?(StrLength(Object)+1)*sizeof(WCHAR):0))
					{
						if(WriteData(Owner,Owner?(StrLength(Owner)+1)*sizeof(WCHAR):0))
						{
							int OpResult=0;
							if(ReadInt(OpResult))
							{
								if(ReceiveLastError())
								{
									Result = OpResult != 0;
								}
							}
						}
					}
				}
			}
		}
	}
	return Result;
}

HANDLE elevation::fCreateFile(LPCWSTR Object, DWORD DesiredAccess, DWORD ShareMode, LPSECURITY_ATTRIBUTES SecurityAttributes, DWORD CreationDistribution, DWORD FlagsAndAttributes, HANDLE TemplateFile)
{
	CriticalSectionLock Lock(CS);
	HANDLE Result=INVALID_HANDLE_VALUE;
	if(ElevationApproveDlg(MElevationRequiredOpen, Object))
	{
		if(Opt.IsUserAdmin)
		{
			Privilege BackupPrivilege(SE_BACKUP_NAME), RestorePrivilege(SE_RESTORE_NAME);
			Result = CreateFile(Object, DesiredAccess, ShareMode, SecurityAttributes, CreationDistribution, FlagsAndAttributes, TemplateFile);
		}
		else
		{
			if(Initialize())
			{
				if(SendCommand(C_FUNCTION_CREATEFILE))
				{
					if(WriteData(Object,Object?(StrLength(Object)+1)*sizeof(WCHAR):0))
					{
						if(WriteInt(DesiredAccess))
						{
							if(WriteInt(ShareMode))
							{
								// BUGBUG: SecurityAttributes ignored
								if(WriteInt(CreationDistribution))
								{
									if(WriteInt(FlagsAndAttributes))
									{
										// BUGBUG: TemplateFile ignored
										AutoObject OpResult;
										if(ReadData(OpResult))
										{
											if(ReceiveLastError())
											{
												Result = *static_cast<PHANDLE>(OpResult.Get());
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	return Result;
}

bool ElevationRequired(ELEVATION_MODE Mode)
{
	bool Result = false;
	if(Opt.ElevationMode&Mode)
	{
		if(ifn.pfnRtlGetLastNtStatus)
		{
			NTSTATUS LastNtStatus = GetLastNtStatus();
			Result = LastNtStatus == STATUS_ACCESS_DENIED || LastNtStatus == STATUS_PRIVILEGE_NOT_HELD;
		}
		else
		{
			// RtlGetLastNtStatus not implemented in w2k.
			DWORD LastWin32Error = GetLastError();
			Result = LastWin32Error == ERROR_ACCESS_DENIED || LastWin32Error == ERROR_PRIVILEGE_NOT_HELD;
		}
	}
	return Result;
}

bool IsUserAdmin()
{
	bool Result=false;
	SID_IDENTIFIER_AUTHORITY NtAuthority=SECURITY_NT_AUTHORITY;
	PSID AdministratorsGroup;
	if(AllocateAndInitializeSid(&NtAuthority,2,SECURITY_BUILTIN_DOMAIN_RID,DOMAIN_ALIAS_RID_ADMINS,0,0,0,0,0,0,&AdministratorsGroup))
	{
		BOOL IsMember=FALSE;
		if(CheckTokenMembership(nullptr,AdministratorsGroup,&IsMember)&&IsMember)
		{
			Result=true;
		}
		FreeSid(AdministratorsGroup);
	}
	return Result;
}

HANDLE Pipe;

bool Process(int Command);

DWORD WINAPI ElevationCopyProgressRoutine(LARGE_INTEGER TotalFileSize, LARGE_INTEGER TotalBytesTransferred, LARGE_INTEGER StreamSize, LARGE_INTEGER StreamBytesTransferred, DWORD StreamNumber, DWORD CallbackReason, HANDLE SourceFile,HANDLE DestinationFile, LPVOID Data)
{
	int Result=0;
	if (WritePipeInt(Pipe, CallbackMagic))
	{
		if (WritePipeData(Pipe, &TotalFileSize, sizeof(TotalFileSize)))
		{
			if (WritePipeData(Pipe, &TotalBytesTransferred, sizeof(TotalBytesTransferred)))
			{
				if (WritePipeData(Pipe, &StreamSize, sizeof(StreamSize)))
				{
					if (WritePipeData(Pipe, &StreamBytesTransferred, sizeof(StreamBytesTransferred)))
					{
						if (WritePipeInt(Pipe, StreamNumber))
						{
							if (WritePipeInt(Pipe, CallbackReason))
							{
								// BUGBUG: SourceFile, DestinationFile ignored
								if (WritePipeData(Pipe, &Data, sizeof(Data)))
								{
									for(;;)
									{
										ReadPipeInt(Pipe, Result);
										if (Result == CallbackMagic)
										{
											ReadPipeInt(Pipe, Result);
											break;
										}
										else
										{
											// nested call from ProgressRoutine()
											Process(Result);
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	return Result;
}

void CreateDirectoryExHandler()
{
	AutoObject TemplateObject;
	if(ReadPipeData(Pipe, TemplateObject))
	{
		AutoObject Object;
		if(ReadPipeData(Pipe, Object))
		{
			// BUGBUG, SecurityAttributes ignored
			int Result = TemplateObject.GetStr()?CreateDirectoryEx(TemplateObject.GetStr(), Object.GetStr(), nullptr):CreateDirectory(Object.GetStr(), nullptr);
			int LastError = GetLastError();
			if(WritePipeInt(Pipe, Result))
			{
				WritePipeInt(Pipe, LastError);
			}
		}
	}
}

void RemoveDirectoryHandler()
{
	AutoObject Object;
	if(ReadPipeData(Pipe, Object))
	{
		int Result = RemoveDirectory(Object.GetStr());
		int LastError = GetLastError();
		if(WritePipeInt(Pipe, Result))
		{
			WritePipeInt(Pipe, LastError);
		}
	}
}

void DeleteFileHandler()
{
	AutoObject Object;
	if(ReadPipeData(Pipe, Object))
	{
		int Result = DeleteFile(Object.GetStr());
		int LastError = GetLastError();
		if(WritePipeInt(Pipe, Result))
		{
			WritePipeInt(Pipe, LastError);
		}
	}
}

void CopyFileExHandler()
{
	AutoObject From;
	if(ReadPipeData(Pipe, From))
	{
		AutoObject To;
		if(ReadPipeData(Pipe, To))
		{
			AutoObject UserCopyProgressRoutine;
			if(ReadPipeData(Pipe, UserCopyProgressRoutine))
			{
				AutoObject Data;
				if(ReadPipeData(Pipe, Data))
				{
					int Flags = 0;
					if(ReadPipeInt(Pipe, Flags))
					{
						// BUGBUG: Cancel ignored
						int Result = CopyFileEx(From.GetStr(), To.GetStr(), UserCopyProgressRoutine.Get()?ElevationCopyProgressRoutine:nullptr, Data.Get(), nullptr, Flags);
						int LastError = GetLastError();
						if(WritePipeInt(Pipe, Result))
						{
							WritePipeInt(Pipe, LastError);
						}
					}
				}
			}
		}
	}
}

void MoveFileExHandler()
{
	AutoObject From;
	if(ReadPipeData(Pipe, From))
	{
		AutoObject To;
		if(ReadPipeData(Pipe, To))
		{
			int Flags = 0;
			if(ReadPipeInt(Pipe, Flags))
			{
				int Result = MoveFileEx(From.GetStr(), To.GetStr(), Flags);
				int LastError = GetLastError();
				if(WritePipeInt(Pipe, Result))
				{
					WritePipeInt(Pipe, LastError);
				}
			}
		}
	}
}

void GetFileAttributesHandler()
{
	AutoObject Object;
	if(ReadPipeData(Pipe, Object))
	{
		int Result = GetFileAttributes(Object.GetStr());
		int LastError = GetLastError();
		if(WritePipeInt(Pipe, Result))
		{
			WritePipeInt(Pipe, LastError);
		}
	}
}

void SetFileAttributesHandler()
{
	AutoObject Object;
	if(ReadPipeData(Pipe, Object))
	{
		int Attributes = 0;
		if(ReadPipeInt(Pipe, Attributes))
		{
			int Result = SetFileAttributes(Object.GetStr(), Attributes);
			int LastError = GetLastError();
			if(WritePipeInt(Pipe, Result))
			{
				WritePipeInt(Pipe, LastError);
			}
		}
	}
}

void CreateHardLinkHandler()
{
	AutoObject Object;
	if(ReadPipeData(Pipe, Object))
	{
		AutoObject Target;
		if(ReadPipeData(Pipe, Target))
		{
			// BUGBUG: SecurityAttributes ignored.
			int Result = CreateHardLink(Object.GetStr(), Target.GetStr(), nullptr);
			int LastError = GetLastError();
			if(WritePipeInt(Pipe, Result))
			{
				WritePipeInt(Pipe, LastError);
			}
		}
	}
}

void CreateSymbolicLinkHandler()
{
	AutoObject Object;
	if(ReadPipeData(Pipe, Object))
	{
		AutoObject Target;
		if(ReadPipeData(Pipe, Target))
		{
			int Flags = 0;
			if(ReadPipeInt(Pipe, Flags))
			{
				int Result = CreateSymbolicLinkInternal(Object.GetStr(), Target.GetStr(), Flags);
				int LastError = GetLastError();
				if(WritePipeInt(Pipe, Result))
				{
					WritePipeInt(Pipe, LastError);
				}
			}
		}
	}
}

void MoveToRecycleBinHandler()
{
	AutoObject Struct;
	if(ReadPipeData(Pipe, Struct))
	{
		AutoObject From;
		if(ReadPipeData(Pipe, From))
		{
			AutoObject To;
			if(ReadPipeData(Pipe, To))
			{
				SHFILEOPSTRUCT* FileOpStruct = static_cast<SHFILEOPSTRUCT*>(Struct.Get());
				FileOpStruct->pFrom = From.GetStr();
				FileOpStruct->pTo = To.GetStr();
				if(WritePipeInt(Pipe, SHFileOperation(FileOpStruct)))
				{
					WritePipeInt(Pipe, FileOpStruct->fAnyOperationsAborted);
				}
			}
		}
	}
}

void SetOwnerHandler()
{
	AutoObject Object;
	if(ReadPipeData(Pipe, Object))
	{
		AutoObject Owner;
		if(ReadPipeData(Pipe, Owner))
		{
			int Result = SetOwnerInternal(Object.GetStr(), Owner.GetStr());
			int LastError = GetLastError();
			if(WritePipeInt(Pipe, Result))
			{
				WritePipeInt(Pipe, LastError);
			}
		}
	}
}

void CreateFileHandler()
{
	AutoObject Object;
	if(ReadPipeData(Pipe, Object))
	{
		int DesiredAccess;
		if(ReadPipeInt(Pipe, DesiredAccess))
		{
			int ShareMode;
			if(ReadPipeInt(Pipe, ShareMode))
			{
				// BUGBUG: SecurityAttributes ignored
				int CreationDistribution;
				if(ReadPipeInt(Pipe, CreationDistribution))
				{
					int FlagsAndAttributes;
					if(ReadPipeInt(Pipe, FlagsAndAttributes))
					{
						// BUGBUG: TemplateFile ignored
						HANDLE Result = apiCreateFile(Object.GetStr(), DesiredAccess, ShareMode, nullptr, CreationDistribution, FlagsAndAttributes, nullptr);
						if(Result!=INVALID_HANDLE_VALUE)
						{
							HANDLE ParentProcess = OpenProcess(PROCESS_DUP_HANDLE, FALSE, ParentPID);
							if(ParentProcess)
							{
								if(!DuplicateHandle(GetCurrentProcess(), Result, ParentProcess, &Result, 0, FALSE, DUPLICATE_CLOSE_SOURCE|DUPLICATE_SAME_ACCESS))
								{
									CloseHandle(Result);
									Result = INVALID_HANDLE_VALUE;
								}
								CloseHandle(ParentProcess);
							}
						}
						int LastError = GetLastError();
						if(WritePipeData(Pipe, &Result, sizeof(Result)))
						{
							WritePipeInt(Pipe, LastError);
						}
					}
				}
			}
		}
	}
}

bool Process(int Command)
{
	bool Exit=false;
	switch(Command)
	{
	case C_SERVICE_EXIT:
		Exit = true;
		break;

	case C_FUNCTION_CREATEDIRECTORYEX:
		CreateDirectoryExHandler();
		break;

	case C_FUNCTION_REMOVEDIRECTORY:
		RemoveDirectoryHandler();
		break;

	case C_FUNCTION_DELETEFILE:
		DeleteFileHandler();
		break;

	case C_FUNCTION_COPYFILEEX:
		CopyFileExHandler();
		break;

	case C_FUNCTION_MOVEFILEEX:
		MoveFileExHandler();
		break;

	case C_FUNCTION_GETFILEATTRIBUTES:
		GetFileAttributesHandler();
		break;

	case C_FUNCTION_SETFILEATTRIBUTES:
		SetFileAttributesHandler();
		break;

	case C_FUNCTION_CREATEHARDLINK:
		CreateHardLinkHandler();
		break;

	case C_FUNCTION_CREATESYMBOLICLINK:
		CreateSymbolicLinkHandler();
		break;

	case C_FUNCTION_MOVETORECYCLEBIN:
		MoveToRecycleBinHandler();
		break;

	case C_FUNCTION_SETOWNER:
		SetOwnerHandler();
		break;

	case C_FUNCTION_CREATEFILE:
		CreateFileHandler();
		break;
	}
	return Exit;
}

int ElevationMain(LPCWSTR guid, DWORD PID, bool UsePrivileges)
{
	int Result = ERROR_SUCCESS;

	SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOOPENFILEERRORBOX);

	Privilege
		BackupPrivilege(UsePrivileges?SE_BACKUP_NAME:nullptr),
		RestorePrivilege(UsePrivileges?SE_RESTORE_NAME:nullptr),
		TakeOwnershipPrivilege(SE_TAKE_OWNERSHIP_NAME),
		DebugPrivilege(SE_DEBUG_NAME),
		CreateSymbolicLinkPrivilege(SE_CREATE_SYMBOLIC_LINK_NAME);

	string strPipe(L"\\\\.\\pipe\\");
	strPipe+=guid;
	WaitNamedPipe(strPipe, NMPWAIT_WAIT_FOREVER);
	Pipe = CreateFile(strPipe,GENERIC_READ|GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
	if (Pipe != INVALID_HANDLE_VALUE)
	{
		ULONG ServerProcessId;
		if(!ifn.pfnGetNamedPipeServerProcessId || (ifn.pfnGetNamedPipeServerProcessId(Pipe, &ServerProcessId) && ServerProcessId == PID))
		{
			HANDLE ParentProcess = OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ, FALSE, PID);
			if(ParentProcess)
			{
				string strCurrentProcess, strParentProcess;
				bool TrustedServer = apiGetModuleFileName(nullptr, strCurrentProcess) && apiGetModuleFileNameEx(ParentProcess, nullptr, strParentProcess) && (!StrCmpI(strCurrentProcess, strParentProcess));
				CloseHandle(ParentProcess);
				if(TrustedServer)
				{
					if(WritePipeInt(Pipe, GetCurrentProcessId()))
					{
						ParentPID = PID;
						bool Exit = false;
						int Command = 0;
						while(!Exit)
						{
							if(ReadPipeInt(Pipe, Command))
							{
								Exit = Process(Command);
							}
							else
							{
								if(GetLastError() == ERROR_BROKEN_PIPE)
								{
									Exit=true;
								}
							}
						}
					}
				}
			}
		}
		CloseHandle(Pipe);
	}
	return Result;
}
