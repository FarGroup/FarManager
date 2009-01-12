#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif

char *MYRTLEXP MakeIntString( char *str,char dDelimiter )
  {  static char buff[50];
     int  sz = sizeof(buff)-1;
     int  len,n,d;
     char *s;

     if ( dDelimiter ) {
       len = strLen(str);
       s   = str + len-1;
       d   = sz;

       for ( buff[d--] = 0,n = 0; n < sz && n < len; n++ ) {
         if ( n && (n%3) == 0 ) {
           buff[d--] = dDelimiter;
           sz--;
         }
         buff[d--] = *(s--);
       }
       return buff+d+1;
     } else {
       StrCpy( buff,str,sizeof(buff) );
       return buff;
     }
}

char *MYRTLEXP MakeIntString( DWORD val,char delimiter, int divider,CONSTSTR fmt )
  {  char  str[50];
     SNprintf( str, sizeof(str),
               fmt ? fmt : "%lu",
               val / Max(divider,1) );
 return MakeIntString( str,delimiter );
}

char *MYRTLEXP MakeIntString( double val,char delimiter, int devider,CONSTSTR fmt )
  {  char  str[50];
     SNprintf( str, sizeof(str),
               fmt ? fmt : "%.3lf",
               val / Max(devider,1) );
 return MakeIntString( str,delimiter );
}

/*
  10^24   1,000,000,000,000,000,000,000,000   yotta   Y
  10^21   1,000,000,000,000,000,000,000       zetta   Z
  10^18   1,000,000,000,000,000,000           exa     E
  10^15   1,000,000,000,000,000               peta    P
  10^12   1,000,000,000,000                   tera    T
  10^9    1,000,000,000                       giga    G
  10^6    1,000,000                           mega    M
  10^3    1,000                               kilo    k
  10^2    100                                 hecto   h
  10^1    10                                  deka    da
  10^0    1                                   -       -
  10^-1   0.1                                 deci    d
  10^-2   0.01                                centi   c
  10^-3   0.001                               milli   m
  10^-6   0.000 001                           micro   m
  10^-9   0.000 000 001                       nano    n
  10^-12  0.000 000 000 001                   pico    p
  10^-15  0.000 000 000 000 001               femto   f
  10^-18  0.000 000 000 000 000 001           atto    a
  10^-21  0.000 000 000 000 000 000 001       zepto   z
  10^-24  0.000 000 000 000 000 000 000 001   yocto   y
*/
const char *fmt_chars  = "BKMGTPEZYABCDEF";
#define FMT_SIZE 15

CONSTSTR MYRTLEXP FormatCps( char *buff,double val )
  {  static char localBuff[ 40 ];

     if ( !buff ) buff = localBuff;

     int charnum = 0;
     while( val >= 1000. ) {
       charnum++;
       val /= 1000;
     }

     if ( !charnum )            Sprintf( buff,"%d",(int)val ); else
     if ( charnum < FMT_SIZE )  Sprintf( buff,"%3.3lf%c",val,fmt_chars[charnum] ); else
       Sprintf( buff,"%3.3lf?",val );

 return buff;
}

#if defined(__HAS_INT64__)
CONSTSTR MYRTLEXP FormatCps( char *buff,__int64 val )
  {  static char localBuff[ 20 ];

     if ( !buff ) buff = localBuff;

     int charnum = 0;
     __int64 ref = 0;
     while( val >= 1000 ) {
       charnum++;
       ref = val % 1000;
       val /= 1000;
     }

     if ( !charnum )            Sprintf( buff,"%I64u",val ); else
     if ( charnum < FMT_SIZE )  Sprintf( buff,"%I64u.%03I64u%c",val,ref,fmt_chars[charnum] ); else
       Sprintf( buff,"%I64u.%03I64u?",val,ref );

 return buff;
}
#endif

CONSTSTR MYRTLEXP FormatCps( char *buff,DWORD val )
  {  static char localBuff[ 20 ];

     if ( !buff ) buff = localBuff;

     int   charnum = 0;
     DWORD ref = 0;

     while( val >= 1000 ) {
       charnum++;
       ref = val % 1000;
       val /= 1000;
     }

     if ( !charnum )            Sprintf( buff,"%u",val ); else
     if ( charnum < FMT_SIZE )  Sprintf( buff,"%u.%03u%c",val,ref,fmt_chars[charnum] ); else
       Sprintf( buff,"%u.%03u?",val,ref );

 return buff;
}

//------------------------------------------------------------------------
pchar MYRTLEXP FormatDouble( double v, char *buff /*=NULL*/, CONSTSTR fmt /*=NULL*/ )
  {  static char b[ 50 ];
     if ( !buff ) buff = b;

     if ( !fmt ) fmt = "%3.3lf";

     sprintf( buff, fmt, v );
 return buff;
}

