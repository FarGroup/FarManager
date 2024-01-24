#ifndef SCOPE_EXIT_HPP_EDB9D84F_7B9F_408C_8FC8_94626C4B3CE3
#define SCOPE_EXIT_HPP_EDB9D84F_7B9F_408C_8FC8_94626C4B3CE3
#pragma once

/*
scope_exit.hpp
*/
/*
Copyright © 2014 Far Group
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

#include "preprocessor.hpp"

#include <exception>
#include <stdexcept>

//----------------------------------------------------------------------------

namespace scope_exit
{
	enum class scope_type
	{
		exit,
		fail,
		success
	};

	template<scope_type Type>
	class exception_monitor
	{
	public:
		[[nodiscard]]
		bool invoke_handler() const noexcept { return (Type == scope_type::fail) == (std::uncaught_exceptions() > m_Count); }

		int m_Count{ std::uncaught_exceptions() }; // int... "a camel is a horse designed by a committee" :(
	};

	template<>
	class exception_monitor<scope_type::exit>
	{
	public:
		[[nodiscard]]
		constexpr static bool invoke_handler() noexcept { return true; }
	};

	template<scope_type Type, typename F>
	class scope_guard
	{
	public:
		NONCOPYABLE(scope_guard);

		explicit scope_guard(F&& f): m_f(FWD(f)) {}

		~scope_guard() noexcept(Type == scope_type::fail)
		{
			if (m_Monitor.invoke_handler())
				m_f();
		}

	private:
		F m_f;
		exception_monitor<Type> m_Monitor;
	};

	template<scope_type Type>
	class make_scope_guard
	{
	public:
		template<typename F>
		[[nodiscard]]
		auto operator<<(F&& f) { return scope_guard<Type, F>(FWD(f)); }
	};
}

#define SCOPE_TYPE(type) \
	const auto ANONYMOUS_VARIABLE(scope_guard) = scope_exit::make_scope_guard<type>() << [&]() /* lambda body here */

#define SCOPE_EXIT    SCOPE_TYPE(scope_exit::scope_type::exit)
#define SCOPE_FAIL    SCOPE_TYPE(scope_exit::scope_type::fail) noexcept
#define SCOPE_SUCCESS SCOPE_TYPE(scope_exit::scope_type::success)

#endif // SCOPE_EXIT_HPP_EDB9D84F_7B9F_408C_8FC8_94626C4B3CE3
