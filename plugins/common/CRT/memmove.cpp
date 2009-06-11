#define __midl  // do not include inline implementation
#include "crt.hpp"

#if defined(UNICODE) && !defined(__BORLANDC__)
typedef wchar_t PTRTYP;
#else
typedef void  PTRTYP;
#endif

PTRTYP * __cdecl
#ifndef UNICODE
               memmove
#else
#ifdef __BORLANDC__
               _wmemmove
#else
               wmemmove
#endif
#endif
                        (PTRTYP *dst, const PTRTYP *src, size_t count)
{
  PTRTYP *ret = dst;
  if (dst <= src || (TCHAR *)dst >= ((TCHAR *)src + count))
  {
    while (count--)
    {
      *(TCHAR *)dst = *(TCHAR *)src;
      dst = (TCHAR *)dst + 1;
      src = (TCHAR *)src + 1;
    }
  }
  else
  {
    dst = (TCHAR *)dst + count - 1;
    src = (TCHAR *)src + count - 1;

    while (count--)
    {
      *(TCHAR *)dst = *(TCHAR *)src;
      dst = (TCHAR *)dst - 1;
      src = (TCHAR *)src - 1;
    }
  }
  return(ret);
}
