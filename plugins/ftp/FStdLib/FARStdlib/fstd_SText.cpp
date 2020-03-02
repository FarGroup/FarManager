#include <all_far.h>
#pragma hdrstop

#include "fstdlib.h"

char *WINAPI Str2Text(LPCSTR name,char *Buff,DWORD BuffSz)
{
	char *buff = Buff;

	for(; BuffSz && *name; BuffSz--,name++,Buff++)
		switch(*name)
		{
			case '\\': *Buff++ = '\\'; *Buff = '\\'; break;
			case '\t': *Buff++ = '\\'; *Buff = 't';  break;
			case '\n': *Buff++ = '\\'; *Buff = 'n';  break;
			case '\r': *Buff++ = '\\'; *Buff = 'r';  break;
			case '\b': *Buff++ = '\\'; *Buff = 'b';  break;
			case '\'': *Buff++ = '\\'; *Buff = '\''; break;
			case '\"': *Buff++ = '\\'; *Buff = '\"'; break;
			default: *Buff = *name;
		}

	*Buff = 0;
	return buff;
}

char *WINAPI Text2Str(LPCSTR name,char *Buff,DWORD BuffSz)
{
	char *buff = Buff;

	for(; BuffSz && *name; BuffSz--,name++,Buff++)
		if(*name == '\\')
		{
			name++;

			switch(*name)
			{
				case '\\': *Buff = '\\'; break;
				case  't': *Buff = '\t'; break;
				case  'n': *Buff = '\n'; break;
				case  'r': *Buff = '\r'; break;
				case  'b': *Buff = '\b'; break;
				case '\'': *Buff = '\''; break;
				case '\"': *Buff = '\"'; break;
			}
		}
		else
			*Buff = *name;

	*Buff = 0;
	return buff;
}
