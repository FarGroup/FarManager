#ifndef __UTILS_H
#define __UTILS_H

class MyString;

/*******************************************************************
   TYPES
 *******************************************************************/
typedef void (RTL_CALLBACK *AbortProc)(void);

enum _CTCP_CodePages {
 CT_CP_DOS866      = 0,   //Code tables for Rus2Eng / Eng2Rus
 CT_CP_DOSWIN1251  = 1,
 CT_CP_ISO8859_5   = 2,
 CT_CP_KOI8        = 3,
 CT_CP_MAC         = 4,
 CT_CP_AMIGA       = 5,
 CT_CP_EMAIL       = 6,
 CT_CP_MAXTABLES   = (CT_CP_EMAIL+1)
};

enum msCaseTypes  {
  mscUnchange   = 0x01,
  mscLower      = 0x02,
  mscUpper      = 0x04,
  mscCapitalize = 0x08,
  mscUpLower    = 0x10,
  mscLoUpper    = 0x20,
  mscInvert     = 0x40
};

enum talTypes {
  talNone,
  talLeft,
  talCenter,
  talRight
};

enum maCmp {
  masLess  = -1,
  masEqual = 0,
  masMore  = 1
};
inline maCmp Int2Cmp( int v ) { return (v==0)?masEqual:((v<0)?masMore:masLess); }

STRUCT( CTScanStruct )
  BYTE  Scan;
  const char *Text;
};

//--- RANDOM
#if defined(__QNX__)
  HDECLSPEC int  MYRTLEXP random( int val );
  HDECLSPEC void MYRTLEXP randomize( void );
#endif

/*******************************************************************
   HAbort
 *******************************************************************/
#if defined(__HWIN__)
  HDECLSPEC CONSTSTR MYRTLEXP    WINError( void );
  HDECLSPEC void     MYRTLEXP_PT __WinAbort( CONSTSTR msg,... );
  HDECLSPEC void     MYRTLEXP    __WinAbortV( CONSTSTR msg,va_list args );
#endif

  HDECLSPEC void     MYRTLEXP_PT __ConAbort( CONSTSTR msg,... );
  HDECLSPEC void     MYRTLEXP    __ConAbortV( CONSTSTR msg,va_list args );

#if defined(__HWIN__) && !defined(__HCONSOLE__)
  #define HAbort   __WinAbort
  #define HAbortV  __WinAbortV
#else
  #define HAbort   __ConAbort
  #define HAbortV  __ConAbortV
#endif

/*******************************************************************
   INT64
 *******************************************************************/
#if defined(__HAS_INT64__)
  inline void Int64ToDWORD( __int64 v,DWORD& hi, DWORD& lo )
    {
       lo = ((LPDWORD)(&v))[0];
       hi = ((LPDWORD)(&v))[1];
  }

  inline void DWORDToInt64( DWORD hi, DWORD lo,__int64& v )
    {
       ((LPDWORD)(&v))[0] = lo;
       ((LPDWORD)(&v))[1] = hi;
  }
#endif

/*******************************************************************
   Clock [mu_clock]
 *******************************************************************/
HDECLSPEC time_t        MYRTLEXP GetClock( void );
HDECLSPEC double        MYRTLEXP CmpClock( time_t,time_t right );
HDECLSPEC pchar         MYRTLEXP MakeClockString( char *buff,time_t cl,int maxw = -1 );

/* Create string of digit with delimit thousands with special character
     sval       - value
     delimiter  - character used to delimit thousands
     divider    - devider (function use `sval/devider` instead `sval` itself)
     fmt        - printf-like format string to create base digit string. "%lu" by default.
*/
HDECLSPEC pchar  MYRTLEXP MakeIntString( DWORD sval, char delimiter = 0,int divider = 1,CONSTSTR fmt = NULL );   //Returns local buffer
HDECLSPEC pchar  MYRTLEXP MakeIntString( double sval, char delimiter = 0,int devider = 1,CONSTSTR fmt = NULL );  //Returns local buffer

//String with separator between groups of 3 characters
HDECLSPEC pchar  MYRTLEXP MakeIntString( char *str,char delimiter = 0 );                                         //Returns local buffer

