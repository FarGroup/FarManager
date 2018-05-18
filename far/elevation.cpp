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

#include "elevation.hpp"

#include "config.hpp"
#include "lang.hpp"
#include "dialog.hpp"
#include "farcolor.hpp"
#include "colormix.hpp"
#include "lasterror.hpp"
#include "fileowner.hpp"
#include "imports.hpp"
#include "taskbar.hpp"
#include "notification.hpp"
#include "scrbuf.hpp"
#include "manager.hpp"
#include "pipe.hpp"
#include "console.hpp"
#include "constitle.hpp"
#include "string_utils.hpp"
#include "global.hpp"

#include "platform.concurrency.hpp"
#include "platform.fs.hpp"
#include "platform.memory.hpp"
#include "platform.security.hpp"

#include "common/bytes_view.hpp"

#include "format.hpp"

using namespace os::security;

static const int CallbackMagic= 0xCA11BAC6;

enum ELEVATION_COMMAND: int
{
	C_SERVICE_EXIT,
	C_FUNCTION_CREATEDIRECTORY,
	C_FUNCTION_REMOVEDIRECTORY,
	C_FUNCTION_DELETEFILE,
	C_FUNCTION_COPYFILE,
	C_FUNCTION_MOVEFILE,
	C_FUNCTION_GETFILEATTRIBUTES,
	C_FUNCTION_SETFILEATTRIBUTES,
	C_FUNCTION_CREATEHARDLINK,
	C_FUNCTION_CREATESYMBOLICLINK,
	C_FUNCTION_MOVETORECYCLEBIN,
	C_FUNCTION_SETOWNER,
	C_FUNCTION_CREATEFILE,
	C_FUNCTION_SETENCRYPTION,
	C_FUNCTION_DETACHVIRTUALDISK,
	C_FUNCTION_GETDISKFREESPACE,

	C_COMMANDS_COUNT
};

static const auto ElevationArgument = L"/service:elevation"sv;

static auto CreateBackupRestorePrivilege() { return privilege{SE_BACKUP_NAME, SE_RESTORE_NAME}; }

elevation::elevation():
	m_Suppressions(),
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
			Write(C_SERVICE_EXIT);
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

template<typename T>
T elevation::Read() const
{
	T Data;
	if (!pipe::Read(m_Pipe, Data))
		throw MAKE_FAR_EXCEPTION(L"Pipe read error"sv);
	return Data;
}

template<typename T, typename... args>
void elevation::Write(const T& Data, args&&... Args) const
{
	WriteArg(Data);
	Write(FWD(Args)...);
}

template<typename T>
void elevation::WriteArg(const T& Data) const
{
	if (!pipe::Write(m_Pipe, Data))
		throw MAKE_FAR_EXCEPTION(L"Pipe write error"sv);
}

void elevation::WriteArg(const bytes_view& Data) const
{
	if (!pipe::Write(m_Pipe, Data.data(), Data.size()))
		throw MAKE_FAR_EXCEPTION(L"Pipe write error"sv);
}

void elevation::RetrieveLastError() const
{
	const auto ErrorState = Read<error_state>();
	SetLastError(ErrorState.Win32Error);
	imports.RtlNtStatusToDosError(ErrorState.NtError);
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
	SCOPED_ACTION(os::critical_section_lock)(m_CS);
	if (!ElevationApproveDlg(Why, Object))
		return Fallback;

	if (is_admin())
	{
		SCOPED_ACTION(auto)(CreateBackupRestorePrivilege());
		return PrivilegedHander();
	}

	m_Elevation = Initialize();
	if (!m_Elevation)
	{
		ResetApprove();
		return Fallback;
	}

	try
	{
		return ElevatedHandler();
	}
	catch (const far_exception&)
	{
		// Something went really bad, it's better to stop any further attempts
		TerminateChildProcess();
		m_Process.close();
		m_Pipe.close();

		// TODO: log
		return Fallback;
	}
}

