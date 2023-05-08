/*
farversion.cpp

Версия Far Manager
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

// BUGBUG
#include "platform.headers.hpp"

// Self:
#include "farversion.hpp"

// Internal:
#include "plugin.hpp"

// Platform:

// Common:
#include "common/compiler.hpp"
#include "common/library.hpp"

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

namespace build
{
#include "bootstrap/copyright.inc"

	VersionInfo version()
	{
		return
		{
#include "bootstrap/farversion.inc"
		};
	}

	string compiler()
	{
		const auto CompilerInfo =
#ifdef _MSC_BUILD
			L"." EXPAND_TO_WIDE_LITERAL(_MSC_BUILD)
#endif
			L""sv;

		return far::format(L"{}, version {}.{}.{}{}"sv,
			COMPILER_NAME,
			COMPILER_VERSION_MAJOR,
			COMPILER_VERSION_MINOR,
			COMPILER_VERSION_PATCH,
			CompilerInfo
		);
	}

	string library()
	{
		return far::format(L"{}, version {}.{}.{}"sv,
			STANDARD_LIBRARY_NAME,
			STANDARD_LIBRARY_VERSION_MAJOR,
			STANDARD_LIBRARY_VERSION_MINOR,
			STANDARD_LIBRARY_VERSION_PATCH
		);
	}

}
