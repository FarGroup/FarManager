#include <windows.h>
#include <tchar.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#pragma hdrstop

// FAR STUFF
#define _FAR_USE_FARFINDDATA
#include "plugin.hpp"
#include "fexcept.h"

//HEX DLL STUFF
#include "ExcDump.h"

#define  MAX_PATH_SIZE 256

#if defined(__BORLANDC__)
  #define DECL extern "C" __declspec(dllexport)
#else
  #define DECL extern "C"
#endif

typedef ULONG (WINAPI *MapFileAndCheckSumA_t)( LPSTR,LPDWORD,LPDWORD );

//Local DATA
static       PHEX_DumpInfo      HInfo;
static       FILE              *LogFile;
const struct PluginStartupInfo *FP_Info;
static       char               LocalPath[ MAX_PATH_SIZE ];
static       char               LogFileName[ MAX_PATH_SIZE ];
static       char               TrapFileName[ MAX_PATH_SIZE ];
static       char               Path2Far[ MAX_PATH_SIZE ];
static       HANDLE             FarScreen_handle;
static       HMODULE            ThisModule;
static       char               TrapFileNameMsg[ 2*NM ];
static       char               LogFileNameMsg[ 2*NM ];
static       HANDLE             InFile;
static       DWORD              FarSize;
static       DWORD              FarVer;
static       HANDLE             FarAddr;
static       DWORD              Checksum1;
static       DWORD              Checksum2;
MapFileAndCheckSumA_t           pCheckSum;


//Try to open log file.
//Clearattributes and try again if fail.
static BOOL LOpen( BOOL append )
  {  const char *WMode = append ? "at" : "wt";

     LogFile = fopen( LogFileName,WMode );
     if ( LogFile )
       return TRUE;

     if ( SetFileAttributesA( LogFileName,0 ) &&
          (LogFile=fopen( LogFileName,WMode )) != NULL )
       return TRUE;

 return FALSE;
}

//Callback called from HEX procedures to output printed data
static int RTL_CALLBACK idOutProc( const char *Format,... )
  {  va_list a;
     int     rc;

    va_start( a, Format );
      rc = vfprintf( LogFile,Format,a );
    va_end( a );

 return rc;
}

template <unsigned MaxLength> char *TruncStr( char *Str )
{
  if ( !Str ) return NULL;

  int Length;

  if ( (Length=strlen(Str)) > MaxLength )
  {
    if (MaxLength>3)
    {
      memmove(Str+3,Str+Length-MaxLength+3,MaxLength);
      memcpy(Str,"...",3);
    }
    Str[MaxLength]=0;
  }

  return Str;
}

template <unsigned MaxLength> char *TruncPathStr( char *Str )
  {  int nLength = Str ? strlen(Str) : 0;

     if ( !nLength ) return NULL;

     if ( nLength <= MaxLength)
       return Str;

     char *lpStart = NULL;

     if ( *Str && (Str[1] == ':') && (Str[2] == '\\') )
        lpStart = Str+3;
     else
     {
       if ( (Str[0] == '\\') && (Str[1] == '\\') )
       {
         if ( (lpStart = strchr (Str+2, '\\')) != NULL )
           if ( (lpStart = strchr (lpStart+1, '\\')) != NULL )
             lpStart++;
       }
     }

     if ( !lpStart || (lpStart-Str > MaxLength-5) )
       return TruncStr<MaxLength>( Str );

     char *lpInPos = lpStart+3+(nLength-MaxLength);
     strcpy (lpStart+3, lpInPos);
     memcpy (lpStart, "...", 3);

  return Str;
}

#define TRUNC( s ) TruncPathStr<(unsigned)64>( s )

//Show FAR message if possible
void SError( const TCHAR *msg, DWORD tp = 0 )
  {
    if ( FP_Info && FP_Info->Message )
      FP_Info->Message( 0, tp | FMSG_WARNING | FMSG_ALLINONE | FMSG_MB_OK, NULL, (const TCHAR * const *)msg, 0, 0 );
}

