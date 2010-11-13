#include <all_far.h>
#pragma hdrstop

#include "fstdlib.h"

char FHotKey::Next(void)
{
	char res;
	if(Key >= FHK_NUMHOTKEYS) res = ' '; else

	{
		if(Alpha)
			res = (Key >= FHK_NUMALPHAS)?('0'+Key-FHK_NUMALPHAS):('A'+Key);
		else
			res = (Key >= FHK_NUMDIGITS)?('A'+Key-FHK_NUMDIGITS):('0'+Key);
	}

	Key++;
	return res;
}

char FHotKey::MkHotKey(int num,bool digOnly)
{
	if(num < 10)
		return (char)('0' + num);

	num -= 10;

	if(!digOnly && num < 'Z'-'A')
		return (char)('A'+num);

	return ' ';
}
