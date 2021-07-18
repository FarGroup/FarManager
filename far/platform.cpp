/*
platform.cpp

Враперы вокруг некоторых WinAPI функций
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

// BUGBUG
#include "platform.headers.hpp"

// Self:
#include "platform.hpp"

// Internal:
#include "imports.hpp"
#include "pathmix.hpp"
#include "string_utils.hpp"
#include "exception.hpp"
#include "log.hpp"

// Platform:
#include "platform.fs.hpp"
#include "platform.memory.hpp"

// Common:
#include "common/algorithm.hpp"
#include "common/range.hpp"
#include "common/string_utils.hpp"

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

namespace os
{
	namespace detail
	{
		HANDLE handle_implementation::normalise(HANDLE const Handle)
		{
			return Handle == INVALID_HANDLE_VALUE? nullptr : Handle;
		}

		bool handle_implementation::wait(HANDLE const Handle, std::optional<std::chrono::milliseconds> const Timeout)
		{
			switch (const auto Result = WaitForSingleObject(Handle, Timeout? *Timeout / 1ms : INFINITE))
			{
			case WAIT_OBJECT_0:
				return true;

			case WAIT_TIMEOUT:
				return false;

			default:
				// Abandoned or error
				throw MAKE_FAR_FATAL_EXCEPTION(format(FSTR(L"WaitForSingleobject returned {}"sv), Result));
			}
		}

		std::optional<size_t> handle_implementation::wait(span<HANDLE const> const Handles, bool const WaitAll, std::optional<std::chrono::milliseconds> Timeout)
		{
			assert(!Handles.empty());
			assert(Handles.size() <= MAXIMUM_WAIT_OBJECTS);

			const auto Result = WaitForMultipleObjects(static_cast<DWORD>(Handles.size()), Handles.data(), WaitAll, Timeout? *Timeout / 1ms : INFINITE);

			if (in_closed_range<size_t>(WAIT_OBJECT_0, Result, WAIT_OBJECT_0 + Handles.size() - 1))
			{
				return Result - WAIT_OBJECT_0;
			}
			else if (Result == WAIT_TIMEOUT)
			{
				return {};
			}
			else
			{
				// Abandoned or error
				throw MAKE_FAR_FATAL_EXCEPTION(format(FSTR(L"WaitForMultipleObjects returned {}"sv), Result));
			}
		}

		void handle_closer::operator()(HANDLE Handle) const noexcept
		{
			CloseHandle(Handle);
		}

		void printer_handle_closer::operator()(HANDLE Handle) const noexcept
		{
			ClosePrinter(Handle);
		}
	}


	void set_error_mode(unsigned Mask)
	{
		// TODO: Use GetErrorMode
		SetErrorMode(SetErrorMode(Mask) | Mask);
	}

	void unset_error_mode(unsigned Mask)
	{
		// TODO: Use GetErrorMode
		SetErrorMode(SetErrorMode(0) & ~Mask);
	}

NTSTATUS get_last_nt_status()
{
	return imports.RtlGetLastNtStatus? imports.RtlGetLastNtStatus() : STATUS_SUCCESS;
}

void set_last_nt_status(NTSTATUS const Status)
{
	if (imports.RtlNtStatusToDosError)
		imports.RtlNtStatusToDosError(Status);
}

void set_last_error_from_ntstatus(NTSTATUS const Status)
{
	if (imports.RtlNtStatusToDosError)
		SetLastError(imports.RtlNtStatusToDosError(Status));
}

static string format_error_impl(unsigned const ErrorCode, bool const Nt)
{
	memory::local::ptr<wchar_t> Buffer;
	const size_t Size = FormatMessage(
			(Nt? FORMAT_MESSAGE_FROM_HMODULE : FORMAT_MESSAGE_FROM_SYSTEM) |
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_IGNORE_INSERTS |
			FORMAT_MESSAGE_MAX_WIDTH_MASK,
		(Nt? GetModuleHandle(L"ntdll.dll") : nullptr),
		ErrorCode,
		0,
		reinterpret_cast<wchar_t*>(&ptr_setter(Buffer)),
		0,
		nullptr);

	string Result(Buffer.get(), Size);
	std::replace_if(ALL_RANGE(Result), IsEol, L' ');
	inplace::trim_right(Result);

	return Result;
}

static string postprocess_error_string(unsigned const ErrorCode, string&& Str)
{
	std::replace_if(ALL_RANGE(Str), IsEol, L' ');
	inplace::trim_right(Str);
	return format(FSTR(L"0x{:0>8X} - {}"sv), ErrorCode, Str.empty() ? L"Unknown error"sv : Str);
}

[[nodiscard]]
string format_errno(int const ErrorCode)
{
	return postprocess_error_string(ErrorCode, _wcserror(ErrorCode));
}

string format_error(DWORD const ErrorCode)
{
	return postprocess_error_string(ErrorCode, format_error_impl(ErrorCode, false));
}

string format_ntstatus(NTSTATUS const Status)
{
	return postprocess_error_string(Status, format_error_impl(Status, true));
}

last_error_guard::last_error_guard():
	m_LastError(GetLastError()),
	m_LastStatus(get_last_nt_status()),
	m_Active(true)
{
}

last_error_guard::~last_error_guard()
{
	if (!m_Active)
		return;

	SetLastError(m_LastError);
	set_last_nt_status(m_LastStatus);
}

void last_error_guard::dismiss()
{
	m_Active = false;
}


bool WNetGetConnection(const string_view LocalName, string &RemoteName)
{
	auto Buffer = os::buffer<wchar_t>();
	// MSDN says that call can fail with ERROR_NOT_CONNECTED or ERROR_CONNECTION_UNAVAIL if calling application
	// is running in a different logon session than the application that made the connection.
	// However, it may fail with ERROR_NOT_CONNECTED for non-network too, in this case Buffer will not be initialised.
	// Deliberately initialised with an empty string to fix that.
	Buffer.front() = {};
	auto Size = static_cast<DWORD>(Buffer.size());
	const null_terminated C_LocalName(LocalName);
	auto Result = ::WNetGetConnection(C_LocalName.c_str(), Buffer.data(), &Size);

	while (Result == ERROR_MORE_DATA)
	{
		Buffer.reset(Size);
		Result = ::WNetGetConnection(C_LocalName.c_str(), Buffer.data(), &Size);
	}

	const auto IsReceived = [](int Code) { return Code == NO_ERROR || Code == ERROR_NOT_CONNECTED || Code == ERROR_CONNECTION_UNAVAIL; };

	if (IsReceived(Result) && *Buffer)
	{
		// Size isn't updated if the buffer is large enough
		RemoteName = Buffer.data();
		return true;
	}

	return false;
}

bool get_locale_value(LCID const LcId, LCTYPE const Id, string& Value)
{
	last_error_guard ErrorGuard;
	SetLastError(ERROR_SUCCESS);

	if (detail::ApiDynamicErrorBasedStringReceiver(ERROR_INSUFFICIENT_BUFFER, Value, [&](span<wchar_t> Buffer)
	{
		const auto ReturnedSize = GetLocaleInfo(LcId, Id, Buffer.data(), static_cast<int>(Buffer.size()));
		return ReturnedSize? ReturnedSize - 1 : 0;
	}))
	{
		return true;
	}

	// An empty string returned
	if (GetLastError() == ERROR_SUCCESS)
	{
		Value.clear();
		return true;
	}

	// Something went wrong, it's better to leave the last error as is
	ErrorGuard.dismiss();
	return false;
}

bool get_locale_value(LCID const LcId, LCTYPE const Id, int& Value)
{
	return GetLocaleInfo(LcId, Id | LOCALE_RETURN_NUMBER, reinterpret_cast<wchar_t*>(&Value), sizeof(Value) / sizeof(wchar_t)) != 0;
}

string GetPrivateProfileString(string_view const AppName, string_view const KeyName, string_view const Default, string_view const FileName)
{
	string Value;
	return detail::ApiDynamicStringReceiver(Value, [&](span<wchar_t> const Buffer)
	{
		const auto Size = ::GetPrivateProfileString(null_terminated(AppName).c_str(), null_terminated(KeyName).c_str(), null_terminated(Default).c_str(), Buffer.data(), static_cast<DWORD>(Buffer.size()), null_terminated(FileName).c_str());
		return Size == Buffer.size() - 1? Buffer.size() * 2 : Size;
	})?
		Value : string(Default);
}

bool GetWindowText(HWND Hwnd, string& Text)
{
	// GetWindowText[Length] might return 0 not only in case of failure, but also when the window title is empty.
	// To recognise this, we set LastError to ERROR_SUCCESS manually and check it after the call,
	// which doesn't change it upon success.
	last_error_guard ErrorGuard;
	SetLastError(ERROR_SUCCESS);

	if (detail::ApiDynamicStringReceiver(Text, [&](span<wchar_t> Buffer)
	{
		const size_t Length = ::GetWindowTextLength(Hwnd);

		if (!Length)
			return Length;

		if (Length + 1 > Buffer.size())
			return Length + 1;

		return static_cast<size_t>(::GetWindowText(Hwnd, Buffer.data(), static_cast<int>(Buffer.size())));
	}))
	{
		return true;
	}

	if (GetLastError() == ERROR_SUCCESS)
	{
		Text.clear();
		return true;
	}

	// Something went wrong, it's better to leave the last error as is
	ErrorGuard.dismiss();
	return false;
}

#ifndef _WIN64
bool IsWow64Process()
{
	static const auto Wow64Process = []
	{
		if (!imports.IsWow64Process)
			return false;

		BOOL Value = FALSE;
		if (!imports.IsWow64Process(GetCurrentProcess(), &Value))
			return false;

		return Value != FALSE;
	}();
	return Wow64Process;
}
#endif

DWORD GetAppPathsRedirectionFlag()
{
	static const auto RedirectionFlag = []
	{
		// App Paths key is shared in Windows 7 and above
		if (!IsWindows7OrGreater())
		{
#ifdef _WIN64
			return KEY_WOW64_32KEY;
#else
			if (IsWow64Process())
			{
				return KEY_WOW64_64KEY;
			}
#endif
		}
		return 0;
	}();
	return RedirectionFlag;
}

bool GetDefaultPrinter(string& Printer)
{
	return detail::ApiDynamicStringReceiver(Printer, [&](span<wchar_t> Buffer)
	{
		auto Size = static_cast<DWORD>(Buffer.size());
		if (::GetDefaultPrinter(Buffer.data(), &Size))
			return Size - 1;

		return GetLastError() == ERROR_INSUFFICIENT_BUFFER? Size : 0;
	});
}

bool GetComputerName(string& Name)
{
	wchar_t Buffer[MAX_COMPUTERNAME_LENGTH + 1];
	auto Size = static_cast<DWORD>(std::size(Buffer));
	if (!::GetComputerName(Buffer, &Size))
		return false;

	Name.assign(Buffer, Size);
	return true;
}

bool GetComputerNameEx(COMPUTER_NAME_FORMAT NameFormat, string& Name)
{
	return detail::ApiDynamicStringReceiver(Name, [&](span<wchar_t> Buffer)
	{
		auto Size = static_cast<DWORD>(Buffer.size());
		if (!::GetComputerNameEx(NameFormat, Buffer.data(), &Size) && GetLastError() != ERROR_MORE_DATA)
			return 0ul;
		return Size;
	});
}

bool GetUserName(string& Name)
{
	wchar_t Buffer[UNLEN + 1];
	auto Size = static_cast<DWORD>(std::size(Buffer));
	if (!::GetUserName(Buffer, &Size))
		return false;

	Name.assign(Buffer, Size - 1);
	return true;
}

bool GetUserNameEx(EXTENDED_NAME_FORMAT NameFormat, string& Name)
{
	return detail::ApiDynamicStringReceiver(Name, [&](span<wchar_t> Buffer)
	{
		auto Size = static_cast<DWORD>(Buffer.size());
		if (!::GetUserNameEx(NameFormat, Buffer.data(), &Size) && GetLastError() != ERROR_MORE_DATA)
			return 0ul;
		return Size;
	});
}

handle OpenCurrentThread()
{
	HANDLE Handle;
	return handle(DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &Handle, 0, FALSE, DUPLICATE_SAME_ACCESS)? Handle : nullptr);
}

handle OpenConsoleInputBuffer()
{
	return handle(fs::low::create_file(L"CONIN$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr));
}

handle OpenConsoleActiveScreenBuffer()
{
	return handle(fs::low::create_file(L"CONOUT$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr));
}

	namespace rtdl
	{
		void module::module_deleter::operator()(HMODULE Module) const
		{
			FreeLibrary(Module);
		}

		HMODULE module::get_module() const noexcept
		{
			if (!m_tried && !m_module && !m_name.empty())
			{
				m_tried = true;
				m_module.reset(LoadLibrary(m_name.c_str()));

				if (!m_module && m_AlternativeLoad && IsAbsolutePath(m_name))
				{
					m_module.reset(LoadLibraryEx(m_name.c_str(), nullptr, LOAD_WITH_ALTERED_SEARCH_PATH));
				}
			}
			return m_module.get();
		}

		FARPROC module::get_proc_address(HMODULE Module, const char* Name) const
		{
			return Module? ::GetProcAddress(Module, Name) : nullptr;
		}
	}

	namespace uuid
	{
		UUID generate()
		{
			UUID Uuid;
			UuidCreate(&Uuid);
			return Uuid;
		}
	}

	namespace debug
	{
		bool debugger_present()
		{
			return IsDebuggerPresent() != FALSE;
		}

		void breakpoint(bool const Always)
		{
			if (Always || debugger_present())
				DebugBreak();
		}

		void print(const wchar_t* const Str)
		{
			OutputDebugString(Str);
		}

		void print(string const& Str)
		{
			print(Str.c_str());
		}

		void set_thread_name(const wchar_t* Name)
		{
			if (imports.SetThreadDescription)
				imports.SetThreadDescription(GetCurrentThread(), Name);
		}

		std::vector<uintptr_t> current_stack(size_t const FramesToSkip, size_t const FramesToCapture)
		{
			if (!imports.RtlCaptureStackBackTrace)
				return {};

			std::vector<uintptr_t> Stack;
			Stack.reserve(128);

			// http://web.archive.org/web/20140815000000*/http://msdn.microsoft.com/en-us/library/windows/hardware/ff552119(v=vs.85).aspx
			// In Windows XP and Windows Server 2003, the sum of the FramesToSkip and FramesToCapture parameters must be less than 63.
			static const auto Limit = IsWindowsVistaOrGreater()? std::numeric_limits<size_t>::max() : 62;

			const auto Skip = FramesToSkip + 1; // 1 for this frame
			const auto Capture = std::min(FramesToCapture, Limit - Skip);

			for (size_t i = 0; i != FramesToCapture;)
			{
				void* Pointers[128];

				DWORD DummyHash;
				const auto Size = imports.RtlCaptureStackBackTrace(
					static_cast<DWORD>(Skip + i),
					static_cast<DWORD>(std::min(std::size(Pointers), Capture - i)),
					Pointers,
					&DummyHash // MSDN says it's optional, but it's not true on Win2k
				);

				if (!Size)
					break;

				std::transform(Pointers, Pointers + Size, std::back_inserter(Stack), [](void* Ptr)
				{
					return reinterpret_cast<uintptr_t>(Ptr);
				});

				i += Size;
			}

			return Stack;
		}
	}
}

#ifdef ENABLE_TESTS

#include "testing.hpp"

TEST_CASE("platform.string.receiver")
{
	const auto api_function = [](size_t const EmulatedSize, wchar_t* const Buffer, size_t const BufferSize)
	{
		string Data;
		Data.resize(EmulatedSize);
		std::iota(ALL_RANGE(Data), L'\1');

		if (BufferSize < Data.size() + 1)
			return Data.size() + 1;

		*copy_string(Data, Buffer) = {};

		return Data.size();
	};

	const auto validate = [](string const& Data)
	{
		for (size_t i = 0, size = Data.size(); i != size; ++i)
		{
			if (Data[i] != i + 1)
				return false;
		}

		return true;
	};

	static size_t const Tests[]
	{
		0,
		1,
		42,
		os::default_buffer_size - 1,
		os::default_buffer_size,
		os::default_buffer_size + 1,
		420,
	};

	for (const auto& i: Tests)
	{
		string Data;
		REQUIRE((i != 0) == os::detail::ApiDynamicStringReceiver(Data, [&](span<wchar_t> const Buffer)
		{
			return api_function(i, Buffer.data(), Buffer.size());
		}));

		REQUIRE(validate(Data));
	}
}
#endif
