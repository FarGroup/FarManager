﻿#ifndef EXCEPTION_HANDLER_HPP_F7B85E85_71DD_483D_BD7F_B26B8566AC8E
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
#include "common/preprocessor.hpp"

// External:

//----------------------------------------------------------------------------

class Plugin;

bool ProcessStdException(const std::exception& e, std::string_view Function, const Plugin* Module = nullptr);
bool ProcessUnknownException(std::string_view Function, const Plugin* Module = nullptr);

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
	void ResetStackOverflowIfNeeded();
	void SetFloatingPointExceptions(bool Enable);
	std::exception_ptr MakeSehExceptionPtr(DWORD Code, EXCEPTION_POINTERS* Pointers, bool ResumeThread);
}


template<class function, class filter, class handler>
auto seh_invoke(function&& Callable, filter&& Filter, handler&& Handler)
{
#if COMPILER(GCC)
	// GCC doesn't support these currently
	return Callable();
#else
#if COMPILER(CLANG)
	// Workaround for clang "filter expression type should be an integral value" error
	std::function<DWORD(DWORD, EXCEPTION_POINTERS*)> FilterWrapper = Filter;
#define Filter FilterWrapper
#endif

	__try
	{
		return Callable();
	}
	__except (detail::SetFloatingPointExceptions(false), Filter(GetExceptionCode(), GetExceptionInformation()))
	{
		detail::ResetStackOverflowIfNeeded();
	}

	return Handler();

#if COMPILER(CLANG)
#undef Filter
#endif
#endif
}

template<class function, class handler>
auto seh_invoke_with_ui(function&& Callable, handler&& Handler, const std::string_view Function, const Plugin* const Module = nullptr)
{
	return seh_invoke(FWD(Callable), [&](DWORD const Code, EXCEPTION_POINTERS* const Info)
	{
		return detail::SehFilter(Code, Info, Function, Module);
	}, FWD(Handler));
}

template<class function, class handler>
auto seh_invoke_no_ui(function&& Callable, handler&& Handler)
{
	return seh_invoke(FWD(Callable), [](DWORD, EXCEPTION_POINTERS*) { return EXCEPTION_EXECUTE_HANDLER; }, FWD(Handler));
}

template<class function>
auto seh_invoke_thread(std::exception_ptr& ExceptionPtr, function&& Callable)
{
	return seh_invoke(FWD(Callable), [&](DWORD const Code, EXCEPTION_POINTERS* const Info)
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