/* ItoA / AtoI
*/
/*@@
  pchar Digit2Str( Type digit,char *buff,int radix = 10,int maxlen = -1 );
   Para:
     digit  - value
     buff   - char buffer
     radix  - used radix
     maxlen - max length of buffer
   Ret:
     buffer
*/
#define DEF_D( t ) HDECLSPEC pchar MYRTLEXP Digit2Str( t digit,char *buff,int radix = 10,int maxlen = -1 );
  DEF_D( signed char )
  DEF_D( unsigned char )
  DEF_D( signed short )
  DEF_D( unsigned short )
  DEF_D( signed int )
  DEF_D( unsigned int )
  DEF_D( signed long )
  DEF_D( unsigned long )
  #if defined(__HAS_INT64__)
  DEF_D( signed __int64 )
  DEF_D( unsigned __int64 )
  #endif
#undef DEF_D

/*@@
  Type Str2Digit( CONSTSTR buff, int radix, Type def );
    Para:
     buff  - buffer to convert from
     radix - use radix
     def   - default value to set on error
    Ret:
     Conversion result on success or `def` on error
*/
#define DEF_D( t ) HDECLSPEC t MYRTLEXP Str2Digit( CONSTSTR buff, int radix, t def );
  DEF_D( signed char )
  DEF_D( unsigned char )
  DEF_D( signed short )
  DEF_D( unsigned short )
  DEF_D( signed int )
  DEF_D( unsigned int )
  DEF_D( signed long )
  DEF_D( unsigned long )
  #if defined(__HAS_INT64__)
  DEF_D( signed __int64 )
  DEF_D( unsigned __int64 )
  #endif
#undef DEF_D

/*@@
  Type Str2DigitDetect( CONSTSTR buff, int radix, Type def );
    Para:
     buff  - buffer to convert from
     radix - use radix. Used only with unknown format.
     def   - default value to set on error
    Ret:
     Conversion result on success or `def` on error
    Known:
     xxxH  - hex
     0xXXX - hex
     0xxx  - oct
     xxxB  - bin
     xxx   - dec
     'x'   - char code
     "xx"  - char codes in litle-endian form
*/
#define DEF_D( t ) HDECLSPEC t MYRTLEXP Str2DigitDetect( CONSTSTR buff, int radix, t def );
  DEF_D( signed char )
  DEF_D( unsigned char )
  DEF_D( signed short )
  DEF_D( unsigned short )
  DEF_D( signed int )
  DEF_D( unsigned int )
  DEF_D( signed long )
  DEF_D( unsigned long )
  #if defined(__HAS_INT64__)
  DEF_D( signed __int64 )
  DEF_D( unsigned __int64 )
  #endif
#undef DEF_D

inline    int    AtoI( CONSTSTR buff )                 { return Str2DigitDetect( buff, 10, (int)0 ); }
inline    pchar  ItoA( int val,char *buff,int radix )  { return Digit2Str( val, buff, radix, -1 ); }

/* Create CPS value string

   The digit allways 3+1+3+1 characters length (8)
   Digit right alignmented, filled with ' ' at left
*/
HDECLSPEC CONSTSTR      MYRTLEXP FormatCps( char *buff/*=NULL*/,double val );
#if defined(__HAS_INT64__)
HDECLSPEC CONSTSTR      MYRTLEXP FormatCps( char *buff/*=NULL*/,__int64 val );
#endif
HDECLSPEC CONSTSTR      MYRTLEXP FormatCps( char *buff,DWORD val );

inline CONSTSTR         MYRTLEXP FormatCps( char *buff,int val )     { return FormatCps( buff, (DWORD)val ); }

HDECLSPEC pchar         MYRTLEXP FormatDouble( double v, char *buff=NULL, CONSTSTR fmt=NULL );

HDECLSPEC BOOL          MYRTLEXP IsSizeStr( CONSTSTR str );
HDECLSPEC pchar         MYRTLEXP Size2Str( char *buff, size_t bsz, DWORD size );
HDECLSPEC DWORD         MYRTLEXP Str2Size( CONSTSTR str );
#if defined(__HAS_INT64__)
HDECLSPEC pchar         MYRTLEXP Size2Str( char *buff, size_t bsz, __int64 size );
HDECLSPEC __int64       MYRTLEXP Str2Size64( CONSTSTR str );
#endif

