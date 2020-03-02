#include "lng.common.h"

void TrimEnd (char *lpStr)
{
	char *Ptr = lpStr+strlen(lpStr)-1;

	while ( Ptr>=lpStr && (*Ptr==' ' || *Ptr=='\t') )
		*Ptr = 0;
}

void TrimStart (char *lpStr)
{
	char *Ptr = lpStr;

	while ( *Ptr && (*Ptr==' ' || *Ptr=='\t') )
		Ptr++;

	memmove (lpStr, Ptr, strlen(Ptr)+1);
}

void Trim (char *lpStr)
{
	TrimEnd (lpStr);
	TrimStart (lpStr);
}

extern "C" int __cdecl sprintf (
		char *pBuffer,
		const char * format,
		...
		)
{
	int retValue;
	va_list argptr;

	va_start (argptr, format);
	retValue = wvsprintfA (pBuffer, format, argptr);
	va_end (argptr);

	return retValue;
}


extern "C" int __cdecl printf (
		const char * format,
		...
		)
{
	char szBuff[1024];

	int retValue;

	DWORD cbWritten;
	va_list argptr;

	va_start (argptr, format);
	retValue = wvsprintfA (szBuff, format, argptr);
	va_end (argptr);

	WriteFile (
			GetStdHandle(STD_OUTPUT_HANDLE),
			szBuff,
			retValue,
			&cbWritten,
			0
			);

	return retValue;
}

#define _tchartodigit(c)    ((c) >= '0' && (c) <= '9' ? (c) - '0' : -1)

long __cdecl atol (
        const char *nptr
        )
{
	char c;
	long total;
	char sign;

	while ( *nptr && (*nptr == ' ') )
		++nptr;

	c = *nptr++;
	sign = c;
	if (c == '-' || c == '+')
		c = *nptr++;

	total = 0;

	while ( (c = _tchartodigit(c)) != -1 ) {
		total = 10 * total + c;
		c = *nptr++;
	}

	if (sign == '-')
		return -total;
	else
		return total;
}

unsigned long CRC32(
		unsigned long crc,
		const char *buf,
		unsigned int len
		)
{
	static unsigned long crc_table[256];

	if (!crc_table[1])
	{
		unsigned long c;
		int n, k;

		for (n = 0; n < 256; n++)
		{
			c = (unsigned long)n;
			for (k = 0; k < 8; k++) c = (c >> 1) ^ (c & 1 ? 0xedb88320L : 0);
				crc_table[n] = c;
		}
	}

	crc = crc ^ 0xffffffffL;
	while (len-- > 0) {
		crc = crc_table[(crc ^ (*buf++)) & 0xff] ^ (crc >> 8);
	}

	return crc ^ 0xffffffffL;
}
