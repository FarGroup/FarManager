#include "crt.hpp"

_CONST_RETURN char * __cdecl strpbrk(const char *string, const char *control)
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

//----------------------------------------------------------------------------
_CONST_RETURN_W wchar_t * __cdecl wcspbrk(const wchar_t *string, const wchar_t *control)
{
  for ( ; *string; string++)
    for (const wchar_t *wcset = control; *wcset; wcset++)
      if (*wcset == *string) return((wchar_t *)string);
  return(NULL);
}

