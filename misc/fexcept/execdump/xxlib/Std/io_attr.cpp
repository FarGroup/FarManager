#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif

BOOL MYRTLEXP isReadOnly( CONSTSTR fname )
  {  struct stat stbuf;
 return ( FIO_STAT(fname,&stbuf) && !IS_FLAG(stbuf.st_mode,S_IWRITE) );
}

BOOL MYRTLEXP ClrReadOnly( CONSTSTR fname )
  {
 return (FIO_CHMOD(fname, S_IREAD|S_IWRITE) && FIO_ACCESS(fname,accWRITE) );
}

BOOL MYRTLEXP SetReadOnly( CONSTSTR fname )
  {
 return (FIO_CHMOD(fname, S_IREAD) && FIO_ACCESS(fname,accREAD) );
}

void MYRTLEXP SetCurRights( char *fname,int rwState )
  {  short attr = (short)rwState;
     FIO_CHMOD( fname,attr );
     FIO_CHOWN( fname,FIO_GETUID,FIO_GETGID );
}

BOOL MYRTLEXP GetFileAttr( CONSTSTR fname,ATTR_TYPE& at )
  {
#if defined(__GNUC__) || defined(__QNX__)
   struct stat s;
   if ( !FIO_STAT(fname,&s) ) return FALSE;
   at = s.st_mode;
  return TRUE;
#else
#if defined(__REALDOS__) || defined(__HWIN16__)
    struct ffblk f;
    if ( findfirst( fname,&f,-1 ) != 0 ) return FALSE;
    at = f.ff_attrib;
    return TRUE;
#else
#if defined(__PROTDOS__)
    struct find_t f;
    if ( _dos_findfirst( fname,-1,&f ) != 0 ) return FALSE;
    at = f.attrib;
    return TRUE;
#else
#if defined(__HWIN32__)
    at = GetFileAttributes( fname );
    if ( at == MAX_DWORD ) return FALSE;
    return TRUE;
#else
  #error ERR_PLATFORM
#endif
#endif
#endif
#endif
}

BOOL MYRTLEXP SetFileAttr( CONSTSTR fname,ATTR_TYPE v )
  {
#if defined(__GNUC__) || defined(__QNX__)
    CLR_FLAG( v,flDirectory );
#else
#if defined(__HDOS__)
    CLR_FLAG( v,FA_DIREC | FA_LABEL );
#else
#if defined(__HWIN16__)
    CLR_FLAG( v,FA_DIREC | FA_LABEL );
#else
#if defined(__HWIN32__)
    CLR_FLAG( v,FILE_ATTRIBUTE_DIRECTORY );
#else
  #error ERR_PLATFORM
#endif
#endif
#endif
#endif

#if defined(__GNUC__) || defined(__QNX__)
  return FIO_CHMOD(fname,v);
#else
#if defined(__REALDOS__)
  return _dos_setfileattr(fname,v) == 0;
#else
#if defined(__PROTDOS__)
  return dos_setfileattr(fname,v) == 0;
#else
#if defined(__BCWIN16__)
  return _dos_setfileattr(fname,v) == 0;
#else
#if defined(__HWIN32__)
  return SetFileAttributes(fname,v) != 0;
#else
  #error ERR_PLATFORM
#endif
#endif
#endif
#endif
#endif
}

#if defined(__BCWIN32__)
static CONSTSTR MYRTLEXP WinFName( CONSTSTR filename )
  {  static char s[MAX_PATH_SIZE];

     GetFullPathName( filename, sizeof(s), s, NULL );
     if ( GetShortPathName(s,s,sizeof(s)) )
       GetFullPathName( s,sizeof(s),s,NULL );
 return s;
}

int MYRTLEXP Stub_rtl_open(const char *filename, int oflags)
  {  int rc = _rtl_open( WinFName(filename),oflags );
     if ( rc == -1 )
       return -1;
     setmode( rc,O_BINARY );
 return rc;
}

int MYRTLEXP Stub_rtl_creat(const char *filename, int attrib)
  {  int rc = _rtl_creat( WinFName(filename),attrib );
     if ( rc == -1 )
       return -1;
     setmode( rc,O_BINARY );
 return rc;
}
#endif