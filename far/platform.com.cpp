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
#include "exception.hpp"
#include "imports.hpp"
#include "log.hpp"
#include "strmix.hpp"

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
			LOGWARNING(L"CoInitializeEx(): {}"sv, format_error(Result));
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

	string get_shell_name(string_view Path)
	{
		// Q: Why not SHCreateItemFromParsingName + IShellItem::GetDisplayName?
		// A: Not available in WinXP.

		// Q: Why not SHParseDisplayName + SHCreateShellItem + IShellItem::GetDisplayName then?
		// A: Not available in Win2k.

		SCOPED_ACTION(initialize);

		ptr<IShellFolder> ShellFolder;
		if (const auto Result = SHGetDesktopFolder(&ptr_setter(ShellFolder)); FAILED(Result))
		{
			LOGWARNING(L"SHGetDesktopFolder(): {}"sv, format_error(Result));
			return {};
		}

		memory<PIDLIST_RELATIVE> IdList;
		null_terminated const C_Path(Path);
		if (const auto Result = ShellFolder->ParseDisplayName({}, {}, UNSAFE_CSTR(C_Path), {}, &ptr_setter(IdList), {}); FAILED(Result))
		{
			LOGWARNING(L"ParseDisplayName(): {}"sv, format_error(Result));
			return {};
		}

		STRRET StrRet;
		if (const auto Result = ShellFolder->GetDisplayNameOf(IdList.get(), SHGDN_FOREDITING, &StrRet); FAILED(Result))
		{
			LOGWARNING(L"GetDisplayNameOf(): {}"sv, format_error(Result));
			return {};
		}

		if (StrRet.uType != STRRET_WSTR)
		{
			LOGWARNING(L"StrRet.uType = {}"sv, StrRet.uType);
			return {};
		}

		const memory<wchar_t*> Str(StrRet.pOleStr);
		return Str.get();
	}

	static bool is_proper_progid(string_view const ProgID)
	{
		return !ProgID.empty() && reg::key::open(reg::key::classes_root, ProgID, KEY_QUERY_VALUE);
	}

	static bool get_shell_type(const string_view Ext, string& strType)
	{
		if (imports.SHCreateAssociationRegistration)
		{
			ptr<IApplicationAssociationRegistration> AAR;
			if (const auto Result = imports.SHCreateAssociationRegistration(IID_IApplicationAssociationRegistration, IID_PPV_ARGS_Helper(&ptr_setter(AAR))); FAILED(Result))
			{
				LOGWARNING(L"cSHCreateAssociationRegistration(): {}"sv, format_error(Result));
				return false;
			}

			memory<wchar_t*> Association;
			if (const auto Result = AAR->QueryCurrentDefault(null_terminated(Ext).c_str(), AT_FILEEXTENSION, AL_EFFECTIVE, &ptr_setter(Association)); FAILED(Result))
			{
				LOGDEBUG(L"QueryCurrentDefault({}): {}"sv, Ext, format_error(Result));
				return false;
			}

			strType = Association.get();
			return true;
		}

		if (const auto UserKey = reg::key::open(reg::key::current_user, concat(L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\"sv, Ext), KEY_QUERY_VALUE))
		{
			if (string Value; UserKey.get(L"ProgId"sv, Value) && is_proper_progid(Value))
			{
				strType = std::move(Value);
				return true;
			}

			if (string Value; UserKey.get(L"Application"sv, Value))
			{
				if (auto ProgId = L"Applications\\"sv + Value; is_proper_progid(ProgId))
				{
					strType = std::move(ProgId);
					return true;
				}
			}
		}

		if (const auto CRKey = reg::key::open(reg::key::classes_root, Ext, KEY_QUERY_VALUE))
		{
			if (string Value; CRKey.get({}, Value) && is_proper_progid(Value))
			{
				strType = std::move(Value);
				return true;
			}
		}

		return false;
	}

	string get_shell_filetype_description(string_view const FileName)
	{
		const auto pos = FileName.rfind(L'.');
		if (pos == string::npos)
			return {};

		string Type;
		if (!get_shell_type(FileName.substr(pos), Type))
			return {};

		string Description;
		if (!reg::key::classes_root.get(Type, {}, Description))
		{
			LOGWARNING(L"classes_root.get({}): {}"sv, Type, last_error());
			return {};
		}

		return Description;
	}

	ptr<IFileIsInUse> create_file_is_in_use(const string& File)
	{
		ptr<IRunningObjectTable> RunningObjectTable;
		if (const auto Result = GetRunningObjectTable(0, &ptr_setter(RunningObjectTable)); FAILED(Result))
		{
			LOGWARNING(L"GetRunningObjectTable(): {}"sv, format_error(Result));
			return {};
		}

		ptr<IMoniker> FileMoniker;
		if (const auto Result = CreateFileMoniker(File.c_str(), &ptr_setter(FileMoniker)); FAILED(Result))
		{
			LOGWARNING(L"CreateFileMoniker({}): {}"sv, File, format_error(Result));
			return {};
		}

		ptr<IEnumMoniker> EnumMoniker;
		if (const auto Result = RunningObjectTable->EnumRunning(&ptr_setter(EnumMoniker)); FAILED(Result))
		{
			LOGWARNING(L"EnumRunning(): {}"sv, format_error(Result));
			return {};
		}

		for (;;)
		{
			ptr<IMoniker> Moniker;
			if (EnumMoniker->Next(1, &ptr_setter(Moniker), {}) == S_FALSE)
				return {};

			DWORD Type;
			if (const auto Result = Moniker->IsSystemMoniker(&Type); FAILED(Result))
			{
				LOGWARNING(L"IsSystemMoniker(): {}"sv, format_error(Result));
				continue;
			}

			if (Type != MKSYS_FILEMONIKER)
				continue;

			ptr<IMoniker> PrefixMoniker;
			if (const auto Result = FileMoniker->CommonPrefixWith(Moniker.get(), &ptr_setter(PrefixMoniker)); FAILED(Result))
			{
				LOGWARNING(L"CommonPrefixWith(): {}"sv, format_error(Result));
				continue;
			}

			if (FileMoniker->IsEqual(PrefixMoniker.get()) == S_FALSE)
				continue;

			ptr<IUnknown> Unknown;
			if (RunningObjectTable->GetObject(Moniker.get(), &ptr_setter(Unknown)) == S_FALSE)
				continue;

			ptr<IFileIsInUse> FileIsInUse;
			if (const auto Result = Unknown->QueryInterface(IID_IFileIsInUse, IID_PPV_ARGS_Helper(&ptr_setter(FileIsInUse))); FAILED(Result))
			{
				LOGWARNING(L"QueryInterface(): {}"sv, format_error(Result));
				continue;
			}

			return FileIsInUse;
		}
	}
}
