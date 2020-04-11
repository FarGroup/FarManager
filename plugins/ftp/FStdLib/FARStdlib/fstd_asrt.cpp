#include <all_far.h>
#pragma hdrstop

#include "fstdlib.h"

void __cdecl __WinAbort(LPCSTR msg,...)
{
	va_list a;
	char    pnm[MAX_PATH],
	   str[ 1000 ];
	int     l;

	if(!msg) exit(1);

//Message
	va_start(a,msg);
	vsnprintf(str,sizeof(str),msg,a);
	va_end(a);
//Plugin name
	strcpy(pnm,"Assertion in \"");
	l = (int)strlen(pnm);
	pnm[ l + GetModuleFileName(FP_HModule,pnm+l,sizeof(pnm)-l)] = 0;
	strcat(pnm,"\" !");
	MessageBox(NULL,str,pnm,MB_OK|MB_ICONHAND);
	TerminateProcess(GetCurrentProcess(),0);
}
