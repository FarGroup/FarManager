/*
del.cpp

Замена RTL-модуля

*/

#include "headers.hpp"
#pragma hdrstop

extern "C"
{
	void  __cdecl xf_free(void *__block);
};

#if defined(SYSLOG)
extern long CallNewDelete;
#endif

void operator delete(void *ptr)
{
#if defined(SYSLOG)
	CallNewDelete--;
#endif
	xf_free(ptr);
}
