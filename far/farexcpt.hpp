#ifndef FAREXCPT_HPP_F7B85E85_71DD_483D_BD7F_B26B8566AC8E
#define FAREXCPT_HPP_F7B85E85_71DD_483D_BD7F_B26B8566AC8E
#pragma once

/*
farexcpt.hpp

Все про исключения
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

class Plugin;

bool ProcessStdException(const std::exception& e, string_view Function, Plugin* Module = nullptr);
bool ProcessUnknownException(string_view Function, Plugin* Module = nullptr);

class unhandled_exception_filter
{
public:
	NONCOPYABLE(unhandled_exception_filter);
	unhandled_exception_filter();
	~unhandled_exception_filter();
	static void dismiss();
};

void RestoreGPFaultUI();

void RegisterTestExceptionsHook();

bool IsCppException(const EXCEPTION_POINTERS* e);

template<class function, class filter, class handler>
auto seh_invoke(function&& Callable, filter&& Filter, handler&& Handler)
{
#if COMPILER == C_GCC
	// GCC doesn't support these currently
	return Callable();
#else
#if COMPILER == C_CLANG
	// Workaround for clang "filter expression type should be an integral value" error
	std::function<DWORD(DWORD, EXCEPTION_POINTERS*)> FilterWrapper = Filter;
#define Filter FilterWrapper
#endif
	void SetFloatingPointExceptions(bool);

	__try
	{
		return Callable();
	}
	__except (SetFloatingPointExceptions(false), Filter(GetExceptionCode(), GetExceptionInformation()))
	{
		void ResetStackOverflowIfNeeded();

		ResetStackOverflowIfNeeded();
		return Handler();
	}
#if COMPILER == C_CLANG
#undef Filter
#endif
#endif
}

template<class function, class handler>
auto seh_invoke_with_ui(function&& Callable, handler&& Handler, const string_view Function, Plugin* const Module = nullptr)
{
	int SehFilter(int, EXCEPTION_POINTERS*, string_view, Plugin*);
	return seh_invoke(FWD(Callable), [&](auto Code, auto Info) { return SehFilter(Code, Info, Function, Module); }, FWD(Handler));
}

template<class function, class handler>
auto seh_invoke_no_ui(function&& Callable, handler&& Handler)
{
	return seh_invoke(FWD(Callable), [](auto, auto) { return EXCEPTION_EXECUTE_HANDLER; }, FWD(Handler));
}

template<class function>
auto seh_invoke_thread(std::exception_ptr& ExceptionPtr, function&& Callable)
{
	return seh_invoke(FWD(Callable), [&](auto Code, auto Info)
	{
		ExceptionPtr = std::make_exception_ptr(seh_exception(Code, Info, true));
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

#endif // FAREXCPT_HPP_F7B85E85_71DD_483D_BD7F_B26B8566AC8E
