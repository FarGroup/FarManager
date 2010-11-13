#include <all_far.h>
#pragma hdrstop

#include "fstdlib.h"

PluginStartupInfo*          FP_Info          = NULL;
FarStandardFunctions*       FP_FSF           = NULL;
char                       *FP_PluginRootKey = NULL;
char                       *FP_PluginStartPath = NULL;
OSVERSIONINFO              *FP_WinVer        = NULL;
HMODULE                     FP_HModule       = NULL;
int                         FP_LastOpMode    = 0;
DWORD                       FP_WinVerDW;

static void _cdecl idAtExit(void)
{
	delete FP_Info;                FP_Info            = NULL;
	delete FP_FSF;                 FP_FSF             = NULL;
	delete FP_WinVer;              FP_WinVer          = NULL;
	delete[] FP_PluginRootKey;     FP_PluginRootKey   = NULL;
	delete[] FP_PluginStartPath;   FP_PluginStartPath = NULL;
	FP_HModule = NULL;
}

void WINAPI FP_SetStartupInfo(const PluginStartupInfo *Info,const char *KeyName)
{
//Info
	FP_HModule = GetModuleHandle(FP_GetPluginName());
	FP_Info    = new PluginStartupInfo;
	memcpy(FP_Info,Info,sizeof(*Info));
//FSF
	FP_FSF = new FarStandardFunctions;
	memcpy(FP_FSF,Info->FSF,sizeof(*FP_FSF));
//Version
	FP_WinVer = new OSVERSIONINFO;
	FP_WinVer->dwOSVersionInfoSize = sizeof(*FP_WinVer);
	GetVersionEx(FP_WinVer);
	FP_WinVerDW = GetVersion();
//Plugin Reg key
	FP_PluginRootKey = new char[FAR_MAX_REG+1];
	StrCpy(FP_PluginRootKey,Info->RootKey,FAR_MAX_REG);
	StrCat(FP_PluginRootKey,"\\",FAR_MAX_REG);
	StrCat(FP_PluginRootKey,KeyName,FAR_MAX_REG);
//Start path
	FP_PluginStartPath = new char[ MAX_PATH_SIZE+1 ];
	FP_PluginStartPath[ GetModuleFileName(FP_HModule,FP_PluginStartPath,MAX_PATH_SIZE)] = 0;
	char *m = strrchr(FP_PluginStartPath,'\\');

	if(m) *m = 0;
}

#if defined(__BORLAND)
BOOL WINAPI DllEntryPoint(HINSTANCE hinst, DWORD reason, LPVOID ptr)
{
	if(reason == DLL_PROCESS_ATTACH)
	{
		FP_HModule = GetModuleHandle(FP_GetPluginName());
		AtExit(idAtExit);
	}

	BOOL res = FP_PluginStartup(reason);

	if(reason == DLL_PROCESS_DETACH)
	{
		CallAtExit();
	}

	return res;
}
#else
#if defined(__MSOFT)
BOOL WINAPI DllMain(HINSTANCE hinst, DWORD reason, LPVOID ptr)
{
	if(reason == DLL_PROCESS_ATTACH)
	{
		FP_HModule = GetModuleHandle(FP_GetPluginName());
		AtExit(idAtExit);
	}

	BOOL res = FP_PluginStartup(reason);

	if(reason == DLL_PROCESS_DETACH)
	{
		CallAtExit();
	}

	return res;
}
#else
#if defined(__GNU)
BOOL WINAPI DllMain(HANDLE hDll,DWORD reason,LPVOID lpReserved)
{
	if(reason == DLL_PROCESS_ATTACH)
	{
		FP_HModule = GetModuleHandle(FP_GetPluginName());
		AtExit(idAtExit);
	}

	BOOL res = FP_PluginStartup(reason);

	if(reason == DLL_PROCESS_DETACH)
	{
		CallAtExit();
	}

	return res;
}
#else
#error "Define plugin DLL entry point procedure for your  compiller"
#endif
#endif
#endif
