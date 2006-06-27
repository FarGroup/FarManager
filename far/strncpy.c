#define __NEW_H
#include "headers.hpp"
#pragma hdrstop

// dest и src НЕ ДОЛЖНЫ пересекаться
// maxlen - максимальное число символов, которое можно скопировать
//          в dest БЕЗ учета заключительного нуля, т.е. в общем
//          случае это "sizeof-1"
char * __cdecl xstrncpy (char * dest,const char * src,size_t maxlen)
{
  char *tmpsrc = dest;
  while (maxlen && 0 != (*dest++ = *src++))
    --maxlen;
  *dest = 0;
  return tmpsrc;
}

wchar_t * __cdecl xwcsncpy (wchar_t * dest,const wchar_t * src,size_t maxlen)
{
  wchar_t *tmpsrc = dest;
  while (maxlen && 0 != (*dest++ = *src++))
    --maxlen;
  *dest = 0;
  return tmpsrc;
}
