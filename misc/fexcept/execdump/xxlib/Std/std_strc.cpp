#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
#if defined(__HWIN__) && !defined(__STR_LOCALE__)
  #define __STR_LOCALE__ LOCALE_USER_DEFAULT
#endif

#if 1
  #define PROC( v )
  #define Log( v )
#else
  #define PROC( v )  INProc __proc v ;
  #define Log( v )   INProc::Say v
#endif

#if defined(__GUARD_MEMORY__)
  #define CHK(v)   TraceAssert( _HeapCheck() && v );
#else
  #define CHK(v)
#endif

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
template <class strT,class chT> strT StrChrT( strT s, chT ch )
  {
    if ( !s ) return NULL;

    for( ; *s; s++ )
      if ( *s == ch ) return s;

 return NULL;
}

template <class strT> strT StrEOL( strT str )
  {
    if ( !str ) return NULL;
    while( *str ) str++;
 return str;
}

template <class srcT,class destT> destT StrCatT( destT dest,srcT src,int dest_sz )
  {  destT _dest = dest;

    if ( !dest )
      return NULL;

    if ( !src || !src[0] )
      return dest;

    if ( !dest_sz ) {
      *dest = 0;
      return dest;
    }

    if ( dest_sz != -1 ) {
      for( ; *dest && dest_sz; dest++ ) dest_sz--;
      if (!dest_sz) return _dest;

      for( ; *src && dest_sz; dest++,src++,dest_sz-- ) *dest = *src;
      if ( !dest_sz ) dest--;
      *dest = 0;
    } else {
      dest = StrEOL( dest );

      for( ; *src; dest++,src++ ) *dest = *src;
      *dest = 0;
    }

 return _dest;
}

template <class strT,class chT> strT StrRChrT( strT str,chT ch )
  {  strT e = StrEOL( str );

     for( ; e > str; e-- )
       if ( *e == ch )
         return e;

 return NULL;
}
//------------------------------------------------------------------------
//
//------------------------------------------------------------------------
int      MYRTLEXP strLen( CONSTSTR str )         { return str ? (int)strlen(str) : 0; }

pchar    MYRTLEXP StrChr( pchar s, char ch )     { return StrChrT( s,ch ); }
CONSTSTR MYRTLEXP StrChr( CONSTSTR s, char ch )  { return StrChrT( s,ch ); }
pchar    MYRTLEXP StrRChr( pchar s, char ch )    { return s ? StrRChrT( (char*)s,ch ) : NULL; }
CONSTSTR MYRTLEXP StrRChr( CONSTSTR s, char ch ) { return s ? StrRChrT( (char*)s,ch ) : NULL; }

CONSTSTR MYRTLEXP StrStr( CONSTSTR str,CONSTSTR sub,BOOL CaseSens )
  {  BOOL same;
     int  n;

    if ( !sub || !sub[0] ) return FALSE;

    for( n = 0; *str; str++ ) {
      if ( CaseSens )
        same = *str == sub[n];
       else
        same = ToUpperI(*str) == ToUpperI(sub[n]);
      if ( same ) {
        n++;
        if ( !sub[n] ) return str-n+1;
      } else
        n = 0;
    }
 return NULL;
}

char *MYRTLEXP StrCpy( char *dest,CONSTSTR src,int dest_sz )
  {
    if ( !dest )
      return NULL;

    *dest = 0;
    if ( dest_sz == 0 || !src )
      return dest;

CHK( "Before strcpy" )
    if ( dest_sz != -1 ) {
      strncpy( dest,src,dest_sz-1 );
      dest[dest_sz-1] = 0;
    } else
      strcpy( dest,src );
CHK( "After strcpy" )

 return dest;
}

pchar MYRTLEXP StrCat( pchar dest,CONSTSTR src,int dest_sz ) { return StrCatT( dest,src,dest_sz ); }

