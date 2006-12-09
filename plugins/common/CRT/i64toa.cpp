#include "crt.hpp"

static void __cdecl x64toa(unsigned __int64 val, char *buf, unsigned radix, int is_neg)
{
  char *p;
  char *firstdig;
  char temp;
  unsigned digval;

  p = buf;

  if ( is_neg )
  {
    *p++ = '-';
    val = (unsigned __int64)(-(__int64)val);
  }

  firstdig = p;

  do {
    digval = (unsigned) (val % radix);
    val /= radix;

    if (digval > 9)
      *p++ = (char) (digval - 10 + 'a');
    else
      *p++ = (char) (digval + '0');
  } while (val > 0);

  *p-- = '\0';

  do {
    temp = *p;
    *p = *firstdig;
    *firstdig = temp;
    --p;
    ++firstdig;
  } while (firstdig < p);
}

char * __cdecl _i64toa(__int64 val, char *buf, int radix )
{
  x64toa((unsigned __int64)val, buf, radix, (radix == 10 && val < 0));
  return buf;
}

char * __cdecl _ui64toa(unsigned __int64 val, char *buf, int radix)
{
  x64toa(val, buf, radix, 0);
  return buf;
}
