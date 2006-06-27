/*******************************************************************
   OPERATORS
 *******************************************************************/
void __cdecl operator delete( void *ptr )   { _Del( ptr ); }
void *__cdecl operator new( size_t sz )     { void *p = _Alloc(sz); if (!p) return NULL; memset( p, 0, sz ); return p; }

#if defined(__BORLANDC__)
 #if sizeof(size_t) < sizeof(long)
  void *operator new( unsigned long sz )    { void *p = _Alloc(sz); if (!p) return NULL; memset( p, 0, sz ); return p; }
 #endif
#endif

#if defined( __HWIN__ ) || defined( __QNX__ )
void __cdecl operator delete[]( void *ptr ) { _Del( ptr ); }
void *__cdecl operator new[]( size_t sz )   { void *p = _Alloc(sz); if (!p) return NULL; memset( p, 0, sz ); return p; }
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
  #define CHK_MEM   if ( !InTrap && !_HeapCheck() ) { InTrap = TRUE; __TrapLog( "Heap corrupt. Found at caller." ); }
#else
  #define CHK_MEM
#endif

#if defined(__BMM_MEMORY__)
  extern LPVOID DECLSPEC __Alloc_B( DWORD sz );
  extern LPVOID DECLSPEC __Realloc_B( LPVOID ptr,DWORD sz );
  extern void   DECLSPEC __Del_B( LPVOID ptr );
  extern DWORD  DECLSPEC __PtrSize_B( LPVOID ptr );
  extern BOOL   DECLSPEC __HeapCheck_B( void );

  LPVOID DECLSPEC _Alloc( DWORD sz )              { CHK_MEM return __Alloc_B( sz ); }
  LPVOID DECLSPEC _Realloc( LPVOID ptr,DWORD sz ) { CHK_MEM return __Realloc_B( ptr,sz ); }
  void   DECLSPEC _Del( LPVOID ptr )              { CHK_MEM __Del_B( ptr ); }
  DWORD  DECLSPEC _PtrSize( LPVOID ptr )          { CHK_MEM return __PtrSize_B( ptr ); }
  BOOL   DECLSPEC _HeapCheck( void )              { return __HeapCheck_B(); }
#else
#if defined(__HEAP_MEMORY__)
  extern LPVOID DECLSPEC __Alloc_H( DWORD sz );
  extern LPVOID DECLSPEC __Realloc_H( LPVOID ptr,DWORD sz );
  extern void   DECLSPEC __Del_H( LPVOID ptr );
  extern DWORD  DECLSPEC __PtrSize_H( LPVOID ptr );
  extern BOOL   DECLSPEC __HeapCheck_H( void );

  LPVOID DECLSPEC _Alloc( DWORD sz )              { CHK_MEM return __Alloc_H( sz ); }
  LPVOID DECLSPEC _Realloc( LPVOID ptr,DWORD sz ) { CHK_MEM return __Realloc_H( ptr,sz ); }
  void   DECLSPEC _Del( LPVOID ptr )              { CHK_MEM __Del_H( ptr ); }
  DWORD  DECLSPEC _PtrSize( LPVOID ptr )          { CHK_MEM return __PtrSize_H( ptr ); }
  BOOL   DECLSPEC _HeapCheck( void )              { return __HeapCheck_H(); }
#else
#if defined(__STD_MEMORY__)
  extern LPVOID DECLSPEC __Alloc_S( DWORD sz );
  extern LPVOID DECLSPEC __Realloc_S( LPVOID ptr,DWORD sz );
  extern void   DECLSPEC __Del_S( LPVOID ptr );
  extern DWORD  DECLSPEC __PtrSize_S( LPVOID ptr );
  extern BOOL   DECLSPEC __HeapCheck_S( void );

  LPVOID DECLSPEC _Alloc( DWORD sz )              { CHK_MEM return __Alloc_S( sz ); }
  LPVOID DECLSPEC _Realloc( LPVOID ptr,DWORD sz ) { CHK_MEM return __Realloc_S( ptr,sz ); }
  void   DECLSPEC _Del( LPVOID ptr )              { CHK_MEM __Del_S( ptr ); }
  DWORD  DECLSPEC _PtrSize( LPVOID ptr )          { CHK_MEM return __PtrSize_S( ptr ); }
  BOOL   DECLSPEC _HeapCheck( void )              { return __HeapCheck_S(); }
#else
  #error "Memory allocation types not defined"
#endif
#endif
#endif
