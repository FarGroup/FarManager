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

namespace scope_exit
{
	class uncaught_exceptions_counter
	{
	public:
		bool is_new() const noexcept { return std::uncaught_exceptions() > m_Count; }
		int m_Count{ std::uncaught_exceptions() }; // int... "a camel is a horse designed by a committee" :(
	};

	enum class scope_type
	{
		exit,
		fail,

		success
	};

	template<typename F, scope_type Type>
	class scope_guard
	{
	public:
		NONCOPYABLE(scope_guard);
		TRIVIALLY_MOVABLE(scope_guard);

		explicit scope_guard(F&& f): m_f(std::forward<F>(f)) {}

		~scope_guard() noexcept(Type == scope_type::fail)
		{
			if (*m_Active && (Type == scope_type::exit || (Type == scope_type::fail) == m_Ec.is_new()))
				m_f();
		}

	private:
		F m_f;
		movable<bool> m_Active{ true };
		uncaught_exceptions_counter m_Ec;
	};

	template<scope_type Type>
	class make_scope_guard
	{
	public:
		template<typename F>
		auto operator << (F&& f) { return scope_guard<F, Type>(std::forward<F>(f)); }
	};
}

#define DETAIL_SCOPE_IMPL(type) \
const auto ANONYMOUS_VARIABLE(scope_##type##_guard) = scope_exit::make_scope_guard<scope_exit::scope_type::type>() << [&]() /* lambda body here */

#define SCOPE_EXIT DETAIL_SCOPE_IMPL(exit)
#define SCOPE_FAIL DETAIL_SCOPE_IMPL(fail) noexcept
#define SCOPE_SUCCESS DETAIL_SCOPE_IMPL(success)

#endif // SCOPE_EXIT_HPP_EDB9D84F_7B9F_408C_8FC8_94626C4B3CE3
