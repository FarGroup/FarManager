#include <all_far.h>
#pragma hdrstop

#include "fstdlib.h"

int DECLSPEC StrPosStr( const char *str,const char *s,int pos )
  {
    if ( !str || !s || !*s || pos < 0 || pos >= (int)strLen(str) ) return -1;
    for ( int l = strLen(s),n = pos; str[n]; n++ )
      if ( StrNCmp(str+n,s,l) == 0 ) return n;
 return -1;
}
