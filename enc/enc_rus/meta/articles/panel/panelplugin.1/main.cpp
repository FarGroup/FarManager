#include <windows.h>
#include <plugin.hpp>
#include <string.h>

#include "Desktoplng.hpp"
#include "Desktop.hpp"

struct Options Opt;
char PluginRootKey[80];

struct PluginStartupInfo Info;

const char *AddToDisksMenu="AddToDisksMenu";
const char *DisksMenuDigit="DisksMenuDigit";
const char *DefaultFolder="DefaultFolder";

void WINAPI _export SetStartupInfo(const struct PluginStartupInfo *Info)
{
	::Info=*Info;
	HKEY hKey;
	DWORD Disposition, DataSize;
	int ExitCode;

	wsprintf(PluginRootKey,"%s\\Desktop",::Info.RootKey);
	char ValueData[456];

	if (RegOpenKeyEx(HKEY_CURRENT_USER,PluginRootKey,0,KEY_READ,&hKey) == ERROR_SUCCESS)
	{
		// if нема, то создадим...
		DataSize=sizeof(Opt.AddToDisksMenu);

		if (RegQueryValueEx(hKey,AddToDisksMenu,0,&Disposition,ValueData,&DataSize) != ERROR_SUCCESS)
		{
			RegCloseKey(hKey);
			createRegistry(PluginRootKey);
		}
		else
		{
			DataSize=sizeof(Opt.AddToDisksMenu);
			Opt.AddToDisksMenu=1;
			RegQueryValueEx(hKey,AddToDisksMenu,0,&Disposition,(BYTE *)&Opt.AddToDisksMenu,&DataSize);

			DataSize=sizeof(Opt.DisksMenuDigit);
			Opt.DisksMenuDigit=9;
			RegQueryValueEx(hKey,DisksMenuDigit,0,&Disposition,(BYTE *)&Opt.DisksMenuDigit,&DataSize);

			DataSize=511;
			RegQueryValueEx(hKey,DefaultFolder,0,&Disposition,Opt.DefaultFolder,&DataSize);
		}

		RegCloseKey(hKey);
	}
	else
		createRegistry(PluginRootKey);
}


int WINAPI _export Configure(int ItemNumber)
{
	switch (ItemNumber)
	{
		case 0:
			return Config();
	}
	return FALSE;
}

void WINAPI _export GetPluginInfo(struct PluginInfo *Info)
{
	memset(Info,0,sizeof(struct PluginInfo));
	Info->StructSize=sizeof(*Info);
	Info->Flags=0;

	static char *DiskMenuStrings[1];
	DiskMenuStrings[0]=GetMsg(MFolderPanel);

	static int DiskMenuNumbers[1];
	Info->DiskMenuStrings=DiskMenuStrings;

	DiskMenuNumbers[0]=Opt.DisksMenuDigit;
	Info->DiskMenuNumbers=DiskMenuNumbers;
	Info->DiskMenuStringsNumber=Opt.AddToDisksMenu ? 1:0;

	static char *PluginMenuStrings[1];
	PluginMenuStrings[0]=GetMsg(MFolderPanel);
	Info->PluginMenuStrings=PluginMenuStrings;
	Info->PluginMenuStringsNumber=sizeof(PluginMenuStrings)/sizeof(PluginMenuStrings[0]);

	static char *PluginCfgStrings[1];
	PluginCfgStrings[0]=GetMsg(MFolderPanel);
	Info->PluginConfigStrings=PluginCfgStrings;
	Info->PluginConfigStringsNumber=sizeof(PluginCfgStrings)/sizeof(PluginCfgStrings[0]);
}
