#include <all_far.h>
#pragma hdrstop

#include "fstdlib.h"

int WINAPI StrCmp(const char *str,const char *str1,int maxlen, BOOL isCaseSens)
{
	int  n,diff;

	if(!str) return (str1 == NULL)?0:(-1);

	if(!str1) return (str == NULL)?0:1;

	if(!isCaseSens)
	{
		for(n = 0; str[n] && str1[n] && (maxlen == -1 || n < maxlen); n++)
		{
			diff = ToLower(str[n]) - ToLower(str1[n]);

			if(diff) return diff;
		}

		diff = ToLower(str[n]) - ToLower(str1[n]);
	}
	else
	{
		for(n = 0; str[n] && str1[n] && (maxlen == -1 || n < maxlen); n++)
			if((diff=(str[n]-str1[n])) != 0) return diff;

		diff = str[n] - str1[n];
	}

	return (n == maxlen) ? 0 : diff;
}
