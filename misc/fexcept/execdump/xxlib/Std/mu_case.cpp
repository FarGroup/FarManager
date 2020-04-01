#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif

#if !defined(__HWIN__)
static char *lwrChars = "©æãª¥­£èé§åêäë¢ ¯à®«¤¦íïçá¬¨âì¡î";
static char *uprChars = "‰–“Š…ƒ˜™‡•š”›‚€Ž‹„†Ÿ—‘Œˆ’œž";

char MYRTLEXP ToUpper( char ch )
  {
    if ( ch >= 'A' && ch <= 'Z' ) return ch;
    if ( ch >= 'a' && ch <= 'z' ) return (char)(ch-'a'+'A');

    for( int n = 0; lwrChars[n]; n++ )
      if ( lwrChars[n] == ch )
        return uprChars[n];
 return ch;
}

char MYRTLEXP ToLower( char ch )
  {
    if ( ch >= 'a' && ch <= 'z' ) return ch;
    if ( ch >= 'A' && ch <= 'Z' ) return (char)(ch-'A'+'a');
    for( int n = 0; uprChars[n]; n++ )
      if ( uprChars[n] == ch )
        return lwrChars[n];
 return ch;
}

BOOL MYRTLEXP isLower( char ch )
  {
    if ( ch >= 'a' && ch <= 'z' ) return TRUE;
 return StrChr(lwrChars,ch) != NULL;
}

BOOL MYRTLEXP isUpper( char ch )
  {
    if ( ch >= 'A' && ch <= 'Z' ) return TRUE;
 return StrChr(uprChars,ch) != NULL;
}

void MYRTLEXP StrLwr( char *str )
  {
    for ( int n = 0; str[n]; n++ )
      str[n] = ToLower( str[n] );
}

void MYRTLEXP StrUpr( char *str )
  {
    for ( int n = 0; str[n]; n++ )
      str[n] = ToUpper( str[n] );
}
#endif

static const char* sysSpaces= " \t";

BOOL MYRTLEXP isSpace( char ch )
  {
 return StrChr( sysSpaces,ch ) != NULL;
}

char *MYRTLEXP StrCase( char *str,msCaseTypes type )
  {  int n;

     if ( !str || *str == 0 ) return str;

     switch( type ){
       case      mscLower: StrLwr( str ); break;
       case      mscUpper: StrUpr( str ); break;
       case mscCapitalize: StrLwr( str );
                           str[0] = ToUpper(str[0]);
                        break;
       case    mscUpLower: for ( n = 0; str[n]; n++ )
                             if ( isLower(str[n]) ) return str;
                           return StrCase( str,mscLower );
       case    mscLoUpper: for ( n = 0; str[n]; n++ )
                             if ( isUpper(str[n]) ) return str;
                           return StrCase( str,mscUpper );
       case     mscInvert: for ( n = 0; str[n]; n++ )
                             if ( isUpper(str[n]) )
                               str[n] = ToLower(str[n]);
                              else
                               str[n] = ToUpper(str[n]);
                        break;
     }
 return str;
}

#if defined(__GNUC__)
static int memicmp(const void *s1, const void *s2, size_t n)
  {
    LPCBYTE b1 = (LPCBYTE)s1,
            b2 = (LPCBYTE)s2;
    for( ; n > 0; n--, b1++, b2++ ) {
      BYTE ch = *b1 - *b2;
      if ( ch )
        return (int)(ch);
    }
 return 0;
}


#endif

BOOL MYRTLEXP BuffCmp( LPBYTE b, LPBYTE b1, DWORD count, BOOL Case )
  {
    if ( Case )
      return memcmp( b,b1,count ) == 0;
     else
      return memicmp( b,b1,count ) == 0;
}
