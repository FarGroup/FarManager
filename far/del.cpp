/*
del.cpp

������ RTL-�����

*/

/* Revision: 1.01 25.02.2003 $ */

/*
Modify:
  25.02.2003 SVS
    ! �ਬ���� ���稪 CallNewDelete/CallMallocFree ��� �⫠���
  24.02.2003 SVS
    ! �뤥����� � ����⢥ ᠬ����⥫쭮�� �����
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