//------------------------------------------------------------------------
template <class T> pchar Size2StrT( char *buff,size_t bsz,T sz )
  {  long double size = (long double)sz;
     int         letter = 0;
     size_t      rc;

     for( ; size > 1000 && letter < FMT_SIZE; letter++ )
       size /= 1000;

     if ( !letter ) {
       SNprintf( buff, bsz, sizeof(sz) <= sizeof(DWORD) ? "%ul" : "%I64u", sz );
       return buff;
     }

     SNprintf( buff, bsz, "%Lf", size );
     rc = strlen(buff);
     if ( !rc || strchr(buff,'.') == NULL )
       return buff;

     for ( rc--; rc && buff[rc] == '0'; rc-- );
     if ( buff[rc] != '.' )
       rc++;
     buff[rc]   = fmt_chars[letter];
     buff[rc+1] = 0;

 return buff;
}

template <class T> T Str2SizeT( CONSTSTR str, T /*fake*/ )
  {  size_t rc = strLen( str );

     if ( !rc )
       return 0;

     rc--;
     long double sz = atof( str );

     if ( isdigit(str[rc]) )
       return (T)sz;

     CONSTSTR m;
     m = strchr( fmt_chars, toupper(str[rc]) );
     if ( !m )
       return (T)sz;

     for( rc = m - fmt_chars; rc; rc-- )
       sz *= 1000;

 return (T)sz;
}

BOOL MYRTLEXP IsSizeStr( CONSTSTR str )
  {  int rc = strLen( str );

     if ( !rc )
       return FALSE;

     rc--;
 return !isdigit(str[rc]) &&
        ( str[rc] == 'b' || strchr( fmt_chars, toupper(str[rc]) ) != NULL );
}

pchar MYRTLEXP Size2Str( char *buff,size_t bsz,DWORD sz )
  {
 return Size2StrT( buff,bsz,sz );
}

DWORD MYRTLEXP Str2Size( CONSTSTR str )
  {
 return Str2SizeT( str, (DWORD)0 );
}

#if defined(__HAS_INT64__)
pchar MYRTLEXP Size2Str( char *buff,size_t bsz,__int64 sz )
  {
 return Size2StrT( buff,bsz,sz );
}

__int64 MYRTLEXP Str2Size64( CONSTSTR str )
  {
 return Str2SizeT( str, (__int64 )0 );
}
#endif

//------------------------------------------------------------------------
template <class Type> pchar Digit2StrTU( Type val, char *buff, int radix /*10*/, int BuffSize /*-1*/ )
  {  char str[ 100 ];
     int  pos = sizeof(str)-1;
     int  t;

     if ( !buff )
       return NULL;

     if ( !val ) {
       buff[0] = '0';
       buff[1] = 0;
       return buff;
     }

     for ( str[pos] = 0; pos && val; val /= radix ) {
       t = (int)( val % ((Type)radix) );
       if ( t < 10 )
         str[--pos] = (char)('0'+t);
        else
         str[--pos] = (char)('A'+t-10);
     }

     StrCpy( buff,str+pos,BuffSize );

 return buff;
}

template <class Type> pchar Digit2StrTS( Type val, char *buff, int radix /*10*/, int BuffSize /*-1*/ )
  {  char str[ 100 ];
     int  pos = sizeof(str)-1;
     int  t;
     BOOL sign;

     if ( !buff )
       return NULL;

     if ( !val ) {
       buff[0] = '0';
       buff[1] = 0;
       return buff;
     }

     if ( val < 0 ) {
       sign = TRUE;
       val = -val;
     } else
       sign = FALSE;

     for ( str[pos] = 0; pos && val; val /= radix ) {
       t = (int)( val % ((Type)radix) );
       if ( t < 10 )
         str[--pos] = (char)('0'+t);
        else
         str[--pos] = (char)('A'+t-10);
     }
     if ( pos && sign )
       str[--pos] = '-';

     StrCpy( buff,str+pos,BuffSize );

 return buff;
}

template <class T> T Str2DigitTU( CONSTSTR buff, int radix, T def )
  {  T   val = 0,
         rad = 1;
     size_t n;
     int num;

     if ( !buff || !buff[0] )
       return def;

     n = strlen(buff)-1;
     while( n >= 0 && buff[n] == ' ' ) n--;
     if ( n < 0 )
       return def;

     for ( ; n >= 0; n-- ) {
       char ch = buff[n];
       if ( ch >= '0' && ch <= '9' ) num = ch - '0'; else
       if ( ch >= 'a' && ch <= 'z' ) num = 10 + ch-'a'; else
       if ( ch >= 'A' && ch <= 'Z' ) num = 10 + ch-'A'; else
         return def;
       val += rad*num;
       rad *= radix;
     }
 return val;
}

