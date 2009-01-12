#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <string.h>
#ifdef __GNUC__
#include <excpt.h>
#endif
#pragma hdrstop

//HEX DLL STUFF
#include "../ExcDump.h"

#define  MAX_PATH_SIZE 256

//Local DATA
static PCHEX_DumpInfo HInfo;
static FILE          *LogFile;
static char           LocalPath[ MAX_PATH_SIZE ];
static char           LogFileName[ MAX_PATH_SIZE ];

//Callback called from HEX procedures to output printed data
static int RTL_CALLBACK idOutProc( const char *Format,... )
  {  va_list a;
     int     rc;

    va_start( a, Format );
      rc = vfprintf( LogFile,Format,a );
    va_end( a );

 return rc;
}

//Exception handler
DWORD WINAPI idException( EXCEPTION_POINTERS *xInfo )
  {  static char *m;
     static HMODULE md;
     static char Path[ MAX_PATH_SIZE ];

//Load HEX
     do{
       md = LoadLibrary( "ExcDump.dll" );
       if ( md ) break;

       strcpy( Path,LocalPath );
       strcat( Path,"\\ExcDump.dll" );
       md = LoadLibrary( Path );
       if ( md ) break;

     }while( 0 );
     if (!md) {
       printf( "Can not load ExcDump library\n" );
       return EXCEPTION_EXECUTE_HANDLER;
     }

     HT_QueryInterface_t p = (HT_QueryInterface_t)GetProcAddress( md,"HEX_QueryInterface" );
     if ( !p || (HInfo=p()) == NULL ) {
       FreeLibrary( md );
       printf( "Can not find desired exports in ExcDump library\n" );
       return EXCEPTION_EXECUTE_HANDLER;
     }

     if ( HInfo->Version != HEX_VERSION ) {
       FreeLibrary( md );
       printf( "Can not use ExcDump with wrong version. Expect %02d.%02d.%04d, found %02d.%02d.%04d\n",
               (HEX_VERSION>>24)&0xFF, (HEX_VERSION>>16)&0xFF, (HEX_VERSION)&0xFFFF,
               (HInfo->Version>>24)&0xFF, (HInfo->Version>>16)&0xFF, (HInfo->Version)&0xFFFF );
       return EXCEPTION_EXECUTE_HANDLER;
     }

//Create log file
     LogFileName[ GetModuleFileName(NULL,LogFileName,sizeof(LogFileName)) ] = 0;
     m = strrchr(LogFileName,'\\');
     if (m) *m = 0;
     strcat( LogFileName, "\\FStd_trap.log" );

     LogFile = fopen( LogFileName,"wt" );
     if ( !LogFile ) {
       printf( "Can not create [%s] log file\n",LogFileName );
       return EXCEPTION_EXECUTE_HANDLER;
     }

//Dump trap
     HInfo->SetLogProc( idOutProc );
     idOutProc( "Exception\n"
                "==============================================================\n" );
     HInfo->ExceptionInfo( idOutProc, xInfo->ExceptionRecord, xInfo->ContextRecord );
     idOutProc( "==============================================================\n" );
     HInfo->StackWalkListContext( idOutProc, xInfo->ContextRecord, STK_DEFAULT, 0 );

//Clear all used STUFF
     fclose( LogFile );
     HInfo->Cleanup();

 return EXCEPTION_EXECUTE_HANDLER;
}

static void exception_occured(void)
{
   printf( "Exception happen! Log file \"FStd_trap.log\" generated.\n" );
}

#ifdef __GNUC__
EXCEPTION_DISPOSITION gccHandler(struct _EXCEPTION_RECORD* er, void*, 
                                 struct _CONTEXT* ctx, void*)
{
  EXCEPTION_POINTERS ep;
  ep.ExceptionRecord = er;
  ep.ContextRecord = ctx;
  idException(&ep);
  exception_occured();
  ExitProcess(0);
  return ExceptionContinueExecution;
}
#endif


int Test( void )
  {
    printf( "Exception test.\nSelect test type:\n"
            "  1. Generate call by zero pointer (call[0])\n"
            "  2. Generate math overflow (value / 0)\n"
            "  3. Generate stack overrun\n"
            "  4. Generate float math overflow (value / 0)\n"
            "--------------------------------------------\n"
            "  Q. Quit\n"
            "Enter your choice: " );

    do{
      switch( _getch() ) {
        case '1': { void (*p)() = NULL;
                    p();                     //Call empty pointer
              } break;

        case '2': { int v = 10;
                    return (v-5) / (v-10);   //Divide by zero
              }

        case '3': { char str[ 10 ];
                    memset( str, 's', 30 );  //Clears return address, traps after RET
                    return 0;
              }

        case '4': { double v = 0.1;
                    return (int)(v / (v/v - 1));    //Float divide by zero
              }
        case 'q':
        case 'Q': exit(1);
      }
      printf( "\b" );
    }while(1);
}

void Proc2( void )
  {
    Test();
}

void Proc1( void )
  {
    Proc2();
}

void Proc( void )
  {
    Proc1();
}
int main( void )
  {  char buffer[ 1024 ];  //Buffer for increase ESP pointer and guard __try variable from overrun

//Set local path to current executable
     buffer[ GetModuleFileName( NULL,buffer,sizeof(buffer) ) ] = 0;
     char *m = strrchr( buffer,'\\' );
     if (m) *m = 0;
     strcpy( LocalPath, buffer );

//Execute test excception
#ifndef __GNUC__
     __try
#else
     __try1(gccHandler);
#endif
     {
        Proc();
     }
#ifndef __GNUC__
     __except( idException( GetExceptionInformation() ) ) {
       exception_occured();
     }
#else
    __except1;
#endif

     return 0;
}
