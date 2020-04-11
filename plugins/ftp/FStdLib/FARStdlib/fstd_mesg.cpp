#include <all_far.h>
#pragma hdrstop

#include "fstdlib.h"

#define MSG_BUFFSIZE 10000

static char *messbuff = NULL;
static AbortProc ExitProc;

static void __cdecl MSG_DelBuff(void)
{
	if(messbuff)
	{
		delete[] messbuff;
		messbuff = NULL;
	}

	if(ExitProc)
		ExitProc();
}

LPCSTR __cdecl Message(LPCSTR patt,...)
{
	va_list  a;
	LPCSTR m;
	va_start(a, patt);
	m = MessageV(patt,a);
	va_end(a);
	return m;
}

LPCSTR WINAPI MessageV(LPCSTR patt,va_list a)
{
	if(!messbuff)
	{
		ExitProc = AtExit(MSG_DelBuff);
		messbuff = new char[MSG_BUFFSIZE];
	}

	vsnprintf(messbuff,MSG_BUFFSIZE,patt,a);
	return messbuff;
}
