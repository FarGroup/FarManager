#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif

/*******************************************************************
   HTreeRegValueBase
 *******************************************************************/
HTreeRegValueBase::HTreeRegValueBase( void )
  {
    Handle = NULL;
}

HTreeRegValueBase::~HTreeRegValueBase()
  {
    Empty();
}
//-HValue
void HTreeRegValueBase::DoEmpty( void )
  {
    if ( isVariable() ) intDelValue( Handle );
}
DWORD HTreeRegValueBase::DoSizeofData( void ) const
  {
    if ( isVariable() )
      return (Tree && Tree->Pool)?Tree->Pool->GetDataSize(Handle):0;
     else
      return HValue::DoSizeofData();
}
LPVOID HTreeRegValueBase::DoPtrData( void ) const
  {
    if ( isVariable() )
      return (LPVOID)GetHandle();
     else
      return HValue::DoPtrData();
}
void HTreeRegValueBase::DoTypeChange( HVType oldT,HVType newT,LPVOID val )
  {
    if ( isVariable(oldT) && isVariable(oldT) != isVariable(newT) && Handle ) {
      intDelValue(Handle);
      Handle = NULL;
    }
    HValue::DoTypeChange( oldT,newT,val );
}
//-Internal
HANDLE HTreeRegValueBase::intAddValue( LPVOID Data,DWORD Size )
  {
 return (Tree && Tree->Pool)?Tree->Pool->CreateData(Data,Size):NULL;
}
BOOL HTreeRegValueBase::intDelValue( HANDLE Value )
  {
  return (Tree && Tree->Pool)?Tree->Pool->DeleteData(Value):FALSE;
}
BOOL HTreeRegValueBase::intGetData( LPVOID Buff,DWORD sz )
  {
    if ( !isVariable() || !Buff || !sz ) return FALSE;

    LPVOID d = (Tree && Tree->Pool)?Tree->Pool->GetData(Handle):NULL;
    if ( !d ) return FALSE;
    MemMove( Buff,d,Min(sz,DataSize()) );
 return TRUE;
}
BOOL HTreeRegValueBase::intSetData( const LPVOID Buff,DWORD sz )
  {
    if ( !Tree || !Tree->Pool || !isVariable() || !Buff || !sz )
      return FALSE;

    if ( !Handle ) {
      Handle = intAddValue( Buff,sz );
      if ( !Handle )
        return FALSE;
    }

    if ( !Tree->Pool->SetData(Handle,Buff,sz) )
      return FALSE;

    if ( Type == HVHandle )
      SetHandle( (HANDLE)Tree->Pool->GetData(Handle) );
     else
      SetString( (CONSTSTR)Tree->Pool->GetData(Handle) );

 return TRUE;
}

PHTreeValue HTreeRegValueBase::intAdd( CONSTSTR PathName )
  {
  return DoAdd( PathName );
}

PHTreeValue HTreeRegValueBase::intAdd( CONSTSTR PathName,DWORD val )
  {  PHTreeRegValueBase p = (PHTreeRegValueBase)DoAdd( PathName );
     if ( !p )
       return NULL;
     p->SetDword( val );
 return p;
}

PHTreeValue HTreeRegValueBase::intAdd( CONSTSTR PathName,LPVOID Data,DWORD dsz )
  {  PHTreeRegValueBase p;

     if ( !Tree || !Tree->Pool ) return NULL;

     if ( (p=(PHTreeRegValueBase)DoAdd(PathName)) == NULL ) return NULL;

     p->SetHandle( NULL );
     if ( !p->intSetData( Data,dsz ) ) {
       delete p;
       return NULL;
     } else
       return p;
}

PHTreeValue HTreeRegValueBase::intAdd( CONSTSTR PathName,CONSTSTR Data )
  {  PHTreeRegValueBase p;
    if ( !Tree || !Tree->Pool ) return NULL;
    if ( (p=(PHTreeRegValueBase)DoAdd(PathName)) == NULL ) return NULL;
    p->SetString( NULL );
    if ( !p->intSetData( (LPVOID)(Data?Data:""),(Data?(int)strlen(Data):0)+1 ) ) {
      delete p;
      return NULL;
    } else
      return p;
}
//-Interface
BOOL HTreeRegValueBase::DoSaveItem( int File )
  {
    if ( FIO_WRITE(File,&Type,sizeof(Type)) != sizeof(Type) )
      return FALSE;

    if ( !isSetted() ) return TRUE;

    if ( isSimple() )
      return FIO_WRITE(File,Ptr(),Sizeof()) == (int)Sizeof();

    if ( isVariable() )
      return Tree->Pool->SaveItem(Handle,File);

 return FALSE;  /*unknown type to save*/
}
BOOL HTreeRegValueBase::DoLoadItem( int File )
  {
    if ( !Tree || !Tree->Pool ||
          FIO_READ(File,&Type,sizeof(Type)) != sizeof(Type) )
      return FALSE;

    if ( !isSetted() ) return TRUE;

    if ( isSimple() )
      return FIO_READ(File,Ptr(),Sizeof()) == (int)Sizeof();

    if ( isVariable() ) {
      Handle = Tree->Pool->LoadItem(File);

      if ( Handle == NULL )
        return FALSE;

      if ( Type == HVHandle )
        SetHandle( (HANDLE)Tree->Pool->GetData(Handle) );
       else
        SetString( (CONSTSTR)Tree->Pool->GetData(Handle) );
      return TRUE;
    }
 return FALSE;  /*unknown type to load*/
}
BOOL HTreeRegValueBase::DoLoadTree( int File )
  {
    if ( !Tree || !Tree->Pool ) return FALSE;
 return HTreeValue::DoLoadTree(File);
}
BOOL HTreeRegValueBase::DoSaveTree( int File )
  {
    if ( !Tree || !Tree->Pool ) return FALSE;
 return HTreeValue::DoSaveTree(File);
}
