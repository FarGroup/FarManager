#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif

/*******************************************************************
   DATA
 *******************************************************************/
static const char* CT_Def_argv[] = { "",NULL };
static BOOL      CT_CaseSensitive;
static BOOL      CT_SelfArgs = FALSE;

static const char** CT_argv = CT_Def_argv;
static int    CT_argc = 1;

/*******************************************************************
   LOCAL
 *******************************************************************/
static void DeleteArgs( void )
  {
     if ( !CT_SelfArgs )
       return;

     for( int n = 0; n < CT_argc; n++ )
       StrFree( CT_argv[n] );
     delete[] CT_argv;

     CT_argv     = CT_Def_argv;
     CT_argc     = 1;
     CT_SelfArgs = FALSE;
}

static AbortProc oldAbort;
static void RTL_CALLBACK idDelArgs( void )
  {
     DeleteArgs();
     if (oldAbort) oldAbort();
}

DECL_GLOBALCALL_PROC( SetArgs )
  {
     oldAbort = AtExit( idDelArgs );

#if defined(__VCL__)
     CT_argc = ParamCount()+1;
     CT_argv = new pchar[ CT_argc+1 ];
     int n;
     for( n = 0; n < CT_argc; n++ )
       CT_argv[n] = StrDup( ParamStr(n).c_str() );
     CT_argv[n] = NULL;

     CT_SelfArgs = TRUE;
#endif

 return 0;
}

/*******************************************************************
   INTF
 *******************************************************************/
int MYRTLEXP CTArgCount( void )
  {
  return CT_argc;
}

void MYRTLEXP CTArgInit( CONSTSTR Args,BOOL CaseSensitive )
  {
    DeleteArgs();
    CT_CaseSensitive = CaseSensitive;
    //??
}

void MYRTLEXP CTArgInit( int argc, char **argv,BOOL CaseSensitive )
  {
    DeleteArgs();
    CT_CaseSensitive = CaseSensitive;
    CT_SelfArgs      = TRUE;

    CT_argc = argc;
    CT_argv = new const char*[ CT_argc+1 ];
    int n;

    if ( StrChr(argv[0],SLASH_CHAR) == NULL )
      CT_argv[0] = StrDup( MakePathName(GetCurDir(),argv[0]).Text() );
     else
      CT_argv[0] = StrDup( argv[0] );

    for( n = 1; n < CT_argc; n++ )
      CT_argv[n] = StrDup( argv[n] );

    CT_argv[n] = NULL;
}

const char* MYRTLEXP CTArgGet( int num )
  {
  return ( num < 0 || num >= CT_argc ) ? NULL : CT_argv[num];
}

const char* MYRTLEXP CTArgGetArg( int num )
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

const char* MYRTLEXP CTArgGet( CONSTSTR name )
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

BOOL MYRTLEXP CTArgCheck( CONSTSTR name )
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

pchar MYRTLEXP CTGetArgPathName( CONSTSTR arg, CONSTSTR def, char *str, int sz )
  {  CONSTSTR m;

     if ( (m=CTArgGet(arg)) == NULL )
       StrCpy( str, MakePathName( GetStartupDir(NULL), def ).c_str(), sz );
      else {
#if defined(__HWIN32__)
       ExpandEnvironmentStrings( m, str, sz );
#else

//??Def expand on QNX

       StrCpy( str, m, sz );
#endif

       if ( !IsAbsolutePath(str) )
         StrCpy( str, MakeStartLocalPath(str).c_str(), sz );
     }

 return str;
}
