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
#include "colormix.hpp"
#include "lasterror.hpp"
#include "privilege.hpp"
#include "fileowner.hpp"
#include "imports.hpp"
#include "TaskBar.hpp"
#include "PluginSynchro.hpp"
#include "scrbuf.hpp"
#include "synchro.hpp"
#include "FarGuid.hpp"
#include "strmix.hpp"

const int CallbackMagic= 0xCA11BAC6;

ENUM(ELEVATION_COMMAND)
{
	C_SERVICE_EXIT,
	C_FUNCTION_CREATEDIRECTORYEX,
	C_FUNCTION_REMOVEDIRECTORY,
	C_FUNCTION_DELETEFILE,
	C_FUNCTION_COPYFILEEX,
	C_FUNCTION_MOVEFILEEX,
	C_FUNCTION_GETFILEATTRIBUTES,
	C_FUNCTION_SETFILEATTRIBUTES,
	C_FUNCTION_CREATEHARDLINK,
	C_FUNCTION_CREATESYMBOLICLINK,
	C_FUNCTION_MOVETORECYCLEBIN,
	C_FUNCTION_SETOWNER,
	C_FUNCTION_CREATEFILE,
	C_FUNCTION_SETENCRYPTION,
	C_FUNCTION_OPENVIRTUALDISK,

	C_COMMANDS_COUNT
};

namespace pipe
{
	bool ReadPipe(HANDLE Pipe, void* Data, size_t DataSize)
	{
		DWORD n;
		return ReadFile(Pipe, Data, static_cast<DWORD>(DataSize), &n, nullptr) && n==DataSize;
	}

	bool WritePipe(HANDLE Pipe, const void* Data, size_t DataSize)
	{
		DWORD n;
		return WriteFile(Pipe, Data, static_cast<DWORD>(DataSize), &n, nullptr) && n==DataSize;
	}

	template<typename T>
	inline bool Read(HANDLE Pipe, T& Data)
	{
		static_assert(!std::is_pointer<T>::value, "ReadPipe template requires a reference to an object");

		bool Result=false;
		size_t DataSize = 0;
		if(ReadPipe(Pipe, &DataSize, sizeof(DataSize)))
		{
			assert(DataSize == sizeof(Data));
			Result = ReadPipe(Pipe, &Data, sizeof(Data));
		}
		return Result;
	}

