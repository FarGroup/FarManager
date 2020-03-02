#include <all_far.h>
#pragma hdrstop

#include "fstdlib.h"

/*
static void WINAPI _strncat( char *dest,const char *src,int dest_sz )
  {
    if ( !dest || !dest[0] || !src || !src[0] ) return;

    int len = (int)strlen(dest);
    if ( len >= dest_sz ) return;

    for( dest += len; *src && len < dest_sz; len++ )
       *dest++ = *src++;
    *dest = 0;
}
*/

char *WINAPI StrCat(char *dest,LPCSTR src,int dest_sz)
{
	if(!dest) return NULL;

	if(dest_sz == 0) return dest;

	if(!src)
	{
		*dest = 0;
		return dest;
	}

	if(dest_sz != -1)
	{
		dest_sz--;
		strncat(dest,src,dest_sz);
		dest[dest_sz] = 0;
	}
	else
		strcat(dest,src);

	return dest;
}
