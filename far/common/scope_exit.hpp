#ifndef SCOPE_EXIT_HPP_EDB9D84F_7B9F_408C_8FC8_94626C4B3CE3
#define SCOPE_EXIT_HPP_EDB9D84F_7B9F_408C_8FC8_94626C4B3CE3
#pragma once

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
	template<typename F>
	class guard
	{
	public:
		guard(const F& f) : m_f(f) {}
		~guard() { m_f(); }

	private:
		const F m_f;
	};

	class make_guard
	{
	public:
		template<typename F>
		guard<F> operator << (const F &f) { return guard<F>(f); }
	};

};

#define SCOPE_EXIT \
const auto ADD_SUFFIX(scope_exit_guard_, __LINE__) = scope_exit::make_guard() << [&]() /* lambda body here */

#endif // SCOPE_EXIT_HPP_EDB9D84F_7B9F_408C_8FC8_94626C4B3CE3
