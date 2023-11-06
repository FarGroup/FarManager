/*
imports.cpp

импортируемые функции
*/
/*
Copyright © 1996 Eugene Roshal
Copyright © 2000 Far Group
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
#include "imports.hpp"

// Internal:
#include "log.hpp"
#include "encoding.hpp"

// Platform:
#include "platform.hpp"

// Common:
#include "common/scope_exit.hpp"

// External:

//----------------------------------------------------------------------------

namespace imports_detail
{
	void* imports::get_pointer_impl(os::rtdl::module const& Module, const char* Name)
	{
		// imports is the lowest level. Everything else depends on it, including logging.
		// Without a suppression we might fall into a recursive initialisation.
		SCOPED_ACTION(logging::suppressor);

		return Module?
			Module.GetProcAddress<void*>(Name) :
			nullptr;
	}

	void imports::log_missing_import(const os::rtdl::module& Module, std::string_view const Name)
	{
		static const os::rtdl::module* CurrentModule{};
		static std::string_view CurrentName;

		if (CurrentModule == &Module && CurrentName.data() == Name.data())
			return;

		CurrentModule = &Module;
		CurrentName = Name;

		SCOPE_EXIT
		{
			CurrentModule = {};
			CurrentName = {};
		};

		const auto LastError = os::last_error();
		LOGWARNING(L"{}::{}: {}"sv, Module.name(), encoding::utf8::get_chars(Name), LastError);
	}

	void imports::log_usage(std::string_view const Name)
	{
		LOGWARNING(L"Stub call to {}"sv, encoding::ansi::get_chars(Name));
	}

	void imports::do_le()  { SetLastError(ERROR_CALL_NOT_IMPLEMENTED); }
	void imports::do_nop() {}

	NTSTATUS       imports::ret_nt()      { return STATUS_NOT_IMPLEMENTED; }
	DWORD          imports::ret_le()      { return ERROR_CALL_NOT_IMPLEMENTED; }
	HANDLE         imports::ret_handle()  { return INVALID_HANDLE_VALUE; }
	HRESULT        imports::ret_hr()      { return E_NOTIMPL; }
	NET_API_STATUS imports::ret_net()     { return NERR_InvalidAPI; }
	std::nullptr_t imports::ret_nullptr() { return {}; }
	int            imports::ret_zero()    { return 0; }
	bool           imports::ret_false()   { return false; }
	void           imports::ret_void()    {}
}

NIFTY_DEFINE(imports_detail::imports, imports);
