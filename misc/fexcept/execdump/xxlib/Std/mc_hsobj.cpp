#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif

/************************************
            HSafeObject
 ************************************/
HSafeObject::HSafeObject( DWORD id )
  {
    Usage = 0;
    SMID = id;
    Allocated = TRUE;
}
HSafeObject::~HSafeObject()                    { Allocated = FALSE; }

BOOL HSafeObject::Use( void )                  { return HSafeObject::Use(this); }
BOOL HSafeObject::Release( void )              { return HSafeObject::Release(this); }
void HSafeObject::Destroy( void )              { }
void HSafeObject::operator delete( void *ptr ) { ::delete ((PHSafeObject)ptr); }

PHSafeObject HSafeObject::Convert( LPVOID h,size_t sz,DWORD id )
  {
 return ( !h                                   ||
          !CheckReadable(h,sz)                 ||
          ((PHSafeObject)h)->Allocated != TRUE ||
          ((PHSafeObject)h)->SMID      != id
        )?NULL:((PHSafeObject)h);
}

BOOL HSafeObject::Use( PHSafeObject p )
  {
    if ( !p                                    ||
         !CheckReadable(p,sizeof(HSafeObject)) ||
         p->Allocated != TRUE
       ) return FALSE;

    p->Usage++;
 return TRUE;
}

BOOL HSafeObject::Release( PHSafeObject p )
  {
    if ( !p                                    ||
         !CheckReadable(p,sizeof(HSafeObject)) ||
         p->Allocated != TRUE
       ) return FALSE;

    if (p->Usage) p->Usage--;
    if (!p->Usage) {
      p->Destroy();
      delete p;
    }
 return TRUE;
}
