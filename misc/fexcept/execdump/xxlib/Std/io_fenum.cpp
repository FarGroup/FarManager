#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif


/***************************************
            FileEnum
 ***************************************/
BOOL MYRTLEXP FILE_ENUM::OpenDir( CONSTSTR path )
  {
    if ( !path ) path = "";

    oldDir = GetCurDir();
    if ( !SetCurDir(path) ) {
      SetCurDir( oldDir.Text() );
      return FALSE;
    }

    Path = GetCurDir();
    SetCurDir( oldDir.Text() );

  #if defined(__GNUC__) || defined(__QNX__)
    dir = opendir( Path.Text() );
    if ( dir == NULL || (ent=readdir(dir)) == NULL ) return FALSE;
    FPath = Path;
    FPath.Add( ent->d_name );
  #else
  #if defined(__REALDOS__) || defined(__HWIN16__)
    MemSet( &f,0,sizeof(f) );
    if ( findfirst( MakePathName(path,ALL_FILES).Text(),&f,FIO_ALLFILES ) != 0 )
      return FALSE;
    FPath = Path+f.ff_name;
  #else
  #if defined(__PROTDOS__)
    MemSet( &f,0,sizeof(f) );
    FIND *res;
    if ( (res=findfirst( MakePathName(path,ALL_FILES).Text(),FIO_ALLFILES )) == NULL )
      return FALSE;
    f = *res;
    FPath = Path+f.name;
  #else
  #if defined(__HWIN__)
    fHandle = FindFirstFile( MakePathName(path,ALL_FILES).Text(),&f );
    if ( fHandle == INVALID_HANDLE_VALUE ) return FALSE;
    FPath = Path;
    FPath.Add( f.cFileName );
  #else
  #error ERR_PLATFORM
  #endif
  #endif
  #endif
  #endif
 return TRUE;
}

char *MYRTLEXP FILE_ENUM::FName( void )
    {
  #if defined(__GNUC__) || defined(__QNX__)
    return ent->d_name;
  #else
  #if defined(__REALDOS__) || defined(__HWIN16__)
    return f.ff_name;
  #else
  #if defined(__PROTDOS__)
    return f.name;
  #else
  #if defined(__HWIN__)
    return f.cFileName;
  #else
  #error ERR_PLATFORM
  #endif
  #endif
  #endif
  #endif
}

char *MYRTLEXP FILE_ENUM::PathName( void )
    {
    return FPath.Text();
}

BOOL MYRTLEXP FILE_ENUM::NextFile( void )
    {
  #if defined(__GNUC__) || defined(__QNX__)
    if ( (ent=readdir(dir)) == NULL ) return FALSE;
  #else
  #if defined(__REALDOS__) || defined(__HWIN16__)
    if ( findnext(&f) != 0 ) return FALSE;
  #else
  #if defined(__PROTDOS__)
    FIND *res = findnext();
    if ( !res ) return FALSE;
    f = *res;
  #else
  #if defined(__HWIN__)
    if ( !FindNextFile(fHandle,&f) ) return FALSE;
  #else
  #error ERR_PLATFORM
  #endif
  #endif
  #endif
  #endif

  FPath = Path;
  FPath.Add( FName() );
 return TRUE;
}

void MYRTLEXP FILE_ENUM::CloseDir( void )
    {
  #if defined(__GNUC__) || defined(__QNX__)
    closedir( dir );
  #else
  #if defined(__HDOS__) || defined(__HWIN16__)
    ;
  #else
  #if defined(__HWIN__)
    FindClose( fHandle );
  #else
  #error ERR_PLATFORM
  #endif
  #endif
  #endif
}
