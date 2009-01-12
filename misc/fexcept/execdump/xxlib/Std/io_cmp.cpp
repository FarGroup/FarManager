#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif


BOOL MYRTLEXP PathCmp( const MyString& path,const MyString& path1 )
  {
 return CMP_FILE( path.Text(),path1.Text() );
}

MyString MYRTLEXP MakeFullPathName( const MyString& fname, const MyString& base )
  {  MyString oldP,s;

    if ( fname.Chr(SLASH_CHAR) == -1 ) {
      s.Set( base );
      s.Add( SLASH_CHAR );
      s.Add( fname );
      return s;
    }

    oldP = GetCurDir();
     //Go base path
     SetCurDir( base.Text() );
     //Go fname path in case it relative
     SetCurDir( FPath(fname.Text()) );
     //Get result path + name
     s.Set( GetCurDir() );
     s.Add( FName(fname.Text()) );
    SetCurDir( oldP.Text() );
 return s;
}

BOOL MYRTLEXP IsSameFile( const MyString& f1,const MyString& f2 )
  {  MyString s,s1;

//Both path exist
     if ( f1.Chr(SLASH_CHAR) != -1 && f2.Chr(SLASH_CHAR) != -1 &&
          !CMP_FILE(GetFPath(f1).Text(),GetFPath(f2).Text()) )
       return FALSE;

 return CMP_FILE( GetFName(f1).Text(),GetFName(f2).Text() );
}

#if defined(__QNX__)
BOOL MYRTLEXP QnxCmpFile( char *f1, char *f2, int len )
  {  BOOL  b1,b2;
     static struct _osinfo oi;
     static char   name[16] = "";
     static int    nmlen;
//CCurrent machine name
    if ( !name[0] ) {
      qnx_osinfo( 0, &oi );
      Sprintf( name,"//%ld",oi.nodename );
      nmlen = strLen( name );
    }
//Cmp not paths
    if ( !f1 || !f2 || f1[0] != '/' || f2[0] != '/' )
      if ( len < 0 )
        return StrCmp(f1,f2) == 0;
       else
        return StrNCmp(f1,f2,len) == 0;
//Paths cmp
    b1 = (f1[0] == '/' && f1[1] == '/');
    b2 = (f2[0] == '/' && f2[1] == '/');

    if ( b1 != b2 ) {
      if ( !b1 ) {
        if ( StrNCmp(name,f2,nmlen) != 0 ) return FALSE;
        if ( len >= 0 && strLen(f2) == len ) len -= nmlen;
        f2 += nmlen;
      }
      if ( !b2 ) {
        if ( StrNCmp(name,f1,nmlen) != 0 ) return FALSE;
        if ( len >= 0 && strLen(f1) == len ) len -= nmlen;
        f1 += nmlen;
      }
    }
    if ( len < 0 )
      return StrCmp(f1,f2) == 0;
     else
      return StrNCmp(f1,f2,len) == 0;
}
#endif
