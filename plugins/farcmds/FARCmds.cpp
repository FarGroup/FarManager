#include <CRT/crt.hpp>
#include <plugin.hpp>
#include "FARCmds.hpp"
#include "Lang.hpp"

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

#ifndef UNICODE
#define GetCheck(i) DialogItems[i].Selected
#define GetDataPtr(i) DialogItems[i].Data
#else
#define GetCheck(i) (int)Info.SendDlgMessage(hDlg,DM_GETCHECK,i,0)
#define GetDataPtr(i) ((const TCHAR *)Info.SendDlgMessage(hDlg,DM_GETCONSTTEXTPTR,i,0))
#endif

OSVERSIONINFO WinVer;

FARSTDCOPYTOCLIPBOARD CopyToClipboard;
FARSTDATOI FarAtoi;
FARSTDITOA FarItoa;
FARSTDSPRINTF FarSprintf;
FARSTDUNQUOTE Unquote;
#ifndef UNICODE
FARSTDEXPANDENVIRONMENTSTR ExpandEnvironmentStr;
#else
#define ExpandEnvironmentStr  ExpandEnvironmentStrings
#endif
FARSTDPOINTTONAME PointToName;
FARSTDGETPATHROOT GetPathRoot;
FARSTDADDENDSLASH AddEndSlash;
FARSTDMKTEMP MkTemp;
FARSTDRTRIM  FarRTrim;
FARSTDLTRIM  FarLTrim;
FARSTDLTRIM  FarTrim;
FARSTDLOCALSTRICMP LStricmp;
FARSTDLOCALSTRNICMP LStrnicmp;
FARSTDMKLINK MkLink;
FARSTDKEYNAMETOKEY FarNameToKey;
FARSTDQUOTESPACEONLY QuoteSpaceOnly;
FARSTDRECURSIVESEARCH FarRecursiveSearch;
FARSTDLOCALISALPHA FarIsAlpha;

struct RegistryStr REGStr={_T("Add2PlugMenu"),_T("Add2DisksMenu"),_T("%s%s%s"),_T("Separator"),
	_T("DisksMenuDigit"), _T("ShowCmdOutput"), _T("CatchMode"), _T("ViewZeroFiles"), _T("EditNewFiles")
};
static struct PluginStartupInfo Info;
FarStandardFunctions FSF;
struct PanelInfo PInfo;
TCHAR PluginRootKey[80];
TCHAR selectItem[MAX_PATH*5];
//TCHAR tempFileNameOut[MAX_PATH*5],tempFileNameErr[MAX_PATH*5],FileNameOut[MAX_PATH*5],FileNameErr[MAX_PATH*5],
TCHAR fullcmd[MAX_PATH*5],cmd[MAX_PATH*5];

#include "Reg.cpp"
#include "Mix.cpp"
#include "OpenCmd.cpp"

int WINAPI EXP_NAME(GetMinFarVersion)()
{
	return FARMANAGERVERSION;
}

