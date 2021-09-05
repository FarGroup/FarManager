#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif

#define _NO_SELF_ALLOC  1

/*******************************************************************
   PACKAGE remove memory management
 *******************************************************************/
#if defined(__MYPACKAGE__)
  #define _NO_SELF_ALLOC 1
#endif

/*******************************************************************
   OPERATORS
 *******************************************************************/
/* Disable operators reload on MS dll with dynamic RTL
*/
#if defined(__MSWIN32__)
  #if defined(__DLL__) && defined(__VC_USE_DYNRTL__)
    #define DISABLE_OPERATORS_RELOAD 1
  #endif
  #if defined(__DLL__) && defined(__VC_USE_IC__)
    #define DISABLE_OPERATORS_RELOAD 1
  #endif
#endif

#if !defined(DISABLE_OPERATORS_RELOAD)
  void __cdecl operator delete( void *ptr )   { _Del( ptr ); }
  void *__cdecl operator new( size_t sz )     { return _Alloc(sz); }

  #if defined(__BORLANDC__)
    #if sizeof(size_t) < sizeof(long)
     void *operator new( unsigned long sz )    { return _Alloc(sz); }
    #endif
  #endif

  #if defined( __HWIN__ ) || defined( __QNX__ )
    void __cdecl operator delete[]( void *ptr ) { _Del( ptr ); }
    void *__cdecl operator new[]( size_t sz )   { return _Alloc(sz); }
  #endif
#endif //DISABLE_OPERATORS_RELOAD

/*******************************************************************
   VCL
 *******************************************************************/
#if defined(__VCL__)

#define GMM TMemoryManager mm; GetMemoryManager(mm);

LPVOID MYRTLEXP __Alloc( SIZE_T sz )              { GMM return mm.GetMem( (int)sz ); }
void   MYRTLEXP __Del( LPVOID ptr )               { GMM mm.FreeMem(ptr); }
LPVOID MYRTLEXP __Realloc( LPVOID ptr,SIZE_T sz ) { GMM return ptr ? mm.ReallocMem(ptr,sz) : mm.GetMem( (int)sz ); }
DWORD  MYRTLEXP __PtrSize( LPVOID ptr )           { GMM return 0; }
BOOL   MYRTLEXP __HeapCheck( void )               { return TRUE; }

#else
/*******************************************************************
    WIN32
 *******************************************************************/
#if defined(__HWIN3_) && !defined(__USE_STD_HEAP__)

static HANDLE Heap = NULL;
#define CHH  if ( !Heap ) Heap = HeapCreate(0,0x400000,0);

LPVOID MYRTLEXP __Alloc( SIZE_T sz )              { CHH return HeapAlloc(Heap,0,sz); }
LPVOID MYRTLEXP __Realloc( LPVOID ptr,SIZE_T sz ) { CHH return HeapReAlloc(Heap,0,ptr,sz); }
void   MYRTLEXP __Del( LPVOID ptr )               { CHH HeapFree(Heap,0,ptr); }
DWORD  MYRTLEXP __PtrSize( LPVOID ptr )           { CHH return HeapSize(Heap,0,ptr); }

BOOL MYRTLEXP __HeapCheck( void )
  {  BOOL rc = TRUE;

     CHH

    do{
      if ( __HeapIgnoreErrors )
        break;

      __try{
        rc = HeapValidate(GetProcessHeap(),0,NULL);
      }__except( EXCEPTION_EXECUTE_HANDLER ) {
        rc = FALSE;
      }
    }while(0);

 return rc;
}

#else
/*******************************************************************
   STDLIB
 *******************************************************************/
LPVOID MYRTLEXP __Alloc( SIZE_T sz )              { return malloc(sz); }
void   MYRTLEXP __Del( LPVOID ptr )               { free(ptr); }
LPVOID MYRTLEXP __Realloc( LPVOID ptr,SIZE_T sz ) { return realloc(ptr,sz); }
DWORD  MYRTLEXP __PtrSize( LPVOID /*ptr*/ )       { return 0; }

