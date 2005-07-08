#include <all_far.h>
#pragma hdrstop

#include "fstdlib.h"

static FILE   *LogFile;
static char    LogFileName[ MAX_PATH_SIZE ];

//Callback called from HEX procedures to output printed data
static int RTL_CALLBACK idOutProc( const char *Format,... )
  {  va_list a;
     int     rc;

    va_start( a, Format );
      rc = vfprintf( LogFile,Format,a );
    va_end( a );

 return rc;
}

void DECLSPEC_PT __TrapLog( CONSTSTR msg,... )
  {  va_list a;
     char    str[ 1000 ];

     if (!msg) exit(1);

//Message
     va_start( a,msg );
       VSNprintf( str,sizeof(str),msg,a );
     va_end( a );

//Trap log
     do{
       if ( !HEX_Info ||
            !HEX_Info->StackWalkListCurrent )
         break;

      //Create log file
      StrCpy( LogFileName, FP_PluginStartPath, sizeof(LogFileName) );
      if ( LogFileName[0] ) StrCat( LogFileName, "\\", sizeof(LogFileName) );
      StrCat( LogFileName, "FStd_trap.log",  sizeof(LogFileName) );

      LogFile = fopen( LogFileName,"w+" );
      if ( !LogFile )
        break;

#if defined(FMSG_ALLINONE)
      FP_Info->Message( 0,
                        FMSG_ALLINONE,
                        NULL, (const char * const *)"Plugin assertion...\nGenerating trap log file...", 0, 0 );
#endif
      //Dump trap
      idOutProc( "==============================================================\n"
                 "FSDLib assertion: %s\n"
                 "==============================================================\n",
                 str );
      HEX_Info->StackWalkListCurrent( idOutProc, STK_DEFAULT, 2 );
      idOutProc( "==============================================================\n" );

      //Clear all used STUFF
      fclose( LogFile );
      HEX_Info->Cleanup();

      //Show error to user
#if defined(FMSG_MB_OK)
      FP_Info->Message( 0, FMSG_WARNING | FMSG_DOWN | FMSG_LEFTALIGN | FMSG_MB_OK | FMSG_ALLINONE,
                        NULL, (const char * const *)str, 0, 0 );
#endif
      TerminateProcess( GetCurrentProcess(),0 );

     }while(0);

     __WinAbort( str );
}
