#ifndef POLYFILLS_HPP_95E41B70_5DB2_4E5B_A468_95343C6438AD
#define POLYFILLS_HPP_95E41B70_5DB2_4E5B_A468_95343C6438AD
#pragma once

/*
polyfills.hpp

Emulation of C++ features missing in supported compilers & libraries

Here be dragons
*/
/*
Copyright © 2013 Far Group
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

#include "compiler.hpp"

#include <version>

//----------------------------------------------------------------------------

#if COMPILER(GCC) && !defined(_GLIBCXX_HAS_GTHREADS)

namespace std::this_thread
{
	inline void yield() noexcept
	{
		Sleep(0);
	}
}
#endif

//----------------------------------------------------------------------------

#ifndef __cpp_size_t_suffix

WARNING_PUSH()
WARNING_DISABLE_MSC(4455) // 'operator operator': literal suffix identifiers that do not start with an underscore are reserved

[[nodiscard]] consteval size_t operator""uz(unsigned long long const Value) noexcept { return Value; }
[[nodiscard]] consteval size_t operator""Uz(unsigned long long const Value) noexcept { return Value; }
[[nodiscard]] consteval size_t operator""uZ(unsigned long long const Value) noexcept { return Value; }
[[nodiscard]] consteval size_t operator""UZ(unsigned long long const Value) noexcept { return Value; }
[[nodiscard]] consteval size_t operator""zu(unsigned long long const Value) noexcept { return Value; }
[[nodiscard]] consteval size_t operator""Zu(unsigned long long const Value) noexcept { return Value; }
[[nodiscard]] consteval size_t operator""zU(unsigned long long const Value) noexcept { return Value; }
[[nodiscard]] consteval size_t operator""ZU(unsigned long long const Value) noexcept { return Value; }

WARNING_POP()

#endif

//----------------------------------------------------------------------------

#endif // POLYFILLS_HPP_95E41B70_5DB2_4E5B_A468_95343C6438AD
