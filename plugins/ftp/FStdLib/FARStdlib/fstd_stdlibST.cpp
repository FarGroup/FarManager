#include <all_far.h>
#pragma hdrstop

#include "fstdlib.h"

char *DECLSPEC StrChr( CONSTSTR s, char ch )
  {
  return (char*)strchr( s,ch );
}

char *DECLSPEC StrChr( CONSTSTR str,CONSTSTR test )
  {  CONSTSTR m;

     if ( !str || !test )
       return NULL;

     for( ; *test; test++ )
       if ( (m=strchr(str,*test)) != NULL )
         return (char*)m;

 return NULL;
}


char *DECLSPEC StrRChr( CONSTSTR s, char ch )
  {
  return (char*)strrchr( s,ch );
}

int DECLSPEC strLen( CONSTSTR str )
  {
  return str ? (int)strlen(str) : 0;
}

char *DECLSPEC StrDup( CONSTSTR m )
  {  char *rc;

     if ( !m ) m = "";

     rc = (char*)_Alloc( strLen(m)+1 );
     if ( rc )
       StrCpy( rc,m );
 return rc;
}
