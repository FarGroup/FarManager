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

#include "exception_handler_test.hpp"

#include "exception_handler.hpp"
#include "global.hpp"
#include "interf.hpp"
#include "keys.hpp"
#include "lang.hpp"
#include "manager.hpp"
#include "message.hpp"
#include "vmenu.hpp"
#include "vmenu2.hpp"

#include "platform.concurrency.hpp"

#include "common/compiler.hpp"


#if defined(FAR_ALPHA_VERSION)

namespace tests
{
	static void cpp_far()
	{
		throw MAKE_FAR_EXCEPTION(L"Test far error"sv);
	}

	static void cpp_std()
	{
		throw std::runtime_error("Test std error"s);
	}

	static void cpp_std_nested()
	{
		{
			std::exception_ptr Ptr;
			try
			{
				throw std::runtime_error("Test nested std error"s);
			}
			CATCH_AND_SAVE_EXCEPTION_TO(Ptr);
			rethrow_if(Ptr);
		}
	}

	static void cpp_std_nested_thread()
	{
		std::exception_ptr Ptr;
		seh_invoke_thread(Ptr, [&Ptr]
		{
			try
			{
				throw std::runtime_error("Test nested std error (thread)"s);
			}
			CATCH_AND_SAVE_EXCEPTION_TO(Ptr);
		});

		rethrow_if(Ptr);
	}

	static void cpp_std_bad_alloc()
	{
		// Less than the physical limit to leave some space for a service block, if any
		const auto Ptr = std::make_unique<char[]>(std::numeric_limits<size_t>::max() - 1024 * 1024);
	}

	static void cpp_unknown()
	{
		throw 42u;
	}

	static void cpp_unknown_nested()
	{
		std::exception_ptr Ptr;
		try
		{
			throw 69u;
		}
		CATCH_AND_SAVE_EXCEPTION_TO(Ptr);
		rethrow_if(Ptr);
	}

	static void access_violation_read()
	{
		volatile const int* InvalidAddress = nullptr;
		volatile const auto Result = *InvalidAddress;
		(void)Result;
	}

	static void access_violation_write()
	{
		volatile int* InvalidAddress = nullptr;
		*InvalidAddress = 42;
	}

	static void access_violation_execute()
	{
		using func_t = void(*)();
		volatile const func_t InvalidAddress = nullptr;
		InvalidAddress();
	}

	static void divide_by_zero()
	{
		volatile const auto InvalidDenominator = 0;
		volatile const auto Result = 42 / InvalidDenominator;
		(void)Result;
	}

	static void divide_by_zero_thread()
	{
		std::exception_ptr Ptr;
		os::thread Thread(&os::thread::join, [&]
		{
			seh_invoke_thread(Ptr, []
			{
				volatile const auto InvalidDenominator = 0;
				volatile const auto Result = 42 / InvalidDenominator;
				(void)Result;
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

	WARNING_PUSH()
	WARNING_DISABLE_MSC(4717) // https://msdn.microsoft.com/en-us/library/97c54274.aspx 'function' : recursive on all control paths, function will cause runtime stack overflow
	WARNING_DISABLE_CLANG("-Winfinite-recursion")

	static void stack_overflow()
	{
		// "side effect" to prevent deletion of this function call due to C4718.
		volatile char Buffer[10240];
		*Buffer = 0;

		stack_overflow();
	}

	WARNING_POP()

	static void fp_divide_by_zero()
	{
		detail::SetFloatingPointExceptions(true);
		volatile const auto InvalidDenominator = 0.0;
		volatile const auto Result = 42.0 / InvalidDenominator;
		(void)Result;
	}

	static void fp_overflow()
	{
		detail::SetFloatingPointExceptions(true);
		volatile const auto Max = std::numeric_limits<double>::max();
		volatile const auto Result = Max * 2;
		(void)Result;
}

	static void fp_underflow()
	{
		detail::SetFloatingPointExceptions(true);
		volatile const auto Min = std::numeric_limits<double>::min();
		volatile const auto Result = Min / 2;
		(void)Result;
	}

	static void fp_inexact_result()
	{
		detail::SetFloatingPointExceptions(true);
		volatile const auto Max = std::numeric_limits<double>::max();
		volatile const auto Result = Max + 1;
		(void)Result;
	}

	static void breakpoint()
	{
		DebugBreak();
	}

	static void alignment_fault()
	{
		volatile const struct data
		{
			char Data[3 + sizeof(double)];
		}
		Data{};
		volatile const auto Result = *reinterpret_cast<volatile const double*>(Data.Data + 3);
		(void)Result;
	}
}

static bool ExceptionTestHook(Manager::Key key)
{
	if (
		key() != KEY_CTRLALTAPPS &&
		key() != KEY_RCTRLRALTAPPS &&
		key() != KEY_CTRLRALTAPPS &&
		key() != KEY_RCTRLALTAPPS
		)
		return false;

	static const std::pair<void(*)(), string_view> Tests[]
	{
		{ tests::cpp_far,                  L"C++ far_exception"sv },
		{ tests::cpp_std,                  L"C++ std::exception"sv },
		{ tests::cpp_std_nested,           L"C++ nested std::exception"sv },
		{ tests::cpp_std_nested_thread,    L"C++ nested std::exception (thread)"sv },
		{ tests::cpp_std_bad_alloc,        L"C++ std::bad_alloc"sv },
		{ tests::cpp_unknown,              L"C++ unknown exception"sv },
		{ tests::cpp_unknown_nested,       L"C++ unknown exception (nested)"sv },
		{ tests::access_violation_read,    L"Access Violation (Read)"sv },
		{ tests::access_violation_write,   L"Access Violation (Write)"sv },
		{ tests::access_violation_execute, L"Access Violation (Execute)"sv },
		{ tests::divide_by_zero,           L"Divide by zero"sv },
		{ tests::divide_by_zero_thread,    L"Divide by zero (thread)"sv },
		{ tests::stack_overflow,           L"Stack Overflow"sv },
		{ tests::fp_divide_by_zero,        L"Floating-point divide by zero"sv },
		{ tests::fp_overflow,              L"Floating-point overflow"sv },
		{ tests::fp_underflow,             L"Floating-point underflow"sv },
		{ tests::fp_inexact_result,        L"Floating-point inexact result"sv },
		{ tests::breakpoint,               L"Breakpoint"sv },
		{ tests::alignment_fault,          L"Alignment fault"sv },
	};

	const auto ModalMenu = VMenu2::create(L"Test Exceptions"s, {}, ScrY - 4);
	ModalMenu->SetMenuFlags(VMENU_WRAPMODE);
	ModalMenu->SetPosition({ -1, -1, 0, 0 });

	for (const auto& i: Tests)
	{
		ModalMenu->AddItem(string(i.second));
	}

	const auto ExitCode = ModalMenu->Run();
	if (ExitCode == -1)
		return true;

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
