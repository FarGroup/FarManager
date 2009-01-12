#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif

int MYRTLEXP BufferedPrintfV( char *Buff,int bSize,HPrintProc_t print,CONSTSTR Fmt,va_list arglist )
  {  char str[ 1000 ];
     int  rc;

     Assert( Buff && bSize > 1 );

     rc = Abs( VSNprintf( str,sizeof(str)-1,Fmt,arglist ) );
     str[rc] = 0;
     if ( !rc ) return 0;

    //Lines
     char *m,*dest;
     int   n;

     n     = strLen(Buff);
     dest  = Buff + n;
     m     = str;
     bSize--;

     for( ; *m; m++ ) {

       if ( n == bSize || *m == '\n' || *m == '\r' ) {
         *dest = 0;
         print( "%s\n",Buff );
         dest = Buff;
         n = 0;
         continue;
       }

       *dest++ = *m;
       n++;
     }
     *dest = 0;

 return rc;
}

int MYRTLEXP_PT BufferedPrintf( char *Buff,int bSize,HPrintProc_t print,CONSTSTR Fmt,... )
  {  va_list argptr;
     int     rc;

    va_start(argptr,Fmt);
      rc = BufferedPrintfV(Buff,bSize,print,Fmt,argptr);
    va_end(argptr);
 return rc;
}
