#include "crt.hpp"
#include <stddef.h>
#include <limits.h>

#define FL_UNSIGNED   1
#define FL_NEG        2
#define FL_OVERFLOW   4
#define FL_READDIGIT  8

static unsigned long __cdecl strtoxl(const TCHAR *nptr, TCHAR **endptr, int ibase, int flags)
{
#ifndef UNICODE
typedef unsigned char uchar_t;
#define UCAST(c)  (int)(uchar_t)c
#else
typedef wchar_t   uchar_t;
#define UCAST(c)  (wint_t)c
#endif
  const TCHAR *p;
  TCHAR c;
  unsigned long number;
  unsigned digval;
  unsigned long maxval;

  p = nptr;
  number = 0;

  c = *p++;
  while ( _istspace(UCAST(c)) )
    c = *p++;

  if (c == _T('-'))
  {
    flags |= FL_NEG;
    c = *p++;
  }
  else if (c == _T('+'))
    c = *p++;

  if (ibase < 0 || ibase == 1 || ibase > 36)
  {
    if (endptr)
      *endptr = (TCHAR *)nptr;
    return 0L;
  }
  else if (ibase == 0)
  {
    if (c != _T('0'))
      ibase = 10;
    else if (*p == _T('x') || *p == _T('X'))
      ibase = 16;
    else
      ibase = 8;
  }

  if (ibase == 16)
  {
    if (c == _T('0') && (*p == _T('x') || *p == _T('X')))
    {
      ++p;
      c = *p++;
    }
  }

  maxval = ULONG_MAX / ibase;


  for (;;)
  {
    if ( _istdigit(UCAST(c)) )
      digval = c - _T('0');
    else if ( (uchar_t)c >= _T('a') )
      digval = (uchar_t)c - _T('a') + 10;
    else if ( (uchar_t)c >= _T('A') )
      digval = (uchar_t)c - _T('A') + 10;
    else
      break;
    if (digval >= (unsigned)ibase)
      break;

    flags |= FL_READDIGIT;

    if (number < maxval || (number == maxval && (unsigned long)digval <= ULONG_MAX % ibase))
      number = number * ibase + digval;
    else
      flags |= FL_OVERFLOW;

    c = *p++;
  }

  --p;

  if (!(flags & FL_READDIGIT))
  {
    if (endptr)
      p = nptr;
    number = 0L;
  }
  else if ( (flags & FL_OVERFLOW) || ( !(flags & FL_UNSIGNED) && ( ( (flags & FL_NEG) && (number > 0-(unsigned long)LONG_MIN) ) || ( !(flags & FL_NEG) && (number > LONG_MAX) ) ) ) )
  {
    if ( flags & FL_UNSIGNED )
      number = ULONG_MAX;
    else if ( flags & FL_NEG )
      number = 0-(unsigned long)(LONG_MIN);
    else
      number = LONG_MAX;
  }

  if (endptr != NULL)
    *endptr = (TCHAR *)p;

  if (flags & FL_NEG)
    number = (unsigned long)(-(long)number);

  return number;
}

long __cdecl
#ifndef UNICODE
             strtol
#else
             wcstol
#endif
                   (const TCHAR *nptr, TCHAR **endptr, int ibase)
{
  return (long) strtoxl(nptr, endptr, ibase, 0);
}

unsigned long __cdecl
#ifndef UNICODE
                      strtoul
#else
                      wcstoul
#endif
                             (const TCHAR *nptr, TCHAR **endptr, int ibase)
{
  return strtoxl(nptr, endptr, ibase, FL_UNSIGNED);
}

