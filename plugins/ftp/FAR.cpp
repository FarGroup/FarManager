#include <all_far.h>
#pragma hdrstop

#include "Int.h"

LPCSTR WINAPI FP_GetPluginLogName(void)
{
	return "farftp.log";
}
BOOL     WINAPI FP_PluginStartup(DWORD Reason)
{
	return TRUE;
}

//------------------------------------------------------------------------
Options       Opt;
FTP          *FTPPanels[3] = { 0 };
FTP          *LastUsedPlugin = NULL;
BOOL          SocketStartup  = FALSE;
int           SocketInitializeError = 0;
AbortProc     ExitProc;
char          DialogEditBuffer[ DIALOG_EDIT_SIZE ];

typedef char  FTPDistString[MAX_PATH];
FTPDistString DiskStrings[ 1+FTP_MAXBACKUPS ];
LPCSTR      DiskMenuStrings[ 1+FTP_MAXBACKUPS ];
int           DiskMenuNumbers[ 1+FTP_MAXBACKUPS ];

//------------------------------------------------------------------------
FTP *WINAPI OtherPlugin(FTP *p)
{
	if(!p)
		return p;

	if(FTPPanels[0] == p)
		return FTPPanels[1];
	else if(FTPPanels[1] == p)
		return FTPPanels[0];

	return NULL;
}

int WINAPI PluginPanelNumber(FTP *p)
{
	if(p)
	{
		if(FTPPanels[0] == p)
			return 1;
		else if(FTPPanels[1] == p)
			return 2;
	}

	return 0;
}

int WINAPI PluginUsed(void)
{
	return PluginPanelNumber(LastUsedPlugin);
}

//------------------------------------------------------------------------
void __cdecl CloseUp(void)
{
	int n;

	if(FTP::BackupCount)
	{
		Log(("CloseUp.FreeBackups"));

		for(n = 0; n < FTP::BackupCount; n++)
			delete FTP::Backups[n];

		FTP::BackupCount = 0;
	}

	Log(("CloseUp.FreePlugins"));
	FreePlugins();
	Log(("CloseUp.Delete Opt data"));

	for(n = 0; n < 12; n++)
		free(Opt.Months[n]);

	memset(Opt.Months, 0, sizeof(Opt.Months));

	if(SocketStartup)
	{
		Log(("CloseUp.CloseWSA"));
		WSACleanup();
	}

	if(ExitProc)
		ExitProc();
}

void AddPlugin(FTP *ftp)
{
	if(!FTPPanels[0])
		FTPPanels[0] = ftp;
	else if(!FTPPanels[1])
		FTPPanels[1] = ftp;
	else if(!FTPPanels[2])
		FTPPanels[2] = ftp;
	else
		Assert(!"More then two plugins in a time !!");
}

void RemovePlugin(FTP *ftp)
{
	PROC(("RemovePlugin","%p",ftp))

	if(FTPPanels[0] == ftp)
		FTPPanels[0] = NULL;
	else if(FTPPanels[1] == ftp)
		FTPPanels[1] = NULL;
	else if(FTPPanels[2] == ftp)
		FTPPanels[2] = NULL;

	if(FTPPanels[2])
	{
		AddPlugin(FTPPanels[2]);
		FTPPanels[2] = NULL;
	}

	LPCSTR rejectReason;

	if(ftp->isBackup())
	{
		ftp->SetBackupMode();
		return;
	}

	LPCSTR itms[] =
	{
		FMSG(MRejectTitle),
		FMSG(MRejectCanNot),
		NULL,
		FMSG(MRejectAsk1),
		FMSG(MRejectAsk2),
		FMSG(MRejectIgnore), FMSG(MRejectSite)
	};

	do
	{
		if((rejectReason=ftp->CloseQuery()) == NULL) break;

		itms[2] = rejectReason;

		if(FMessage(FMSG_LEFTALIGN|FMSG_WARNING,"CloseQueryReject",itms,ARRAYSIZE(itms),2) != 1)
			break;

		ftp->AddToBackup();

		if(!ftp->isBackup())
		{
			SayMsg(FMSG(MRejectFail));
			break;
		}

		return;
	}
	while(0);

	delete ftp;
}

