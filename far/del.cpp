/*
del.cpp

Замена RTL-модуля

*/

/* Revision: 1.01 25.02.2003 $ */

/*
Modify:
  25.02.2003 SVS
    ! применим счетчик CallNewDelete/CallMallocFree для отладки
  24.02.2003 SVS
    ! Выделение в качестве самостоятельного модуля
*/

#include "headers.hpp"
#pragma hdrstop

extern "C" {
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
