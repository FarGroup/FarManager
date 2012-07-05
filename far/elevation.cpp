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
		Data(nullptr),
		size(0)
	{
	}

	LPVOID Allocate(size_t Size)
	{
		Free();
		Data=xf_malloc(Size);
		size = Size;
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

	size_t Size() {return size;}

	~AutoObject()
	{
		Free();
	}

	LPVOID Get()
	{
		return Data;
	}

private:
	LPVOID Data;
	size_t size;
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

template<typename T>
inline bool ReadPipe(HANDLE Pipe, T& Data)
{
	bool Result=false;
	size_t DataSize = 0;
	if(RawReadPipe(Pipe, &DataSize, sizeof(DataSize)))
	{
		assert(DataSize == sizeof(Data));
		Result = RawReadPipe(Pipe, &Data, sizeof(Data));
	}
	return Result;
}

template<>
inline bool ReadPipe(HANDLE Pipe, string& Data)
{
	bool Result=false;
	size_t DataSize = 0;
	if(RawReadPipe(Pipe, &DataSize, sizeof(DataSize)))
	{
		if(DataSize)
		{
			LPVOID Ptr=Data.GetBuffer(DataSize/sizeof(wchar_t));
			if(RawReadPipe(Pipe, Ptr, DataSize))
			{
				Data.ReleaseBuffer(DataSize/sizeof(wchar_t)-1);
				Result=true;
			}
		}
		else
		{
			Result=true;
		}
	}
	return Result;
}

template<>
inline bool ReadPipe(HANDLE Pipe, AutoObject& Data)
{
	bool Result=false;
	size_t DataSize = 0;
	if(RawReadPipe(Pipe, &DataSize, sizeof(DataSize)))
	{
		if(DataSize)
		{
			LPVOID Ptr=Data.Allocate(DataSize);
			if(RawReadPipe(Pipe, Ptr, DataSize))
			{
				Result=true;
			}
		}
		else
		{
			Result=true;
		}
	}
	return Result;
}

template<typename T>
inline bool ReadPipe(HANDLE Pipe, T* Data) {static_assert(sizeof(T) < 0 /* always false */, "ReadPipe template requires a reference to an object"); return false;}

bool WritePipe(HANDLE Pipe, const void* Data, size_t DataSize)
{
	bool Result=false;
	if(RawWritePipe(Pipe, &DataSize, sizeof(DataSize)) && RawWritePipe(Pipe, Data, DataSize))
	{
		Result=true;
	}
	return Result;
}

template<typename T>
inline bool WritePipe(HANDLE Pipe, const T& Data)
{
	return WritePipe(Pipe, &Data, sizeof(Data));
}

template<typename T>
inline bool WritePipe(HANDLE Pipe, const T* Data) {static_assert(sizeof(T) < 0 /* always false */, "WritePipe template requires a reference to an object"); return false;}

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
	Job(nullptr),
	PID(0),
	Elevation(false),
	DontAskAgain(false),
	Approve(false),
	AskApprove(true),
	Recurse(false)
{
}

elevation::~elevation()
{
	if(Pipe != INVALID_HANDLE_VALUE)
	{
		SendCommand(C_SERVICE_EXIT);
		DisconnectNamedPipe(Pipe);
		CloseHandle(Pipe);
	}

	if(Job)
	{
		CloseHandle(Job);
	}
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

bool elevation::Write(LPCVOID Data,size_t DataSize) const
{
	return WritePipe(Pipe, Data, DataSize);
}

template<typename T>
inline bool elevation::Read(T& Data) const
{
	return ReadPipe(Pipe, Data);
}

template<>
inline bool elevation::Read(AutoObject& Data) const
{
	return ReadPipe(Pipe, Data);
}

template<typename T>
inline bool elevation::Write(const T& Data) const
{
	return WritePipe(Pipe, Data);
}

template<>
inline bool elevation::Write(const string& Data) const
{
	return Write(Data.CPtr(), (Data.GetLength()+1)*sizeof(wchar_t));
}

bool elevation::SendCommand(ELEVATION_COMMAND Command) const
{
	return WritePipe(Pipe, Command);
}

struct ERRORCODES
{
	DWORD Win32Error;
	NTSTATUS NtError;

	ERRORCODES():Win32Error(GetLastError()), NtError(ifn.RtlGetLastNtStatus()){}
	ERRORCODES(DWORD Win32Error, NTSTATUS NtError):Win32Error(Win32Error), NtError(NtError){}
};

bool elevation::ReceiveLastError() const
{
	ERRORCODES ErrorCodes(ERROR_SUCCESS, STATUS_SUCCESS);
	bool Result = ReadPipe(Pipe, ErrorCodes);
	SetLastError(ErrorCodes.Win32Error);
	ifn.RtlNtStatusToDosError(ErrorCodes.NtError);
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
						ea.Trustee.ptstrName = static_cast<LPWSTR>(AdminSID);
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

			BOOL InJob = FALSE;
			if(!Job)
			{
				// IsProcessInJob not exist in win2k. use QueryInformationJobObject(nullptr, ...) instead.
				// IsProcessInJob(GetCurrentProcess(), nullptr, &InJob);

				JOBOBJECT_EXTENDED_LIMIT_INFORMATION jeli={};
				InJob = QueryInformationJobObject(nullptr, JobObjectExtendedLimitInformation, &jeli, sizeof(jeli), nullptr);
				if (!InJob)
				{
					Job = CreateJobObject(nullptr, nullptr);
					if(Job)
					{
						jeli.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_SILENT_BREAKAWAY_OK;
						if(SetInformationJobObject(Job, JobObjectExtendedLimitInformation, &jeli, sizeof(jeli)))
						{
							AssignProcessToJobObject(Job, GetCurrentProcess());
						}
					}
				}
			}

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
				if(!InJob && Job)
				{
					AssignProcessToJobObject(Job, Process);
				}
				OVERLAPPED Overlapped;
				Event AEvent;
				Overlapped.hEvent = AEvent.Handle();
				ConnectNamedPipe(Pipe, &Overlapped);
				if(AEvent.Wait(15000))
				{
					DWORD NumberOfBytesTransferred;
					if(GetOverlappedResult(Pipe, &Overlapped, &NumberOfBytesTransferred, FALSE))
					{
						if(ReadPipe(Pipe, PID))
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
						if(Job)
						{
							CloseHandle(Job);
							Job = nullptr;
						}
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

intptr_t WINAPI ElevationApproveDlgProc(HANDLE hDlg,int Msg,int Param1,void* Param2)
{
	switch (Msg)
	{
	case DN_CTLCOLORDLGITEM:
		{
			if(Param1==AAD_EDIT_OBJECT)
			{
				FarColor Color=ColorIndexToColor(COL_DIALOGTEXT);
				FarDialogItemColors* Colors = static_cast<FarDialogItemColors*>(Param2);
				Colors->Colors[0] = Color;
				Colors->Colors[2] = Color;
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
	const string& Object;
	LNGID Why;
	bool& AskApprove;
	bool& Approve;
	bool& DontAskAgain;
	EAData(Event* pEvent, const string& Object, LNGID Why, bool& AskApprove, bool& Approve, bool& DontAskAgain):
		pEvent(pEvent), Object(Object), Why(Why), AskApprove(AskApprove), Approve(Approve), DontAskAgain(DontAskAgain){}
};

void ElevationApproveDlgSync(LPVOID Param)
{
	EAData* Data=static_cast<EAData*>(Param);
	enum {DlgX=64,DlgY=12};
	FarDialogItem ElevationApproveDlgData[]=
	{
		{DI_DOUBLEBOX,3,1,DlgX-4,DlgY-2,0,nullptr,nullptr,0,MSG(MAccessDenied)},
		{DI_TEXT,5,2,0,2,0,nullptr,nullptr,0,MSG(Opt.IsUserAdmin?MElevationRequiredPrivileges:MElevationRequired)},
		{DI_TEXT,5,3,0,3,0,nullptr,nullptr,0,MSG(Data->Why)},
		{DI_EDIT,5,4,DlgX-6,4,0,nullptr,nullptr,DIF_READONLY,Data->Object},
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
	Frame* Current = FrameManager->GetCurrentFrame();
	if(Current)
	{
		Current->Lock();
	}
	Dlg.Process();
	if(Current)
	{
		Current->Unlock();
	}

	Data->AskApprove=!ElevationApproveDlg[AAD_CHECKBOX_DOFORALL].Selected;
	Data->Approve=Dlg.GetExitCode()==AAD_BUTTON_OK;
	Data->DontAskAgain=ElevationApproveDlg[AAD_CHECKBOX_DONTASKAGAIN].Selected!=FALSE;
	if(Data->pEvent)
	{
		Data->pEvent->Set();
	}
}

bool elevation::ElevationApproveDlg(LNGID Why, const string& Object)
{
	// request for backup&restore privilege is useless if the user already has them
	{
		GuardLastError Error;
		if(AskApprove &&  IsUserAdmin() && CheckPrivilege(SE_BACKUP_NAME) && CheckPrivilege(SE_RESTORE_NAME))
		{
			Approve = true;
			AskApprove = false;
		}
	}

	if(!(Opt.IsUserAdmin && !(Opt.ElevationMode&ELEVATION_USE_PRIVILEGES)) &&
		AskApprove && !DontAskAgain && !Recurse &&
		FrameManager && !FrameManager->ManagerIsDown())
	{
		Recurse = true;
		GuardLastError error;
		TaskBarPause TBP;
		EAData Data(nullptr, Object, Why, AskApprove, Approve, DontAskAgain);
		if(!MainThread())
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

bool elevation::fCreateDirectoryEx(const string& TemplateObject, const string& Object, LPSECURITY_ATTRIBUTES Attributes)
{
	CriticalSectionLock Lock(CS);
	bool Result=false;
	if(ElevationApproveDlg(MElevationRequiredCreate, Object))
	{
		if(Opt.IsUserAdmin)
		{
			Privilege BackupPrivilege(SE_BACKUP_NAME), RestorePrivilege(SE_RESTORE_NAME);
			Result = (TemplateObject.IsEmpty()?CreateDirectory(Object, Attributes) : CreateDirectoryEx(TemplateObject, Object, Attributes)) != FALSE;
		}
		else if(Initialize() && SendCommand(C_FUNCTION_CREATEDIRECTORYEX) && Write(TemplateObject) && Write(Object))
		{
			// BUGBUG: SecurityAttributes ignored
			bool OpResult = false;
			if(Read(OpResult) && ReceiveLastError())
			{
				Result = OpResult;
			}
		}
	}
	return Result;
}

bool elevation::fRemoveDirectory(const string& Object)
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
		else if(Initialize() && SendCommand(C_FUNCTION_REMOVEDIRECTORY) && Write(Object))
		{
			bool OpResult = false;
			if(Read(OpResult) && ReceiveLastError())
			{
				Result = OpResult;
			}
		}
	}
	return Result;
}

bool elevation::fDeleteFile(const string& Object)
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
		else if(Initialize() && SendCommand(C_FUNCTION_DELETEFILE) && Write(Object))
		{
			bool OpResult = false;
			if(Read(OpResult) && ReceiveLastError())
			{
				Result = OpResult;
			}
		}
	}
	return Result;
}

void elevation::fCallbackRoutine(LPPROGRESS_ROUTINE ProgressRoutine) const
{
	if(ProgressRoutine)
	{
		LARGE_INTEGER TotalFileSize, TotalBytesTransferred, StreamSize, StreamBytesTransferred;
		DWORD StreamNumber, CallbackReason;
		// BUGBUG: SourceFile, DestinationFile ignored
		AutoObject Data;
		if(Read(TotalFileSize) && Read(TotalBytesTransferred) && Read(StreamSize) && Read(StreamBytesTransferred) && Read(StreamNumber) && Read(CallbackReason) && Read(Data))
		{
			int Result=ProgressRoutine(TotalFileSize, TotalBytesTransferred, StreamSize, StreamBytesTransferred, StreamNumber, CallbackReason, INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE, Data.Get());
			if(Write(CallbackMagic))
			{
				Write(Result);
			}
		}
	}
}

bool elevation::fCopyFileEx(const string& From, const string& To, LPPROGRESS_ROUTINE ProgressRoutine, LPVOID Data, LPBOOL Cancel, DWORD Flags)
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
		// BUGBUG: Cancel ignored
		else if(Initialize() && SendCommand(C_FUNCTION_COPYFILEEX) && Write(From) && Write(To) && Write(&ProgressRoutine, sizeof(ProgressRoutine)) && Write(&Data, sizeof(Data)) && Write(Flags))
		{
			int OpResult = 0;
			if(Read(OpResult))
			{
				if (OpResult == CallbackMagic)
				{
					while(OpResult == CallbackMagic)
					{
						fCallbackRoutine(ProgressRoutine);
						Read(OpResult);
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
	return Result;
}

bool elevation::fMoveFileEx(const string& From, const string& To, DWORD Flags)
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
		else if(Initialize() && SendCommand(C_FUNCTION_MOVEFILEEX) && Write(From) && Write(To) && Write(Flags))
		{
			bool OpResult = false;
			if(Read(OpResult) && ReceiveLastError())
			{
				Result = OpResult;
			}
		}
	}
	return Result;
}

DWORD elevation::fGetFileAttributes(const string& Object)
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
		else if(Initialize() && SendCommand(C_FUNCTION_GETFILEATTRIBUTES) && Write(Object))
		{
			DWORD OpResult = INVALID_FILE_ATTRIBUTES;
			if(Read(OpResult) && ReceiveLastError())
			{
				Result = OpResult;
			}
		}
	}
	return Result;
}

bool elevation::fSetFileAttributes(const string& Object, DWORD FileAttributes)
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
		else if(Initialize() && SendCommand(C_FUNCTION_SETFILEATTRIBUTES) && Write(Object) && Write(FileAttributes))
		{
			bool OpResult = false;
			if(Read(OpResult) && ReceiveLastError())
			{
				Result = OpResult;
			}
		}
	}
	return Result;
}

bool elevation::fCreateHardLink(const string& Object, const string& Target, LPSECURITY_ATTRIBUTES SecurityAttributes)
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
		// BUGBUG: SecurityAttributes ignored.
		else if(Initialize() && SendCommand(C_FUNCTION_CREATEHARDLINK) && Write(Object) && Write(Target))
		{
			bool OpResult = false;
			if(Read(OpResult) && ReceiveLastError())
			{
				Result = OpResult;
			}
		}
	}
	return Result;
}

bool elevation::fCreateSymbolicLink(const string& Object, const string& Target, DWORD Flags)
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
		else if(Initialize() && SendCommand(C_FUNCTION_CREATESYMBOLICLINK) && Write(Object) && Write(Target) && Write(Flags))
		{
			bool OpResult = false;
			if(Read(OpResult) && ReceiveLastError())
			{
				Result = OpResult != 0;
			}
		}
	}
	return Result;
}

