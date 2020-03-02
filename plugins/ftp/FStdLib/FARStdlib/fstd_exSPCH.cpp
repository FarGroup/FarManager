#include <all_far.h>
#pragma hdrstop

#include "fstdlib.h"

int WINAPI StrPosChr(const char *str,char ch,int pos)
{
	if(!str || pos < 0 || pos >= (int)strlen(str)) return -1;

	for(int n = pos; str[n]; n++)
		if(str[n] == ch) return n;

	return -1;
}
