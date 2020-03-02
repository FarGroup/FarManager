#pragma once
#include <windows.h>

extern unsigned long CRC32 (
		unsigned long crc,
		const char *buf,
		unsigned int len
		);

extern "C" int __cdecl sprintf (char *pBuffer, const char * format, ...);
extern "C" int __cdecl printf (const char * format, ...);

#define _tchartodigit(c)    ((c) >= '0' && (c) <= '9' ? (c) - '0' : -1)

long __cdecl atol (const char *nptr);

void TrimEnd (char *lpStr);
void TrimStart (char *lpStr);
void Trim (char *lpStr);
