/*
new.cpp

������ RTL-�����

*/

/* Revision: 1.02 06.08.2004 $ */

/*
Modify:
  06.08.2004 SKV
    ! see 01825.MSVCRT.txt
  25.02.2003 SVS
    ! �ਬ���� ���稪 CallNewDelete/CallMallocFree ��� �⫠���
  24.02.2003 SVS
    ! �뤥����� � ����⢥ ᠬ����⥫쭮�� �����
*/

#include "headers.hpp"
#pragma hdrstop

extern "C" {
void *__cdecl xf_malloc(size_t __size);
};

#if defined(SYSLOG)
extern long CallNewDelete;
#endif


#if defined(_MSC_VER)
extern _PNH _pnhHeap;

extern "C" {
void * __cdecl  _nh_malloc(size_t, int);
};

void * operator new( size_t cb )
{
  // ����� ��� - �� ������ - �� (�᫨ �� ����� - ᤥ����!!!)
  void *res = xf_malloc(cb);//_nh_malloc( cb, 1 );
  return res;
}

#elif defined(__BORLANDC__)
extern new_handler _new_handler;

//namespace std {

new_handler set_new_handler(new_handler p)
{
  new_handler t = _new_handler;
  _new_handler = p;
  return t;
}
//}

void *operator new(size_t size)
{
  void * p;

  size = size ? size : 1;

  /* FDIS 18.4.1.1 (3,4) now require new to throw bad_alloc if the
     most recent call to set_new_handler was passed NULL.
     To ensure no exception throwing, use the forms of new that take a
     nothrow_t, as they will call straight to malloc().
  */
  while ( (p = xf_malloc(size)) == NULL)
      if (_new_handler)
          _new_handler();
      else
         /* This is illegal according to ANSI, but if we've compiled the
            RTL without exception support we had better just return NULL.
         */
         break;
#if defined(SYSLOG)
  CallNewDelete++;
#endif
  return p;
}

#endif
