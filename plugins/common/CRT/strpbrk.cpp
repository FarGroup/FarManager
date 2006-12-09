#include "crt.hpp"

char * __cdecl strpbrk(const char *string, const char *control)
{
  const unsigned char *str = (const unsigned char *)string;
  const unsigned char *ctrl = (const unsigned char *)control;

  unsigned char map[32];
  int count;

  for (count=0; count<32; count++)
    map[count] = 0;

  while (*ctrl)
  {
    map[*ctrl >> 3] |= (1 << (*ctrl & 7));
    ctrl++;
  }

  while (*str)
  {
    if (map[*str >> 3] & (1 << (*str & 7)))
      return((char *)str);
    str++;
  }
  return(NULL);
}