DECL BOOL WINAPI ExceptionProcINT( EXCEPTION_POINTERS *xInfo,
                                   const struct PLUGINRECORD *Module,
                                   BOOL Nested )
  {  static char     Path[ MAX_PATH_SIZE ];
     static TCHAR    str[ 1000 ];
     static char    *m;
     static HMODULE  md;
     BOOL            isFARTrap = !Module ||
#ifndef UNICODE
                                 Module->FindData.cFileName[0] == 0;
#else
                                 Module->ModuleName == NULL || *(Module->ModuleName) == 0;
#endif

     GetModuleFileNameA( NULL,Path2Far,sizeof(Path2Far));

//Plugin path
     if ( !isFARTrap ) {
#ifndef UNICODE
       strncpy( TrapFileName, Module->FindData.cFileName, sizeof(TrapFileName)-1 );
#else
       WideCharToMultiByte(CP_OEMCP,0,Module->ModuleName,-1,TrapFileName,sizeof(TrapFileName)-1,NULL,NULL);
#endif
       TrapFileName[ sizeof(TrapFileName)-1 ] = 0;
     } else
       TrapFileName[ GetModuleFileNameA( NULL,TrapFileName,sizeof(TrapFileName) ) ] = 0;

     md = GetModuleHandleA( TrapFileName );
     if ( md )
       LocalPath[ GetModuleFileNameA( md,TrapFileName,sizeof(TrapFileName) ) ] = 0;

     LocalPath[ GetModuleFileNameA( md,LocalPath,sizeof(LocalPath) ) ] = 0;
     m = strrchr( LocalPath,'\\' );
     if (m) *m = 0;

     TRUNC( strcpy(TrapFileNameMsg,TrapFileName) );

//Load HEX
     do{
       //Accesible on paths
       md = LoadLibraryA( "ExcDump.dll" );
       if ( md ) break;

       //In fexcept directory
       Path[ GetModuleFileNameA( ThisModule,Path,sizeof(Path) ) ] = 0;
       m = strrchr( Path,'\\' ); if (m) *m = 0;
       strcat( Path,"\\ExcDump.dll" );
       md = LoadLibraryA( Path );
       if ( md ) break;

       //In plugin directory
       strcpy( Path, LocalPath );
       strcat( Path,"\\ExcDump.dll" );
       md = LoadLibraryA( Path );
       if ( md ) break;

       //In FAR directory
       Path[ GetModuleFileNameA( NULL,Path,sizeof(Path) ) ] = 0;
       m = strrchr( Path,'\\' ); if (m) *m = 0;
       strcat( Path,"\\ExcDump.dll" );
       md = LoadLibraryA( Path );
       if ( md ) break;

       //Somewhere else ?

       //...

     }while( 0 );
     if (!md) {
       SError( _T("Error processing exception...\nError loading ExcDump library"), FMSG_ERRORTYPE );
       return FALSE;
     }

     HT_QueryInterface_t p = (HT_QueryInterface_t)GetProcAddress( md,"HEX_QueryInterface" );
     if ( !p || (HInfo=p()) == NULL ) {
       SError( _T("Error processing exception...\nError querying ExcDump structures"), FMSG_ERRORTYPE );
       FreeLibrary( md );
       return FALSE;
     }

     if ( HInfo->Version != HEX_VERSION ) {
       _stprintf( str,
                _T("Error processing exception...\n")
                _T("ExcDump has a wrong version.\n")
                _T("Version %02d.%02d.%04d needed, but %02d.%02d.%04d found."),
               (HEX_VERSION>>24)&0xFF, (HEX_VERSION>>16)&0xFF, (HEX_VERSION)&0xFFFF,
               (HInfo->Version>>24)&0xFF, (HInfo->Version>>16)&0xFF, (HInfo->Version)&0xFFFF );
       SError( str, FMSG_ERRORTYPE );
       FreeLibrary( md );
       return FALSE;
     }

//Create log file
     do{
       LogFile = NULL;

       //FAR directory
       LogFileName[ GetModuleFileNameA(NULL,LogFileName,sizeof(LogFileName)) ] = 0;
       m = strrchr(LogFileName,'\\');
       if (m) *m = 0;
       strcat( LogFileName, "\\FStd_trap.log" );
       if ( LOpen( Nested ) )
         break;

       //Plugin directory
       if ( !isFARTrap ) {
         strcpy( LogFileName, LocalPath );
         strcat( LogFileName, "\\FStd_trap.log" );
         if ( LOpen( Nested ) )
           break;
        }

       //Somewhere else ?
       //...

     }while(0);
     if ( !LogFile ) {
       SError( _T("Error processing exception...\nCan not create log file"), FMSG_ERRORTYPE );
       return FALSE;
     }

     setbuf( LogFile,NULL );

     TRUNC( strcpy(LogFileNameMsg,LogFileName) );

//Dump trap
     idOutProc( "Exception in [%s] %s.\n"
                "==============================================================\n",
                TrapFileName,
                (isFARTrap ? "FAR itself" : "plugin") );

     idOutProc( "FAR.EXE information:\n");

//Try Get Version Info
     if ( FP_Info && FP_Info->AdvControl ) {
       FP_Info->AdvControl(FP_Info->ModuleNumber, ACTL_GETFARVERSION, &FarVer);
       idOutProc( " Version info: %d.%d.%d\n",HIBYTE(LOWORD(FarVer)),LOBYTE(LOWORD(FarVer)),HIWORD(FarVer));
     }

//Try Get FileSize
     InFile = CreateFileA( Path2Far, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0);
     if (InFile != INVALID_HANDLE_VALUE) {
       FarSize = GetFileSize( InFile, NULL);
       idOutProc( "    File size: %u bytes\n",FarSize);
       CloseHandle(InFile);
     }

//Try Get TimeStamp
     FarAddr = GetModuleHandle( NULL );
     PIMAGE_NT_HEADERS fh;
     fh = (PIMAGE_NT_HEADERS)FarAddr;
     fh = (PIMAGE_NT_HEADERS)((DWORD)fh + ((PIMAGE_DOS_HEADER)fh)->e_lfanew);

     time_t tm = (time_t)( fh->FileHeader.TimeDateStamp );
     char   buff[100];
     strftime( buff, sizeof(buff),"%d.%m.%Y %H:%M:%S",localtime(&tm) );
     idOutProc( "   Time stamp: %s\n", buff);

//Try Get CheckSum
//Load imagehlp.dll dynamically because in Win95 this dll not present
     HINSTANCE h_imagehlp = LoadLibraryA( "imagehlp.dll" );
     if ( h_imagehlp ) {
       pCheckSum = (MapFileAndCheckSumA_t) GetProcAddress(h_imagehlp, "MapFileAndCheckSumA");

       //CHECKSUM_SUCCESS = 0
       if ( pCheckSum( Path2Far, &Checksum1, &Checksum2) == 0 )
         idOutProc( " Hdr checksum: 0x%08lX (computed: 0x%08lX)\n", Checksum1, Checksum2);
     }

//Exception info
     idOutProc( "==============================================================\n" );

     HInfo->ExceptionInfo( idOutProc, xInfo->ExceptionRecord, xInfo->ContextRecord );

     idOutProc( "==============================================================\n" );

//Check recurse
     if ( Nested ) {
       SError( _T("Nested exceptions...\n")
               _T("Exceptions occur inside exception handler.\n")
               _T("We are sorry. FAR will be terminated...") );
       TerminateProcess( GetCurrentProcess(),0 );
     }

     FarScreen_handle = FP_Info->SaveScreen(0,0,-1,-1);

//Say "waiting"
     if ( FP_Info && FP_Info->Message )
       FP_Info->Message( 0,
                         FMSG_WARNING | FMSG_ALLINONE,
                         NULL, (const TCHAR * const *)_T("Trap log\nGenerating trap log file..."),
                         0, 0 );

     HInfo->StackWalkListContext( idOutProc, xInfo->ContextRecord, STK_FULL, 0 );

//Clear all used STUFF
     fclose( LogFile );
     HInfo->Cleanup();

//Ask user to terminate FAR
     if ( isFARTrap )
       _stprintf( str,
                _T("Exception error...\n")
                _T("FAR itself executes an error and will be terminated.\n")
                _T("The trap log file has been saved to file:\n")
#ifndef UNICODE
                _T("  %s\n")
#else
                _T("  %S\n")
#endif
                _T(""), LogFileNameMsg );
      else
       _stprintf( str,
                _T("Exception error...\n")
                _T("Plugin:\n")
#ifndef UNICODE
                _T("  %s\n")
#else
                _T("  %S\n")
#endif
                _T("executes an error and will be unloaded.\n")
                _T("The trap log file has been saved to file:\n")
#ifndef UNICODE
                _T("  %s\n")
#else
                _T("  %S\n")
#endif
                _T("\x1\n")
                _T("Do you want to terminate FAR itself ? (recommended)")
                _T(""),
                TrapFileNameMsg, LogFileNameMsg );

     if ( !FP_Info ||
          !FP_Info->Message ||
          FP_Info->Message( 0, FMSG_DOWN | FMSG_LEFTALIGN | FMSG_ALLINONE | (isFARTrap ? FMSG_MB_OK : FMSG_MB_YESNO ),
                            NULL, (const TCHAR * const *)str, 0, 0 ) == 0 ) {
       TerminateProcess( GetCurrentProcess(),0 );
     }

 FP_Info->RestoreScreen(FarScreen_handle);
 return TRUE;
}

