/*
legacy_cpu_check.cpp

Проверка поддержки SSE2 для x86
*/
/*
Copyright © 2017 Far Group
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

#include "headers.hpp"
#pragma hdrstop

#include "legacy_cpu_check.hpp"

#if defined(_WIN32) && !defined(_WIN64)
# if defined(_MSC_VER)
#  include <intrin.h>
#  define cpuid(inf, n) __cpuidex(inf, n, 0)
# elif defined(__GNUC__)
#  include <cpuid.h>
#  define cpuid(inf, n) __cpuid_count(n, 0, inf[0], inf[1], inf[2], inf[3])
# endif
#endif

bool IsLegacyCPU()
{
#if defined(_WIN32) && !defined(_WIN64) && (defined(_MSC_VER) || defined(__GNUC__))
	bool have_sse2 = false;
	int info[4] = { 0, 0, 0, 0 };
	cpuid(info, 0);
	if (info[0] >= 1)
	{
		cpuid(info, 1);
		have_sse2 = (info[3] & ((int)1 << 26)) != 0;
	}
	return !have_sse2;
#else
	return false;
#endif
}