int elevation::fMoveToRecycleBin(SHFILEOPSTRUCT& FileOpStruct)
{
	CriticalSectionLock Lock(CS);
	int Result = 0x78; //DE_ACCESSDENIEDSRC
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
				if(SendCommand(C_FUNCTION_MOVETORECYCLEBIN) && Write(FileOpStruct)
					&& Write(FileOpStruct.pFrom,FileOpStruct.pFrom?(StrLength(FileOpStruct.pFrom)+1+1)*sizeof(WCHAR):0) // achtung! +1
					&& Write(FileOpStruct.pTo,FileOpStruct.pTo?(StrLength(FileOpStruct.pTo)+1+1)*sizeof(WCHAR):0)) // achtung! +1
				{
					int OpResult = 0;
					if(Read(OpResult) && Read(FileOpStruct.fAnyOperationsAborted))
					{
						// achtung! no "last error" here
						Result = OpResult;
					}
				}
			}
			else
			{
				Result = 0x75; // DE_OPCANCELLED
			}
		}
	}
	return Result;
}

bool elevation::fSetOwner(const string& Object, const string& Owner)
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
		else if(Initialize() && SendCommand(C_FUNCTION_SETOWNER) && Write(Object) && Write(Owner))
		{
			bool OpResult = false;
			if(Read(OpResult) && ReceiveLastError())
			{
				Result = OpResult;
			}
		}
	}
	return Result;
}

