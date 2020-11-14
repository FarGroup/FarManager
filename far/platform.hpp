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

// Internal:

// Platform:

// Common:
#include "common/noncopyable.hpp"
#include "common/smart_ptr.hpp"
#include "common/utility.hpp"

// External:

//----------------------------------------------------------------------------

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

	template<typename buffer_type>
	auto buffer()
	{
		return array_ptr<buffer_type, default_buffer_size>(default_buffer_size);
	}

	namespace detail
	{
		template<typename buffer_type, typename receiver, typename condition, typename assigner>
		[[nodiscard]]
		bool ApiDynamicReceiver(buffer_type&& Buffer, const receiver& Receiver, const condition& Condition, const assigner& Assigner)
		{
			size_t Size = Receiver(Buffer);

			while (Condition(Size, Buffer.size()))
			{
				Buffer.reset(Size? Size : Buffer.size() * 2);
				Size = Receiver(Buffer);
			}

			if (!Size)
				return false;

			Assigner({ Buffer.data(), Size });
			return true;
		}

		template<typename T>
		[[nodiscard]]
		bool ApiDynamicStringReceiver(string& Destination, const T& Callable)
		{
			return ApiDynamicReceiver(
				buffer<wchar_t>(),
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
				[&](string_view const Buffer)
				{
					Destination = Buffer;
				});
		}

		template<typename T>
		[[nodiscard]]
		bool ApiDynamicErrorBasedStringReceiver(DWORD ExpectedErrorCode, string& Destination, const T& Callable)
		{
			return ApiDynamicReceiver(
				buffer<wchar_t>(),
				Callable,
				[&](size_t ReturnedSize, size_t AllocatedSize)
				{
					return !ReturnedSize && GetLastError() == ExpectedErrorCode;
				},
				[&](string_view const Buffer)
				{
					Destination = Buffer;
				});
		}

		class handle_implementation
		{
		protected:
			[[nodiscard]]
			static HANDLE normalise(HANDLE Handle);
			[[nodiscard]]
			static bool wait(HANDLE Handle, std::optional<std::chrono::milliseconds> Timeout = {});
			[[nodiscard]]
			static std::optional<size_t> wait(span<HANDLE const> Handles, bool WaitAll, std::optional<std::chrono::milliseconds> Timeout = {});
		};

		template<class deleter>
		class handle_t: handle_implementation, public base<std::unique_ptr<std::remove_pointer_t<HANDLE>, deleter>>
		{
		public:
			MOVABLE(handle_t);

			constexpr handle_t() = default;

			constexpr handle_t(std::nullptr_t)
			{
			}

			explicit handle_t(HANDLE Handle):
				handle_t::base_ctor(normalise(Handle))
			{
			}

			void reset(HANDLE Handle)
			{
				handle_t::base_type::reset(normalise(Handle));
			}

			[[nodiscard]]
			HANDLE native_handle() const
			{
				return handle_t::base_type::get();
			}

			void close()
			{
				reset(nullptr);
			}

			void wait() const
			{
				(void)handle_implementation::wait(native_handle());
			}

			[[nodiscard]]
			bool is_signaled(std::chrono::milliseconds Timeout = 0ms) const
			{
				return handle_implementation::wait(native_handle(), Timeout);
			}

			[[nodiscard]]
			static bool is_signaled(HANDLE const Handle, std::chrono::milliseconds const Timeout)
			{
				return handle_implementation::wait(Handle, Timeout);
			}

			[[nodiscard]]
			static auto wait_any(span<HANDLE const> const Handles, std::chrono::milliseconds const Timeout)
			{
				return handle_implementation::wait(Handles, false, Timeout);
			}

			[[nodiscard]]
			static auto wait_any(span<HANDLE const> const Handles)
			{
				return *handle_implementation::wait(Handles, false);
			}

			[[nodiscard]]
			static auto wait_all(span<HANDLE const> const Handles, std::chrono::milliseconds const Timeout)
			{
				return handle_implementation::wait(Handles, true, Timeout);
			}

			[[nodiscard]]
			static auto wait_all(span<HANDLE const> const Handles)
			{
				return *handle_implementation::wait(Handles, true);
			}
		};

		struct handle_closer
		{
			void operator()(HANDLE Handle) const noexcept;
		};

		struct printer_handle_closer
		{
			void operator()(HANDLE Handle) const noexcept;
		};
	}

	using handle = detail::handle_t<detail::handle_closer>;
	using printer_handle = detail::handle_t<detail::printer_handle_closer>;

	void set_error_mode(unsigned Mask);
	void unset_error_mode(unsigned Mask);

	[[nodiscard]]
	NTSTATUS GetLastNtStatus();

	[[nodiscard]]
	string GetErrorString(bool Nt, DWORD Code);

	string format_system_error(unsigned int ErrorCode, string_view ErrorMessage);

	class last_error_guard
	{
	public:
		NONCOPYABLE(last_error_guard);

		last_error_guard();
		~last_error_guard();

		void dismiss();

	private:
		DWORD m_LastError;
		NTSTATUS m_LastStatus;
		bool m_Active;
	};


	bool WNetGetConnection(string_view LocalName, string &RemoteName);

	[[nodiscard]]
	string GetPrivateProfileString(string_view AppName, string_view KeyName, string_view Default, string_view FileName);

	bool GetWindowText(HWND Hwnd, string& Text);

	[[nodiscard]]
