#include <all_far.h>
#pragma hdrstop

#include "Int.h"

int TrimLen(char *m)
{
	int len = 0;

	while(*m && isspace(*m))  m++;

	while(*m && !isspace(*m)) len++,m++;

	return len;
}

//------------------------------------------------------------------------
void WINAPI ReadCfg(void)
{
	char str[ FAR_MAX_NAME ],*m;
	int  val,n;
#define GCMD( fnm,nm,v ) FP_GetRegKey( fnm,Opt.v,nm,ARRAYSIZE(Opt.v) ); if (TrimLen(Opt.v) == 0) strcpy( Opt.v,nm );
	Opt.AddToDisksMenu     = FP_GetRegKey("AddToDisksMenu",     1);
	Opt.AddToPluginsMenu   = FP_GetRegKey("AddToPluginsMenu",   1);
	Opt.DisksMenuDigit     = FP_GetRegKey("DisksMenuDigit",     2);
	Opt.ReadDescriptions   = FP_GetRegKey("ReadDescriptions",   0);
	Opt.UploadLowCase      = FP_GetRegKey("UploadLowCase",      0);
	Opt.ShowUploadDialog   = FP_GetRegKey("ShowUploadDialog",   1);
	Opt.ResumeDefault      = FP_GetRegKey("ResumeDefault",      0);
	Opt.UpdateDescriptions = FP_GetRegKey("UpdateDescriptions", 0);
	Opt.PassiveMode        = FP_GetRegKey("PassiveMode",        0);
	FP_GetRegKey("CharTable",        Opt.Table,NULL,ARRAYSIZE(Opt.Table));
	FP_GetRegKey("DescriptionNames", Opt.DescriptionNames,"00_index.txt,0index,0index.txt",ARRAYSIZE(Opt.DescriptionNames));
	FP_GetRegKey("Firewall",Opt.Firewall,NULL,ARRAYSIZE(Opt.Firewall));
	FP_GetRegKey("DefaultPassword", (BYTE *)str,(BYTE *)NULL,ARRAYSIZE(str));
	DecryptPassword((BYTE*)str,Opt.DefaultPassword);
//JM
	Opt.CmdLength          =       Max(5,Min(FP_ConHeight()-5,FP_GetRegKey("CmdLength",7)));
	Opt.CmdLine            =       Max(10,Min(FP_ConWidth()-9,FP_GetRegKey("CmdLine",70)));
	Opt.IOBuffSize         =       Max(FTR_MINBUFFSIZE,(DWORD)FP_GetRegKey("IOBuffSize",512));
	Opt.dDelimit           =       FP_GetRegKey("DigitDelimit",       TRUE);
	Opt.dDelimiter         = (char)FP_GetRegKey("DigitDelimiter",     0);
	Opt.WaitTimeout        =       FP_GetRegKey("WaitTimeout",        30);
	Opt.AskAbort           =       FP_GetRegKey("AskAbort",           TRUE);
	Opt.WaitIdle           =       FP_GetRegKey("WaitIdle",           1);
	Opt.CmdLogLimit        =       FP_GetRegKey("CmdLogLimit",        100);
	Opt.ShowIdle           =       FP_GetRegKey("ShowIdle",           TRUE);
	Opt.TimeoutRetry       =       FP_GetRegKey("TimeoutRetry",       FALSE);
	Opt.RetryCount         =       FP_GetRegKey("RetryCount",         0);
	Opt.LogOutput          =       FP_GetRegKey("LogOutput",          FALSE);
	Opt._ShowPassword      =       FP_GetRegKey("ShowPassword",       FALSE);
	Opt.IdleColor          =       FP_GetRegKey("IdleColor",          FAR_COLOR(fccCYAN,fccBLUE));
	Opt.IdleMode           =       FP_GetRegKey("IdleMode",           IDLE_CONSOLE);
	Opt.LongBeepTimeout    =       FP_GetRegKey("BeepTimeout",        30);
	Opt.KeepAlive          =       FP_GetRegKey("KeepAlive",          60);
	Opt.IdleShowPeriod     =       FP_GetRegKey("IdleShowPeriod",     700);
	Opt.IdleStartPeriod    =       FP_GetRegKey("IdleStartPeriod",    4000);
	Opt.AskLoginFail       =       FP_GetRegKey("AskLoginFail",       TRUE);
	Opt.ExtCmdView         =       FP_GetRegKey("ExtCmdView",         TRUE);
	Opt.AutoAnonymous      =       FP_GetRegKey("AutoAnonymous",      TRUE);
	Opt.CloseDots          =       FP_GetRegKey("CloseDots",          TRUE);
	Opt.QuoteClipboardNames=       FP_GetRegKey("QuoteClipboardNames",TRUE);
	Opt.SetHiddenOnAbort   =       FP_GetRegKey("SetHiddenOnAbort",   FALSE);
	Opt.PwdSecurity        =       FP_GetRegKey("PwdSecurity",        0);
	Opt.WaitCounter        =       FP_GetRegKey("WaitCounter",        0);
	Opt.RetryTimeout       =       FP_GetRegKey("RetryTimeout",       10);
	Opt.DoNotExpandErrors  =       FP_GetRegKey("DoNotExpandErrors",  FALSE);
	Opt.TruncateLogFile    =       FP_GetRegKey("TruncateLogFile",    FALSE);
	Opt.ServerType         =       FP_GetRegKey("ServerType",         FTP_TYPE_DETECT);
	Opt.UseBackups         =       FP_GetRegKey("UseBackups",         TRUE);
	Opt.ProcessCmd         =       FP_GetRegKey("ProcessCmd",         TRUE);
	Opt.FFDup              =       FP_GetRegKey("FFDup",              FALSE);
	Opt.UndupFF            =       FP_GetRegKey("UndupFF",            FALSE);
	Opt.ShowSilentProgress =       FP_GetRegKey("ShowSilentProgress", FALSE);
	FP_GetRegKey("InvalidSymbols",     Opt.InvalidSymbols,   "<>|?*\"", ARRAYSIZE(Opt.InvalidSymbols));
	FP_GetRegKey("CorrectedSymbols",   Opt.CorrectedSymbols, "()!__\'", ARRAYSIZE(Opt.CorrectedSymbols));

	for(n = 0; Opt.InvalidSymbols[n] && Opt.CorrectedSymbols[n]; n++);

	Opt.InvalidSymbols[n]   = 0;
	Opt.CorrectedSymbols[n] = 0;
	Opt.PluginColumnMode   = (int)FP_GetRegKey("PluginColumnMode", MAX_DWORD);

	if(Opt.PluginColumnMode < 0 || Opt.PluginColumnMode >= 10)
		Opt.PluginColumnMode = -1;

	FP_GetRegKey("CmdLogFile", Opt.CmdLogFile,"",ARRAYSIZE(Opt.CmdLogFile));
//Queue
	Opt.RestoreState           = FP_GetRegKey("QueueRestoreState",    TRUE);
	Opt.RemoveCompleted        = FP_GetRegKey("QueueRemoveCompleted", TRUE);
	Opt.sli.AddPrefix          = FP_GetRegKey("AddPrefix",          TRUE);
	Opt.sli.AddPasswordAndUser = FP_GetRegKey("AddPasswordAndUser", TRUE);
	Opt.sli.Quote              = FP_GetRegKey("Quote",              TRUE);
	Opt.sli.Size               = FP_GetRegKey("Size",               TRUE);
	Opt.sli.RightBound         = FP_GetRegKey("RightBound",         80);
	Opt.sli.ListType           = (sliTypes)FP_GetRegKey("ListType",           sltUrlList);
//Formats
	GCMD("ServerDateFormat", "%*s %04d%02d%02d%02d%02d%02d", fmtDateFormat)
//Months
	static const char *Months[12]= { "Jan","Feb","Mar","Apr","May","Jun",
	                                 "Jul","Aug","Sep","Oct","Nov","Dec"
	                               };

	for(n = 0; n < 12; n++)
	{
		FP_GetRegKey(Months[n], str, Months[n], ARRAYSIZE(str));

		while((m=strpbrk(str,"\n\r\b")) != NULL) *m = 0;

		if(!str[0])
			strcpy(str,Months[n]);

		Log(("month %d [%s]=[%s]",n,Opt.Months[n],str));

		if(Opt.Months[n])
			free(Opt.Months[n]);

		Opt.Months[n] = strdup(str);
	}

//CMD`s
	GCMD("xcmdPUT",  "STOR", cmdPut)
	GCMD("xcmdAPPE", "APPE", cmdAppe)
	GCMD("xcmdSTOR", "STOR", cmdStor)
	GCMD("xcmdSTOU", "STOU", cmdPutUniq)
	GCMD("xcmdPASV", "PASV", cmdPasv)
	GCMD("xcmdPORT", "PORT", cmdPort)
	GCMD("xcmdMDTM", "MDTM", cmdMDTM)
	GCMD("xcmdRETR", "RETR", cmdRetr)
	GCMD("xcmdREST", "REST", cmdRest)
	GCMD("xcmdALLO", "ALLO", cmdAllo)
	GCMD("xcmdCWD",  "CWD",  cmdCwd)
	GCMD("xcmdXCWD", "XCWD", cmdXCwd)
	GCMD("xcmdDELE", "DELE", cmdDel)
	GCMD("xcmdRNFR", "RNFR", cmdRen)
	GCMD("xcmdRNTO", "RNTO", cmdRenTo)
	GCMD("xcmdLIST", "LIST", cmdList)
	GCMD("xcmdNLIST","NLIST",cmdNList)
	GCMD("xcmdUSER", "USER", cmdUser)
	GCMD("xcmdPASS", "PASS", cmdPass)
	GCMD("xcmdACCT", "ACCT", cmdAcct)
	GCMD("xcmdPWD",  "PWD",  cmdPwd)
	GCMD("xcmdXPWD", "XPWD", cmdXPwd)
	GCMD("xcmdMKD",  "MKD",  cmdMkd)
	GCMD("xcmdXMKD", "XMKD", cmdXMkd)
	GCMD("xcmdRMD",  "RMD",  cmdRmd)
	GCMD("xcmdXRMD", "XRMD", cmdXRmd)
	GCMD("xcmdSITE", "SITE", cmdSite)
	GCMD("xcmdCHMOD","CHMOD",cmdChmod)
	GCMD("xcmdUMASK","UMASK",cmdUmask)
	GCMD("xcmdIDLE", "IDLE", cmdIdle)
	GCMD("xcmdHELP", "HELP", cmdHelp)
	GCMD("xcmdQUIT", "QUIT", cmdQuit)
	GCMD("xcmdCDUP", "CDUP", cmdCDUp)
	GCMD("xcmdXCUP", "XCUP", cmdXCDUp)
	GCMD("xcmdSYST", "SYST", cmdSyst)
	GCMD("xcmdSIZE", "SIZE", cmdSize)
	GCMD("xcmdSTAT", "STAT", cmdStat)
//ProcessColor
	val = (int)FP_Info->AdvControl(FP_Info->ModuleNumber,ACTL_GETCOLOR,(void*)COL_DIALOGBOX);
	Opt.ProcessColor = FP_GetRegKey("ProcessColor",val);

//dDelimit && dDelimiter
	if(Opt.dDelimit && Opt.dDelimiter == 0)
	{
		if(GetLocaleInfo(GetThreadLocale(),LOCALE_STHOUSAND,str,ARRAYSIZE(str)))
		{
			CharToOemBuff(str,str,2);
			Opt.dDelimiter = str[0];
		}
		else
			Opt.dDelimiter = '.';
	}
}