HANDLE elevation::fCreateFile(const string& Object, DWORD DesiredAccess, DWORD ShareMode, LPSECURITY_ATTRIBUTES SecurityAttributes, DWORD CreationDistribution, DWORD FlagsAndAttributes, HANDLE TemplateFile)
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
		// BUGBUG: SecurityAttributes ignored
		// BUGBUG: TemplateFile ignored
		else if(Initialize() && SendCommand(C_FUNCTION_CREATEFILE) && Write(Object) && Write(DesiredAccess) && Write(ShareMode) && Write(CreationDistribution) && Write(FlagsAndAttributes))
		{
			intptr_t OpResult;
			if(Read(OpResult) && ReceiveLastError())
			{
				Result = ToPtr(OpResult);
			}
		}
	}
	return Result;
}

bool elevation::fSetFileEncryption(const string& Object, bool Encrypt)
{
	CriticalSectionLock Lock(CS);
	bool Result=false;
	if(ElevationApproveDlg(Encrypt? MElevationRequiredEncryptFile : MElevationRequiredDecryptFile, Object))
	{
		if(Opt.IsUserAdmin)
		{
			Privilege BackupPrivilege(SE_BACKUP_NAME), RestorePrivilege(SE_RESTORE_NAME);
			Result = apiSetFileEncryptionInternal(Object, Encrypt);
		}
		else if(Initialize() && SendCommand(C_FUNCTION_SETENCRYPTION) && Write(Object) && Write(Encrypt))
		{
			bool OpResult = false;
			if(Read(OpResult) && ReceiveLastError())
			{
				Result = OpResult;
			}
		}
	}
	return Result;
}

