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
#include "farcolor.hpp"
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
#include "constitle.hpp"

using namespace os::security;

static constexpr int CallbackMagic= 0xCA11BAC6;

enum ELEVATION_COMMAND: int
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

static constexpr wchar_t ElevationArgument[] = L"/service:elevation";

static auto CreateBackupRestorePrivilege() { return privilege(SE_BACKUP_NAME, SE_RESTORE_NAME); }

elevation::elevation():
	m_Suppressions(),
	m_ProcessId(),
	m_IsApproved(false),
	m_AskApprove(true),
	m_Elevation(false),
	m_DontAskAgain(false),
	m_Recurse(0)
{
}

elevation::~elevation()
{
	if (!m_Pipe)
		return;

	if (m_Process)
	{
		try
		{
			SendCommand(C_SERVICE_EXIT);
		}
		catch (const far_exception&)
		{
			// TODO: log
		}
	}

	DisconnectNamedPipe(m_Pipe.native_handle());
}

void elevation::ResetApprove()
{
	if (m_DontAskAgain)
		return;

	m_AskApprove=true;

	if (!m_Elevation)
		return;

	m_Elevation=false;
	Global->ScrBuf->RestoreElevationChar();
}

void elevation::Write(const blob_view& Data) const
{
	if (!pipe::Write(m_Pipe, Data.data(), Data.size()))
		throw MAKE_FAR_EXCEPTION(L"Pipe write error");
}

template<typename T>
T elevation::Read() const
{
	T Data;
	if (!pipe::Read(m_Pipe, Data))
		throw MAKE_FAR_EXCEPTION(L"Pipe read error");
	return Data;
}

template<typename T, typename... args>
void elevation::Write(const T& Data, args&&... Args) const
{
	if (!pipe::Write(m_Pipe, Data))
		throw MAKE_FAR_EXCEPTION(L"Pipe write error");
	Write(std::forward<args>(Args)...);
}

template<typename... args>
void elevation::SendCommand(ELEVATION_COMMAND Command, args&&... Args) const
{
	Write(Command);
	Write(std::forward<args>(Args)...);
}

void elevation::RetrieveLastError() const
{
	const auto ErrorCodes = Read<error_codes>();
	SetLastError(ErrorCodes.Win32Error);
	Imports().RtlNtStatusToDosError(ErrorCodes.NtError);
}

template<typename T>
T elevation::RetrieveLastErrorAndResult() const
{
	RetrieveLastError();
	return Read<T>();
}

template<typename T, typename F1, typename F2>
auto elevation::execute(lng Why, const string& Object, T Fallback, const F1& PrivilegedHander, const F2& ElevatedHandler)
{
	SCOPED_ACTION(CriticalSectionLock)(m_CS);
	if (!ElevationApproveDlg(Why, Object))
		return Fallback;

	if (is_admin())
	{
		SCOPED_ACTION(auto)(CreateBackupRestorePrivilege());
		return PrivilegedHander();
	}

	if (!Initialize())
		return Fallback;

	try
	{
		return ElevatedHandler();
	}
	catch(const far_exception&)
	{
		// TODO: log
		return Fallback;
	}
}