static os::handle create_named_pipe(const string& Name)
{
	SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
	const auto pSD = os::memory::local::alloc<SECURITY_DESCRIPTOR>(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH);
	if (!pSD)
		return nullptr;

	if (!InitializeSecurityDescriptor(pSD.get(), SECURITY_DESCRIPTOR_REVISION))
		return nullptr;

	const auto AdminSID = os::security::make_sid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS);

	if (!AdminSID)
		return nullptr;

	EXPLICIT_ACCESS ea{};
	ea.grfAccessPermissions = GENERIC_READ | GENERIC_WRITE;
	ea.grfAccessMode = SET_ACCESS;
	ea.grfInheritance = NO_INHERITANCE;
	ea.Trustee.TrusteeForm = TRUSTEE_IS_SID;
	ea.Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
	ea.Trustee.ptstrName = static_cast<LPWSTR>(AdminSID.get());

	os::memory::local::ptr<ACL> pACL;
	if (SetEntriesInAcl(1, &ea, nullptr, &ptr_setter(pACL)) != ERROR_SUCCESS)
		return nullptr;

	if (!SetSecurityDescriptorDacl(pSD.get(), TRUE, pACL.get(), FALSE))
		return nullptr;

	SECURITY_ATTRIBUTES sa{ sizeof(SECURITY_ATTRIBUTES), pSD.get(), FALSE };
	return os::handle(CreateNamedPipe(concat(L"\\\\.\\pipe\\"sv, Name).c_str(), PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED, PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT, 1, 0, 0, 0, &sa));
}

static os::handle create_job_for_current_process()
{
	// IsProcessInJob not exist in win2k. use QueryInformationJobObject(nullptr, ...) instead.
	// IsProcessInJob(GetCurrentProcess(), nullptr, &InJob);

	JOBOBJECT_EXTENDED_LIMIT_INFORMATION jeli = {};
	const auto InJob = QueryInformationJobObject(nullptr, JobObjectExtendedLimitInformation, &jeli, sizeof(jeli), nullptr) != FALSE;
	if (InJob)
	{
		// TODO: Windows 8+ supports nested jobs
		return nullptr;
	}

	os::handle Job(CreateJobObject(nullptr, nullptr));
	if (!Job)
		return nullptr;

	// Child processes shall not inherit this job by default
	jeli.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_SILENT_BREAKAWAY_OK | JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
	if (!SetInformationJobObject(Job.native_handle(), JobObjectExtendedLimitInformation, &jeli, sizeof(jeli)))
		return nullptr;
	
	if (!AssignProcessToJobObject(Job.native_handle(), GetCurrentProcess()))
		return nullptr;

	return Job;
}

static os::handle create_elevated_process(const string& Parameters)
{
	SHELLEXECUTEINFO info
	{
		sizeof(info),
		SEE_MASK_FLAG_NO_UI | SEE_MASK_UNICODE | SEE_MASK_NOASYNC | SEE_MASK_NOCLOSEPROCESS,
		nullptr,
		L"runas",
		Global->g_strFarModuleName.c_str(),
		Parameters.c_str(),
		Global->g_strFarPath.c_str(),
	};

	if (!ShellExecuteEx(&info))
		return nullptr;

	return os::handle(info.hProcess);
}

static bool connect_pipe_to_process(const os::handle& Process, const os::handle& Pipe)
{
	os::event AEvent(os::event::type::automatic, os::event::state::nonsignaled);
	OVERLAPPED Overlapped;
	AEvent.associate(Overlapped);
	if (!ConnectNamedPipe(Pipe.native_handle(), &Overlapped))
	{
		const auto LastError = GetLastError();
		if (LastError != ERROR_IO_PENDING && LastError != ERROR_PIPE_CONNECTED)
			return false;
	}

	os::multi_waiter Waiter;
	Waiter.add(AEvent);
	Waiter.add(Process.native_handle());
	if (Waiter.wait(os::multi_waiter::mode::any, 15s) != WAIT_OBJECT_0)
		return false;

	DWORD NumberOfBytesTransferred;
	return GetOverlappedResult(Pipe.native_handle(), &Overlapped, &NumberOfBytesTransferred, FALSE) != FALSE;
}

