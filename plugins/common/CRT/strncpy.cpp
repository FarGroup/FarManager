#include "crt.hpp"

TCHAR * __cdecl
#ifndef UNICODE
               strncpy
#else
               wcsncpy
#endif
                      (TCHAR *dest, const TCHAR *src, size_t count)
{
  TCHAR *start = dest;

  while (count && (*dest++ = *src++))
    count--;

  if (count)
    while (--count)
      *dest++ = _T('\0');

  return(start);
}
