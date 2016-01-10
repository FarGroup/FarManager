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
#include "colors.hpp"
#include "colormix.hpp"
#include "lasterror.hpp"
#include "privilege.hpp"
#include "fileowner.hpp"
#include "imports.hpp"
#include "TaskBar.hpp"
#include "notification.hpp"
#include "scrbuf.hpp"
#include "synchro.hpp"
#include "strmix.hpp"
#include "manager.hpp"
#include "pipe.hpp"
#include "console.hpp"

using namespace os::security;

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
	C_FUNCTION_DETACHVIRTUALDISK,
	C_FUNCTION_GETDISKFREESPACEEX,

	C_COMMANDS_COUNT
};

static const wchar_t ElevationArgument[] = L"/service:elevation";

elevation::elevation():
	m_suppressions(),
	m_pid(),
	IsApproved(false),
	AskApprove(true),
	Elevation(false),
	DontAskAgain(false),
	Recurse(false)
{
}

elevation::~elevation()
{
	if (m_pipe)
	{
		if (m_process)
		{
			SendCommand(C_SERVICE_EXIT);
		}
		DisconnectNamedPipe(m_pipe.native_handle());
	}
}

void elevation::ResetApprove()
{
	if(!DontAskAgain)
	{
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
	return pipe::Write(m_pipe.native_handle(), Data, DataSize);
}

template<typename T>
inline bool elevation::Read(T& Data) const
{
	return pipe::Read(m_pipe.native_handle(), Data);
}

template<typename T>
inline bool elevation::Write(const T& Data) const
{
	return pipe::Write(m_pipe.native_handle(), Data);
}

bool elevation::SendCommand(ELEVATION_COMMAND Command)
{
	return Initialize() && pipe::Write(m_pipe.native_handle(), Command);
}

struct ERRORCODES
{
	struct
	{
		DWORD Win32Error;
		NTSTATUS NtError;
	}
	Codes;

	ERRORCODES()
	{
		Codes.Win32Error = GetLastError();
		Codes.NtError = Imports().RtlGetLastNtStatus();
	}
	
	ERRORCODES(DWORD InitWin32Error, NTSTATUS InitNtError)
	{
		Codes.Win32Error = InitWin32Error;
		Codes.NtError = InitNtError;
	}
};

bool elevation::ReceiveLastError() const
{
	ERRORCODES ErrorCodes(ERROR_SUCCESS, STATUS_SUCCESS);
	const bool Result = Read(ErrorCodes.Codes);
	SetLastError(ErrorCodes.Codes.Win32Error);
	Imports().RtlNtStatusToDosError(ErrorCodes.Codes.NtError);
	return Result;
}

bool elevation::Initialize()
{
	bool Result=false;
	if (!m_pipe)
	{
		GUID Id;
		if(CoCreateGuid(&Id) == S_OK)
		{
			strPipeID = GuidToStr(Id);
			SID_IDENTIFIER_AUTHORITY NtAuthority=SECURITY_NT_AUTHORITY;

			if (const auto pSD = os::memory::local::alloc<PSECURITY_DESCRIPTOR>(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH))
			{
				if (InitializeSecurityDescriptor(pSD.get(), SECURITY_DESCRIPTOR_REVISION))
				{
					os::sid_object AdminSID(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS);

					EXPLICIT_ACCESS ea = {};
					ea.grfAccessPermissions = GENERIC_READ|GENERIC_WRITE;
					ea.grfAccessMode = SET_ACCESS;
					ea.grfInheritance= NO_INHERITANCE;
					ea.Trustee.TrusteeForm = TRUSTEE_IS_SID;
					ea.Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
					ea.Trustee.ptstrName = static_cast<LPWSTR>(AdminSID.get());
					PACL pRawACL = nullptr;
					if (SetEntriesInAcl(1, &ea, nullptr, &pRawACL) == ERROR_SUCCESS)
					{
						const auto pACL = os::memory::local::ptr(pRawACL);

						if(SetSecurityDescriptorDacl(pSD.get(), TRUE, pACL.get(), FALSE))
						{
							SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), pSD.get(), FALSE };
							const auto strPipe = L"\\\\.\\pipe\\" + strPipeID;
							m_pipe.reset(CreateNamedPipe(strPipe.data(), PIPE_ACCESS_DUPLEX|FILE_FLAG_OVERLAPPED, PIPE_TYPE_BYTE|PIPE_READMODE_BYTE|PIPE_WAIT, 1, 0, 0, 0, &sa));
						}
					}
				}
			}
		}
	}
	if (m_pipe)
	{
		if(m_process)
		{
			if (!m_process.signaled())
			{
				Result = true;
			}
			else
			{
				m_process.close();
			}
		}
		if(!Result)
		{
			SCOPED_ACTION(IndeterminateTaskBar);
			DisconnectNamedPipe(m_pipe.native_handle());

			BOOL InJob = FALSE;
			if (!m_job)
			{
				// IsProcessInJob not exist in win2k. use QueryInformationJobObject(nullptr, ...) instead.
				// IsProcessInJob(GetCurrentProcess(), nullptr, &InJob);

				JOBOBJECT_EXTENDED_LIMIT_INFORMATION jeli={};
				InJob = QueryInformationJobObject(nullptr, JobObjectExtendedLimitInformation, &jeli, sizeof(jeli), nullptr);
				if (!InJob)
				{
					m_job.reset(CreateJobObject(nullptr, nullptr));
					if (m_job)
					{
						jeli.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_SILENT_BREAKAWAY_OK;
						if (SetInformationJobObject(m_job.native_handle(), JobObjectExtendedLimitInformation, &jeli, sizeof(jeli)))
						{
							AssignProcessToJobObject(m_job.native_handle(), GetCurrentProcess());
						}
					}
				}
			}

			string Param = string(ElevationArgument) + L" " + strPipeID + L' ' + std::to_wstring(GetCurrentProcessId()) + L' ' + ((Global->Opt->ElevationMode&ELEVATION_USE_PRIVILEGES) ? L'1' : L'0');

			SHELLEXECUTEINFO info =
			{
				sizeof(info),
				SEE_MASK_FLAG_NO_UI|SEE_MASK_UNICODE|SEE_MASK_NOASYNC|SEE_MASK_NOCLOSEPROCESS,
				nullptr,
				L"runas",
				Global->g_strFarModuleName.data(),
				Param.data(),
				Global->g_strFarPath.data(),
			};
			if (ShellExecuteEx(&info) && info.hProcess)
			{
				m_process.reset(info.hProcess);
				if (!InJob && m_job)
				{
					AssignProcessToJobObject(m_job.native_handle(), m_process.native_handle());
				}
				Event AEvent(Event::automatic, Event::nonsignaled);
				OVERLAPPED Overlapped;
				AEvent.Associate(Overlapped);
				ConnectNamedPipe(m_pipe.native_handle(), &Overlapped);
				MultiWaiter Waiter;
				Waiter.Add(AEvent);
				Waiter.Add(m_process.native_handle());
				switch (Waiter.Wait(MultiWaiter::wait_any, 15000))
				{
				case WAIT_OBJECT_0:
					{
						DWORD NumberOfBytesTransferred;
						if (GetOverlappedResult(m_pipe.native_handle(), &Overlapped, &NumberOfBytesTransferred, FALSE))
						{
							if (Read(m_pid))
							{
								Result = true;
							}
						}
					}
					break;

				case WAIT_OBJECT_0 + 1:
					{
						DWORD ExitCode;
						SetLastError(GetExitCodeProcess(m_process.native_handle(), &ExitCode)? ExitCode : ERROR_GEN_FAILURE);
					}
					break;

				default:
					break;
				}

				if(!Result)
				{
					if (!m_process.signaled())
					{
						TerminateProcess(m_process.native_handle(), 0);
						m_process.close();
						m_job.close();
						SetLastError(ERROR_PROCESS_ABORTED);
					}
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
				FarColor Color=colors::PaletteColorToFarColor(COL_DIALOGTEXT);
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

struct EAData: noncopyable
{
	const string& Object;
	LNGID Why;
	bool& AskApprove;
	bool& IsApproved;
	bool& DontAskAgain;
	EAData(const string& Object, LNGID Why, bool& AskApprove, bool& IsApproved, bool& DontAskAgain):
		Object(Object), Why(Why), AskApprove(AskApprove), IsApproved(IsApproved), DontAskAgain(DontAskAgain){}
};

void ElevationApproveDlgSync(const EAData& Data)
{
	SCOPED_ACTION(message_manager::suppress);

	enum {DlgX=64,DlgY=12};
	FarDialogItem ElevationApproveDlgData[]=
	{
		{DI_DOUBLEBOX,3,1,DlgX-4,DlgY-2,0,nullptr,nullptr,0,MSG(MAccessDenied)},
		{DI_TEXT,5,2,0,2,0,nullptr,nullptr,0,MSG(is_admin()?MElevationRequiredPrivileges:MElevationRequired)},
		{DI_TEXT,5,3,0,3,0,nullptr,nullptr,0,MSG(Data.Why)},
		{DI_EDIT,5,4,DlgX-6,4,0,nullptr,nullptr,DIF_READONLY,Data.Object.data()},
		{DI_CHECKBOX,5,6,0,6,1,nullptr,nullptr,0,MSG(MElevationDoForAll)},
		{DI_CHECKBOX,5,7,0,7,0,nullptr,nullptr,0,MSG(MElevationDoNotAskAgainInTheCurrentSession)},
		{DI_TEXT,-1,DlgY-4,0,DlgY-4,0,nullptr,nullptr,DIF_SEPARATOR,L""},
		{DI_BUTTON,0,DlgY-3,0,DlgY-3,0,nullptr,nullptr,DIF_DEFAULTBUTTON|DIF_FOCUS|DIF_SETSHIELD|DIF_CENTERGROUP,MSG(MOk)},
		{DI_BUTTON,0,DlgY-3,0,DlgY-3,0,nullptr,nullptr,DIF_CENTERGROUP,MSG(MSkip)},
	};
	auto ElevationApproveDlg = MakeDialogItemsEx(ElevationApproveDlgData);
	const auto Dlg = Dialog::create(ElevationApproveDlg, ElevationApproveDlgProc);
	Dlg->SetHelp(L"ElevationDlg");
	Dlg->SetPosition(-1, -1, DlgX, DlgY);
	Dlg->SetDialogMode(DMODE_FULLSHADOW | DMODE_NOPLUGINS);
	const auto Current = Global->WindowManager->GetCurrentWindow();
	if(Current)
	{
		Current->Lock();
	}
	const auto Lock = Global->ScrBuf->GetLockCount();
	Global->ScrBuf->SetLockCount(0);
	Dlg->Process();
	Global->ScrBuf->SetLockCount(Lock);
	if(Current)
	{
		Current->Unlock();
	}

	Data.AskApprove=!ElevationApproveDlg[AAD_CHECKBOX_DOFORALL].Selected;
	Data.IsApproved = Dlg->GetExitCode() == AAD_BUTTON_OK;
	Data.DontAskAgain=ElevationApproveDlg[AAD_CHECKBOX_DONTASKAGAIN].Selected!=FALSE;
}

bool elevation::ElevationApproveDlg(LNGID Why, const string& Object)
{
	if (m_suppressions)
		return false;

	Console().FlushInputBuffer();

	// request for backup&restore privilege is useless if the user already has them
	{
		SCOPED_ACTION(GuardLastError);
		if (AskApprove && is_admin() && privilege::is_set(SE_BACKUP_NAME) && privilege::is_set(SE_RESTORE_NAME))
		{
			AskApprove = false;
			return true;
		}
	}

	if(!(is_admin() && !(Global->Opt->ElevationMode&ELEVATION_USE_PRIVILEGES)) &&
		AskApprove && !DontAskAgain && !Recurse &&
 		Global->WindowManager && !Global->WindowManager->ManagerIsDown())
	{
		Recurse = true;
		SCOPED_ACTION(GuardLastError);
		SCOPED_ACTION(TaskBarPause);
		EAData Data(Object, Why, AskApprove, IsApproved, DontAskAgain);
		if(!Global->IsMainThread())
		{
			Event SyncEvent(Event::automatic, Event::nonsignaled);
			SCOPED_ACTION(listener_ex)(elevation_dialog, [&SyncEvent](const any& Payload)
			{
				ElevationApproveDlgSync(*any_cast<EAData*>(Payload));
				SyncEvent.Set();
			});
			MessageManager().notify(elevation_dialog, any(&Data));
			SyncEvent.Wait();
		}
		else
		{
			ElevationApproveDlgSync(Data);
		}
		Recurse = false;
	}
	return IsApproved;
}

bool elevation::fCreateDirectoryEx(const string& TemplateObject, const string& Object, LPSECURITY_ATTRIBUTES Attributes)
{
	SCOPED_ACTION(CriticalSectionLock)(CS);
	bool Result=false;
	if(ElevationApproveDlg(MElevationRequiredCreate, Object))
	{
		if(is_admin())
		{
			SCOPED_ACTION(privilege)(make_vector<const wchar_t*>(SE_BACKUP_NAME, SE_RESTORE_NAME));
			Result = (TemplateObject.empty()?CreateDirectory(Object.data(), Attributes) : CreateDirectoryEx(TemplateObject.data(), Object.data(), Attributes)) != FALSE;
		}
		else if(SendCommand(C_FUNCTION_CREATEDIRECTORYEX) && Write(TemplateObject) && Write(Object))
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
	SCOPED_ACTION(CriticalSectionLock)(CS);
	bool Result=false;
	if(ElevationApproveDlg(MElevationRequiredDelete, Object))
	{
		if(is_admin())
		{
			SCOPED_ACTION(privilege)(make_vector<const wchar_t*>(SE_BACKUP_NAME, SE_RESTORE_NAME));
			Result = RemoveDirectory(Object.data()) != FALSE;
		}
		else if(SendCommand(C_FUNCTION_REMOVEDIRECTORY) && Write(Object))
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
	SCOPED_ACTION(CriticalSectionLock)(CS);
	bool Result=false;
	if(ElevationApproveDlg(MElevationRequiredDelete, Object))
	{
		if(is_admin())
		{
			SCOPED_ACTION(privilege)(make_vector<const wchar_t*>(SE_BACKUP_NAME, SE_RESTORE_NAME));
			Result = DeleteFile(Object.data()) != FALSE;
		}
		else if(SendCommand(C_FUNCTION_DELETEFILE) && Write(Object))
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
		intptr_t Data;
		if(Read(TotalFileSize) && Read(TotalBytesTransferred) && Read(StreamSize) && Read(StreamBytesTransferred) && Read(StreamNumber) && Read(CallbackReason) && Read(Data))
		{
			int Result=ProgressRoutine(TotalFileSize, TotalBytesTransferred, StreamSize, StreamBytesTransferred, StreamNumber, CallbackReason, INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE, reinterpret_cast<void*>(Data));
			if(Write(CallbackMagic))
			{
				Write(Result);
			}
		}
	}
}

bool elevation::fCopyFileEx(const string& From, const string& To, LPPROGRESS_ROUTINE ProgressRoutine, LPVOID Data, LPBOOL Cancel, DWORD Flags)
{
	SCOPED_ACTION(CriticalSectionLock)(CS);
	bool Result = false;
	if(ElevationApproveDlg(MElevationRequiredCopy, From))
	{
		if(is_admin())
		{
			SCOPED_ACTION(privilege)(make_vector<const wchar_t*>(SE_BACKUP_NAME, SE_RESTORE_NAME));
			Result = CopyFileEx(From.data(), To.data(), ProgressRoutine, Data, Cancel, Flags) != FALSE;
		}
		// BUGBUG: Cancel ignored
		else if(SendCommand(C_FUNCTION_COPYFILEEX) && Write(From) && Write(To) && Write(&ProgressRoutine, sizeof(ProgressRoutine)) && Write(reinterpret_cast<intptr_t>(Data)) && Write(Flags))
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
	SCOPED_ACTION(CriticalSectionLock)(CS);
	bool Result=false;
	if(ElevationApproveDlg(MElevationRequiredMove, From))
	{
		if(is_admin())
		{
			SCOPED_ACTION(privilege)(make_vector<const wchar_t*>(SE_BACKUP_NAME, SE_RESTORE_NAME));
			Result = MoveFileEx(From.data(), To.data(), Flags) != FALSE;
		}
		else if(SendCommand(C_FUNCTION_MOVEFILEEX) && Write(From) && Write(To) && Write(Flags))
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
	SCOPED_ACTION(CriticalSectionLock)(CS);
	DWORD Result = INVALID_FILE_ATTRIBUTES;
	if(ElevationApproveDlg(MElevationRequiredGetAttributes, Object))
	{
		if(is_admin())
		{
			SCOPED_ACTION(privilege)(make_vector<const wchar_t*>(SE_BACKUP_NAME, SE_RESTORE_NAME));
			Result = GetFileAttributes(Object.data());
		}
		else if(SendCommand(C_FUNCTION_GETFILEATTRIBUTES) && Write(Object))
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
	SCOPED_ACTION(CriticalSectionLock)(CS);
	bool Result=false;
	if(ElevationApproveDlg(MElevationRequiredSetAttributes, Object))
	{
		if(is_admin())
		{
			SCOPED_ACTION(privilege)(make_vector<const wchar_t*>(SE_BACKUP_NAME, SE_RESTORE_NAME));
			Result = SetFileAttributes(Object.data(), FileAttributes) != FALSE;
		}
		else if(SendCommand(C_FUNCTION_SETFILEATTRIBUTES) && Write(Object) && Write(FileAttributes))
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
	SCOPED_ACTION(CriticalSectionLock)(CS);
	bool Result=false;
	if(ElevationApproveDlg(MElevationRequiredHardLink, Object))
	{
		if(is_admin())
		{
			SCOPED_ACTION(privilege)(make_vector<const wchar_t*>(SE_BACKUP_NAME, SE_RESTORE_NAME));
			Result = CreateHardLink(Object.data(), Target.data(), SecurityAttributes) != FALSE;
		}
		// BUGBUG: SecurityAttributes ignored.
		else if(SendCommand(C_FUNCTION_CREATEHARDLINK) && Write(Object) && Write(Target))
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
	SCOPED_ACTION(CriticalSectionLock)(CS);
	bool Result=false;
	if(ElevationApproveDlg(MElevationRequiredSymLink, Object))
	{
		if(is_admin())
		{
			SCOPED_ACTION(privilege)(make_vector<const wchar_t*>(SE_BACKUP_NAME, SE_RESTORE_NAME));
			Result = os::CreateSymbolicLinkInternal(Object, Target, Flags);
		}
		else if(SendCommand(C_FUNCTION_CREATESYMBOLICLINK) && Write(Object) && Write(Target) && Write(Flags))
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
	SCOPED_ACTION(CriticalSectionLock)(CS);
	int Result = 0x78; //DE_ACCESSDENIEDSRC
	if(ElevationApproveDlg(MElevationRequiredRecycle, FileOpStruct.pFrom))
	{
		if(is_admin())
		{
			SCOPED_ACTION(privilege)(make_vector<const wchar_t*>(SE_BACKUP_NAME, SE_RESTORE_NAME));
			Result = SHFileOperation(&FileOpStruct);
		}
		else
		{
			if(SendCommand(C_FUNCTION_MOVETORECYCLEBIN) && Write(FileOpStruct)
			   && Write(FileOpStruct.pFrom, FileOpStruct.pFrom? (wcslen(FileOpStruct.pFrom) + 1 + 1) * sizeof(wchar_t) : 0) // achtung! +1
			   && Write(FileOpStruct.pTo, FileOpStruct.pTo? (wcslen(FileOpStruct.pTo) + 1 + 1) * sizeof(wchar_t) : 0)) // achtung! +1
			{
				int OpResult = 0;
				if(Read(OpResult) && Read(FileOpStruct.fAnyOperationsAborted))
				{
					// achtung! no "last error" here
					Result = OpResult;
				}
			}
		}
	}
	return Result;
}

bool elevation::fSetOwner(const string& Object, const string& Owner)
{
	SCOPED_ACTION(CriticalSectionLock)(CS);
	bool Result=false;
	if(ElevationApproveDlg(MElevationRequiredSetOwner, Object))
	{
		if(is_admin())
		{
			SCOPED_ACTION(privilege)(make_vector<const wchar_t*>(SE_BACKUP_NAME, SE_RESTORE_NAME));
			Result = SetOwnerInternal(Object, Owner);
		}
		else if(SendCommand(C_FUNCTION_SETOWNER) && Write(Object) && Write(Owner))
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
	SCOPED_ACTION(CriticalSectionLock)(CS);
	HANDLE Result=INVALID_HANDLE_VALUE;
	if(ElevationApproveDlg(MElevationRequiredOpen, Object))
	{
		if(is_admin())
		{
			SCOPED_ACTION(privilege)(make_vector<const wchar_t*>(SE_BACKUP_NAME, SE_RESTORE_NAME));
			Result = CreateFile(Object.data(), DesiredAccess, ShareMode, SecurityAttributes, CreationDistribution, FlagsAndAttributes, TemplateFile);
		}
		// BUGBUG: SecurityAttributes ignored
		// BUGBUG: TemplateFile ignored
		else if(SendCommand(C_FUNCTION_CREATEFILE) && Write(Object) && Write(DesiredAccess) && Write(ShareMode) && Write(CreationDistribution) && Write(FlagsAndAttributes))
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
	SCOPED_ACTION(CriticalSectionLock)(CS);
	bool Result=false;
	if(ElevationApproveDlg(Encrypt? MElevationRequiredEncryptFile : MElevationRequiredDecryptFile, Object))
	{
		if(is_admin())
		{
			SCOPED_ACTION(privilege)(make_vector<const wchar_t*>(SE_BACKUP_NAME, SE_RESTORE_NAME));
			Result = os::SetFileEncryptionInternal(Object.data(), Encrypt);
		}
		else if(SendCommand(C_FUNCTION_SETENCRYPTION) && Write(Object) && Write(Encrypt))
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

bool elevation::fDetachVirtualDisk(const string& Object, VIRTUAL_STORAGE_TYPE& VirtualStorageType)
{
	SCOPED_ACTION(CriticalSectionLock)(CS);
	bool Result=false;
	if(ElevationApproveDlg(MElevationRequiredCreate, Object))
	{
		if(is_admin())
		{
			SCOPED_ACTION(privilege)(make_vector<const wchar_t*>(SE_BACKUP_NAME, SE_RESTORE_NAME));
			Result = os::DetachVirtualDiskInternal(Object, VirtualStorageType);
		}
		else if (SendCommand(C_FUNCTION_DETACHVIRTUALDISK) && Write(Object) && Write(VirtualStorageType))
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

bool elevation::fGetDiskFreeSpaceEx(const string& Object, ULARGE_INTEGER* FreeBytesAvailableToCaller, ULARGE_INTEGER* TotalNumberOfBytes, ULARGE_INTEGER* TotalNumberOfFreeBytes)
{
	SCOPED_ACTION(CriticalSectionLock)(CS);
	bool Result=false;
	if(ElevationApproveDlg(MElevationRequiredList, Object))
	{
		if(is_admin())
		{
			SCOPED_ACTION(privilege)(make_vector<const wchar_t*>(SE_BACKUP_NAME, SE_RESTORE_NAME));
			Result = GetDiskFreeSpaceEx(Object.data(), FreeBytesAvailableToCaller, TotalNumberOfBytes, TotalNumberOfFreeBytes) != FALSE;
		}
		else if(SendCommand(C_FUNCTION_GETDISKFREESPACEEX) && Write(Object))
		{
			bool OpResult = false;
			if(Read(OpResult) && ReceiveLastError())
			{
				Result = OpResult;
			}
			if(Result)
			{
				ULARGE_INTEGER Buffer;
				Read(Buffer);
				if (FreeBytesAvailableToCaller)
					*FreeBytesAvailableToCaller = Buffer;
				Read(Buffer);
				if (TotalNumberOfBytes)
					*TotalNumberOfBytes = Buffer;
				Read(Buffer);
				if (TotalNumberOfFreeBytes)
					*TotalNumberOfFreeBytes = Buffer;
			}
		}
	}
	return Result;
}


bool ElevationRequired(ELEVATION_MODE Mode, bool UseNtStatus)
{
	bool Result = false;
	if(Global && Global->Opt && Global->Opt->ElevationMode & Mode)
	{
		if(UseNtStatus && Imports().RtlGetLastNtStatus)
		{
				const auto LastNtStatus = os::GetLastNtStatus();
				Result = LastNtStatus == STATUS_ACCESS_DENIED || LastNtStatus == STATUS_PRIVILEGE_NOT_HELD;
		}
		else
		{
			// RtlGetLastNtStatus not implemented in w2k.
			const auto LastWin32Error = GetLastError();
			Result = LastWin32Error == ERROR_ACCESS_DENIED || LastWin32Error == ERROR_PRIVILEGE_NOT_HELD;
		}
	}
	return Result;
}

class elevated:noncopyable
{
public:
	elevated():
		ParentPID(0),
		Exit(false)
	{}

	int Run(const wchar_t* guid, DWORD PID, bool UsePrivileges)
	{
		int Result = ERROR_SUCCESS;

		SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOOPENFILEERRORBOX);

		auto Privileges = make_vector<const wchar_t*>(SE_TAKE_OWNERSHIP_NAME, SE_DEBUG_NAME, SE_CREATE_SYMBOLIC_LINK_NAME);
		if (UsePrivileges)
		{
			Privileges.emplace_back(SE_BACKUP_NAME);
			Privileges.emplace_back(SE_RESTORE_NAME);
		}

		SCOPED_ACTION(privilege)(Privileges);

		const auto strPipe = string(L"\\\\.\\pipe\\") + guid;
		WaitNamedPipe(strPipe.data(), NMPWAIT_WAIT_FOREVER);
		m_Pipe.reset(CreateFile(strPipe.data(),GENERIC_READ|GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr));
		if (m_Pipe)
		{
			ULONG ServerProcessId;
			if(!Imports().GetNamedPipeServerProcessId || (Imports().GetNamedPipeServerProcessId(m_Pipe.native_handle(), &ServerProcessId) && ServerProcessId == PID))
			{
				if (auto ParentProcess = os::handle(OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, PID)))
				{
					string strCurrentProcess, strParentProcess;
					bool TrustedServer = os::GetModuleFileNameEx(GetCurrentProcess(), nullptr, strCurrentProcess) && os::GetModuleFileNameEx(ParentProcess.native_handle(), nullptr, strParentProcess) && (!StrCmpI(strCurrentProcess, strParentProcess));
					ParentProcess.close();
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
			m_Pipe.close();
		}
		else
		{
			Result = GetLastError();
		}
		return Result;
	}

private:
	os::handle m_Pipe;
	DWORD ParentPID;
	mutable bool Exit;
	typedef std::pair<const class elevated*, void*> copy_progress_routine_param;

	bool Write(const void* Data,size_t DataSize) const
	{
		return pipe::Write(m_Pipe.native_handle(), Data, DataSize);
	}

	template<typename T>
	inline bool Read(T& Data) const
	{
		return pipe::Read(m_Pipe.native_handle(), Data);
	}

	template<typename T>
	inline bool Write(const T& Data) const
	{
		return pipe::Write(m_Pipe.native_handle(), Data);
	}

	void ExitHandler() const
	{
		Exit = true;
	}

	void CreateDirectoryExHandler() const
	{
		string TemplateObject;
		if(Read(TemplateObject))
		{
			string Object;
			if(Read(Object))
			{
				// BUGBUG, SecurityAttributes ignored
				const bool Result = TemplateObject.empty() ? CreateDirectory(Object.data(), nullptr) != FALSE : CreateDirectoryEx(TemplateObject.data(), Object.data(), nullptr) != FALSE;
				const ERRORCODES ErrorCodes;
				if(Write(Result))
				{
					Write(ErrorCodes.Codes);
				}
			}
		}
	}

	void RemoveDirectoryHandler() const
	{
		string Object;
		if(Read(Object))
		{
			const bool Result = RemoveDirectory(Object.data()) != FALSE;
			const ERRORCODES ErrorCodes;
			if(Write(Result))
			{
				Write(ErrorCodes.Codes);
			}
		}
	}

	void DeleteFileHandler() const
	{
		string Object;
		if(Read(Object))
		{
			const bool Result = DeleteFile(Object.data()) != FALSE;
			const ERRORCODES ErrorCodes;
			if(Write(Result))
			{
				Write(ErrorCodes.Codes);
			}
		}
	}

	void CopyFileExHandler() const
	{
		string From, To;
		intptr_t UserCopyProgressRoutine, Data;
		DWORD Flags = 0;
		// BUGBUG: Cancel ignored
		if(Read(From) && Read(To) && Read(UserCopyProgressRoutine) && Read(Data) && Read(Flags))
		{
			copy_progress_routine_param Param(this, reinterpret_cast<void*>(Data));
			int Result = CopyFileEx(From.data(), To.data(), UserCopyProgressRoutine? CopyProgressRoutineWrapper : nullptr, &Param, nullptr, Flags);
			const ERRORCODES ErrorCodes;
			if(Write(Result))
			{
				Write(ErrorCodes.Codes);
			}
		}
	}

	void MoveFileExHandler() const
	{
		string From, To;
		DWORD Flags = 0;
		if(Read(From) && Read(To) && Read(Flags))
		{
			const bool Result = MoveFileEx(From.data(), To.data(), Flags) != FALSE;
			const ERRORCODES ErrorCodes;
			if(Write(Result))
			{
				Write(ErrorCodes.Codes);
			}
		}
	}

	void GetFileAttributesHandler() const
	{
		string Object;
		if(Read(Object))
		{
			const DWORD Result = GetFileAttributes(Object.data());
			const ERRORCODES ErrorCodes;
			if(Write(Result))
			{
				Write(ErrorCodes.Codes);
			}
		}
	}

	void SetFileAttributesHandler() const
	{
		string Object;
		DWORD Attributes = 0;
		if(Read(Object) && Read(Attributes))
		{
			const bool Result = SetFileAttributes(Object.data(), Attributes) != FALSE;
			const ERRORCODES ErrorCodes;
			if(Write(Result))
			{
				Write(ErrorCodes.Codes);
			}
		}
	}

	void CreateHardLinkHandler() const
	{
		string Object, Target;
		if(Read(Object) && Read(Target))
		{
			// BUGBUG: SecurityAttributes ignored.
			const bool Result = CreateHardLink(Object.data(), Target.data(), nullptr) != FALSE;
			const ERRORCODES ErrorCodes;
			if(Write(Result))
			{
				Write(ErrorCodes.Codes);
			}
		}
	}

	void CreateSymbolicLinkHandler() const
	{
		string Object, Target;
		DWORD Flags = 0;
		if(Read(Object) && Read(Target) && Read(Flags))
		{
			const bool Result = os::CreateSymbolicLinkInternal(Object, Target, Flags);
			const ERRORCODES ErrorCodes;
			if(Write(Result))
			{
				Write(ErrorCodes.Codes);
			}
		}
	}

	void MoveToRecycleBinHandler() const
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

	void SetOwnerHandler() const
	{
		string Object, Owner;
		if(Read(Object) && Read(Owner))
		{
			const bool Result = SetOwnerInternal(Object, Owner);
			const ERRORCODES ErrorCodes;
			if(Write(Result))
			{
				Write(ErrorCodes.Codes);
			}
		}
	}

	void CreateFileHandler() const
	{
		string Object;
		DWORD DesiredAccess, ShareMode, CreationDistribution, FlagsAndAttributes;
		// BUGBUG: SecurityAttributes, TemplateFile ignored
		if(Read(Object) && Read(DesiredAccess) && Read(ShareMode) && Read(CreationDistribution) && Read(FlagsAndAttributes))
		{
			HANDLE Duplicate = INVALID_HANDLE_VALUE;
			if (const os::handle Handle = os::CreateFile(Object, DesiredAccess, ShareMode, nullptr, CreationDistribution, FlagsAndAttributes, nullptr))
			{
				if (const auto ParentProcess = os::handle(OpenProcess(PROCESS_DUP_HANDLE, FALSE, ParentPID)))
				{
					DuplicateHandle(GetCurrentProcess(), Handle.native_handle(), ParentProcess.native_handle(), &Duplicate, 0, FALSE, DUPLICATE_SAME_ACCESS);
				}
			}
			const ERRORCODES ErrorCodes;
			if (Write(reinterpret_cast<intptr_t>(Duplicate)))
			{
				Write(ErrorCodes.Codes);
			}
		}
	}

	void SetEncryptionHandler() const
	{
		string Object;
		bool Encrypt = false;
		if(Read(Object) && Read(Encrypt))
		{
			const bool Result = os::SetFileEncryptionInternal(Object.data(), Encrypt);
			const ERRORCODES ErrorCodes;
			if(Write(Result))
			{
				Write(ErrorCodes.Codes);
			}
		}
	}

	void DetachVirtualDiskHandler() const
	{
		string Object;
		VIRTUAL_STORAGE_TYPE VirtualStorateType;
		if (Read(Object) && Read(VirtualStorateType))
		{
			const bool Result = os::DetachVirtualDiskInternal(Object, VirtualStorateType);
			const ERRORCODES ErrorCodes;
			if (Write(Result))
			{
				Write(ErrorCodes.Codes);
			}
		}
	}

	void GetDiskFreeSpaceExHandler() const
	{
		ULARGE_INTEGER FreeBytesAvailableToCaller, TotalNumberOfBytes, TotalNumberOfFreeBytes;
		string Object;
		if(Read(Object))
		{
			const bool Result = GetDiskFreeSpaceEx(Object.data(), &FreeBytesAvailableToCaller, &TotalNumberOfBytes, &TotalNumberOfFreeBytes) != FALSE;
			const ERRORCODES ErrorCodes;
			if (Write(Result) && Write(ErrorCodes.Codes))
			{
				if(Result)
				{
					Write(FreeBytesAvailableToCaller);
					Write(TotalNumberOfBytes);
					Write(TotalNumberOfFreeBytes);
				}
			}
		}
	}

	static DWORD WINAPI CopyProgressRoutineWrapper(LARGE_INTEGER TotalFileSize, LARGE_INTEGER TotalBytesTransferred, LARGE_INTEGER StreamSize, LARGE_INTEGER StreamBytesTransferred, DWORD StreamNumber, DWORD CallbackReason, HANDLE SourceFile,HANDLE DestinationFile, LPVOID Data)
	{
		int Result=0;
		const auto Param = reinterpret_cast<const copy_progress_routine_param*>(Data);
		const auto Context = Param->first;
		// BUGBUG: SourceFile, DestinationFile ignored
		if (Context->Write(CallbackMagic) && Context->Write(TotalFileSize) && Context->Write(TotalBytesTransferred) && Context->Write(StreamSize) && Context->Write(StreamBytesTransferred) && Context->Write(StreamNumber) && Context->Write(CallbackReason) && Context->Write(reinterpret_cast<intptr_t>(Param->second)))
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

	bool Process(int Command) const
	{
		assert(Command < C_COMMANDS_COUNT);

		Exit = false;

		static const decltype(&elevated::ExitHandler) Handlers[] =
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
			&elevated::DetachVirtualDiskHandler,
			&elevated::GetDiskFreeSpaceExHandler,
		};

		static_assert(ARRAYSIZE(Handlers) == C_COMMANDS_COUNT, "not all commands handled");

		(this->*Handlers[Command])();

		return !Exit;
	}
};

int ElevationMain(const wchar_t* guid, DWORD PID, bool UsePrivileges)
{
	return elevated().Run(guid, PID, UsePrivileges);
}

bool IsElevationArgument(const wchar_t* Argument)
{
	return !StrCmp(Argument, ElevationArgument);
}