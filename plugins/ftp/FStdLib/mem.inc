/*******************************************************************
   OPERATORS
 *******************************************************************/
void __cdecl operator delete( void *ptr )   { _Del( ptr ); }
void *__cdecl operator new( size_t sz )     { void *p = _Alloc(sz); if (p) memset( p, 0, sz ); return p; }

#if defined( __HWIN__ ) || defined( __QNX__ )
void __cdecl operator delete[]( void *ptr ) { _Del( ptr ); }
void *__cdecl operator new[]( size_t sz )   { void *p = _Alloc(sz); if (p) memset( p, 0, sz ); return p; }
#endif

/*******************************************************************
   STDLIB
 *******************************************************************/
#if !defined(__STD_MEMORY__)
#endif

/*******************************************************************
   IMPLEMENTATION
 *******************************************************************/
#if defined(__DEBUG__)
  static BOOL InTrap = FALSE;
  #define CHK_MEM   if ( !InTrap && !_HeapCheck() ) { InTrap = TRUE; __WinAbort( "Heap corrupt. Found at caller." ); }
#else
  #define CHK_MEM
#endif

#if defined(__HEAP_MEMORY__)
  extern LPVOID WINAPI __Alloc_H( SIZE_T sz );
  extern LPVOID WINAPI __Realloc_H( LPVOID ptr,SIZE_T sz );
  extern void   WINAPI __Del_H( LPVOID ptr );
  extern SIZE_T WINAPI __PtrSize_H( LPVOID ptr );
  extern BOOL   WINAPI __HeapCheck_H( void );

  LPVOID WINAPI _Alloc( SIZE_T sz )             { CHK_MEM return __Alloc_H( sz ); }
  LPVOID WINAPI _Realloc( LPVOID ptr,SIZE_T sz ){ CHK_MEM return __Realloc_H( ptr,sz ); }
  void   WINAPI _Del( LPVOID ptr )              { CHK_MEM __Del_H( ptr ); }
  SIZE_T WINAPI _PtrSize( LPVOID ptr )          { CHK_MEM return __PtrSize_H( ptr ); }
  BOOL   WINAPI _HeapCheck( void )              { return __HeapCheck_H(); }
#else
#if defined(__STD_MEMORY__)
  extern LPVOID WINAPI __Alloc_S( SIZE_T sz );
  extern LPVOID WINAPI __Realloc_S( LPVOID ptr,SIZE_T sz );
  extern void   WINAPI __Del_S( LPVOID ptr );
  extern SIZE_T WINAPI __PtrSize_S( LPVOID ptr );
  extern BOOL   WINAPI __HeapCheck_S( void );

  LPVOID WINAPI _Alloc( SIZE_T sz )             { CHK_MEM return __Alloc_S( sz ); }
  LPVOID WINAPI _Realloc( LPVOID ptr,SIZE_T sz ){ CHK_MEM return __Realloc_S( ptr,sz ); }
  void   WINAPI _Del( LPVOID ptr )              { CHK_MEM __Del_S( ptr ); }
  SIZE_T WINAPI _PtrSize( LPVOID ptr )          { CHK_MEM return __PtrSize_S( ptr ); }
  BOOL   WINAPI _HeapCheck( void )              { return __HeapCheck_S(); }
#else
  #error "Memory allocation types not defined"
#endif
#endif
