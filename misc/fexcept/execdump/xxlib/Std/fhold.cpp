#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif

/***************************************
               FileHolder
 ***************************************/
CTFileHolder::CTFileHolder( void )
  {
    file = -1;
    Data = NULL;
    winSize = Pos = Size = 0;
}

CTFileHolder::CTFileHolder( CONSTSTR fname,DWORD wSize )
  {
    file = -1;
    Data = NULL;
    winSize = Pos = Size = 0;
    SetupMinimumSize( wSize );
    Assign( fname,0 );
}

CTFileHolder::~CTFileHolder()
  {
    if ( file != -1 ) {
      FIO_CLOSE(file);
      file = -1;
    }
    _Del( Data );
}

DWORD    CTFileHolder::GetSize( void ) { return Size; }
void     CTFileHolder::Reload( void )  { ReloadData(); }
CONSTSTR CTFileHolder::GetName( void ) { return File.Text(); }
void     CTFileHolder::Close( void )   { if (file != -1) { FIO_CLOSE(file); file = -1; } }
int      CTFileHolder::FHandle( void ) { return file; }

void CTFileHolder::SetupMinimumSize( DWORD sz )
  {
    _Del( Data );
    Data = (PBYTE)_Alloc( winSize = sz );
    MemSet( Data,0,sizeof(BYTE)*winSize );
    ReloadData();
}

BOOL CTFileHolder::Assign( CONSTSTR data,DWORD sz )
  {
    File = data;
    Size = FileLength( data );
    Pos  = 0;
    if (sz) SetupMinimumSize(sz);
    OK = ReloadData();
 return OK;
}

BYTE CTFileHolder::Chr( DWORD pos )
  {
    if ( !Data || pos >= Size )  return 0;
    if ( pos < Pos )             { Pos = (pos>winSize)?(pos-winSize+1):0; ReloadData(); }
    if ( pos >= Pos+readedSize ) { Pos = pos; ReloadData(); }
 return Data[ pos-Pos ];
}

PBYTE CTFileHolder::GetData( DWORD pos, DWORD size )
  {
    if ( !Data || pos >= Size )  return NULL;

    if ( size >= winSize )           SetupMinimumSize( size );
    if ( pos < Pos )                 { Pos = (pos>winSize)?(pos-winSize+1):0; ReloadData(); }
    if ( pos >= Pos+readedSize )     { Pos = pos; ReloadData(); }
    if ( pos+size > Pos+readedSize ) { Pos = pos; ReloadData(); }

 return Data+(pos-Pos);
}

BOOL CTFileHolder::StoreData( DWORD off, PBYTE data,DWORD size )
  {  int file;
     BOOL res = FALSE;

    if ( File.Length() == 0 ) return FALSE;
    if ( !Data ) SetupMinimumSize( CT_DATA_WINDOWSIZE );

//Write data
#if defined(MSDOS) || defined(__MSOFT) || defined(__INTEL)
    _fmode = O_BINARY;
#endif
    file = FIO_OPEN( File.Text(),O_RDWR );
    if ( file == -1 ) return FALSE;
    do{
      if ( ((DWORD)FIO_SEEK( file,off,seekBEGIN )) != off ) break;
      if ( ((DWORD)FIO_WRITE( file,data,size )) != size ) break;
      res = TRUE;
    }while(0);
    FIO_CLOSE( file );
    if ( res && off > Pos && off < Pos+readedSize ) ReloadData();
 return res;
}

BOOL CTFileHolder::ReloadData( void )
  {  BOOL res = FALSE;

    if ( File.Length() == 0 ) return FALSE;
    if ( !Data ) SetupMinimumSize( CT_DATA_WINDOWSIZE );

//Setup Pos
    if ( Pos+winSize > Size ) Pos = (Size>winSize)?(Size-winSize):0;

//Read data  _creat
#if defined(MSDOS) || defined(__MSOFT) || defined(__INTEL)
    _fmode = O_BINARY;
#endif

    if ( file == -1 ) {
      file = FIO_OPEN( File.Text(),O_RDONLY );
      if ( file == -1 ) return FALSE;
    }
    do{
      if ( ((DWORD)FIO_SEEK( file,Pos,seekBEGIN )) != Pos ) break;
      readedSize = Min(winSize,Size-Pos);
      if ( ((DWORD)FIO_READ(file,Data,readedSize)) != readedSize ) break;
      res = TRUE;
    }while(0);

 return res;
}
/***************************************
               CTDumpHolder
 ***************************************/
CTDumpHolder::CTDumpHolder( LPCBYTE data,DWORD sz ) { Assign((const char*)data,sz); }
CTDumpHolder::CTDumpHolder( CONSTSTR data )             { Assign(data,strLen(data)); }

BOOL     CTDumpHolder::Assign( CONSTSTR data,DWORD sz )    { Size = sz; Data = (PBYTE)data; return TRUE; }
BYTE     CTDumpHolder::Chr( DWORD off )                    { return (BYTE)((off >= Size)?0:Data[off]); }
PBYTE    CTDumpHolder::GetData( DWORD off, DWORD size )    { return (off+size > Size)?NULL:(Data+off); }
DWORD    CTDumpHolder::GetSize( void )                     { return Size; }
CONSTSTR CTDumpHolder::GetName( void )                     { Sprintf( Name,"?%lp",Data ); return Name; }
void     CTDumpHolder::SetupMinimumSize( DWORD )           { }
void     CTDumpHolder::Reload( void )                      { }

BOOL CTDumpHolder::StoreData( DWORD off, PBYTE data,DWORD size )
  {
    if ( off > Size || off+size > Size ) return FALSE;
    MemMove( Data+off,data,size );
 return TRUE;
}
