#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif

char MYRTLEXP GetRunChar( void )
  {  static char say[] = "|/-\\";
     static int  counter = 0;
     const int ssay = 4;
 return say[ (counter++)%ssay ];
}
