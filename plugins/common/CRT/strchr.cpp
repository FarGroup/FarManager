#include "crt.hpp"

char * __cdecl strchr(register const char *s,int c)
{
  do
  {
    if(*s==c)
      return (char*)s;
  } while (*s++);
  return 0;
}
