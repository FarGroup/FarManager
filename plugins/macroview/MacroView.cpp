#define _FAR_NO_NAMELESS_UNIONS
#define _FAR_USE_FARFINDDATA
#include <CRT/crt.hpp>
#include "plugin.hpp"

#include "MacroLng.hpp"
#include "MacroView.hpp"
#include "farcolor.hpp"
#include "farkeys.hpp"

#if defined(__GNUC__)
#ifdef __cplusplus
extern "C"
{
#endif
	BOOL WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved);
#ifdef __cplusplus
};
#endif

BOOL WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved)
{
	(void) lpReserved;
	(void) dwReason;
	(void) hDll;
	return TRUE;
}
#endif

#include "MacroMix.cpp"
#include "strclass.cpp"
#include "regclass.cpp"
#include "Config.cpp"
#include "MacroDiff.cpp"

int WINAPI EXP_NAME(GetMinFarVersion)()
{
	return FARMANAGERVERSION;
}

void WINAPI EXP_NAME(SetStartupInfo)(const struct PluginStartupInfo *Info)
{
	hInstance=GetModuleHandle(NULL);
	::Info=*Info;
	::FSF=*Info->FSF;
	::Info.FSF=&::FSF;
	vi.dwOSVersionInfoSize=sizeof(vi);
	GetVersionEx(&vi);
	wsprintf(PluginRootKey, _T("%s\\%s"), Info->RootKey, Module_KEY);
	CheckFirstBackSlash(PluginRootKey,TRUE);
	const TCHAR *ptr=_tcsstr(Info->RootKey,Plugins_KEY);

	if (ptr)
		lstrcpyn(FarKey,Info->RootKey,(unsigned)(ptr-Info->RootKey)+1);
	else
		lstrcpyn(FarKey,Default_KEY,ARRAYSIZE(FarKey));

	CheckFirstBackSlash(FarKey,TRUE);
	ptr=_tcsstr(FarKey,Users_KEY);

	if (ptr)
	{
		ptr=_tcsrchr(FarKey,_T('\\'));

		if (ptr)
			lstrcpyn(FarUserName,ptr+1,lstrlen(ptr+1)+1);
		else
			*FarUserName=0;
	}
	else
		*FarUserName=0;

	lstrcpyn(FarUsersKey,Users_KEY,ARRAYSIZE(FarUsersKey));
	CheckFirstBackSlash(FarUsersKey,TRUE);
	wsprintf(KeyMacros,_T("%s\\%s"),FarKey,KeyMacros_KEY);
	CheckFirstBackSlash(KeyMacros,TRUE);
}

HANDLE WINAPI EXP_NAME(OpenPlugin)(int OpenFrom, INT_PTR Item)
{
	::OpenFrom=OpenFrom;

	// создаем экземпляр класса для работы с реестром
	if (!Reg)
		Reg=new TReg;

	Reg->SetRootKey(HKEY_CURRENT_USER);

	// создаем экземпляр Macro browser'а
	if (!Macro)
		Macro=new TMacroView;

	switch (Item)
	{
		case 0:
			Macro->MacroList();
	}

	return INVALID_HANDLE_VALUE;
}


void WINAPI EXP_NAME(ClosePlugin)(HANDLE hPlugin)
{
	(void)hPlugin;

	if (Reg)
		delete Reg;

	Reg=NULL;

	if (Macro)
		delete Macro;

	Macro=NULL;
}


void WINAPI EXP_NAME(ExitFAR)()
{
	if (Reg)
		delete Reg;

	Reg=NULL;

	if (Macro)
		delete Macro;

	Macro=NULL;
}


int WINAPI EXP_NAME(Configure)(int ItemNumber)
{
	// создаем экземпляр класса для работы с реестром
	if (!Reg)
		Reg=new TReg;

	Reg->SetRootKey(HKEY_CURRENT_USER);

	// создаем экземпляр Macro browser'а
	if (!Macro)
		Macro=new TMacroView;

	switch (ItemNumber)
	{
		case 0:
			return(Macro->Configure());
	}

	return FALSE;
}


void WINAPI EXP_NAME(GetPluginInfo)(struct PluginInfo *Info)
{
	Info->StructSize=sizeof(*Info);
	Info->Flags=PF_EDITOR|PF_VIEWER;
	Info->DiskMenuStrings=NULL;
	Info->DiskMenuStringsNumber=0;
	static TCHAR *PluginMenuStrings[1];
	PluginMenuStrings[0]=GetMsg(MMacroMenu);
	Info->PluginMenuStrings=PluginMenuStrings;
	Info->PluginMenuStringsNumber=ARRAYSIZE(PluginMenuStrings);
	static TCHAR *PluginConfigStrings[1];
	PluginConfigStrings[0]=GetMsg(MMacroMenu);
	Info->PluginConfigStrings=PluginConfigStrings;
	Info->PluginConfigStringsNumber=ARRAYSIZE(PluginConfigStrings);
}
