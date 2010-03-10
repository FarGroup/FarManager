#include <windows.h>
#include <plugin.hpp>
#include <string.h>

#include "Desktoplng.hpp"
#include "Desktop.hpp"

char *GetMsg(int MsgId)
{
	return (char *)Info.GetMsg(Info.ModuleNumber,MsgId);
}


void InitDialogItems(struct InitDialogItem *Init, struct FarDialogItem *Item, int ItemsNumber)
{
	for (int I=0;I<ItemsNumber;I++)
	{
		Item[I].Type=Init[I].Type;
		Item[I].X1=Init[I].X1;
		Item[I].Y1=Init[I].Y1;
		Item[I].X2=Init[I].X2;
		Item[I].Y2=Init[I].Y2;
		Item[I].Focus=Init[I].Focus;
		Item[I].Selected=Init[I].Selected;
		Item[I].Flags=Init[I].Flags;
		Item[I].DefaultButton=Init[I].DefaultButton;
		if ((unsigned int)Init[I].Data<2000)
			strcpy(Item[I].Data,GetMsg((unsigned int)Init[I].Data));
		else
			strcpy(Item[I].Data,Init[I].Data);
	}
}


/*
ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
³ÉÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ Folder panel ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ» ³
³º [x] Add to Disks menu                                              º ³
³º  _  Disks menu hotkey ('1'-'9'). Leave empty to autoassign         º ³
³º Default folder                                                     º ³
³º __________________________________________________________________ º ³
³ÈÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ¼ ³
³                          [ Ok ]  [ Cancel ]                           ³
ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ
*/
int Config()
{
	HKEY hKey;
	struct InitDialogItem InitItems[]={
	/*      Type          X1 Y1 X2  Y2 Focus Flags            DefaultButton
	                                      Selected              Data
	*/
	/* 0 */ DI_DOUBLEBOX, 3, 1, 72, 6, 0, 0, 0,               0,(char *)MConfigTitle,
	/* 1 */ DI_CHECKBOX,  5, 2,  0, 0, 0, 0, 0,               0,(char *)MConfigAddToDisksMenu,
	/* 2 */ DI_FIXEDIT,   7, 3,  7, 3, 1, 0, 0,               0,"",
	/* 3 */ DI_TEXT,      9, 3,  0, 0, 0, 0, 0,               0,(char *)MConfigDisksMenuDigit,
	/* 4 */ DI_TEXT,      5, 4,  0, 0, 0, 0, 0,               0,(char *)MDefaultFolder,
	/* 5 */ DI_FIXEDIT,   6, 5, 67, 5, 0, 0, 0,               0,(char *)"",
	/* 6 */ DI_BUTTON,    0, 7,  0, 0, 0, 0, DIF_CENTERGROUP, 1,(char *)MOk,
	/* 7 */ DI_BUTTON,    0, 7,  0, 0, 0, 0, DIF_CENTERGROUP, 0,(char *)MCancel
	};

	struct FarDialogItem DialogItems[sizeof(InitItems)/sizeof(InitItems[0])];

	InitDialogItems(InitItems,DialogItems,sizeof(InitItems)/sizeof(InitItems[0]));
	DialogItems[1].Selected=Opt.AddToDisksMenu;
	if (Opt.DisksMenuDigit)
		wsprintf(DialogItems[2].Data,"%d",Opt.DisksMenuDigit);
	strcpy(DialogItems[5].Data,Opt.DefaultFolder);

	int ExitCode=Info.Dialog(Info.ModuleNumber,-1,-1,75,9,"FolderCfg",
				DialogItems,sizeof(DialogItems)/sizeof(DialogItems[0]));

	if (ExitCode!=6)
		return(FALSE);

	Opt.AddToDisksMenu=DialogItems[1].Selected;
	Opt.DisksMenuDigit=atoi(DialogItems[2].Data);
	strcpy(Opt.DefaultFolder,DialogItems[5].Data);

	DWORD Disposition, DataSize;

	// ¤«ï á®ªà é¥­¨ï ®¡ê¥¬®¢ :-)
	if (RegCreateKeyEx(HKEY_CURRENT_USER,PluginRootKey,0,NULL,0,KEY_WRITE,NULL,&hKey,&Disposition) == ERROR_SUCCESS)
	{
		RegSetValueEx(hKey,AddToDisksMenu,0,REG_DWORD,(BYTE *)&Opt.AddToDisksMenu,sizeof(Opt.AddToDisksMenu));
		RegSetValueEx(hKey,DisksMenuDigit,0,REG_DWORD,(BYTE *)&Opt.DisksMenuDigit,sizeof(Opt.DisksMenuDigit));
		DataSize=511;
		RegSetValueEx(hKey,DefaultFolder,0,REG_SZ,Opt.DefaultFolder,lstrlen(Opt.DefaultFolder)+1);
		RegCloseKey(hKey);
	}

	return TRUE;
}


char *getDesktop(void)
{
	DWORD DataSize=511;
	DWORD Disposition;
	HKEY hKey;
	*Opt.DefaultFolder=0;
	if ((RegOpenKeyEx(HKEY_CURRENT_USER,
			"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders",
			NULL,KEY_QUERY_VALUE,&hKey)) != ERROR_SUCCESS)
		return "";

	RegQueryValueEx(hKey,"Desktop",0,&Disposition,Opt.DefaultFolder,&DataSize);
	RegCloseKey(hKey);
	CharToOem(Opt.DefaultFolder,Opt.DefaultFolder);
	return Opt.DefaultFolder;
}

void createRegistry(char *PluginRootKey1)
{
	// ¤«ï á®ªà é¥­¨ï ®¡ê¥¬®¢ :-)
	HKEY hKey;
	DWORD Disposition, DataSize;
	int ExitCode;

	if (RegCreateKeyEx(HKEY_CURRENT_USER,PluginRootKey1,0,NULL,0,KEY_WRITE,NULL,&hKey,&Disposition) == ERROR_SUCCESS)
	{
		Opt.AddToDisksMenu=1;
		RegSetValueEx(hKey,AddToDisksMenu,0,REG_DWORD,(BYTE *)&Opt.AddToDisksMenu,sizeof(Opt.AddToDisksMenu));
		Opt.DisksMenuDigit=9;
		RegSetValueEx(hKey,DisksMenuDigit,0,REG_DWORD,(BYTE *)&Opt.DisksMenuDigit,sizeof(Opt.DisksMenuDigit));

		getDesktop();

		DataSize=511;
		RegSetValueEx(hKey,DefaultFolder,0,REG_SZ,Opt.DefaultFolder,lstrlen(Opt.DefaultFolder)+1);
		RegCloseKey(hKey);
	}
}
