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

#include "common/compiler.hpp"

#define SQLITE_CONFIG_ONLY
#include "sqlite.hpp"
#undef SQLITE_CONFIG_ONLY

WARNING_PUSH(3)

WARNING_DISABLE_MSC(4555) // https://msdn.microsoft.com/en-us/library/k64a6he5.aspx result of expression not used
WARNING_DISABLE_MSC(4701) // https://msdn.microsoft.com/en-us/library/1wea5zwe.aspx Potentially uninitialized local variable 'name' used
WARNING_DISABLE_MSC(4703) // https://msdn.microsoft.com/en-us/library/jj851030.aspx Potentially uninitialized local pointer variable 'name' used

WARNING_DISABLE_GCC("-Wpragmas")
WARNING_DISABLE_GCC("-Warray-bounds")
WARNING_DISABLE_GCC("-Wcast-qual")
WARNING_DISABLE_GCC("-Wimplicit-fallthrough")
WARNING_DISABLE_GCC("-Wunused-but-set-variable")
WARNING_DISABLE_GCC("-Wunused-value")
WARNING_DISABLE_GCC("-Wcast-function-type")
WARNING_DISABLE_GCC("-Wunused-function")
WARNING_DISABLE_GCC("-Wempty-body")
#ifdef _DEBUG
WARNING_DISABLE_GCC("-Wformat=")
WARNING_DISABLE_GCC("-Wformat-extra-args")
WARNING_DISABLE_GCC("-Wformat-nonliteral")
WARNING_DISABLE_GCC("-Wsign-compare")
#endif

WARNING_DISABLE_CLANG("-Weverything")

#include "thirdparty/sqlite/sqlite3.c"

WARNING_POP()
