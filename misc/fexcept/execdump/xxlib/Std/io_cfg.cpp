#include <all_lib.h>
#pragma hdrstop
#if defined(__MYPACKAGE__)
#pragma package(smart_init)
#endif

char *CTCfgOverride = NULL;

char *MYRTLEXP MakeCfgName( char *name )
  {  static char str[ MAX_PATH_SIZE+1 ];
     char *m;

    do{
     if ( CTCfgOverride )
       StrCpy( str,CTCfgOverride,sizeof(str) );
      else
     if ( (m=getenv( CT_ENV_CFG )) != NULL || (m=getenv( CT_ENV_CFG1 )) != NULL )
       StrCpy( str,m,sizeof(str) );
      else {
#if defined(__QNX__)
       if ( (m=getenv( "HOME" )) == NULL )
         StrCpy( str,SLASH_STR,sizeof(str) );
        else
         StrCpy( str,m,sizeof(str) );
#else
#if defined( __HDOS__ )
       StrCpy( str,_argv[0],sizeof(str) );
       if ( StrRChr(str,SLASH_CHAR) != NULL )
         *StrRChr(str,SLASH_CHAR) = 0;
        else
         str[0] = 0;
#else
#if defined(__HWIN__ )
    str[ GetModuleFileName(NULL,str,sizeof(str)) ] = 0;
    *StrRChr( str,SLASH_CHAR ) = 0;
#endif
#endif
#endif
      }
    }while(0);

    AddLastSlash( str );
    StrCat( str,name,sizeof(str) );

return str;
}
