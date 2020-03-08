﻿/*
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

// Self:
#include "platform.hpp"

// Internal:
#include "imports.hpp"
#include "pathmix.hpp"
#include "string_utils.hpp"
#include "lasterror.hpp"
#include "exception.hpp"

// Platform:
#include "platform.fs.hpp"
#include "platform.memory.hpp"

// Common:
#include "common/range.hpp"
#include "common/scope_exit.hpp"
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

		void handle_implementation::wait(HANDLE const Handle)
		{
			WaitForSingleObject(Handle, INFINITE);
		}

		bool handle_implementation::is_signaled(HANDLE const Handle, std::chrono::milliseconds const Timeout)
		{
			const auto Result = WaitForSingleObject(Handle, Timeout / 1ms);
			switch (Result)
			{
			case WAIT_OBJECT_0:
				return true;

			case WAIT_TIMEOUT:
				return false;

			default:
				throw MAKE_FAR_FATAL_EXCEPTION(format(FSTR(L"WaitForSingleobject returned {0}"), Result));
			}
		}

		void handle_closer::operator()(HANDLE Handle) const
		{
			CloseHandle(Handle);
		}

		void printer_handle_closer::operator()(HANDLE Handle) const
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

NTSTATUS GetLastNtStatus()
{
	return imports.RtlGetLastNtStatus? imports.RtlGetLastNtStatus() : STATUS_SUCCESS;
}

string GetErrorString(bool Nt, DWORD Code)
{
	memory::local::ptr<wchar_t> Buffer;
	const size_t Size = FormatMessage((Nt? FORMAT_MESSAGE_FROM_HMODULE : FORMAT_MESSAGE_FROM_SYSTEM) | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS, (Nt? GetModuleHandle(L"ntdll.dll") : nullptr), Code, 0, reinterpret_cast<wchar_t*>(&ptr_setter(Buffer)), 0, nullptr);
	string Result(Buffer.get(), Size);
	std::replace_if(ALL_RANGE(Result), IsEol, L' ');
	inplace::trim_right(Result);
	return Result;
}

bool WNetGetConnection(const string_view LocalName, string &RemoteName)
{
	auto Buffer = os::buffer<wchar_t>();
	// MSDN says that call can fail with ERROR_NOT_CONNECTED or ERROR_CONNECTION_UNAVAIL if calling application
	// is running in a different logon session than the application that made the connection.
	// However, it may fail with ERROR_NOT_CONNECTED for non-network too, in this case Buffer will not be initialised.
	// Deliberately initialised with empty string to fix that.
	Buffer[0] = L'\0';
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

void EnableLowFragmentationHeap()
{
	// Starting with Windows Vista, the system uses the low-fragmentation heap (LFH) as needed to service memory allocation requests.
	// Applications do not need to enable the LFH for their heaps.
	if (IsWindowsVistaOrGreater())
		return;

	if (!imports.HeapSetInformation)
		return;

	std::vector<HANDLE> Heaps(10);
	for (;;)
	{
		const auto NumberOfHeaps = ::GetProcessHeaps(static_cast<DWORD>(Heaps.size()), Heaps.data());
		const auto Received = NumberOfHeaps <= Heaps.size();
		Heaps.resize(NumberOfHeaps);
		if (Received)
			break;
	}

	for (const auto i: Heaps)
	{
		ULONG HeapFragValue = 2;
		imports.HeapSetInformation(i, HeapCompatibilityInformation, &HeapFragValue, sizeof(HeapFragValue));
	}
}

bool get_locale_value(LCID const LcId, LCTYPE const Id, string& Value)
{
	return detail::ApiDynamicErrorBasedStringReceiver(ERROR_INSUFFICIENT_BUFFER, Value, [&](span<wchar_t> Buffer)
	{
		const auto ReturnedSize = GetLocaleInfo(LcId, Id, Buffer.data(), static_cast<int>(Buffer.size()));
		return ReturnedSize? ReturnedSize - 1 : 0;
	});
}

bool get_locale_value(LCID const LcId, LCTYPE const Id, int& Value)
{
	return GetLocaleInfo(LcId, Id | LOCALE_RETURN_NUMBER, reinterpret_cast<wchar_t*>(&Value), sizeof(Value) / sizeof(wchar_t)) != 0;
}

string GetPrivateProfileString(string_view const AppName, string_view const KeyName, string_view const Default, string_view const FileName)
{
	string Value;

	(void)detail::ApiDynamicStringReceiver(Value, [&](span<wchar_t> const Buffer)
	{
		const auto Size = ::GetPrivateProfileString(null_terminated(AppName).c_str(), null_terminated(KeyName).c_str(), null_terminated(Default).c_str(), Buffer.data(), static_cast<DWORD>(Buffer.size()), null_terminated(FileName).c_str());
		return Size == Buffer.size() - 1? Buffer.size() * 2 : Size;
	});

	return Value;
}

bool GetWindowText(HWND Hwnd, string& Text)
{
	// GetWindowText[Length] might return 0 not only in case of failure, but also when the window title is empty.
	// To recognise this, we set LastError to ERROR_SUCCESS manually and check it after the call,
	// which doesn't change it upon success.
	GuardLastError ErrorGuard;
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

bool IsWow64Process()
{
#ifdef _WIN64
	return false;
#else
	static const auto Wow64Process = []{ BOOL Value = FALSE; return imports.IsWow64Process(GetCurrentProcess(), &Value) && Value; }();
	return Wow64Process;
#endif
}

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
	return os::handle(DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &Handle, 0, FALSE, DUPLICATE_SAME_ACCESS) ? Handle : nullptr);
}

handle OpenConsoleInputBuffer()
{
	return handle(fs::low::create_file(L"CONIN$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr));
}

handle OpenConsoleActiveScreenBuffer()
{
	return handle(fs::low::create_file(L"CONOUT$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr));
}

	namespace com
	{
		initialize::initialize():
			m_Initialised(SUCCEEDED(CoInitializeEx(nullptr, COINIT_MULTITHREADED)))
		{
		}

		initialize::~initialize()
		{
			if (m_Initialised)
				CoUninitialize();
		}

		void detail::memory_releaser::operator()(const void* Object) const
		{
			CoTaskMemFree(const_cast<void*>(Object));
		}
	}

	namespace rtdl
	{
		void module::module_deleter::operator()(HMODULE Module) const
		{
			FreeLibrary(Module);
		}

		HMODULE module::get_module() const
		{
			if (!m_tried && !m_module && !m_name.empty())
			{
				m_tried = true;
				m_module.reset(LoadLibrary(m_name.c_str()));

				if (!m_module && m_AlternativeLoad && IsAbsolutePath(m_name))
				{
					m_module.reset(LoadLibraryEx(m_name.c_str(), nullptr, LOAD_WITH_ALTERED_SEARCH_PATH));
				}
				// TODO: log if nullptr
			}
			return m_module.get();
		}
	}
}

UUID CreateUuid()
{
	UUID Uuid;
	UuidCreate(&Uuid);
	return Uuid;
}

string GuidToStr(const GUID& Guid)
{
	RPC_WSTR Str;
	// declared as non-const in GCC headers :(
	if (UuidToString(const_cast<GUID*>(&Guid), &Str) != RPC_S_OK)
		throw std::bad_alloc{};

	SCOPE_EXIT{ RpcStringFree(&Str); };
	return upper(string_view{ reinterpret_cast<const wchar_t*>(Str) });
}

bool StrToGuid(string_view const Value, GUID& Guid)
{
	return UuidFromString(reinterpret_cast<RPC_WSTR>(const_cast<wchar_t*>(null_terminated(Value).c_str())), &Guid) == RPC_S_OK;
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

		*std::copy(ALL_CONST_RANGE(Data), Buffer) = L'\0';

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
