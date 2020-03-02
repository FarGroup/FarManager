#include <all_far.h>
#pragma hdrstop

#include "fstdlib.h"

LPSTR WINAPI StrFromOEMDup(LPCSTR str,int num /*0|1*/)
{
	static char nm[2][MAX_PATH];

	if(num < 0 || num >= (int)ARRAYSIZE(nm)) return (LPSTR)str;

	OemToCharBuff(str, nm[num], sizeof(nm[0])-1);
	return nm[num];
}

LPSTR WINAPI StrToOEMDup(LPCSTR str,int num /*0|1*/)
{
	static char nm[2][MAX_PATH];

	if(num < 0 || num >= (int)ARRAYSIZE(nm)) return (LPSTR)str;

	CharToOemBuff(str, nm[num], sizeof(nm[0])-1);
	return nm[num];
}

LPSTR WINAPI StrFromOEM(LPSTR str,int sz /*=-1*/)
{
	if(sz == -1)
		OemToChar(str, str);
	else
		OemToCharBuff(str, str, sz-1);

	return str;
}

LPSTR WINAPI StrToOEM(LPSTR str,int sz /*=-1*/)
{
	if(sz == -1)
		CharToOem(str, str);
	else
		CharToOemBuff(str, str, sz-1);

	return str;
}
