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
#if defined(_MSC_VER)
// implemented in separate .asm file
#elif defined(__GNUC__)
int __cdecl memicmp(const void *first, const void *last, size_t count)
    __attribute__((alias("_memicmp")));
#elif defined(__BORLANDC__)
#pragma alias "_memicmp" = "__memicmp"
#endif
