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
#include "common/function_ref.hpp"
#include "common/smart_ptr.hpp"
#include "common/span.hpp"
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
		template<typename buffer_type>
		[[nodiscard]]
		bool ApiDynamicReceiver(
			buffer_type&& Buffer,
			function_ref<size_t(std::span<std::ranges::range_value_t<buffer_type>> WritableBuffer)> const Receiver,
			function_ref<bool(size_t ReturnedSize, size_t AllocatedSize)> const Condition,
			function_ref<void(std::span<std::ranges::range_value_t<buffer_type> const> ReadableBuffer)> const Assigner
		)
		{
			size_t Size = Receiver({ Buffer.data(), Buffer.size() });

			while (Condition(Size, Buffer.size()))
			{
				Buffer.reset(Size? Size : Buffer.size() * 2);
				Size = Receiver({ Buffer.data(), Buffer.size() });
			}

			if (!Size)
				return false;

			Assigner({ Buffer.data(), Size });
			return true;
		}

		[[nodiscard]]
		bool ApiDynamicStringReceiver(string& Destination, function_ref<size_t(std::span<wchar_t> WritableBuffer)> Callable);

		[[nodiscard]]
		bool ApiDynamicErrorBasedStringReceiver(DWORD ExpectedErrorCode, string& Destination, function_ref<size_t(std::span<wchar_t> WritableBuffer)> Callable);

		class handle_implementation
		{
		public:
			static void wait(HANDLE Handle);

			[[nodiscard]]
			static bool is_signaled(HANDLE Handle, std::chrono::milliseconds Timeout = 0ms);

			[[nodiscard]]
			static std::optional<size_t> wait_any(span<HANDLE const> Handles, std::optional<std::chrono::milliseconds> Timeout);

			[[nodiscard]]
			static size_t wait_any(span<HANDLE const> Handles);

			[[nodiscard]]
			static bool wait_all(span<HANDLE const> Handles, std::optional<std::chrono::milliseconds> Timeout);

			static void wait_all(span<HANDLE const> Handles);

		protected:
			[[nodiscard]]
			static HANDLE normalise(HANDLE Handle);
		};

		template<class deleter>
		class handle_t: public handle_implementation, public base<std::unique_ptr<std::remove_pointer_t<HANDLE>, deleter>>
		{
		public:
			MOVABLE(handle_t);

			constexpr handle_t() = default;

			constexpr explicit(false) handle_t(std::nullptr_t)
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

			using handle_implementation::wait;

			void wait() const
			{
				wait(native_handle());
			}

			using handle_implementation::is_signaled;

			[[nodiscard]]
			bool is_signaled(std::chrono::milliseconds Timeout = 0ms) const
			{
				return is_signaled(native_handle(), Timeout);
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
	NTSTATUS get_last_nt_status(void const* Teb);

	[[nodiscard]]
	DWORD get_last_error(void const* Teb);

	[[nodiscard]]
	NTSTATUS get_last_nt_status();

	void set_last_nt_status(NTSTATUS Status);
	void set_last_error_from_ntstatus(NTSTATUS Status);

	[[nodiscard]]
	string format_errno(int ErrorCode);

	[[nodiscard]]
	string format_error(DWORD ErrorCode);

	[[nodiscard]]
	string format_ntstatus(NTSTATUS Status);

	struct error_state
	{
		DWORD Win32Error = ERROR_SUCCESS;
		NTSTATUS NtError = STATUS_SUCCESS;

		[[nodiscard]]
		bool any() const
		{
			return Win32Error != ERROR_SUCCESS || !NT_SUCCESS(NtError);
		}

		[[nodiscard]] string Win32ErrorStr() const;
		[[nodiscard]] string NtErrorStr() const;

		[[nodiscard]] string to_string() const;
	};

	error_state last_error();

	class last_error_guard
	{
	public:
		NONCOPYABLE(last_error_guard);

		last_error_guard();
		~last_error_guard();

		void dismiss();

	private:
		std::optional<error_state> m_Error;
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

			explicit module(string_view Name, bool AlternativeLoad = false);

			template<typename T>
			[[nodiscard]]
			T GetProcAddress(const char* name) const
			{
				return std::bit_cast<T>(get_proc_address(name));
			}

			[[nodiscard]]
			explicit operator bool() const noexcept;

			[[nodiscard]]
			const string& name() const;

		private:
			[[nodiscard]]
			HMODULE get_module(bool Mandatory) const;

			FARPROC get_proc_address(const char* Name) const;

			struct module_deleter
			{
				void operator()(HMODULE Module) const;
			};

			using module_ptr = std::unique_ptr<std::remove_pointer_t<HMODULE>, module_deleter>;

			string m_name;
			mutable std::optional<module_ptr> m_module;
			bool m_AlternativeLoad;
		};

		class opaque_function_pointer
		{
		public:
			NONCOPYABLE(opaque_function_pointer);

			opaque_function_pointer(const module& Module, const char* Name);

			[[nodiscard]]
			explicit operator bool() const noexcept;

			[[nodiscard]]
			std::string_view name() const { return m_Name; }

		protected:
			[[nodiscard]]
			void* get_pointer(bool Mandatory) const;

		private:
			const module* m_Module;
			const char* m_Name;
			mutable std::optional<void*> m_Pointer;
		};

		template<typename T>
		class function_pointer: public opaque_function_pointer
		{
			using raw_function_pointer = std::conditional_t<std::is_pointer_v<T>, T, T*>;

		public:
			using opaque_function_pointer::opaque_function_pointer;

			[[nodiscard]]
			explicit(false) operator raw_function_pointer() const { return std::bit_cast<raw_function_pointer>(get_pointer(true)); }
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

	namespace uuid
	{
		[[nodiscard]]
		UUID generate();
	}

	HKL make_hkl(int32_t Layout);
	HKL make_hkl(string_view LayoutStr);
	std::vector<HKL> get_keyboard_layout_list();

	int to_unicode(unsigned VirtKey, unsigned ScanCode, BYTE const* KeyState, span<wchar_t> Buffer, unsigned Flags, HKL Hkl);
	bool is_dead_key(KEY_EVENT_RECORD const& Key, HKL Layout);

	bool is_interactive_user_session();
}

#endif // PLATFORM_HPP_632CB91D_08A9_4793_8FC7_2E38C30CE234