//------------------------------------------------------------------------
extern "C" void WINAPI ExitFAR(void)
{
	//FP_Info = NULL; FP_GetMsg( "temp" );
	CallAtExit();
}

extern "C" void WINAPI SetStartupInfo(const struct PluginStartupInfo *Info)
{
	LastUsedPlugin = NULL;
	FP_SetStartupInfo(Info,"FTP");
	ExitProc = AtExit(CloseUp);
	memset(&Opt, 0, sizeof(Opt));

	for(int n = 0; n < FTP_MAXBACKUPS; n++)
		DiskMenuStrings[n] = DiskStrings[n];

	memset(DiskMenuNumbers, 0, sizeof(DiskMenuNumbers));
	PROC(("SetStartupInfo",NULL))
	ReadCfg();
	LogCmd("FTP plugin loaded", ldInt);
}

extern "C" void WINAPI GetPluginInfo(struct PluginInfo *Info)
{
	LastUsedPlugin = NULL;
	PROC(("GetPluginInfo","%p",Info))
	static LPCSTR PluginMenuStrings[1];
	static LPCSTR PluginCfgStrings[1];
	static char     MenuString[MAX_PATH];
	static char     CfgString[MAX_PATH];
	_snprintf(MenuString,     ARRAYSIZE(MenuString),     "%s", FP_GetMsg(MFtpMenu));
	_snprintf(DiskStrings[0], ARRAYSIZE(DiskStrings[0]), "%s", FP_GetMsg(MFtpDiskMenu));
	_snprintf(CfgString,      ARRAYSIZE(CfgString),      "%s", FP_GetMsg(MFtpMenu));
	FTPHost* p;
	int      n,
	  uLen = 0,
	  hLen = 0;
	char     str[MAX_PATH];
	FTP*     ftp;

	for(n = 0; n < FTP::BackupCount; n++)
	{
		ftp = FTP::Backups[n];

		if(!ftp->FTPMode())
			continue;

		p = &ftp->Host;
		uLen = Max(uLen, static_cast<int>(strlen(p->User)));
		hLen = Max(hLen, static_cast<int>(strlen(p->Host)));
	}

	for(n = 0; n < FTP::BackupCount; n++)
	{
		ftp = FTP::Backups[n];
		ftp->GetCurPath(str,ARRAYSIZE(str));

		if(ftp->FTPMode())
		{
			p = &ftp->Host;
			_snprintf(DiskStrings[1+n], ARRAYSIZE(DiskStrings[0]),
			          "FTP: %-*s %-*s %s",
			          uLen, p->User, hLen, p->Host, str);
		}
		else
			_snprintf(DiskStrings[1+n], ARRAYSIZE(DiskStrings[0]), "FTP: %s", str);
	}

	DiskMenuNumbers[0]   = Opt.DisksMenuDigit;
	PluginMenuStrings[0] = MenuString;
	PluginCfgStrings[0]  = CfgString;
	Info->StructSize                = sizeof(*Info);
	Info->Flags                     = 0;
	Info->DiskMenuStrings           = DiskMenuStrings;
	Info->DiskMenuNumbers           = DiskMenuNumbers;
	Info->DiskMenuStringsNumber     = Opt.AddToDisksMenu ? (1+FTP::BackupCount) : 0;
	Info->PluginMenuStrings         = PluginMenuStrings;
	Info->PluginMenuStringsNumber   = Opt.AddToPluginsMenu ? ARRAYSIZE(PluginMenuStrings):0;
	Info->PluginConfigStrings       = PluginCfgStrings;
	Info->PluginConfigStringsNumber = ARRAYSIZE(PluginCfgStrings);
	Info->CommandPrefix             = FTP_CMDPREFIX;
}

extern "C" int WINAPI Configure(int ItemNumber)
{
	LastUsedPlugin = NULL;
	PROC(("Configure","%d",ItemNumber))

	switch(ItemNumber)
	{
		case 0:

			if(!Config())
				return FALSE;
	}

	//Update panels
	return TRUE;
}