/*******************************************************************
   Character coders [mu_cp]
 *******************************************************************/
HDECLSPEC char          MYRTLEXP Eng2Rus( char ch,int tablenum = CT_CP_DOS866 );
HDECLSPEC char          MYRTLEXP Rus2Eng( char ch,int tablenum = CT_CP_DOS866 );
HDECLSPEC char          MYRTLEXP Rus2CP( char ch,int tablefrom, int tableto );

/*******************************************************************
   tolower XXX [mu_case]
 *******************************************************************/
#if !defined(__HWIN__) || defined(__HWIN16__)
inline    char                   ToUpperI( char c ) { return (c >= 'a' && c <= 'z')?((char)((c) + 'A' - 'a')):c; }
inline    char                   ToLowerI( char c ) { return (c >= 'A' && c <= 'Z')?((char)((c) + 'a' - 'A')):c; }
HDECLSPEC char          MYRTLEXP ToLower( char ch );
HDECLSPEC char          MYRTLEXP ToUpper( char ch );
HDECLSPEC BOOL          MYRTLEXP isLower( char ch );
HDECLSPEC BOOL          MYRTLEXP isUpper( char ch );
HDECLSPEC void          MYRTLEXP StrUpr( char *str );
HDECLSPEC void          MYRTLEXP StrLwr( char *str );
#else
inline    char                   ToUpperI( char c )  { return (char)CharUpperBuff(&c, 1); }
inline    char                   ToLowerI( char c )  { return (char)CharLowerBuff(&c, 1); }
inline    char                   ToLower( char ch )  { return ToLowerI(ch); }
inline    char                   ToUpper( char ch )  { return ToUpperI(ch); }
inline    BOOL                   isLower( char ch )  { return IsCharLower( (TCHAR)ch ); }
inline    BOOL                   isUpper( char ch )  { return IsCharUpper( (TCHAR)ch ); }
inline    void                   StrUpr( char *str ) { if ( str ) CharUpper( str ); }
inline    void                   StrLwr( char *str ) { if ( str ) CharLower( str ); }
#endif
HDECLSPEC BOOL          MYRTLEXP IsPrintChar( char key );
HDECLSPEC BOOL          MYRTLEXP isSpace( char ch );
/*******************************************************************
    Str
 *******************************************************************/
#if defined(__WINUNICODE__)
HDECLSPEC size_t        MYRTLEXP strLen( WCONSTSTR str );
HDECLSPEC wpchar        MYRTLEXP StrCpy( wchar_t *dest,WCONSTSTR src,int dest_sz = -1 );
HDECLSPEC wpchar        MYRTLEXP StrCpy( wchar_t *dest, CONSTSTR src, int sz = -1 );
HDECLSPEC pchar         MYRTLEXP StrCpy( char *dest, WCONSTSTR src, int sz = -1 );
HDECLSPEC wpchar        MYRTLEXP StrCat( wpchar dest,WCONSTSTR src,int dest_sz = -1 );
HDECLSPEC wpchar        MYRTLEXP StrCat( wpchar dest,CONSTSTR src,int dest_sz = -1 );

HDECLSPEC int           MYRTLEXP StrCmp( WCONSTSTR str,WCONSTSTR str1,int maxlen = -1, BOOL isCaseSens = TRUE );
inline    int                    StrCmpI( WCONSTSTR str,WCONSTSTR str1 )               { return StrCmp(str,str1,-1,FALSE); }
inline    int                    StrNCmp( WCONSTSTR str,WCONSTSTR str1,int maxlen )    { return StrCmp(str,str1,maxlen,TRUE); }
inline    int                    StrNCmpI( WCONSTSTR str,WCONSTSTR str1,int maxlen )   { return StrCmp(str,str1,maxlen,FALSE); }