bool elevation::fOpenVirtualDisk(VIRTUAL_STORAGE_TYPE& VirtualStorageType, const string& Object, VIRTUAL_DISK_ACCESS_MASK VirtualDiskAccessMask, OPEN_VIRTUAL_DISK_FLAG Flags, OPEN_VIRTUAL_DISK_PARAMETERS& Parameters, HANDLE& Handle)
{
	CriticalSectionLock Lock(CS);
	bool Result=false;
	if(ElevationApproveDlg(MElevationRequiredCreate, Object))
	{
		if(Opt.IsUserAdmin)
		{
			Privilege BackupPrivilege(SE_BACKUP_NAME), RestorePrivilege(SE_RESTORE_NAME);
			Result = apiOpenVirtualDiskInternal(VirtualStorageType, Object, VirtualDiskAccessMask, Flags, Parameters, Handle);
		}
		else if(Initialize() && SendCommand(C_FUNCTION_OPENVIRTUALDISK) && Write(VirtualStorageType) && Write(Object) && Write(VirtualDiskAccessMask) && Write(Flags) && Write(Parameters))
		{
			bool OpResult = false;
			if(Read(OpResult) && ReceiveLastError())
			{
				Result = OpResult;
			}
			if(Result)
			{
				intptr_t iHandle;
				Read(iHandle);
				Handle = ToPtr(iHandle);
			}
		}
	}
	return Result;
}


