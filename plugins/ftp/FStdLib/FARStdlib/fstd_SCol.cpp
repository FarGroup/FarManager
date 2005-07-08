#include <all_far.h>
#pragma hdrstop

#include "fstdlib.h"

int DECLSPEC StrColCount( CONSTSTR str,CONSTSTR seps )
  {  int res = 1;

     if ( !str || !seps || !str[0] || !seps[0] ) return 0;

     for ( int n = 0; str[n]; n++ )
       if ( StrChr((char*)seps,str[n]) != NULL )
         res++;

  return res;
}

CONSTSTR DECLSPEC StrGetCol( CONSTSTR str,int number,CONSTSTR seps )
  {  static char resStr[ MAX_PATH_SIZE ];
     int res;
     int num;

     for ( res = 1; *str && res < number; str++ )
       if ( StrChr((char*)seps,*str) != NULL ) res++;

     resStr[num=0] = 0;
     if ( res == number )
       for( ; *str && StrChr((char*)seps,*str) == NULL; str++ )
         resStr[num++] = *str;
     resStr[num] = 0;

 return resStr;
}
