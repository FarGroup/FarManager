#include <all_far.h>
#pragma hdrstop

#include "fstdlib.h"

struct PRPeriod
{
	DWORD b,e;
	DWORD     Period;
	DWORD     LastDiff;
};

/** @brief Create time period.

    @param ms Period length.

    @return handle of new period or NULL on error.
*/
HANDLE WINAPI FP_PeriodCreate(DWORD ms)
{
	PRPeriod* p = new PRPeriod;
	GET_TIME(p->b);
	p->Period = ms;
	return (HANDLE)p;
}

void WINAPI FP_PeriodDestroy(HANDLE& _p)
{
	delete((PRPeriod*)_p);
	_p = NULL;
}

BOOL WINAPI FP_PeriodEnd(HANDLE _p)
{
	PRPeriod* p = (PRPeriod*)_p;
	BOOL      res;

	if(!p) return FALSE;

	GET_TIME(p->e);
	p->LastDiff = (DWORD)(CMP_TIME(p->e,p->b)*1000.);
	res = p->LastDiff >= p->Period;

	if(res) p->b = p->e;

	return res;
}

DWORD WINAPI FP_PeriodTime(HANDLE _p)
{
	PRPeriod* p = (PRPeriod*)_p;

	if(!p) return 0;

	return p->LastDiff;
}

void WINAPI FP_PeriodReset(HANDLE _p)
{
	PRPeriod* p = (PRPeriod*)_p;

	if(p) GET_TIME(p->b);
}
