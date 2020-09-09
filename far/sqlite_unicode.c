/*
sqlite_unicode.c

sqlite_unicode wrapper

*/
/*
Copyright © 2015 Far Group
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

// Common:
#include "common/compiler.hpp"
#include "common/preprocessor.hpp"

// External:

//----------------------------------------------------------------------------

#define SQLITE_CORE
#define SQLITE_ENABLE_UNICODE

WARNING_PUSH(3)
WARNING_DISABLE_GCC("-Wcast-qual")
WARNING_DISABLE_GCC("-Wsequence-point")
WARNING_DISABLE_GCC("-Wsign-compare")

WARNING_DISABLE_CLANG("-Weverything")

#define sqlite3_value_text16 sqlite3_value_text16_hook
#define sqlite3_value_bytes16 sqlite3_value_bytes16_hook
#define sqlite3_result_text16 sqlite3_result_text16_hook

#include "thirdparty/sqlite/sqlite3_unicode.c"


const void* far_value_text16(void*);

const void* sqlite3_value_text16_hook(sqlite3_value* Val)
{
	return far_value_text16(Val);
}

int far_value_bytes16(void*);

int sqlite3_value_bytes16_hook(sqlite3_value* Val)
{
	return far_value_bytes16(Val);
}

void far_result_text16(void*, const void*, int);

void sqlite3_result_text16_hook(sqlite3_context* Ctx, const void* Val, int Length, void (*Del)(void*))
{
	far_result_text16(Ctx, Val, Length);
}


const wchar_t SQLite_Unicode_Version[] = WIDE(SQLITE3_UNICODE_VERSION_STRING);

WARNING_POP()