void WINAPI EXP_NAME(SetStartupInfo)(const struct PluginStartupInfo *psInfo)
{
	Info=*psInfo;
	FSF=*psInfo->FSF;
	Info.FSF=&FSF;
	FarItoa=Info.FSF->itoa;
	FarAtoi=Info.FSF->atoi;
	FarSprintf=Info.FSF->sprintf;
#ifndef UNICODE
	ExpandEnvironmentStr=Info.FSF->ExpandEnvironmentStr;
#endif
	Unquote=Info.FSF->Unquote;
	PointToName=Info.FSF->PointToName;
	GetPathRoot=Info.FSF->GetPathRoot;
	AddEndSlash=Info.FSF->AddEndSlash;
	MkTemp=Info.FSF->MkTemp;
	FarRTrim=Info.FSF->RTrim;
	FarLTrim=Info.FSF->LTrim;
	FarTrim=Info.FSF->Trim;
	LStricmp=Info.FSF->LStricmp;
	LStrnicmp=Info.FSF->LStrnicmp;
	CopyToClipboard=Info.FSF->CopyToClipboard;
	MkLink=Info.FSF->MkLink;
	FarNameToKey=Info.FSF->FarNameToKey;
	QuoteSpaceOnly=Info.FSF->QuoteSpaceOnly;
	FarRecursiveSearch=Info.FSF->FarRecursiveSearch;
	FarIsAlpha=Info.FSF->LIsAlpha;
	lstrcpy(PluginRootKey,Info.RootKey);
	lstrcat(PluginRootKey,_T("\\FARCmds"));
	GetRegKey(HKEY_CURRENT_USER,_T(""),REGStr.Separator,Opt.Separator,_T(" "),3);
	Opt.Add2PlugMenu=GetRegKey(HKEY_CURRENT_USER,_T(""),REGStr.Add2PlugMenu,0);
	Opt.Add2DisksMenu=GetRegKey(HKEY_CURRENT_USER,_T(""),REGStr.Add2DisksMenu,0);
	Opt.DisksMenuDigit=GetRegKey(HKEY_CURRENT_USER,_T(""),REGStr.DisksMenuDigit,0);
	Opt.ShowCmdOutput=GetRegKey(HKEY_CURRENT_USER,_T(""),REGStr.ShowCmdOutput,0);
	Opt.CatchMode=GetRegKey(HKEY_CURRENT_USER,_T(""),REGStr.CatchMode,0);
	Opt.ViewZeroFiles=GetRegKey(HKEY_CURRENT_USER,_T(""),REGStr.ViewZeroFiles,1);
	Opt.EditNewFiles=GetRegKey(HKEY_CURRENT_USER,_T(""),REGStr.EditNewFiles,1);
	WinVer.dwOSVersionInfoSize=sizeof(WinVer);
#ifndef UNICODE
	GetVersionEx(&WinVer);
#else
	GetVersionExW(&WinVer);
#endif
}




