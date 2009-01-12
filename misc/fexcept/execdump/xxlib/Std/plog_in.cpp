#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif

int INProc::Counter = 0;

INProc::INProc( CONSTSTR nm,CONSTSTR s,... )
    : Name(nm)
  {  va_list  ap;
     char     str[500];
     DWORD    err = FIO_ERRORN;

    SNprintf( str,sizeof(str), "%*c%s(", Counter*2,' ',nm );

    if ( s ) {
      va_start( ap,s );
        StrCat( str,MessageV(s,ap),sizeof(str) );
      va_end(ap);
    }
    StrCat( str,") {",sizeof(str) );
    FILELog( str );

    Counter++;
    FIO_SETERRORN( err );
}

INProc::INProc( CONSTSTR nm )
    : Name(nm)
  {  DWORD    err;

    err = FIO_ERRORN;
      FILELog( "%*c%s(void) {", Counter*2,' ',nm );
      Counter++;
    FIO_SETERRORN( err );
}

INProc::~INProc()
  {  DWORD err = FIO_ERRORN;

    Counter--;

    FILELog( "%*c}<%s>", Counter*2,' ',Name );

    FIO_SETERRORN( err );
}
void INProc::Say( CONSTSTR s,... )
  {  va_list ap;
     char    str[500];
     DWORD   err = FIO_ERRORN;

    va_start( ap,s );
      SNprintf( str,sizeof(str), "%*c%s", Counter*2,' ', MessageV(s,ap) );
    va_end(ap);
    FILELog( str );
    FIO_SETERRORN( err );
}
