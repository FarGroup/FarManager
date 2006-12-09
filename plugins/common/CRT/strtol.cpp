#include "crt.hpp"
#include <stddef.h>
#include <limits.h>

#define FL_UNSIGNED   1
#define FL_NEG        2
#define FL_OVERFLOW   4
#define FL_READDIGIT  8

static unsigned long __cdecl strtoxl(const char *nptr, char **endptr, int ibase, int flags)
{
  const char *p;
  char c;
  unsigned long number;
  unsigned digval;
  unsigned long maxval;

  p = nptr;
  number = 0;

  c = *p++;
  while ( isspace((int)(unsigned char)c) )
    c = *p++;

  if (c == '-')
  {
    flags |= FL_NEG;
    c = *p++;
  }
  else if (c == '+')
    c = *p++;

  if (ibase < 0 || ibase == 1 || ibase > 36)
  {
    if (endptr)
      *endptr = (char *)nptr;
    return 0L;
  }
  else if (ibase == 0)
  {
    if (c != '0')
      ibase = 10;
    else if (*p == 'x' || *p == 'X')
      ibase = 16;
    else
      ibase = 8;
  }

  if (ibase == 16)
  {
    if (c == '0' && (*p == 'x' || *p == 'X'))
    {
      ++p;
      c = *p++;
    }
  }

  maxval = ULONG_MAX / ibase;


  for (;;)
  {
    if ( isdigit((int)(unsigned char)c) )
      digval = c - '0';
    else if ( (unsigned char)c >= 'a' )
      digval = (unsigned char)c - 'a' + 10;
    else if ( (unsigned char)c >= 'A' )
      digval = (unsigned char)c - 'A' + 10;
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
  else if ( (flags & FL_OVERFLOW) || ( !(flags & FL_UNSIGNED) && ( ( (flags & FL_NEG) && (number > -LONG_MIN) ) || ( !(flags & FL_NEG) && (number > LONG_MAX) ) ) ) )
  {
    if ( flags & FL_UNSIGNED )
      number = ULONG_MAX;
    else if ( flags & FL_NEG )
      number = (unsigned long)(-LONG_MIN);
    else
      number = LONG_MAX;
  }

  if (endptr != NULL)
    *endptr = (char *)p;

  if (flags & FL_NEG)
    number = (unsigned long)(-(long)number);

  return number;
}

long __cdecl strtol(const char *nptr, char **endptr, int ibase)
{
  return (long) strtoxl(nptr, endptr, ibase, 0);
}

unsigned long __cdecl strtoul(const char *nptr, char **endptr, int ibase)
{
  return strtoxl(nptr, endptr, ibase, FL_UNSIGNED);
}
