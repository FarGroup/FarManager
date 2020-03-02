#include <all_far.h>
#pragma hdrstop

#include "Int.h"

/*
   Calculate percentage of `N1` in `N2`
   Returns a number in range 0..100

   Input values must be equal signed or result will be undefined
*/
double ToPercent(__int64 N1,__int64 N2)
{
	while(N1 > 10000 || N2 > 10000)
	{
		N1/=100;
		N2/=100;
	}

	if(N2==0) return 0;

	if(N2<N1) return 100;

	return ((double)N1) * 100.0 / ((double)N2);
}

/*
   Create CPS value string with standart(?) round rulles.

   The digit returns string with variable sizes from 1 to 5
   symbols in length.
*/
LPCSTR FCps4(char *buff,double val)
{
	static char Names[] = { 'M', 'K', '\x0' };
	int Letter;

//1M
	if(val >= 10000000.)
	{
		Letter = 0;
		val   /= 1000000;
	}
	else

//1K
		if(val >= 10000.)
		{
			Letter = 1;
			val   /= 1000;
		}
		else
		{
//<1K
			Letter = 2;
		}

	sprintf(buff,"%d%c",(int)val,Names[Letter]);
	return buff;
}

/*
    Fill part of string buffer width percent-character and non-percent-character

    `str` points to start of whole string, `x` and `x1` determine portion of
    string to fill
    Do not check `x` and `x1` fit at string; do not chage any part of
    string outside of x..x1 bounds
*/
void PPercent(char *str,int x,int x1,int percent)
{
	if(x1-x < 1)
		return;

	if(percent == 0)
	{
		memset(str+x,FAR_SHADOW_CHAR,x1-x+1);
		return;
	}

	if(percent == 100)
	{
		memset(str+x,FAR_FULL_CHAR,x1-x+1);
		return;
	}

	percent = Min(100,Max(0,percent));
	percent = x + (x1-x)*percent/99;

	for(; x < percent; x++) str[x] = FAR_FULL_CHAR;

	for(; x <= x1; x++)     str[x] = FAR_SHADOW_CHAR;
}

void StrYTime(char *str,struct tm *tm)
{
	Assert(tm != NULL);
	char tmp[10],*m;
	StrCpy(tmp, (char*)FTP_Info->GetOpt()->Months[tm->tm_mon], ARRAYSIZE(tmp));

	if((m=strchr(tmp,';')) != NULL) *m = 0;

	sprintf(str,"%02d-%s", tm->tm_mday, tmp);
}

void StrTTime(char *str,struct tm *tm)
{
	Assert(tm != NULL);
	sprintf(str,"%02d:%02d:%02d",
	        tm->tm_hour,tm->tm_min,tm->tm_sec);
}
