#include <all_far.h>
#pragma hdrstop

#include "fstdlib.h"

int WINAPI strLen(LPCSTR str)
{
	return str ? (int)strlen(str) : 0;
}

char *WINAPI StrDup(LPCSTR m)
{
	char *rc;

	if(!m) m = "";

	rc = (char*)_Alloc(strLen(m)+1);

	if(rc)
		StrCpy(rc,m);

	return rc;
}
