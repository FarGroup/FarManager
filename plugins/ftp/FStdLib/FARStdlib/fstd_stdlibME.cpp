#include <all_far.h>
#pragma hdrstop

#include "fstdlib.h"

LPVOID DECLSPEC MemSet( LPVOID Buff, BYTE Val, size_t sz )
  {
    if ( !Buff || !sz ) return Buff;
  return memset( Buff,Val,sz );
}

LPVOID DECLSPEC MemMove( LPVOID dest, const void *src, size_t n)
  {
     if ( !dest || !src || !n ) return dest;
  return memmove( dest,src,n );
}

LPVOID DECLSPEC MemCpy( LPVOID dest, const void *src, size_t n)
  {
     if ( !dest || !src || !n ) return dest;
  return memcpy( dest,src,n );
}
