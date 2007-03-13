#include "crt.hpp"

size_t __cdecl strcspn(const char *string, const char *control)
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

//----------------------------------------------------------------------------
size_t __cdecl wcscspn(const wchar_t *string, const wchar_t *control)
{
  const wchar_t *str =string;

  for ( ; *str; str++)
    for (const wchar_t *wcset = control; *wcset; wcset++)
      if (*wcset == *str) goto done;
done:
  return((size_t)(str - string));
}