HDECLSPEC wpchar        MYRTLEXP StrChr( wpchar s,    wchar_t ch );
HDECLSPEC WCONSTSTR     MYRTLEXP StrChr( WCONSTSTR s, wchar_t ch );
HDECLSPEC WCONSTSTR     MYRTLEXP StrRChr( WCONSTSTR s, char ch );
HDECLSPEC wpchar        MYRTLEXP StrRChr( wpchar s, char ch );
HDECLSPEC WCONSTSTR     MYRTLEXP StrRChr( WCONSTSTR s, wchar_t ch );
HDECLSPEC wpchar        MYRTLEXP StrRChr( wpchar s, wchar_t ch );
#endif

HDECLSPEC int           MYRTLEXP strLen( CONSTSTR str );
HDECLSPEC pchar         MYRTLEXP StrCpy( char *dest,CONSTSTR src,int dest_sz = -1 );
HDECLSPEC pchar         MYRTLEXP StrCat( char *dest,CONSTSTR src,int dest_sz = -1 );

HDECLSPEC int           MYRTLEXP StrCmp( CONSTSTR str,CONSTSTR str1,int maxlen = -1, BOOL isCaseSens = TRUE );
inline    int                    StrCmpI( CONSTSTR str,CONSTSTR str1 )               { return StrCmp(str,str1,-1,FALSE); }
inline    int                    StrNCmp( CONSTSTR str,CONSTSTR str1,int maxlen )    { return StrCmp(str,str1,maxlen,TRUE); }
inline    int                    StrNCmpI( CONSTSTR str,CONSTSTR str1,int maxlen )   { return StrCmp(str,str1,maxlen,FALSE); }

HDECLSPEC pchar         MYRTLEXP StrChr( pchar s, char ch );
HDECLSPEC CONSTSTR      MYRTLEXP StrChr( CONSTSTR s, char ch );
HDECLSPEC pchar         MYRTLEXP StrRChr( pchar s, char ch );
HDECLSPEC CONSTSTR      MYRTLEXP StrRChr( CONSTSTR s, char ch );
HDECLSPEC int           MYRTLEXP StrNChr( CONSTSTR str,char ch,int maxlen = -1 );
HDECLSPEC int           MYRTLEXP StrChrCount( CONSTSTR str,char ch,int maxlen = -1 );
HDECLSPEC int           MYRTLEXP StrPosChr( CONSTSTR str,char ch,int pos = 0 );

HDECLSPEC int           MYRTLEXP StrPosStr( CONSTSTR str,CONSTSTR s,int pos = 0 );
HDECLSPEC CONSTSTR      MYRTLEXP StrStr( CONSTSTR str,CONSTSTR sub,BOOL CaseSens = FALSE );
HDECLSPEC CONSTSTR      MYRTLEXP StrDelStr( CONSTSTR str,CONSTSTR subStr,int pos = 0 );

HDECLSPEC pchar         MYRTLEXP StrCase( char *str,msCaseTypes type );
HDECLSPEC BOOL          MYRTLEXP BuffCmp( LPBYTE b, LPBYTE b1, DWORD count, BOOL CaseSensitive );

#define TStrCpy( d, s ) StrCpy( d, s, sizeof(d) )
#define TStrCat( d, s ) StrCat( d, s, sizeof(d) )

/*******************************************************************
   [mu_str]
 *******************************************************************/
HDECLSPEC pchar         MYRTLEXP StrAlloc( int newLen );
HDECLSPEC void          MYRTLEXP StrFree( CONSTSTR s );
HDECLSPEC pchar         MYRTLEXP StrDup( CONSTSTR old, int newLen = -1 );
HDECLSPEC pchar         MYRTLEXP StrRealloc( CONSTSTR old, int newLen );
HDECLSPEC pchar         MYRTLEXP StrTrim( char *str );
HDECLSPEC pchar         MYRTLEXP StrTrim( char *buff, CONSTSTR str, int bsz /*=-1*/ );
/*******************************************************************
   Tools
 *******************************************************************/
HDECLSPEC DWORD         MYRTLEXP MakeStringHash( CONSTSTR dptr,DWORD dsize );
HDECLSPEC pchar         MYRTLEXP StrMakeAlign( char *dest,CONSTSTR source,
                                               int maxWidth, talTypes align, talTypes *over );
/*******************************************************************
   Text <=> STRZ [u_text]
 *******************************************************************/
