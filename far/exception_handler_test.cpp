/*
exception_handler_test.cpp
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

// BUGBUG
#include "platform.headers.hpp"

// Self:
#include "exception_handler_test.hpp"

// Internal:
#include "exception_handler.hpp"
#include "global.hpp"
#include "interf.hpp"
#include "keys.hpp"
#include "lang.hpp"
#include "message.hpp"
#include "vmenu.hpp"
#include "vmenu2.hpp"
#include "tracer.hpp"

// Platform:
#include "platform.concurrency.hpp"
#include "platform.debug.hpp"

// Common:
#include "common/preprocessor.hpp"
#include "common/scope_exit.hpp"
#include "common/string_utils.hpp"

// External:

//----------------------------------------------------------------------------

namespace tests
{
	[[noreturn]]
	static void cpp_far()
	{
		throw MAKE_FAR_EXCEPTION(L"Test far error"sv);
	}

	[[noreturn]]
	static void cpp_far_rethrow()
	{
		try
		{
			cpp_far();
		}
		catch (...)
		{
			throw;
		}
	}

	[[noreturn]]
	static void cpp_far_fatal()
	{
		throw MAKE_FAR_FATAL_EXCEPTION(L"Test far fatal error"sv);
	}

	[[noreturn]]
	static void cpp_std()
	{
		throw std::runtime_error("Test std error"s);
	}

	static void cpp_std_lib()
	{
		string().at(42) = 0;
	}

	static void cpp_std_nested()
	{
		std::exception_ptr Ptr;

		[&]() noexcept
		{
			cpp_try(
			[]
			{
				throw std::runtime_error("Test nested std error"s);
			},
			[&]
			{
				SAVE_EXCEPTION_TO(Ptr);
			});
		}();

		assert(Ptr);
		rethrow_if(Ptr);
	}

	static void cpp_std_nested_thread()
	{
		std::exception_ptr Ptr;
		os::thread(os::thread::mode::join, [&]
		{
			os::debug::set_thread_name(L"Nested thread exception test");

			cpp_try(
			[]
			{
				throw std::runtime_error("Test nested std error (thread)"s);
			},
			[&]
			{
				SAVE_EXCEPTION_TO(Ptr);
			});
		});

		assert(Ptr);
		rethrow_if(Ptr);
	}

	static void cpp_std_bad_alloc()
	{
		// Less than the physical limit to leave some space for a service block, if any
		volatile const auto Size = std::numeric_limits<ptrdiff_t>::max() - 1024 * 1024;
		[[maybe_unused]]
		volatile const auto Ptr = new char[Size];
		delete[] Ptr;
	}

	static void cpp_bad_malloc()
	{
		// Less than the physical limit to leave some space for a service block, if any
		volatile const auto Size = std::numeric_limits<ptrdiff_t>::max() - 1024 * 1024;
		if (volatile const auto Ptr = static_cast<char*>(malloc(Size)))
			free(Ptr);
	}

	[[noreturn]]
	static void cpp_unknown()
	{
		throw 42u;
	}

	static void cpp_unknown_nested()
	{
		std::exception_ptr Ptr;

		cpp_try(
		[]
		{
			throw 69u;
		},
		[&]
		{
			SAVE_EXCEPTION_TO(Ptr);
		});

		rethrow_if(Ptr);
	}

	[[noreturn]]
	static void cpp_abort()
	{
		std::abort();
	}

	[[noreturn]]
	static void cpp_terminate()
	{
		std::terminate();
	}

	static void cpp_terminate_unwind()
	{
		struct c
		{
			~c() noexcept(false)
			{
				if ([[maybe_unused]] volatile const auto Throw = true)
					throw MAKE_FAR_EXCEPTION(L"Dtor exception"s);
			}
		};

		try
		{
			c C;
			throw MAKE_FAR_EXCEPTION(L"Regular exception"s);
		}
		catch (...)
		{
		}
	}

	static void cpp_noexcept_throw()
	{
		try
		{
			const auto do_throw = []{ throw MAKE_FAR_EXCEPTION(L"throw from a noexcept function"); };

			[&]() noexcept
			{
				do_throw();
			}();
		}
		catch (far_exception const&)
		{
			UNREACHABLE;
		}
	}

	static void cpp_pure_virtual_call()
	{
		struct base
		{
			base()
			{
				bar();
			}

			virtual ~base() = default;

			virtual void foo() = 0;

			void bar()
			{
				foo();
			}
		};

		struct derived1: base
		{
			void foo() override
			{
			}
		};

		struct derived2 : base
		{
			void foo() override
			{
			}
		};

		// Nondeterminitic condition to prevent devirtualization
		if (std::chrono::system_clock().now().time_since_epoch() / 1s & 1)
			derived1{};
		else
			derived2{};
	}

	static void cpp_memory_leak()
	{
		const auto Str1 = "We're walking in the air"sv;
		copy_string(Str1, new char[Str1.size()]);

		const auto Str2 = L"We're floating in the moonlit sky"sv;
		copy_string(Str2, new wchar_t[Str2.size()]);

		*new int = 42;

		Global->WindowManager->ExitMainLoop(FALSE);
	}

	static void cpp_invalid_parameter()
	{
#if IS_MICROSOFT_SDK()
		const auto Func = printf;
		Func({});
#endif
	}

	static void cpp_assertion_failure()
	{
		assert(true == false);
	}

	static void seh_access_violation_read()
	{
		volatile const int* InvalidAddress = nullptr;
		[[maybe_unused]]
		volatile const auto Result = *InvalidAddress;
	}

	static volatile const int ReadOnly = 0;
	static void seh_access_violation_write()
	{
		// Try something real first to see the address
		const_cast<int&>(ReadOnly) = 42;

		// Fallback
		volatile int* InvalidAddress = nullptr;
		*InvalidAddress = 42;
	}

	static volatile const int NotExecutable = 42;
	static void seh_access_violation_ex_nx()
	{
		using func_t = void(*)();

		reinterpret_cast<func_t>(const_cast<int*>(&NotExecutable))();
	}

	static void seh_access_violation_ex_nul()
	{
		using func_t = void(*)();

		volatile const func_t InvalidAddress = nullptr;
		InvalidAddress();
	}

	static void seh_divide_by_zero()
	{
		volatile const auto InvalidDenominator = 0;
		[[maybe_unused]]
		volatile const auto Result = 42 / InvalidDenominator;
	}

	static void seh_divide_by_zero_thread()
	{
		seh_exception SehException;
		os::thread const Thread(os::thread::mode::join, [&]
		{
			os::debug::set_thread_name(L"Divide by zero test");

			seh_try_thread(SehException, []
			{
				volatile const auto InvalidDenominator = 0;
				[[maybe_unused]]
				volatile const auto Result = 42 / InvalidDenominator;
			});
		});

		const auto Result = os::handle::wait_any({ Thread.native_handle(), SehException.native_handle() });

		assert(Result == 1);
		if (Result == 1)
			SehException.raise();
	}

	static void seh_int_overflow()
	{
		volatile const auto Denominator = -1;
		[[maybe_unused]]
		volatile const auto Result = std::numeric_limits<int>::min() / Denominator;
	}

	static void seh_stack_overflow()
	{
		[[maybe_unused]]
		volatile char Buffer[10240];

		// Prevent the compiler from detecting recursion on all control paths:
		if ([[maybe_unused]] volatile const auto Condition = true)
			seh_stack_overflow();

		// After the recursive call to prevent the tail call optimisation.
		Buffer[0] = 1;
	}

	[[noreturn]]
	static void seh_heap_corruption()
	{
		// Using low level functions to avoid access violations in debug blocks
		const auto m = HeapAlloc(GetProcessHeap(), 0, 42);

		for (;;)
		{
			HeapFree(GetProcessHeap(), 0, m);
		}
	}

	static void seh_fp_divide_by_zero()
	{
		detail::set_fp_exceptions(true);
		volatile const auto InvalidDenominator = 0.0;
		[[maybe_unused]]
		volatile const auto Result = 42.0 / InvalidDenominator;
	}

	static void seh_fp_overflow()
	{
		detail::set_fp_exceptions(true);
		volatile const auto Max = std::numeric_limits<double>::max();
		[[maybe_unused]]
		volatile const auto Result = Max * 2;
	}

	static void seh_fp_underflow()
	{
		detail::set_fp_exceptions(true);
		volatile const auto Min = std::numeric_limits<double>::min();
		[[maybe_unused]]
		volatile const auto Result = Min / 2;
	}

	static void seh_fp_inexact_result()
	{
		detail::set_fp_exceptions(true);
		volatile const auto Max = std::numeric_limits<double>::max();
		[[maybe_unused]]
		volatile const auto Result = Max + 1;
	}

	static void seh_breakpoint()
	{
		os::debug::breakpoint();
	}

	static void seh_alignment_fault()
	{
		volatile const struct data
		{
			char Data[3 + sizeof(double)];
		}
		Data{};
		[[maybe_unused]]
		volatile const auto Result = *reinterpret_cast<volatile const double*>(Data.Data + 3);
	}

	static void seh_unknown()
	{
		RaiseException(-1, 0, 0, {});
	}

	static void seh_unhandled()
	{
		os::thread Thread(os::thread::mode::join, [&]
		{
			os::debug::set_thread_name(L"Unhandled exception test");
			RaiseException(-1, 0, 0, {});
		});
	}

	static void seh_assertion_failure()
	{
		if ([[maybe_unused]] volatile const auto Condition = true)
			DbgRaiseAssertionFailure();
	}

	static void debug_bounds_check()
	{
		[[maybe_unused]] std::vector<int> v(1);
		const volatile size_t Index = 1;
		v[Index] = 42;
	}

	static void debug_reach_unreachable()
	{
		UNREACHABLE;
	}

	static void asan_stack_buffer_overflow()
	{
		[[maybe_unused]] int v[1];
		const volatile size_t Index = 1;
		v[Index] = 42;
	}

	static void asan_heap_buffer_overflow()
	{
		[[maybe_unused]] std::vector<int> v(1);
		const volatile size_t Index = 1;
		v.data()[Index] = 42;
	}

	static void asan_stack_use_after_scope()
	{
		volatile int* Ptr;

		{
			volatile int i = 42;
			Ptr = &i;
		}

		[[maybe_unused]]
		volatile const auto i = *Ptr;
	}
}

static bool trace()
{
	static auto Processing = false;

	if (Processing)
		return false;

	Processing = true;
	SCOPE_EXIT{ Processing = false; };

	const auto Menu = VMenu2::create(L"Current stack"s, {}, ScrY - 4);
	Menu->SetMenuFlags(VMENU_WRAPMODE | VMENU_SHOWAMPERSAND);
	Menu->SetPosition({ -1, -1, 0, 0 });

	tracer.get_symbols({}, os::debug::current_stacktrace(), [&](string_view const Line)
	{
		Menu->AddItem(string(Line));
	});

	Menu->Run();

	return true;
}

static bool ExceptionTestHook(Manager::Key const& key)
{
	if (any_of(key(), KEY_CTRLALTF1, KEY_RCTRLRALTF1, KEY_CTRLRALTF1, KEY_RCTRLALTF1))
		return trace();

	if (none_of(key(), KEY_CTRLALTAPPS, KEY_RCTRLRALTAPPS, KEY_CTRLRALTAPPS, KEY_RCTRLALTAPPS))
		return false;

	static auto Processing = false;

	if (Processing)
		return false;

	Processing = true;
	SCOPE_EXIT{ Processing = false; };

	static const std::pair<void(*)(), string_view> Tests[]
	{
		{ tests::cpp_far,                      L"C++ far_exception"sv },
		{ tests::cpp_far_rethrow,              L"C++ far_exception (rethrow)"sv },
		{ tests::cpp_far_fatal,                L"C++ far_fatal_exception"sv },
		{ tests::cpp_std,                      L"C++ std::exception"sv },
		{ tests::cpp_std_lib,                  L"C++ std::exception from stdlib"sv },
		{ tests::cpp_std_nested,               L"C++ nested std::exception"sv },
		{ tests::cpp_std_nested_thread,        L"C++ nested std::exception (thread)"sv },
		{ tests::cpp_std_bad_alloc,            L"C++ std::bad_alloc"sv },
		{ tests::cpp_bad_malloc,               L"C++ malloc failure"sv },
		{ tests::cpp_unknown,                  L"C++ unknown exception"sv },
		{ tests::cpp_unknown_nested,           L"C++ unknown exception (nested)"sv },
		{ tests::cpp_abort,                    L"C++ abort"sv },
		{ tests::cpp_terminate,                L"C++ terminate"sv },
		{ tests::cpp_terminate_unwind,         L"C++ terminate unwind"sv },
		{ tests::cpp_noexcept_throw,           L"C++ noexcept throw"sv },
		{ tests::cpp_pure_virtual_call,        L"C++ pure virtual call"sv },
		{ tests::cpp_memory_leak,              L"C++ memory leak"sv },
		{ tests::cpp_invalid_parameter,        L"C++ invalid parameter"sv },
		{ tests::cpp_assertion_failure,        L"C++ assertion failure"sv },
		{ tests::seh_access_violation_read,    L"SEH access violation (read)"sv },
		{ tests::seh_access_violation_write,   L"SEH access violation (write)"sv },
		{ tests::seh_access_violation_ex_nx,   L"SEH access violation (execute NX)"sv },
		{ tests::seh_access_violation_ex_nul,  L"SEH access violation (execute nullptr)"sv },
		{ tests::seh_divide_by_zero,           L"SEH divide by zero"sv },
		{ tests::seh_divide_by_zero_thread,    L"SEH divide by zero (thread)"sv },
		{ tests::seh_int_overflow,             L"SEH int overflow"sv },
		{ tests::seh_stack_overflow,           L"SEH stack overflow"sv },
		{ tests::seh_heap_corruption,          L"SEH heap corruption"sv },
		{ tests::seh_fp_divide_by_zero,        L"SEH floating-point divide by zero"sv },
		{ tests::seh_fp_overflow,              L"SEH floating-point overflow"sv },
		{ tests::seh_fp_underflow,             L"SEH floating-point underflow"sv },
		{ tests::seh_fp_inexact_result,        L"SEH floating-point inexact result"sv },
		{ tests::seh_breakpoint,               L"SEH breakpoint"sv },
		{ tests::seh_alignment_fault,          L"SEH alignment fault"sv },
		{ tests::seh_unknown,                  L"SEH unknown"sv },
		{ tests::seh_unhandled,                L"SEH unhandled"sv },
		{ tests::seh_assertion_failure,        L"SEH assertion failure"sv },
		{ tests::debug_bounds_check,           L"Debug bounds check"sv },
		{ tests::debug_reach_unreachable,      L"Debug reach unreachable"sv },
		{ tests::asan_stack_buffer_overflow,   L"ASan stack-buffer-overflow"sv },
		{ tests::asan_heap_buffer_overflow,    L"ASan heap-buffer-overflow"sv },
		{ tests::asan_stack_use_after_scope,   L"ASan stack-use-after-scope"sv },
	};

	const auto ModalMenu = VMenu2::create(L"Test Exceptions"s, {}, ScrY - 4);
	ModalMenu->SetMenuFlags(VMENU_WRAPMODE);
	ModalMenu->SetPosition({ -1, -1, 0, 0 });

	for (const auto& [Ptr, Description]: Tests)
	{
		ModalMenu->AddItem(string(Description));
	}

	static auto ForceStderrUI = false;

	ModalMenu->AddItem(MenuItemEx{ {}, LIF_SEPARATOR });
	ModalMenu->AddItem(MenuItemEx{ L"Use stderr UI"s, ForceStderrUI ? LIF_CHECKED : LIF_NONE });

	const auto ExitCode = ModalMenu->Run();
	if (ExitCode < 0)
		return true;

	if (static_cast<size_t>(ExitCode) == ModalMenu->size() - 1)
	{
		ForceStderrUI = !ForceStderrUI;
		return true;
	}

	force_stderr_exception_ui(ForceStderrUI);

	Tests[ExitCode].first();

	Message(MSG_WARNING,
		L"Exception test failed"sv,
		{
			string(Tests[ExitCode].second),
		},
		{ lng::MOk });

	return true;
}

void RegisterTestExceptionsHook()
{
	Global->WindowManager->AddGlobalKeyHandler(ExceptionTestHook);
}