void elevation::TerminateChildProcess() const
{
	if (!m_Process.is_signaled())
	{
		TerminateProcess(m_Process.native_handle(), ERROR_PROCESS_ABORTED);
		SetLastError(ERROR_PROCESS_ABORTED);
	}
}

bool elevation::Initialize()
{
	if (m_Process && !m_Process.is_signaled())
		return true;

	if (!m_Pipe)
	{
		m_PipeName = GuidToStr(CreateUuid());
		m_Pipe = create_named_pipe(m_PipeName);
		if (!m_Pipe)
			return false;
	}

	SCOPED_ACTION(IndeterminateTaskbar);
	DisconnectNamedPipe(m_Pipe.native_handle());

	const auto Param = concat(ElevationArgument, L' ', m_PipeName, L' ', str(GetCurrentProcessId()), L' ', (Global->Opt->ElevationMode & ELEVATION_USE_PRIVILEGES)? L'1' : L'0');

	m_Process = create_elevated_process(Param);

	if (!m_Process)
		return false;

	if (!m_Job)
	{
		m_Job = create_job_for_current_process();
	}

	if (m_Job)
	{
		AssignProcessToJobObject(m_Job.native_handle(), m_Process.native_handle());
	}

	if (!connect_pipe_to_process(m_Process, m_Pipe))
	{
		if (m_Process.is_signaled())
		{
			DWORD ExitCode;
			SetLastError(GetExitCodeProcess(m_Process.native_handle(), &ExitCode)? ExitCode : ERROR_GEN_FAILURE);
		}
		else
		{
			TerminateChildProcess();
		}

		m_Process.close();
		return false;
	}

	return true;
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
				const auto Color = colors::PaletteColorToFarColor(COL_DIALOGTEXT);
				const auto Colors = static_cast<FarDialogItemColors*>(Param2);
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
	SCOPED_ACTION(auto)(message_manager::instance().suppress());

	enum {DlgX=64,DlgY=12};
	FarDialogItem ElevationApproveDlgData[]=
	{
		{DI_DOUBLEBOX,3,1,DlgX-4,DlgY-2,0,nullptr,nullptr,0,msg(lng::MAccessDenied).c_str()},
		{DI_TEXT,5,2,0,2,0,nullptr,nullptr,0,msg(is_admin()? lng::MElevationRequiredPrivileges : lng::MElevationRequired).c_str()},
		{DI_TEXT,5,3,0,3,0,nullptr,nullptr,0,msg(Data.Why).c_str()},
		{DI_EDIT,5,4,DlgX-6,4,0,nullptr,nullptr,DIF_READONLY,Data.Object.c_str()},
		{DI_CHECKBOX,5,6,0,6,1,nullptr,nullptr,0,msg(lng::MElevationDoForAll).c_str()},
		{DI_CHECKBOX,5,7,0,7,0,nullptr,nullptr,0,msg(lng::MElevationDoNotAskAgainInTheCurrentSession).c_str()},
		{DI_TEXT,-1,DlgY-4,0,DlgY-4,0,nullptr,nullptr,DIF_SEPARATOR,L""},
		{DI_BUTTON,0,DlgY-3,0,DlgY-3,0,nullptr,nullptr,DIF_DEFAULTBUTTON|DIF_FOCUS|DIF_SETSHIELD|DIF_CENTERGROUP,msg(lng::MOk).c_str()},
		{DI_BUTTON,0,DlgY-3,0,DlgY-3,0,nullptr,nullptr,DIF_CENTERGROUP,msg(lng::MSkip).c_str()},
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

	console.FlushInputBuffer();

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
		SCOPED_ACTION(TaskbarPause);
		EAData Data(Object, Why, m_AskApprove, m_IsApproved, m_DontAskAgain);

		if(!Global->IsMainThread())
		{
			os::event SyncEvent(os::event::type::automatic, os::event::state::nonsignaled);
			listener Listener([&SyncEvent](const std::any& Payload)
			{
				ElevationApproveDlgSync(*std::any_cast<EAData*>(Payload));
				SyncEvent.set();
			});
			message_manager::instance().notify(Listener.GetEventName(), &Data);
			SyncEvent.wait();
		}
		else
		{
			ElevationApproveDlgSync(Data);
		}
		--m_Recurse;
	}
	return m_IsApproved;
}

