#if defined(_MSC_VER) && _MSC_VER < 1900
#define __midl  // do not include inline implementation
#endif

#include "crt.hpp"
#if defined(_MSC_VER) && !defined(UNICODE)
#pragma function(memset)
#endif

#if !(defined(_MSC_VER) && _MSC_VER >= 1900 && defined(UNICODE))

#if defined(UNICODE) && !defined(__BORLANDC__)
typedef wchar_t PTRTYP;
typedef wchar_t FILLTYP;
#else
typedef void  PTRTYP;
typedef int   FILLTYP;
#endif

PTRTYP * __cdecl
#ifndef UNICODE
               memset
#else
#ifdef __BORLANDC__
               _wmemset
#else
               wmemset
#endif
#endif
                        (PTRTYP *dst, FILLTYP val, size_t count)
{
  PTRTYP *start = dst;

  while (count--)
  {
    *(TCHAR *)dst = (TCHAR)val;
    dst = (TCHAR *)dst + 1;
  }
  return(start);
}

#endif
