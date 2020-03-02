#include <all_far.h>
#pragma hdrstop

#include "fstdlib.h"

int WINAPI StrColCount(LPCSTR str,LPCSTR seps)
{
	int res = 1;

	if(!str || !seps || !str[0] || !seps[0]) return 0;

	for(int n = 0; str[n]; n++)
		if(strchr((char*)seps,str[n]) != NULL)
			res++;

	return res;
}

LPCSTR WINAPI StrGetCol(LPCSTR str,int number,LPCSTR seps)
{
	static char resStr[MAX_PATH];
	int res;
	int num;

	for(res = 1; *str && res < number; str++)
		if(strchr((char*)seps,*str) != NULL) res++;

	resStr[num=0] = 0;

	if(res == number)
		for(; *str && strchr((char*)seps,*str) == NULL; str++)
			resStr[num++] = *str;

	resStr[num] = 0;
	return resStr;
}
