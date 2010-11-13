#include <all_far.h>
#pragma hdrstop

#include "fstdlib.h"

int WINAPI strchrCount(const char *str,char ch,int maxlen)
{
	int cn = 0;

	for(int n = 0; str[n] && (maxlen == -1 || n < maxlen); n++)
		if(str[n] == ch) cn++;

	return cn;
}
