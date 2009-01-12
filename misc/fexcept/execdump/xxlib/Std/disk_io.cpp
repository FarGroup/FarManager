#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif

/***************************************
            Procedures
 ***************************************/
BOOL MYRTLEXP BasePath( const MyString& base, const MyString& path )
  {
  return path.Length() >= base.Length() &&
         CMPN_FILE( path.Text(),base.Text(),base.Length() );
}

BOOL MYRTLEXP PossibleFChar( char ch )
  {

 return !( isSpace(ch) || ch == '<'  || ch == '>' ||
           ch == '\''  || ch == '\"' ||
           ch == '?'   || ch == '*');
}

BOOL MYRTLEXP CheckExist( CONSTSTR path, int mode )
  {
 return access(path,mode) == 0;
}

#if !defined(__HWIN32__)
BOOL MYRTLEXP DeleteFile( CONSTSTR path )
  {
    if ( unlink(path) == 0 )
      return TRUE;
#if defined(__HDOS__) || defined(__HWIN16__)
 return (chmod( path, S_IREAD|S_IWRITE) == 0 &&
         unlink(path) == 0);
#else
#if defined(__QNX__)
 return FALSE;
#else
#if defined(__GNUC__)
 return FALSE;
#else
  #error ERR_PLATFORM
#endif
#endif
#endif
}
#endif

BOOL MYRTLEXP DeleteDir( CONSTSTR path )
  {
 return rmdir(path) == 0;
}

BOOL MYRTLEXP RenameFile( CONSTSTR old, CONSTSTR newNm )
  {
 return rename(old,newNm) == 0;
}

BOOL MYRTLEXP SetCurDir( CONSTSTR str )
  {
     if ( !str || !str[0] ) return TRUE;

#if defined(__REALDOS__) || defined(__PROTDOS__)
  if ( str[1] == ':' && setdisk(ToUpper(str[0])-'A') <= 0 )
      return FALSE;
#else
#if defined(__GNUC__) || defined(__QNX__) || defined(__HWIN__)
  ;//
#else
#error ERR_PLATFORM
#endif
#endif
return FIO_CHDIR(str) == 0;
}

CONSTSTR MYRTLEXP GetCurDir( void )
  {  static char path[ MAX_PATH_SIZE+1 ];

#if defined(__GNUC__) || defined(__QNX__)
    getcwd( path,MAX_PATH_SIZE );
#else
#if defined(__HDOS__) || defined(__HWIN16__)
   strcpy(path, "X:\\");      /* fill string with form of response: X:\ */
   path[0] = 'A' + getdisk();    /* replace X with current drive letter */
   getcurdir(0, path+3);  /* fill rest of string with current directory */
#else
#if defined(__HWIN__)
   GetCurrentDirectory( sizeof(path),path );
#else
#error ERR_PLATFORM
#endif
#endif
#endif
 return AddLastSlash( path );
}

BOOL MYRTLEXP MakeDir( CONSTSTR path )
  {
#if defined(__GNUC__) || defined(__QNX__)
 return mkdir( path,0744 ) == 0;
#else
#if defined(__HDOS__) || defined(__HWIN16__)
 return mkdir( path ) == 0;
#else
#if defined(__HWIN__)
 return CreateDirectory( path,NULL );
#else
 #error ERR_PLATFORM
#endif
#endif
#endif
}

DWORD MYRTLEXP FileLength( CONSTSTR fname )
  {  struct stat st;

     if ( !FIO_STAT( fname,&st ) ) return MAX_DWORD;

#if defined(__QNX__)
     int    file;
     DWORD  sz;

     if ( S_ISLNK(st.st_mode) ) {
       int file = FIO_OPEN( fname,O_RDONLY );
       if ( file == -1 ) return MAX_DWORD;
       sz = filelength( file );
       FIO_CLOSE(file);
       return sz;
     }
#endif

 return st.st_size;
}

BOOL MYRTLEXP MakeLink( char *src, char *dest )
  {
#if defined(__GNUC__) || defined(__QNX__)
  return symlink( src,dest ) == 0;
#else
#if defined(__HDOS__) || defined(__HWIN__)
  src; dest;
  return FALSE;
#else
#error ERR_PLATFORM
#endif
#endif
}

BOOL MYRTLEXP UnLink( char *lnk )
  {
#if defined(__GNUC__) || defined(__QNX__)
  return unlink( lnk ) == 0;
#else
#if defined(__HDOS__) || defined(__HWIN__)
  lnk;
  return FALSE;
#else
#error ERR_PLATFORM
#endif
#endif
}

BOOL MYRTLEXP GetFileTimes( CONSTSTR fname,time_t *cr,time_t *wr,time_t *ac )
  {  struct stat s;

     if ( stat( fname,&s ) != 0 )
       return FALSE;

     if (cr) *cr = s.st_ctime;
     if (wr) *wr = s.st_mtime;
     if (ac) *ac = s.st_atime;

 return TRUE;
}

BOOL MYRTLEXP SetFileTimes( CONSTSTR fname,time_t cr,time_t wr,time_t ac )
  {
    if ( !fname || (!cr && !wr && !ac) ) {
      FIO_SETERRORN( ERROR_INVALID_PARAMETER );
      return FALSE;
    }

//Creation time
    cr = cr;      //?

//Access & modify time
    struct utimbuf tmp;
    tmp.actime  = ac ? ac : ( cr ? cr : wr );
    tmp.modtime = wr ? wr : ( cr ? cr : ac );

#if defined(__BC31__)
 return utime( (char*)fname,&tmp ) == 0;
#else
 return utime( fname,&tmp ) == 0;
#endif
}
