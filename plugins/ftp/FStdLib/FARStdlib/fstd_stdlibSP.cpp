#include <all_far.h>
#pragma hdrstop

#include "fstdlib.h"

int DECLSPEC_PT Sprintf( char *Buff,const char *Fmt,... )
  {  va_list a;
     int     res;

     va_start( a, Fmt );
     res = VSprintf( Buff,Fmt,a );
     va_end( a );
 return res;
}

int DECLSPEC_PT SNprintf( char *Buff,size_t cn,const char *Fmt,... )
  {  va_list a;
     size_t  res;

     if (cn) cn--;
     va_start( a, Fmt );
     res = VSNprintf( Buff,cn,Fmt,a );
     va_end( a );

     if ( cn && res >= cn ) Buff[cn] = 0;

 return res;
}

int DECLSPEC VSprintf( char *Buff,const char *Fmt,va_list arglist )
  {
 return vsprintf( Buff,Fmt,arglist );
}

int DECLSPEC VSNprintf( char *Buff,size_t cn,const char *Fmt,va_list arglist )
  {  size_t res;

     if (cn) cn--;
       res = vsnprintf( Buff,cn,Fmt,arglist );
     if ( cn && res >= cn ) Buff[cn] = 0;
 return res;
}