extern "C" HANDLE WINAPI OpenPlugin(int OpenFrom,INT_PTR Item)
{
	LastUsedPlugin = NULL;
	PROC(("OpenPlugin","%d,%d",OpenFrom,Item))
	FTP *Ftp;
	ReadCfg();

	if(!InitPlugins())
		return INVALID_HANDLE_VALUE;

	if(Item == 0 || Item > FTP::BackupCount)
		Ftp = new FTP;
	else
	{
		Ftp = FTP::Backups[ Item-1 ];
		Ftp->SetActiveMode();
	}

	AddPlugin(Ftp);
	Ftp->Call();
	Log(("FTP handle: %p",Ftp));

	do
	{
		if(OpenFrom==OPEN_SHORTCUT)
		{
			if(!Ftp->ProcessShortcutLine((char *)Item))
				break;

			Ftp->End();
		}
		else if(OpenFrom==OPEN_COMMANDLINE)
		{
			if(!Ftp->ProcessCommandLine((char *)Item))
				break;
		}

		Ftp->End();
		return (HANDLE)Ftp;
	}
	while(0);

	RemovePlugin(Ftp);
	return INVALID_HANDLE_VALUE;
}

extern "C" void WINAPI ClosePlugin(HANDLE hPlugin)
{
	FTP *p = (FTP*)hPlugin;
	PROC(("ClosePlugin","%p",hPlugin))
	LastUsedPlugin = p;
	RemovePlugin(p);
}

extern "C" int WINAPI GetFindData(HANDLE hPlugin,struct PluginPanelItem **pPanelItem,int *pItemsNumber,int OpMode)
{
	FPOpMode _op(OpMode);
	FTP*     p = (FTP*)hPlugin;
	p->Call();
	int rc = p->GetFindData(pPanelItem,pItemsNumber,OpMode);
	p->End(rc);
	return rc;
}

extern "C" void WINAPI FreeFindData(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber)
{
	FTP*     p = (FTP*)hPlugin;
	p->Call();
	p->FreeFindData(PanelItem,ItemsNumber);
	p->End();
}

extern "C" void WINAPI GetOpenPluginInfo(HANDLE hPlugin,struct OpenPluginInfo *Info)
{
	FTP*     p = (FTP*)hPlugin;
	p->Call();
	p->GetOpenPluginInfo(Info);
	p->End();
}

extern "C" int WINAPI SetDirectory(HANDLE hPlugin,LPCSTR Dir,int OpMode)
{
	FPOpMode _op(OpMode);
	FTP*     p = (FTP*)hPlugin;
	p->Call();
	int rc = p->SetDirectoryFAR(Dir,OpMode);
	p->End(rc);
	return rc;
}

extern "C" int WINAPI GetFiles(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int Move,char *DestPath,int OpMode)
{
	FPOpMode _op(OpMode);
	FTP*     p = (FTP*)hPlugin;

	if(!p || !DestPath || !DestPath[0])
		return FALSE;

	String s(DestPath);
	p->Call();
	int rc = p->GetFiles(PanelItem,ItemsNumber,Move,s,OpMode);
	p->End(rc);
	return rc;
}

extern "C" int WINAPI PutFiles(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int Move,int OpMode)
{
	FPOpMode _op(OpMode);
	FTP*     p = (FTP*)hPlugin;
	p->Call();
	int rc = p->PutFiles(PanelItem,ItemsNumber,Move,OpMode);
	p->End(rc);
	return rc;
}

extern "C" int WINAPI DeleteFiles(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int OpMode)
{
	FPOpMode _op(OpMode);
	FTP*     p = (FTP*)hPlugin;
	p->Call();
	int rc = p->DeleteFiles(PanelItem,ItemsNumber,OpMode);
	p->End(rc);
	return rc;
}

