#include <all_far.h>
#pragma hdrstop

#include "fstdlib.h"

BOOL WINAPI FP_InPattern(LPCSTR patt,LPCSTR nm)
{
	char c;
	size_t n;
	char str[MAX_PATH];

	if(!patt || !patt[0] || !nm) return FALSE;

	do
	{
		for(n = 0; (c=*patt) != 0 && n < sizeof(str)-1 && *patt != ',' && *patt != ';'; patt++)
			str[n++] = *patt;

		str[n] = 0;

		if(*patt) patt++;

		if(strchr(str,'.') == NULL)
			StrCat(str,".*",sizeof(str));

		if(FP_Info->CmpName(str,nm,TRUE))
			return TRUE;
	}
	while(c);

	return FALSE;
}
