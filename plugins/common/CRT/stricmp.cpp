#include "crt.hpp"

#define TOLOWER(c) ( ((c) >= 'A') && ((c) <= 'Z') ? ((c) - 'A' + 'a') : (c) )

int __cdecl
#ifndef UNICODE
            _strnicmp
#else
            _wcsnicmp
#endif
                     (const TCHAR *first, const TCHAR *last, size_t count)
{
  if (!count)
    return(0);

  while (--count && *first && *first == *last)
  {
    first++;
    last++;
  }
#ifndef UNICODE
  return (*(unsigned char *)first - *(unsigned char *)last);
#else
  return ((int)(*first - *last));
#endif
}

//----------------------------------------------------------------------------
int __cdecl
#ifndef UNICODE
            _stricmp
#else
            _wcsicmp
#endif
                    (const TCHAR *first, const TCHAR *last)
{
  while (*first && *first == *last)
  {
    first++;
    last++;
  }
#ifndef UNICODE
  return (*(unsigned char *)first - *(unsigned char *)last);
#else
  return ((int)(*first - *last));
#endif
}
