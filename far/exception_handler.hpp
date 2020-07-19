#ifndef EXCEPTION_HANDLER_HPP_F7B85E85_71DD_483D_BD7F_B26B8566AC8E
#define EXCEPTION_HANDLER_HPP_F7B85E85_71DD_483D_BD7F_B26B8566AC8E
#pragma once

/*
exception_handler.hpp

*/
/*
Copyright © 2018 Far Group
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

// Internal:

// Platform:

// Common:
#include "common/function_ref.hpp"
#include "common/function_traits.hpp"
#include "common/preprocessor.hpp"

// External:

//----------------------------------------------------------------------------

class Plugin;

bool ProcessStdException(const std::exception& e, std::string_view Function, const Plugin* Module = nullptr);
bool ProcessUnknownException(std::string_view Function, const Plugin* Module = nullptr);
bool use_terminate_handler();

class unhandled_exception_filter
{
public:
	NONCOPYABLE(unhandled_exception_filter);
	unhandled_exception_filter();
	~unhandled_exception_filter();
	static void dismiss();
};

void RestoreGPFaultUI();

namespace detail
{
	int SehFilter(int Code, const EXCEPTION_POINTERS* Info, std::string_view Function, const Plugin* Module);
	void SetFloatingPointExceptions(bool Enable);
	std::exception_ptr MakeSehExceptionPtr(DWORD Code, EXCEPTION_POINTERS* Pointers, bool ResumeThread);
	void seh_invoke_impl(function_ref<void()> Callable, function_ref<DWORD(DWORD, EXCEPTION_POINTERS*)> Filter, function_ref<void()> Handler);
}


template<class function, class filter, class handler>
auto seh_invoke(function const& Callable, filter const& Filter, handler const& Handler)
{
	using result_type = typename function_traits<function>::result_type;

WARNING_PUSH()
WARNING_DISABLE_MSC(4702) // unreachable code

	if constexpr (std::is_same_v<result_type, void>)
	{
		detail::seh_invoke_impl([&]{ Callable(); }, Filter, [&]{ Handler(); });
	}
	else
	{
		result_type Result;
		detail::seh_invoke_impl([&]{ Result = Callable(); }, Filter, [&]{ Result = Handler(); });
		return Result;
	}

WARNING_POP()
}

template<class function, class handler>
auto seh_invoke_with_ui(function const& Callable, handler const& Handler, const std::string_view Function, const Plugin* const Module = nullptr)
{
	return seh_invoke(Callable, [&](DWORD const Code, EXCEPTION_POINTERS* const Info)
	{
		return detail::SehFilter(Code, Info, Function, Module);
	}, Handler);
}

template<class function, class handler>
auto seh_invoke_no_ui(function const& Callable, handler const& Handler)
{
	return seh_invoke(Callable, [](DWORD, EXCEPTION_POINTERS*) { return EXCEPTION_EXECUTE_HANDLER; }, Handler);
}

template<class function>
auto seh_invoke_thread(std::exception_ptr& ExceptionPtr, function const& Callable)
{
	return seh_invoke(Callable, [&](DWORD const Code, EXCEPTION_POINTERS* const Info)
	{
		ExceptionPtr = detail::MakeSehExceptionPtr(Code, Info, true);
		return EXCEPTION_EXECUTE_HANDLER;
	},
	[]
	{
		// The thread is about to quit, but we still need it to get a stack trace.
		// It will be released once the corresponding exception context is destroyed.
		// The caller MUST detach it if ExceptionPtr is not empty.
		SuspendThread(GetCurrentThread());
	});
}

#endif // EXCEPTION_HANDLER_HPP_F7B85E85_71DD_483D_BD7F_B26B8566AC8E
