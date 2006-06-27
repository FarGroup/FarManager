#include "strdup.hpp"
#include "malloc.hpp"
#include <windows.h>

char *strdup(const char *block)
{
  char *result = (char *)malloc(lstrlen(block)+1);
  if (!result)
    return NULL;
  lstrcpy(result,block);
  return result;
}
