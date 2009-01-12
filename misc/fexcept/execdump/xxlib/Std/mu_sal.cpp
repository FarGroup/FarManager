#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif


char *MYRTLEXP StrMakeAlign( char *dest,CONSTSTR source,int maxWidth, talTypes align, talTypes *over )
  {  int      n,nm;
     talTypes overflow = talNone;
     int      len = strLen( source );

    switch( align )
      {
     case  talNone:
     case  talLeft: if ( len > maxWidth )
                      {  overflow = talRight;
                         StrCpy( dest,source,maxWidth );
                         dest[maxWidth] = 0;
                      }
                     else
                      {  StrCpy( dest,source );
                         for ( nm = len; nm < maxWidth; nm++ ) dest[nm] = ' ';
                         dest[nm] = 0;
                      }
                 break;
     case talRight: if ( len > maxWidth )
                      {  overflow = talLeft;
                         StrCpy( dest,source+len-maxWidth,maxWidth );
                         dest[maxWidth] = 0;
                      }
                     else
                      {  for ( nm = maxWidth-len; nm >= 0; nm-- ) dest[nm] = ' ';
                         StrCpy( dest+maxWidth-len,source );
                      }
                 break;
    case talCenter: if ( len > maxWidth )
                      {  overflow = talRight;
                         StrCpy( dest,source,maxWidth );
                         dest[maxWidth] = 0;
                      }
                     else
                      {  n = maxWidth-len;
                         for ( nm = 0; nm < n/2; n--,nm++ ) dest[nm] = ' ';
                         StrCpy( dest+nm,source );
                         for ( nm += len; nm < maxWidth; nm++ ) dest[nm] = ' ';
                         dest[maxWidth] = 0;
                      }
                 break;
      }
    if ( over ) *over = overflow;
 return dest;
}
