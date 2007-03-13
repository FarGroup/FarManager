#include "crt.hpp"

#ifndef UNICODE
_CONST_RETURN char * __cdecl strchr(register const char *s, int c)
#else
_CONST_RETURN_W wchar_t * __cdecl wcschr(register const wchar_t *s, wchar_t c)
#endif
{
  do
  {
    if(*s==(TCHAR)c)
      return (TCHAR*)s;
  } while (*s++);
  return 0;
}
