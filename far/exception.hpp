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
#include "common/source_location.hpp"

// External:

//----------------------------------------------------------------------------

struct error_state_ex: public os::error_state
{
	error_state_ex() = default;

	explicit(false) error_state_ex(const error_state& State, string_view const Message = {}, int Errno = 0):
		error_state(State),
		What(Message),
		Errno(Errno)
	{
	}

	[[nodiscard]]
	bool any() const
	{
		return Errno != 0 || error_state::any();
	}

	[[nodiscard]] string ErrnoStr() const;

	[[nodiscard]] string system_error() const;
	[[nodiscard]] string to_string() const;

	string What;
	int Errno{};
};

string source_location_to_string(source_location const& Location);

namespace detail
{
	class far_base_exception: public error_state_ex
	{
	public:
		[[nodiscard]] const auto& message() const noexcept { return What; }
		[[nodiscard]] const auto& full_message() const noexcept { return m_FullMessage; }
		[[nodiscard]] const auto& location() const noexcept { return m_Location; }
		[[nodiscard]] string to_string() const;

	protected:
		explicit far_base_exception(error_state_ex ErrorState, source_location const& Location);

	private:
		source_location m_Location;
		string m_FullMessage;
	};

	class far_std_exception : public far_base_exception, public std::runtime_error
	{
	public:
		explicit far_std_exception(error_state_ex ErrorState, source_location const& Location = source_location::current()):
			far_base_exception(std::move(ErrorState), Location),
			std::runtime_error(convert_message(full_message()))
		{
		}

		explicit far_std_exception(string_view Message, bool CaptureErrors = true, source_location const& Location = source_location::current());

	private:
		[[nodiscard]] static std::string convert_message(string_view Message);
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
class far_fatal_exception final: private detail::break_into_debugger, public detail::far_std_exception
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
class far_known_exception final: public far_exception
{
public:
	explicit far_known_exception(string_view const Message):
		far_exception(Message, false, {})
	{
	}
};

template<typename T>
struct formattable;

template<>
struct formattable<std::exception>
{
	static string to_string(std::exception const& e);
};

constexpr inline struct unknown_exception_t
{
	static string to_string();
}
unknown_exception;

#endif // EXCEPTION_HPP_2CD5B7D1_D39C_4CAF_858A_62496C9221DF
