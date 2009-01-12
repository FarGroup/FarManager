#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif

#define HDP_MAGIC  MK_ID( 'B','T','r','e' )   //Used as b-tree ID in IO operations
#define HDP_DELTA  0

#if 1
  #define  PROC( v )
  #define Log( v )
#else
  #define  PROC( v )  INProc __proc v ;
  #define  Log( v )   INProc::Say v
#endif

//---------------------------------------------------------------------------
LOCALSTRUCT( DataItem )
  LPVOID Data;
  DWORD  Size;
  DWORD  MaxSize;

  DataItem( void )                    { Detach(); }
  DataItem( LPVOID d, DWORD sz  )     { Detach(); SetData(d,sz); }
  DataItem( DataItem& p );
  ~DataItem()                         { Free(); }

  void Free( void );
  void Detach( void )                 { Data = NULL; Size = MaxSize = 0; }
  BOOL SetData( LPVOID d, DWORD sz );
};

DataItem::DataItem( DataItem& p )
  {
    Data    = p.Data;
    Size    = p.Size;
    MaxSize = p.MaxSize;

    p.Detach();
}

void DataItem::Free( void )
  {
    if ( Data ) {
      Log(( "DataItem::Free %p[%d]", Data, Size ));
      _Del(Data);
    }
    Detach();
}

BOOL DataItem::SetData( LPVOID d, DWORD sz )
  {  PROC(( "DataItem::SetData", "%p[%d] (%p,%d,%d)", d, sz, Data, Size, MaxSize ))

    if ( !sz ) {
      Log(( "Zero size" ));
      Size = 0;
      return TRUE;
    }

    if ( sz < MaxSize ) {
      Log(( "Fit to maxsize" ));
      Size = sz;
      if (d) MemMove( Data, d, sz );
      return TRUE;
    }

    _Del( Data );
    Data = _Alloc( sz+HDP_DELTA+1 );
    Log(( "Reallocated %d->%d=%p", Size, sz, Data ));
    if ( !Data ) {
      Log(( "!allocate data" ));
      Detach();
      return FALSE;
    }

    MaxSize = sz+HDP_DELTA;
    Size    = sz;
    if (d) MemMove( Data, d, sz );

 return TRUE;
}

class DataArray : public MyArray<PDataItem> {
  public:
    DataArray( void ) {}
};

//---------------------------------------------------------------------------
class HMemDataPool1 : public HDataPool {
    DataArray Items;
  public:
    HMemDataPool1( void ) {}

    virtual void   Clear( void )                               { Items.DeleteAll(); }
    virtual BOOL   LocateData( HANDLE h )                      { return Items.IndexOf( (PDataItem)h ) != -1; }
    virtual HANDLE CreateData( LPVOID Data,DWORD sz );
    virtual BOOL   DeleteData( HANDLE h )                      { Items.Delete( (PDataItem)h ); return TRUE; }
    virtual BOOL   SetData( HANDLE h,LPVOID Data,DWORD sz )    { return ((PDataItem)h)->SetData( Data,sz ); }
    virtual LPVOID GetData( HANDLE h )                         { return ((PDataItem)h)->Data; }
    virtual DWORD  GetDataSize( HANDLE h )                     { return ((PDataItem)h)->Size; }

    virtual BOOL   SaveItem( HANDLE h,int File );
    virtual HANDLE LoadItem( int File );
    virtual BOOL   Load( CONSTSTR FileName );
    virtual BOOL   Save( CONSTSTR FileName );
};

//---------------------------------------------------------------------------
HANDLE HMemDataPool1::CreateData( LPVOID Data,DWORD sz )
  {  PROC(( "HMemDataPool1::CreateData", "%p[%d]", Data, sz ))

     DataItem p;
     if ( !p.SetData(Data,sz) )
       return NULL;

 return (HANDLE)Items.Add( new DataItem( p ) );
}

BOOL HMemDataPool1::SaveItem( HANDLE h,int File )
  {  PDataItem p = (PDataItem)h;

     if ( FIO_WRITE(File,&p->Size,sizeof(p->Size)) != sizeof(p->Size) )
       return FALSE;

     if ( p->Size &&
          FIO_WRITE(File,p->Data,p->Size) != (int)p->Size )
       return FALSE;

     Log(( "Item %p saved", h ));

 return TRUE;
}
HANDLE HMemDataPool1::LoadItem( int File )
  {  PROC(( "HMemDataPool1::LoadItem", NULL ))
     DataItem p;
     DWORD    dw;

     if ( FIO_READ(File,&dw,sizeof(dw)) != sizeof(dw) ) {
       Log(( "!read size" ));
       return NULL;
     }

     if ( !p.SetData(NULL,dw) )
       return NULL;

     if ( dw &&
          FIO_READ(File,p.Data,dw) != (int)dw ) {
       Log(( "!read data [%d]",dw ));
       return FALSE;
     }

     Log(( "Item with data %p loaded", p.Data ));

 return (HANDLE)Items.Add( new DataItem(p) );
}

BOOL   HMemDataPool1::Load( CONSTSTR FileName ) { USEARG(FileName) return FALSE; }
BOOL   HMemDataPool1::Save( CONSTSTR FileName ) { USEARG(FileName) return FALSE; }

//---------------------------------------------------------------------------
HDECLSPEC PHDataPool MYRTLEXP CreateMemDataPool( void )
  {
  return new HMemDataPool1;
}
