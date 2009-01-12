#include <all_lib.h>
#pragma hdrstop
#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif

BOOL SayLogQuiet = FALSE;

static char buff[ 1000 ];

int MYRTLEXP_PT SayCL( const char *msg,... )
  {  int rc;

     if (!msg || SayLogQuiet)
       return 0;

     va_list a;
     va_start( a,msg );
       VSNprintf( buff, sizeof(buff), msg, a );
     va_end( a );

     FILELog( "%s",buff );
     rc = printf( "%s",buff );

     size_t len = strlen(buff);
     if ( len != 0 ) {
       --len;
       if ( buff[len] != '\r' && buff[len] != '\n' )
         printf( "\n" );
     }

 return rc;
}

int MYRTLEXP_PT _SayC( const char *msg,... )
  {  int rc;

     if (!msg || SayLogQuiet )
       return 0;

     va_list a;
     va_start( a,msg );
       VSNprintf( buff, sizeof(buff), msg, a );
     va_end( a );

     rc = printf( "%s",buff );

     size_t len = strlen(buff);
     if ( len != 0 ) {
       --len;
       if ( buff[len] != '\r' && buff[len] != '\n' )
         printf( "\n" );
     }

 return rc;
}


int MYRTLEXP_PT _SayL( const char *msg,... )
  {  int rc;

     if (!msg || SayLogQuiet)
       return -1;

     va_list a;
     va_start( a,msg );
       rc = FILELogV( msg,a );
     va_end( a );

 return rc;
}
