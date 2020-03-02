#include <all_far.h>
#pragma hdrstop

#include "fstdlib.h"

static AbortProc CTAbortProc = NULL;

AbortProc WINAPI AtExit(AbortProc p)
{
	AbortProc old = CTAbortProc;
	CTAbortProc = p;
	return old;
}

void WINAPI CallAtExit(void)
{
	if(CTAbortProc)
	{
		CTAbortProc();
		CTAbortProc = NULL;
	}
}
