#include <all_far.h>
#pragma hdrstop

#include "fstdlib.h"

class THeapStatus {
  public:
    DWORD TotalAddrSpace;
    DWORD TotalUncommitted;
    DWORD TotalCommitted;
    DWORD TotalAllocated;
    DWORD TotalFree;
    DWORD FreeSmall;
    DWORD FreeBig;
    DWORD Unused;
    DWORD Overhead;
    DWORD HeapErrorCode;
};

extern "C" {
  void *      __cdecl GetMemory(size_t Size);
  void *      __cdecl ReallocMemory(void * P, size_t Size);
  int         __cdecl FreeMemory(void * P);
  THeapStatus __cdecl GetHeapStatus();
};

LPVOID DECLSPEC __Alloc_B( SIZE_T sz )              { return GetMemory(sz); }
LPVOID DECLSPEC __Realloc_B( LPVOID ptr,SIZE_T sz ) { return (!ptr) ? GetMemory(sz) : ReallocMemory(ptr,sz); }
void   DECLSPEC __Del_B( LPVOID ptr )               { if (ptr) FreeMemory(ptr); }
SIZE_T DECLSPEC __PtrSize_B( LPVOID ptr )           { return 0; }
BOOL   DECLSPEC __HeapCheck_B( void )               { THeapStatus st = GetHeapStatus(); return st.HeapErrorCode == 0; }
