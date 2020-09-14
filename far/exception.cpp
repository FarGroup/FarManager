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

// Self:
#include "exception.hpp"

// Internal:
#include "imports.hpp"
#include "encoding.hpp"
#include "tracer.hpp"

// Platform:

// Common:
#include "common/string_utils.hpp"

// External:
#include "format.hpp"

//----------------------------------------------------------------------------


error_state error_state::fetch()
{
	error_state State;
	State.Errno = errno;
	State.Win32Error = GetLastError();
	State.NtError = imports.RtlGetLastNtStatus();
	return State;
}

string error_state::ErrnoStr() const
{
	return _wcserror(Errno);
}

string error_state::Win32ErrorStr() const
{
	return os::GetErrorString(false, Win32Error);
}

string error_state::NtErrorStr() const
{
	return os::GetErrorString(true, NtError);
}

std::array<string, 3> error_state::format_errors() const
{
	return
	{
		os::format_system_error(Errno, ErrnoStr()),
		os::format_system_error(Win32Error, Win32ErrorStr()),
		os::format_system_error(NtError, NtErrorStr())
	};
}


namespace detail
{
	far_base_exception::far_base_exception(string_view const Message, const char* const Function, string_view const File, int const Line):
		error_state_ex(fetch(), Message),
		m_Function(Function),
		m_Location(format(FSTR(L"{0}:{1}"), File, Line)),
		m_FullMessage(format(FSTR(L"{0} (at {1}, {2})"), Message, encoding::utf8::get_chars(m_Function), m_Location))
	{
	}

	std::string far_std_exception::convert_message() const
	{
		return encoding::utf8::get_bytes(full_message());
	}

	attach_debugger::attach_debugger()
	{
		if (IsDebuggerPresent())
			DebugBreak();
	}

	exception_context::exception_context(DWORD const Code, const EXCEPTION_POINTERS& Pointers, os::handle&& ThreadHandle, DWORD const ThreadId) noexcept:
		m_Code(Code),
		m_Pointers(Pointers),
		m_ThreadHandle(std::move(ThreadHandle)),
		m_ThreadId(ThreadId)
	{
	}

	seh_exception_context::~seh_exception_context()
	{
		ResumeThread(thread_handle());
	}
}

string error_state_ex::format_error() const
{
	auto Str = What;
	if (!Str.empty())
		append(Str, L": "sv);

	constexpr auto UseNtMessages = false;

	return Str + os::format_system_error(
		UseNtMessages? NtError : Win32Error,
		UseNtMessages? NtErrorStr() : Win32ErrorStr());
}


far_wrapper_exception::far_wrapper_exception(const char* const Function, string_view const File, int const Line):
	far_exception(L"exception_ptr"sv, Function, File, Line),
	m_ThreadHandle(std::make_shared<os::handle>(os::OpenCurrentThread())),
	m_Stack(tracer::get({}, tracer::get_pointers(), m_ThreadHandle->native_handle()))
{
}

seh_exception::seh_exception(DWORD const Code, EXCEPTION_POINTERS& Pointers, os::handle&& ThreadHandle, DWORD const ThreadId):
	error_state_ex(fetch()),
	m_Context(std::make_shared<detail::seh_exception_context>(Code, Pointers, std::move(ThreadHandle), ThreadId))
{
}

std::exception_ptr wrap_currrent_exception(const char* const Function, string_view const File, int const Line)
{
	try
	{
		std::throw_with_nested(far_wrapper_exception(Function, File, Line));
	}
	catch (...)
	{
		return std::current_exception();
	}
}

void rethrow_if(std::exception_ptr& Ptr)
{
	if (Ptr)
		std::rethrow_exception(std::exchange(Ptr, {}));
}