HANDLE WINAPI EXP_NAME(OpenPlugin)(int OpenFrom,INT_PTR Item)
{
#ifndef UNICODE
	Info.Control(INVALID_HANDLE_VALUE,FCTL_GETPANELINFO,&PInfo);
#else
	Info.Control(PANEL_ACTIVE,FCTL_GETPANELINFO,0,(LONG_PTR)&PInfo);
#endif
//  tempFileNameOut[0]=tempFileNameErr[0]=FileNameOut[0]=FileNameErr[0]=
	fullcmd[0]=cmd[0]=selectItem[0]=_T('\0');
#ifndef UNICODE
	int _GETPANELINFO=FCTL_GETPANELINFO,
	                  _SETPANELDIR=FCTL_SETPANELDIR,
	                               _REDRAWPANEL=FCTL_REDRAWPANEL;
#define _PANEL_HANDLE INVALID_HANDLE_VALUE
#else
	HANDLE _PANEL_HANDLE = PANEL_ACTIVE;
#define _GETPANELINFO FCTL_GETPANELINFO
#define _SETPANELDIR  FCTL_SETPANELDIR
#define _REDRAWPANEL  FCTL_REDRAWPANEL
#endif

	if (OpenFrom==OPEN_COMMANDLINE)
	{
		OpenFromCommandLine((TCHAR *)Item);
	}
	else if (OpenFrom == OPEN_PLUGINSMENU && !Item && PInfo.PanelType != PTYPE_FILEPANEL)
	{
		return INVALID_HANDLE_VALUE;
	}
	else
	{
#ifndef UNICODE
		lstrcpy(selectItem,PInfo.CurDir);
#else
		Info.Control(PANEL_ACTIVE,FCTL_GETPANELDIR,ArraySize(selectItem),(LONG_PTR)selectItem);
#endif

		if (lstrlen(selectItem))
			AddEndSlash(selectItem);

#ifndef UNICODE
		lstrcat(selectItem, PInfo.PanelItems[PInfo.CurrentItem].FindData.cFileName);
#else
		PluginPanelItem* PPI=(PluginPanelItem*)malloc(Info.Control(PANEL_ACTIVE,FCTL_GETPANELITEM,PInfo.CurrentItem,0));

		if (PPI)
		{
			Info.Control(PANEL_ACTIVE,FCTL_GETPANELITEM,PInfo.CurrentItem,(LONG_PTR)PPI);
			lstrcat(selectItem,PPI->FindData.lpwszFileName);
			free(PPI);
		}

#endif
#ifndef UNICODE
		_GETPANELINFO=FCTL_GETANOTHERPANELINFO,
		              _SETPANELDIR=FCTL_SETANOTHERPANELDIR,
		                           _REDRAWPANEL=FCTL_REDRAWANOTHERPANEL;
#else
		_PANEL_HANDLE=PANEL_PASSIVE;
#endif
	}

	/*установить курсор на объект*/
	if (lstrlen(selectItem))
	{
		static struct PanelRedrawInfo PRI;
		static TCHAR Name[MAX_PATH], Dir[MAX_PATH*5];
		int pathlen;
		lstrcpy(Name,PointToName(selectItem));
		pathlen=(int)(PointToName(selectItem)-selectItem);

		if (pathlen)
			_tmemcpy(Dir,selectItem,pathlen);

		Dir[pathlen]=0;
		FarTrim(Name);
		FarTrim(Dir);
		Unquote(Name);
		Unquote(Dir);

		if (*Dir)
#ifndef UNICODE
			Info.Control(_PANEL_HANDLE,_SETPANELDIR,&Dir);

		Info.Control(_PANEL_HANDLE,_GETPANELINFO,&PInfo);
#else
			Info.Control(_PANEL_HANDLE,_SETPANELDIR,0,(LONG_PTR)&Dir);
		Info.Control(_PANEL_HANDLE,_GETPANELINFO,0,(LONG_PTR)&PInfo);
#endif
		PRI.CurrentItem=PInfo.CurrentItem;
		PRI.TopPanelItem=PInfo.TopPanelItem;

		for (int J=0; J < PInfo.ItemsNumber; J++)
		{
#ifndef UNICODE

			if (!LStricmp(Name,PointToName(PInfo.PanelItems[J].FindData.cFileName)))
#else
			bool Equal=false;

			PluginPanelItem* PPI=(PluginPanelItem*)malloc(Info.Control(_PANEL_HANDLE,FCTL_GETPANELITEM,J,0));

			if (PPI)
			{
				Info.Control(_PANEL_HANDLE,FCTL_GETPANELITEM,J,(LONG_PTR)PPI);
				Equal=!LStricmp(Name,PointToName(PPI->FindData.lpwszFileName));
				free(PPI);
			}

			if (Equal)
#endif
			{
				PRI.CurrentItem=J;
				PRI.TopPanelItem=J;
				break;
			}
		}

#ifndef UNICODE
		Info.Control(_PANEL_HANDLE,_REDRAWPANEL,&PRI);
#else
		Info.Control(_PANEL_HANDLE,_REDRAWPANEL,0,(LONG_PTR)&PRI);
#endif
	}
	else
#ifndef UNICODE
		Info.Control(_PANEL_HANDLE,_REDRAWPANEL,NULL);

#else
		Info.Control(_PANEL_HANDLE,_REDRAWPANEL,0,NULL);
#endif
#undef _PANEL_HANDLE
#undef _GETPANELINFO
#undef _SETPANELDIR
#undef _REDRAWPANEL
	return(INVALID_HANDLE_VALUE);
}

