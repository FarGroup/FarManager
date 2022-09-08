/*
exception.cpp
*/
/*
Copyright © 2016 Far Group
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
#include "exception.hpp"

// Internal:
#include "imports.hpp"
#include "encoding.hpp"
#include "log.hpp"

// Platform:
#include "platform.debug.hpp"

// Common:
#include "common/string_utils.hpp"
#include "common/view/where.hpp"

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

error_state last_error()
{
	return
	{
		errno,
		GetLastError(),
		os::get_last_nt_status(),
	};
}

string error_state::ErrnoStr() const
{
	return os::format_errno(Errno);
}

string error_state::Win32ErrorStr() const
{
	return os::format_error(Win32Error);
}

string error_state::NtErrorStr() const
{
	return os::format_ntstatus(NtError);
}

std::array<string, 3> error_state::format_errors() const
{
	return
	{
		ErrnoStr(),
		Win32ErrorStr(),
		NtErrorStr()
	};
}

string error_state::to_string() const
{
	const auto StrErrno      = Errno?      format(FSTR(L"errno: {}"sv),     ErrnoStr())      : L""s;
	const auto StrWin32Error = Win32Error? format(FSTR(L"LastError: {}"sv), Win32ErrorStr()) : L""s;
	const auto StrNtError    = NtError?    format(FSTR(L"NTSTATUS: {}"sv),  NtErrorStr())    : L""s;

	string_view const Errors[]
	{
		StrErrno,
		StrWin32Error,
		StrNtError,
	};

	return join(L", "sv, where(Errors, [](string_view const Str){ return !Str.empty(); }));
}

namespace detail
{
	string far_base_exception::to_string() const
	{
		return any()?
			format(FSTR(L"far_base_exception: {}, Error: {}"sv), full_message(), error_state::to_string()) :
			format(FSTR(L"far_base_exception: {}"sv), full_message());
	}

	far_base_exception::far_base_exception(bool const CaptureErrors, string_view const Message, std::string_view const Function, std::string_view const File, int const Line):
		error_state_ex(CaptureErrors? last_error(): error_state{}, Message),
		m_Function(Function),
		m_Location(format(FSTR(L"{}({})"sv), encoding::utf8::get_chars(File), Line)),
		m_FullMessage(format(FSTR(L"{} ({}, {})"sv), Message, encoding::utf8::get_chars(m_Function), m_Location))
	{
		LOGTRACE(L"{}"sv, *this);
	}

	std::string far_std_exception::convert_message() const
	{
		return encoding::utf8::get_bytes(full_message());
	}

	break_into_debugger::break_into_debugger()
	{
		os::debug::breakpoint(false);
	}
}

string error_state_ex::system_error() const
{
	constexpr auto UseNtMessages = false;
	return UseNtMessages? NtErrorStr() : Win32ErrorStr();
}

string error_state_ex::to_string() const
{
	return any()?
		format(FSTR(L"Message: {}, Error: {}"sv), What, error_state::to_string()) :
		format(FSTR(L"Message: {}"sv), What);
}

string formattable<std::exception>::to_string(std::exception const& e)
{
	return ::format(FSTR(L"std::exception: {}"sv), encoding::utf8::get_chars(e.what()));
}
