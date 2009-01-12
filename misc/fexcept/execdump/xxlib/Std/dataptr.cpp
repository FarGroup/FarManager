#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif

#include <Std/dataptr.h>

/*******************************************************************
    HDataPtr - holder for dynamicaly allocated data
 *******************************************************************/
HDataPtr::HDataPtr( void )                         { Data = NULL; MaxSize = Size = 0; }
HDataPtr::HDataPtr( DWORD sz )                     { Data = NULL; MaxSize = Size = 0; Resize(sz); }
HDataPtr::HDataPtr( DWORD sz, LPVOID p,DWORD psz ) { Data = NULL; MaxSize = Size = 0; Resize(sz); Assign(p,psz); }
HDataPtr::HDataPtr( const HDataPtr& p )            { Data = NULL; MaxSize = Size = 0; if (p.Allocated()) Assign(p.Data,p.Size); }
HDataPtr::~HDataPtr()                              { Free(); }

LPVOID HDataPtr::Ptr( void )       const           { return Data; }
BOOL   HDataPtr::Allocated( void ) const           { return Data != NULL; }
DWORD  HDataPtr::Sizeof( void )    const           { return Size; }

BOOL HDataPtr::Assign( LPVOID p,DWORD sz )
  {
    if ( !Resize(sz) ) return FALSE;
    if (p) MemMove( Data,p,sz );
    Size = sz;
 return TRUE;
}

BOOL HDataPtr::Resize( DWORD sz )
  {  LPVOID ptr;

    if ( sz <= MaxSize ) {
      Size = sz;
      return TRUE;
    }

    ptr = _Alloc( sz );
    if ( !ptr )
      return FALSE;

    if (Data)
      MemCpy( ptr,Data,Size );

    Free();
    Size = MaxSize = sz;
    Data = ptr;

 return TRUE;
}

void HDataPtr::Free( void )
  {
    _Del(Data); Data = NULL;
    MaxSize = Size = 0;
}
/*******************************************************************
    HCharArray
 *******************************************************************/
HCharArray::HCharArray( int ssz,CONSTSTR str )
   : Len( strLen(str) ),
     HDataArray<char>(ssz,str,Len)
  {
}

void  HCharArray::Refresh( void )                   { Len = strLen(Text()); HDataArray<char>::Refresh(); }
char *HCharArray::Text( void )                const { return (char*)(Start() ? Start() : ""); }
char *HCharArray::c_str( void )               const { return Text(); }
int   HCharArray::Length( void )              const { return Len; }
char *HCharArray::Set( char *s )                    { return Alloc(s,strLen(s)); }
void  HCharArray::SetChar( int num,char v )         { (*this)[num] = v; Refresh(); }
BOOL  HCharArray::Cmp( CONSTSTR s,int count ) const { return StrCmp(Text(),s,count) == 0; }
void  HCharArray::Add( char ch )                    { if (ch) {(*this)[Len++] = ch; (*this)[Len] = 0;} }
