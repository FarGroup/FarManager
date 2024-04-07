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
#include "platform.memory.hpp"

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
		throw far_exception(L"Test far error"sv);
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
		throw far_fatal_exception(L"Test far fatal error"sv);
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
			save_exception_to(Ptr)
			);
		}();

		assert(Ptr);
		rethrow_if(Ptr);
	}

	static void cpp_std_nested_thread()
	{
		std::exception_ptr Ptr;
		os::thread([&]
		{
			os::debug::set_thread_name(L"Nested thread exception test");

			cpp_try(
			[]
			{
				throw std::runtime_error("Test nested std error (thread)"s);
			},
			save_exception_to(Ptr)
			);
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
		save_exception_to(Ptr)
		);

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
				if ([[maybe_unused]] volatile auto Throw = true)
					throw far_exception(L"Dtor exception"s);
			}
		};

		try
		{
			c C;
			throw far_exception(L"Regular exception"s);
		}
		catch (...)
		{
		}
	}

	static void cpp_noexcept_throw()
	{
		try
		{
			const auto do_throw = []{ throw far_exception(L"throw from a noexcept function"); };

			[&]() noexcept
			{
				do_throw();
			}();
		}
		catch (far_exception const&)
		{
			std::unreachable();
		}
	}

	static void cpp_reach_unreachable()
	{
		if ([[maybe_unused]] volatile auto Reach = true)
			std::unreachable();
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
		if ([[maybe_unused]] volatile auto Value = true)
			assert(!Value);
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

		std::bit_cast<func_t>(const_cast<int*>(&NotExecutable))();
	}

	static void seh_access_violation_ex_nul()
	{
		using func_t = void(*)();

		volatile const func_t InvalidAddress = nullptr;
		InvalidAddress();
	}

	static void seh_access_violation_ex_np()
	{
		using func_t = void(*)();

		volatile const func_t InvalidAddress = std::bit_cast<func_t>(intptr_t{1});
		assert(!os::memory::is_pointer(std::bit_cast<void const*>(InvalidAddress)));
		InvalidAddress();
	}

	static void seh_access_violation_bad()
	{
		static const ULONG_PTR Args[]
		{
			EXCEPTION_WRITE_FAULT,
			std::bit_cast<ULONG_PTR>(&seh_access_violation_bad),
			static_cast<ULONG_PTR>(STATUS_ACCESS_DENIED),
		};

		// Use [ Continue ] to test all
		RaiseException(STATUS_ACCESS_VIOLATION, 0, 0, Args);
		RaiseException(STATUS_ACCESS_VIOLATION, 0, 1, Args);
		RaiseException(STATUS_IN_PAGE_ERROR, 0, 2, Args);
	}

	static void seh_in_page_error()
	{
		// No idea how to generate a real one easily

		static const int Symbol = 42;

		ULONG_PTR const Args[]
		{
			static_cast<ULONG_PTR>(EXCEPTION_READ_FAULT),
			std::bit_cast<ULONG_PTR>(&Symbol),
			static_cast<ULONG_PTR>(STATUS_IO_DEVICE_ERROR),
		};

		RaiseException(STATUS_IN_PAGE_ERROR, 0, static_cast<DWORD>(std::size(Args)), Args);
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
		os::thread const Thread([&]
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

	static void seh_heap_no_memory()
	{
		[[maybe_unused]] const auto m = HeapAlloc(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, std::numeric_limits<size_t>::max());
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

	static void seh_fp_invalid()
	{
		detail::set_fp_exceptions(true);
		volatile const auto InvalidDenominator = 0.0;
		[[maybe_unused]]
		volatile const auto Result = InvalidDenominator / InvalidDenominator;
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
		volatile const auto Result = *std::bit_cast<volatile const double*>(Data.Data + 3);
	}

	static void seh_unknown()
	{
		RaiseException(-1, 0, 0, {});
	}

	static void seh_unhandled()
	{
		os::thread Thread([&]
		{
			os::debug::set_thread_name(L"Unhandled exception test");
			RaiseException(-1, 0, 0, {});
		});
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

	const auto add_to_menu = [&](string_view const Line)
	{
		Menu->AddItem(string(Line));
	};

	tracer.current_stacktrace({}, add_to_menu);

	if (std::current_exception())
	{
		Menu->AddItem(MenuItemEx(L"Exception stack"s, LIF_SEPARATOR));
		tracer.exception_stacktrace({}, add_to_menu);
	}

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

	using test_entry = std::pair<void(*)(), string_view>;

	static constexpr test_entry CppTests[]
	{
		{ tests::cpp_far,                      L"far_exception"sv },
		{ tests::cpp_far_rethrow,              L"far_exception (rethrow)"sv },
		{ tests::cpp_far_fatal,                L"far_fatal_exception"sv },
		{ tests::cpp_std,                      L"std::exception"sv },
		{ tests::cpp_std_lib,                  L"std::exception from stdlib"sv },
		{ tests::cpp_std_nested,               L"nested std::exception"sv },
		{ tests::cpp_std_nested_thread,        L"nested std::exception (thread)"sv },
		{ tests::cpp_std_bad_alloc,            L"std::bad_alloc"sv },
		{ tests::cpp_bad_malloc,               L"malloc failure"sv },
		{ tests::cpp_unknown,                  L"unknown exception"sv },
		{ tests::cpp_unknown_nested,           L"unknown exception (nested)"sv },
		{ tests::cpp_abort,                    L"abort"sv },
		{ tests::cpp_terminate,                L"terminate"sv },
		{ tests::cpp_terminate_unwind,         L"terminate unwind"sv },
		{ tests::cpp_noexcept_throw,           L"noexcept throw"sv },
		{ tests::cpp_reach_unreachable,        L"reach unreachable"sv },
		{ tests::cpp_pure_virtual_call,        L"pure virtual call"sv },
		{ tests::cpp_memory_leak,              L"memory leak"sv },
		{ tests::cpp_invalid_parameter,        L"invalid parameter"sv },
		{ tests::cpp_assertion_failure,        L"assertion failure"sv },
	};

	static constexpr test_entry SehTests[]
	{
		{ tests::seh_access_violation_read,    L"access violation (read)"sv },
		{ tests::seh_access_violation_write,   L"access violation (write)"sv },
		{ tests::seh_access_violation_ex_nx,   L"access violation (execute NX)"sv },
		{ tests::seh_access_violation_ex_nul,  L"access violation (execute nullptr)"sv },
		{ tests::seh_access_violation_ex_np,   L"access violation (execute non-ptr)"sv },
		{ tests::seh_access_violation_bad,     L"access violation (malformed)"sv },
		{ tests::seh_in_page_error,            L"in page error"sv },
		{ tests::seh_divide_by_zero,           L"divide by zero"sv },
		{ tests::seh_divide_by_zero_thread,    L"divide by zero (thread)"sv },
		{ tests::seh_int_overflow,             L"int overflow"sv },
		{ tests::seh_stack_overflow,           L"stack overflow"sv },
		{ tests::seh_heap_corruption,          L"heap corruption"sv },
		{ tests::seh_heap_no_memory,           L"heap no memory"sv },
		{ tests::seh_fp_divide_by_zero,        L"floating-point divide by zero"sv },
		{ tests::seh_fp_overflow,              L"floating-point overflow"sv },
		{ tests::seh_fp_underflow,             L"floating-point underflow"sv },
		{ tests::seh_fp_inexact_result,        L"floating-point inexact result"sv },
		{ tests::seh_fp_invalid,               L"floating-point invalid operation"sv },
		{ tests::seh_breakpoint,               L"breakpoint"sv },
		{ tests::seh_alignment_fault,          L"alignment fault"sv },
		{ tests::seh_unknown,                  L"unknown"sv },
		{ tests::seh_unhandled,                L"unhandled"sv },
	};

	static constexpr test_entry AsanTests[]
	{
		{ tests::asan_stack_buffer_overflow,   L"stack-buffer-overflow"sv },
		{ tests::asan_heap_buffer_overflow,    L"heap-buffer-overflow"sv },
		{ tests::asan_stack_use_after_scope,   L"stack-use-after-scope"sv },
	};

	static constexpr std::pair<std::span<test_entry const>, string_view> TestGroups[]
	{
		{ CppTests,       L"C++"sv },
		{ SehTests,       L"SEH"sv },
		{ AsanTests,      L"ASan"sv },
	};

	const auto TestGroupsMenu = VMenu2::create(L"Test Exceptions"s, {}, ScrY - 3);
	TestGroupsMenu->SetMenuFlags(VMENU_WRAPMODE);
	TestGroupsMenu->SetPosition({ -1, -1, 0, 0 });

	for (const auto& [Ptr, Description]: TestGroups)
	{
		TestGroupsMenu->AddItem(string(Description));
	}

	static auto ForceStderrUI = false;

	TestGroupsMenu->AddItem(MenuItemEx{ {}, LIF_SEPARATOR });
	TestGroupsMenu->AddItem(MenuItemEx{ L"Use stderr UI"s, ForceStderrUI ? LIF_CHECKED : LIF_NONE });

	const auto StdErrId = static_cast<int>(TestGroupsMenu->size() - 1);

	for (;;)
	{
		const auto TestGroupsMenuExitCode = TestGroupsMenu->Run();
		if (TestGroupsMenuExitCode < 0)
			break;

		if (TestGroupsMenuExitCode == StdErrId)
		{
			ForceStderrUI = !ForceStderrUI;
			force_stderr_exception_ui(ForceStderrUI);
			TestGroupsMenu->FlipCheck(StdErrId);
			continue;
		}

		const auto& TestGroup = TestGroups[TestGroupsMenuExitCode];

		const auto TestsMenu = VMenu2::create(string(TestGroup.second), {}, ScrY - 3);
		TestsMenu->SetMenuFlags(VMENU_WRAPMODE);
		TestsMenu->SetPosition({ -1, -1, 0, 0 });

		for (const auto& [Ptr, Description]: TestGroup.first)
		{
			TestsMenu->AddItem(string(Description));
		}

		for (;;)
		{
			const auto TestsExitCode = TestsMenu->Run();
			if (TestsExitCode < 0)
				break;

			const auto& Test = TestGroup.first[TestsExitCode];

			Test.first();
		}
	}

	return true;
}

void RegisterTestExceptionsHook()
{
	Global->WindowManager->AddGlobalKeyHandler(ExceptionTestHook);
}
