#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif

pchar MYRTLEXP StrAlloc( int len )
  {  char *s = (char*)_Alloc( len+1 );
     if (s) memset( s,0,len );
 return s;
}

void MYRTLEXP StrFree( CONSTSTR s )
  {
  _Del( (LPVOID)s );
}

pchar MYRTLEXP StrDup( CONSTSTR old, int newLen )
  {  int len = ((newLen==-1)?strLen(old):newLen)+1;
     char *s = StrAlloc( len );
     if ( !s ) return NULL;
     if ( old ) StrCpy( s,old,len );
 return s;
}

pchar MYRTLEXP StrRealloc( CONSTSTR old, int newLen )
  {  char *ss = StrAlloc( newLen );
     if ( !ss ) return NULL;
     StrCpy( ss,old,newLen );
     ss[newLen] = 0;
     StrFree( old );
 return ss;
}

pchar MYRTLEXP StrTrim( char *str )
  {  CONSTSTR m;

     if ( !str )
       return str;

     for( m = str; *m && isSpace(*m); m++ );

     if ( *m == 0 ) {
       str[0] = 0;
       return str;
     }

     int len;
     for( len = strLen( m )-1; len >= 0; len-- )
       if ( !isSpace(m[len]) ) {
         len++;
         break;
       }

     if ( len <= 0 ) {
       str[0] = 0;
       return str;
     }

     if ( m > str )
       memmove( str, m, len );
     str[len] = 0;

 return str;
}

pchar MYRTLEXP StrTrim( char *buff, CONSTSTR str, int bsz /*=-1*/ )
  {  CONSTSTR m;

     buff[0] = 0;

     if ( !str )
       return buff;

     for( m = str; *m && isSpace(*m); m++ );

     if ( *m == 0 )
       return buff;

     int len;
     for( len = strLen( m )-1; len >= 0; len-- )
       if ( !isSpace(m[len]) ) {
         len++;
         break;
       }

     if ( len <= 0 )
       return buff;

     StrCpy( buff, m, Min(bsz,len) );

 return buff;
}
