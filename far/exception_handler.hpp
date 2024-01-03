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
#include "platform.concurrency.hpp"

// Common:
#include "common/function_ref.hpp"
#include "common/function_traits.hpp"
#include "common/preprocessor.hpp"
#include "common/source_location.hpp"

// External:

//----------------------------------------------------------------------------

void disable_exception_handling();

bool exception_handling_in_progress();

void force_stderr_exception_ui(bool Force);

class Plugin;

bool handle_std_exception(const std::exception& e, const Plugin* Module = nullptr, source_location const& Location = source_location::current());
bool handle_unknown_exception(const Plugin* Module = nullptr, source_location const& Location = source_location::current());
bool use_terminate_handler();

class unhandled_exception_filter
{
public:
	NONCOPYABLE(unhandled_exception_filter);
	unhandled_exception_filter();
	~unhandled_exception_filter();

private:
	PTOP_LEVEL_EXCEPTION_FILTER m_PreviousFilter;
};

class signal_handler
{
public:
	NONCOPYABLE(signal_handler);

	signal_handler();
	~signal_handler();

private:
	using signal_handler_t = void(*)(int);
	signal_handler_t m_PreviousHandler;
};

class invalid_parameter_handler
{
public:
	NONCOPYABLE(invalid_parameter_handler);

	invalid_parameter_handler();
	~invalid_parameter_handler();

private:
	_invalid_parameter_handler m_PreviousHandler;
};

class vectored_exception_handler
{
public:
	NONCOPYABLE(vectored_exception_handler);

	vectored_exception_handler();
	~vectored_exception_handler();

private:
	void* m_Handler;
};

void restore_system_exception_handler();

class seh_exception: public os::event
{
public:
	seh_exception();
	~seh_exception();

	void set(EXCEPTION_POINTERS const& Pointers);
	void raise();
	void dismiss();
	class seh_exception_impl;
	seh_exception_impl const& get() const;

private:
	std::unique_ptr<seh_exception_impl> m_Impl;
};

namespace detail
{
	template<typename callable>
	concept unknown_handler = std::invocable<callable, source_location const&>;

	struct no_handler
	{
		void operator()(auto const&, auto const&) const {}
	};

	void cpp_try(function_ref<void()> Callable, function_ref<void(source_location const&)> UnknownHandler, function_ref<void(std::exception const&, source_location const&)> StdHandler, source_location const& Location);
	void seh_try(function_ref<void()> Callable, function_ref<DWORD(EXCEPTION_POINTERS*)> Filter, function_ref<void(DWORD)> Handler);
	int seh_filter(EXCEPTION_POINTERS const* Info, Plugin const* Module, source_location const& Location = source_location::current());
	int seh_thread_filter(seh_exception& Exception, EXCEPTION_POINTERS const* Info);
	void seh_thread_handler(DWORD ExceptionCode);
	void set_fp_exceptions(bool Enable);
}

template<typename callable, typename std_handler = ::detail::no_handler>
auto cpp_try(callable const& Callable, ::detail::unknown_handler auto const& UnknownHandler, std_handler const& StdHandler = {}, source_location const& Location = source_location::current())
{
	using result_type = typename function_traits<callable>::result_type;
	using std_handler_ref = function_ref<void(std::exception const&, source_location const&)>;

	enum
	{
		HasStdHandler = !std::same_as<std_handler, ::detail::no_handler>,
		IsVoid = std::same_as<result_type, void>,
	};

	std_handler_ref StdHandlerRef = nullptr;

	if constexpr (IsVoid)
	{
		if constexpr (HasStdHandler)
			StdHandlerRef = StdHandler;

		::detail::cpp_try(Callable, UnknownHandler, StdHandlerRef, Location);
	}
	else
	{
		result_type Result;

WARNING_PUSH()
WARNING_DISABLE_MSC(4702) // unreachable code

		[[maybe_unused]]
		const auto StdHandlerEx = [&](std::exception const& e, source_location const&)
		{
			if constexpr (HasStdHandler)
				Result = StdHandler(e, Location);
		};

		if constexpr (HasStdHandler)
			StdHandlerRef = StdHandlerEx;

		::detail::cpp_try(
		[&]
		{
			Result = Callable();
		},
		[&](source_location const&)
		{
			Result = UnknownHandler(Location);
		},
		StdHandlerRef,
		Location);

WARNING_POP()

		return Result;
	}
}

std::exception_ptr wrap_current_exception(source_location const& Location);

class save_exception_to
{
public:
	explicit save_exception_to(std::exception_ptr& Ptr):
		m_Ptr(&Ptr)
	{
	}

	void operator()(source_location const& Location) const
	{
		*m_Ptr = wrap_current_exception(Location);
	}

private:
	std::exception_ptr* m_Ptr;
};

template<auto Fallback>
class save_exception_and_return: save_exception_to
{
public:
	explicit save_exception_and_return(std::exception_ptr& Ptr):
		save_exception_to(Ptr)
	{}

	auto operator()(source_location const& Location) const
	{
		save_exception_to::operator()(Location);
		return Fallback;
	}
};

void rethrow_if(std::exception_ptr& Ptr);

template<typename function>
auto seh_try(function const& Callable, auto const& Filter, auto const& Handler)
{
	using result_type = typename function_traits<function>::result_type;

	if constexpr (std::same_as<result_type, void>)
	{
		::detail::seh_try(Callable, Filter, Handler);
	}
	else
	{
		result_type Result;
WARNING_PUSH()
WARNING_DISABLE_MSC(4702) // unreachable code
		::detail::seh_try([&]{ Result = Callable(); }, Filter, [&](DWORD const ExceptionCode){ Result = Handler(ExceptionCode); });
WARNING_POP()
		return Result;
	}
}

auto seh_try_with_ui(auto const& Callable, auto const& Handler, const Plugin* const Module = nullptr, source_location const& Location = source_location::current())
{
	return seh_try(
		Callable,
		[&](EXCEPTION_POINTERS const* const Info){ return detail::seh_filter(Info, Module, Location); },
		Handler
	);
}

auto seh_try_no_ui(auto const& Callable, auto const& Handler)
{
	return seh_try(
		Callable,
		[](EXCEPTION_POINTERS const*) { return EXCEPTION_EXECUTE_HANDLER; },
		Handler
	);
}

auto seh_try_thread(seh_exception& Exception, auto const& Callable)
{
	return seh_try(
		Callable,
		[&](EXCEPTION_POINTERS const* const Info){ return detail::seh_thread_filter(Exception, Info); },
		detail::seh_thread_handler
	);
}

#endif // EXCEPTION_HANDLER_HPP_F7B85E85_71DD_483D_BD7F_B26B8566AC8E
