#include <windows.h>
#include <tchar.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <locale.h>

// FAR STUFF
#include <plugin.hpp>
#include "fexcept.h"

//HEX DLL STUFF
#include "execdump/ExcDump.h"

#define  MAX_PATH_SIZE 256

#if defined(__BORLANDC__)
  #define DECL extern "C" __declspec(dllexport)
#else
  #define DECL extern "C"
#endif

// {2DD20BEA-C58F-4933-815E-A1AA9B047A86}
static const GUID MainGuid =
{ 0x2dd20bea, 0xc58f, 0x4933, { 0x81, 0x5e, 0xa1, 0xaa, 0x9b, 0x4, 0x7a, 0x86 } };


typedef ULONG (WINAPI *MAPFILEANDCHECKSUM)( LPTSTR,LPDWORD,LPDWORD );

//Local DATA
static       PCHEX_DumpInfo     HInfo;
static       FILE              *LogFile;
const struct PluginStartupInfo *FP_Info;
static       TCHAR              LocalPath[ MAX_PATH_SIZE ];
static       TCHAR              LogFileName[ MAX_PATH_SIZE ];
static       TCHAR              LogTime[32];
static       TCHAR              TrapFileName[ MAX_PATH_SIZE ];
static       TCHAR              Path2Far[ MAX_PATH_SIZE ];
static       HANDLE             FarScreen_handle;
static       HMODULE            ThisModule;
static       TCHAR              TrapFileNameMsg[ 2*MAX_PATH ];
static       TCHAR              LogFileNameMsg[ 2*MAX_PATH ];
static       HANDLE             InFile;
static       DWORD              FarSize;
static       VersionInfo        FarVer;
static       HANDLE             FarAddr;
static       DWORD              Checksum1;
static       DWORD              Checksum2;
MAPFILEANDCHECKSUM              pCheckSum;
//Try to open log file.
//Clearattributes and try again if fail.
static BOOL LOpen( BOOL append )
  {  const TCHAR *WMode = (append ? _T("at") : _T("wt"));

     LogFile = _tfopen( LogFileName,WMode );
     if ( LogFile )
       return TRUE;

     if ( SetFileAttributes( LogFileName,0 ) &&
          (LogFile=_tfopen( LogFileName,WMode )) != NULL )
       return TRUE;

 return FALSE;
}

//Callback called from HEX procedures to output printed data
static int RTL_CALLBACK idOutProc( const TCHAR *Format,... )
  {  va_list a;
     int     rc;

    va_start( a, Format );
      rc = _vftprintf( LogFile,Format,a );
    va_end( a );

 return rc;
}
static int RTL_CALLBACK idOutProcA( const char *Format,... )
  {  va_list a;
     int     rc;

    va_start( a, Format );
      rc = vfprintf( LogFile,Format,a );
    va_end( a );

 return rc;
}

//Show FAR message if possible
void SError( const TCHAR *msg, DWORD tp = 0 )
  {
    if ( FP_Info && FP_Info->Message )
      FP_Info->Message( &MainGuid, nullptr, tp | FMSG_WARNING | FMSG_ALLINONE | FMSG_MB_OK, NULL, (const TCHAR * const *)msg, 0, 0 );
}

