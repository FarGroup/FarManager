#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif


pchar MYRTLEXP FixSlashChars( char *str,char SlashChar )
  {
    if ( !str ) return NULL;
    for ( int n = 0; str[n]; n++ )
      if ( str[n] == '\\' || str[n] == '/' )
        str[n] = SlashChar;
 return str;
}

MyString MYRTLEXP ChangeFilePart( const char* fpath,CONSTSTR newPart,cfpTypes chType, char Slash )
  {  MyString p,n,e;

    if ( !fpath ) fpath = "";
    if ( !newPart ) newPart = "";
    p = GetFPath(fpath);
    n = GetFNameOnly(fpath);
    e = GetFExtOnly(fpath);

    if ( p.Length() && p[p.Length()-1] != Slash )
      p.Add( Slash );

    switch( chType ) {
      case    cfpPATH: p = newPart; break;
      case    cfpNAME: n = newPart; break;
      case cfpNAMEEXT: n = newPart; e = ""; break;
      case     cfpEXT: e = newPart; break;
    }

    p.Add( n );
    if ( e.Length() ) p.Add( "." );
    p.Add( e );

 return p;
}

pchar MYRTLEXP ChangeFileExt( char *path,CONSTSTR Ext, char Slash  )
  {  char *m,*m1;
     BOOL  hasExt = Ext != NULL && Ext[0],
           hasPt  = hasExt && Ext[0] == '.';

     Assert( path && "!ChangeFileExt.path" );

     m = strrchr( path,Slash );
     if ( !m ) m = path; else m++;

     m1 = strrchr( m,'.' );
     if ( !m1 )
       m1 = m + strLen(m);

     if ( !hasExt )
       *m1 = 0;
      else {
       if ( !hasPt && m1 > path ) *m1++ = '.';
       strcpy( m1,Ext + (hasPt && m1 == path) );
     }

 return path;
}

pchar MYRTLEXP ChangeFileName( char *path,CONSTSTR Name, char Slash )
  {  char *m;
     BOOL  hasName = Name != NULL && Name[0],
           hasPt   = hasName && Name[0] == Slash;

     Assert( path && "!ChangeFileName.path" );

     m = strrchr( path,Slash );
     if ( !m )
       m = path;

     if ( !hasName )
       *m = 0;
      else {
       if ( !hasPt && m > path )
         *m++ = Slash;

       strcpy( m,Name + (hasPt && m == path) );
     }

 return path;
}

//---------------------------------------------------------------------------
MyString MYRTLEXP AddLastSlash( MyString& path, char Slash )
  {
    if ( path.Length() && path[ path.Length()-1 ] != Slash )
      path.Add( Slash );
 return path;
}

void MYRTLEXP AddLastSlash( PathString& path, char Slash )
  {
    if ( path.Length() && path[ path.Length()-1 ] != Slash )
      path.Add( Slash );
}

pchar MYRTLEXP AddLastSlash( char *path, char Slash )
  { int len;
    if ( (len=strLen(path)) != 0 && path[len-1] != Slash ) {
      path[len]   = Slash;
      path[len+1] = 0;
    }
 return path;
}
//---------------------------------------------------------------------------
MyString MYRTLEXP DelLastSlash( const MyString& _path, char Slash )
  {  MyString path( _path );
     int      len;

    if ( (len=path.Length()) != 0 && path[len-1] == Slash )
      path.SetChar(len-1,0);

 return path;
}

void MYRTLEXP DelLastSlash( PathString& path, char Slash )
  {  int len;

    if ( (len=path.Length()) != 0 && path[len-1] == Slash )
      path.SetLength(len-1);
}

pchar MYRTLEXP DelLastSlash( char *path, char Slash )
  {  int len;

    if ( path && path[0] && path[len=(strLen(path)-1)] == Slash )
      path[len] = 0;

 return path;
}

//---------------------------------------------------------------------------
MyString MYRTLEXP GetFPath( const MyString& nm, char Slash )
  {  char  str[ MAX_PATH_SIZE ];
     char *m;

     StrCpy( str,nm.Text(),sizeof(str) );
     m = StrRChr( str,Slash );
     if (m) *m = 0; else str[0] = 0;

 return MyString( AddLastSlash(str) );
}
CONSTSTR MYRTLEXP FPath( CONSTSTR nm, char Slash )
  {  static char str[ MAX_PATH_SIZE ];
     char     *m;

     StrCpy( str,nm,sizeof(str) );
     m = StrRChr( str,Slash );
     if (m) *m = 0; else str[0] = 0;

 return AddLastSlash(str);
}

//---------------------------------------------------------------------------
MyString MYRTLEXP GetFName( const MyString& nm, char Slash )
  {  char *m = StrRChr( nm.Text(), Slash );

     if (!m)
       return MyString(nm);
      else
       return MyString(m+1);
}

CONSTSTR MYRTLEXP FName( CONSTSTR nm, char Slash )
  {  static char str[ MAX_PATH_SIZE ];
     CONSTSTR m = StrRChr( nm,Slash );

     if (!m) m = nm; else m++;
     StrCpy( str,m,sizeof(str) );
 return str;
}

//---------------------------------------------------------------------------
CONSTSTR MYRTLEXP FNameOnly( CONSTSTR nm, char Slash  )
  {  static char str[ MAX_PATH_SIZE ];

     CONSTSTR m = StrRChr( nm,Slash );

     StrCpy( str, m ? (m+1) : nm, sizeof(str) );

     char *dot = StrRChr(str,'.');
     if (dot) *dot = 0;

 return str;
}

MyString MYRTLEXP GetFNameOnly( const MyString& nm, char Slash  )
  {  char  str[ MAX_PATH_SIZE ];
     char *m = StrRChr( nm.Text(),Slash );

     if (!m) m = nm.Text(); else m++;
     StrCpy( str,m,sizeof(str) );

     m = StrRChr(str,'.');
     if (m) *m = 0;

 return MyString(str);
}

//---------------------------------------------------------------------------
MyString MYRTLEXP GetFExtOnly( const MyString& nm, char Slash  )
  {  char *m = StrRChr( nm.Text(),Slash );

    if ( !m ) m = nm.Text();
    m = StrRChr( m,'.' );

    if (m)
      return MyString(m+1);
     else
      return MyString("");
}

CONSTSTR MYRTLEXP FExtOnly( CONSTSTR nm, char Slash  )
  {  CONSTSTR m = StrRChr( nm,Slash ),
              ext;

    if ( !m ) m = nm;
    ext = StrRChr( m,'.' );

    if (ext)
      return ext+1;
     else
      return "";
}
