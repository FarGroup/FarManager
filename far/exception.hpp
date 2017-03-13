#ifndef EXCEPTION_HPP_2CD5B7D1_D39C_4CAF_858A_62496C9221DF
#define EXCEPTION_HPP_2CD5B7D1_D39C_4CAF_858A_62496C9221DF
#pragma once

/*
exception.hpp
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

struct error_codes
{
	struct ignore{};

	error_codes();
	error_codes(ignore);

	DWORD Win32Error;
	NTSTATUS NtError;
};

namespace detail
{
	class exception_impl
	{
	public:
		exception_impl(const string& Message, const char* Function, const char* File, int Line):
			m_Message(Message),
			m_FullMessage(format(L"{0} (at {1}, {2}:{3})", Message, Function, File, Line))
		{
		}

		const auto& get_message() const noexcept { return m_Message; }
		const auto& get_full_message() const noexcept { return m_FullMessage; }
		const auto& get_error_codes() const noexcept { return m_ErrorCodes; }

	private:
		string m_Message;
		string m_FullMessage;
		error_codes m_ErrorCodes;
	};
}

class far_exception: public detail::exception_impl, public std::runtime_error
{
public:
	far_exception(const string& Message, const char* Function, const char* File, int Line);
};

#define MAKE_EXCEPTION(ExceptionType, ...) ExceptionType(__VA_ARGS__, __FUNCTION__, __FILE__, __LINE__)
#define MAKE_FAR_EXCEPTION(...) MAKE_EXCEPTION(far_exception, __VA_ARGS__)

class exception_context
{
public:
	NONCOPYABLE(exception_context);

	exception_context(DWORD Code = 0, const EXCEPTION_POINTERS* Pointers = nullptr, bool ResumeThread = false);
	~exception_context();

	auto GetCode() const { return m_Code; }
	EXCEPTION_POINTERS* GetPointers() const;
	auto GetThreadHandle() const { return m_ThreadHandle.native_handle(); }
	auto GetThreadId() const { return m_ThreadId; }

private:
	DWORD m_Code;
	EXCEPTION_RECORD m_ExceptionRecord;
	std::list<EXCEPTION_RECORD> m_ExceptionRecords;
	CONTEXT m_ContextRecord;
	mutable EXCEPTION_POINTERS m_Pointers;
	os::handle m_ThreadHandle;
	DWORD m_ThreadId;
	bool m_ResumeThread;
};

class seh_exception: public std::exception
{
public:
	seh_exception(DWORD Code, EXCEPTION_POINTERS* Pointers, bool ResumeThread):
		m_Context(std::make_shared<exception_context>(Code, Pointers, ResumeThread))
	{
	}

	const auto& GetContext() const { return *m_Context; }

private:
	std::shared_ptr<exception_context> m_Context;
};


std::exception_ptr& GlobalExceptionPtr();
void StoreGlobalException();
void RethrowIfNeeded(std::exception_ptr& Ptr);

#endif // EXCEPTION_HPP_2CD5B7D1_D39C_4CAF_858A_62496C9221DF
