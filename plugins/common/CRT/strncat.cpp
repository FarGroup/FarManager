#include "crt.hpp"

TCHAR * __cdecl
#ifndef UNICODE
               strncat
#else
               wcsncat
#endif
                      (TCHAR *first, const TCHAR *last, size_t count)
{
  TCHAR *start = first;

  while (*first++)
    ;
  first--;

  while (count--)
    if (!(*first++ = *last++))
      return(start);

  *first = '\0';
  return(start);
}
