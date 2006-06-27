#include <all_far.h>
#pragma hdrstop

#include "fstdlib.h"

pchar DECLSPEC AddLastSlash( char *path, char Slash )
  { int len;
    if ( (len=strLen(path)) != 0 && path[len-1] != Slash ) {
      path[len]   = Slash;
      path[len+1] = 0;
    }
 return path;
}

pchar DECLSPEC DelLastSlash( char *path, char Slash )
  {  int len;

    if ( path && path[0] && path[len=(strLen(path)-1)] == Slash )
      path[len] = 0;

 return path;
}

CONSTSTR DECLSPEC FPath( CONSTSTR nm, char Slash )
  {  static char str[ MAX_PATH_SIZE ];
     char     *m;

     StrCpy( str,nm,sizeof(str) );
     m = StrRChr( str,Slash );
     if (m) *m = 0; else str[0] = 0;

 return AddLastSlash(str);
}

CONSTSTR DECLSPEC FName( CONSTSTR nm, char Slash )
  {  static char str[ MAX_PATH_SIZE ];
     CONSTSTR m = StrRChr( nm,Slash );

     if (!m) m = nm; else m++;
     StrCpy( str,m,sizeof(str) );
 return str;
}

CONSTSTR DECLSPEC FExtOnly( CONSTSTR nm, char Slash  )
  {  CONSTSTR m = StrRChr( nm,Slash ),
              ext;

    if ( !m ) m = nm;
    ext = StrRChr( m,'.' );

    if (ext)
      return ext+1;
     else
      return "";
}

/*
   Create CPS value string

   The digit allways 3+1+3+1 characters length (8)
   Digit right alignmented, filled with ' ' at left
*/
CONSTSTR DECLSPEC FCps( char *buff,double val )
  {  char     Letter;
     char     str[50];
     CONSTSTR _buff = buff;

//1M
    if ( val >= 1000000. ) {
      Letter = 'M';
      val   /= 1000000;
    } else
//1K
    if ( val >= 1000. ) {
      Letter = 'K';
      val   /= 1000;
    } else {
//<1K
      Letter = 'b';
    }

    if ( Letter == 'b' )
      Sprintf( str,"%db",(int)val );
     else
      Sprintf( str,"%3.3lf%c",val,Letter );

    int sz;
    for( sz = 8 - strLen(str); sz > 0; buff++,sz-- ) *buff = ' ';
    for( sz = 0; str[sz]; buff++,sz++ ) *buff = str[sz];
    *buff = 0;

 return _buff;
}