bool ElevationRequired(ELEVATION_MODE Mode, bool UseNtStatus)
{
	bool Result = false;
	if(Opt.ElevationMode&Mode)
	{
		if(UseNtStatus && ifn.RtlGetLastNtStatusPresent())
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
	// BUGBUG: SourceFile, DestinationFile ignored
	if (WritePipe(Pipe, CallbackMagic) && WritePipe(Pipe, TotalFileSize) && WritePipe(Pipe, TotalBytesTransferred) && WritePipe(Pipe, StreamSize) && WritePipe(Pipe, StreamBytesTransferred) && WritePipe(Pipe, StreamNumber) && WritePipe(Pipe, CallbackReason) && WritePipe(Pipe, &Data, sizeof(Data)))
	{
		for(;;)
		{
			ReadPipe(Pipe, Result);
			if (Result == CallbackMagic)
			{
				ReadPipe(Pipe, Result);
				break;
			}
			else
			{
				// nested call from ProgressRoutine()
				Process(Result);
			}
		}
	}
	return Result;
}

void CreateDirectoryExHandler()
{
	string TemplateObject;
	if(ReadPipe(Pipe, TemplateObject))
	{
		string Object;
		if(ReadPipe(Pipe, Object))
		{
			// BUGBUG, SecurityAttributes ignored
			bool Result = TemplateObject.IsEmpty()? CreateDirectory(Object, nullptr) != FALSE : CreateDirectoryEx(TemplateObject, Object, nullptr) != FALSE;
			ERRORCODES ErrorCodes;
			if(WritePipe(Pipe, Result))
			{
				WritePipe(Pipe, ErrorCodes);
			}
		}
	}
}

void RemoveDirectoryHandler()
{
	string Object;
	if(ReadPipe(Pipe, Object))
	{
		bool Result = RemoveDirectory(Object) != FALSE;
		ERRORCODES ErrorCodes;
		if(WritePipe(Pipe, Result))
		{
			WritePipe(Pipe, ErrorCodes);
		}
	}
}

void DeleteFileHandler()
{
	string Object;
	if(ReadPipe(Pipe, Object))
	{
		bool Result = DeleteFile(Object) != FALSE;
		ERRORCODES ErrorCodes;
		if(WritePipe(Pipe, Result))
		{
			WritePipe(Pipe, ErrorCodes);
		}
	}
}

void CopyFileExHandler()
{
	string From, To;
	AutoObject UserCopyProgressRoutine, Data;
	DWORD Flags = 0;
	// BUGBUG: Cancel ignored
	if(ReadPipe(Pipe, From) && ReadPipe(Pipe, To) && ReadPipe(Pipe, UserCopyProgressRoutine) && ReadPipe(Pipe, Data) && ReadPipe(Pipe, Flags))
	{
		int Result = CopyFileEx(From, To, UserCopyProgressRoutine.Get()?ElevationCopyProgressRoutine:nullptr, Data.Get(), nullptr, Flags);
		ERRORCODES ErrorCodes;
		if(WritePipe(Pipe, Result))
		{
			WritePipe(Pipe, ErrorCodes);
		}
	}
}

void MoveFileExHandler()
{
	string From, To;
	DWORD Flags = 0;
	if(ReadPipe(Pipe, From) && ReadPipe(Pipe, To) && ReadPipe(Pipe, Flags))
	{
		bool Result = MoveFileEx(From, To, Flags) != FALSE;
		ERRORCODES ErrorCodes;
		if(WritePipe(Pipe, Result))
		{
			WritePipe(Pipe, ErrorCodes);
		}
	}
}

void GetFileAttributesHandler()
{
	string Object;
	if(ReadPipe(Pipe, Object))
	{
		DWORD Result = GetFileAttributes(Object);
		ERRORCODES ErrorCodes;
		if(WritePipe(Pipe, Result))
		{
			WritePipe(Pipe, ErrorCodes);
		}
	}
}

void SetFileAttributesHandler()
{
	string Object;
	DWORD Attributes = 0;
	if(ReadPipe(Pipe, Object) && ReadPipe(Pipe, Attributes))
	{
		bool Result = SetFileAttributes(Object, Attributes) != FALSE;
		ERRORCODES ErrorCodes;
		if(WritePipe(Pipe, Result))
		{
			WritePipe(Pipe, ErrorCodes);
		}
	}
}

void CreateHardLinkHandler()
{
	string Object, Target;
	if(ReadPipe(Pipe, Object) && ReadPipe(Pipe, Target))
	{
		// BUGBUG: SecurityAttributes ignored.
		bool Result = CreateHardLink(Object, Target, nullptr) != FALSE;
		ERRORCODES ErrorCodes;
		if(WritePipe(Pipe, Result))
		{
			WritePipe(Pipe, ErrorCodes);
		}
	}
}

void CreateSymbolicLinkHandler()
{
	string Object, Target;
	DWORD Flags = 0;
	if(ReadPipe(Pipe, Object) && ReadPipe(Pipe, Target) && ReadPipe(Pipe, Flags))
	{
		bool Result = CreateSymbolicLinkInternal(Object, Target, Flags);
		ERRORCODES ErrorCodes;
		if(WritePipe(Pipe, Result))
		{
			WritePipe(Pipe, ErrorCodes);
		}
	}
}

void MoveToRecycleBinHandler()
{
	SHFILEOPSTRUCT Struct;
	string From, To;
	if(ReadPipe(Pipe, Struct) && ReadPipe(Pipe, From) && ReadPipe(Pipe, To))
	{
		Struct.pFrom = From;
		Struct.pTo = To;
		if(WritePipe(Pipe, SHFileOperation(&Struct)))
		{
			WritePipe(Pipe, Struct.fAnyOperationsAborted);
		}
	}
}

void SetOwnerHandler()
{
	string Object, Owner;
	if(ReadPipe(Pipe, Object) && ReadPipe(Pipe, Owner))
	{
		bool Result = SetOwnerInternal(Object, Owner);
		ERRORCODES ErrorCodes;
		if(WritePipe(Pipe, Result))
		{
			WritePipe(Pipe, ErrorCodes);
		}
	}
}

void CreateFileHandler()
{
	string Object;
	DWORD DesiredAccess, ShareMode, CreationDistribution, FlagsAndAttributes;
	// BUGBUG: SecurityAttributes ignored
	// BUGBUG: SecurityAttributes ignored
	if(ReadPipe(Pipe, Object) && ReadPipe(Pipe, DesiredAccess) && ReadPipe(Pipe, ShareMode) && ReadPipe(Pipe, CreationDistribution) && ReadPipe(Pipe, FlagsAndAttributes))
	{
		HANDLE Result = apiCreateFile(Object, DesiredAccess, ShareMode, nullptr, CreationDistribution, FlagsAndAttributes, nullptr);
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
		ERRORCODES ErrorCodes;
		if(WritePipe(Pipe, Result))
		{
			WritePipe(Pipe, ErrorCodes);
		}
	}
}

void SetEncryptionHandler()
{
	string Object;
	bool Encrypt = false;
	if(ReadPipe(Pipe, Object) && ReadPipe(Pipe, Encrypt))
	{
		bool Result = apiSetFileEncryptionInternal(Object, Encrypt);
		ERRORCODES ErrorCodes;
		if(WritePipe(Pipe, Result))
		{
			WritePipe(Pipe, ErrorCodes);
		}
	}
}

void OpenVirtualDiskHandler()
{
	VIRTUAL_STORAGE_TYPE VirtualStorageType;
	string Object;
	VIRTUAL_DISK_ACCESS_MASK VirtualDiskAccessMask;
	OPEN_VIRTUAL_DISK_FLAG Flags;
	OPEN_VIRTUAL_DISK_PARAMETERS Parameters;
	if(ReadPipe(Pipe, VirtualStorageType) && ReadPipe(Pipe, Object) && ReadPipe(Pipe, VirtualDiskAccessMask) && ReadPipe(Pipe, Flags) &&ReadPipe(Pipe, Parameters))
	{
		HANDLE Handle;
		bool Result = apiOpenVirtualDiskInternal(VirtualStorageType, Object, VirtualDiskAccessMask, Flags, Parameters, Handle);
		if(Result)
		{
			HANDLE ParentProcess = OpenProcess(PROCESS_DUP_HANDLE, FALSE, ParentPID);
			if(ParentProcess)
			{
				if(!DuplicateHandle(GetCurrentProcess(), Handle, ParentProcess, &Handle, 0, FALSE, DUPLICATE_CLOSE_SOURCE|DUPLICATE_SAME_ACCESS))
				{
					CloseHandle(Handle);
				}
				CloseHandle(ParentProcess);
			}
		}
		ERRORCODES ErrorCodes;
		if(WritePipe(Pipe, Result) && WritePipe(Pipe, ErrorCodes))
		{
			if(Result)
			{
				WritePipe(Pipe, Handle);
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

	case C_FUNCTION_SETENCRYPTION:
		SetEncryptionHandler();
		break;

	case C_FUNCTION_OPENVIRTUALDISK:
		OpenVirtualDiskHandler();
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
		if(!ifn.GetNamedPipeServerProcessIdPresent() || (ifn.GetNamedPipeServerProcessId(Pipe, &ServerProcessId) && ServerProcessId == PID))
		{
			HANDLE ParentProcess = OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ, FALSE, PID);
			if(ParentProcess)
			{
				string strCurrentProcess, strParentProcess;
				bool TrustedServer = apiGetModuleFileNameEx(GetCurrentProcess(), nullptr, strCurrentProcess) && apiGetModuleFileNameEx(ParentProcess, nullptr, strParentProcess) && (!StrCmpI(strCurrentProcess, strParentProcess));
				CloseHandle(ParentProcess);
				if(TrustedServer)
				{
					if(WritePipe(Pipe, GetCurrentProcessId()))
					{
						ParentPID = PID;
						bool Exit = false;
						int Command = 0;
						while(!Exit)
						{
							if(ReadPipe(Pipe, Command))
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
