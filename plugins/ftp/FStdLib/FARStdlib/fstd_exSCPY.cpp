#include <all_far.h>
#pragma hdrstop

#include "fstdlib.h"

char *WINAPI StrCpy(char *dest,LPCSTR src,int dest_sz)
{
	if(dest == NULL)    return NULL;

	if(dest_sz == 0) return dest;

	if(!src)         { *dest = 0; return dest; }

	if(dest_sz != -1)
	{
		strncpy(dest,src,dest_sz-1);
		dest[dest_sz-1] = 0;
	}
	else
		strcpy(dest,src);

	return dest;
}
