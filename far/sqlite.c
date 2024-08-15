/*
sqlite.c

sqlite wrapper

*/
/*
Copyright © 2011 Far Group
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

// Self

// Internal:

// Platform:

// Common:
#include "common/preprocessor.hpp"

// External:

#include "sqlite.config.h"

//----------------------------------------------------------------------------

#define SQLITE_API __declspec(dllexport)

WARNING_PUSH(3)

WARNING_DISABLE_MSC(4018) // '>': signed / unsigned mismatch
WARNING_DISABLE_MSC(4391) // 'signature' : incorrect return type for intrinsic function, expected 'type'
WARNING_DISABLE_MSC(4668) // 'symbol' is not defined as a preprocessor macro, replacing with '0' for 'directives'
WARNING_DISABLE_MSC(5105) // macro expansion producing 'defined' has undefined behavior
WARNING_DISABLE_MSC(4191) // 'operator/operation' : unsafe conversion from 'type of expression' to 'type required'

WARNING_DISABLE_GCC("-Wpedantic")
WARNING_DISABLE_GCC("-Wcast-align")
WARNING_DISABLE_GCC("-Wcast-function-type")
WARNING_DISABLE_GCC("-Wcast-qual")
WARNING_DISABLE_GCC("-Wdouble-promotion")
WARNING_DISABLE_GCC("-Wduplicated-branches")
WARNING_DISABLE_GCC("-Wimplicit-fallthrough")
WARNING_DISABLE_GCC("-Wmisleading-indentation")
WARNING_DISABLE_GCC("-Wmissing-declarations")
WARNING_DISABLE_GCC("-Wredundant-decls")
WARNING_DISABLE_GCC("-Wstringop-overread")
WARNING_DISABLE_GCC("-Wundef")
WARNING_DISABLE_GCC("-Wunused-but-set-variable")
#ifdef _DEBUG
WARNING_DISABLE_GCC("-Wformat=")
WARNING_DISABLE_GCC("-Wformat-extra-args")
WARNING_DISABLE_GCC("-Wformat-nonliteral")
WARNING_DISABLE_GCC("-Wsign-compare")
WARNING_DISABLE_GCC("-Wtype-limits")
WARNING_DISABLE_GCC("-Wunused-function")
#endif

WARNING_DISABLE_CLANG("-Weverything")

#include "thirdparty/sqlite/sqlite3.c"

WARNING_POP()
