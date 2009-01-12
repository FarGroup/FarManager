#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif

BOOL MYRTLEXP IsAbsolutePath( CONSTSTR path )
  {
    Assert( path && "!IsAbsolutePath.path" );
#if defined(__HDOS__) || defined(__HWIN__)
  #if defined(__HWIN__)
    if ( strncmp( path, "\\\\?\\",4 ) == 0 )
      path += 4;

    if ( path[0] == '\\' && path[1] == '\\' )
      return TRUE;

  #endif
  return path[0] &&
         path[1] == ':' && (path[2] == '\\'
  #if defined(__HWIN__)
         || path[2] == '/'
  #endif
         );
#else
#if defined(__QNX__) || defined(__GNUC__)
  return path[0] == '/';
#else
 #error ERR_PLATFORM
#endif
#endif
}

pchar MYRTLEXP MakeAbsolutePath( char *path )
  {  int  nmOff;
     char str[ MAX_PATH_SIZE ];

    if ( IsAbsolutePath(path) )
      return path;

#if defined(__HDOS__) || defined(__HWIN__)
    if ( path[0] && path[1] == ':' )
      nmOff = 2;
     else
      nmOff = 0;
  #if defined(__HDOS__)
     str[0] = ToUpper(path[0]);
     str[1] = ':';
     str[2] = '\\';
     getcurdir( str[0]-'A'+1, str+3);  /* fill rest of string with current directory */
  #else
  #if defined(__HWIN16__)
     str[0] = ToUpper(path[0]);
     str[1] = ':';
     str[2] = '\\';
     getcurdir( str[0]-'A'+1, str+3);  /* fill rest of string with current directory */
  #else
  #if defined(__HWIN32__)
     char oldd[ MAX_PATH_SIZE ];
     BOOL ch;

     if ( nmOff == 2 ) {
       GetCurrentDirectory( sizeof(oldd),oldd );
       str[0] = (char)toupper(path[0]);
       str[1] = ':';
       str[2] = 0;
       ch = SetCurrentDirectory( str );
       GetCurrentDirectory( sizeof(str),str );
       if ( ch ) SetCurrentDirectory( oldd );
     } else
       GetCurrentDirectory( sizeof(str),str );
  #endif
  #endif
  #endif
#else
#if defined(__QNX__) || defined(__GNUC__)
    nmOff = 0;
    getcwd( str,sizeof(str)-1 );
#else
 #error ERR_PLATFORM
#endif
#endif
   if ( path[nmOff] != SLASH_CHAR )
     AddLastSlash( str );

   StrCat( str,path+nmOff,sizeof(str) );
   StrCpy( path,str );
 return path;
}

BOOL MYRTLEXP RelativeConvertable( CONSTSTR base, CONSTSTR path )
  {
    if ( !IsAbsolutePath(base) ) return FALSE;
    if ( !IsAbsolutePath(path) ) return TRUE;
#if defined(__HDOS__) || defined(__HWIN__)
 return CMPN_FILE( base,path,3 );
#else
#if defined(__QNX__) || defined(__GNUC__)
    if ( base[0] == SLASH_CHAR && base[1] == SLASH_CHAR ) {
      for ( int n = 0; base[n] && path[n] && base[n] != SLASH_CHAR; n++ )
        if ( base[n] != path[n] )
          return FALSE;
    }
  return TRUE;
#else
 #error ERR_PLATFORM
#endif
#endif
}

MyString MYRTLEXP MakeStartRelativePath( CONSTSTR path )
  {
 return MakeRelativePath( GetStartupDir(), path );
}

MyString MYRTLEXP MakeStartLocalPath( CONSTSTR fnm )
  {
    if ( IsAbsolutePath(fnm) )
      return fnm;
     else
      return MakePathName( GetStartupDir(), fnm );
}

MyString MYRTLEXP MakeRelativePath( CONSTSTR base,CONSTSTR path )
  {  MyString back,top,
              s,s1;
     int      n,i;
     char     ch,ch1;

    if ( !base || !path )
      return "";

    if ( !IsAbsolutePath(path) ||
         !RelativeConvertable(base,path) )
      return path;

//Skip head of absolute path
#if defined(__HDOS__) || defined(__HWIN__)
    n = i = 3;
#else
#if defined(__QNX__) || defined(__GNUC__)
    if ( base[0] == SLASH_CHAR && base[1] == SLASH_CHAR ) {
      for ( n = 2; base[n] != SLASH_CHAR; n++ );
      n++;
    } else
       n = 1;
    if ( path[0] == SLASH_CHAR && path[1] == SLASH_CHAR ) {
      for ( i = 2; path[i] != SLASH_CHAR; i++ );
      i++;
    } else
      i = 1;
#else
 #error ERR_PLATFORM
#endif
#endif

#define GETNAME( dest,counter,path,last )          \
  dest = "";                                       \
  for( ; (last=path[counter]) != 0; counter++ ) {  \
    if ( last == SLASH_CHAR) {                     \
      last = path[++counter];                      \
      break;                                       \
    }                                              \
    dest.Add(last);                                \
  }

    if ( CMP_FILE( base+n, path+i ) )
      return "." SLASH_STR;

    do{
      GETNAME( s,  n, base, ch )  //Skips but not include slash
      GETNAME( s1, i, path, ch1 )
      if ( !CMP_FILE( s.Text(),s1.Text() ) )
        break;

      if ( !ch && !ch1 )
        return "." SLASH_STR;

    }while( 1 );

    do{
      if ( !s.Length() )
        break;

      back.Add( ".." SLASH_STR );
      GETNAME( s,  n, base, ch )
    }while(1);

    if ( s1.Length() ) {
      back.Add( s1 );
      if ( ch1 ) {
        back.Add( SLASH_CHAR );
        back.Add( path+i );
      }
    }


    DelLastSlash( back );

 return back;
}