//------------------------------------------------------------------------
void WINAPI WriteCfg(void)
{
//Dialog
	FP_SetRegKey("AddToDisksMenu",       Opt.AddToDisksMenu);
	FP_SetRegKey("DisksMenuDigit",       Opt.DisksMenuDigit);
	FP_SetRegKey("AddToPluginsMenu",     Opt.AddToPluginsMenu);
	FP_SetRegKey("PluginColumnMode",     Opt.PluginColumnMode);
	FP_SetRegKey("ReadDescriptions",     Opt.ReadDescriptions);
	FP_SetRegKey("UpdateDescriptions",   Opt.UpdateDescriptions);
	FP_SetRegKey("UploadLowCase",        Opt.UploadLowCase);
	FP_SetRegKey("ShowUploadDialog",     Opt.ShowUploadDialog);
	FP_SetRegKey("ResumeDefault",        Opt.ResumeDefault);
	FP_SetRegKey("WaitTimeout",          Opt.WaitTimeout);
	FP_SetRegKey("ShowIdle",             Opt.ShowIdle);
	FP_SetRegKey("IdleColor",            Opt.IdleColor);
	FP_SetRegKey("IdleMode",             Opt.IdleMode);
	FP_SetRegKey("KeepAlive",            Opt.KeepAlive);
	FP_SetRegKey("IdleShowPeriod",       Opt.IdleShowPeriod);
	FP_SetRegKey("IdleStartPeriod",      Opt.IdleStartPeriod);
	FP_SetRegKey("TimeoutRetry",         Opt.TimeoutRetry);
	FP_SetRegKey("RetryCount",           Opt.RetryCount);
	FP_SetRegKey("BeepTimeout",          Opt.LongBeepTimeout);
	FP_SetRegKey("AskAbort",             Opt.AskAbort);
	FP_SetRegKey("WaitIdle",             Opt.WaitIdle);
	FP_SetRegKey("DigitDelimit",         Opt.dDelimit);
	FP_SetRegKey("DigitDelimiter", (int)Opt.dDelimiter);
	FP_SetRegKey("ExtCmdView",           Opt.ExtCmdView);
	FP_SetRegKey("CmdLine",              Opt.CmdLine);
	FP_SetRegKey("CmdLength",            Opt.CmdLength);
	FP_SetRegKey("IOBuffSize",           Opt.IOBuffSize);
	FP_SetRegKey("ProcessColor",         Opt.ProcessColor);
	FP_SetRegKey("ServerType",           Opt.ServerType);
	FP_SetRegKey("ProcessCmd",           Opt.ProcessCmd);
	FP_SetRegKey("QueueRestoreState",    Opt.RestoreState);
	FP_SetRegKey("QueueRemoveCompleted", Opt.RemoveCompleted);
	FP_SetRegKey("DescriptionNames",     Opt.DescriptionNames);
	BYTE CryptedPassword[FTP_PWD_LEN];
	MakeCryptPassword(Opt.DefaultPassword,CryptedPassword);
	FP_SetRegKey("DefaultPassword", CryptedPassword,ARRAYSIZE(CryptedPassword)-1);
	FP_SetRegKey("Firewall",            Opt.Firewall);
	FP_SetRegKey("CmdLogLimit",         Opt.CmdLogLimit);
	FP_SetRegKey("CmdLogFile",          Opt.CmdLogFile);
	FP_SetRegKey("LogOutput",           Opt.LogOutput);
	FP_SetRegKey("PassiveMode",         Opt.PassiveMode);
	FP_SetRegKey("AutoAnonymous",        Opt.AutoAnonymous);
	FP_SetRegKey("CloseDots",            Opt.CloseDots);
	FP_SetRegKey("QuoteClipboardNames",  Opt.QuoteClipboardNames);
	FP_SetRegKey("SetHiddenOnAbort",     Opt.SetHiddenOnAbort);
	FP_SetRegKey("PwdSecurity",          Opt.PwdSecurity);
	FP_SetRegKey("WaitCounter",          Opt.WaitCounter);
	FP_SetRegKey("RetryTimeout",         Opt.RetryTimeout);
	FP_SetRegKey("DoNotExpandErrors",    Opt.DoNotExpandErrors);
	FP_SetRegKey("TruncateLogFile",      Opt.TruncateLogFile);
	FP_SetRegKey("ServerType",           Opt.ServerType);
	FP_SetRegKey("UseBackups",           Opt.UseBackups);
	FP_SetRegKey("ProcessCmd",           Opt.ProcessCmd);
	FP_SetRegKey("FFDup",                Opt.FFDup);
	FP_SetRegKey("UndupFF",              Opt.UndupFF);
	FP_SetRegKey("ShowSilentProgress",   Opt.ShowSilentProgress);
	FP_SetRegKey("AskLoginFail",         Opt.AskLoginFail);
	FP_SetRegKey("InvalidSymbols",      Opt.InvalidSymbols);
	FP_SetRegKey("CorrectedSymbols",    Opt.CorrectedSymbols);
//Queue
	FP_SetRegKey("QueueRestoreState",    Opt.RestoreState);
	FP_SetRegKey("QueueRemoveCompleted", Opt.RemoveCompleted);
	FP_SetRegKey("AddPrefix",            Opt.sli.AddPrefix);
	FP_SetRegKey("AddPasswordAndUser",   Opt.sli.AddPasswordAndUser);
	FP_SetRegKey("Quote",                Opt.sli.Quote);
	FP_SetRegKey("Size",                 Opt.sli.Size);
	FP_SetRegKey("RightBound",           Opt.sli.RightBound);
	FP_SetRegKey("ListType",             Opt.sli.ListType);
}
//------------------------------------------------------------------------
void ExtendedConfig(void)
{
	InitDialogItem InitItems[]=
	{
		{DI_DOUBLEBOX, 3, 1,72,17, 0,0,0,0, FMSG(METitle)},
		{DI_CHECKBOX,5, 2,0,0,0,  0,0,0,  FMSG(MDupFF)},
		{DI_CHECKBOX,5, 3,0,0,0,  0,0,0,  FMSG(MUndupFF)},
		{DI_CHECKBOX,5, 4,0,0,0,  0,0,0,  FMSG(MEShowProgress)},
		{DI_CHECKBOX,5, 5,0,0,0,  0,0,0,  FMSG(MEBackup)},
		{DI_CHECKBOX,5, 6,0,0,0,  0,0,0,  FMSG(MESendCmd)},
		{DI_CHECKBOX,5, 7,0,0,0,  0,0,0,  FMSG(MEDontError)},
		{DI_CHECKBOX,5, 8,0,0,0,  0,0,0,  FMSG(MEAskLoginAtFail)},
		{DI_CHECKBOX,5, 9,0,0,0,  0,0,0,  FMSG(MEAutoAn)},
		{DI_CHECKBOX,5,10,0,0,0,  0,0,0,  FMSG(MECloseDots)},
		{DI_CHECKBOX,5,11,0,0,0,  0,0,0,  FMSG(MQuoteClipboardNames)},
		{DI_CHECKBOX,5,12,0,0,0,   0,0,0, FMSG(MSetHiddenOnAbort)},

		{DI_TEXT,3,15,3,15,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,NULL },

		{DI_BUTTON,0,16,0,0,0,0,DIF_CENTERGROUP, 1,   FMSG(MOk)},
		{DI_BUTTON,0,16,0,0,0,0,DIF_CENTERGROUP, 0,   FMSG(MCancel)},
	};
	enum
	{
		dDupFF        = 1,
		dUndupFF      = 2,
		dShowProgress = 3,
		dBackup       = 4,
		dSendCmd      = 5,
		dDontErr      = 6,
		dAskFail      = 7,
		dAutoAnn      = 8,
		dCloseDots    = 9,
		dQuoteCN      = 10,
		dSetHiddenOnAbort = 11,

		dOk           = 13
	};
	int           rc;
	FarDialogItem DialogItems[ARRAYSIZE(InitItems)];
	InitDialogItems(InitItems,DialogItems,ARRAYSIZE(DialogItems));
	DialogItems[dDupFF].Selected        = Opt.FFDup;
	DialogItems[dUndupFF].Selected      = Opt.UndupFF;
	DialogItems[dShowProgress].Selected = Opt.ShowSilentProgress;
	DialogItems[dBackup].Selected       = Opt.UseBackups;
	DialogItems[dSendCmd].Selected      = Opt.ProcessCmd;
	DialogItems[dDontErr].Selected      = Opt.DoNotExpandErrors;
	DialogItems[dAskFail].Selected      = Opt.AskLoginFail;
	DialogItems[dAutoAnn].Selected      = Opt.AutoAnonymous;
	DialogItems[dCloseDots].Selected    = Opt.CloseDots;
	DialogItems[dQuoteCN].Selected      = Opt.QuoteClipboardNames;
	DialogItems[dSetHiddenOnAbort].Selected  = Opt.SetHiddenOnAbort;

	do
	{
		rc = FDialog(76,19,"FTPExtGlobal",DialogItems,ARRAYSIZE(DialogItems));

		if(rc == -1)
			return;

		if(rc != dOk)
			return;

		break;
	}
	while(true);

	Opt.FFDup              = DialogItems[dDupFF].Selected;
	Opt.UndupFF            = DialogItems[dUndupFF].Selected;
	Opt.ShowSilentProgress = DialogItems[dShowProgress].Selected;
	Opt.UseBackups         = DialogItems[dBackup].Selected;
	Opt.ProcessCmd         = DialogItems[dSendCmd].Selected;
	Opt.DoNotExpandErrors  = DialogItems[dDontErr].Selected;
	Opt.AskLoginFail       = DialogItems[dAskFail].Selected;
	Opt.AutoAnonymous      = DialogItems[dAutoAnn].Selected;
	Opt.CloseDots          = DialogItems[dCloseDots].Selected;
	Opt.QuoteClipboardNames= DialogItems[dQuoteCN].Selected;
	Opt.SetHiddenOnAbort   = DialogItems[dSetHiddenOnAbort].Selected;
}

