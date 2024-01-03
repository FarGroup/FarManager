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
#include "tracer.hpp"

// Platform:
#include "platform.debug.hpp"

// Common:
#include "common/source_location.hpp"
#include "common/string_utils.hpp"

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

string source_location_to_string(source_location const& Location)
{
	return far::format(
		L"{}, {}({})"sv,
		encoding::utf8::get_chars(Location.function_name()),
		encoding::utf8::get_chars(Location.file_name()),
		Location.line());
}

namespace detail
{
	string far_base_exception::to_string() const
	{
		return any()?
			far::format(L"far_base_exception: {}, Error: {}"sv, full_message(), error_state::to_string()) :
			far::format(L"far_base_exception: {}"sv, full_message());
	}

	far_base_exception::far_base_exception(bool const CaptureErrors, string_view const Message, source_location const& Location):
		error_state_ex(CaptureErrors? os::last_error(): os::error_state{}, Message, CaptureErrors? errno : 0),
		m_Location(Location),
		m_FullMessage(far::format(L"{} ({})"sv, Message, source_location_to_string(m_Location)))
	{
		LOGTRACE(L"{}"sv, *this);
	}

	std::string far_std_exception::convert_message(string_view const Message)
	{
		return encoding::utf8::get_bytes(Message);
	}

	break_into_debugger::break_into_debugger()
	{
		os::debug::breakpoint_if_debugging();
	}
}

string error_state_ex::ErrnoStr() const
{
	return os::format_errno(Errno);
}

string error_state_ex::system_error() const
{
	constexpr auto UseNtMessages = false;
	return UseNtMessages? NtErrorStr() : Win32ErrorStr();
}

static auto with_exception_stacktrace(string_view const Str)
{
	string Result;

	tracer.exception_stacktrace({}, [&](string_view const Line)
	{
		append(Result, Line, L'\n');
	});

	return Result.empty()? string(Str) : concat(Str, L"\n\n"sv, Result);
}

string error_state_ex::to_string() const
{
	if (any())
	{
		auto Str = error_state::to_string();
		if (Errno)
			Str = concat(ErrnoStr(), L", "sv, Str);

		return with_exception_stacktrace(far::format(L"Message: {}, Error: {}"sv, What, Str));
	}

	return with_exception_stacktrace(far::format(L"Message: {}"sv, What));
}

string formattable<std::exception>::to_string(std::exception const& e)
{
	return with_exception_stacktrace(far::format(L"std::exception: {}"sv, encoding::utf8::get_chars(e.what())));
}

string unknown_exception_t::to_string()
{
	return with_exception_stacktrace(L"Unknown exception"sv);
}
