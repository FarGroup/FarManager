#include "crt.hpp"
#ifdef _MSC_VER
#ifndef UNICODE
#pragma function(strcpy)
#else
#pragma function(wcscpy)
#endif
#endif

TCHAR * __cdecl
#ifndef UNICODE
               strcpy
#else
               wcscpy
#endif
                        (TCHAR *dst, const TCHAR *src)
{
  TCHAR *cp = dst;

  while((*cp = *src) != 0) { ++cp; ++src; } /* Copy src over dst */

  return(dst);
}