//------------------------------------------------------------------------

struct FLngColorDialog
{
	LPCSTR MTitle;         ///< Message for dialog title (Default text: "Colors").
	LPCSTR MFore;          ///< Message for foreground label (Default text: "Fore").
	LPCSTR MBk;            ///< Message for background label (Default text: "Back").
	LPCSTR MSet;           ///< Message for set button (Default text: "Set").
	LPCSTR MCancel;        ///< Message for cancel button (Default text: "Cancel").
	LPCSTR MText;          ///< Message for sample text (Default text: "text text text").
};

static FLngColorDialog ColorLangs =
{
	/*MTitle*/    FMSG(MColorTitle),
	/*MFore*/     FMSG(MColorFore),
	/*MBk*/       FMSG(MColorBk),
	/*MSet*/      FMSG(MColorColorSet),
	/*MCancel*/   FMSG(MCancel),
	/*MText*/     FMSG(MColorText)
};

//--------------------------------------------------------------------------------
//-- Color dialog
//--------------------------------------------------------------------------------
#define cdlgFORE 2
#define cdlgBK   19
#define cdlgTEXT 35
#define cdlgOK   39


static int  ColorFore;
static int  ColorBk;
static char Title[FAR_MAX_CAPTION];

static LONG_PTR WINAPI CDLG_WndProc(HANDLE hDlg,int Msg,int Param1,LONG_PTR Param2)
{
	FarDialogItemData id;
	char str[FAR_MAX_CAPTION+50];

	switch(Msg)
	{
		case        DN_BTNCLICK:

			if(Param1 >= cdlgFORE && Param1 < cdlgFORE+16)
				ColorFore = Param1 - cdlgFORE;
			else if(Param1 >= cdlgBK && Param1 < cdlgBK+16)
				ColorBk = Param1 - cdlgBK;

			sprintf(str,"%s(%3d 0x%02X %03o)",
			        Title,
			        FAR_COLOR(ColorFore,ColorBk),
			        FAR_COLOR(ColorFore,ColorBk),
			        FAR_COLOR(ColorFore,ColorBk));
			//set caption
			id.PtrLength = static_cast<int>(strlen(str));
			id.PtrData   = str;
			FP_Info->SendDlgMessage(hDlg,DM_SETTEXT,0,(LONG_PTR)(&id));
			//Invalidate
			FP_Info->SendDlgMessage(hDlg,DM_SETREDRAW,0,0);
			break;
		case DN_CTLCOLORDLGITEM:

			if(Param1 >= cdlgTEXT && Param1 < cdlgTEXT+3)
				return FAR_COLOR(ColorFore,ColorBk);

			break;
	}

	return FP_Info->DefDlgProc(hDlg,Msg,Param1,Param2);
}

