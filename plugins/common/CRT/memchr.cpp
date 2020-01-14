#include "crt.hpp"

#ifdef _MSC_VER
#pragma function(memchr)
#endif

_CONST_RETURN void * __cdecl memchr(const void *buf, int chr, size_t cnt)
{
  while (cnt && (*(unsigned char *)buf != (unsigned char)chr))
  {
    buf = (unsigned char *)buf + 1;
    cnt--;
  }
  return(cnt ? (void *)buf : NULL);
}

//---------------------------------------------------------------------------
_CONST_RETURN_W wchar_t * __cdecl_inline wmemchr(const wchar_t *buf, wchar_t chr, size_t cnt)
{
  while (cnt && (*buf != (wchar_t)chr))
  {
    buf++;
    cnt--;
  }
  return(cnt ? (wchar_t *)buf : NULL);
}
