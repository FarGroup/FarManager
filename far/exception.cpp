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

#include "exception.hpp"

#include "imports.hpp"
#include "encoding.hpp"
#include "tracer.hpp"

#include "format.hpp"

error_state error_state::fetch()
{
	error_state State;
	State.Win32Error = GetLastError();
	State.NtError = imports.RtlGetLastNtStatus();
	State.m_Engaged = true;
	return State;
}

bool error_state::engaged() const
{
	return m_Engaged;
}

detail::exception_impl::exception_impl(string_view const Message, const char* const Function, const char* const File, int const Line):
	m_FullMessage(
		format(
			L"{0} (at {1}, {2}:{3})",
			Message,
			encoding::ansi::get_chars(Function),
			encoding::ansi::get_chars(File),
			Line)),
	m_ErrorState(error_state::fetch(), Message)
{
}

far_exception::far_exception(string_view const Message, const char* const Function, const char* const File, int const Line):
	exception_impl(Message, Function, File, Line),
	std::runtime_error(encoding::utf8::get_bytes(get_full_message()))
{
}

far_exception::far_exception(string_view const Message, std::vector<string>&& Stack, const char* const Function, const char* const File, int const Line):
	exception_impl(Message, Function, File, Line),
	std::runtime_error(encoding::utf8::get_bytes(get_full_message())),
	m_Stack(std::move(Stack))
{
}

const std::vector<string>& far_exception::get_stack() const
{
	return m_Stack;
}


exception_context::exception_context(DWORD Code, const EXCEPTION_POINTERS* Pointers, bool ResumeThread):
	m_Code(Code),
	m_ExceptionRecord(),
	m_ContextRecord(),
	m_Pointers{ &m_ExceptionRecord, &m_ContextRecord },
	m_ThreadHandle(os::OpenCurrentThread()),
	m_ThreadId(GetCurrentThreadId()),
	m_ResumeThread(ResumeThread)
{
	if (Pointers)
	{
		m_ExceptionRecord = *Pointers->ExceptionRecord;
		m_ContextRecord = *Pointers->ContextRecord;
	}

	auto Previous = &m_ExceptionRecord;
	for (auto Iterator = m_ExceptionRecord.ExceptionRecord; Iterator; Iterator = Iterator->ExceptionRecord)
	{
		m_ExceptionRecords.emplace_back(*Iterator);
		Previous->ExceptionRecord = &m_ExceptionRecords.back();
		Previous = Iterator;
	}
}

exception_context::~exception_context()
{
	if (m_ResumeThread)
		ResumeThread(m_ThreadHandle.native_handle());
}


std::exception_ptr CurrentException()
{
	return std::current_exception();
}

std::exception_ptr CurrentException(const std::exception& e)
{
	try
	{
		std::throw_with_nested(MAKE_FAR_EXCEPTION(L"->"sv, tracer::get(&e)));
	}
	catch (...)
	{
		return CurrentException();
	}
}

void RethrowIfNeeded(std::exception_ptr& Ptr)
{
	if (Ptr)
	{
		std::rethrow_exception(std::exchange(Ptr, {}));
	}
}