bool elevation::create_directory(const string& TemplateObject, const string& Object, SECURITY_ATTRIBUTES* Attributes)
{
	return execute(lng::MElevationRequiredCreate, Object,
		false,
		[&]
		{
			return os::fs::low::create_directory(TemplateObject.c_str(), Object.c_str(), Attributes);
		},
		[&]
		{
			Write(C_FUNCTION_CREATEDIRECTORY, TemplateObject, Object);
			// BUGBUG: SecurityAttributes ignored
			return RetrieveLastErrorAndResult<bool>();
		});
}

bool elevation::remove_directory(const string& Object)
{
	return execute(lng::MElevationRequiredDelete, Object,
		false,
		[&]
		{
			return os::fs::low::remove_directory(Object.c_str());
		},
		[&]
		{
			Write(C_FUNCTION_REMOVEDIRECTORY, Object);
			return RetrieveLastErrorAndResult<bool>();
		});
}

bool elevation::delete_file(const string& Object)
{
	return execute(lng::MElevationRequiredDelete, Object,
		false,
		[&]
		{
			return os::fs::low::delete_file(Object.c_str());
		},
		[&]
		{
			Write(C_FUNCTION_DELETEFILE, Object);
			return RetrieveLastErrorAndResult<bool>();
		});
}

void elevation::progress_routine(LPPROGRESS_ROUTINE ProgressRoutine) const
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

	Write(CallbackMagic, Result);
}

bool elevation::copy_file(const string& From, const string& To, LPPROGRESS_ROUTINE ProgressRoutine, void* Data, BOOL* Cancel, DWORD Flags)
{
	return execute(lng::MElevationRequiredCopy, From,
		false,
		[&]
		{
			return os::fs::low::copy_file(From.c_str(), To.c_str(), ProgressRoutine, Data, Cancel, Flags);
		},
		[&]
		{
			Write(C_FUNCTION_COPYFILE, From, To, reinterpret_cast<intptr_t>(ProgressRoutine), reinterpret_cast<intptr_t>(Data), Flags);
			// BUGBUG: Cancel ignored

			while (Read<int>() == CallbackMagic)
			{
				progress_routine(ProgressRoutine);
			}

			return RetrieveLastErrorAndResult<bool>();
		});
}

bool elevation::move_file(const string& From, const string& To, DWORD Flags)
{
	return execute(lng::MElevationRequiredMove, From,
		false,
		[&]
		{
			return os::fs::low::move_file(From.c_str(), To.c_str(), Flags);
		},
		[&]
		{
			Write(C_FUNCTION_MOVEFILE, From, To, Flags);
			return RetrieveLastErrorAndResult<bool>();
		});
}

DWORD elevation::get_file_attributes(const string& Object)
{
	return execute(lng::MElevationRequiredGetAttributes, Object,
		INVALID_FILE_ATTRIBUTES,
		[&]
		{
			return os::fs::low::get_file_attributes(Object.c_str());
		},
		[&]
		{
			Write(C_FUNCTION_GETFILEATTRIBUTES, Object);
			return RetrieveLastErrorAndResult<DWORD>();
		});
}