#ifdef _WIN64
	constexpr bool IsWow64Process() { return false; }
#else
	bool IsWow64Process();
#endif

	[[nodiscard]]
	DWORD GetAppPathsRedirectionFlag();

	[[nodiscard]]
	bool GetDefaultPrinter(string& Printer);

	[[nodiscard]]
	bool GetComputerName(string& Name);

	[[nodiscard]]
	bool GetComputerNameEx(COMPUTER_NAME_FORMAT NameFormat, string& Name);

	[[nodiscard]]
	bool GetUserName(string& Name);

	[[nodiscard]]
	bool GetUserNameEx(EXTENDED_NAME_FORMAT NameFormat, string& Name);

	[[nodiscard]]
	bool get_locale_value(LCID LcId, LCTYPE Id, string& Value);

	[[nodiscard]]
	bool get_locale_value(LCID LcId, LCTYPE Id, int& Value);

	[[nodiscard]]
	handle OpenCurrentThread();

	[[nodiscard]]
	handle OpenConsoleInputBuffer();

	[[nodiscard]]
	handle OpenConsoleActiveScreenBuffer();


	// Run-Time Dynamic Linking
	namespace rtdl
	{
		class module
		{
		public:
			NONCOPYABLE(module);
			MOVABLE(module);

			explicit module(string_view const Name, bool AlternativeLoad = false):
				m_name(Name),
				m_tried(),
				m_AlternativeLoad(AlternativeLoad)
			{}

			template<typename T>
			[[nodiscard]]
			T GetProcAddress(const char* name) const { return reinterpret_cast<T>(reinterpret_cast<void*>(::GetProcAddress(get_module(), name))); }

			[[nodiscard]]
			explicit operator bool() const noexcept { return get_module() != nullptr; }

		private:
			[[nodiscard]]
			HMODULE get_module() const noexcept;

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
				m_Module(&Module),
				m_Name(Name)
			{}

			[[nodiscard]]
			operator T() const { return get_pointer(); }

			[[nodiscard]]
			explicit operator bool() const noexcept { return get_pointer() != nullptr; }

			[[nodiscard]]
			std::string_view name() const { return m_Name; }

		private:
			[[nodiscard]]
			T get_pointer() const
			{
				if (!m_Tried)
				{
					m_Pointer = m_Module->GetProcAddress<T>(m_Name);
					m_Tried = true;
				}

				return m_Pointer;
			}

			const module* m_Module;
			const char* m_Name;
			mutable T m_Pointer{};
			mutable bool m_Tried{};
		};
	}

	namespace netapi
	{
		namespace detail
		{
			template<class T>
			struct deleter
			{
				void operator()(T* Ptr) const noexcept
				{
					NetApiBufferFree(Ptr);
				}
			};
		}

		template<class T>
		using ptr = std::unique_ptr<T, detail::deleter<T>>;
	}

	namespace com
	{
		class initialize: noncopyable
		{
		public:
			initialize();
			~initialize();

		private:
			const bool m_Initialised;
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

			struct memory_releaser
			{
				void operator()(const void* Object) const;
			};
		}

		template<typename T>
		using ptr = std::unique_ptr<T, detail::releaser<T>>;

		template<typename T>
		using memory = std::unique_ptr<std::remove_pointer_t<T>, detail::memory_releaser>;
	}

	namespace uuid
	{
		[[nodiscard]]
		UUID generate();
	}

	namespace debug
	{
		bool debugger_present();
		void breakpoint(bool Always = true);
		void print(const wchar_t* Str);
		void print(string const& Str);
	}
}

#endif // PLATFORM_HPP_632CB91D_08A9_4793_8FC7_2E38C30CE234
