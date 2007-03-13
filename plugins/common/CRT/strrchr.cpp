#include "crt.hpp"

#ifndef UNICODE
_CONST_RETURN char * __cdecl strrchr(const char *string, int ch)
#else
_CONST_RETURN_W wchar_t * __cdecl wcsrchr(const wchar_t *string, wchar_t ch)
#endif
{
  const TCHAR *start = string;

  while (*string++)
    ;

  while (--string != start && *string != (TCHAR)ch)
    ;

  if (*string == (TCHAR)ch)
    return (TCHAR *)string;

  return NULL;
}