	template<>
	inline bool Read(HANDLE Pipe, string& Data)
	{
		bool Result=false;
		size_t DataSize = 0;
		if(ReadPipe(Pipe, &DataSize, sizeof(DataSize)))
		{
			if(DataSize)
			{
				LPVOID Ptr=Data.GetBuffer(DataSize/sizeof(wchar_t));
				if(ReadPipe(Pipe, Ptr, DataSize))
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
	inline bool Read(HANDLE Pipe, char_ptr& Data)
	{
		bool Result=false;
		size_t DataSize = 0;
		if(ReadPipe(Pipe, &DataSize, sizeof(DataSize)))
		{
			if(DataSize)
			{
				Data.reset(DataSize);
				if(ReadPipe(Pipe, Data.get(), DataSize))
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

	bool Write(HANDLE Pipe, const void* Data, size_t DataSize)
	{
		bool Result=false;
		if(WritePipe(Pipe, &DataSize, sizeof(DataSize)) && WritePipe(Pipe, Data, DataSize))
		{
			Result=true;
		}
		return Result;
	}

	template<typename T>
	inline bool Write(HANDLE Pipe, const T& Data)
	{
		static_assert(!std::is_pointer<T>::value, "WritePipe template requires a reference to an object");

		return Write(Pipe, &Data, sizeof(Data));
	}
}

DisableElevation::DisableElevation()
{
	Value = Global->Opt->ElevationMode;
	Global->Opt->ElevationMode = 0;
}

DisableElevation::~DisableElevation()
{
	Global->Opt->ElevationMode = Value;
}


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
			Global->ScrBuf->RestoreElevationChar();
		}
	}
}

bool elevation::Write(const void* Data,size_t DataSize) const
{
	return pipe::Write(Pipe, Data, DataSize);
}

template<typename T>
inline bool elevation::Read(T& Data) const
{
	return pipe::Read(Pipe, Data);
}

template<>
inline bool elevation::Read(char_ptr& Data) const
{
	return pipe::Read(Pipe, Data);
}

template<typename T>
inline bool elevation::Write(const T& Data) const
{
	return pipe::Write(Pipe, Data);
}

template<>
inline bool elevation::Write(const string& Data) const
{
	return Write(Data.data(), (Data.size()+1)*sizeof(wchar_t));
}

bool elevation::SendCommand(ELEVATION_COMMAND Command) const
{
	return pipe::Write(Pipe, Command);
}

struct ERRORCODES
{
	DWORD Win32Error;
	NTSTATUS NtError;

	ERRORCODES():Win32Error(GetLastError()), NtError(Global->ifn->RtlGetLastNtStatus()){}
	ERRORCODES(DWORD Win32Error, NTSTATUS NtError):Win32Error(Win32Error), NtError(NtError){}
};

bool elevation::ReceiveLastError() const
{
	ERRORCODES ErrorCodes(ERROR_SUCCESS, STATUS_SUCCESS);
	bool Result = Read(ErrorCodes);
	SetLastError(ErrorCodes.Win32Error);
	Global->ifn->RtlNtStatusToDosError(ErrorCodes.NtError);
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
								Pipe=CreateNamedPipe(strPipe.data(), PIPE_ACCESS_DUPLEX|FILE_FLAG_OVERLAPPED, PIPE_TYPE_BYTE|PIPE_READMODE_BYTE|PIPE_WAIT, 1, 0, 0, 0, &sa);
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
			strParam << L"/elevation " << strPipeID << L" " << GetCurrentProcessId() << L" " << ((Global->Opt->ElevationMode&ELEVATION_USE_PRIVILEGES)? L"1" : L"0");
			SHELLEXECUTEINFO info=
			{
				sizeof(info),
				SEE_MASK_FLAG_NO_UI|SEE_MASK_UNICODE|SEE_MASK_NOASYNC|SEE_MASK_NOCLOSEPROCESS,
				nullptr,
				L"runas",
				Global->g_strFarModuleName.data(),
				strParam.data(),
				Global->g_strFarPath.data(),
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
				AEvent.Open();
				AEvent.Associate(Overlapped);
				ConnectNamedPipe(Pipe, &Overlapped);
				if(AEvent.Wait(15000))
				{
					DWORD NumberOfBytesTransferred;
					if(GetOverlappedResult(Pipe, &Overlapped, &NumberOfBytesTransferred, FALSE))
					{
						if(Read(PID))
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

intptr_t ElevationApproveDlgProc(Dialog* Dlg, intptr_t Msg, intptr_t Param1, void* Param2)
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
	return Dlg->DefProc(Msg, Param1, Param2);
}

struct EAData
{
	std::unique_ptr<Event> pEvent;
	const string& Object;
	LNGID Why;
	bool& AskApprove;
	bool& Approve;
	bool& DontAskAgain;
	EAData(const string& Object, LNGID Why, bool& AskApprove, bool& Approve, bool& DontAskAgain):
		Object(Object), Why(Why), AskApprove(AskApprove), Approve(Approve), DontAskAgain(DontAskAgain){}
};

void ElevationApproveDlgSync(LPVOID Param)
{
	EAData* Data=static_cast<EAData*>(Param);
	enum {DlgX=64,DlgY=12};
	FarDialogItem ElevationApproveDlgData[]=
	{
		{DI_DOUBLEBOX,3,1,DlgX-4,DlgY-2,0,nullptr,nullptr,0,MSG(MAccessDenied)},
		{DI_TEXT,5,2,0,2,0,nullptr,nullptr,0,MSG(Global->IsUserAdmin()?MElevationRequiredPrivileges:MElevationRequired)},
		{DI_TEXT,5,3,0,3,0,nullptr,nullptr,0,MSG(Data->Why)},
		{DI_EDIT,5,4,DlgX-6,4,0,nullptr,nullptr,DIF_READONLY,Data->Object.data()},
		{DI_CHECKBOX,5,6,0,6,1,nullptr,nullptr,0,MSG(MElevationDoForAll)},
		{DI_CHECKBOX,5,7,0,7,0,nullptr,nullptr,0,MSG(MElevationDoNotAskAgainInTheCurrentSession)},
		{DI_TEXT,-1,DlgY-4,0,DlgY-4,0,nullptr,nullptr,DIF_SEPARATOR,L""},
		{DI_BUTTON,0,DlgY-3,0,DlgY-3,0,nullptr,nullptr,DIF_DEFAULTBUTTON|DIF_FOCUS|DIF_SETSHIELD|DIF_CENTERGROUP,MSG(MOk)},
		{DI_BUTTON,0,DlgY-3,0,DlgY-3,0,nullptr,nullptr,DIF_CENTERGROUP,MSG(MSkip)},
	};
	auto ElevationApproveDlg = MakeDialogItemsEx(ElevationApproveDlgData);
	Dialog Dlg(ElevationApproveDlg, ElevationApproveDlgProc);
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
		if(AskApprove &&  Global->IsUserAdmin() && CheckPrivilege(SE_BACKUP_NAME) && CheckPrivilege(SE_RESTORE_NAME))
		{
			Approve = true;
			AskApprove = false;
		}
	}

	if(!(Global->IsUserAdmin() && !(Global->Opt->ElevationMode&ELEVATION_USE_PRIVILEGES)) &&
		AskApprove && !DontAskAgain && !Recurse &&
		FrameManager && !FrameManager->ManagerIsDown())
	{
		Recurse = true;
		GuardLastError error;
		TaskBarPause TBP;
		EAData Data(Object, Why, AskApprove, Approve, DontAskAgain);
		if(!Global->IsMainThread())
		{
			Data.pEvent.reset(new Event());
			Data.pEvent->Open();
			Global->PluginSynchroManager->Synchro(false, FarGuid, &Data);
			Data.pEvent->Wait();
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
		if(Global->IsUserAdmin())
		{
			Privilege BackupPrivilege(SE_BACKUP_NAME), RestorePrivilege(SE_RESTORE_NAME);
			Result = (TemplateObject.empty()?CreateDirectory(Object.data(), Attributes) : CreateDirectoryEx(TemplateObject.data(), Object.data(), Attributes)) != FALSE;
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
		if(Global->IsUserAdmin())
		{
			Privilege BackupPrivilege(SE_BACKUP_NAME), RestorePrivilege(SE_RESTORE_NAME);
			Result = RemoveDirectory(Object.data()) != FALSE;
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
		if(Global->IsUserAdmin())
		{
			Privilege BackupPrivilege(SE_BACKUP_NAME), RestorePrivilege(SE_RESTORE_NAME);
			Result = DeleteFile(Object.data()) != FALSE;
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
		char_ptr Data;
		if(Read(TotalFileSize) && Read(TotalBytesTransferred) && Read(StreamSize) && Read(StreamBytesTransferred) && Read(StreamNumber) && Read(CallbackReason) && Read(Data))
		{
			int Result=ProgressRoutine(TotalFileSize, TotalBytesTransferred, StreamSize, StreamBytesTransferred, StreamNumber, CallbackReason, INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE, Data.get());
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
		if(Global->IsUserAdmin())
		{
			Privilege BackupPrivilege(SE_BACKUP_NAME), RestorePrivilege(SE_RESTORE_NAME);
			Result = CopyFileEx(From.data(), To.data(), ProgressRoutine, Data, Cancel, Flags) != FALSE;
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
		if(Global->IsUserAdmin())
		{
			Privilege BackupPrivilege(SE_BACKUP_NAME), RestorePrivilege(SE_RESTORE_NAME);
			Result = MoveFileEx(From.data(), To.data(), Flags) != FALSE;
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
		if(Global->IsUserAdmin())
		{
			Privilege BackupPrivilege(SE_BACKUP_NAME), RestorePrivilege(SE_RESTORE_NAME);
			Result = GetFileAttributes(Object.data());
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
		if(Global->IsUserAdmin())
		{
			Privilege BackupPrivilege(SE_BACKUP_NAME), RestorePrivilege(SE_RESTORE_NAME);
			Result = SetFileAttributes(Object.data(), FileAttributes) != FALSE;
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
		if(Global->IsUserAdmin())
		{
			Privilege BackupPrivilege(SE_BACKUP_NAME), RestorePrivilege(SE_RESTORE_NAME);
			Result = CreateHardLink(Object.data(), Target.data(), SecurityAttributes) != FALSE;
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
		if(Global->IsUserAdmin())
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
		if(Global->IsUserAdmin())
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
		if(Global->IsUserAdmin())
		{
			Privilege BackupPrivilege(SE_BACKUP_NAME), RestorePrivilege(SE_RESTORE_NAME);
			Result = SetOwnerInternal(Object.data(), Owner.data());
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
		if(Global->IsUserAdmin())
		{
			Privilege BackupPrivilege(SE_BACKUP_NAME), RestorePrivilege(SE_RESTORE_NAME);
			Result = CreateFile(Object.data(), DesiredAccess, ShareMode, SecurityAttributes, CreationDistribution, FlagsAndAttributes, TemplateFile);
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
		if(Global->IsUserAdmin())
		{
			Privilege BackupPrivilege(SE_BACKUP_NAME), RestorePrivilege(SE_RESTORE_NAME);
			Result = apiSetFileEncryptionInternal(Object.data(), Encrypt);
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
		if(Global->IsUserAdmin())
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
	if(Global->Opt->ElevationMode&Mode)
	{
		if(UseNtStatus && Global->ifn->RtlGetLastNtStatusPresent())
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

class elevated
{
private:
	DWORD ParentPID;
	HANDLE Pipe;
	bool Exit;
	typedef std::pair<class elevated*, void*> copy_progress_routine_param;

	bool Write(const void* Data,size_t DataSize) const
	{
		return pipe::Write(Pipe, Data, DataSize);
	}

	template<typename T>
	inline bool Read(T& Data) const
	{
		return pipe::Read(Pipe, Data);
	}

	inline bool Read(char_ptr& Data) const
	{
		return pipe::Read(Pipe, Data);
	}

	template<typename T>
	inline bool Write(const T& Data) const
	{
		return pipe::Write(Pipe, Data);
	}

	inline bool Write(const string& Data) const
	{
		return Write(Data.data(), (Data.size()+1)*sizeof(wchar_t));
	}

	void ExitHandler()
	{
		Exit = true;
	}

	void CreateDirectoryExHandler()
	{
		string TemplateObject;
		if(Read(TemplateObject))
		{
			string Object;
			if(Read(Object))
			{
				// BUGBUG, SecurityAttributes ignored
				bool Result = TemplateObject.empty()? CreateDirectory(Object.data(), nullptr) != FALSE : CreateDirectoryEx(TemplateObject.data(), Object.data(), nullptr) != FALSE;
				ERRORCODES ErrorCodes;
				if(Write(Result))
				{
					Write(ErrorCodes);
				}
			}
		}
	}

	void RemoveDirectoryHandler()
	{
		string Object;
		if(Read(Object))
		{
			bool Result = RemoveDirectory(Object.data()) != FALSE;
			ERRORCODES ErrorCodes;
			if(Write(Result))
			{
				Write(ErrorCodes);
			}
		}
	}

	void DeleteFileHandler()
	{
		string Object;
		if(Read(Object))
		{
			bool Result = DeleteFile(Object.data()) != FALSE;
			ERRORCODES ErrorCodes;
			if(Write(Result))
			{
				Write(ErrorCodes);
			}
		}
	}

	void CopyFileExHandler()
	{
		string From, To;
		char_ptr UserCopyProgressRoutine, Data;
		DWORD Flags = 0;
		// BUGBUG: Cancel ignored
		if(Read(From) && Read(To) && Read(UserCopyProgressRoutine) && Read(Data) && Read(Flags))
		{
			copy_progress_routine_param Param(this, Data.get());
			int Result = CopyFileEx(From.data(), To.data(), UserCopyProgressRoutine.get()? CopyProgressRoutineWrapper : nullptr, &Param, nullptr, Flags);
			ERRORCODES ErrorCodes;
			if(Write(Result))
			{
				Write(ErrorCodes);
			}
		}
	}

	void MoveFileExHandler()
	{
		string From, To;
		DWORD Flags = 0;
		if(Read(From) && Read(To) && Read(Flags))
		{
			bool Result = MoveFileEx(From.data(), To.data(), Flags) != FALSE;
			ERRORCODES ErrorCodes;
			if(Write(Result))
			{
				Write(ErrorCodes);
			}
		}
	}

	void GetFileAttributesHandler()
	{
		string Object;
		if(Read(Object))
		{
			DWORD Result = GetFileAttributes(Object.data());
			ERRORCODES ErrorCodes;
			if(Write(Result))
			{
				Write(ErrorCodes);
			}
		}
	}

	void SetFileAttributesHandler()
	{
		string Object;
		DWORD Attributes = 0;
		if(Read(Object) && Read(Attributes))
		{
			bool Result = SetFileAttributes(Object.data(), Attributes) != FALSE;
			ERRORCODES ErrorCodes;
			if(Write(Result))
			{
				Write(ErrorCodes);
			}
		}
	}

	void CreateHardLinkHandler()
	{
		string Object, Target;
		if(Read(Object) && Read(Target))
		{
			// BUGBUG: SecurityAttributes ignored.
			bool Result = CreateHardLink(Object.data(), Target.data(), nullptr) != FALSE;
			ERRORCODES ErrorCodes;
			if(Write(Result))
			{
				Write(ErrorCodes);
			}
		}
	}

	void CreateSymbolicLinkHandler()
	{
		string Object, Target;
		DWORD Flags = 0;
		if(Read(Object) && Read(Target) && Read(Flags))
		{
			bool Result = CreateSymbolicLinkInternal(Object, Target, Flags);
			ERRORCODES ErrorCodes;
			if(Write(Result))
			{
				Write(ErrorCodes);
			}
		}
	}

	void MoveToRecycleBinHandler()
	{
		SHFILEOPSTRUCT Struct;
		string From, To;
		if(Read(Struct) && Read(From) && Read(To))
		{
			Struct.pFrom = From.data();
			Struct.pTo = To.data();
			if(Write(SHFileOperation(&Struct)))
			{
				Write(Struct.fAnyOperationsAborted);
			}
		}
	}

	void SetOwnerHandler()
	{
		string Object, Owner;
		if(Read(Object) && Read(Owner))
		{
			bool Result = SetOwnerInternal(Object.data(), Owner.data());
			ERRORCODES ErrorCodes;
			if(Write(Result))
			{
				Write(ErrorCodes);
			}
		}
	}

	void CreateFileHandler()
	{
		string Object;
		DWORD DesiredAccess, ShareMode, CreationDistribution, FlagsAndAttributes;
		// BUGBUG: SecurityAttributes, TemplateFile ignored
		if(Read(Object) && Read(DesiredAccess) && Read(ShareMode) && Read(CreationDistribution) && Read(FlagsAndAttributes))
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
			if(Write(reinterpret_cast<intptr_t>(Result)))
			{
				Write(ErrorCodes);
			}
		}
	}

	void SetEncryptionHandler()
	{
		string Object;
		bool Encrypt = false;
		if(Read(Object) && Read(Encrypt))
		{
			bool Result = apiSetFileEncryptionInternal(Object.data(), Encrypt);
			ERRORCODES ErrorCodes;
			if(Write(Result))
			{
				Write(ErrorCodes);
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
		if(Read(VirtualStorageType) && Read(Object) && Read(VirtualDiskAccessMask) && Read(Flags) &&Read(Parameters))
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
			if(Write(Result) && Write(ErrorCodes))
			{
				if(Result)
				{
					Write(reinterpret_cast<intptr_t>(Handle));
				}
			}
		}
	}

	static DWORD WINAPI CopyProgressRoutineWrapper(LARGE_INTEGER TotalFileSize, LARGE_INTEGER TotalBytesTransferred, LARGE_INTEGER StreamSize, LARGE_INTEGER StreamBytesTransferred, DWORD StreamNumber, DWORD CallbackReason, HANDLE SourceFile,HANDLE DestinationFile, LPVOID Data)
	{
		int Result=0;
		auto Param = reinterpret_cast<copy_progress_routine_param*>(Data);
		elevated* Context = Param->first;
		// BUGBUG: SourceFile, DestinationFile ignored
		if (Context->Write(CallbackMagic) && Context->Write(TotalFileSize) && Context->Write(TotalBytesTransferred) && Context->Write(StreamSize) && Context->Write(StreamBytesTransferred) && Context->Write(StreamNumber) && Context->Write(CallbackReason) && Context->Write(&Param->second, sizeof(Param->second)))
		{
			for(;;)
			{
				Context->Read(Result);
				if (Result == CallbackMagic)
				{
					Context->Read(Result);
					break;
				}
				else
				{
					// nested call from ProgressRoutine()
					Context->Process(Result);
				}
			}
		}
		return Result;
	}

	bool Process(int Command)
	{
		assert(Command < C_COMMANDS_COUNT);

		Exit = false;

		static void (elevated::*Handlers[])() =
		{
			&elevated::ExitHandler,
			&elevated::CreateDirectoryExHandler,
			&elevated::RemoveDirectoryHandler,
			&elevated::DeleteFileHandler,
			&elevated::CopyFileExHandler,
			&elevated::MoveFileExHandler,
			&elevated::GetFileAttributesHandler,
			&elevated::SetFileAttributesHandler,
			&elevated::CreateHardLinkHandler,
			&elevated::CreateSymbolicLinkHandler,
			&elevated::MoveToRecycleBinHandler,
			&elevated::SetOwnerHandler,
			&elevated::CreateFileHandler,
			&elevated::SetEncryptionHandler,
			&elevated::OpenVirtualDiskHandler,
		};

		static_assert(ARRAYSIZE(Handlers) == C_COMMANDS_COUNT, "not all commands handled");

		(this->*Handlers[Command])();

		return !Exit;
	}

public:
	int Run(const wchar_t* guid, DWORD PID, bool UsePrivileges)
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
		WaitNamedPipe(strPipe.data(), NMPWAIT_WAIT_FOREVER);
		Pipe = CreateFile(strPipe.data(),GENERIC_READ|GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
		if (Pipe != INVALID_HANDLE_VALUE)
		{
			ULONG ServerProcessId;
			if(!Global->ifn->GetNamedPipeServerProcessIdPresent() || (Global->ifn->GetNamedPipeServerProcessId(Pipe, &ServerProcessId) && ServerProcessId == PID))
			{
				HANDLE ParentProcess = OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ, FALSE, PID);
				if(ParentProcess)
				{
					string strCurrentProcess, strParentProcess;
					bool TrustedServer = apiGetModuleFileNameEx(GetCurrentProcess(), nullptr, strCurrentProcess) && apiGetModuleFileNameEx(ParentProcess, nullptr, strParentProcess) && (!StrCmpI(strCurrentProcess.data(), strParentProcess.data()));
					CloseHandle(ParentProcess);
					if(TrustedServer)
					{
						if(Write(GetCurrentProcessId()))
						{
							ParentPID = PID;
							for (int Command; Read(Command) && Process(Command););
						}
					}
				}
			}
			CloseHandle(Pipe);
		}
		return Result;
	}
};

int ElevationMain(const wchar_t* guid, DWORD PID, bool UsePrivileges)
{
	return elevated().Run(guid, PID, UsePrivileges);
}
