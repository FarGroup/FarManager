#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif

/** @brief Ge startup dir for specified module

    Function returns path from where module start.
    In different platform uses different technics to find startup directory.
      - Windows Uses GetModuleFileName(NULL) function;
      - DOS Uses argv[0] path;
      - QNX Uses argv[0] and HOME enviropment variable.

    @note Function returns static local buffer with size up to
          MAX_PATH_SIZE characters.
    @note Returns path with trailing slash.
*/
CONSTSTR MYRTLEXP GetStartupDir( HANDLE h )
  {  static char  path[ MAX_PATH_SIZE ];
            char *m;

#if defined(__GNUC__) || defined(__QNX__)
   m = CTArgGet(0);
   //argv[0] has path
   if ( m && m[0] && StrChr(m,SLASH_CHAR) ) {
     StrCpy( path,m,sizeof(path) );
     StrRChr( path,SLASH_CHAR )[0] = 0;
   } else
/*
   //HOME variable
   if ( (m=getenv("HOME")) != NULL )
     StrCpy( path,m,sizeof(path) );
    else
*/
     path[0] = 0;
#else
#if defined(__HDOS__)
   USEARG( h )
   StrCpy( path,CTArgGet(0),sizeof(path) );
   if ( (m=StrRChr(path,SLASH_CHAR)) != 0 )
     *m = 0;
    else
     m[0] = 0;
#else
#if defined(__HWIN__)
   path[ GetModuleFileName( (HMODULE)h,path,sizeof(path) ) ] = 0;
   if ( (m=StrRChr(path,SLASH_CHAR)) != 0 ) *m = 0; else m[0] = 0;
#else
#error ERR_PLATFORM
#endif
#endif
#endif

 return AddLastSlash(path);
}

/** @brief Locate path for temprorary directory.

    Function looking for values in next enviropment
    variables: "TEMP", "TMP", "TMPDIR", "HOME".

    @note Function returns static local buffer with size up to
          MAX_PATH_SIZE characters.
    @note Returns path with trailing slash.
*/
CONSTSTR MYRTLEXP GetTmpDir( void )
  {  char *m;
     static char path[ MAX_PATH_SIZE ];

   do{
     m = getenv( "TEMP" );
     if ( m ) break;
     m = getenv( "TMP" );
     if ( m ) break;
     m = getenv( "TMPDIR" );
     if ( m ) break;
     m = getenv( "HOME" );
     if ( m ) break;
     return GetCurDir();
   }while(0);
   StrCpy( path,m,sizeof(path) );

 return AddLastSlash(path);
}

/** @brief Create full file path name
    @param FileName
    @param basePath Optional pointer to base directory.
    @return Full pathname for given name or empty string on error.

    Returns string as follows:
      - name itself if name starting from absolute path;
      - name added to basePath if basePath specified;
      - name added to startup directory;

    Errors:
      - ERROR_INVALID_PARAMETER FileName does not specified.

    @note Function returns static local buffer with maximum size of
          MAX_PATH_SIZE characters.
*/
CONSTSTR MYRTLEXP LocalFile( CONSTSTR FileName,CONSTSTR basePath )
  {  static char str[ MAX_PATH_SIZE ];

    if ( !FileName ) {
      FIO_SETERRORN( ERROR_INVALID_PARAMETER );
      return "";
    }
    if ( IsAbsolutePath(FileName) )
      StrCpy( str,FileName,sizeof(str) );
     else {
      while( *FileName && *FileName == SLASH_CHAR ) FileName++;
      if ( !basePath )
        StrCpy( str, GetStartupDir(NULL), sizeof(str) );
       else
        StrCpy( str,basePath,sizeof(str) );

      AddLastSlash(str);

      StrCat( str,FileName,sizeof(str) );
    }

 return str;
}

/** Returns type of stdio file
*/
FileType MYRTLEXP QueryFileType( int f )
  {
#if defined(__GNUC__)
    if ( isatty(f) )
      return ftPipe;

    struct stat st;
    if ( fstat(f,&st) != 0 )
      return ftUnknown;

    if ( S_ISFIFO(st.st_mode) || S_ISWHT(st.st_mode) || S_ISSOCK(st.st_mode) )
      return ftPipe;
     else
      return ftDisk;
#else
#if defined(__QNX__)
    if ( isatty(f) )
      return ftPipe;

    _fsys_stat st;
    if ( fsys_fstat(f,&st) != 0 )
      return ftUnknown;

    if ( S_ISNAM(st.st_mode) || S_ISSOCK(st.st_mode) )
      return ftPipe;
     else
      return ftDisk;
#else
#if defined(__HWIN32__)
   HANDLE h = (HANDLE)_get_osfhandle(f);
   if ( !h || h == INVALID_HANDLE_VALUE )
     return ftUnknown;

   switch( GetFileType(h) ) {
     case    FILE_TYPE_DISK: return ftDisk;
     case    FILE_TYPE_CHAR: return ftDevice;
     case    FILE_TYPE_PIPE: return ftPipe;
     case FILE_TYPE_UNKNOWN:
                    default: return ftUnknown;
   }
#else
#if defined(__REALDOS__) || defined(__PROTDOS__)
  USEARG( f )
  return ftDisk;
#else
#if defined(__HWIN16__)
  USEARG( f )
  return ftDisk;
#else
#error ERR_PLATFORM
#endif //GNUC
#endif //QNX
#endif //HWIN32
#endif //HDOS
#endif //Win16
}

FileType MYRTLEXP QueryFileType( FILE *f )
  {
 return QueryFileType( fileno(f) );
}
