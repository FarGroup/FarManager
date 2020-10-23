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

void disable_exception_handling();

void force_stderr_exception_ui(bool Force);

class Plugin;

bool handle_std_exception(const std::exception& e, std::string_view Function, const Plugin* Module = nullptr);
bool handle_unknown_exception(std::string_view Function, const Plugin* Module = nullptr);
bool use_terminate_handler();

class seh_terminate_handler
{
public:
	NONCOPYABLE(seh_terminate_handler);

	seh_terminate_handler();
	~seh_terminate_handler();

private:
	std::terminate_handler m_PreviousHandler;
};

class unhandled_exception_filter
{
public:
	NONCOPYABLE(unhandled_exception_filter);
	unhandled_exception_filter();
	~unhandled_exception_filter();

private:
	PTOP_LEVEL_EXCEPTION_FILTER m_PreviousFilter;
};

void restore_system_exception_handler();

namespace detail
{
	struct no_handler
	{
		template<typename T>
		void operator()(T const&) const {}
	};

	void cpp_try(function_ref<void()> Callable, function_ref<void()> UnknownHandler, function_ref<void(std::exception const&)> StdHandler);
	void seh_try(function_ref<void()> Callable, function_ref<DWORD(EXCEPTION_POINTERS*)> Filter, function_ref<void()> Handler);
	int seh_filter(EXCEPTION_POINTERS const* Info, std::string_view Function, Plugin const* Module);
	int seh_thread_filter(std::exception_ptr& Ptr, EXCEPTION_POINTERS* Info);
	void seh_thread_handler();
	void set_fp_exceptions(bool Enable);

	// A workaround for 2017
	// TODO: remove once we drop support for VS2017.
	template<typename result_type, typename std_handler>
	void assign(result_type& Result, std_handler const& StdHandler, std::exception const& e)
	{
		Result = StdHandler(e);
	}
}

template<typename callable, typename unknown_handler, typename std_handler = ::detail::no_handler>
auto cpp_try(callable const& Callable, unknown_handler const& UnknownHandler, std_handler const& StdHandler = {})
{
	using result_type = typename function_traits<callable>::result_type;
	using std_handler_ref = function_ref<void(std::exception const&)>;

	enum
	{
		HasStdHandler = !std::is_same_v<std_handler, ::detail::no_handler>,
		IsVoid = std::is_same_v<result_type, void>,
	};

	std_handler_ref StdHandlerRef = nullptr;

	if constexpr (IsVoid)
	{
		if constexpr (HasStdHandler)
			StdHandlerRef = StdHandler;

		::detail::cpp_try(Callable, UnknownHandler, StdHandlerRef);
	}
	else
	{
		result_type Result;

		[[maybe_unused]]
		const auto StdHandlerEx = [&](std::exception const& e)
		{
			// IsVoid is a workaround for 2017
			// TODO: remove once we drop support for VS2017.
			if constexpr (HasStdHandler && !IsVoid)
				::detail::assign(Result, StdHandler, e);
		};

		if constexpr (HasStdHandler)
			StdHandlerRef = StdHandlerEx;

		::detail::cpp_try(
		[&]
		{
			Result = Callable();
		},
		[&]
		{
WARNING_PUSH()
WARNING_DISABLE_MSC(4702) // unreachable code
			Result = UnknownHandler();
		},
		StdHandlerRef);
WARNING_POP()

		return Result;
	}
}

std::exception_ptr wrap_current_exception(const char* Function, string_view File, int Line);

#define SAVE_EXCEPTION_TO(ExceptionPtr) \
	ExceptionPtr = wrap_current_exception(__FUNCTION__, WIDE_SV(__FILE__), __LINE__)

void rethrow_if(std::exception_ptr& Ptr);

template<class function, class filter, class handler>
auto seh_try(function const& Callable, filter const& Filter, handler const& Handler)
{
	using result_type = typename function_traits<function>::result_type;

	if constexpr (std::is_same_v<result_type, void>)
	{
		::detail::seh_try(Callable, Filter, Handler);
	}
	else
	{
		result_type Result;
WARNING_PUSH()
WARNING_DISABLE_MSC(4702) // unreachable code
		::detail::seh_try([&]{ Result = Callable(); }, Filter, [&]{ Result = Handler(); });
WARNING_POP()
		return Result;
	}
}

template<class function, class handler>
auto seh_try_with_ui(function const& Callable, handler const& Handler, const std::string_view Function, const Plugin* const Module = nullptr)
{
	return seh_try(
		Callable,
		[&](EXCEPTION_POINTERS* const Info){ return detail::seh_filter(Info, Function, Module); },
		Handler
	);
}

template<class function, class handler>
auto seh_try_no_ui(function const& Callable, handler const& Handler)
{
	return seh_try(
		Callable,
		[](EXCEPTION_POINTERS*) { return EXCEPTION_EXECUTE_HANDLER; },
		Handler
	);
}

template<class function>
auto seh_try_thread(std::exception_ptr& ExceptionPtr, function const& Callable)
{
	return seh_try(
		Callable,
		[&](EXCEPTION_POINTERS* const Info){ return detail::seh_thread_filter(ExceptionPtr, Info); },
		detail::seh_thread_handler
	);
}

#endif // EXCEPTION_HANDLER_HPP_F7B85E85_71DD_483D_BD7F_B26B8566AC8E