int WINAPI FP_GetColorDialog(int color,FLngColorDialog* p,LPCSTR Help)
{
	InitDialogItem InitItems[]=
	{
		{DI_DOUBLEBOX,3, 1,35,13,0,0,DIF_BOXCOLOR, 0,NULL},
		{DI_SINGLEBOX,5, 2,18, 7,0,0,DIF_BOXCOLOR|DIF_LEFTTEXT,0, NULL},
		{DI_RADIOBUTTON,6,3,0,0,0,0,DIF_GROUP|DIF_MOVESELECT|DIF_SETCOLOR|(FAR_COLOR((~0)&0x7, 0)),0,NULL},
		{DI_RADIOBUTTON,6,4,0,0,0,0,DIF_MOVESELECT|DIF_SETCOLOR|(FAR_COLOR((~1)&0x7, 1)),0,NULL},
		{DI_RADIOBUTTON,6,5,0,0,0,0,DIF_MOVESELECT|DIF_SETCOLOR|(FAR_COLOR((~2)&0x7, 2)),0,NULL},
		{DI_RADIOBUTTON,6,6,0,0,0,0,DIF_MOVESELECT|DIF_SETCOLOR|(FAR_COLOR((~3)&0x7, 3)),0,NULL},
		{DI_RADIOBUTTON,9,3,0,0,0,0,DIF_MOVESELECT|DIF_SETCOLOR|(FAR_COLOR((~4)&0x7, 4)),0,NULL},
		{DI_RADIOBUTTON,9,4,0,0,0,0,DIF_MOVESELECT|DIF_SETCOLOR|(FAR_COLOR((~5)&0x7, 5)),0,NULL},
		{DI_RADIOBUTTON,9,5,0,0,0,0,DIF_MOVESELECT|DIF_SETCOLOR|(FAR_COLOR((~6)&0x7, 6)),0,NULL},
		{DI_RADIOBUTTON,9,6,0,0,0,0,DIF_MOVESELECT|DIF_SETCOLOR|(FAR_COLOR((~7)&0x7, 7)),0,NULL},
		{DI_RADIOBUTTON,12,3,0,0,0,0,DIF_MOVESELECT|DIF_SETCOLOR|(FAR_COLOR((~8)&0x7, 8)),0,NULL},
		{DI_RADIOBUTTON,12,4,0,0,0,0,DIF_MOVESELECT|DIF_SETCOLOR|(FAR_COLOR((~9)&0x7, 9)),0,NULL},
		{DI_RADIOBUTTON,12,5,0,0,0,0,DIF_MOVESELECT|DIF_SETCOLOR|(FAR_COLOR((~10)&0x7,10)),0,NULL},
		{DI_RADIOBUTTON,12,6,0,0,0,0,DIF_MOVESELECT|DIF_SETCOLOR|(FAR_COLOR((~11)&0x7,11)),0,NULL},
		{DI_RADIOBUTTON,15,3,0,0,0,0,DIF_MOVESELECT|DIF_SETCOLOR|(FAR_COLOR((~12)&0x7,12)),0,NULL},
		{DI_RADIOBUTTON,15,4,0,0,0,0,DIF_MOVESELECT|DIF_SETCOLOR|(FAR_COLOR((~13)&0x7,13)),0,NULL},
		{DI_RADIOBUTTON,15,5,0,0,0,0,DIF_MOVESELECT|DIF_SETCOLOR|(FAR_COLOR((~14)&0x7,14)),0,NULL},
		{DI_RADIOBUTTON,15,6,0,0,0,0,DIF_MOVESELECT|DIF_SETCOLOR|(FAR_COLOR((~15)&0x7,15)),0,NULL},
		{DI_SINGLEBOX,20, 2,33, 7,0,0,DIF_BOXCOLOR|DIF_LEFTTEXT, 0,NULL},
		{DI_RADIOBUTTON,21,3,0,0,0,0,DIF_GROUP|DIF_MOVESELECT|DIF_SETCOLOR|(FAR_COLOR((~0)&0x7, 0)),0,NULL},
		{DI_RADIOBUTTON,21,4,0,0,0,0,DIF_MOVESELECT|DIF_SETCOLOR|(FAR_COLOR((~1)&0x7, 1)),0,NULL},
		{DI_RADIOBUTTON,21,5,0,0,0,0,DIF_MOVESELECT|DIF_SETCOLOR|(FAR_COLOR((~2)&0x7, 2)),0,NULL},
		{DI_RADIOBUTTON,21,6,0,0,0,0,DIF_MOVESELECT|DIF_SETCOLOR|(FAR_COLOR((~3)&0x7, 3)),0,NULL},
		{DI_RADIOBUTTON,24,3,0,0,0,0,DIF_MOVESELECT|DIF_SETCOLOR|(FAR_COLOR((~4)&0x7, 4)),0,NULL},
		{DI_RADIOBUTTON,24,4,0,0,0,0,DIF_MOVESELECT|DIF_SETCOLOR|(FAR_COLOR((~5)&0x7, 5)),0,NULL},
		{DI_RADIOBUTTON,24,5,0,0,0,0,DIF_MOVESELECT|DIF_SETCOLOR|(FAR_COLOR((~6)&0x7, 6)),0,NULL},
		{DI_RADIOBUTTON,24,6,0,0,0,0,DIF_MOVESELECT|DIF_SETCOLOR|(FAR_COLOR((~7)&0x7, 7)),0,NULL},
		{DI_RADIOBUTTON,27,3,0,0,0,0,DIF_MOVESELECT|DIF_SETCOLOR|(FAR_COLOR((~8)&0x7, 8)),0,NULL},
		{DI_RADIOBUTTON,27,4,0,0,0,0,DIF_MOVESELECT|DIF_SETCOLOR|(FAR_COLOR((~9)&0x7, 9)),0,NULL},
		{DI_RADIOBUTTON,27,5,0,0,0,0,DIF_MOVESELECT|DIF_SETCOLOR|(FAR_COLOR((~10)&0x7,10)),0,NULL},
		{DI_RADIOBUTTON,27,6,0,0,0,0,DIF_MOVESELECT|DIF_SETCOLOR|(FAR_COLOR((~11)&0x7,11)),0,NULL},
		{DI_RADIOBUTTON,30,3,0,0,0,0,DIF_MOVESELECT|DIF_SETCOLOR|(FAR_COLOR((~12)&0x7,12)),0,NULL},
		{DI_RADIOBUTTON,30,4,0,0,0,0,DIF_MOVESELECT|DIF_SETCOLOR|(FAR_COLOR((~13)&0x7,13)),0,NULL},
		{DI_RADIOBUTTON,30,5,0,0,0,0,DIF_MOVESELECT|DIF_SETCOLOR|(FAR_COLOR((~14)&0x7,14)),0,NULL},
		{DI_RADIOBUTTON,30,6,0,0,0,0,DIF_MOVESELECT|DIF_SETCOLOR|(FAR_COLOR((~15)&0x7,15)),0,NULL},
		{DI_TEXT,5, 8,0,0,0,0,DIF_SETCOLOR|FAR_COLOR(fccYELLOW,fccGREEN), 0,NULL},
		{DI_TEXT,5, 9,0,0,0,0,DIF_SETCOLOR|FAR_COLOR(fccYELLOW,fccGREEN), 0,NULL},
		{DI_TEXT,5,10,0,0,0,0,DIF_SETCOLOR|FAR_COLOR(fccYELLOW,fccGREEN), 0,NULL},
		{DI_TEXT,3,11,3,11,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,NULL },
		{DI_BUTTON,0,12,0,0,0,0,DIF_CENTERGROUP, 1, NULL},
		{DI_BUTTON,0,12,0,0,0,0,DIF_CENTERGROUP, 0,NULL},
	};
	FarDialogItem DialogItems[ARRAYSIZE(InitItems)];
	static FLngColorDialog base =
	{
		FMSG("Color"),
		FMSG("&Foreground"),
		FMSG("&Background"),
		FMSG("&Set"),
		FMSG("&Cancel"),
		FMSG("Text Text Text Text Text Text")
	};

	if(!p) p = &base;

	char str[FAR_MAX_CAPTION+50];
	int  n;
	InitDialogItems(InitItems,DialogItems,ARRAYSIZE(DialogItems));
	StrCpy(DialogItems[ 0].Data,         FP_GetMsg(p->MTitle?p->MTitle:base.MTitle),    FAR_MAX_CAPTION);
	StrCpy(DialogItems[ 1].Data,         FP_GetMsg(p->MFore?p->MFore:base.MFore),       FAR_MAX_CAPTION);
	StrCpy(DialogItems[18].Data,         FP_GetMsg(p->MBk?p->MBk:base.MBk),             FAR_MAX_CAPTION);
	StrCpy(DialogItems[39].Data,         FP_GetMsg(p->MSet?p->MSet:base.MSet),          FAR_MAX_CAPTION);
	StrCpy(DialogItems[40].Data,         FP_GetMsg(p->MCancel?p->MCancel:base.MCancel), FAR_MAX_CAPTION);
	StrCpy(DialogItems[cdlgTEXT].Data,   FP_GetMsg(p->MText?p->MText:base.MText),       FAR_MAX_CAPTION);
	strcpy(DialogItems[cdlgTEXT+1].Data, DialogItems[cdlgTEXT].Data);
	strcpy(DialogItems[cdlgTEXT+2].Data, DialogItems[cdlgTEXT].Data);
	ColorFore = FAR_COLOR_FORE(color);
	ColorBk   = FAR_COLOR_BK(color);

	for(n = 0; n < 16; n++)
	{
		DialogItems[cdlgFORE+n].Selected = FALSE;
		DialogItems[cdlgBK+n].Selected   = FALSE;
		DialogItems[cdlgFORE+n].Focus    = FALSE;
		DialogItems[cdlgBK+n].Focus      = FALSE;
	}

	DialogItems[ cdlgFORE+ColorFore ].Selected = TRUE;
	DialogItems[ cdlgFORE+ColorFore ].Focus = TRUE;
	DialogItems[ cdlgBK+ColorBk ].Selected     = TRUE;
	StrCpy(Title,DialogItems[0].Data,ARRAYSIZE(Title));
	sprintf(str,"(%3d 0x%02X %03o)",
	        FAR_COLOR(ColorFore,ColorBk),
	        FAR_COLOR(ColorFore,ColorBk),
	        FAR_COLOR(ColorFore,ColorBk));
	StrCat(DialogItems[0].Data,str,512);
	n = FP_Info->DialogEx(FP_Info->ModuleNumber,-1,-1,39,15,Help,DialogItems,ARRAYSIZE(DialogItems),0,0,CDLG_WndProc,0);

	if(n == cdlgOK)
		color = FAR_COLOR(ColorFore,ColorBk);

	StrCpy(DialogItems[0].Data,Title,FAR_MAX_CAPTION);
	return color;
}