bool elevation::set_file_attributes(const string& Object, DWORD FileAttributes)
{
	return execute(lng::MElevationRequiredSetAttributes, Object,
		false,
		[&]
		{
			return os::fs::low::set_file_attributes(Object.c_str(), FileAttributes);
		},
		[&]
		{
			Write(C_FUNCTION_SETFILEATTRIBUTES, Object, FileAttributes);
			return RetrieveLastErrorAndResult<bool>();
		});
}

bool elevation::create_hard_link(const string& Object, const string& Target, SECURITY_ATTRIBUTES* SecurityAttributes)
{
	return execute(lng::MElevationRequiredHardLink, Object,
		false,
		[&]
		{
			return os::fs::low::create_hard_link(Object.c_str(), Target.c_str(), SecurityAttributes);
		},
		[&]
		{
			Write(C_FUNCTION_CREATEHARDLINK, Object, Target);
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
			return os::fs::CreateSymbolicLinkInternal(Object, Target, Flags);
		},
		[&]
		{
			Write(C_FUNCTION_CREATESYMBOLICLINK, Object, Target, Flags);
			return RetrieveLastErrorAndResult<bool>();
		});
}

int elevation::fMoveToRecycleBin(SHFILEOPSTRUCT& FileOpStruct)
{
	static const auto DE_ACCESSDENIEDSRC = 0x78;
	return execute(lng::MElevationRequiredRecycle, FileOpStruct.pFrom,
		DE_ACCESSDENIEDSRC,
		[&]
		{
			return SHFileOperation(&FileOpStruct);
		},
		[&]
		{
			Write(C_FUNCTION_MOVETORECYCLEBIN, FileOpStruct,
				bytes_view(FileOpStruct.pFrom, (wcslen(FileOpStruct.pFrom) + 1 + 1) * sizeof(wchar_t)), // achtung! +1
				bytes_view(FileOpStruct.pTo, FileOpStruct.pTo ? (wcslen(FileOpStruct.pTo) + 1 + 1) * sizeof(wchar_t) : 0)); // achtung! +1

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
			Write(C_FUNCTION_SETOWNER, Object, Owner);
			return RetrieveLastErrorAndResult<bool>();
		});
}

HANDLE elevation::create_file(const string& Object, DWORD DesiredAccess, DWORD ShareMode, SECURITY_ATTRIBUTES* SecurityAttributes, DWORD CreationDistribution, DWORD FlagsAndAttributes, HANDLE TemplateFile)
{
	return execute(lng::MElevationRequiredOpen, Object,
		INVALID_HANDLE_VALUE,
		[&]
		{
			return os::fs::low::create_file(Object.c_str(), DesiredAccess, ShareMode, SecurityAttributes, CreationDistribution, FlagsAndAttributes, TemplateFile);
		},
		[&]
		{
			Write(C_FUNCTION_CREATEFILE, Object, DesiredAccess, ShareMode, CreationDistribution, FlagsAndAttributes);
			// BUGBUG: SecurityAttributes & TemplateFile ignored
			return ToPtr(RetrieveLastErrorAndResult<intptr_t>());
		});
}

bool elevation::set_file_encryption(const string& Object, bool Encrypt)
{
	return execute(Encrypt? lng::MElevationRequiredEncryptFile : lng::MElevationRequiredDecryptFile, Object,
		false,
		[&]
		{
			return os::fs::low::set_file_encryption(Object.c_str(), Encrypt);
		},
		[&]
		{
			Write(C_FUNCTION_SETENCRYPTION, Object, Encrypt);
			return RetrieveLastErrorAndResult<bool>();
		});
}

bool elevation::detach_virtual_disk(const string& Object, VIRTUAL_STORAGE_TYPE& VirtualStorageType)
{
	return execute(lng::MElevationRequiredCreate, Object,
		false,
		[&]
		{
			return os::fs::low::detach_virtual_disk(Object.c_str(), VirtualStorageType);
		},
		[&]
		{
			Write(C_FUNCTION_DETACHVIRTUALDISK, Object, VirtualStorageType);
			return RetrieveLastErrorAndResult<bool>();
		});
}

