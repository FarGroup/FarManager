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

#include "disable_warnings_in_std_begin.hpp"
#include <windows.h>
#include "disable_warnings_in_std_end.hpp"

#include "common/preprocessor.hpp"

WARNING_PUSH(3)

WARNING_DISABLE_MSC(4701) // https://msdn.microsoft.com/en-us/library/1wea5zwe.aspx Potentially uninitialized local variable 'name' used
WARNING_DISABLE_MSC(4703) // https://msdn.microsoft.com/en-us/library/jj851030.aspx Potentially uninitialized local pointer variable 'name' used

WARNING_DISABLE_GCC("-Warray-bounds")
WARNING_DISABLE_GCC("-Wunused-but-set-variable")
WARNING_DISABLE_GCC("-Wcast-qual")

// SQlite 3.12 suddenly started using rand_s function, which depends on RtlGenRandom (SystemFunction036), which is not available in Win2k.
// It would be better to hook only SystemFunction036 via our vc_crt_fix* facilities, but ucrt devs load it via GetProcAddress and call abort() if it's not available,
// which is, of course, a truly splendid design decision, my hat's off to them.
// So, no other choice but to craft the whole thing manually:
static int rand_s(unsigned int* randomValue)
{
	typedef BOOLEAN (WINAPI *SystemFunction036)(PVOID Buffer, ULONG Size);
	const SystemFunction036 RtlGetRandomPtr = (SystemFunction036)GetProcAddress(GetModuleHandle(L"advapi32"), "SystemFunction036");
	if (RtlGetRandomPtr)
		return RtlGetRandomPtr(randomValue, sizeof(*randomValue));
	
	*randomValue = rand();
	return 0;
}

#include "thirdparty/sqlite/sqlite3.c"

WARNING_POP()
