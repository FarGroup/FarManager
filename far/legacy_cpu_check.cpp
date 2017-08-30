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
