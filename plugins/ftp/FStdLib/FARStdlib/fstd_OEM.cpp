#include <all_far.h>
#pragma hdrstop

#include "fstdlib.h"

pchar DECLSPEC StrFromOEMDup( CONSTSTR str,int num /*0|1*/ )
  {  static char nm[2][ MAX_PATH_SIZE ];
     if ( num < 0 || num >= (int)ARRAY_SIZE(nm) ) return (pchar)str;
     OemToCharBuff( str, nm[num], sizeof(nm[0])-1 );
 return nm[num];
}

pchar DECLSPEC StrToOEMDup( CONSTSTR str,int num /*0|1*/ )
  {  static char nm[2][ MAX_PATH_SIZE ];
     if ( num < 0 || num >= (int)ARRAY_SIZE(nm) ) return (pchar)str;
     CharToOemBuff( str, nm[num], sizeof(nm[0])-1 );
 return nm[num];
}

pchar DECLSPEC StrFromOEM( pchar str,int sz /*=-1*/ )
  {
     if ( sz == -1 )
       OemToChar( str, str );
      else
       OemToCharBuff( str, str, sz-1 );
 return str;
}

pchar DECLSPEC StrToOEM( pchar str,int sz /*=-1*/ )
  {
     if ( sz == -1 )
       CharToOem( str, str );
      else
       CharToOemBuff( str, str, sz-1 );

 return str;
}