DECL BOOL WINAPI ExceptionProcINT( EXCEPTION_POINTERS *xInfo,
                                   const struct PLUGINRECORD *Module,
                                   BOOL Nested )
  {  static TCHAR     Path[ MAX_PATH_SIZE ];
     static TCHAR    str[ 1000 ];
     static TCHAR    *m;
     static HMODULE  md;
     BOOL            isFARTrap = !Module || Module->ModuleName == NULL || *(Module->ModuleName) == 0;

     _tsetlocale(LC_ALL,_T(""));

     GetModuleFileName( NULL,Path2Far,ArraySize(Path2Far));

//Plugin path
     if ( !isFARTrap ) {
       _tcsncpy( TrapFileName, Module->ModuleName, ArraySize(TrapFileName)-1 );
       TrapFileName[ ArraySize(TrapFileName)-1 ] = 0;
     } else
       TrapFileName[ GetModuleFileName( NULL,TrapFileName,ArraySize(TrapFileName) ) ] = 0;

     md = GetModuleHandle( TrapFileName );
     if ( md )
       LocalPath[ GetModuleFileName( md,TrapFileName,ArraySize(TrapFileName) ) ] = 0;

     LocalPath[ GetModuleFileName( md,LocalPath,ArraySize(LocalPath) ) ] = 0;
     m = _tcsrchr( LocalPath,_T('\\') );
     if (m) *m = 0;

      _tcscpy(TrapFileNameMsg,TrapFileName);

//Load HEX
     do{
       //Accesible on paths
       md = LoadLibrary( _T("ExcDump.dll") );
       if ( md ) break;

       //In fexcept directory
       Path[ GetModuleFileName( ThisModule,Path,ArraySize(Path) ) ] = 0;
       m = _tcsrchr( Path,_T('\\') ); if (m) *m = 0;
       _tcscat( Path,_T("\\ExcDump.dll") );
       md = LoadLibrary( Path );
       if ( md ) break;

       //In plugin directory
       _tcscpy( Path, LocalPath );
       _tcscat( Path,_T("\\ExcDump.dll") );
       md = LoadLibrary( Path );
       if ( md ) break;

       //In FAR directory
       Path[ GetModuleFileName( NULL,Path,ArraySize(Path) ) ] = 0;
       m = _tcsrchr( Path,_T('\\') ); if (m) *m = 0;
       _tcscat( Path,_T("\\ExcDump.dll") );
       md = LoadLibrary( Path );
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
               (int)(HInfo->Version>>24)&0xFF, (int)(HInfo->Version>>16)&0xFF, (int)(HInfo->Version)&0xFFFF );
       SError( str, FMSG_ERRORTYPE );
       FreeLibrary( md );
       return FALSE;
     }

	//Create log file
	LogFile = NULL;

	// %localprofile%
	if(!GetEnvironmentVariable(_T("farlocalprofile"), LogFileName, ArraySize(LogFileName)))
	{
		// %farhome%
		LogFileName[ GetModuleFileName(NULL,LogFileName,ArraySize(LogFileName) ) ] = 0;
		m = _tcsrchr(LogFileName,_T('\\'));
		if (m) *m = 0;
	}
	_tcscat(LogFileName, L"\\CrashLogs");
	CreateDirectory(LogFileName, NULL);
	time_t rawtime;
	time(&rawtime);
	tm* timeinfo = localtime(&rawtime);
	_tcsftime(LogTime, ArraySize(LogTime), _T("\\FarTrap_%Y%m%d%H%M%S.log"), timeinfo);
	_tcscat( LogFileName, LogTime );
	LOpen(Nested);

     if ( !LogFile ) {
       SError( _T("Error processing exception...\nCan not create log file"), FMSG_ERRORTYPE );
       return FALSE;
     }

     setbuf( LogFile,NULL );

     _tcscpy(LogFileNameMsg,LogFileName);

//Dump trap
     idOutProc( _T("Exception in [%s] %s.\n")
                _T("==============================================================\n"),
                TrapFileName,
                (isFARTrap ? _T("FAR itself") : _T("plugin")) );

     OSVERSIONINFO vi={sizeof(vi)};
     GetVersionEx(&vi);
     idOutProc(_T("System information:\n Windows %s, version %d.%d.%d%s%s\n")
               _T("==============================================================\n"),
               (vi.dwPlatformId>1?_T("NT"):_T("9x")),
               vi.dwMajorVersion,
               vi.dwMinorVersion,
               LOWORD(vi.dwBuildNumber),
               (*vi.szCSDVersion?_T(" "):_T("")),
               vi.szCSDVersion
              );

     idOutProc( _T("FAR.EXE information:\n"));

