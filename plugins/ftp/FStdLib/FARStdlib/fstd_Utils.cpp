#include <all_far.h>
#pragma hdrstop

#include "fstdlib.h"

LPSTR WINAPI AddLastSlash(char *path, char Slash)
{
	size_t len;

	if((len=strlen(path)) != 0 && path[len-1] != Slash)
	{
		path[len]   = Slash;
		path[len+1] = 0;
	}

	return path;
}

LPSTR WINAPI DelLastSlash(char *path, char Slash)
{
	size_t len;

	if(path && path[0] && path[len=(strlen(path)-1)] == Slash)
		path[len] = 0;

	return path;
}

LPCSTR WINAPI FPath(LPCSTR nm, char Slash)
{
	static char str[MAX_PATH];
	char     *m;
	StrCpy(str,nm,sizeof(str));
	m = strrchr(str,Slash);
	if(m) *m = 0; else str[0] = 0;

	return AddLastSlash(str);
}

LPCSTR WINAPI FName(LPCSTR nm, char Slash)
{
	static char str[MAX_PATH];
	LPCSTR m = strrchr(nm,Slash);
	if(!m) m = nm; else m++;

	StrCpy(str,m,sizeof(str));
	return str;
}

LPCSTR WINAPI FExtOnly(LPCSTR nm, char Slash)
{
	LPCSTR m = strrchr(nm,Slash),
	       ext;

	if(!m) m = nm;

	ext = strrchr(m,'.');

	if(ext)
		return ext+1;
	else
		return "";
}

/*
   Create CPS value string

   The digit allways 3+1+3+1 characters length (8)
   Digit right alignmented, filled with ' ' at left
*/
LPCSTR WINAPI FCps(char *buff,double val)
{
	char     Letter;
	char     str[50];
	LPCSTR _buff = buff;

//1M
	if(val >= 1000000.)
	{
		Letter = 'M';
		val   /= 1000000;
	}
	else

//1K
		if(val >= 1000.)
		{
			Letter = 'K';
			val   /= 1000;
		}
		else
		{
//<1K
			Letter = 'b';
		}

	if(Letter == 'b')
		sprintf(str,"%db",(int)val);
	else
		sprintf(str,"%3.3lf%c",val,Letter);

	int sz;

	for(sz = 8 - (int)strlen(str); sz > 0; buff++,sz--) *buff = ' ';

	for(sz = 0; str[sz]; buff++,sz++) *buff = str[sz];

	*buff = 0;
	return _buff;
}