template <class T> T Str2DigitTS( CONSTSTR buff, int radix, T def )
  {  T   val = 0,
         rad = 1;
     size_t n;
     int num;
     BOOL sign;

     if ( !buff || !buff[0] )
       return def;

     if ( *buff == '-' ) {
       buff++;
       sign = TRUE;
     } else
       sign = FALSE;

     n = strlen(buff)-1;
     while( n >= 0 && buff[n] == ' ' ) n--;
     if ( n < 0 )
       return def;

     for ( ; n >= 0; n-- ) {
       char ch = buff[n];
       if ( ch >= '0' && ch <= '9' ) num = ch-'0'; else
       if ( ch >= 'a' && ch <= 'z' ) num = 10 + ch-'a'; else
       if ( ch >= 'A' && ch <= 'Z' ) num = 10 + ch-'A'; else
         return def;
       if ( num >= radix )
         return def;
       val += rad*num;
       rad *= radix;
     }

     if ( sign )
       return (T)(-val);
      else
       return val;
}

template <class T> T Str2DigitDetectTU( CONSTSTR buff,int DefRadix, T def )
  {  size_t sz;
     T      val;
     char   save;

    if ( !buff || !buff[0] )
      return def;

    sz = strLen(buff)-1;
    save = buff[sz];

  //Char
    if ( buff[0] == '\'' )
      val = (T)buff[1];
     else
  //Str
    if ( buff[0] == '\"' ) {
      val = 0;
      for( buff++,sz = 0; *buff && sz < sizeof(T)/sizeof(BYTE); buff++,sz++ ) {
        val = (T)(val << 8);
        val  |= (T)(*buff);
      }
    } else
  //Hex / Oct
    if ( buff[0] == '0' ) {
      if ( buff[1] == 'x' || buff[1] == 'X' )
        val = Str2Digit(buff+2,16,def);
       else
        val = Str2Digit(buff+1,8,def);
    } else
  //Hex
    if ( save == 'h' || save == 'H' ) {
      ((char*)buff)[sz] = 0;
      val = Str2Digit(buff,16,def);
      ((char*)buff)[sz] = save;
    } else
  //Bin
    if ( save == 'b' || save == 'B' ) {
      ((char*)buff)[sz] = 0;
      val = Str2Digit(buff,2,def);
      ((char*)buff)[sz] = save;
    } else
  //Unknown
      val = Str2Digit( buff, DefRadix, def);

 return val;
}

template <class T> T Str2DigitDetectTS( CONSTSTR buff,int DefRadix, T def )
  {  T      val;
     BOOL   negate;

    if ( !buff || !buff[0] )
      return def;

    if ( *buff == '-' ) {
      negate = TRUE;
      buff++;
    } else
      negate = FALSE;

    val = Str2DigitDetectTU( buff, DefRadix, def );

    if ( negate )
      return (T)( -val );
     else
      return val;
}

#define DECL_D( t ) pchar MYRTLEXP Digit2Str( t digit,char *buff,int radix,int maxlen ) { return Digit2StrTS( digit,buff,radix,maxlen ); }
  DECL_D( signed char )
  DECL_D( signed short )
  DECL_D( signed int )
  DECL_D( signed long )
  #if defined(__HAS_INT64__)
  DECL_D( signed __int64 )
  #endif
#undef DECL_D

#define DECL_D( t ) pchar MYRTLEXP Digit2Str( t digit,char *buff,int radix,int maxlen ) { return Digit2StrTU( digit,buff,radix,maxlen ); }
  DECL_D( unsigned char )
  DECL_D( unsigned short )
  DECL_D( unsigned int )
  DECL_D( unsigned long )
  #if defined(__HAS_INT64__)
  DECL_D( unsigned __int64 )
  #endif
#undef DECL_D

#define DECL_D( t ) t MYRTLEXP Str2Digit( CONSTSTR buff, int radix, t def ) { return Str2DigitTS( buff, radix, def ); }
  DECL_D( signed char )
  DECL_D( signed short )
  DECL_D( signed int )
  DECL_D( signed long )
  #if defined(__HAS_INT64__)
  DECL_D( signed __int64 )
  #endif
#undef DECL_D

#define DECL_D( t ) t MYRTLEXP Str2Digit( CONSTSTR buff, int radix, t def ) { return Str2DigitTU( buff, radix, def ); }
  DECL_D( unsigned char )
  DECL_D( unsigned short )
  DECL_D( unsigned int )
  DECL_D( unsigned long )
  #if defined(__HAS_INT64__)
  DECL_D( unsigned __int64 )
  #endif
#undef DECL_D

#define DECL_D( t ) t MYRTLEXP Str2DigitDetect( CONSTSTR buff, int radix, t def ) { return Str2DigitDetectTU( buff, radix, def ); }
  DECL_D( unsigned char )
  DECL_D( unsigned short )
  DECL_D( unsigned int )
  DECL_D( unsigned long )
  #if defined(__HAS_INT64__)
  DECL_D( unsigned __int64 )
  #endif
#undef DECL_D

#define DECL_D( t ) t MYRTLEXP Str2DigitDetect( CONSTSTR buff, int radix, t def ) { return Str2DigitDetectTS( buff, radix, def ); }
  DECL_D( signed char )
  DECL_D( signed short )
  DECL_D( signed int )
  DECL_D( signed long )
  #if defined(__HAS_INT64__)
  DECL_D( signed __int64 )
  #endif
#undef DECL_D
