/*
platform.com.cpp
*/
/*
Copyright © 2021 Far Group
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

// BUGBUG
#include "platform.headers.hpp"

// Self:
#include "platform.com.hpp"

// Internal:
#include "imports.hpp"
#include "log.hpp"
#include "strmix.hpp"
#include "pathmix.hpp"

// Platform:
#include "platform.reg.hpp"

// Common:
#include "common/string_utils.hpp"

// External:

//----------------------------------------------------------------------------

namespace os::com
{
	static bool initialize_impl(mode const Mode)
	{
		if (const auto Result = CoInitializeEx({}, COINIT_DISABLE_OLE1DDE | (Mode == mode::sta? COINIT_APARTMENTTHREADED : COINIT_MULTITHREADED)); FAILED(Result))
		{
			LOG(Result == RPC_E_CHANGED_MODE? logging::level::warning : logging::level::error, L"CoInitializeEx(): {}"sv, format_error(Result));
			return false;
		}

		return true;
	}

	initialize::initialize(mode const Mode):
		m_Initialised(initialize_impl(Mode))
	{
	}

	initialize::~initialize()
	{
		if (m_Initialised)
			CoUninitialize();
	}

	void detail::memory_releaser::operator()(const void* Object) const
	{
		CoTaskMemFree(const_cast<void*>(Object));
	}

	detail::check_result::check_result(string_view CallableName, source_location const& Location):
		m_CallableName(CallableName),
		m_Location(Location)
	{
	}

	HRESULT detail::check_result::operator%(HRESULT const Result) const
	{
		if (FAILED(Result))
			throw exception(Result, m_CallableName, m_Location);

		return Result;
	}

	string get_shell_name(string_view Path)
	{
		// Q: Why not SHCreateItemFromParsingName + IShellItem::GetDisplayName?
		// A: Not available in WinXP.

		// Q: Why not SHParseDisplayName + SHCreateShellItem + IShellItem::GetDisplayName then?
		// A: Not available in Win2k.

		try
		{
			SCOPED_ACTION(initialize)(mode::sta);

			ptr<IShellFolder> ShellFolder;
			COM_INVOKE(SHGetDesktopFolder)(&ptr_setter(ShellFolder));

			memory<PIDLIST_RELATIVE> IdList;
			null_terminated const C_Path(Path);
			COM_INVOKE(ShellFolder->ParseDisplayName)({}, {}, UNSAFE_CSTR(C_Path), {}, &ptr_setter(IdList), {});

			STRRET StrRet;
			COM_INVOKE(ShellFolder->GetDisplayNameOf)(IdList.get(), SHGDN_FOREDITING, &StrRet);

			if (StrRet.uType != STRRET_WSTR)
			{
				LOGWARNING(L"StrRet.uType = {}"sv, StrRet.uType);
				return {};
			}

			const memory<wchar_t*> Str(StrRet.pOleStr);
			return Str.get();
		}
		catch (exception const& e)
		{
			LOGWARNING(L"{}"sv, e);
			return {};
		}
	}

	static bool is_proper_progid(string_view const ProgID)
	{
		return !ProgID.empty() && reg::key::classes_root.open(ProgID, KEY_QUERY_VALUE);
	}

	static string get_shell_type(string_view const FileName)
	{
		auto [Name, Ext] = name_ext(FileName);
		if (Ext.empty())
			Ext = L"."sv;

		if (imports.SHCreateAssociationRegistration)
		{
			try
			{
				ptr<IApplicationAssociationRegistration> AAR;
				COM_INVOKE(imports.SHCreateAssociationRegistration)(IID_IApplicationAssociationRegistration, IID_PPV_ARGS_Helper(&ptr_setter(AAR)));

				memory<wchar_t*> Association;
				COM_INVOKE(AAR->QueryCurrentDefault)(null_terminated(Ext).c_str(), AT_FILEEXTENSION, AL_EFFECTIVE, &ptr_setter(Association));

				return Association.get();
			}
			catch (exception const& e)
			{
				// This is informational, debug will do fine
				LOGDEBUG(L"{}"sv, e);
			}
		}

		try
		{
			if (const auto UserKey = reg::key::current_user.open(concat(L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\"sv, Ext), KEY_QUERY_VALUE))
			{
				if (const auto ProgId = UserKey->get_string(L"ProgId"sv); ProgId && is_proper_progid(*ProgId))
					return *ProgId;

				if (const auto Application = UserKey->get_string(L"Application"sv))
					if (const auto ProgId = L"Applications\\"sv + *Application; is_proper_progid(ProgId))
						return ProgId;
			}

			if (const auto CRKey = reg::key::classes_root.open(Ext, KEY_QUERY_VALUE))
			{
				if (const auto ProgId = CRKey->get_string({}); ProgId && is_proper_progid(*ProgId))
					return *ProgId;
			}
		}
		catch (std::exception const& e)
		{
			// This is informational, debug will do fine
			LOGDEBUG(L"{}"sv, e);
		}

		return {};
	}

	string get_shell_filetype_description(string_view const FileName)
	{
		const auto Type = get_shell_type(FileName);
		if (Type.empty())
			return {};

		if (const auto Description = reg::key::classes_root.get_string(Type, {}))
			return *Description;

		return {};
	}

	ptr<IFileIsInUse> create_file_is_in_use(const string& File)
	{
		try
		{
			ptr<IRunningObjectTable> RunningObjectTable;
			COM_INVOKE(GetRunningObjectTable)(0, &ptr_setter(RunningObjectTable));

			ptr<IMoniker> FileMoniker;
			COM_INVOKE(CreateFileMoniker)(File.c_str(), &ptr_setter(FileMoniker));

			ptr<IEnumMoniker> EnumMoniker;
			COM_INVOKE(RunningObjectTable->EnumRunning)(&ptr_setter(EnumMoniker));

			for (;;)
			{
				try
				{
					ptr<IMoniker> Moniker;
					if (COM_INVOKE(EnumMoniker->Next)(1, &ptr_setter(Moniker), {}) == S_FALSE)
						return {};

					DWORD Type;
					COM_INVOKE(Moniker->IsSystemMoniker)(&Type);

					if (Type != MKSYS_FILEMONIKER)
						continue;

					ptr<IMoniker> PrefixMoniker;
					if (const auto Result = FileMoniker->CommonPrefixWith(Moniker.get(), &ptr_setter(PrefixMoniker)); FAILED(Result))
					{
						// MSDN mentions MK_S_NOPREFIX, but there's no such thing.
						// Actually it's MK_E_NOPREFIX, and it's the most common case,
						// so it's better to handle it explicitly and don't spam the log with exceptions
						if (Result == MK_E_NOPREFIX)
							continue;

						throw exception(Result, WIDE_SV_LITERAL(FileMoniker->CommonPrefixWith));
					}

					if (COM_INVOKE(FileMoniker->IsEqual)(PrefixMoniker.get()) == S_FALSE)
						continue;

					ptr<IUnknown> Unknown;
					if (COM_INVOKE(RunningObjectTable->GetObject)(Moniker.get(), &ptr_setter(Unknown)) == S_FALSE)
						continue;

					ptr<IFileIsInUse> FileIsInUse;
					COM_INVOKE(Unknown->QueryInterface)(IID_IFileIsInUse, IID_PPV_ARGS_Helper(&ptr_setter(FileIsInUse)));

					return FileIsInUse;
				}
				catch (exception const& e)
				{
					LOGWARNING(L"{}"sv, e);
					continue;
				}
			}
		}
		catch (exception const& e)
		{
			LOGWARNING(L"{}"sv, e);
			return {};
		}
	}

	static bool can_recycle_impl(string_view const Object)
	{
WARNING_PUSH()
WARNING_DISABLE_GCC("-Wnon-virtual-dtor")
		class FileOperationProgressSink final: public IFileOperationProgressSink
		{
		public:
			explicit FileOperationProgressSink(bool& CanRecycle):
				m_CanRecycle(&CanRecycle)
			{
			}

			// IUnknown
			IFACEMETHODIMP QueryInterface(REFIID InterfaceId, PVOID* Interface) override
			{
				if (InterfaceId == IID_IUnknown)
					*Interface = static_cast<IUnknown*>(static_cast<IFileOperationProgressSink*>(this));
				else if (InterfaceId == IID_IFileOperationProgressSink)
					*Interface = static_cast<IFileOperationProgressSink*>(this);
				else
				{
					*Interface = {};
					return E_NOINTERFACE;
				}

				AddRef();
				return S_OK;
			}

			IFACEMETHODIMP_(ULONG) AddRef() override { return 1; }
			IFACEMETHODIMP_(ULONG) Release() override { return 1; }

			// IFileOperationProgressSink
			IFACEMETHODIMP StartOperations() override { return S_OK; }
			IFACEMETHODIMP FinishOperations(HRESULT hrResult) override { return S_OK; }
			IFACEMETHODIMP PreRenameItem(DWORD, IShellItem*, PCWSTR) override { return S_OK; }
			IFACEMETHODIMP PostRenameItem(DWORD, IShellItem*, PCWSTR, HRESULT, IShellItem*) override { return S_OK; }
			IFACEMETHODIMP PreMoveItem(DWORD, IShellItem*, IShellItem*, PCWSTR) override { return S_OK; }
			IFACEMETHODIMP PostMoveItem(DWORD, IShellItem*, IShellItem*, PCWSTR, HRESULT, IShellItem*) override { return S_OK; }
			IFACEMETHODIMP PreCopyItem(DWORD, IShellItem*, IShellItem*, PCWSTR) override { return S_OK; }
			IFACEMETHODIMP PostCopyItem(DWORD, IShellItem*, IShellItem*, PCWSTR, HRESULT, IShellItem*) override { return S_OK; }

			IFACEMETHODIMP PreDeleteItem(DWORD Flags, IShellItem*) override
			{
				*m_CanRecycle = flags::check_one(Flags, TSF_DELETE_RECYCLE_IF_POSSIBLE);
				return E_ABORT;
			}

			IFACEMETHODIMP PostDeleteItem(DWORD, IShellItem*, HRESULT, IShellItem*) override { return S_OK; }
			IFACEMETHODIMP PreNewItem(DWORD, IShellItem*, PCWSTR) override { return S_OK; }
			IFACEMETHODIMP PostNewItem(DWORD, IShellItem*, PCWSTR, PCWSTR, DWORD, HRESULT, IShellItem*) override { return S_OK; }
			IFACEMETHODIMP UpdateProgress(UINT, UINT) override { return S_OK; }
			IFACEMETHODIMP ResetTimer() override { return S_OK; }
			IFACEMETHODIMP PauseTimer() override { return S_OK; }
			IFACEMETHODIMP ResumeTimer() override { return S_OK; }

		private:
			bool* m_CanRecycle;
		};
WARNING_POP()

		// Unfortunately, there seems to be no way to just query if an item can be recycled (shame on you, Microsoft), so we have to get creative:
		// IFileOperation has pre- and post-callbacks for all the actions it performs, including delete.
		// The pre-delete callback receives a set of flags, one of which indicates whether the shell considers the item recyclable or not.
		// So we call delete on the item in question, check the flag in the callback and then immediately abort the operation.
		// This is ludicrous, but hey, as long as it works.

		SCOPED_ACTION(initialize)(mode::sta);

		ptr<IFileOperation> FileOperation;
		COM_INVOKE(CoCreateInstance)(CLSID_FileOperation, nullptr, CLSCTX_INPROC_SERVER, IID_IFileOperation, IID_PPV_ARGS_Helper(&ptr_setter(FileOperation)));

		COM_INVOKE(FileOperation->SetOperationFlags)(
			FOF_ALLOWUNDO |    // We want to know if the item can be recycled
			FOF_NORECURSION |  // Since we're not actually deleting anything, probing only the top level item is fine (and way faster)
			FOF_NO_UI          // Obviously, we don't want any UI
		);

		ptr<IShellItem> Item;
		COM_INVOKE(imports.SHCreateItemFromParsingName)(null_terminated(Object).c_str(), nullptr, IID_IShellItem, IID_PPV_ARGS_Helper(&ptr_setter(Item)));

		bool CanRecycle{};
		FileOperationProgressSink Sink(CanRecycle);

		COM_INVOKE(FileOperation->DeleteItem)(Item.get(), &Sink);

		if (const auto Result = FileOperation->PerformOperations(); Result != E_ABORT)
			throw exception(Result, WIDE_SV_LITERAL(FileOperation->PerformOperations)); // We should have aborted the operation in the callback, so it must fail

		return CanRecycle;
	}

	std::optional<bool> can_recycle(string_view const Object)
	{
		// Both SHCreateItemFromParsingName and IFileOperation are Vista+, no point in trying if it's not there.
		if (!imports.SHCreateItemFromParsingName)
			return {};

		try
		{
			return can_recycle_impl(Object);
		}
		catch (exception const& e)
		{
			LOGWARNING(L"can_recycle: {}"sv, e);
			return {};
		}
	}
}
