#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif

#if defined(__GUARD_MEMORY__)
  #define CHK(v)   TraceAssert( _HeapCheck() && v );
#else
  #define CHK(v)
#endif

LPVOID MYRTLEXP MemSet( LPVOID Buff, BYTE Val, size_t sz )
  {
    if ( !Buff || !sz ) return Buff;

    CHK( "Before memset" )
      LPVOID rc = memset( Buff,Val,sz );
    CHK( "After memset" )
 return rc;
}

LPVOID MYRTLEXP MemMove( LPVOID dest, const void *src, size_t n)
  {
    if ( !dest || !src || !n ) return dest;

    CHK( "Before memmove" )
      LPVOID rc = memmove( dest,src,n );
    CHK( "After memmove" )
  return rc;
}

LPVOID MYRTLEXP MemCpy( LPVOID dest, const void *src, size_t n)
  {
    if ( !dest || !src || !n ) return dest;
    CHK( "Before memcpy" )
      LPVOID rc = memcpy( dest,src,n );
    CHK( "After memcpy" )
  return rc;
}
