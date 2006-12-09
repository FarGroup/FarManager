#include "crt.hpp"
#include <windows.h>

char * __cdecl strdup(const char *block)
{
  char *result = (char *)malloc(lstrlen(block)+1);
  if (!result)
    return NULL;
  lstrcpy(result,block);
  return result;
}
