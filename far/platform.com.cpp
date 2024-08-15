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
	static bool initialize_impl()
	{
		if (const auto Result = CoInitializeEx({}, COINIT_DISABLE_OLE1DDE | COINIT_MULTITHREADED); FAILED(Result))
		{
			LOG(Result == RPC_E_CHANGED_MODE? logging::level::warning : logging::level::error, L"CoInitializeEx(): {}"sv, format_error(Result));
			return false;
		}

		return true;
	}

	initialize::initialize():
		m_Initialised(initialize_impl())
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

	void invoke(function_ref<HRESULT()> const Callable, string_view CallableName, source_location const& Location)
	{
		if (const auto Result = Callable(); FAILED(Result))
			throw exception(Result, CallableName, Location);
	}

	string get_shell_name(string_view Path)
	{
		// Q: Why not SHCreateItemFromParsingName + IShellItem::GetDisplayName?
		// A: Not available in WinXP.

		// Q: Why not SHParseDisplayName + SHCreateShellItem + IShellItem::GetDisplayName then?
		// A: Not available in Win2k.

		try
		{
			SCOPED_ACTION(initialize);

			ptr<IShellFolder> ShellFolder;
			COM_INVOKE(SHGetDesktopFolder, (&ptr_setter(ShellFolder)));

			memory<PIDLIST_RELATIVE> IdList;
			null_terminated const C_Path(Path);
			COM_INVOKE(ShellFolder->ParseDisplayName, ({}, {}, UNSAFE_CSTR(C_Path), {}, &ptr_setter(IdList), {}));

			STRRET StrRet;
			COM_INVOKE(ShellFolder->GetDisplayNameOf, (IdList.get(), SHGDN_FOREDITING, &StrRet));

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
				COM_INVOKE(imports.SHCreateAssociationRegistration, (IID_IApplicationAssociationRegistration, IID_PPV_ARGS_Helper(&ptr_setter(AAR))));

				memory<wchar_t*> Association;
				COM_INVOKE(AAR->QueryCurrentDefault, (null_terminated(Ext).c_str(), AT_FILEEXTENSION, AL_EFFECTIVE, &ptr_setter(Association)));

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
		catch (far_exception const& e)
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
			COM_INVOKE(GetRunningObjectTable, (0, &ptr_setter(RunningObjectTable)));

			ptr<IMoniker> FileMoniker;
			COM_INVOKE(CreateFileMoniker, (File.c_str(), &ptr_setter(FileMoniker)));

			ptr<IEnumMoniker> EnumMoniker;
			COM_INVOKE(RunningObjectTable->EnumRunning, (&ptr_setter(EnumMoniker)));

			for (;;)
			{
				try
				{
					ptr<IMoniker> Moniker;
					if (EnumMoniker->Next(1, &ptr_setter(Moniker), {}) == S_FALSE)
						return {};

					DWORD Type;
					COM_INVOKE(Moniker->IsSystemMoniker, (&Type));

					if (Type != MKSYS_FILEMONIKER)
						continue;

					ptr<IMoniker> PrefixMoniker;
					COM_INVOKE(FileMoniker->CommonPrefixWith, (Moniker.get(), &ptr_setter(PrefixMoniker)));

					if (FileMoniker->IsEqual(PrefixMoniker.get()) == S_FALSE)
						continue;

					ptr<IUnknown> Unknown;
					if (RunningObjectTable->GetObject(Moniker.get(), &ptr_setter(Unknown)) == S_FALSE)
						continue;

					ptr<IFileIsInUse> FileIsInUse;
					COM_INVOKE(Unknown->QueryInterface, (IID_IFileIsInUse, IID_PPV_ARGS_Helper(&ptr_setter(FileIsInUse))));

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
}
