#include <all_far.h>
#pragma hdrstop

#include "fstdlib.h"

/*******************************************************************
   Crc32

   Generate 32bit CRC for buffer.

   Algoriphm getted from sources of ZLib.
 *******************************************************************/

static int   crc_table_empty = 1;
static DWORD crc_table[256];

static void make_crc_table(void)
{
	DWORD c, n;
	int k;
	DWORD poly;            /* polynomial exclusive-or pattern */
	/* terms of polynomial defining this crc (except x^32): */
	static const BYTE p[] = {0,1,2,4,5,7,8,10,11,12,16,22,23,26};
	/* make exclusive-or pattern from polynomial (0xedb88320L) */
	poly = 0L;

	for(n = 0; n < sizeof(p)/sizeof(BYTE); n++)
		poly |= 1L << (31 - p[n]);

	for(n = 0; n < 256; n++)
	{
		c = n;

		for(k = 0; k < 8; k++)
			c = c & 1 ? poly ^(c >> 1) : c >> 1;

		crc_table[n] = c;
	}

	crc_table_empty = 0;
}

/* ========================================================================= */
#define DO1(buf) crc = crc_table[((int)crc ^ (*buf++)) & 0xff] ^ (crc >> 8);
#define DO2(buf)  DO1(buf); DO1(buf);
#define DO4(buf)  DO2(buf); DO2(buf);
#define DO8(buf)  DO4(buf); DO4(buf);

DWORD WINAPI Crc32(DWORD crc, const BYTE *buf,DWORD len)
{
	if(buf == NULL)
		return 0L;

	if(crc_table_empty)
		make_crc_table();

	crc = crc ^ 0xffffffffL;

	while(len >= 8)
	{
		DO8(buf);
		len -= 8;
	}

	if(len)
		do
		{
			DO1(buf);
		}
		while(--len);

	return crc ^ 0xffffffffL;
}