//Try Get Version Info
     if ( FP_Info && FP_Info->AdvControl ) {
       FP_Info->AdvControl(&MainGuid, ACTL_GETFARMANAGERVERSION, 0, &FarVer);
       idOutProc( _T(" Version info: %d.%d.%d\n"),FarVer.Major,FarVer.Minor,FarVer.Build);
     }

//Try Get FileSize
     InFile = CreateFile( Path2Far, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0);
     if (InFile != INVALID_HANDLE_VALUE) {
       FarSize = GetFileSize( InFile, NULL);
       idOutProc( _T("    File size: %u bytes\n"),FarSize);
       CloseHandle(InFile);
     }

//Try Get TimeStamp
     FarAddr = GetModuleHandle( NULL );
     PIMAGE_NT_HEADERS fh;
     fh = (PIMAGE_NT_HEADERS)FarAddr;
     fh = (PIMAGE_NT_HEADERS)((DWORD)fh + ((PIMAGE_DOS_HEADER)fh)->e_lfanew);

     time_t tm = (time_t)( fh->FileHeader.TimeDateStamp );
     TCHAR   buff[100];
     _tcsftime( buff, ArraySize(buff),_T("%d.%m.%Y %H:%M:%S"),localtime(&tm) );
     idOutProc( _T("   Time stamp: %s\n"), buff);

//Try Get CheckSum
//Load imagehlp.dll dynamically because in Win95 this dll not present
     HINSTANCE h_imagehlp = LoadLibrary( _T("imagehlp.dll") );
     if ( h_imagehlp ) {
       pCheckSum = (MAPFILEANDCHECKSUM) GetProcAddress(h_imagehlp, "MapFileAndCheckSumW");
        //CHECKSUM_SUCCESS = 0
       if ( pCheckSum( Path2Far, &Checksum1, &Checksum2) == 0 )
         idOutProc( _T(" Hdr checksum: 0x%08lX (computed: 0x%08lX)\n"), Checksum1, Checksum2);
     }
//Exception info
     idOutProc( _T("==============================================================\n") );

     HInfo->ExceptionInfo( idOutProcA, xInfo->ExceptionRecord, xInfo->ContextRecord );

     idOutProc( _T("==============================================================\n") );

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
       FP_Info->Message( &MainGuid, nullptr,
                         FMSG_WARNING | FMSG_ALLINONE,
                         NULL, (const TCHAR * const *)_T("Trap log\nGenerating trap log file..."),
                         0, 0 );

     HInfo->StackWalkListContext( idOutProcA, xInfo->ContextRecord, STK_FULL, 0 );

//Clear all used STUFF
     fclose( LogFile );
     HInfo->Cleanup();

//Ask user to terminate FAR
     if ( isFARTrap )
       _stprintf( str,
                _T("Exception error...\n")
                _T("FAR itself executes an error and will be terminated.\n")
                _T("The trap log file has been saved to file:\n")
                _T("  %s\n")
                _T(""), LogFileNameMsg );
      else
       _stprintf( str,
                _T("Exception error...\n")
                _T("Plugin:\n")
                _T("  %s\n")
                _T("executes an error and will be unloaded.\n")
                _T("The trap log file has been saved to file:\n")
                _T("  %s\n")
                _T("\x1\n")
                _T("Do you want to terminate FAR itself ? (recommended)")
                _T(""),
                TrapFileNameMsg, LogFileNameMsg );

     if ( !FP_Info ||
          !FP_Info->Message ||
          FP_Info->Message( &MainGuid, nullptr,
          FMSG_LEFTALIGN | FMSG_ALLINONE | (isFARTrap ? FMSG_MB_OK : FMSG_MB_YESNO ),
                            NULL, (const TCHAR * const *)str, 0, 0 ) == 0 ) {
       TerminateProcess( GetCurrentProcess(),0 );
     }
     FreeLibrary( md );

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
#if defined(_MSC_VER) || defined(__GNUC__)
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