HDECLSPEC CONSTSTR      MYRTLEXP Str2Text( CONSTSTR name );    //Expand '\n' chans to "\\n"
HDECLSPEC CONSTSTR      MYRTLEXP Text2Str( CONSTSTR val1 );    //Cat "\\n" chars to '\n'
HDECLSPEC DWORD         MYRTLEXP Text2Bytes( CONSTSTR text, LPBYTE buff, DWORD bsz );
HDECLSPEC CONSTSTR      MYRTLEXP MakeOuttedStr( char *dest,CONSTSTR source,int maxlen );
HDECLSPEC CONSTSTR      MYRTLEXP GetEOLStr( WORD eol );             //Rets string w eol codes but expand "\n" to "\\n"
HDECLSPEC CONSTSTR      MYRTLEXP GetTextEOLStr( WORD eol );
HDECLSPEC int           MYRTLEXP AttrStrLen( CONSTSTR str );
/*******************************************************************
   Cols [mu_scol]
 *******************************************************************/
HDECLSPEC int           MYRTLEXP StrColCount( CONSTSTR str,CONSTSTR seps );
HDECLSPEC CONSTSTR      MYRTLEXP StrGetCol( MyString& buff, CONSTSTR str,int number,CONSTSTR seps );
HDECLSPEC CONSTSTR      MYRTLEXP StrGetCol( CONSTSTR str,int number,CONSTSTR seps );  //!! Uses stat buffer
HDECLSPEC CONSTSTR      MYRTLEXP StrDelCol( MyString& buff, CONSTSTR str,int number,CONSTSTR seps );
HDECLSPEC CONSTSTR      MYRTLEXP StrDelCol( CONSTSTR str,int number,CONSTSTR seps );  //!! Uses stat buffer
HDECLSPEC int           MYRTLEXP StrFindCol( CONSTSTR str,CONSTSTR seps,CONSTSTR col,int fromCol = -1 ); //!! Uses stat buffer
HDECLSPEC BOOL          MYRTLEXP StrContainCol( CONSTSTR str,CONSTSTR seps,CONSTSTR col ); //!! Uses stat buffer
/*******************************************************************
   Utils [mu_os]
 *******************************************************************/
HDECLSPEC BOOL          MYRTLEXP CheckReadable( const LPVOID Ptr,SIZE_T sz );
HDECLSPEC WORD          MYRTLEXP OSVersion( void );
HDECLSPEC const char*   MYRTLEXP OSPlatform( void );
HDECLSPEC void          MYRTLEXP ErrorBeep( void );
HDECLSPEC AbortProc     MYRTLEXP AtExit( AbortProc p );                //Regiseter exit proc. Client must call prev proc
HDECLSPEC void      RTL_CALLBACK CallAtExit( void );                   //Call registered exit chain. Unregister all.
HDECLSPEC void          MYRTLEXP FreeSlice( void );
HDECLSPEC HANDLE        MYRTLEXP GetHInstance( void );
#if defined(__HWIN32__)
HDECLSPEC CONSTSTR      MYRTLEXP ToOEM( CONSTSTR s );
HDECLSPEC CONSTSTR      MYRTLEXP FromOEM( CONSTSTR s );
#else
#define ToOEM( v )   v
#define FromOEM( v ) v
#endif
/*******************************************************************
   Cmd [mu_arg]
 *******************************************************************/
HDECLSPEC void          MYRTLEXP CTArgInit( int argc, char **argv,BOOL CaseSensitive = FALSE );
HDECLSPEC void          MYRTLEXP CTArgInit( CONSTSTR Args,BOOL CaseSensitive = FALSE );
HDECLSPEC const char*   MYRTLEXP CTArgGet( int num );                   //get base argument
HDECLSPEC const char*   MYRTLEXP CTArgGetArg( int num );                //get`s argument that not switch
HDECLSPEC const char*   MYRTLEXP CTArgGet( CONSTSTR name );             //find "[-/]<name>[=<value>]" key and ret <value> || NULL
HDECLSPEC BOOL          MYRTLEXP CTArgCheck( CONSTSTR name );           //check for "[-/]<name>" exist
HDECLSPEC int           MYRTLEXP CTArgCount( void );
HDECLSPEC pchar         MYRTLEXP CTGetArgPathName( CONSTSTR ArgName, CONSTSTR DefFileName, char *Buffer, int bSz );

