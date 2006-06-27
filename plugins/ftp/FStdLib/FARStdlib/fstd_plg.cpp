#include <all_far.h>
#pragma hdrstop

#include "fstdlib.h"

PPluginStartupInfo          FP_Info          = NULL;
PFarStandardFunctions       FP_FSF           = NULL;
char                       *FP_PluginRootKey = NULL;
char                       *FP_PluginStartPath = NULL;
OSVERSIONINFO              *FP_WinVer        = NULL;
HMODULE                     FP_HModule       = NULL;
int                         FP_LastOpMode    = 0;
BOOL                        FP_IsOldFar;
DWORD                       FP_WinVerDW;
#if !defined(USE_ALL_LIB)
PHEX_DumpInfo               HEX_Info         = NULL;
#endif

static void RTL_CALLBACK idAtExit( void )
  {
     delete FP_Info;                FP_Info            = NULL;
     delete FP_FSF;                 FP_FSF             = NULL;
     delete FP_WinVer;              FP_WinVer          = NULL;
     delete[] FP_PluginRootKey;     FP_PluginRootKey   = NULL;
     delete[] FP_PluginStartPath;   FP_PluginStartPath = NULL;

     FP_HModule = NULL;
}

void DECLSPEC FP_SetStartupInfo( const PluginStartupInfo *Info,const char *KeyName )
  {

//Info
    FP_HModule = GetModuleHandle( FP_GetPluginName() );
    FP_Info    = new PluginStartupInfo;
    MemCpy( FP_Info,Info,sizeof(*Info) );

//FSF
    FP_IsOldFar = TRUE;
    if( Info->StructSize >= FAR_SIZE_170 ) {
#if !defined(__USE_165_HEADER__)
      FP_FSF = new FarStandardFunctions;
      MemCpy( FP_FSF,Info->FSF,sizeof(*FP_FSF) );
#endif
      FP_IsOldFar = FALSE;
    }

//Version
    FP_WinVer = new OSVERSIONINFO;
    FP_WinVer->dwOSVersionInfoSize = sizeof(*FP_WinVer);
    GetVersionEx( FP_WinVer );

    FP_WinVerDW = GetVersion();

//Plugin Reg key
    FP_PluginRootKey = new char[FAR_MAX_REG+1];
    StrCpy(FP_PluginRootKey,Info->RootKey,FAR_MAX_REG );
    StrCat(FP_PluginRootKey,"\\",FAR_MAX_REG );
    StrCat(FP_PluginRootKey,KeyName,FAR_MAX_REG );

//Start path
    FP_PluginStartPath = new char[ MAX_PATH_SIZE+1 ];
    FP_PluginStartPath[ GetModuleFileName( FP_HModule,FP_PluginStartPath,MAX_PATH_SIZE ) ] = 0;
    char *m = strrchr(FP_PluginStartPath,'\\');
    if ( m ) *m = 0;

//ExcDUMP
#if !defined(USE_ALL_LIB)
    HMODULE md;
    char    Path[ MAX_PATH_SIZE ];

    //ExcDump
    do{
      md = LoadLibrary( "ExcDump.dll" );
      if ( md ) break;

      StrCpy( Path, FP_PluginStartPath, sizeof(Path) );
      StrCat( Path, "\\ExcDump.dll", sizeof(Path) );
      md = LoadLibrary( Path );
      if ( md ) break;

    }while( 0 );

    HEX_Info = NULL;

    if ( md ) {
      HT_QueryInterface_t p = (HT_QueryInterface_t)GetProcAddress( md,"HEX_QueryInterface" );
      if ( !p || (HEX_Info=p()) == NULL )
        FreeLibrary( md );
    }
#endif
}

#if defined(USE_ALL_LIB)
static char LogFileName[ MAX_PATH_SIZE ];

static CONSTSTR RTL_CALLBACK idGetLog( void ) { return LogFileName; }

void SetLogProc( void )
  {  CONSTSTR m = FP_GetPluginLogName();

     if ( !m || !m[0] ) {
       LogFileName[0] = 0;
       return;
     }

     SetLogNameProc( idGetLog );

     if ( IsAbsolutePath(m) ) {
       TStrCpy( LogFileName, m );
       return;
     }

     LogFileName[ GetModuleFileName( FP_HModule, LogFileName, sizeof(LogFileName) ) ] = 0;
     strcpy( strrchr(LogFileName,'\\')+1, m );
}
#else
#define SetLogProc()
#endif

#if defined(__BORLAND)
BOOL WINAPI DllEntryPoint( HINSTANCE hinst, DWORD reason, LPVOID ptr )
  {
    if ( reason == DLL_PROCESS_ATTACH ) {
      FP_HModule = GetModuleHandle( FP_GetPluginName() );
      SetLogProc();
      AtExit( idAtExit );
    }

    BOOL res = FP_PluginStartup(reason);

    if ( reason == DLL_PROCESS_DETACH ) {
      CallAtExit();
    }

 return res;
}
#else
#if defined(__MSOFT)
BOOL WINAPI DllMain( HINSTANCE hinst, DWORD reason, LPVOID ptr )
  {
    if ( reason == DLL_PROCESS_ATTACH ) {
      FP_HModule = GetModuleHandle( FP_GetPluginName() );
      SetLogProc();
      AtExit( idAtExit );
    }

    BOOL res = FP_PluginStartup(reason);

    if ( reason == DLL_PROCESS_DETACH ) {
      CallAtExit();
    }

 return res;
}
#else
  #error "Define plugin DLL entry point procedure for your  compiller"
#endif
#endif
