#include "crt.hpp"
#pragma warning (disable : 4005)
#include <windows.h>

TCHAR * __cdecl
#ifndef UNICODE
               strdup
#else
               wcsdup
#endif
                     (const TCHAR *block)
{
  TCHAR *result = (TCHAR *)malloc((lstrlen(block)+1)*sizeof(TCHAR));
  if (!result)
    return NULL;
  lstrcpy(result,block);
  return result;
}
