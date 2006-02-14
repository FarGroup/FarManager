#include "strcspn.hpp"

size_t strcspn(const char *string, const char *control)
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

  count=0;
  map[0] |= 1;
  while (!(map[*str >> 3] & (1 << (*str & 7))))
  {
    count++;
    str++;
  }

  return(count);
}
