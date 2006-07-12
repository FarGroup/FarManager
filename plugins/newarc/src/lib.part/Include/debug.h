#pragma once
#include <Rtl.Base.h>
#include <windows.h>

#define TIME_THAT(proc) \
	LARGE_INTEGER __counter1; \
	LARGE_INTEGER __counter2; \
	char *__s = StrCreate (260); \
	QueryPerformanceCounter (&__counter1); \
	proc; \
	QueryPerformanceCounter (&__counter2); \
	_i64toa (__counter2.QuadPart-__counter1.QuadPart, __s, 10); \
	__debug (__s); \
	StrFree (__s);


extern void __debug (const char *format, ...);
extern int dprintf (const char * format, ...);
extern void __cdecl __fdebug (const char *format, ...);