/*******************************************************************
   Key [mu_key]
 *******************************************************************/
HDECLSPEC WORD          MYRTLEXP Scan2ANSII( BYTE scan );   // ret`s scNOANSII if no ANSII
HDECLSPEC BYTE          MYRTLEXP ANSII2Scan( char scan );
HDECLSPEC BOOL          MYRTLEXP IsPrintKey( DWORD key );
HDECLSPEC DWORD         MYRTLEXP Str2Key( CONSTSTR str );
HDECLSPEC BYTE          MYRTLEXP Scan2HWScan( BYTE sc );
HDECLSPEC BYTE          MYRTLEXP HWScan2Scan( BYTE sc );
HDECLSPEC CONSTSTR      MYRTLEXP Key2Str( DWORD key );
HDECLSPEC int           MYRTLEXP FindScanKey( BYTE scan );
HDECLSPEC int           MYRTLEXP FindScanKey( CONSTSTR str );
HDECLSPEC int           MYRTLEXP FindHWScan( BYTE scan );
HDECLSPEC int           MYRTLEXP FindScanHW( BYTE scan );
/*******************************************************************
   DIFFERENT
 *******************************************************************/
HDECLSPEC char          MYRTLEXP GetRunChar( void );    // [mu_runch.cpp]    Gets next character to simulate running "|/-\"
HDECLSPEC CONSTSTR      MYRTLEXP SignalName( int sig ); // [mu_sig.cpp]  Return signal name and description

/*******************************************************************
   AttachFileOutput
   [io_dupout.cpp]

   Attach additional file to existing file.
   All output will be duplicated to `destFile` too.

   Hooks up: printf, vprintf, fprintf, vfprintf

   Set `destFile` to NULL to ignore all output to `srcFile`
   To catch stdout and stderr use FILE_STD_OUT and FILE_STD_ERR instead `srcFile`
 *******************************************************************/
#define FILE_STD_OUT ((FILE*)1)
#define FILE_STD_ERR ((FILE*)2)

HDECLSPEC void          MYRTLEXP AttachFileOutput( FILE *srcFile,FILE *destFile );
HDECLSPEC void          MYRTLEXP AttachFileOutput( FILE *srcFile,HVPrintProc_t destCallBack );
HDECLSPEC void          MYRTLEXP DetachFileOutput( FILE *srcFile );

/*******************************************************************
   BufferedPrintf

 [mu_bprint.cpp]

   Buffered output to `print`.

   Accumulate output in buffer and call `print` only if buff size reached
   or on '\n' and '\r' characters.
 *******************************************************************/
HDECLSPEC int MYRTLEXP_PT BufferedPrintf( char *Buff,int bSize,HPrintProc_t print,CONSTSTR Fmt,... );
HDECLSPEC int MYRTLEXP    BufferedPrintfV( char *Buff,int bSize,HPrintProc_t print,CONSTSTR Fmt,va_list arglist );

/*******************************************************************
   Crc32

 [mu_crc32.cpp]

   Generate 32bit CRC for buffer.

   Algoriphm getted from sources of ZLib.
 *******************************************************************/
HDECLSPEC DWORD MYRTLEXP Crc32( DWORD crc, LPCBYTE buf,DWORD len );
HDECLSPEC DWORD MYRTLEXP Crc32( DWORD crc, CONSTSTR buf );

/**
  [timer.cpp]
*/
HDECLSPEC HANDLE MYRTLEXP PRTimerCreate( sDWORD ms );
HDECLSPEC void   MYRTLEXP PRTimerDestroy( HANDLE p );
HDECLSPEC BOOL   MYRTLEXP PRTimerWait( HANDLE p );

/*******************************************************************
   X_TRY
     <actions>
   X_CATCH
     <report error>
   X_END

 [io_xjmp.cpp]

   A try\catch variant for catch any Win32 exceptions inside `actions`.
   Can not be nested, but can be used in nested procedures.
 *******************************************************************/
#if defined(__HWIN__)
  #define X_TRY         __try{
  #define X_CATCH       }__except( EXCEPTION_EXECUTE_HANDLER ) {
  #define X_END         }
#endif

#endif  //MUTILS
