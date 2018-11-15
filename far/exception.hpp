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

#include "platform.hpp"

struct error_state
{
	static error_state fetch();
	bool engaged() const;

	DWORD Win32Error = ERROR_SUCCESS;
	NTSTATUS NtError = STATUS_SUCCESS;

private:
	bool m_Engaged = false;
};

struct error_state_ex: public error_state
{
	error_state_ex() = default;

	error_state_ex(const error_state& State, string_view const Message = {}):
		error_state(State),
		What(Message)
	{
	}

	string What;
};

namespace detail
{
	class exception_impl
	{
	public:
		exception_impl(string_view Message, const char* Function, const char* File, int Line);

		const auto& get_message() const noexcept { return m_ErrorState.What; }
		const auto& get_full_message() const noexcept { return m_FullMessage; }
		const auto& get_error_state() const noexcept { return m_ErrorState; }
		const auto& get_function() const noexcept { return m_Function; }
		const auto& get_location() const noexcept { return m_Location; }

	private:
		string m_Function;
		string m_Location;
		string m_FullMessage;
		error_state_ex m_ErrorState;
	};
}

class far_base_exception: public detail::exception_impl, public std::runtime_error
{
public:
	far_base_exception(string_view Message, const char* Function, const char* File, int Line);
	far_base_exception(string_view Message, std::vector<string>&& Stack, const char* Function, const char* File, int Line);
	const std::vector<string>& get_stack() const;

private:
	std::vector<string> m_Stack;
};

class attach_debugger
{
protected:
	attach_debugger();
};

/*
  Represents a non-continuable failure:
  - logic errors, which shouldn't happen
  - fatal OS errors
  - ...
  I.e. we either don't really know what to do or doing anything will do more harm than good.
  It shouldn't be caught explicitly in general and fly straight to main().
*/
class far_fatal_exception : private attach_debugger, public far_base_exception
{
	using far_base_exception::far_base_exception;
};

/*
  Represents all other failures, potentially continuable.
  Base class for more specific exceptions.
*/
class far_exception : public far_base_exception
{
	using far_base_exception::far_base_exception;
};

#define MAKE_EXCEPTION(ExceptionType, ...) ExceptionType(__VA_ARGS__, __FUNCTION__, __FILE__, __LINE__)
#define MAKE_FAR_FATAL_EXCEPTION(...) MAKE_EXCEPTION(far_fatal_exception, __VA_ARGS__)
#define MAKE_FAR_EXCEPTION(...) MAKE_EXCEPTION(far_exception, __VA_ARGS__)

class exception_context
{
public:
	NONCOPYABLE(exception_context);

	explicit exception_context(DWORD Code = 0, const EXCEPTION_POINTERS* Pointers = nullptr, bool ResumeThread = false);
	~exception_context();

	auto code() const { return m_Code; }
	auto pointers() const { return const_cast<EXCEPTION_POINTERS*>(&m_Pointers); }
	auto thread_handle() const { return m_ThreadHandle.native_handle(); }
	auto thread_id() const { return m_ThreadId; }

private:
	DWORD m_Code;
	EXCEPTION_RECORD m_ExceptionRecord;
	std::list<EXCEPTION_RECORD> m_ExceptionRecords;
	CONTEXT m_ContextRecord;
	EXCEPTION_POINTERS m_Pointers;
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

	const auto& context() const { return *m_Context; }

private:
	std::shared_ptr<exception_context> m_Context;
};


std::exception_ptr CurrentException();
std::exception_ptr CurrentException(const std::exception& e);
void RethrowIfNeeded(std::exception_ptr& Ptr);

#define CATCH_AND_SAVE_EXCEPTION_TO(ExceptionPtr) \
		catch (const std::exception& e) \
		{ \
			ExceptionPtr = CurrentException(e); \
		} \
		catch (...) \
		{ \
			ExceptionPtr = CurrentException(); \
		}

#endif // EXCEPTION_HPP_2CD5B7D1_D39C_4CAF_858A_62496C9221DF
