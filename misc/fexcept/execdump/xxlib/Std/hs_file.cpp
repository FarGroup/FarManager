#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif

#if 1
  #define  PROC( v )
  #define  Log( v )
#else
  #define  PROC( v )  INProc __proc v ;
  #define  Log( v )   INProc::Say v
#endif

/****************************************************
   HFileIStream
 ****************************************************/
HFileIStream::HFileIStream( DWORD bsz )
    : HIStream(bsz)
  {
    File = -1;
}

HFileIStream::HFileIStream( CONSTSTR fnm,DWORD bsz )
    : HIStream(bsz)
  {
    File = -1;
    Assign(fnm);
}

HFileIStream::HFileIStream( int fnm,DWORD bsz )
    : HIStream(bsz)
  {
    File = -1;
    Assign(fnm);
}

HFileIStream::~HFileIStream()
  {
    Close();
}

void HFileIStream::Close( void )
  {
    if ( File != -1 )
      FIO_CLOSE( File );
    Reset();
}

BOOL HFileIStream::Assign( CONSTSTR FileName )
  {
    Close();

    File = FIO_OPEN( FileName,O_RDONLY|O_BINARY );
 return File != -1;
}

BOOL HFileIStream::Assign( int nFile )
  {
    Close();
    File = nFile;

 return TRUE;
}

void HFileIStream::doReset( void )    { File = -1; }
BOOL HFileIStream::doAssigned( void ) { return File != -1; }

BOOL HFileIStream::ReadCompleted( void )
  {
    if ( File == -1 )
      return FALSE;

    if ( !rSize )
      return TRUE;

    int rc = FIO_READ(File,rBuff,rSize);
    if ( rc <= 0 )
      return FALSE;

    rCount = (DWORD)rc;

 return TRUE;
}

BOOL HFileIStream::FEOF( void )
  {
 return ReadySize() == 0 &&
        (File == -1 || FIO_EOF(File));
}
/****************************************************
   HFileOStream
 ****************************************************/
HFileOStream::HFileOStream( DWORD bsz )
    : HOStream(bsz)
  {
    File = -1;
}

HFileOStream::HFileOStream( CONSTSTR fnm,DWORD bsz )
    : HOStream(bsz)
  {
    File = -1;
    Assign( fnm );
}

HFileOStream::HFileOStream( int fnm,DWORD bsz )
    : HOStream(bsz)
  {
    File = -1;
    Assign( fnm );
}

HFileOStream::~HFileOStream()
  {
    Close();
}

void HFileOStream::Close( void )
  {
    if ( File != -1 ) {
      Flush();
      FIO_TRUNC( File,FIO_TELL(File) );
      FIO_CLOSE( File );
    }
    Reset();
}

BOOL HFileOStream::Assign( CONSTSTR FileName )
  {
    Close();

    File = FIO_OPEN( FileName,O_WRONLY|O_BINARY );
    if ( File == -1 ) {
      File = FIO_CREAT( FileName,FIO_DEF_ATTR );
      if ( File == -1 )
        return FALSE;
    }

 return TRUE;
}

BOOL HFileOStream::Assign( int nFile )
  {
    Close();

    File = nFile;
 return File != -1;
}

void HFileOStream::doReset( void )    { File = -1; }
BOOL HFileOStream::doAssigned( void ) { return File != -1; }

BOOL HFileOStream::WriteCompleted( void )
  {
  return File != -1 &&
         (!rCount || FIO_WRITE( File,rBuff,rCount) == (int)rCount);
}
