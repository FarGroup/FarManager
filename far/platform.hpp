#ifndef PLATFORM_HPP_632CB91D_08A9_4793_8FC7_2E38C30CE234
#define PLATFORM_HPP_632CB91D_08A9_4793_8FC7_2E38C30CE234
#pragma once

/*
platform.hpp

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

namespace os
{
	enum
	{
		default_buffer_size = MAX_PATH
	};

	enum
	{
		NT_MAX_PATH = 32768
	};

	namespace detail
	{
		template<typename buffer_type>
		auto default_buffer()
		{
			return array_ptr<buffer_type, default_buffer_size>(default_buffer_size);
		}

		template<typename buffer_type, typename receiver, typename condition, typename assigner>
		bool ApiDynamicReceiver(buffer_type&& Buffer, const receiver& Receiver, const condition& Condition, const assigner& Assigner)
		{
			auto Size = Receiver(Buffer.get(), Buffer.size());

			while (Condition(Size, Buffer.size()))
			{
				Buffer.reset(Size? Size : Buffer.size() * 2);
				Size = Receiver(Buffer.get(), Buffer.size());
			}

			if (!Size)
				return false;

			Assigner(Buffer.get(), Size);
			return true;
		}

		template<typename T>
		bool ApiDynamicStringReceiver(string& Destination, const T& Callable)
		{
			return ApiDynamicReceiver(
				default_buffer<wchar_t>(),
				Callable,
				[](size_t ReturnedSize, size_t AllocatedSize)
				{
					// Why such condition?
					// Usually API functions return string length (without \0) on success and
					// required buffer size (i. e. string length + \0) on failure.
					// Some of them, however, always return buffer size.
					// It's Callable's responsibility to handle and fix that.
					return ReturnedSize >= AllocatedSize;
				},
				[&](const wchar_t* Buffer, size_t Size)
				{
					Destination.assign(Buffer, Size);
				});
		}

		template<typename T>
		bool ApiDynamicErrorBasedStringReceiver(DWORD ExpectedErrorCode, string& Destination, const T& Callable)
		{
			return ApiDynamicReceiver(
				default_buffer<wchar_t>(),
				Callable,
				[&](size_t ReturnedSize, size_t AllocatedSize)
				{
					return !ReturnedSize && GetLastError() == ExpectedErrorCode;
				},
				[&](const wchar_t* Buffer, size_t Size)
				{
					Destination.assign(Buffer, Size);
				});
		}

		template<class deleter>
		class handle_t: public base<std::unique_ptr<std::remove_pointer_t<HANDLE>, deleter>>
		{
			using base_type = typename handle_t::base_type;

		public:
			MOVABLE(handle_t);

			constexpr handle_t() = default;
			constexpr handle_t(std::nullptr_t) {}
			explicit handle_t(HANDLE Handle): base_type(normalise(Handle)) {}
			void reset(HANDLE Handle = nullptr) { base_type::reset(normalise(Handle)); }
			auto native_handle() const { return base_type::get(); }
			void close() { reset(); }
			bool wait(std::chrono::milliseconds Timeout) const { return WaitForSingleObject(native_handle(), Timeout.count()) == WAIT_OBJECT_0; }
			bool wait() const { return WaitForSingleObject(native_handle(), INFINITE) == WAIT_OBJECT_0; }
			bool is_signaled() const { return wait(0ms); }

		private:
			static HANDLE normalise(HANDLE Handle) { return Handle == INVALID_HANDLE_VALUE? nullptr : Handle; }
		};

		struct handle_closer
		{
			void operator()(HANDLE Handle) const;
		};

		struct printer_handle_closer
		{
			void operator()(HANDLE Handle) const;
		};

		struct sid_deleter
		{
			void operator()(PSID Sid) const noexcept
			{
				FreeSid(Sid);
			}
		};
	}

	using handle = detail::handle_t<detail::handle_closer>;

	using printer_handle = detail::handle_t<detail::printer_handle_closer>;

	NTSTATUS GetLastNtStatus();

	string GetErrorString(bool Nt, DWORD Code);

	bool WNetGetConnection(const string_view& LocalName, string &RemoteName);

	void EnableLowFragmentationHeap();

	string GetPrivateProfileString(const string& AppName, const string& KeyName, const string& Default, const string& FileName);

	bool GetWindowText(HWND Hwnd, string& Text);

	bool IsWow64Process();

	DWORD GetAppPathsRedirectionFlag();

	bool GetDefaultPrinter(string& Printer);

	bool GetComputerName(string& Name);
	bool GetComputerNameEx(COMPUTER_NAME_FORMAT NameFormat, string& Name);
	bool GetUserName(string& Name);
	bool GetUserNameEx(EXTENDED_NAME_FORMAT NameFormat, string& Name);


	string GetLocaleValue(LCID lcid, LCTYPE id);

	handle OpenCurrentThread();

	handle OpenConsoleInputBuffer();
	handle OpenConsoleActiveScreenBuffer();


	// Run-Time Dynamic Linking
	namespace rtdl
	{
		class module
		{
		public:
			NONCOPYABLE(module);
			MOVABLE(module);

			explicit module(string name, bool AlternativeLoad = false):
				m_name(std::move(name)),
				m_tried(),
				m_AlternativeLoad(AlternativeLoad)
			{}

			void* GetProcAddress(const char* name) const { return reinterpret_cast<void*>(::GetProcAddress(get_module(), name)); }
			explicit operator bool() const noexcept { return get_module() != nullptr; }

		private:
			HMODULE get_module() const;

			struct module_deleter { void operator()(HMODULE Module) const; };
			using module_ptr = std::unique_ptr<std::remove_pointer_t<HMODULE>, module_deleter>;

			string m_name;
			mutable module_ptr m_module;
			mutable bool m_tried;
			bool m_AlternativeLoad;
		};

		template<typename T>
		class function_pointer: noncopyable
		{
		public:
			function_pointer(const module& Module, const char* Name):
				m_module(Module),
				m_pointer(reinterpret_cast<T>(m_module.GetProcAddress(Name)))
			{}
			operator T() const { return m_pointer; }

		private:
			const module& m_module;
			mutable T m_pointer;
		};
	}

	namespace netapi
	{
		namespace detail
		{
			template<class T>
			struct deleter
			{
				void operator()(T* Ptr) const
				{
					NetApiBufferFree(Ptr);
				}
			};
		};

		template<class T>
		using ptr = std::unique_ptr<T, detail::deleter<T>>;
	}

	namespace com
	{
		class initialize: noncopyable
		{
		public:
			initialize(): m_hr(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED)) {}
			~initialize() { if (SUCCEEDED(m_hr)) CoUninitialize(); }

		private:
			const HRESULT m_hr;
		};

		namespace detail
		{
			template<typename T>
			struct releaser
			{
				void operator()(T* Object) const
				{
					Object->Release();
				}
			};
		}

		template<typename T>
		using ptr = std::unique_ptr<T, detail::releaser<T>>;
	}
}

UUID CreateUuid();
string GuidToStr(const GUID& Guid);
bool StrToGuid(const wchar_t* Value,GUID& Guid);
inline bool StrToGuid(const string& Value, GUID& Guid) { return StrToGuid(Value.data(), Guid); }

#endif // PLATFORM_HPP_632CB91D_08A9_4793_8FC7_2E38C30CE234
