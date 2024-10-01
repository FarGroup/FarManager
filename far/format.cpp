/*
format.cpp

Форматирование строк
*/
/*
Copyright © 2009 Far Group
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
#include "format.hpp"

// Internal:
#include "encoding.hpp"
#include "components.hpp"
#include "locale.hpp"

// Platform:

// Common:

// External:

//----------------------------------------------------------------------------

WARNING_PUSH(3)

WARNING_DISABLE_GCC("-Wdangling-reference")
WARNING_DISABLE_GCC("-Wmissing-declarations")

WARNING_DISABLE_CLANG("-Weverything")

struct thousands_separator
{
	explicit(false) operator wchar_t() const
	{
		return ::locale.thousand_separator();
	}

	explicit(false) operator char() const
	{
		return static_cast<char>(operator wchar_t());
	}
};

#define FMT_STATIC_THOUSANDS_SEPARATOR (thousands_separator{})

#include "thirdparty/fmt/format.cc"

WARNING_POP()

namespace
{
	SCOPED_ACTION(components::component)([]
	{
		return components::info{ L"fmt"sv, far::format(L"{}.{}.{}"sv, FMT_VERSION / 10000, FMT_VERSION % 10000 / 100, FMT_VERSION % 100) };
	});
}
