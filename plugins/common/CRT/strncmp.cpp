#include "crt.hpp"

int __cdecl strncmp(const char *first, const char *last, size_t count)
{
  if (!count)
    return(0);

  while (--count && *first && *first == *last)
  {
    first++;
    last++;
  }

  return (*(unsigned char *)first - *(unsigned char *)last);
}
