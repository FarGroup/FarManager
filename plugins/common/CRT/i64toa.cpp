#include "crt.hpp"

static void __cdecl x64toaw(unsigned __int64 val, TCHAR *buf, unsigned radix, int is_neg)
{
  TCHAR *p;
  TCHAR *firstdig;
  TCHAR temp;
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
      *p++ = (TCHAR) (digval - 10 + 'a');
    else
      *p++ = (TCHAR) (digval + '0');
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

//----------------------------------------------------------------------------
TCHAR * __cdecl
#ifndef UNICODE
               _i64toa
#else
               _i64tow
#endif
                      (__int64 val, TCHAR *buf, int radix)
{
  x64toaw((unsigned __int64)val, buf, radix, (radix == 10 && val < 0));
  return buf;
}

//----------------------------------------------------------------------------
TCHAR * __cdecl
#ifndef UNICODE
               _ui64toa
#else
               _ui64tow
#endif
                       (unsigned __int64 val, TCHAR *buf, int radix)
{
  x64toaw(val, buf, radix, 0);
  return buf;
}

//----------------------------------------------------------------------------