//------------------------------------------------------------------------
#if defined(__HWIN32__)
int MYRTLEXP StrCmp( CONSTSTR str,CONSTSTR str1,int maxlen, BOOL isCaseSens )
  {
    if ( !str )  return (str1 == NULL) ? 0 : (-1);
    if ( !str1 ) return 1;

    int rc;
    rc = CompareString( __STR_LOCALE__,
                        isCaseSens ? 0 : NORM_IGNORECASE,
                        str,maxlen,str1, maxlen );

    if ( rc == 0 )
      HAbort( "!CompareString" );

 return rc-2;
}
#else
int MYRTLEXP StrCmp( CONSTSTR str,CONSTSTR str1,int maxlen, BOOL isCaseSens )
  {  int  n,diff;

  if ( !str ) return (str1 == NULL)?0:(-1);
  if ( !str1 ) return (str == NULL)?0:1;

  if ( !isCaseSens ) {
    for ( n = 0; str[n] && str1[n] && (maxlen == -1 || n < maxlen); n++ ) {
      diff = ToLower(str[n]) - ToLower(str1[n]);
      if ( diff ) return diff;
    }
    diff = ToLower(str[n]) - ToLower(str1[n]);
  } else {
    for ( n = 0; str[n] && str1[n] && (maxlen == -1 || n < maxlen); n++ )
      if ( (diff=(str[n]-str1[n])) != 0 ) return diff;
    diff = str[n] - str1[n];
  }

 return (n == maxlen)?0:diff;
}
#endif

//------------------------------------------------------------------------
#if defined(__WINUNICODE__)
size_t MYRTLEXP strLen( WCONSTSTR str )
  {
 return str ? lstrlenW(str) : 0;
}

wpchar MYRTLEXP StrCpy( wchar_t *dest,WCONSTSTR src,int dest_sz )
  {
    if ( !dest )
      return NULL;

    *dest = 0;
    if ( dest_sz == 0 || !src )
      return dest;

CHK( "Before strcpy" )
    if ( dest_sz != -1 ) {
      lstrcpynW( dest,src,dest_sz-1 );
      dest[dest_sz-1] = 0;
    } else
      lstrcpyW( dest,src );
CHK( "After strcpy" )

 return dest;
}

wpchar MYRTLEXP StrCpy( wchar_t *dest, CONSTSTR src, int sz )
  {
    if ( sz == -1 ) sz = strLen(src);
    MultiByteToWideChar( CP_ACP,0,src,-1,dest,sz );
    dest[sz] = 0;
 return dest;
}

pchar MYRTLEXP StrCpy( char *dest, WCONSTSTR src, int sz )
  {
    if ( sz == -1 ) sz = strLen(src);
    WideCharToMultiByte( CP_ACP,0,src,-1,dest,sz,NULL,NULL );
    dest[sz] = 0;
 return dest;
}

int MYRTLEXP StrCmp( WCONSTSTR str,WCONSTSTR str1,int maxlen, BOOL isCaseSens )
  {
    if ( !str )  return (str1 == NULL) ? 0 : (-1);
    if ( !str1 ) return 1;

    int rc;
    rc = CompareStringW( __STR_LOCALE__,
                        isCaseSens ? 0 : NORM_IGNORECASE,
                        str,maxlen,str1, maxlen );

    if ( rc == 0 )
      HAbort( "!CompareString" );

 return rc-2;
}

wpchar    MYRTLEXP StrChr( wpchar s, wchar_t ch )    { return StrChrT( s,ch ); }
WCONSTSTR MYRTLEXP StrChr( WCONSTSTR s, wchar_t ch ) { return StrChrT( s,ch ); }

wpchar    MYRTLEXP StrCat( wpchar dest,WCONSTSTR src,int dest_sz ) { return StrCatT( dest,src,dest_sz ); }
wpchar    MYRTLEXP StrCat( wpchar dest,CONSTSTR src,int dest_sz )  { return StrCatT( dest,src,dest_sz ); }

WCONSTSTR MYRTLEXP StrRChr( WCONSTSTR s, char ch )    { return StrRChr(s,ch); }
wpchar    MYRTLEXP StrRChr( wpchar s, char ch )       { return StrRChr(s,ch); }
WCONSTSTR MYRTLEXP StrRChr( WCONSTSTR s, wchar_t ch ) { return StrRChr(s,ch); }
wpchar    MYRTLEXP StrRChr( wpchar s, wchar_t ch )    { return StrRChr(s,ch); }

#endif
