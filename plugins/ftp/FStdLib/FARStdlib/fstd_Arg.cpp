#include <all_far.h>
#pragma hdrstop

#include "fstdlib.h"

static char     *CT_Def_argv[] = { "",NULL };
static char    **CT_argv = CT_Def_argv;
static int       CT_argc = 1;
static BOOL      CT_CaseSensitive;

void DECLSPEC CTArgInit( int argc, char **argv,BOOL CaseSensitive )
  {
    CT_argc = argc;
    CT_argv = argv;
    CT_CaseSensitive = CaseSensitive;
#if defined(__QNX__)
    if ( StrChr(argv[0],SLASH_CHAR) == NULL )
      CT_argv[0] = StrDup( MakePathName(GetCurDir(),argv[0]).Text() );
#endif
}

char *DECLSPEC CTArgGet( int num )
  {
  return ( num < 0 || num >= CT_argc )?NULL:CT_argv[num];
}

char *DECLSPEC CTArgGetArg( int num )
  {
    for ( int n = 1; num >= 0 && n < CT_argc && CT_argv[n]; n++ )
      if (
#if defined(__HDOS__) || defined(__HWIN__)
        (CT_argv[n][0] == '-' || CT_argv[n][0] == '/')
#else
        CT_argv[n][0] == '-'
#endif
        ) continue;
       else {
        if (!num) return CT_argv[n];
        num--;
       }
  return NULL;
}

char *DECLSPEC CTArgGet( CONSTSTR name )
  {  int         cn = StrColCount(name,";"),
                 n,i,len;
     CONSTSTR m;

    for ( n = 1; n < CT_argc && CT_argv[n]; n++ )
      if (
#if defined(__HDOS__) || defined(__HWIN__)
        (CT_argv[n][0] == '-' || CT_argv[n][0] == '/')
#else
        CT_argv[n][0] == '-'
#endif
        ) for ( i = 1; i <= cn; i++ ) {
            m = StrGetCol(name,i,";");
            if ( StrCmp(CT_argv[n]+1,m,len=strLen(m),CT_CaseSensitive) == 0 &&
           CT_argv[n][1+len] == '=') return CT_argv[n]+1+len+1;
          }
 return NULL;
}

BOOL DECLSPEC CTArgCheck( CONSTSTR name )
  {  int      cn = StrColCount(name,";"),
              n,i;
     CONSTSTR m;

    for ( n = 1; n < CT_argc && CT_argv[n]; n++ )
      if (
#if defined(__HDOS__) || defined(__HWIN__)
           (CT_argv[n][0] == '-' || CT_argv[n][0] == '/')
#else
           CT_argv[n][0] == '-'
#endif
        ) for ( i = 1; i <= cn; i++ ) {
            m = StrGetCol(name,i,";");
            if ( StrCmp(CT_argv[n]+1,m,-1,CT_CaseSensitive) == 0 ) return TRUE;
          }

 return FALSE;
}