void WINAPI EXP_NAME(GetPluginInfo)(struct PluginInfo *Info)
{
	Info->StructSize=sizeof(*Info);
	Info->Flags=PF_FULLCMDLINE;
	static TCHAR *PluginMenuStrings[1],*PluginConfigStrings[1],
	*DiskMenuStrings[1];

	if (Opt.Add2PlugMenu)
	{
		PluginMenuStrings[0]=(TCHAR*)GetMsg(MSetPassiveDir);
		Info->PluginMenuStrings=PluginMenuStrings;
		Info->PluginMenuStringsNumber=ArraySize(PluginMenuStrings);
	}
	else
	{
		Info->PluginMenuStringsNumber=0;
		Info->PluginMenuStrings=0;
	}

	if (Opt.Add2DisksMenu)
	{
		DiskMenuStrings[0]=(TCHAR*)GetMsg(MSetPassiveDir);
		Info->DiskMenuStrings=DiskMenuStrings;
		Info->DiskMenuStringsNumber=1;
		Info->DiskMenuNumbers=&Opt.DisksMenuDigit;
	}
	else
	{
		Info->DiskMenuStringsNumber=0;
		Info->DiskMenuStrings=0;
	}

	PluginConfigStrings[0]=(TCHAR*)GetMsg(MConfig);
	Info->PluginConfigStrings=PluginConfigStrings;
	Info->PluginConfigStringsNumber=ArraySize(PluginConfigStrings);
#ifndef UNICODE
	Info->CommandPrefix=_T("far:view:edit:goto:clip:whereis:macro:link:run");
#else
	Info->CommandPrefix=_T("far:view:edit:goto:clip:whereis:macro:link:run:load:unload");
#endif
}

