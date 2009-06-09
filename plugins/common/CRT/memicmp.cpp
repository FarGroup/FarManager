#include "crt.hpp"

#define TOLOWER(c) ( ((c) >= 'A') && ((c) <= 'Z') ? ((c) - 'A' + 'a') : (c) )

int __cdecl _memicmp(const void *first, const void *last, size_t count)
{
  int f = 0;
  int l = 0;

  while ( count-- )
  {
    if ( (*(unsigned char *)first == *(unsigned char *)last) || ((f = TOLOWER( *(unsigned char *)first )) == (l = TOLOWER( *(unsigned char *)last ))) )
    {
      first = (char *)first + 1;
      last = (char *)last + 1;
    }
    else
        break;
  }

  return (f - l);
}
