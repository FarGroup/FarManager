#ifndef EXCEPTION_HPP_2CD5B7D1_D39C_4CAF_858A_62496C9221DF
#define EXCEPTION_HPP_2CD5B7D1_D39C_4CAF_858A_62496C9221DF
#pragma once

/*
exception.hpp
*/
/*
Copyright © 2016 Far Group
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
#include "platform.hpp"

// Common:

// External:

//----------------------------------------------------------------------------

struct error_state
{
	static error_state fetch();

	int Errno = 0;
	DWORD Win32Error = ERROR_SUCCESS;
	NTSTATUS NtError = STATUS_SUCCESS;

	string ErrnoStr() const;
	string Win32ErrorStr() const;
	string NtErrorStr() const;

	std::array<string, 3> format_errors() const;
};

struct error_state_ex: public error_state
{
	error_state_ex() = default;

	error_state_ex(const error_state& State, string_view const Message = {}):
		error_state(State),
		What(Message)
	{
	}

	string format_error() const;

	string What;
};

namespace detail
{
	class far_base_exception: public error_state_ex
	{
	public:
		[[nodiscard]] const auto& message() const noexcept { return What; }
		[[nodiscard]] const auto& full_message() const noexcept { return m_FullMessage; }
		[[nodiscard]] const auto& function() const noexcept { return m_Function; }
		[[nodiscard]] const auto& location() const noexcept { return m_Location; }

	protected:
		far_base_exception(string_view Message, const char* Function, string_view File, int Line);

	private:
		std::string m_Function;
		string m_Location;
		string m_FullMessage;
	};

	class far_std_exception : public far_base_exception, public std::runtime_error
	{
	public:
		template<typename... args>
		explicit far_std_exception(args&&... Args):
			far_base_exception(FWD(Args)...),
			std::runtime_error(convert_message())
		{}

	private:
		[[nodiscard]] std::string convert_message() const;
	};

	class break_into_debugger
	{
	protected:
		break_into_debugger();
	};
}

/*
  Represents a non-continuable failure:
  - logic errors, which shouldn't happen
  - fatal OS errors
  - ...
  I.e. we either don't really know what to do or doing anything will do more harm than good.
  It shouldn't be caught explicitly in general and fly straight to main().
*/
class far_fatal_exception: private detail::break_into_debugger, public detail::far_std_exception
{
	using far_std_exception::far_std_exception;
};

/*
  Represents all other failures, potentially continuable.
  Base class for more specific exceptions.
*/
class far_exception: public detail::far_std_exception
{
	using far_std_exception::far_std_exception;
};

/*
  For the cases where it is pretty clear what is wrong, no need to show the stack etc.
 */
class far_known_exception: public far_exception
{
	using far_exception::far_exception;
};

#define MAKE_EXCEPTION(ExceptionType, ...) ExceptionType(__VA_ARGS__, __FUNCTION__, WIDE_SV(__FILE__), __LINE__)
#define MAKE_FAR_FATAL_EXCEPTION(...) MAKE_EXCEPTION(far_fatal_exception, __VA_ARGS__)
#define MAKE_FAR_EXCEPTION(...) MAKE_EXCEPTION(far_exception, __VA_ARGS__)
#define MAKE_FAR_KNOWN_EXCEPTION(...) MAKE_EXCEPTION(far_known_exception, __VA_ARGS__)

#endif // EXCEPTION_HPP_2CD5B7D1_D39C_4CAF_858A_62496C9221DF
