/*
 * This file has no copyright assigned and is placed in the Public Domain.
 * This file is a part of the mingw-runtime package.
 * No warranty is given; refer to the file DISCLAIMER within the package.
 *
 * Some changes by FarTeam
 *
 */

#include <Rtl.Base.h>

typedef void (*func_ptr) (void);
extern func_ptr __CTOR_LIST__[];
extern func_ptr __DTOR_LIST__[];

void
__do_global_dtors (void)
{
  static func_ptr *p = __DTOR_LIST__ + 1;

  /*
   * Call each destructor in the destructor list until a null pointer
   * is encountered.
   */
  while (*p)
    {
      (*(p)) ();
      p++;
    }
}

void
__do_global_ctors (void)
{
  unsigned long nptrs = (unsigned long) __CTOR_LIST__[0];
  unsigned i;

  /*
   * If the first entry in the constructor list is -1 then the list
   * is terminated with a null entry. Otherwise the first entry was
   * the number of pointers in the list.
   */
  if (nptrs == (unsigned long)-1)
    {
      for (nptrs = 0; __CTOR_LIST__[nptrs + 1] != 0; nptrs++)
  ;
    }

  /*
   * Go through the list backwards calling constructors.
   */
  for (i = nptrs; i >= 1; i--)
    {
      __CTOR_LIST__[i] ();
    }
}

static int init = 0;

BOOL WINAPI
DllMainGCC (HANDLE hDll, DWORD dwReason, LPVOID lpReserved)
{
  if (dwReason == DLL_PROCESS_ATTACH && !init)
  {
  	init = 1;
    __do_global_ctors();
  }

  if (dwReason == DLL_PROCESS_DETACH && init)
  {
  	init = 0;
    __do_global_dtors();
  }

  return TRUE;
}
