/*
adminmode.cpp

Admin mode
*/
/*
Copyright (c) 2010 Far Group
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

#include "adminmode.hpp"
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

#define PIPE_NAME L"\\\\.\\pipe\\FarPipe"

const int Magic = 0x1DD0D;
const int CallbackMagic= 0xCA11BAC6;

class AutoObject
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
		return reinterpret_cast<wchar_t*>(Get());
	}

private:
	LPVOID Data;
};

bool RawReadPipe(HANDLE Pipe, LPVOID Data, DWORD DataSize)
{
	bool Result=false;
	DWORD n;
	if(ReadFile(Pipe, Data, DataSize, &n, nullptr) && n==DataSize)
	{
		Result=true;
	}
	return Result;
}

bool RawWritePipe(HANDLE Pipe, LPCVOID Data, DWORD DataSize)
{
	bool Result=false;
	DWORD n;
	if(WriteFile(Pipe, Data, DataSize, &n, nullptr) && n==DataSize)
	{
		Result=true;
	}
	return Result;
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

bool WritePipeData(HANDLE Pipe, LPCVOID Data, int DataSize)
{
	bool Result=false;
	if(WritePipeInt(Pipe, DataSize))
	{
		if(!DataSize || RawWritePipe(Pipe, Data, DataSize))
		{
			Result=true;
		}
	}
	return Result;
}

AdminMode Admin;

AdminMode::AdminMode():
	Pipe(INVALID_HANDLE_VALUE),
	Process(nullptr),
	PID(0),
	MainThreadID(GetCurrentThreadId()),
	Elevation(false),
	DontAskAgain(false),
	Approve(false),
	AskApprove(true),
	ProgressRoutine(nullptr),
	Recurse(false)
{
}

AdminMode::~AdminMode()
{
	SendCommand(C_SERVICE_EXIT);
	DisconnectNamedPipe(Pipe);
	PID=0;
	CloseHandle(Pipe);
}

void AdminMode::ResetApprove()
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

bool AdminMode::ReadData(AutoObject& Data) const
{
	return ReadPipeData(Pipe, Data);
}

bool AdminMode::WriteData(LPCVOID Data,DWORD DataSize) const
{
	return WritePipeData(Pipe, Data, DataSize);
}

bool AdminMode::ReadInt(int& Data) const
{
	return ReadPipeInt(Pipe, Data);
}

bool AdminMode::ReadInt64(INT64& Data) const
{
	return ReadPipeInt64(Pipe, Data);
}

bool AdminMode::WriteInt(int Data) const
{
	return WritePipeInt(Pipe, Data);
}

bool AdminMode::WriteInt64(INT64 Data) const
{
	return WritePipeInt64(Pipe, Data);
}

bool AdminMode::SendCommand(ADMIN_COMMAND Command) const
{
	return WritePipeInt(Pipe, Command);
}

bool AdminMode::ReceiveLastError() const
{
	int LastError = ERROR_SUCCESS;
	bool Result = ReadPipeInt(Pipe, LastError);
	SetLastError(LastError);
	return Result;
}

bool AdminMode::Initialize()
{
	bool Result=false;
	if(Pipe==INVALID_HANDLE_VALUE)
	{
		FormatString strPipe;
		strPipe << PIPE_NAME << GetCurrentProcessId();
		Pipe=CreateNamedPipe(strPipe, PIPE_ACCESS_DUPLEX|FILE_FLAG_OVERLAPPED, PIPE_TYPE_BYTE|PIPE_READMODE_BYTE|PIPE_WAIT, 1, 0, 0, 0, nullptr);
	}
	if(Pipe!=INVALID_HANDLE_VALUE)
	{
		if(Process)
		{
			DWORD ExitCode = 0;
			GetExitCodeProcess(Process, &ExitCode);
			if(ExitCode == STILL_ACTIVE)
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
			strParam << L"/admin " << GetCurrentProcessId();
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
				if(AEvent.Wait(5000))
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
					DWORD ExitCode = 0;
					GetExitCodeProcess(Process, &ExitCode);
					if(ExitCode == STILL_ACTIVE)
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

enum ADMINAPPROVEDLGITEM
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

LONG_PTR WINAPI AdminApproveDlgProc(HANDLE hDlg,int Msg,int Param1,LONG_PTR Param2)
{
	switch (Msg)
	{
	case DN_CTLCOLORDLGITEM:
		{
			if(Param1==AAD_EDIT_OBJECT)
			{
				int Color=FarColorToReal(COL_DIALOGTEXT);
				return ((Param2&0xFF00FF00)|(Color<<16)|Color);
			}
		}
		break;
	}
	return DefDlgProc(hDlg,Msg,Param1,Param2);
}

struct AAData
{
	Event* pEvent;
	int Why;
	LPCWSTR Object;
	bool& AskApprove;
	bool& Approve;
	bool& DontAskAgain;
};

void AdminApproveDlgSync(LPVOID Param)
{
	AAData* Data=reinterpret_cast<AAData*>(Param);
	enum {DlgX=64,DlgY=12};
	DialogDataEx AdminApproveDlgData[]=
	{
		DI_DOUBLEBOX,3,1,DlgX-4,DlgY-2,0,0,MSG(MErrorAccessDenied),
		DI_TEXT,5,2,0,2,0,0,MSG(Opt.IsUserAdmin?MAdminRequiredPrivileges:MAdminRequired),
		DI_TEXT,5,3,0,3,0,0,MSG(Data->Why),
		DI_EDIT,5,4,DlgX-6,4,0,DIF_READONLY|DIF_SETCOLOR|FarColorToReal(COL_DIALOGTEXT),Data->Object,
		DI_CHECKBOX,5,6,0,6,1,0,MSG(MAdminDoForAll),
		DI_CHECKBOX,5,7,0,7,0,0,MSG(MAdminDoNotAskAgainInTheCurrentSession),
		DI_TEXT,3,DlgY-4,0,DlgY-4,0,DIF_SEPARATOR,L"",
		DI_BUTTON,0,DlgY-3,0,DlgY-3,0,DIF_DEFAULT|DIF_FOCUS|DIF_SETSHIELD|DIF_CENTERGROUP,MSG(MOk),
		DI_BUTTON,0,DlgY-3,0,DlgY-3,0,DIF_CENTERGROUP,MSG(MSkip),
	};
	MakeDialogItemsEx(AdminApproveDlgData,AdminApproveDlg);
	Dialog Dlg(AdminApproveDlg,ARRAYSIZE(AdminApproveDlg),AdminApproveDlgProc);
	Dlg.SetHelp(L"ElevationDlg");
	Dlg.SetPosition(-1,-1,DlgX,DlgY);
	Dlg.SetDialogMode(DMODE_FULLSHADOW|DMODE_NOPLUGINS);
	Dlg.Process();
	Data->AskApprove=!AdminApproveDlg[AAD_CHECKBOX_DOFORALL].Selected;
	Data->Approve=Dlg.GetExitCode()==AAD_BUTTON_OK;
	Data->DontAskAgain=AdminApproveDlg[AAD_CHECKBOX_DONTASKAGAIN].Selected!=FALSE;
	if(Data->pEvent)
	{
		Data->pEvent->Set();
	}
}

bool AdminMode::AdminApproveDlg(int Why, LPCWSTR Object)
{
	if(FrameManager && !FrameManager->ManagerIsDown() && AskApprove && !DontAskAgain && !Recurse)
	{
		Recurse = true;
		GuardLastError error;
		TaskBarPause TBP;
		AAData Data={nullptr, Why, Object, AskApprove, Approve, DontAskAgain};
		if(GetCurrentThreadId()!=MainThreadID)
		{
			Data.pEvent=new Event();
			if(Data.pEvent)
			{
				PluginSynchroManager.Synchro(false, 0, &Data);
				Data.pEvent->Wait();
				delete Data.pEvent;
			}
		}
		else
		{
			AdminApproveDlgSync(&Data);
		}
		Recurse = false;
	}
	return Approve;
}

bool AdminMode::fCreateDirectoryEx(LPCWSTR TemplateObject, LPCWSTR Object, LPSECURITY_ATTRIBUTES Attributes)
{
	CriticalSectionLock Lock(CS);
	bool Result=false;
	if(AdminApproveDlg(MAdminRequiredCreate, Object))
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

bool AdminMode::fRemoveDirectory(LPCWSTR Object)
{
	CriticalSectionLock Lock(CS);
	bool Result=false;
	if(AdminApproveDlg(MAdminRequiredDelete, Object))
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

bool AdminMode::fDeleteFile(LPCWSTR Object)
{
	CriticalSectionLock Lock(CS);
	bool Result=false;
	if(AdminApproveDlg(MAdminRequiredDelete, Object))
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

void AdminMode::fCallbackRoutine() const
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
									int Result=ProgressRoutine(*reinterpret_cast<PLARGE_INTEGER>(TotalFileSize.Get()), *reinterpret_cast<PLARGE_INTEGER>(TotalBytesTransferred.Get()), *reinterpret_cast<PLARGE_INTEGER>(StreamSize.Get()), *reinterpret_cast<PLARGE_INTEGER>(StreamBytesTransferred.Get()), StreamNumber, CallbackReason, INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE, Data.Get());
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

bool AdminMode::fCopyFileEx(LPCWSTR From, LPCWSTR To, LPPROGRESS_ROUTINE ProgressRoutine, LPVOID Data, LPBOOL Cancel, DWORD Flags)
{
	CriticalSectionLock Lock(CS);
	bool Result = false;
	if(AdminApproveDlg(MAdminRequiredCopy, From))
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
					this->ProgressRoutine=ProgressRoutine;
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
													fCallbackRoutine();
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

bool AdminMode::fMoveFileEx(LPCWSTR From, LPCWSTR To, DWORD Flags)
{
	CriticalSectionLock Lock(CS);
	bool Result=false;
	if(AdminApproveDlg(MAdminRequiredMove, From))
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

DWORD AdminMode::fGetFileAttributes(LPCWSTR Object)
{
	CriticalSectionLock Lock(CS);
	DWORD Result = INVALID_FILE_ATTRIBUTES;
	if(AdminApproveDlg(MAdminRequiredGetAttributes, Object))
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

bool AdminMode::fSetFileAttributes(LPCWSTR Object, DWORD FileAttributes)
{
	CriticalSectionLock Lock(CS);
	bool Result=false;
	if(AdminApproveDlg(MAdminRequiredSetAttributes, Object))
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

bool AdminMode::fCreateHardLink(LPCWSTR Object, LPCWSTR Target, LPSECURITY_ATTRIBUTES SecurityAttributes)
{
	CriticalSectionLock Lock(CS);
	bool Result=false;
	if(AdminApproveDlg(MAdminRequiredHardLink, Object))
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

bool AdminMode::fCreateSymbolicLink(LPCWSTR Object, LPCWSTR Target, DWORD Flags)
{
	CriticalSectionLock Lock(CS);
	bool Result=false;
	if(AdminApproveDlg(MAdminRequiredSymLink, Object))
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


int AdminMode::fMoveToRecycleBin(SHFILEOPSTRUCT& FileOpStruct)
{
	CriticalSectionLock Lock(CS);
	int Result=0;
	if(AdminApproveDlg(MAdminRequiredRecycle, FileOpStruct.pFrom))
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

HANDLE AdminMode::fFindFirstFileEx(LPCWSTR Object, FINDEX_INFO_LEVELS InfoLevelId, LPVOID FindFileData, FINDEX_SEARCH_OPS SearchOp, LPVOID lpSearchFilter, DWORD AdditionalFlags)
{
	CriticalSectionLock Lock(CS);
	HANDLE Result=INVALID_HANDLE_VALUE;
	if(AdminApproveDlg(MAdminRequiredList, Object))
	{
		if(Opt.IsUserAdmin)
		{
			Privilege BackupPrivilege(SE_BACKUP_NAME), RestorePrivilege(SE_RESTORE_NAME);
			Result = FindFirstFileEx(Object, InfoLevelId, FindFileData, SearchOp, lpSearchFilter, AdditionalFlags);
		}
		else
		{
			if(Initialize())
			{
				if(SendCommand(C_FUNCTION_FINDFIRSTFILEEX))
				{
					if(WriteData(Object,Object?(StrLength(Object)+1)*sizeof(WCHAR):0))
					{
						if(WriteInt(InfoLevelId))
						{
							if(WriteInt(SearchOp))
							{
								// BUGBUG: SearchFilter ignored
								if(WriteInt(AdditionalFlags))
								{
									AutoObject OpResult;
									if(ReadData(OpResult))
									{
										AutoObject FindData;
										if(ReadData(FindData))
										{
											if(ReceiveLastError())
											{
												Result = *reinterpret_cast<PHANDLE>(OpResult.Get());
												if(Result!=INVALID_HANDLE_VALUE && FindFileData)
												{
													// BUGBUG: check InfoLevelId
													*reinterpret_cast<PWIN32_FIND_DATA>(FindFileData) = *reinterpret_cast<PWIN32_FIND_DATA>(FindData.Get());
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
	}
	return Result;
}

bool AdminMode::fFindNextFile(HANDLE Handle, PWIN32_FIND_DATA W32FindData)
{
	CriticalSectionLock Lock(CS);
	bool Result=false;
	if(Opt.IsUserAdmin)
	{
		Result = FindNextFile(Handle, W32FindData) != FALSE;
	}
	else
	{
		if(SendCommand(C_FUNCTION_FINDNEXTFILE))
		{
			if(WriteData(&Handle,Handle?sizeof(Handle):0))
			{
				int OpResult;
				if(ReadInt(OpResult))
				{
					AutoObject FindData;
					if(ReadData(FindData))
					{
						if(ReceiveLastError())
						{
							Result = OpResult != FALSE;
							if(Result && W32FindData)
							{
								*W32FindData = *reinterpret_cast<PWIN32_FIND_DATA>(FindData.Get());
							}
						}
					}
				}
			}
		}
	}
	return Result;
}

bool AdminMode::fFindClose(HANDLE Handle)
{
	CriticalSectionLock Lock(CS);
	bool Result=false;
	if(Opt.IsUserAdmin)
	{
		Result = FindClose(Handle) != FALSE;
	}
	else
	{
		if(SendCommand(C_FUNCTION_FINDCLOSE))
		{
			if(WriteData(&Handle,Handle?sizeof(Handle):0))
			{
				int OpResult;
				if(ReadInt(OpResult))
				{
					if(ReceiveLastError())
					{
						Result = OpResult != FALSE;
					}
				}
			}
		}
	}
	return Result;
}

bool AdminMode::fSetOwner(LPCWSTR Object, LPCWSTR Owner)
{
	CriticalSectionLock Lock(CS);
	bool Result=false;
	if(AdminApproveDlg(MAdminRequiredSetOwner, Object))
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

HANDLE AdminMode::fCreateFile(LPCWSTR Object, DWORD DesiredAccess, DWORD ShareMode, LPSECURITY_ATTRIBUTES SecurityAttributes, DWORD CreationDistribution, DWORD FlagsAndAttributes, HANDLE TemplateFile)
{
	CriticalSectionLock Lock(CS);
	HANDLE Result=INVALID_HANDLE_VALUE;
	if(AdminApproveDlg(MAdminRequiredOpen, Object))
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
												Result = *reinterpret_cast<PHANDLE>(OpResult.Get());
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

bool AdminMode::fCloseHandle(HANDLE Handle)
{
	CriticalSectionLock Lock(CS);
	bool Result=false;
	if(Opt.IsUserAdmin)
	{
		Result = CloseHandle(Handle) != FALSE;
	}
	else
	{
		if(SendCommand(C_FUNCTION_CLOSEHANDLE))
		{
			if(WriteData(&Handle,Handle?sizeof(Handle):0))
			{
				int OpResult;
				if(ReadInt(OpResult))
				{
					if(ReceiveLastError())
					{
						Result = OpResult != FALSE;
					}
				}
			}
		}
	}
	return Result;
}

bool AdminMode::fReadFile(HANDLE Handle, LPVOID Buffer, DWORD NumberOfBytesToRead, LPDWORD NumberOfBytesRead, LPOVERLAPPED Overlapped)
{
	CriticalSectionLock Lock(CS);
	bool Result=false;
	if(Opt.IsUserAdmin)
	{
		Result = ReadFile(Handle, Buffer, NumberOfBytesToRead, NumberOfBytesRead, Overlapped) != FALSE;
	}
	else
	{
		if(SendCommand(C_FUNCTION_READFILE))
		{
			if(WriteData(&Handle,Handle?sizeof(Handle):0))
			{
				if(WriteInt(NumberOfBytesToRead))
				{
					// BUGBUG: Overlapped ignored
					int OpResult;
					if(ReadInt(OpResult))
					{
						if(OpResult)
						{
							int Read;
							if(ReadInt(Read))
							{
								if(NumberOfBytesRead)
								{
									*NumberOfBytesRead = Read;
								}
								if(Read)
								{
									RawReadPipe(Pipe, Buffer, Read);
								}
							}
						}
						if(ReceiveLastError())
						{
							Result = OpResult != FALSE;
						}
					}
				}
			}
		}
	}
	return Result;
}

bool AdminMode::fWriteFile(HANDLE Handle, LPCVOID Buffer, DWORD NumberOfBytesToWrite, LPDWORD NumberOfBytesWritten, LPOVERLAPPED Overlapped)
{
	CriticalSectionLock Lock(CS);
	bool Result=false;
	if(Opt.IsUserAdmin)
	{
		Result = WriteFile(Handle, Buffer, NumberOfBytesToWrite, NumberOfBytesWritten, Overlapped) != FALSE;
	}
	else
	{
		if(SendCommand(C_FUNCTION_WRITEFILE))
		{
			if(WriteData(&Handle,Handle?sizeof(Handle):0))
			{
				if(WriteData(Buffer, Buffer?NumberOfBytesToWrite:0))
				{
					if(WriteInt(NumberOfBytesToWrite))
					{
						// BUGBUG: Overlapped ignored
						int OpResult;
						if(ReadInt(OpResult))
						{
							if(OpResult)
							{
								int Written = 0;
								if(ReadInt(Written))
								{
									if(NumberOfBytesWritten)
									{
										*NumberOfBytesWritten = Written;
									}
								}
							}
							if(ReceiveLastError())
							{
								Result = OpResult != FALSE;
							}
						}
					}
				}
			}
		}
	}
	return Result;
}

bool AdminMode::fSetFilePointerEx(HANDLE Handle, INT64 DistanceToMove, PINT64 NewFilePointer, DWORD MoveMethod)
{
	CriticalSectionLock Lock(CS);
	bool Result=false;
	if(Opt.IsUserAdmin)
	{
		Result = SetFilePointerEx(Handle, *reinterpret_cast<PLARGE_INTEGER>(&DistanceToMove), reinterpret_cast<PLARGE_INTEGER>(NewFilePointer), MoveMethod) != FALSE;
	}
	else
	{
		if(SendCommand(C_FUNCTION_SETFILEPOINTEREX))
		{
			if(WriteData(&Handle,Handle?sizeof(Handle):0))
			{
				if(WriteInt64(DistanceToMove))
				{
					if(WriteInt(MoveMethod))
					{
						int OpResult;
						if(ReadInt(OpResult))
						{
							if(OpResult)
							{
								INT64 NewPtr = 0;
								if(ReadInt64(NewPtr))
								{
									if(NewFilePointer)
									{
										*NewFilePointer = NewPtr;
									}
								}
								if(ReceiveLastError())
								{
									Result = OpResult != FALSE;
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

bool AdminMode::fSetEndOfFile(HANDLE Handle)
{
	CriticalSectionLock Lock(CS);
	bool Result=false;
	if(Opt.IsUserAdmin)
	{
		Result = SetEndOfFile(Handle) != FALSE;
	}
	else
	{
		if(SendCommand(C_FUNCTION_SETENDOFFILE))
		{
			if(WriteData(&Handle,Handle?sizeof(Handle):0))
			{
				int OpResult;
				if(ReadInt(OpResult))
				{
					if(ReceiveLastError())
					{
						Result = OpResult != FALSE;
					}
				}
			}
		}
	}
	return Result;
}

bool AdminMode::fGetFileTime(HANDLE Handle, LPFILETIME CreationTime, LPFILETIME LastAccessTime, LPFILETIME LastWriteTime)
{
	CriticalSectionLock Lock(CS);
	bool Result=false;
	if(Opt.IsUserAdmin)
	{
		Result = GetFileTime(Handle, CreationTime, LastAccessTime, LastWriteTime) != FALSE;
	}
	else
	{
		if(SendCommand(C_FUNCTION_GETFILETIME))
		{
			if(WriteData(&Handle,Handle?sizeof(Handle):0))
			{
				int OpResult;
				if(ReadInt(OpResult))
				{
					if(OpResult)
					{
						AutoObject AutoCreationTime;
						if(ReadData(AutoCreationTime))
						{
							if(CreationTime)
							{
								*CreationTime = *reinterpret_cast<LPFILETIME>(AutoCreationTime.Get());
							}
							AutoObject AutoLastAccessTime;
							if(ReadData(AutoLastAccessTime))
							{
								if(LastAccessTime)
								{
									*LastAccessTime = *reinterpret_cast<LPFILETIME>(AutoLastAccessTime.Get());
								}
								AutoObject AutoLastWriteTime;
								if(ReadData(AutoLastWriteTime))
								{
									if(LastWriteTime)
									{
										*LastWriteTime = *reinterpret_cast<LPFILETIME>(AutoLastWriteTime.Get());
									}
								}
							}
						}
					}
					if(ReceiveLastError())
					{
						Result = OpResult != FALSE;
					}
				}
			}
		}
	}
	return Result;
}

bool AdminMode::fSetFileTime(HANDLE Handle, const FILETIME* CreationTime, const FILETIME* LastAccessTime, const FILETIME* LastWriteTime)
{
	CriticalSectionLock Lock(CS);
	bool Result=false;
	if(Opt.IsUserAdmin)
	{
		Result = SetFileTime(Handle, CreationTime, LastAccessTime, LastWriteTime) != FALSE;
	}
	else
	{
		if(SendCommand(C_FUNCTION_SETFILETIME))
		{
			if(WriteData(&Handle,Handle?sizeof(Handle):0))
			{
				if(WriteData(CreationTime, CreationTime?sizeof(*CreationTime):0))
				{
					if(WriteData(LastAccessTime, LastAccessTime?sizeof(*LastAccessTime):0))
					{
						if(WriteData(LastWriteTime, LastWriteTime?sizeof(*LastWriteTime):0))
						{
							int OpResult;
							if(ReadInt(OpResult))
							{
								if(ReceiveLastError())
								{
									Result = OpResult != FALSE;
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

bool AdminMode::fGetFileSizeEx(HANDLE Handle, UINT64& Size)
{
	CriticalSectionLock Lock(CS);
	bool Result=false;
	if(Opt.IsUserAdmin)
	{
		Result = apiGetFileSizeEx(Handle, Size);
	}
	else
	{
		if(SendCommand(C_FUNCTION_GETFILESIZEEX))
		{
			if(WriteData(&Handle,Handle?sizeof(Handle):0))
			{
				int OpResult;
				if(ReadInt(OpResult))
				{
					if(OpResult)
					{
						INT64 iSize;
						ReadInt64(iSize);
						Size = iSize;
					}
					if(ReceiveLastError())
					{
						Result = OpResult != FALSE;
					}
				}
			}
		}
	}
	return Result;
}

bool AdminMode::fFlushFileBuffers(HANDLE Handle)
{
	CriticalSectionLock Lock(CS);
	bool Result = false;
	if(Opt.IsUserAdmin)
	{
		Result = FlushFileBuffers(Handle) != FALSE;
	}
	else
	{
		if(SendCommand(C_FUNCTION_FLUSHFILEBUFFERS))
		{
			if(WriteData(&Handle,Handle?sizeof(Handle):0))
			{
				int OpResult;
				if(ReadInt(OpResult))
				{
					if(ReceiveLastError())
					{
						Result = OpResult != FALSE;
					}
				}
			}
		}
	}
	return Result;
}

bool AdminMode::fGetFileInformationByHandle(HANDLE Handle, BY_HANDLE_FILE_INFORMATION& bhfi)
{
	CriticalSectionLock Lock(CS);
	bool Result = false;
	if(Opt.IsUserAdmin)
	{
		Result = GetFileInformationByHandle(Handle, &bhfi) != FALSE;
	}
	else
	{
		if(SendCommand(C_FUNCTION_GETFILEINFORMATIONBYHANDLE))
		{
			if(WriteData(&Handle,Handle?sizeof(Handle):0))
			{
				int OpResult;
				if(ReadInt(OpResult))
				{
					if(OpResult)
					{
						AutoObject AutoInformation;
						if(ReadData(AutoInformation))
						{
							bhfi = *reinterpret_cast<LPBY_HANDLE_FILE_INFORMATION>(AutoInformation.Get());
						}
					}
					if(ReceiveLastError())
					{
						Result = OpResult != FALSE;
					}
				}
			}
		}
	}
	return Result;
}

bool AdminMode::fDeviceIoControl(HANDLE Handle, DWORD IoControlCode, LPVOID InBuffer, DWORD InBufferSize, LPVOID OutBuffer, DWORD OutBufferSize, LPDWORD BytesReturned, LPOVERLAPPED Overlapped)
{
	CriticalSectionLock Lock(CS);
	bool Result=false;
	if(Opt.IsUserAdmin)
	{
		Result = DeviceIoControl(Handle, IoControlCode, InBuffer, InBufferSize, OutBuffer, OutBufferSize, BytesReturned, Overlapped) != FALSE;
	}
	else
	{
		if(SendCommand(C_FUNCTION_DEVICEIOCONTROL))
		{
			if(WriteData(&Handle,Handle?sizeof(Handle):0))
			{
				if(WriteInt(IoControlCode))
				{
					if(WriteData(InBuffer, InBufferSize))
					{
						if(WriteInt(InBufferSize))
						{
							if(WriteInt(OutBufferSize))
							{
								// BUGBUG: Overlapped ignored
								int OpResult;
								if(ReadInt(OpResult))
								{
									if(OpResult)
									{
										int Bytes = 0;
										if(ReadInt(Bytes))
										{
											if(BytesReturned)
											{
												*BytesReturned = Bytes;
											}
											if(Bytes)
											{
												RawReadPipe(Pipe, OutBuffer, Bytes);
											}
										}
									}
									if(ReceiveLastError())
									{
										Result = OpResult != FALSE;
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

DWORD WINAPI AdminCopyProgressRoutine(LARGE_INTEGER TotalFileSize, LARGE_INTEGER TotalBytesTransferred, LARGE_INTEGER StreamSize, LARGE_INTEGER StreamBytesTransferred, DWORD StreamNumber, DWORD CallbackReason, HANDLE SourceFile,HANDLE DestinationFile, LPVOID Data)
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
						int Result = CopyFileEx(From.GetStr(), To.GetStr(), UserCopyProgressRoutine.Get()?AdminCopyProgressRoutine:nullptr, Data.Get(), nullptr, Flags);
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
				SHFILEOPSTRUCT* FileOpStruct = reinterpret_cast<SHFILEOPSTRUCT*>(Struct.Get());
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

void FindFirstFileExHandler()
{
	AutoObject Object;
	if(ReadPipeData(Pipe, Object))
	{
		int InfoLevelId;
		if(ReadPipeInt(Pipe, InfoLevelId))
		{
			int SearchOp;
			if(ReadPipeInt(Pipe, SearchOp))
			{
				int AdditionalFlags;
				if(ReadPipeInt(Pipe, AdditionalFlags))
				{
					//BUGBUG: Check InfoLevelId
					//BUGBUG: SearchFilter ignored
					WIN32_FIND_DATA W32FindData;
					HANDLE Result = FindFirstFileEx(Object.GetStr(), static_cast<FINDEX_INFO_LEVELS>(InfoLevelId), &W32FindData, static_cast<FINDEX_SEARCH_OPS>(SearchOp), nullptr, AdditionalFlags);
					int LastError = GetLastError();
					if(WritePipeData(Pipe, &Result, sizeof(Result)))
					{
						if(WritePipeData(Pipe, &W32FindData, sizeof(WIN32_FIND_DATA)))
						{
							WritePipeInt(Pipe, LastError);
						}
					}
				}
			}
		}
	}
}

void FindNextFileHandler()
{
	AutoObject Handle;
	if(ReadPipeData(Pipe, Handle))
	{
		WIN32_FIND_DATA W32FindData;
		int Result = FindNextFile(*reinterpret_cast<PHANDLE>(Handle.Get()), &W32FindData);
		int LastError = GetLastError();
		if(WritePipeInt(Pipe, Result))
		{
			if(WritePipeData(Pipe, &W32FindData, sizeof(WIN32_FIND_DATA)))
			{
				WritePipeInt(Pipe, LastError);
			}
		}
	}
}

void FindCloseHandler()
{
	AutoObject Handle;
	if(ReadPipeData(Pipe, Handle))
	{
		int Result = FindClose(*reinterpret_cast<PHANDLE>(Handle.Get()));
		int LastError = GetLastError();
		if(WritePipeInt(Pipe, Result))
		{
			WritePipeInt(Pipe, LastError);
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

void CloseHandleHandler()
{
	AutoObject Handle;
	if(ReadPipeData(Pipe, Handle))
	{
		int Result = CloseHandle(*reinterpret_cast<PHANDLE>(Handle.Get()));
		int LastError = GetLastError();
		if(WritePipeInt(Pipe, Result))
		{
			WritePipeInt(Pipe, LastError);
		}
	}
}

void ReadFileHandler()
{
	AutoObject Handle;
	if(ReadPipeData(Pipe, Handle))
	{
		int NumberOfBytesToRead = 0;
		if(ReadPipeInt(Pipe, NumberOfBytesToRead))
		{
			LPBYTE Buffer = new BYTE[NumberOfBytesToRead];
			if(Buffer)
			{
				int Read = 0;
				// BUGBUG: Overlapped ignored
				int Result = ReadFile(*reinterpret_cast<PHANDLE>(Handle.Get()), Buffer, NumberOfBytesToRead, reinterpret_cast<LPDWORD>(&Read), nullptr);
				int LastError = GetLastError();
				if(WritePipeInt(Pipe, Result))
				{
					if(Result)
					{
						if(WritePipeInt(Pipe, Read))
						{
							if(Read)
							{
								RawWritePipe(Pipe, Buffer, Read);
							}
						}
					}
					WritePipeInt(Pipe, LastError);
				}
				delete[] Buffer;
			}
		}
	}
}

void WriteFileHandler()
{
	AutoObject Handle;
	if(ReadPipeData(Pipe, Handle))
	{
		AutoObject Buffer;
		if(ReadPipeData(Pipe, Buffer))
		{
			int NumberOfBytesToWrite;
			if(ReadPipeInt(Pipe, NumberOfBytesToWrite))
			{
				// BUGBUG: Overlapped ignored
				int Written;
				int Result = WriteFile(*reinterpret_cast<PHANDLE>(Handle.Get()), Buffer.Get(), NumberOfBytesToWrite, reinterpret_cast<LPDWORD>(&Written), nullptr);
				int LastError = GetLastError();
				if(WritePipeInt(Pipe, Result))
				{
					if(Result)
					{
						WritePipeInt(Pipe, Written);
					}
					WritePipeInt(Pipe, LastError);
				}
			}
		}
	}
}

void SetFilePointerExHandler()
{
	AutoObject Handle;
	if(ReadPipeData(Pipe, Handle))
	{
		INT64 DistanceToMove;
		if(ReadPipeInt64(Pipe, DistanceToMove))
		{
			INT MoveMethod;
			if(ReadPipeInt(Pipe, MoveMethod))
			{
				INT64 NewFilePointer;
				int Result = SetFilePointerEx(*reinterpret_cast<PHANDLE>(Handle.Get()), *reinterpret_cast<PLARGE_INTEGER>(&DistanceToMove), reinterpret_cast<PLARGE_INTEGER>(&NewFilePointer), MoveMethod);
				int LastError = GetLastError();
				if(WritePipeInt(Pipe, Result))
				{
					if(Result)
					{
						WritePipeInt64(Pipe, NewFilePointer);
					}
					WritePipeInt(Pipe, LastError);
				}
			}
		}
	}
}

void SetEndOfFileHandler()
{
	AutoObject Handle;
	if(ReadPipeData(Pipe, Handle))
	{
		int Result = SetEndOfFile(*reinterpret_cast<PHANDLE>(Handle.Get()));
		int LastError = GetLastError();
		if(WritePipeInt(Pipe, Result))
		{
			WritePipeInt(Pipe, LastError);
		}
	}
}

void GetFileTimeHandler()
{
	AutoObject Handle;
	if(ReadPipeData(Pipe, Handle))
	{
		FILETIME CreationTime, LastAccessTime, LastWriteTime;
		int Result = GetFileTime(*reinterpret_cast<PHANDLE>(Handle.Get()), &CreationTime, &LastAccessTime, &LastWriteTime);
		int LastError = GetLastError();
		if(WritePipeInt(Pipe, Result))
		{
			if(Result)
			{
				if(WritePipeData(Pipe, &CreationTime, sizeof(CreationTime)))
				{
					if(WritePipeData(Pipe, &LastAccessTime, sizeof(LastAccessTime)))
					{
						WritePipeData(Pipe, &LastWriteTime, sizeof(LastWriteTime));
					}
				}
			}
			WritePipeInt(Pipe, LastError);
		}
	}
}

void SetFileTimeHandler()
{
	AutoObject Handle;
	if(ReadPipeData(Pipe, Handle))
	{
		AutoObject CreationTime;
		if(ReadPipeData(Pipe, CreationTime))
		{
			AutoObject LastAccessTime;
			if(ReadPipeData(Pipe, LastAccessTime))
			{
				AutoObject LastWriteTime;
				if(ReadPipeData(Pipe, LastWriteTime))
				{
					int Result = SetFileTime(*reinterpret_cast<PHANDLE>(Handle.Get()), reinterpret_cast<PFILETIME>(CreationTime.Get()), reinterpret_cast<PFILETIME>(LastAccessTime.Get()), reinterpret_cast<PFILETIME>(LastWriteTime.Get()));
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

void GetFileSizeExHandler()
{
	AutoObject Handle;
	if(ReadPipeData(Pipe, Handle))
	{
		UINT64 Size;
		int Result = apiGetFileSizeEx(*reinterpret_cast<PHANDLE>(Handle.Get()), Size);
		int LastError = GetLastError();
		if(WritePipeInt(Pipe, Result))
		{
			if(Result)
			{
				WritePipeInt64(Pipe, Size);
			}
			WritePipeInt(Pipe, LastError);
		}
	}
}

void FlushFileBuffersHandler()
{
	AutoObject Handle;
	if(ReadPipeData(Pipe, Handle))
	{
		int Result = FlushFileBuffers(*reinterpret_cast<PHANDLE>(Handle.Get()));
		int LastError = GetLastError();
		if(WritePipeInt(Pipe, Result))
		{
			WritePipeInt(Pipe, LastError);
		}
	}
}

void GetFileInformationByHandleHandler()
{
	AutoObject Handle;
	if(ReadPipeData(Pipe, Handle))
	{
		BY_HANDLE_FILE_INFORMATION bhfi;
		int Result = GetFileInformationByHandle(*reinterpret_cast<PHANDLE>(Handle.Get()), &bhfi);
		int LastError = GetLastError();
		if(WritePipeInt(Pipe, Result))
		{
			if(Result)
			{
				WritePipeData(Pipe, &bhfi, sizeof(bhfi));
			}
			WritePipeInt(Pipe, LastError);
		}
	}
}

void DeviceIoControlHandler()
{
	AutoObject Handle;
	if(ReadPipeData(Pipe, Handle))
	{
		int IoControlCode;
		if(ReadPipeInt(Pipe, IoControlCode))
		{
			AutoObject InBuffer;
			if(ReadPipeData(Pipe, InBuffer))
			{
				int InBufferSize;
				if(ReadPipeInt(Pipe, InBufferSize))
				{
					int OutBufferSize;
					if(ReadPipeInt(Pipe, OutBufferSize))
					{
						LPBYTE OutBuffer = nullptr;
						if(OutBufferSize)
						{
							OutBuffer = new BYTE[OutBufferSize];
						}
						// BUGBUG: Overlapped ignored
						int BytesReturned = 0;
						int Result = DeviceIoControl(*reinterpret_cast<PHANDLE>(Handle.Get()), IoControlCode, InBuffer.Get(), InBufferSize, OutBuffer, OutBufferSize, reinterpret_cast<LPDWORD>(&BytesReturned), nullptr);
						int LastError = GetLastError();
						if(WritePipeInt(Pipe, Result))
						{
							if(Result)
							{
								if(WritePipeInt(Pipe, BytesReturned))
								{
									if(BytesReturned)
									{
										RawWritePipe(Pipe, OutBuffer, BytesReturned);
									}
								}
							}
							WritePipeInt(Pipe, LastError);
						}
						if(OutBuffer)
						{
							delete[] OutBuffer;
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

	case C_FUNCTION_FINDFIRSTFILEEX:
		FindFirstFileExHandler();
		break;

	case C_FUNCTION_FINDNEXTFILE:
		FindNextFileHandler();
		break;

	case C_FUNCTION_FINDCLOSE:
		FindCloseHandler();
		break;

	case C_FUNCTION_SETOWNER:
		SetOwnerHandler();
		break;

	case C_FUNCTION_CREATEFILE:
		CreateFileHandler();
		break;

	case C_FUNCTION_CLOSEHANDLE:
		CloseHandleHandler();
		break;

	case C_FUNCTION_READFILE:
		ReadFileHandler();
		break;

	case C_FUNCTION_WRITEFILE:
		WriteFileHandler();
		break;

	case C_FUNCTION_SETFILEPOINTEREX:
		SetFilePointerExHandler();
		break;

	case C_FUNCTION_SETENDOFFILE:
		SetEndOfFileHandler();
		break;

	case C_FUNCTION_GETFILETIME:
		GetFileTimeHandler();
		break;

	case C_FUNCTION_SETFILETIME:
		SetFileTimeHandler();
		break;

	case C_FUNCTION_GETFILESIZEEX:
		GetFileSizeExHandler();
		break;

	case C_FUNCTION_FLUSHFILEBUFFERS:
		FlushFileBuffersHandler();
		break;

	case C_FUNCTION_GETFILEINFORMATIONBYHANDLE:
		GetFileInformationByHandleHandler();
		break;

	case C_FUNCTION_DEVICEIOCONTROL:
		DeviceIoControlHandler();
		break;

	}
	return Exit;
}

int AdminMain(int PID)
{
	int Result = ERROR_SUCCESS;

	SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOOPENFILEERRORBOX);

	Privilege
		BackupPrivilege(SE_BACKUP_NAME),
		RestorePrivilege(SE_RESTORE_NAME),
		TakeOwnershipPrivilege(SE_TAKE_OWNERSHIP_NAME),
		CreateSymbolicLinkPrivilege(SE_CREATE_SYMBOLIC_LINK_NAME);

	FormatString strPipe;
	strPipe << PIPE_NAME << PID;
	WaitNamedPipe(strPipe, NMPWAIT_WAIT_FOREVER);
	Pipe = CreateFile(strPipe,GENERIC_READ|GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
	if (Pipe != INVALID_HANDLE_VALUE)
	{
		if(WritePipeInt(Pipe, GetCurrentProcessId()))
		{
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
	return Result;
}
