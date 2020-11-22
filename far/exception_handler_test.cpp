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

// Platform:
#include "platform.concurrency.hpp"

// Common:
#include "common/preprocessor.hpp"
#include "common/string_utils.hpp"

// External:

//----------------------------------------------------------------------------

#if defined(FAR_ALPHA_VERSION)

namespace tests
{
	[[noreturn]]
	static void cpp_far()
	{
		throw MAKE_FAR_EXCEPTION(L"Test far error"sv);
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

		cpp_try(
		[]
		{
			throw std::runtime_error("Test nested std error"s);
		},
		[&]
		{
			SAVE_EXCEPTION_TO(Ptr);
		});

		rethrow_if(Ptr);
	}

	static void cpp_std_nested_thread()
	{
		std::exception_ptr Ptr;
		seh_try_thread(Ptr, [&Ptr]
		{
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

		rethrow_if(Ptr);
	}

	static void cpp_std_bad_alloc()
	{
		// Less than the physical limit to leave some space for a service block, if any
		const auto Ptr = std::make_unique<char[]>(std::numeric_limits<size_t>::max() - 1024 * 1024);
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
				volatile const auto Throw = true;
				if (Throw)
					throw MAKE_FAR_EXCEPTION(L"Dtor exception"s);
			}
		};

		try
		{
			c C;
			throw MAKE_FAR_EXCEPTION(L"Regular exception"s);
		}
		catch(...)
		{
		}
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

	static void seh_access_violation_read()
	{
		volatile const int* InvalidAddress = nullptr;
		[[maybe_unused]]
		volatile const auto Result = *InvalidAddress;
	}

	static void seh_access_violation_write()
	{
		volatile int* InvalidAddress = nullptr;
		*InvalidAddress = 42;
	}

	static void seh_access_violation_execute()
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
		std::exception_ptr Ptr;
		os::thread Thread(os::thread::mode::join, [&]
		{
			seh_try_thread(Ptr, []
			{
				volatile const auto InvalidDenominator = 0;
				[[maybe_unused]]
				volatile const auto Result = 42 / InvalidDenominator;
			});
		});

		while (!Thread.is_signaled() && !Ptr)
			;

		if (Ptr)
		{
			// You're someone else's problem
			Thread.detach();
		}

		rethrow_if(Ptr);
	}

	static void seh_int_overflow()
	{
		volatile const auto Denominator = -1;
		[[maybe_unused]]
		volatile const auto Result = std::numeric_limits<int>::min() / Denominator;
	}

	WARNING_PUSH()
	WARNING_DISABLE_MSC(4717) // 'function': recursive on all control paths, function will cause runtime stack overflow
	WARNING_DISABLE_CLANG("-Winfinite-recursion")

	static void seh_stack_overflow()
	{
		volatile char Buffer[10240];

		seh_stack_overflow();

		// A "side effect" to prevent deletion of this function call due to C4718.
		// After the recursive call to prevent the tail call optimisation.
		Sleep(Buffer[0]);
	}

	WARNING_POP()

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
}

static bool ExceptionTestHook(Manager::Key const& key)
{
	if (none_of(key(), KEY_CTRLALTAPPS, KEY_RCTRLRALTAPPS, KEY_CTRLRALTAPPS, KEY_RCTRLALTAPPS))
		return false;

	static const std::pair<void(*)(), string_view> Tests[]
	{
		{ tests::cpp_far,                      L"C++ far_exception"sv },
		{ tests::cpp_far_fatal,                L"C++ far_fatal_exception"sv },
		{ tests::cpp_std,                      L"C++ std::exception"sv },
		{ tests::cpp_std_lib,                  L"C++ std::exception from stdlib"sv },
		{ tests::cpp_std_nested,               L"C++ nested std::exception"sv },
		{ tests::cpp_std_nested_thread,        L"C++ nested std::exception (thread)"sv },
		{ tests::cpp_std_bad_alloc,            L"C++ std::bad_alloc"sv },
		{ tests::cpp_unknown,                  L"C++ unknown exception"sv },
		{ tests::cpp_unknown_nested,           L"C++ unknown exception (nested)"sv },
		{ tests::cpp_terminate,                L"C++ terminate"sv },
		{ tests::cpp_terminate_unwind,         L"C++ terminate unwind"sv },
		{ tests::cpp_memory_leak,              L"C++ memory leak"sv },
		{ tests::seh_access_violation_read,    L"SEH access violation (read)"sv },
		{ tests::seh_access_violation_write,   L"SEH access violation (write)"sv },
		{ tests::seh_access_violation_execute, L"SEH access violation (execute)"sv },
		{ tests::seh_divide_by_zero,           L"SEH divide by zero"sv },
		{ tests::seh_divide_by_zero_thread,    L"SEH divide by zero (thread)"sv },
		{ tests::seh_int_overflow,             L"SEH int owerflow"sv },
		{ tests::seh_stack_overflow,           L"SEH stack overflow"sv },
		{ tests::seh_fp_divide_by_zero,        L"SEH floating-point divide by zero"sv },
		{ tests::seh_fp_overflow,              L"SEH floating-point overflow"sv },
		{ tests::seh_fp_underflow,             L"SEH floating-point underflow"sv },
		{ tests::seh_fp_inexact_result,        L"SEH floating-point inexact result"sv },
		{ tests::seh_breakpoint,               L"SEH breakpoint"sv },
		{ tests::seh_alignment_fault,          L"SEH alignment fault"sv },
		{ tests::seh_unknown,                  L"SEH unknown"sv },
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
#endif

void RegisterTestExceptionsHook()
{
#ifdef FAR_ALPHA_VERSION
	Global->WindowManager->AddGlobalKeyHandler(ExceptionTestHook);
#endif
}
