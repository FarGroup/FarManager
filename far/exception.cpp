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

// External:
#include "format.hpp"

//----------------------------------------------------------------------------


error_state error_state::fetch()
{
	error_state State;
	State.Errno = errno;
	State.Win32Error = GetLastError();
	State.NtError = imports.RtlGetLastNtStatus();
	State.m_Engaged = true;
	return State;
}

error_state::operator bool() const
{
	return m_Engaged;
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


namespace detail
{
	far_base_exception::far_base_exception(const char* const Function, const char* const File, int const Line, string_view const Message):
		m_Function(encoding::ansi::get_chars(Function)),
		m_Location(format(L"{0}:{1}"sv, encoding::ansi::get_chars(File), Line)),
		m_FullMessage(format(L"{0} (at {1}, {2})"sv, Message, m_Function, m_Location)),
		m_ErrorState(error_state::fetch(), Message)
	{
	}

	std::string far_std_exception::convert_message() const
	{
		return encoding::utf8::get_bytes(get_full_message());
	}

	attach_debugger::attach_debugger()
	{
		if (IsDebuggerPresent())
			DebugBreak();
	}

	exception_context::exception_context(DWORD const Code, const EXCEPTION_POINTERS& Pointers, os::handle&& ThreadHandle, DWORD const ThreadId):
		m_Code(Code),
		m_Pointers(Pointers),
		m_ThreadHandle(std::move(ThreadHandle)),
		m_ThreadId(ThreadId)
	{
	}

	seh_exception_context::~seh_exception_context()
	{
		if (m_ResumeThread)
			ResumeThread(thread_handle());
	}
}

far_wrapper_exception::far_wrapper_exception(const char* const Function, const char* const File, int const Line):
	far_exception(Function, File, Line, L"->"sv),
	m_ThreadHandle(std::make_shared<os::handle>(os::OpenCurrentThread())),
	m_Stack(tracer::get(tracer::get_pointers(), m_ThreadHandle->native_handle()))
{
}

seh_exception::seh_exception(DWORD const Code, EXCEPTION_POINTERS& Pointers, os::handle&& ThreadHandle, DWORD const ThreadId, bool const ResumeThread):
	m_Context(std::make_shared<detail::seh_exception_context>(Code, Pointers, std::move(ThreadHandle), ThreadId, ResumeThread))
{
}

std::exception_ptr wrap_currrent_exception()
{
	try
	{
		std::throw_with_nested(MAKE_EXCEPTION(far_wrapper_exception));
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
