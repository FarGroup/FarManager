#include <all_far.h>
#pragma hdrstop

#include "fstdlib.h"

BOOL DECLSPEC FP_InPattern( CONSTSTR patt,CONSTSTR nm )
  {  char c;
     char str[ FAR_MAX_PATHSIZE ];
     int  n;

    if ( !patt || !patt[0] || !nm ) return FALSE;

    do{
        for( n = 0; (c=*patt) != 0 && n < sizeof(str)-1 && *patt != ',' && *patt != ';'; patt++ )
          str[n++] = *patt;
        str[n] = 0;
        if ( *patt ) patt++;

        if ( strchr(str,'.') == NULL )
          StrCat( str,".*",sizeof(str) );

#if !defined(__USE_165_HEADER__)
        if ( FP_Info->CmpName(str,nm,TRUE) )
          return TRUE;
#else
        if ( StrCmp( str,nm,-1,FALSE ) == 0 )
          return TRUE;
#endif
    }while( c );

 return FALSE;
}
