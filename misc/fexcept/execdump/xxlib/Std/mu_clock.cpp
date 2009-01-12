#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif


#if defined(__SCWIN32__)
extern time_t time( time_t * );
#endif

time_t MYRTLEXP GetClock( void )
  {
 return time( NULL );
}

double MYRTLEXP CmpClock( time_t left,time_t right )
  {
  return difftime( left,right );
}

char *MYRTLEXP MakeClockString( char *buff,time_t cl,int maxw )
  {
     strftime( buff,(maxw==-1)?99:maxw,"%d/%m/%Y %H:%M:%S",localtime(&cl) );
 return buff;
}