int WINAPI Config(void)
{
	InitDialogItem InitItems[]=
	{
		{DI_DOUBLEBOX, 3, 1,72,21, 0,0,0,0, FMSG(MConfigTitle)},
		{DI_CHECKBOX,5, 2,0,0,0, 0,0,0,  FMSG(MConfigAddToDisksMenu)},      //Add to Disks menu
		{DI_FIXEDIT,35, 2,37, 2,0,0,0,0,NULL},
		{DI_CHECKBOX,5, 3,0,0,0, 0,0,0,  FMSG(MConfigAddToPluginsMenu)},    //Add to Plugins menu
		{DI_TEXT,9, 4,0,0,0, 0,0,0,  FMSG(MHostsMode)},                 //Hosts panel mode
		{DI_FIXEDIT,5, 4, 7, 4,0,0,0,0,NULL},
		{DI_CHECKBOX,5, 5,0,0,0,  0,0,0, FMSG(MConfigReadDiz)},             //Read descriptions
		{DI_CHECKBOX,5, 6,0,0,0, 0,0,0,  FMSG(MConfigUpdateDiz)},           //Update descriptions
		{DI_CHECKBOX,5, 7,0,0,0, 0,0,0,  FMSG(MConfigUploadLowCase)},       //Upload upper in lowercase
		{DI_CHECKBOX,5, 8,0,0,0, 0,0,0,  FMSG(MConfigUploadDialog)},        //Show upload options dialog
		{DI_CHECKBOX,5, 9,0,0,0, 0,0,0,  FMSG(MConfigDefaultResume)},       //Default button is 'Resume'
		{DI_CHECKBOX,5,10,0,0,0, 0,0,0,  FMSG(MAskAbort)},                  //Confirm abort
		{DI_CHECKBOX,5,11,0,0,0, 0,0,0,  FMSG(MShowIdle)},                  //Show idle
		{DI_BUTTON,21,11,0,0,0, 0,0,0,  FMSG(MColor)},                    //Color
		{DI_RADIOBUTTON,32,11,0,0,0,0,DIF_GROUP, 0,  FMSG(MScreen)},                   //( ) 1.Screen
		{DI_RADIOBUTTON,44,11,0,0,0,  0,0,0, FMSG(MCaption)},                  //( ) 2.Caption
		{DI_RADIOBUTTON,58,11,0,0,0,  0,0,0, FMSG(MBoth)},                     //( ) 3.Both

		{DI_CHECKBOX,40, 2,0,0,0, 0,0,0,  FMSG(MKeepAlive)},                //Keepalive packet
		{DI_EDIT,67, 2,70, 2,0,0,0,0,NULL},
		{DI_TEXT,71, 2,0,0,0, 0,0,0,  FMSG(MSec)},                      //s
		{DI_CHECKBOX,40, 3,0,0,0, 0,0,0,  FMSG(MAutoRetry)},                //AutoRetry
		{DI_EDIT,67, 3,70, 3,0,0,0,0,NULL},
		{DI_CHECKBOX,40, 4,0,0,0, 0,0,0,  FMSG(MLongOp)},                   //Long operation beep
		{DI_EDIT,67, 4,70, 4,0,0,0,0,NULL},
		{DI_TEXT,71, 4,0,0,0,  0,0,0, FMSG(MSec)},                      //s
		{DI_TEXT,40, 5,0,0,0, 0,0,0,  FMSG(MWaitTimeout)},              //Server reply timeout (s)
		{DI_EDIT,67, 5,70, 5,0,0,0,0,NULL},
		{DI_CHECKBOX,40, 6,0,0,0, 0,0,0,  FMSG(MDigitDelimit)},             //Digits grouping symbol
		{DI_EDIT,69, 6,70, 6,0,0,0,0,NULL},
		{DI_CHECKBOX,40, 7,0,0,0, 0,0,0,  FMSG(MExtWindow)},                //Show FTP command log
		{DI_TEXT,40, 8,0,0,0,  0,0,0, FMSG(MExtSize)},                  //Log window size
		{DI_EDIT,60, 8,63, 8,0,0,0,0,NULL},
		{DI_TEXT,64, 8,0,0,0,0,0,0,   " x "},
		{DI_EDIT,67, 8,70, 8,0,0,0,0,NULL},
		{DI_TEXT,40, 9,0,0,0,  0,0,0, FMSG(MHostIOSize)},               //I/O buffer size
		{DI_EDIT,60, 9,70, 9,0,0,0,0,NULL},
		{DI_TEXT,40,10,0,0,0,  0,0,0, FMSG(MSilentText)},               //Alert text
		{DI_BUTTON,60,10,0,0,0,0,0,0,   FMSG(MColor)},

		{DI_TEXT,5,12,5,12,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,NULL },

		{DI_TEXT,5,13,0,0,0, 0,0,0,  FMSG(MConfigDizNames)},            //Dis names
		{DI_EDIT,5,14,70,14,0,0,0,0,NULL},
		{DI_TEXT,5,15,0,0,0, 0,0,0,  FMSG(MConfigDefPassword)},         //Def pass
		{DI_PSWEDIT,5,16,34,16,0,0,0,0,NULL},
		{DI_TEXT,5,17,0,0,0, 0,0,0,  FMSG(MConfigFirewall)},            //Firewall
		{DI_EDIT,5,18,34,18,0,0,0,0,NULL},
		{DI_TEXT,40,15,0,0,0, 0,0,0,  FMSG(MLogFilename)},              //Log filename
		{DI_EDIT,66,15,70,15,0,0,0,0,NULL},
		{DI_TEXT,71,15,0,0,0, 0,0,0,  FMSG(MKBytes)},
		{DI_EDIT,40,16,70,16,0,0,0,0,NULL},
		{DI_CHECKBOX,40,17,0,0,0,  0,0,0, FMSG(MLogDir)},                   //Log DIR contents
		{DI_CHECKBOX,40,18,0,0,0, 0,0,0,  FMSG(MConfigPassiveMode)},
		{DI_TEXT,5,19,5,19,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,NULL },
		{DI_BUTTON,0,20,0,0,0,0,DIF_CENTERGROUP, 1,  FMSG(MOk)},
		{DI_BUTTON,0,20,0,0,0,0,DIF_CENTERGROUP, 0,  FMSG(MCancel)},
		{DI_BUTTON,0,20,0,0,0,0,DIF_CENTERGROUP, 0,  FMSG(MExtOpt)},
	};
#define CFG_ADDDISK       1
#define CFG_DIGIT         2
#define CFG_ADDPLUGINS    3
#define CFG_HOSTMODE      5
#define CFG_READDIZ       6
#define CFG_UPDDIZ        7
#define CFG_UPCASE        8
#define CFG_SHOWUP        9
#define CFG_RESDEF        10
#define CFG_ASKABORT      11
#define CFG_SHOWIDLE      12
#define CFG_IDLECOLOR     13
#define CFG_IDLE_SCREEN   14
#define CFG_IDLE_CAPTION  15
#define CFG_IDLE_BOTH     16
#define CFG_KEEPALIVE     17
#define CFG_KEEPTIME      18
#define CFG_AUTOR         20
#define CFG_AUTORTIME     21
#define CFG_LONGOP        22
#define CFG_LONGOPTIME    23
#define CFG_WAITTIMEOUT   26
#define CFG_DIGDEL        27
#define CFG_DIGCHAR       28
#define CFG_EXT           29
#define CFG_EXT_W         31
#define CFG_EXT_H         33
#define CFG_BUFFSIZE      35
#define CFG_SILENT        37
#define CFG_DESC          40
#define CFG_PASS          42
#define CFG_FIRE          44
#define CFG_LOGLIMIT      46
#define CFG_LOGFILE       48
#define CFG_LOGDIR        49
#define CFG_PASV          50
#define CFG_OK            52
#define CFG_CANCEL        53
#define CFG_EXTBTN        54
	FarDialogItem DialogItems[ARRAYSIZE(InitItems)];
	int           IdleColor    = Opt.IdleColor,
	              ProcessColor = Opt.ProcessColor;
	int           rc;
	InitDialogItems(InitItems,DialogItems,ARRAYSIZE(DialogItems));
	DialogItems[CFG_ADDDISK].Selected = Opt.AddToDisksMenu;
	sprintf(DialogItems[CFG_DIGIT].Data,"%d",Opt.DisksMenuDigit);
	DialogItems[CFG_ADDPLUGINS].Selected  = Opt.AddToPluginsMenu;
	sprintf(DialogItems[CFG_HOSTMODE].Data,"%d",Opt.PluginColumnMode);
	DialogItems[CFG_READDIZ].Selected     = Opt.ReadDescriptions;
	DialogItems[CFG_UPDDIZ].Selected      = Opt.UpdateDescriptions;
	DialogItems[CFG_UPCASE].Selected      = Opt.UploadLowCase;
	DialogItems[CFG_SHOWUP].Selected      = Opt.ShowUploadDialog;
	DialogItems[CFG_RESDEF].Selected      = Opt.ResumeDefault;
	sprintf(DialogItems[CFG_WAITTIMEOUT].Data,"%d",Opt.WaitTimeout);
	DialogItems[CFG_SHOWIDLE].Selected    = Opt.ShowIdle;
	DialogItems[CFG_IDLE_SCREEN].Selected  = Opt.IdleMode == IDLE_CONSOLE;
	DialogItems[CFG_IDLE_CAPTION].Selected = Opt.IdleMode == IDLE_CAPTION;
	DialogItems[CFG_IDLE_BOTH].Selected    = Opt.IdleMode == (IDLE_CONSOLE|IDLE_CAPTION);
	DialogItems[CFG_KEEPALIVE].Selected   = Opt.KeepAlive != 0;
	sprintf(DialogItems[CFG_KEEPTIME].Data,"%d",Opt.KeepAlive);
	DialogItems[CFG_AUTOR].Selected       = Opt.TimeoutRetry;
	sprintf(DialogItems[CFG_AUTORTIME].Data,"%d",Opt.RetryCount);
	DialogItems[CFG_LONGOP].Selected      = Opt.LongBeepTimeout != 0;
	sprintf(DialogItems[CFG_LONGOPTIME].Data,"%d",Opt.LongBeepTimeout);
	DialogItems[CFG_ASKABORT].Selected    = Opt.AskAbort;
	DialogItems[CFG_DIGDEL].Selected      = Opt.dDelimit;
	sprintf(DialogItems[CFG_DIGCHAR].Data,"%c",Opt.dDelimiter);
	DialogItems[CFG_EXT].Selected         = Opt.ExtCmdView;
	sprintf(DialogItems[CFG_EXT_W].Data,"%d",Opt.CmdLine);
	sprintf(DialogItems[CFG_EXT_H].Data,"%d",Opt.CmdLength);
	Size2Str(DialogItems[CFG_BUFFSIZE].Data,Opt.IOBuffSize);
	strcpy(DialogItems[CFG_DESC].Data,Opt.DescriptionNames);
	strcpy(DialogItems[CFG_PASS].Data,Opt.DefaultPassword);
	strcpy(DialogItems[CFG_FIRE].Data,Opt.Firewall);
	sprintf(DialogItems[CFG_LOGLIMIT].Data,"%d",Opt.CmdLogLimit);
	sprintf(DialogItems[CFG_LOGFILE].Data,"%s",Opt.CmdLogFile);
	DialogItems[CFG_LOGDIR].Selected      = Opt.LogOutput;
	DialogItems[CFG_PASV].Selected        = Opt.PassiveMode;

	do
	{
		rc = FDialog(76,23,"Config",DialogItems,ARRAYSIZE(DialogItems));

		if(rc == CFG_OK)
			break;

		if(rc == -1 || rc == CFG_CANCEL)
			return FALSE;

		if(rc == CFG_IDLECOLOR)
			IdleColor = FP_GetColorDialog(IdleColor,&ColorLangs,NULL);
		else if(rc == CFG_SILENT)
			ProcessColor = FP_GetColorDialog(ProcessColor,&ColorLangs,NULL);
		else if(rc == CFG_EXTBTN)
			ExtendedConfig();
	}
	while(true);

//Set to OPT
	Opt.IdleColor          = IdleColor;
	Opt.ProcessColor       = ProcessColor;
	Opt.AddToDisksMenu     = DialogItems[CFG_ADDDISK].Selected;
	Opt.DisksMenuDigit     = atoi(DialogItems[CFG_DIGIT].Data);
	Opt.AddToPluginsMenu   = DialogItems[CFG_ADDPLUGINS].Selected;
	Opt.PluginColumnMode   = atoi(DialogItems[CFG_HOSTMODE].Data);
	Opt.ReadDescriptions   = DialogItems[CFG_READDIZ].Selected;
	Opt.UpdateDescriptions = DialogItems[CFG_UPDDIZ].Selected;
	Opt.UploadLowCase      = DialogItems[CFG_UPCASE].Selected;
	Opt.ShowUploadDialog   = DialogItems[CFG_SHOWUP].Selected;
	Opt.ResumeDefault      = DialogItems[CFG_RESDEF].Selected;
	Opt.WaitTimeout        = atoi(DialogItems[CFG_WAITTIMEOUT].Data);
	Opt.ShowIdle           = DialogItems[CFG_SHOWIDLE].Selected;

	if(DialogItems[CFG_IDLE_SCREEN].Selected)  Opt.IdleMode = IDLE_CONSOLE;
	else if(DialogItems[CFG_IDLE_CAPTION].Selected) Opt.IdleMode = IDLE_CAPTION;
	else
		Opt.IdleMode = IDLE_CONSOLE | IDLE_CAPTION;

	if(DialogItems[CFG_KEEPALIVE].Selected)
		Opt.KeepAlive = atoi(DialogItems[CFG_KEEPTIME].Data);
	else
		Opt.KeepAlive = 0;

	Opt.TimeoutRetry = DialogItems[CFG_AUTOR].Selected;
	Opt.RetryCount   = atoi(DialogItems[CFG_AUTORTIME].Data);

	if(DialogItems[CFG_LONGOP].Selected)
		Opt.LongBeepTimeout = atoi(DialogItems[CFG_LONGOPTIME].Data);
	else
		Opt.LongBeepTimeout = 0;

	Opt.AskAbort           = DialogItems[CFG_ASKABORT].Selected;
	Opt.dDelimit           = DialogItems[CFG_DIGDEL].Selected;
	Opt.dDelimiter         = DialogItems[CFG_DIGCHAR].Data[0];
	Opt.ExtCmdView         = DialogItems[CFG_EXT].Selected;
	Opt.CmdLine            = Max(10,Min(FP_ConWidth()-9,atoi(DialogItems[CFG_EXT_W].Data)));
	Opt.CmdLength          = Max(5,Min(FP_ConHeight()-5,atoi(DialogItems[CFG_EXT_H].Data)));
	Opt.IOBuffSize         = Max((DWORD)FTR_MINBUFFSIZE,Str2Size(DialogItems[CFG_BUFFSIZE].Data));
	strcpy(Opt.DescriptionNames,DialogItems[CFG_DESC].Data);
	strcpy(Opt.DefaultPassword,DialogItems[CFG_PASS].Data);
	strcpy(Opt.Firewall,DialogItems[CFG_FIRE].Data);
	Opt.CmdLogLimit        = atoi(DialogItems[CFG_LOGLIMIT].Data);
	strcpy(Opt.CmdLogFile, DialogItems[CFG_LOGFILE].Data);
	Opt.LogOutput          = DialogItems[CFG_LOGDIR].Selected;
	Opt.PassiveMode        = DialogItems[CFG_PASV].Selected;
//Write to REG
	WriteCfg();

	if(FTPPanels[0]) FTPPanels[0]->Invalidate();

	if(FTPPanels[1]) FTPPanels[1]->Invalidate();

	return TRUE;
}