extern "C" int WINAPI MakeDirectory(HANDLE hPlugin,char *Name,int OpMode)
{
	FPOpMode _op(OpMode);

	if(!hPlugin)
		return FALSE;

	FTP*     p = (FTP*)hPlugin;
	String   s(Name ? Name : "");
	p->Call();
	int rc = p->MakeDirectory(s,OpMode);
	p->End(rc);
	return rc;
}

extern "C" int WINAPI ProcessKey(HANDLE hPlugin,int Key,unsigned int ControlState)
{
	FTP*     p = (FTP*)hPlugin;
	p->Call();
	int rc = p->ProcessKey(Key,ControlState);
	p->End(rc);
	return rc;
}

extern "C" int WINAPI ProcessEvent(HANDLE hPlugin,int Event,void *Param)
{
	FTP*     p = (FTP*)hPlugin;
	LastUsedPlugin = p;
#if defined(__FILELOG__)
	static LPCSTR evts[] = { "CHANGEVIEWMODE", "REDRAW", "IDLE", "CLOSE", "BREAK", "COMMAND" };
	PROC(("FAR.ProcessEvent","%p,%s[%08X]",
	      hPlugin,
	      (Event < ARRAYSIZE(evts)) ? evts[Event] : Message("<unk>%d",Event),Param))
#endif
	p->Call();
	int rc = p->ProcessEvent(Event,Param);
	p->End(rc);
	return rc;
}

extern "C" int WINAPI Compare(HANDLE hPlugin,const PluginPanelItem *i,const PluginPanelItem *i1,unsigned int Mode)
{
	if(Mode == SM_UNSORTED)
		return -2;

	FTPHost* p  = FTPHost::Convert(i),
	         *p1 = FTPHost::Convert(i1);
	int      n;

	if(!i || !i1 || !p || !p1)
		return -2;

#define CMP( v,v1 ) (CompareString( LOCALE_USER_DEFAULT,NORM_IGNORECASE|SORT_STRINGSORT,v,-1,v1,-1 ) - 2)

	switch(Mode)
	{
		case   SM_EXT:
			n = CMP(p->Home,p1->Home);
			break;
		case SM_DESCR:
			n = CMP(p->HostDescr,p1->HostDescr);
			break;
		case SM_OWNER:
			n = CMP(p->User,p1->User);
			break;
		case SM_MTIME:
		case SM_CTIME:
		case SM_ATIME:
			n = (int)CompareFileTime(&p1->LastWrite, &p->LastWrite);
			break;
		default:
			n = CMP(p->Host,p1->Host);
			break;
	}

	do
	{
		if(n) break;

		n = CMP(p->Host,p1->Host);

		if(n) break;

		n = CMP(p->User,p1->User);

		if(n) break;

		n = CMP(p->HostDescr,p1->HostDescr);

		if(n) break;
	}
	while(0);

#undef CMP

	if(n)
		return (n>0)?1:(-1);
	else
		return 0;
}

void InitDialogItems(const InitDialogItem *Init,FarDialogItem *Item, int ItemsNumber)
{
	for(int i=0; i<ItemsNumber; i++)
	{
		Item[i].Type=Init[i].Type;
		Item[i].X1=Init[i].X1;
		Item[i].Y1=Init[i].Y1;
		Item[i].X2=Init[i].X2;
		Item[i].Y2=Init[i].Y2;
		Item[i].Focus=Init[i].Focus;
		Item[i].History=(const TCHAR *)Init[i].Selected;
		Item[i].Flags=Init[i].Flags;
		Item[i].DefaultButton=Init[i].DefaultButton;
#ifdef UNICODE
		Item[i].MaxLen=0;
#endif

		if((DWORD_PTR)Init[i].Data<2000)
#ifndef UNICODE
			lstrcpy(Item[i].Data,FP_GetMsg((unsigned int)(DWORD_PTR)Init[i].Data));

#else
			Item[i].PtrData = FP_GetMsg((unsigned int)(DWORD_PTR)Init[i].Data);
#endif
		else
#ifndef UNICODE
			lstrcpy(Item[i].Data,Init[i].Data);

#else
			Item[i].PtrData = Init[i].Data;
#endif
	}
}