#if defined(__BCWIN16__)
BOOL   MYRTLEXP __HeapCheck( void )               { return TRUE; }
#else
#if defined(__BORLAND)
BOOL   MYRTLEXP __HeapCheck( void )               { return __HeapIgnoreErrors || heapcheck() == _HEAPOK; }
#else
#if defined(__QNX__)
BOOL   MYRTLEXP __HeapCheck( void )               { return __HeapIgnoreErrors || _heapchk() == _HEAPOK; }
#else
#if defined(__MSOFT) || defined(__INTEL)
BOOL   MYRTLEXP __HeapCheck( void )               { return __HeapIgnoreErrors || _heapchk() == _HEAPOK; }
#else
#if defined(__SYMANTEC)
BOOL   MYRTLEXP __HeapCheck( void )               { return __HeapIgnoreErrors || ??/*_heapchk() == _HEAPOK*/; }
#else
#if defined(__TEC32__)
BOOL   MYRTLEXP __HeapCheck( void )               { return TRUE; }
#else
#if defined(__GNUC__)
BOOL   MYRTLEXP __HeapCheck( void )               { return TRUE; }
#else
#error ERR_PLATFORM
#endif //GNUC
#endif //TEC
#endif //SYMANTEC
#endif //MSOFT || INTEL
#endif //QNX
#endif //BCWIN16
#endif //BORLAND
#endif //HWIN && !defined(__USE_STD_HEAP__)
#endif //VCL

/*******************************************************************
   STATISTICS DATA
 *******************************************************************/
DWORD __GlobalMemTotalCount = 0;
DWORD __GlobalMemNewCount   = 0;
DWORD __GlobalMemCounts     = 0;
BOOL  __HeapIgnoreErrors    = FALSE;

/*******************************************************************
   DO NOT MANAGE MEMORY
 *******************************************************************/
#if defined(_NO_SELF_ALLOC)
LPVOID MYRTLEXP _Alloc( SIZE_T sz )              { return __Alloc(sz); }
void   MYRTLEXP _Del( LPVOID ptr )               { if (ptr) __Del(ptr); }
LPVOID MYRTLEXP _Realloc( LPVOID ptr,SIZE_T sz ) { return ptr?__Realloc(ptr,sz):__Alloc(sz); }
DWORD  MYRTLEXP _PtrSize( LPVOID ptr )           { return ptr?__PtrSize(ptr):0; }
BOOL   MYRTLEXP _HeapCheck( void )               { return __HeapCheck(); }
#else
/*******************************************************************
   MANAGE MEMORY
 *******************************************************************/
#if defined( __HWIN__ )
 #if defined(__VCL__)
  #define EXCEPT_REACTION(ptr,err,ret) throw Exception(AnsiString("CHKNEW:: ")+err );
 #else
  #define EXCEPT_REACTION(ptr,err,ret) __WinAbort( "NEW|DELETE error: [%s] \"%s:%d\"",err,__FILE__,__LINE__ );
 #endif
#else
#if defined( __REALDOS__ )
  #define EXCEPT_REACTION(ptr,err,ret) { THROW_ERROR(err,chknew.cpp,0)                                    \
                                         HAbort("coreleft:%ld\nALLOC ERROR: \"%s\"\n",farcoreleft(),err ); \
                                         return ret }
#else
#if defined(__GNUC__) || defined( __QNX__ )
  #define EXCEPT_REACTION(ptr,err,ret) { HAbort("\nALLOC ERROR: \"%s\"\n",err); return ret }
#else
#if defined( __PROTDOS__ )
  #define EXCEPT_REACTION(ptr,err,ret) { HAbort("\nALLOC ERROR: \"%s\"\n",err); return ret };
#else
#error ERR_PLATFORM
#endif  //PROTDOS
#endif  //QNX, GNUC
#endif  //MSDOS
#endif  //WIN32

#define MEMID_1 'M'
#define MEMID_2 'E'

#include <Global/pack1.h>
typedef struct {
   char  memID[2];
   BOOL  free;
   DWORD size;
               } MemoryInfoStruct;
#include <Global/pop.h>