bool elevation::get_disk_free_space(const string& Object, unsigned long long* FreeBytesAvailableToCaller, unsigned long long* TotalNumberOfBytes, unsigned long long* TotalNumberOfFreeBytes)
{
	return execute(lng::MElevationRequiredList, Object,
		false,
		[&]
		{
			return os::fs::low::get_disk_free_space(Object.c_str(), FreeBytesAvailableToCaller, TotalNumberOfBytes, TotalNumberOfFreeBytes);
		},
		[&]
		{
			Write(C_FUNCTION_GETDISKFREESPACE, Object);
			const auto Result = RetrieveLastErrorAndResult<bool>();
			if (Result)
			{
				const auto& ReadAndAssign = [this](auto* Destination)
				{
					unsigned long long Value;
					Read(Value);
					if (Destination)
						*Destination = Value;
				};

				ReadAndAssign(FreeBytesAvailableToCaller);
				ReadAndAssign(TotalNumberOfBytes);
				ReadAndAssign(TotalNumberOfFreeBytes);
			}
			return Result;
		});
}

elevation::suppress::suppress():
	m_owner(Global? &instance() : nullptr)
{
	if (m_owner)
		++m_owner->m_Suppressions;
}

elevation::suppress::~suppress()
{
	if (m_owner)
		--m_owner->m_Suppressions;
}


