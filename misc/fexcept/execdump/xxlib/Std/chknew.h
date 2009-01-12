#ifndef __CHECKED_NEW
#define __CHECKED_NEW

#define TOTALMEM                 __GlobalMemTotalCount
#define USEDMEM                  __GlobalMemNewCount
#define USEDMEMCOUNTS            __GlobalMemCounts

#if defined(__REALDOS__)
    #define LOSTMEM farcoreleft()
#else
#if defined(__PROTDOS__)
    #define LOSTMEM _memavl()
#else
#if defined(__QNX__)
    #define LOSTMEM 100000UL
#else
#if defined(__HWIN__)
    #define LOSTMEM 100000UL
#else
#if defined(__TEC32__)
    #define LOSTMEM 100000UL
#else
#if defined(__GNUC__)
    #define LOSTMEM 100000UL
#else
  #error "correct LOSTMEM for this platform"
#endif
#endif
#endif
#endif
#endif
#endif

#define GET_USEDMEM( var )       var = USEDMEM
#define GET_USEDMEMCOUNTS( var ) var = USEDMEMCOUNTS
#define CMP_USEDMEM( evar,bvar ) (evar-bvar)
#define CMP_USEDMEM_FORMAT       "%ld"

#if defined( __cplusplus )
template <class T> void  FreePtr( T& p ) { if (p) delete p; p = NULL; }
#endif

HDECLSPEC LPVOID MYRTLEXP _Alloc( SIZE_T sz );
HDECLSPEC void   MYRTLEXP _Del( LPVOID ptr );
HDECLSPEC LPVOID MYRTLEXP _Realloc( LPVOID ptr,SIZE_T sz );
HDECLSPEC DWORD  MYRTLEXP _PtrSize( LPVOID ptr );
HDECLSPEC BOOL   MYRTLEXP _HeapCheck( void );

HDECLSPEC DWORD __GlobalMemTotalCount,
                __GlobalMemNewCount,
                __GlobalMemCounts;
HDECLSPEC BOOL  __HeapIgnoreErrors;

#endif