int WINAPI EXP_NAME(Configure)(int /*ItemNumber*/)
{
	struct InitDialogItem InitItems[]=
	{
		//   Type           X1 Y1 X2 Y2 Fo Se Fl               DB Data
		/*00*/ { DI_DOUBLEBOX,   3, 1,69,20, 0, 0, DIF_BOXCOLOR,    0, (TCHAR *)MConfig},
		/*01*/ { DI_CHECKBOX,    5, 2, 0, 0, 1, 0, 0,               0, (TCHAR *)MAddSetPassiveDir2PlugMenu},
		/*02*/ { DI_TEXT,        0, 3, 0, 3, 0, 0, DIF_BOXCOLOR|DIF_SEPARATOR,   0,_T("")},
		/*03*/ { DI_CHECKBOX,    5, 4, 0, 0, 0, 0, 0,               0, (TCHAR *)MAddToDisksMenu},
		/*04*/ { DI_FIXEDIT,     7, 5, 7, 0, 0, 0, DIF_BOXCOLOR|DIF_MASKEDIT,    0, _T("")},
		/*05*/ { DI_TEXT,        9, 5, 0, 0, 0, 0, 0,               0, (TCHAR *)MDisksMenuDigit},
		/*06*/ { DI_TEXT,        0, 6, 0, 6, 0, 0, DIF_BOXCOLOR|DIF_SEPARATOR,   0,_T("")},
		/*07*/ { DI_RADIOBUTTON, 5, 7, 0, 0, 0, 0, DIF_GROUP,       0, (TCHAR *)MHideCmdOutput},
		/*08*/ { DI_RADIOBUTTON, 5, 8, 0, 0, 0, 0, 0,               0, (TCHAR *)MKeepCmdOutput},
		/*09*/ { DI_RADIOBUTTON, 5, 9, 0, 0, 0, 0, 0,               0, (TCHAR *)MEchoCmdOutput},
		/*10*/ { DI_TEXT,        0,10, 0,10, 0, 0, DIF_BOXCOLOR|DIF_SEPARATOR,   0,_T("")},
		/*11*/ { DI_RADIOBUTTON, 5,11, 0, 0, 0, 0, DIF_GROUP,       0, (TCHAR *)MCatchAllInOne},
		/*12*/ { DI_RADIOBUTTON, 5,12, 0, 0, 0, 0, 0,               0, (TCHAR *)MCatchStdOutput},
		/*13*/ { DI_RADIOBUTTON, 5,13, 0, 0, 0, 0, 0,               0, (TCHAR *)MCatchStdError},
		/*14*/ { DI_RADIOBUTTON, 5,14, 0, 0, 0, 0, 0,               0, (TCHAR *)MCatchSeparate},
		/*15*/ { DI_TEXT,        0,15, 0,15, 0, 0, DIF_BOXCOLOR|DIF_SEPARATOR,   0,_T("")},
		/*16*/ { DI_CHECKBOX,    5,16, 0, 0, 0, 0, 0,               0, (TCHAR *)MViewZeroFiles},
		/*17*/ { DI_CHECKBOX,    5,17, 0, 0, 0, 0, 0,               0, (TCHAR *)MEditNewFiles},
		/*18*/ { DI_TEXT,        0,18, 0,18, 0, 0, DIF_BOXCOLOR|DIF_SEPARATOR,   0,_T("")},
		/*19*/ { DI_BUTTON,      0,19, 0, 0, 0, 0, DIF_CENTERGROUP, 1, (TCHAR *)MOk},
		/*20*/ { DI_BUTTON,      0,19, 0, 0, 0, 0, DIF_CENTERGROUP, 0, (TCHAR *)MCancel},
	};
	BOOL ret=FALSE;
	struct FarDialogItem DialogItems[ArraySize(InitItems)];
	InitDialogItems(InitItems,DialogItems,ArraySize(InitItems));
	DialogItems[4].Mask=_T("9");
	DialogItems[1].Selected=Opt.Add2PlugMenu;
	DialogItems[3].Selected=Opt.Add2DisksMenu;
	DialogItems[7+Opt.ShowCmdOutput].Selected = 1;
	DialogItems[11+Opt.CatchMode].Selected = 1;
	DialogItems[16].Selected = Opt.ViewZeroFiles;
	DialogItems[17].Selected = Opt.EditNewFiles;
#ifdef UNICODE
	wchar_t numstr[32];
	DialogItems[4].PtrData = numstr;
	FarItoa(Opt.DisksMenuDigit,numstr,10);
#else
	FarItoa(Opt.DisksMenuDigit,(TCHAR *)DialogItems[4].Data,10);
#endif
#ifndef UNICODE
	int ExitCode=Info.Dialog(Info.ModuleNumber,-1,-1,73,22,"Config",
	                         DialogItems,ArraySize(InitItems));
#else
	HANDLE hDlg=Info.DialogInit(Info.ModuleNumber,-1,-1,73,22,L"Config",
	                            DialogItems,ArraySize(InitItems),0,0,NULL,0);

	if (hDlg == INVALID_HANDLE_VALUE)
		return ret;

	int ExitCode=Info.DialogRun(hDlg);
#endif

	if (19 == ExitCode)
	{
		Opt.Add2PlugMenu=GetCheck(1);
		Opt.Add2DisksMenu=GetCheck(3);
		Opt.DisksMenuDigit=FarAtoi(GetDataPtr(4));
		Opt.ViewZeroFiles = GetCheck(16);
		Opt.EditNewFiles = GetCheck(17);
		Opt.CatchMode = Opt.ShowCmdOutput = 0;

		for (int i = 0 ; i < 3 ; i++)
			if (GetCheck(7+i))
			{
				Opt.ShowCmdOutput = i;
				break;
			}

		for (int j = 0 ; j < 4 ; j++)
			if (GetCheck(11+j))
			{
				Opt.CatchMode = j;
				break;
			}

		SetRegKey(HKEY_CURRENT_USER,_T(""),REGStr.Add2PlugMenu,Opt.Add2PlugMenu);
		SetRegKey(HKEY_CURRENT_USER,_T(""),REGStr.Add2DisksMenu,Opt.Add2DisksMenu);
		SetRegKey(HKEY_CURRENT_USER,_T(""),REGStr.DisksMenuDigit,Opt.DisksMenuDigit);
		SetRegKey(HKEY_CURRENT_USER,_T(""),REGStr.ShowCmdOutput,Opt.ShowCmdOutput);
		SetRegKey(HKEY_CURRENT_USER,_T(""),REGStr.CatchMode,Opt.CatchMode);
		SetRegKey(HKEY_CURRENT_USER,_T(""),REGStr.ViewZeroFiles,Opt.ViewZeroFiles);
		SetRegKey(HKEY_CURRENT_USER,_T(""),REGStr.EditNewFiles,Opt.EditNewFiles);
		ret=TRUE;
	}

#ifdef UNICODE
	Info.DialogFree(hDlg);
#endif
	return ret;
}
