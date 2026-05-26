#include "crt.hpp"

#if _MSC_VER >= 1951
 #pragma function(strncmp)
 #pragma function(wcsncmp)
#endif

int __cdecl
#ifndef UNICODE
            strncmp
#else
            wcsncmp
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