bool ElevationRequired(ELEVATION_MODE Mode, bool UseNtStatus)
{
	if (!Global || !Global->Opt || !(Global->Opt->ElevationMode & Mode))
		return false;

	if(UseNtStatus && imports.RtlGetLastNtStatus)
	{
		const auto LastNtStatus = os::GetLastNtStatus();
		return LastNtStatus == STATUS_ACCESS_DENIED || LastNtStatus == STATUS_PRIVILEGE_NOT_HELD;
	}

	// RtlGetLastNtStatus not implemented in w2k.
	const auto LastWin32Error = GetLastError();
	return LastWin32Error == ERROR_ACCESS_DENIED || LastWin32Error == ERROR_PRIVILEGE_NOT_HELD;
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

		const auto PipeName = concat(L"\\\\.\\pipe\\"sv, guid);
		WaitNamedPipe(PipeName.c_str(), NMPWAIT_WAIT_FOREVER);
		m_Pipe.reset(os::fs::low::create_file(PipeName.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr));
		if (!m_Pipe)
			return GetLastError();

		{
			// basic security checks
			ULONG ServerProcessId;
			if (imports.GetNamedPipeServerProcessId && (!imports.GetNamedPipeServerProcessId(m_Pipe.native_handle(), &ServerProcessId) || ServerProcessId != PID))
				return GetLastError();

			const auto ParentProcess = os::handle(OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, PID));
			if (!ParentProcess)
				return GetLastError();

			string ParentProcessFileName;
			if (!os::fs::GetModuleFileName(ParentProcess.native_handle(), nullptr, ParentProcessFileName))
				return GetLastError();

			string CurrentProcessFileName;
			if (!os::fs::GetModuleFileName(GetCurrentProcess(), nullptr, CurrentProcessFileName))
				return GetLastError();

			if (!equal_icase(CurrentProcessFileName, ParentProcessFileName))
				return DNS_ERROR_INVALID_NAME;
		}

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

	void Write(const void* Data, size_t DataSize) const
	{
		if (!pipe::Write(m_Pipe, Data, DataSize))
			throw MAKE_FAR_EXCEPTION(L"Pipe write error"sv);
	}

	template<typename T>
	T Read() const
	{
		T Data;
		if (!pipe::Read(m_Pipe, Data))
			throw MAKE_FAR_EXCEPTION(L"Pipe read error"sv);
		return Data;
	}

	static void Write() {}

	template<typename T, typename... args>
	void Write(const T& Data, args&&... Args) const
	{
		if (!pipe::Write(m_Pipe, Data))
			throw MAKE_FAR_EXCEPTION(L"Pipe write error"sv);
		Write(FWD(Args)...);
	}

	void ExitHandler() const
	{
		m_Active = false;
	}

	void CreateDirectoryHandler() const
	{
		const auto TemplateObject = Read<string>();
		const auto Object = Read<string>();
		// BUGBUG, SecurityAttributes ignored

		const auto Result = os::fs::low::create_directory(TemplateObject.c_str(), Object.c_str(), nullptr);

		Write(error_state::fetch(), Result);
	}

	void RemoveDirectoryHandler() const
	{
		const auto Object = Read<string>();

		const auto Result = os::fs::low::remove_directory(Object.c_str());

		Write(error_state::fetch(), Result);
	}

	void DeleteFileHandler() const
	{
		const auto Object = Read<string>();

		const auto Result = os::fs::low::delete_file(Object.c_str());

		Write(error_state::fetch(), Result);
	}

	void CopyFileHandler() const
	{
		const auto From = Read<string>();
		const auto To = Read<string>();
		const auto UserCopyProgressRoutine = Read<intptr_t>();
		const auto Data = Read<intptr_t>();
		const auto Flags = Read<DWORD>();
		// BUGBUG: Cancel ignored

		callback_param Param{ this, reinterpret_cast<void*>(Data) };
		const auto Result = os::fs::low::copy_file(From.c_str(), To.c_str(), UserCopyProgressRoutine? CopyProgressRoutineWrapper : nullptr, &Param, nullptr, Flags);

		Write(0 /* not CallbackMagic */, error_state::fetch(), Result);

		RethrowIfNeeded(Param.ExceptionPtr);
	}

	void MoveFileHandler() const
	{
		const auto From = Read<string>();
		const auto To = Read<string>();
		const auto Flags = Read<DWORD>();

		const auto Result = os::fs::low::move_file(From.c_str(), To.c_str(), Flags);

		Write(error_state::fetch(), Result);
	}

	void GetFileAttributesHandler() const
	{
		const auto Object = Read<string>();

		const auto Result = os::fs::low::get_file_attributes(Object.c_str());

		Write(error_state::fetch(), Result);
	}

	void SetFileAttributesHandler() const
	{
		const auto Object = Read<string>();
		const auto Attributes = Read<DWORD>();

		const auto Result = os::fs::low::set_file_attributes(Object.c_str(), Attributes);

		Write(error_state::fetch(), Result);
	}

	void CreateHardLinkHandler() const
	{
		const auto Object = Read<string>();
		const auto Target = Read<string>();
		// BUGBUG: SecurityAttributes ignored.

		const auto Result = os::fs::low::create_hard_link(Object.c_str(), Target.c_str(), nullptr);

		Write(error_state::fetch(), Result);
	}

	void CreateSymbolicLinkHandler() const
	{
		const auto Object = Read<string>();
		const auto Target = Read<string>();
		const auto Flags = Read<DWORD>();

		const auto Result = os::fs::CreateSymbolicLinkInternal(Object, Target, Flags);

		Write(error_state::fetch(), Result);
	}

	void MoveToRecycleBinHandler() const
	{
		auto Struct = Read<SHFILEOPSTRUCT>();
		const auto From = Read<string>();
		const auto To = Read<string>();

		Struct.pFrom = From.c_str();
		Struct.pTo = To.c_str();

		const auto Result = SHFileOperation(&Struct);

		Write(Struct.fAnyOperationsAborted, Result);
	}

	void SetOwnerHandler() const
	{
		const auto Object = Read<string>();
		const auto Owner = Read<string>();

		const auto Result = SetOwnerInternal(Object, Owner);

		Write(error_state::fetch(), Result);
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
		if (const auto Handle = os::fs::create_file(Object, DesiredAccess, ShareMode, nullptr, CreationDistribution, FlagsAndAttributes, nullptr))
		{
			if (const auto ParentProcess = os::handle(OpenProcess(PROCESS_DUP_HANDLE, FALSE, m_ParentPid)))
			{
				DuplicateHandle(GetCurrentProcess(), Handle.native_handle(), ParentProcess.native_handle(), &Duplicate, 0, FALSE, DUPLICATE_SAME_ACCESS);
			}
		}

		Write(error_state::fetch(), reinterpret_cast<intptr_t>(Duplicate));
	}

	void SetEncryptionHandler() const
	{
		const auto Object = Read<string>();
		const auto Encrypt = Read<bool>();

		const auto Result = os::fs::low::set_file_encryption(Object.c_str(), Encrypt);

		Write(error_state::fetch(), Result);
	}

	void DetachVirtualDiskHandler() const
	{
		const auto Object = Read<string>();
		auto VirtualStorageType = Read<VIRTUAL_STORAGE_TYPE>();

		const auto Result = os::fs::low::detach_virtual_disk(Object.c_str(), VirtualStorageType);

		Write(error_state::fetch(), Result);
	}

	void GetDiskFreeSpaceHandler() const
	{
		const auto Object = Read<string>();

		unsigned long long FreeBytesAvailableToCaller, TotalNumberOfBytes, TotalNumberOfFreeBytes;
		const auto Result = os::fs::low::get_disk_free_space(Object.c_str(), &FreeBytesAvailableToCaller, &TotalNumberOfBytes, &TotalNumberOfFreeBytes);

		Write(error_state::fetch(), Result);

		if(Result)
		{
			Write(FreeBytesAvailableToCaller, TotalNumberOfBytes, TotalNumberOfFreeBytes);
		}
	}

	static DWORD CALLBACK CopyProgressRoutineWrapper(LARGE_INTEGER TotalFileSize, LARGE_INTEGER TotalBytesTransferred, LARGE_INTEGER StreamSize, LARGE_INTEGER StreamBytesTransferred, DWORD StreamNumber, DWORD CallbackReason, HANDLE SourceFile,HANDLE DestinationFile, LPVOID Data)
	{
		const auto Param = reinterpret_cast<callback_param*>(Data);
		try
		{
			const auto Context = Param->Owner;

			Context->Write(
				CallbackMagic,
				TotalFileSize,
				TotalBytesTransferred,
				StreamSize,
				StreamBytesTransferred,
				StreamNumber,
				CallbackReason,
				reinterpret_cast<intptr_t>(Param->UserData));
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
		CATCH_AND_SAVE_EXCEPTION_TO(Param->ExceptionPtr)

		return PROGRESS_CANCEL;
	}

	bool Process(int Command) const
	{
		assert(Command < C_COMMANDS_COUNT);

		static const decltype(&elevated::ExitHandler) Handlers[] =
		{
			&elevated::ExitHandler,
			&elevated::CreateDirectoryHandler,
			&elevated::RemoveDirectoryHandler,
			&elevated::DeleteFileHandler,
			&elevated::CopyFileHandler,
			&elevated::MoveFileHandler,
			&elevated::GetFileAttributesHandler,
			&elevated::SetFileAttributesHandler,
			&elevated::CreateHardLinkHandler,
			&elevated::CreateSymbolicLinkHandler,
			&elevated::MoveToRecycleBinHandler,
			&elevated::SetOwnerHandler,
			&elevated::CreateFileHandler,
			&elevated::SetEncryptionHandler,
			&elevated::DetachVirtualDiskHandler,
			&elevated::GetDiskFreeSpaceHandler,
		};

		static_assert(std::size(Handlers) == C_COMMANDS_COUNT);

		try
		{
			std::invoke(Handlers[Command], this);
			return m_Active;
		}
		catch (...)
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
	return equal(Argument, ElevationArgument);
}
