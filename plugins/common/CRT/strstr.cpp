#include "crt.hpp"

#ifndef UNICODE
_CONST_RETURN char * __cdecl strstr
#else
_CONST_RETURN_W wchar_t * __cdecl wcsstr
#endif
                                    (const TCHAR * str1, const TCHAR * str2)
{
  TCHAR *cp = (TCHAR *) str1;
  TCHAR *s1, *s2;

  if ( !*str2 )
    return((TCHAR *)str1);

  while (*cp)
  {
    s1 = cp;
    s2 = (TCHAR *) str2;

    while ( *s1 && *s2 && !(*s1-*s2) )
      s1++, s2++;

    if (!*s2)
      return(cp);

    cp++;
  }

  return(NULL);
}
