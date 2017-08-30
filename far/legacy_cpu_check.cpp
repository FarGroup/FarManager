#include "headers.hpp"
#pragma hdrstop

#include "legacy_cpu_check.hpp"

bool IsLegacyCPU()
{
#if defined(_WIN32) && !defined(_WIN64) && (defined(_MSC_VER) || defined(__GNUC__))
	bool have_sse2 = false;
	int info[4] = { 0, 0, 0, 0 };
#ifdef _MSC_VER
#include <intrin.h>
	__cpuidex(info, 0, 0);
	if (info[0] >= 1) {
		__cpuidex(info, 1, 0);
#else
#include <cpuid.h>
	__cpuid_count(0, 0, info[0], info[1], info[2], info[3]);
	if (info[0] >= 1) {
		__cpuid_count(1, 0, info[0], info[1], info[2], info[3]);
#endif
		have_sse2 = (info[3] & ((int)1 << 26)) != 0;
	}
	return !have_sse2;
#else
	return false;
#endif
}
