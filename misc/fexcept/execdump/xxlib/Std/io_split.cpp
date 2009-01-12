#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif

MyString MYRTLEXP MakePathName( CONSTSTR path,CONSTSTR fname,char Slash )
  {  MyString str( path );

     AddLastSlash( str,Slash );
     if ( fname ) {
       while( *fname == Slash ) fname++;
       str.Add( fname );
     }

 return str;
}

pchar MYRTLEXP MakePathName( char *path,CONSTSTR fname,char Slash )
  {
     AddLastSlash(path,Slash);
     if ( fname ) {
       while( *fname == Slash ) fname++;
       strcat( path, fname );
     }

 return path;
}

MyString MYRTLEXP GetLastDirName( const MyString& path, char Slash )
  {  MyString s;
     int      len = path.Length(),
              num;

     if ( !len )
       return s;
     len--;

     if ( path[len] == Slash ) len--;

     num = path.RChr( len );

     if ( num == -1 )
       s.Set( path.c_str(), 0, len );
      else
       s.Set( path.c_str(), num+1,len-num+1 );

 return s;
}

BOOL MYRTLEXP RootDir( const MyString& path )
  {
#if defined(__QNX__) || defined(__GNUC__)
    if ( path.Length() == 1 )
      return TRUE;

    if ( path[0] == SLASH_CHAR && path[1] == SLASH_CHAR ) {
        char *m = StrChr( path.Text()+2,SLASH_CHAR );
        if ( !m || strLen(m) <=1 ) return TRUE;
    }
  return FALSE;
#else
#if defined(__HDOS__) || defined(__HWIN__)
  return path[1] == ':' && path.Length() == 3;
#else
#error ERR_PLATFORM
#endif
#endif
}

MyString MYRTLEXP MakeRootDir( CONSTSTR path )
  {  char str[ MAX_PATH_SIZE+1 ];

    if ( RootDir(MyString(path)) )
      return path;

#if defined(__QNX__) || defined(__GNUC__)
    char *m;
    strcpy( str,path );
    if ( str[0] == SLASH_CHAR && str[1] == SLASH_CHAR )
      {  m = StrChr( str+2,SLASH_CHAR );
         if ( m ) { m[1] = 0; return str; }
      }
    return SLASH_STR;
#else
#if defined(__HDOS__) || defined(__HWIN__)
    return MyString(StrCpy( str,path,3));
#else
#error ERR_PLATFORM
#endif
#endif
}