//---------------------------------------------------------------------------
LPVOID  MYRTLEXP _Alloc( SIZE_T sz )
  {  MemoryInfoStruct *nptr;

    if ( __GlobalMemTotalCount == 0 ) {
#if defined( __HWIN__ )
      MEMORYSTATUS st;
      st.dwLength = sizeof(st);
      GlobalMemoryStatus( &st );
      __GlobalMemTotalCount = st.dwAvailPhys/1000;
#else
#if defined( __REALDOS__ )
      __GlobalMemTotalCount = farcoreleft();
#else
#if defined( __QNX__ )
    struct _osinfo info;
    qnx_osinfo( 0,&info );
    __GlobalMemTotalCount = info.totmemk;
#else
#if defined( __PROTDOS__ )
    __GlobalMemTotalCount = _memavl();
#else
    __GlobalMemTotalCount = 0;
#endif  //PROTDOS
#endif  //QNX
#endif  //MSDOS
#endif  //WIN32
    }

     sz += sizeof(MemoryInfoStruct);
     nptr = (MemoryInfoStruct*)__Alloc( sz );
     if ( nptr == NULL )
       EXCEPT_REACTION( nptr,"Alloc::new == NULL",NULL; )

    __GlobalMemNewCount += sz;
    __GlobalMemCounts++;

    nptr->memID[0] = MEMID_1;
    nptr->memID[1] = MEMID_2;
    nptr->size     = sz;
    nptr->free     = FALSE;

#ifdef __NEW_DEBUG
    printf( "▓ %8ld %8ld, My: %8ld, MyLeft: %8ld, Core: %8ld\n",
            sz-sizeof(heapinfo)-sizeof(MemoryInfoStruct),
            __GlobalMemCounts,__GlobalMemNewCount,
            __GlobalMemTotalCount - __GlobalMemNewCount,
            farcoreleft() );
#endif

    return nptr+1;
}
//---------------------------------------------------------------------------
DWORD  MYRTLEXP _PtrSize( LPVOID ptr )
  {  if ( !ptr ) return 0 ;

     MemoryInfoStruct *nptr = ((MemoryInfoStruct*)ptr)-1;
return (nptr->memID[0] != MEMID_1 || nptr->memID[1] != MEMID_2 || nptr->free == TRUE)?0:nptr->size;
}
//---------------------------------------------------------------------------
BOOL MYRTLEXP _HeapCheck( void )
  {
  return TRUE;
}
//---------------------------------------------------------------------------
void  MYRTLEXP _Del( LPVOID ptr )
  {  if ( !ptr ) return;

     MemoryInfoStruct *nptr = ((MemoryInfoStruct*)ptr)-1;

//Check allocated block
    if ( nptr->memID[0] != MEMID_1 || nptr->memID[1] != MEMID_2 )
      EXCEPT_REACTION( nptr+1,"Del::[bad block ID]",; )
    if ( __GlobalMemCounts == 0 || __GlobalMemNewCount < nptr->size )
      EXCEPT_REACTION( nptr+1,"Del::[block seems to not been allocated]",; )
    if ( nptr->free == TRUE )
      EXCEPT_REACTION( nptr+1,"Del::[block alredy freed]",; )
//Correct pointer
    nptr->free = TRUE;
//Correct account variables
    __GlobalMemNewCount -= nptr->size;
    __GlobalMemCounts--;
//Realy free
    __Del( nptr );
#if defined(__QNX__)
//    _heapshrink();
#endif
#ifdef __NEW_DEBUG
    printf( "- %8ld %8ld, My: %8ld, MyLeft: %8ld, Core: %8ld\n",
            nptr->size-sizeof(MemoryInfoStruct),
            __GlobalMemCounts,__GlobalMemNewCount,
            __GlobalMemTotalCount - __GlobalMemNewCount,
            farcoreleft() );
#endif
}
//---------------------------------------------------------------------------
LPVOID  MYRTLEXP _Realloc( LPVOID ptr,SIZE_T sz )
  {  MemoryInfoStruct *nptr;

    if ( !ptr ) return _Alloc( sz );

    nptr = ((MemoryInfoStruct*)ptr)-1;
    //Check allocated block
    if ( nptr->memID[0] != MEMID_1 || nptr->memID[1] != MEMID_2 )
      EXCEPT_REACTION( nptr+1,"Realloc::[bad block ID]",NULL; )
    if ( __GlobalMemCounts == 0 || __GlobalMemNewCount < nptr->size )
      EXCEPT_REACTION( nptr+1,"Realloc::[block seems to not been allocated]",NULL; )
    if ( nptr->free == TRUE )
      EXCEPT_REACTION( nptr+1,"Realloc::[block alredy freed]",NULL; )
    //Correct pointer
    nptr->free = TRUE;
    //Correct account variables
    __GlobalMemNewCount -= nptr->size;
    __GlobalMemCounts--;
//Realloc
     sz += sizeof(MemoryInfoStruct);
     nptr = (MemoryInfoStruct*)__Realloc( nptr,sz );
     if ( nptr == NULL )
       EXCEPT_REACTION( nptr,"Realloc::[new == NULL]",NULL; )

    __GlobalMemNewCount += sz;
    __GlobalMemCounts++;

    nptr->memID[0] = MEMID_1;
    nptr->memID[1] = MEMID_2;
    nptr->size = sz;
    nptr->free = FALSE;
return nptr+1;
}
#endif
