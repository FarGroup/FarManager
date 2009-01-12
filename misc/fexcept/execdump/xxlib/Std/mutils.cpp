#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif

/***************************************
         All platform Procedures
 ***************************************/
int MYRTLEXP StrChrCount( CONSTSTR str,char ch,int maxlen )
  {  int cn = 0;
    for ( int n = 0; str[n] && (maxlen == -1 || n < maxlen); n++ )
      if ( str[n] == ch ) cn++;
 return cn;
}

int MYRTLEXP StrNChr( CONSTSTR str,char ch,int maxlen )
  {
    for ( int n = 0; str[n] && (maxlen == -1 || n < maxlen); n++ )
      if ( str[n] == ch ) return n;
 return -1;
}

int MYRTLEXP StrPosChr( CONSTSTR str,char ch,int pos )
  {
    if ( !str || pos < 0 || pos >= (int)strLen(str) ) return -1;
    for ( int n = pos; str[n]; n++ )
      if ( str[n] == ch ) return n;
 return -1;
}

int MYRTLEXP StrPosStr( CONSTSTR str,CONSTSTR s,int pos )
  {
    if ( !str || !s || !*s || pos < 0 || pos >= (int)strLen(str) ) return -1;
    for ( int l = strLen(s),n = pos; str[n]; n++ )
      if ( StrNCmp(str+n,s,l) == 0 ) return n;
 return -1;
}

DWORD MYRTLEXP MakeStringHash( CONSTSTR dptr,DWORD dsize )
  {  DWORD value;
     DWORD index;

     value = 0x238F13AFUL * dsize;
     for (index = 0; index < dsize; index++)
       value = (value + (dptr[index] << (index*5 % 24))) & 0x7FFFFFFFUL;

     value = (1103515243UL * value + 12345UL) & 0x7FFFFFFFUL;

 return value;
}

const char *MYRTLEXP GetEOLStr( WORD eol )
  {
    switch( eol ) {
      case    MK_WORD( '\n',0 ): return "\n";
      case    MK_WORD( '\r',0 ): return "\r";
      case MK_WORD( '\n','\r' ): return "\n\r";
      case MK_WORD( '\r','\n' ): return "\r\n";
      case       MK_WORD( 0,0 ): return "";
    }
 return "\r\n";
}

const char *MYRTLEXP GetTextEOLStr( WORD eol )
  {
    switch( eol ) {
      case    MK_WORD( '\n',0 ): return "\\n";
      case    MK_WORD( '\r',0 ): return "\\r";
      case MK_WORD( '\n','\r' ): return "\\n\\r";
      case MK_WORD( '\r','\n' ): return "\\r\\n";
      case       MK_WORD( 0,0 ): return "";
    }
 return "\\r\\n";
}

BOOL MYRTLEXP IsPrintChar( char n )
  {
#if defined(__QNX__) && !defined(CONSOLE_ONLY)
 return StrChr("\n\r\t",n) != NULL || (n >= ' ' && n != (char)0xFF);
#else
 return n != 0;
#endif
}