//FAR exception handler replacement
DECL BOOL WINAPI ExceptionProc( EXCEPTION_POINTERS *xInfo,
                                const struct PLUGINRECORD *Module,
                                const struct PluginStartupInfo *Info,
                                LPDWORD /*Result*/ )
  {  static int inside = 0;

     FP_Info = Info;
     if ( !xInfo )
       return FALSE;


     BOOL rc;

     if ( inside ) {
         rc = ExceptionProcINT( xInfo, Module, inside );
     } else {
       inside++;
         rc = ExceptionProcINT( xInfo, Module, 0 );
       inside--;
     }

 return rc;
}

#if defined(__BORLANDC__)
BOOL WINAPI DllEntryPoint( HINSTANCE hinst, DWORD reason, LPVOID /*ptr*/ )
  {
    if ( reason == DLL_PROCESS_ATTACH )
      ThisModule = (HMODULE)hinst;
 return TRUE;
}
#else
#if defined(_MSC_VER)
BOOL WINAPI DllMain( HINSTANCE hinst, DWORD reason, LPVOID /*ptr*/ )
  {
    if ( reason == DLL_PROCESS_ATTACH )
      ThisModule = (HMODULE)hinst;
 return TRUE;
}
#else
  #error "Define plugin DLL entry point procedure for your  compiller"
#endif
#endif