bool elevation::Initialize()
{
	bool Result=false;
	if (!m_Pipe)
	{
		m_PipeName = GuidToStr(CreateUuid());
		SID_IDENTIFIER_AUTHORITY NtAuthority=SECURITY_NT_AUTHORITY;

		if (const auto pSD = os::memory::local::alloc<SECURITY_DESCRIPTOR>(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH))
		{
			if (InitializeSecurityDescriptor(pSD.get(), SECURITY_DESCRIPTOR_REVISION))
			{
				if (const auto AdminSID = os::make_sid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS))
				{
					EXPLICIT_ACCESS ea{};
					ea.grfAccessPermissions = GENERIC_READ | GENERIC_WRITE;
					ea.grfAccessMode = SET_ACCESS;
					ea.grfInheritance = NO_INHERITANCE;
					ea.Trustee.TrusteeForm = TRUSTEE_IS_SID;
					ea.Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
					ea.Trustee.ptstrName = static_cast<LPWSTR>(AdminSID.get());

					os::memory::local::ptr<ACL> pACL;
					if (SetEntriesInAcl(1, &ea, nullptr, &ptr_setter(pACL)) == ERROR_SUCCESS)
					{
						if (SetSecurityDescriptorDacl(pSD.get(), TRUE, pACL.get(), FALSE))
						{
							SECURITY_ATTRIBUTES sa{ sizeof(SECURITY_ATTRIBUTES), pSD.get(), FALSE };
							const auto strPipe = L"\\\\.\\pipe\\" + m_PipeName;
							m_Pipe.reset(CreateNamedPipe(strPipe.data(), PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED, PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT, 1, 0, 0, 0, &sa));
						}
					}
				}
			}
		}
	}
	if (m_Pipe)
	{
		if(m_Process)
		{
			if (!m_Process.signaled())
			{
				Result = true;
			}
			else
			{
				m_Process.close();
			}
		}
		if(!Result)
		{
			SCOPED_ACTION(IndeterminateTaskBar);
			DisconnectNamedPipe(m_Pipe.native_handle());

			bool InJob = false;
			if (!m_Job)
			{
				// IsProcessInJob not exist in win2k. use QueryInformationJobObject(nullptr, ...) instead.
				// IsProcessInJob(GetCurrentProcess(), nullptr, &InJob);

				JOBOBJECT_EXTENDED_LIMIT_INFORMATION jeli={};
				InJob = QueryInformationJobObject(nullptr, JobObjectExtendedLimitInformation, &jeli, sizeof(jeli), nullptr) != FALSE;
				if (!InJob)
				{
					m_Job.reset(CreateJobObject(nullptr, nullptr));
					if (m_Job)
					{
						jeli.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_SILENT_BREAKAWAY_OK | JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
						if (SetInformationJobObject(m_Job.native_handle(), JobObjectExtendedLimitInformation, &jeli, sizeof(jeli)))
						{
							AssignProcessToJobObject(m_Job.native_handle(), GetCurrentProcess());
						}
					}
				}
			}

			string Param = concat(ElevationArgument, L' ', m_PipeName, L' ', str(GetCurrentProcessId()), L' ', (Global->Opt->ElevationMode&ELEVATION_USE_PRIVILEGES)? L'1' : L'0');

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
				m_Process.reset(info.hProcess);
				if (!InJob && m_Job)
				{
					AssignProcessToJobObject(m_Job.native_handle(), m_Process.native_handle());
				}
				Event AEvent(Event::automatic, Event::nonsignaled);
				OVERLAPPED Overlapped;
				AEvent.Associate(Overlapped);
				ConnectNamedPipe(m_Pipe.native_handle(), &Overlapped);
				MultiWaiter Waiter;
				Waiter.Add(AEvent);
				Waiter.Add(m_Process.native_handle());
				switch (Waiter.Wait(MultiWaiter::wait_any, 15000))
				{
				case WAIT_OBJECT_0:
					{
						DWORD NumberOfBytesTransferred;
						if (GetOverlappedResult(m_Pipe.native_handle(), &Overlapped, &NumberOfBytesTransferred, FALSE))
						{
							Read(m_ProcessId);
							Result = true;
						}
					}
					break;

				case WAIT_OBJECT_0 + 1:
					{
						DWORD ExitCode;
						SetLastError(GetExitCodeProcess(m_Process.native_handle(), &ExitCode)? ExitCode : ERROR_GEN_FAILURE);
					}
					break;

				default:
					break;
				}

				if(!Result)
				{
					if (!m_Process.signaled())
					{
						TerminateProcess(m_Process.native_handle(), 0);
						m_Process.close();
						m_Job.close();
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
	m_Elevation=Result;
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
	lng Why;
	bool& AskApprove;
	bool& IsApproved;
	bool& DontAskAgain;
	EAData(const string& Object, lng Why, bool& AskApprove, bool& IsApproved, bool& DontAskAgain):
		Object(Object), Why(Why), AskApprove(AskApprove), IsApproved(IsApproved), DontAskAgain(DontAskAgain){}
};

void ElevationApproveDlgSync(const EAData& Data)
{
	SCOPED_ACTION(message_manager::suppress);

	enum {DlgX=64,DlgY=12};
	FarDialogItem ElevationApproveDlgData[]=
	{
		{DI_DOUBLEBOX,3,1,DlgX-4,DlgY-2,0,nullptr,nullptr,0,MSG(lng::MAccessDenied)},
		{DI_TEXT,5,2,0,2,0,nullptr,nullptr,0,MSG(is_admin()? lng::MElevationRequiredPrivileges : lng::MElevationRequired)},
		{DI_TEXT,5,3,0,3,0,nullptr,nullptr,0,MSG(Data.Why)},
		{DI_EDIT,5,4,DlgX-6,4,0,nullptr,nullptr,DIF_READONLY,Data.Object.data()},
		{DI_CHECKBOX,5,6,0,6,1,nullptr,nullptr,0,MSG(lng::MElevationDoForAll)},
		{DI_CHECKBOX,5,7,0,7,0,nullptr,nullptr,0,MSG(lng::MElevationDoNotAskAgainInTheCurrentSession)},
		{DI_TEXT,-1,DlgY-4,0,DlgY-4,0,nullptr,nullptr,DIF_SEPARATOR,L""},
		{DI_BUTTON,0,DlgY-3,0,DlgY-3,0,nullptr,nullptr,DIF_DEFAULTBUTTON|DIF_FOCUS|DIF_SETSHIELD|DIF_CENTERGROUP,MSG(lng::MOk)},
		{DI_BUTTON,0,DlgY-3,0,DlgY-3,0,nullptr,nullptr,DIF_CENTERGROUP,MSG(lng::MSkip)},
	};
	auto ElevationApproveDlg = MakeDialogItemsEx(ElevationApproveDlgData);
	const auto Dlg = Dialog::create(ElevationApproveDlg, ElevationApproveDlgProc);
	Dlg->SetHelp(L"ElevationDlg");
	Dlg->SetPosition(-1, -1, DlgX, DlgY);
	Dlg->SetDialogMode(DMODE_FULLSHADOW | DMODE_NOPLUGINS);
	const auto Current = Global->WindowManager->GetCurrentWindow();
	const auto Lock = Global->ScrBuf->GetLockCount();
	Global->ScrBuf->SetLockCount(0);

	// We're locking current window as it might not expect refresh at this time at all
	// However, that also mean that title won't be restored after closing the dialog.
	// So we do it manually.
	const auto OldTitle = ConsoleTitle::GetTitle();

	Console().FlushInputBuffer();

	Dlg->Process();
	ConsoleTitle::SetFarTitle(OldTitle);
	Global->ScrBuf->SetLockCount(Lock);

	Data.AskApprove = ElevationApproveDlg[AAD_CHECKBOX_DOFORALL].Selected == BSTATE_UNCHECKED;
	Data.IsApproved = Dlg->GetExitCode() == AAD_BUTTON_OK;
	Data.DontAskAgain = ElevationApproveDlg[AAD_CHECKBOX_DONTASKAGAIN].Selected == BSTATE_CHECKED;
}

bool elevation::ElevationApproveDlg(lng Why, const string& Object)
{
	if (m_Suppressions)
		return false;

	// request for backup&restore privilege is useless if the user already has them
	{
		SCOPED_ACTION(GuardLastError);
		if (m_AskApprove && is_admin() && privilege::check(SE_BACKUP_NAME, SE_RESTORE_NAME))
		{
			m_AskApprove = false;
			return true;
		}
	}

	if(!(is_admin() && !(Global->Opt->ElevationMode&ELEVATION_USE_PRIVILEGES)) &&
		m_AskApprove && !m_DontAskAgain && !m_Recurse &&
 		Global->WindowManager && !Global->WindowManager->ManagerIsDown())
	{
		++m_Recurse;
		SCOPED_ACTION(GuardLastError);
		SCOPED_ACTION(TaskBarPause);
		EAData Data(Object, Why, m_AskApprove, m_IsApproved, m_DontAskAgain);

		if(!Global->IsMainThread())
		{
			Event SyncEvent(Event::automatic, Event::nonsignaled);
			listener_ex Listener([&SyncEvent](const any& Payload)
			{
				ElevationApproveDlgSync(*any_cast<EAData*>(Payload));
				SyncEvent.Set();
			});
			MessageManager().notify(Listener.GetEventName(), &Data);
			SyncEvent.Wait();
		}
		else
		{
			ElevationApproveDlgSync(Data);
		}
		--m_Recurse;
	}
	return m_IsApproved;
}

bool elevation::fCreateDirectoryEx(const string& TemplateObject, const string& Object, LPSECURITY_ATTRIBUTES Attributes)
{
	return execute(lng::MElevationRequiredCreate, Object,
		false,
		[&]
		{
			return TemplateObject.empty()? CreateDirectory(Object.data(), Attributes) != FALSE : CreateDirectoryEx(TemplateObject.data(), Object.data(), Attributes) != FALSE;
		},
		[&]
		{
			SendCommand(C_FUNCTION_CREATEDIRECTORYEX, TemplateObject, Object);
			// BUGBUG: SecurityAttributes ignored
			return RetrieveLastErrorAndResult<bool>();
		});
}

bool elevation::fRemoveDirectory(const string& Object)
{
	return execute(lng::MElevationRequiredDelete, Object,
		false,
		[&]
		{
			return RemoveDirectory(Object.data()) != FALSE;
		},
		[&]
		{
			SendCommand(C_FUNCTION_REMOVEDIRECTORY, Object);
			return RetrieveLastErrorAndResult<bool>();
		});
}

bool elevation::fDeleteFile(const string& Object)
{
	return execute(lng::MElevationRequiredDelete, Object,
		false,
		[&]
		{
			return DeleteFile(Object.data()) != FALSE;
		},
		[&]
		{
			SendCommand(C_FUNCTION_DELETEFILE, Object);
			return RetrieveLastErrorAndResult<bool>();
		});
}

void elevation::fCallbackRoutine(LPPROGRESS_ROUTINE ProgressRoutine) const
{
	if (!ProgressRoutine)
		return;

	const auto TotalFileSize = Read<LARGE_INTEGER>();
	const auto TotalBytesTransferred = Read<LARGE_INTEGER>();
	const auto StreamSize = Read<LARGE_INTEGER>();
	const auto StreamBytesTransferred = Read<LARGE_INTEGER>();
	const auto StreamNumber = Read<DWORD>();
	const auto CallbackReason = Read<DWORD>();
	const auto Data = Read<intptr_t>();
	// BUGBUG: SourceFile, DestinationFile ignored

	const auto Result = ProgressRoutine(TotalFileSize, TotalBytesTransferred, StreamSize, StreamBytesTransferred, StreamNumber, CallbackReason, INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE, reinterpret_cast<void*>(Data));

	Write(CallbackMagic);
	Write(Result);
}

bool elevation::fCopyFileEx(const string& From, const string& To, LPPROGRESS_ROUTINE ProgressRoutine, LPVOID Data, LPBOOL Cancel, DWORD Flags)
{
	return execute(lng::MElevationRequiredCopy, From,
		false,
		[&]
		{
			return CopyFileEx(From.data(), To.data(), ProgressRoutine, Data, Cancel, Flags) != FALSE;
		},
		[&]
		{
			SendCommand(C_FUNCTION_COPYFILEEX, From, To, reinterpret_cast<intptr_t>(ProgressRoutine), reinterpret_cast<intptr_t>(Data), Flags);
			// BUGBUG: Cancel ignored

			while (Read<int>() == CallbackMagic)
			{
				fCallbackRoutine(ProgressRoutine);
			}

			return RetrieveLastErrorAndResult<bool>();
		});
}

bool elevation::fMoveFileEx(const string& From, const string& To, DWORD Flags)
{
	return execute(lng::MElevationRequiredMove, From,
		false,
		[&]
		{
			return MoveFileEx(From.data(), To.data(), Flags) != FALSE;
		},
		[&]
		{
			SendCommand(C_FUNCTION_MOVEFILEEX, From, To, Flags);
			return RetrieveLastErrorAndResult<bool>();
		});
}

DWORD elevation::fGetFileAttributes(const string& Object)
{
	return execute(lng::MElevationRequiredGetAttributes, Object,
		INVALID_FILE_ATTRIBUTES,
		[&]
		{
			return GetFileAttributes(Object.data());
		},
		[&]
		{
			SendCommand(C_FUNCTION_GETFILEATTRIBUTES, Object);
			return RetrieveLastErrorAndResult<DWORD>();
		});
}

bool elevation::fSetFileAttributes(const string& Object, DWORD FileAttributes)
{
	return execute(lng::MElevationRequiredSetAttributes, Object,
		false,
		[&]
		{
			return SetFileAttributes(Object.data(), FileAttributes) != FALSE;
		},
		[&]
		{
			SendCommand(C_FUNCTION_SETFILEATTRIBUTES, Object, FileAttributes);
			return RetrieveLastErrorAndResult<bool>();
		});
}

bool elevation::fCreateHardLink(const string& Object, const string& Target, LPSECURITY_ATTRIBUTES SecurityAttributes)
{
	return execute(lng::MElevationRequiredHardLink, Object,
		false,
		[&]
		{
			return CreateHardLink(Object.data(), Target.data(), SecurityAttributes) != FALSE;
		},
		[&]
		{
			SendCommand(C_FUNCTION_CREATEHARDLINK, Object, Target);
			// BUGBUG: SecurityAttributes ignored.
			return RetrieveLastErrorAndResult<bool>();
		});
}

bool elevation::fCreateSymbolicLink(const string& Object, const string& Target, DWORD Flags)
{
	return execute(lng::MElevationRequiredSymLink, Object,
		false,
		[&]
		{
			return os::CreateSymbolicLinkInternal(Object, Target, Flags);
		},
		[&]
		{
			SendCommand(C_FUNCTION_CREATESYMBOLICLINK, Object, Target, Flags);
			return RetrieveLastErrorAndResult<bool>();
		});
}

int elevation::fMoveToRecycleBin(SHFILEOPSTRUCT& FileOpStruct)
{
	static constexpr auto DE_ACCESSDENIEDSRC = 0x78;
	return execute(lng::MElevationRequiredRecycle, FileOpStruct.pFrom,
		DE_ACCESSDENIEDSRC,
		[&]
		{
			return SHFileOperation(&FileOpStruct);
		},
		[&]
		{
			SendCommand(C_FUNCTION_MOVETORECYCLEBIN, FileOpStruct,
				make_blob_view(FileOpStruct.pFrom, (wcslen(FileOpStruct.pFrom) + 1 + 1) * sizeof(wchar_t)), // achtung! +1
				make_blob_view(FileOpStruct.pTo, FileOpStruct.pTo ? (wcslen(FileOpStruct.pTo) + 1 + 1) * sizeof(wchar_t) : 0)); // achtung! +1

			Read(FileOpStruct.fAnyOperationsAborted);
			// achtung! no "last error" here
			return Read<int>();
		});
}

bool elevation::fSetOwner(const string& Object, const string& Owner)
{
	return execute(lng::MElevationRequiredSetOwner, Object,
		false,
		[&]
		{
			return SetOwnerInternal(Object, Owner);
		},
		[&]
		{
			SendCommand(C_FUNCTION_SETOWNER, Object, Owner);
			return RetrieveLastErrorAndResult<bool>();
		});
}

HANDLE elevation::fCreateFile(const string& Object, DWORD DesiredAccess, DWORD ShareMode, LPSECURITY_ATTRIBUTES SecurityAttributes, DWORD CreationDistribution, DWORD FlagsAndAttributes, HANDLE TemplateFile)
{
	return execute(lng::MElevationRequiredOpen, Object,
		INVALID_HANDLE_VALUE,
		[&]
		{
			return CreateFile(Object.data(), DesiredAccess, ShareMode, SecurityAttributes, CreationDistribution, FlagsAndAttributes, TemplateFile);
		},
		[&]
		{
			SendCommand(C_FUNCTION_CREATEFILE, Object, DesiredAccess, ShareMode, CreationDistribution, FlagsAndAttributes);
			// BUGBUG: SecurityAttributes & TemplateFile ignored
			return ToPtr(RetrieveLastErrorAndResult<intptr_t>());
		});
}

bool elevation::fSetFileEncryption(const string& Object, bool Encrypt)
{
	return execute(Encrypt? lng::MElevationRequiredEncryptFile : lng::MElevationRequiredDecryptFile, Object,
		false,
		[&]
		{
			return os::SetFileEncryptionInternal(Object.data(), Encrypt);
		},
		[&]
		{
			SendCommand(C_FUNCTION_SETENCRYPTION, Object, Encrypt);
			return RetrieveLastErrorAndResult<bool>();
		});
}

bool elevation::fDetachVirtualDisk(const string& Object, VIRTUAL_STORAGE_TYPE& VirtualStorageType)
{
	return execute(lng::MElevationRequiredCreate, Object,
		false,
		[&]
		{
			return os::DetachVirtualDiskInternal(Object, VirtualStorageType);
		},
		[&]
		{
			SendCommand(C_FUNCTION_DETACHVIRTUALDISK, Object, VirtualStorageType);
			return RetrieveLastErrorAndResult<bool>();
		});
}

bool elevation::fGetDiskFreeSpaceEx(const string& Object, ULARGE_INTEGER* FreeBytesAvailableToCaller, ULARGE_INTEGER* TotalNumberOfBytes, ULARGE_INTEGER* TotalNumberOfFreeBytes)
{
	return execute(lng::MElevationRequiredList, Object,
		false,
		[&]
		{
			return GetDiskFreeSpaceEx(Object.data(), FreeBytesAvailableToCaller, TotalNumberOfBytes, TotalNumberOfFreeBytes) != FALSE;
		},
		[&]
		{
			SendCommand(C_FUNCTION_GETDISKFREESPACEEX, Object);
			const auto Result = RetrieveLastErrorAndResult<bool>();
			if (Result)
			{
				const auto& ReadAndAssign = [this](auto* Destination)
				{
					ULARGE_INTEGER Buffer;
					Read(Buffer);
					if (Destination)
						*Destination = Buffer;
				};

				ReadAndAssign(FreeBytesAvailableToCaller);
				ReadAndAssign(TotalNumberOfBytes);
				ReadAndAssign(TotalNumberOfFreeBytes);
			}
			return Result;
		});
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
	int Run(const wchar_t* guid, DWORD PID, bool UsePrivileges)
	{
		SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOOPENFILEERRORBOX);

		std::vector<const wchar_t*> Privileges{ SE_TAKE_OWNERSHIP_NAME, SE_DEBUG_NAME, SE_CREATE_SYMBOLIC_LINK_NAME };
		if (UsePrivileges)
		{
			Privileges.emplace_back(SE_BACKUP_NAME);
			Privileges.emplace_back(SE_RESTORE_NAME);
		}

		SCOPED_ACTION(privilege)(Privileges);

		const auto PipeName = string(L"\\\\.\\pipe\\") + guid;
		WaitNamedPipe(PipeName.data(), NMPWAIT_WAIT_FOREVER);
		m_Pipe.reset(CreateFile(PipeName.data(),GENERIC_READ|GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr));
		if (!m_Pipe)
			return GetLastError();

		{
			// basic security checks
			ULONG ServerProcessId;
			if (Imports().GetNamedPipeServerProcessId && (!Imports().GetNamedPipeServerProcessId(m_Pipe.native_handle(), &ServerProcessId) || ServerProcessId != PID))
				return GetLastError();

			auto ParentProcess = os::handle(OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, PID));
			if (!ParentProcess)
				return GetLastError();

			string ParentProcessFileName;
			if (!os::GetModuleFileNameEx(ParentProcess.native_handle(), nullptr, ParentProcessFileName))
				return GetLastError();

			string CurrentProcessFileName;
			if (!os::GetModuleFileNameEx(GetCurrentProcess(), nullptr, CurrentProcessFileName))
				return GetLastError();

			if (StrCmpI(CurrentProcessFileName, ParentProcessFileName))
				return -1;
		}

		Write(GetCurrentProcessId());
		m_ParentPid = PID;

		for (;;)
		{
			if (!Process(Read<int>()))
				break;
		}

		return 0;
	}

private:
	os::handle m_Pipe;
	DWORD m_ParentPid{};
	mutable bool m_Active{true};

	struct callback_param
	{
		const class elevated* Owner;
		void* UserData;
		std::exception_ptr ExceptionPtr;
	};

	void Write(const void* Data,size_t DataSize) const
	{
		if (!pipe::Write(m_Pipe, Data, DataSize))
			throw MAKE_FAR_EXCEPTION(L"Pipe write error");
	}

	template<typename T>
	T Read() const
	{
		T Data;
		if (!pipe::Read(m_Pipe, Data))
			throw MAKE_FAR_EXCEPTION(L"Pipe read error");
		return Data;
	}

	template<typename T>
	void Write(const T& Data) const
	{
		if (!pipe::Write(m_Pipe, Data))
			throw MAKE_FAR_EXCEPTION(L"Pipe write error");
	}

	void ExitHandler() const
	{
		m_Active = false;
	}

	void CreateDirectoryExHandler() const
	{
		const auto TemplateObject = Read<string>();
		const auto Object = Read<string>();
		// BUGBUG, SecurityAttributes ignored

		const auto Result = TemplateObject.empty()? CreateDirectory(Object.data(), nullptr) != FALSE : CreateDirectoryEx(TemplateObject.data(), Object.data(), nullptr) != FALSE;

		Write(error_codes{});
		Write(Result);
	}

	void RemoveDirectoryHandler() const
	{
		const auto Object = Read<string>();

		const auto Result = RemoveDirectory(Object.data()) != FALSE;

		Write(error_codes{});
		Write(Result);
	}

	void DeleteFileHandler() const
	{
		const auto Object = Read<string>();

		const auto Result = DeleteFile(Object.data()) != FALSE;

		Write(error_codes{});
		Write(Result);
	}

	void CopyFileExHandler() const
	{
		const auto From = Read<string>();
		const auto To = Read<string>();
		const auto UserCopyProgressRoutine = Read<intptr_t>();
		const auto Data = Read<intptr_t>();
		const auto Flags = Read<DWORD>();
		// BUGBUG: Cancel ignored

		callback_param Param{ this, reinterpret_cast<void*>(Data) };
		const auto Result = CopyFileEx(From.data(), To.data(), UserCopyProgressRoutine? CopyProgressRoutineWrapper : nullptr, &Param, nullptr, Flags) != FALSE;

		Write(0); // not CallbackMagic
		Write(error_codes{});
		Write(Result);

		RethrowIfNeeded(Param.ExceptionPtr);
	}

	void MoveFileExHandler() const
	{
		const auto From = Read<string>();
		const auto To = Read<string>();
		const auto Flags = Read<DWORD>();

		const auto Result = MoveFileEx(From.data(), To.data(), Flags) != FALSE;

		Write(error_codes{});
		Write(Result);
	}

	void GetFileAttributesHandler() const
	{
		const auto Object = Read<string>();

		const auto Result = GetFileAttributes(Object.data());

		Write(error_codes{});
		Write(Result);
	}

	void SetFileAttributesHandler() const
	{
		const auto Object = Read<string>();
		const auto Attributes = Read<DWORD>();

		const auto Result = SetFileAttributes(Object.data(), Attributes) != FALSE;

		Write(error_codes{});
		Write(Result);
	}

	void CreateHardLinkHandler() const
	{
		const auto Object = Read<string>();
		const auto Target = Read<string>();
		// BUGBUG: SecurityAttributes ignored.

		const auto Result = CreateHardLink(Object.data(), Target.data(), nullptr) != FALSE;

		Write(error_codes{});
		Write(Result);
	}

	void CreateSymbolicLinkHandler() const
	{
		const auto Object = Read<string>();
		const auto Target = Read<string>();
		const auto Flags = Read<DWORD>();

		const auto Result = os::CreateSymbolicLinkInternal(Object, Target, Flags);

		Write(error_codes{});
		Write(Result);
	}

	void MoveToRecycleBinHandler() const
	{
		auto Struct = Read<SHFILEOPSTRUCT>();
		const auto From = Read<string>();
		const auto To = Read<string>();

		Struct.pFrom = From.data();
		Struct.pTo = To.data();

		const auto Result = SHFileOperation(&Struct);

		Write(Struct.fAnyOperationsAborted);
		Write(Result);
	}

	void SetOwnerHandler() const
	{
		const auto Object = Read<string>();
		const auto Owner = Read<string>();

		const auto Result = SetOwnerInternal(Object, Owner);

		Write(error_codes{});
		Write(Result);
	}

	void CreateFileHandler() const
	{
		const auto Object = Read<string>();
		const auto DesiredAccess = Read<DWORD>();
		const auto ShareMode = Read<DWORD>();
		const auto CreationDistribution = Read<DWORD>();
		const auto FlagsAndAttributes = Read<DWORD>();
		// BUGBUG: SecurityAttributes, TemplateFile ignored

		auto Duplicate = INVALID_HANDLE_VALUE;
		if (const auto Handle = os::CreateFile(Object, DesiredAccess, ShareMode, nullptr, CreationDistribution, FlagsAndAttributes, nullptr))
		{
			if (const auto ParentProcess = os::handle(OpenProcess(PROCESS_DUP_HANDLE, FALSE, m_ParentPid)))
			{
				DuplicateHandle(GetCurrentProcess(), Handle.native_handle(), ParentProcess.native_handle(), &Duplicate, 0, FALSE, DUPLICATE_SAME_ACCESS);
			}
		}

		Write(error_codes{});
		Write(reinterpret_cast<intptr_t>(Duplicate));
	}

	void SetEncryptionHandler() const
	{
		const auto Object = Read<string>();
		const auto Encrypt = Read<bool>();

		const auto Result = os::SetFileEncryptionInternal(Object.data(), Encrypt);

		Write(error_codes{});
		Write(Result);
	}

	void DetachVirtualDiskHandler() const
	{
		const auto Object = Read<string>();
		auto VirtualStorageType = Read<VIRTUAL_STORAGE_TYPE>();

		const auto Result = os::DetachVirtualDiskInternal(Object, VirtualStorageType);

		Write(error_codes{});
		Write(Result);
	}

	void GetDiskFreeSpaceExHandler() const
	{
		const auto Object = Read<string>();

		ULARGE_INTEGER FreeBytesAvailableToCaller, TotalNumberOfBytes, TotalNumberOfFreeBytes;
		const auto Result = GetDiskFreeSpaceEx(Object.data(), &FreeBytesAvailableToCaller, &TotalNumberOfBytes, &TotalNumberOfFreeBytes) != FALSE;

		Write(error_codes{});
		Write(Result);

		if(Result)
		{
			Write(FreeBytesAvailableToCaller);
			Write(TotalNumberOfBytes);
			Write(TotalNumberOfFreeBytes);
		}
	}

	static DWORD CALLBACK CopyProgressRoutineWrapper(LARGE_INTEGER TotalFileSize, LARGE_INTEGER TotalBytesTransferred, LARGE_INTEGER StreamSize, LARGE_INTEGER StreamBytesTransferred, DWORD StreamNumber, DWORD CallbackReason, HANDLE SourceFile,HANDLE DestinationFile, LPVOID Data)
	{
		const auto Param = reinterpret_cast<callback_param*>(Data);
		try
		{
			const auto Context = Param->Owner;

			Context->Write(CallbackMagic);
			Context->Write(TotalFileSize);
			Context->Write(TotalBytesTransferred);
			Context->Write(StreamSize);
			Context->Write(StreamBytesTransferred);
			Context->Write(StreamNumber);
			Context->Write(CallbackReason);
			Context->Write(reinterpret_cast<intptr_t>(Param->UserData));
			// BUGBUG: SourceFile, DestinationFile ignored

			for (;;)
			{
				const auto Result = Context->Read<int>();
				if (Result == CallbackMagic)
				{
					return Context->Read<int>();
				}
				// nested call from ProgressRoutine()
				Context->Process(Result);
			}
		}
		catch(...)
		{
			Param->ExceptionPtr = std::current_exception();
			return PROGRESS_CANCEL;
		}
	}

	bool Process(int Command) const
	{
		assert(Command < C_COMMANDS_COUNT);

		static constexpr decltype(&elevated::ExitHandler) Handlers[] =
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

		TERSE_STATIC_ASSERT(std::size(Handlers) == C_COMMANDS_COUNT);

		try
		{
			std::invoke(Handlers[Command], this);
			return m_Active;
		}
		catch(...)
		{
			// TODO: log
			return false;
		}
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