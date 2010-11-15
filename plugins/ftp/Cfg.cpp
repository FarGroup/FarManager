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
#define GCMD( fnm,nm,v ) FP_GetRegKey( fnm,Opt.v,nm,sizeof(Opt.v) ); if (TrimLen(Opt.v) == 0) StrCpy( Opt.v,nm );
	Opt.AddToDisksMenu     = FP_GetRegKey("AddToDisksMenu",     1);
	Opt.AddToPluginsMenu   = FP_GetRegKey("AddToPluginsMenu",   1);
	Opt.DisksMenuDigit     = FP_GetRegKey("DisksMenuDigit",     2);
	Opt.ReadDescriptions   = FP_GetRegKey("ReadDescriptions",   0);
	Opt.UploadLowCase      = FP_GetRegKey("UploadLowCase",      0);
	Opt.ShowUploadDialog   = FP_GetRegKey("ShowUploadDialog",   1);
	Opt.ResumeDefault      = FP_GetRegKey("ResumeDefault",      0);
	Opt.UpdateDescriptions = FP_GetRegKey("UpdateDescriptions", 0);
	Opt.PassiveMode        = FP_GetRegKey("PassiveMode",        0);
	FP_GetRegKey("CharTable",        Opt.Table,NULL,sizeof(Opt.Table));
	FP_GetRegKey("DescriptionNames", Opt.DescriptionNames,"00_index.txt,0index,0index.txt",sizeof(Opt.DescriptionNames));
	FP_GetRegKey("Firewall",Opt.Firewall,NULL,sizeof(Opt.Firewall));
	FP_GetRegKey("DefaultPassword", (BYTE *)str,(BYTE *)NULL,sizeof(str));
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
	FP_GetRegKey("InvalidSymbols",     Opt.InvalidSymbols,   "<>|?*\"", sizeof(Opt.InvalidSymbols));
	FP_GetRegKey("CorrectedSymbols",   Opt.CorrectedSymbols, "()!__\'", sizeof(Opt.CorrectedSymbols));

	for(n = 0; Opt.InvalidSymbols[n] && Opt.CorrectedSymbols[n]; n++);

	Opt.InvalidSymbols[n]   = 0;
	Opt.CorrectedSymbols[n] = 0;
	Opt.PluginColumnMode   = (int)FP_GetRegKey("PluginColumnMode", MAX_DWORD);

	if(Opt.PluginColumnMode < 0 || Opt.PluginColumnMode >= 10)
		Opt.PluginColumnMode = -1;

	FP_GetRegKey("CmdLogFile", Opt.CmdLogFile,"",sizeof(Opt.CmdLogFile));
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
		FP_GetRegKey(Months[n], str, Months[n], sizeof(str));

		while((m=strpbrk(str,"\n\r\b")) != NULL) *m = 0;

		if(!str[0])
			strcpy(str,Months[n]);

		Log(("month %d [%s]=[%s]",n,Opt.Months[n],str));

		if(Opt.Months[n])
			_Del(Opt.Months[n]);

		Opt.Months[n] = StrDup(str);
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
		if(GetLocaleInfo(GetThreadLocale(),LOCALE_STHOUSAND,str,sizeof(str)))
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
	FP_SetRegKey("DefaultPassword", CryptedPassword,sizeof(CryptedPassword)-1);
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
	static FP_DialogItem InitItems[]=
	{
		/*00*/    FDI_CONTROL(DI_DOUBLEBOX, 3, 1,72,17, 0, FMSG(METitle))

		/*01*/      FDI_CHECK(5, 2,    FMSG(MDupFF))
		/*02*/      FDI_CHECK(5, 3,    FMSG(MUndupFF))
		/*03*/      FDI_CHECK(5, 4,    FMSG(MEShowProgress))
		/*04*/      FDI_CHECK(5, 5,    FMSG(MEBackup))
		/*05*/      FDI_CHECK(5, 6,    FMSG(MESendCmd))
		/*06*/      FDI_CHECK(5, 7,    FMSG(MEDontError))
		/*07*/      FDI_CHECK(5, 8,    FMSG(MEAskLoginAtFail))
		/*08*/      FDI_CHECK(5, 9,    FMSG(MEAutoAn))
		/*09*/      FDI_CHECK(5,10,    FMSG(MECloseDots))
		/*10*/      FDI_CHECK(5,11,    FMSG(MQuoteClipboardNames))
		/*11*/      FDI_CHECK(5,12,    FMSG(MSetHiddenOnAbort))

		/*12*/      FDI_HLINE(3,15)

		/*13*/ FDI_GDEFBUTTON(0,16,    FMSG(MOk))
		/*14*/    FDI_GBUTTON(0,16,    FMSG(MCancel))
		{FFDI_NONE,0,0,0,0,0,NULL}
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
	FarDialogItem DialogItems[(sizeof(InitItems)/sizeof(InitItems[0])-1)];
	FP_InitDialogItems(InitItems,DialogItems);
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
		rc = FDialog(76,19,"FTPExtGlobal",DialogItems,(sizeof(InitItems)/sizeof(InitItems[0])-1));

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
static FLngColorDialog ColorLangs =
{
	/*MTitle*/    FMSG(MColorTitle),
	/*MFore*/     FMSG(MColorFore),
	/*MBk*/       FMSG(MColorBk),
	/*MSet*/      FMSG(MColorColorSet),
	/*MCancel*/   FMSG(MCancel),
	/*MText*/     FMSG(MColorText)
};

int WINAPI Config(void)
{
	static FP_DialogItem InitItems[]=
	{
		/*00*/   FDI_CONTROL(DI_DOUBLEBOX, 3, 1,72,21, 0, FMSG(MConfigTitle))

		/*01*/     FDI_CHECK(5, 2,   FMSG(MConfigAddToDisksMenu))      //Add to Disks menu
		/*02*/   FDI_FIXEDIT(35, 2,37)
		/*03*/     FDI_CHECK(5, 3,   FMSG(MConfigAddToPluginsMenu))    //Add to Plugins menu
		/*04*/     FDI_LABEL(9, 4,   FMSG(MHostsMode))                 //Hosts panel mode
		/*05*/   FDI_FIXEDIT(5, 4, 7)
		/*06*/     FDI_CHECK(5, 5,   FMSG(MConfigReadDiz))             //Read descriptions
		/*07*/     FDI_CHECK(5, 6,   FMSG(MConfigUpdateDiz))           //Update descriptions
		/*08*/     FDI_CHECK(5, 7,   FMSG(MConfigUploadLowCase))       //Upload upper in lowercase
		/*09*/     FDI_CHECK(5, 8,   FMSG(MConfigUploadDialog))        //Show upload options dialog
		/*10*/     FDI_CHECK(5, 9,   FMSG(MConfigDefaultResume))       //Default button is 'Resume'
		/*11*/     FDI_CHECK(5,10,   FMSG(MAskAbort))                  //Confirm abort
		/*12*/     FDI_CHECK(5,11,   FMSG(MShowIdle))                  //Show idle
		/*13*/    FDI_BUTTON(21,11,   FMSG(MColor))                    //Color
		/*14*/FDI_STARTRADIO(32,11,   FMSG(MScreen))                   //( ) 1.Screen
		/*15*/     FDI_RADIO(44,11,   FMSG(MCaption))                  //( ) 2.Caption
		/*16*/     FDI_RADIO(58,11,   FMSG(MBoth))                     //( ) 3.Both

		/*17*/     FDI_CHECK(40, 2,   FMSG(MKeepAlive))                //Keepalive packet
		/*18*/      FDI_EDIT(67, 2,70)
		/*19*/     FDI_LABEL(71, 2,   FMSG(MSec))                      //s
		/*20*/     FDI_CHECK(40, 3,   FMSG(MAutoRetry))                //AutoRetry
		/*21*/      FDI_EDIT(67, 3,70)
		/*22*/     FDI_CHECK(40, 4,   FMSG(MLongOp))                   //Long operation beep
		/*23*/      FDI_EDIT(67, 4,70)
		/*24*/     FDI_LABEL(71, 4,   FMSG(MSec))                      //s
		/*25*/     FDI_LABEL(40, 5,   FMSG(MWaitTimeout))              //Server reply timeout (s)
		/*26*/      FDI_EDIT(67, 5,70)
		/*27*/     FDI_CHECK(40, 6,   FMSG(MDigitDelimit))             //Digits grouping symbol
		/*28*/      FDI_EDIT(69, 6,70)
		/*29*/     FDI_CHECK(40, 7,   FMSG(MExtWindow))                //Show FTP command log
		/*30*/     FDI_LABEL(40, 8,   FMSG(MExtSize))                  //Log window size
		/*31*/      FDI_EDIT(60, 8,63)
		/*32*/     FDI_LABEL(64, 8,   " x ")
		/*33*/      FDI_EDIT(67, 8,70)
		/*34*/     FDI_LABEL(40, 9,   FMSG(MHostIOSize))               //I/O buffer size
		/*35*/      FDI_EDIT(60, 9,70)
		/*36*/     FDI_LABEL(40,10,   FMSG(MSilentText))               //Alert text
		/*37*/    FDI_BUTTON(60,10,   FMSG(MColor))

		/*38*/     FDI_HLINE(5,12)

		/*39*/     FDI_LABEL(5,13,   FMSG(MConfigDizNames))            //Dis names
		/*40*/      FDI_EDIT(5,14,70)
		/*41*/     FDI_LABEL(5,15,   FMSG(MConfigDefPassword))         //Def pass
		/*42*/   FDI_PSWEDIT(5,16,34)
		/*43*/     FDI_LABEL(5,17,   FMSG(MConfigFirewall))            //Firewall
		/*44*/      FDI_EDIT(5,18,34)
		/*45*/     FDI_LABEL(40,15,   FMSG(MLogFilename))              //Log filename
		/*46*/      FDI_EDIT(66,15,70)
		/*47*/     FDI_LABEL(71,15,   FMSG(MKBytes))
		/*48*/      FDI_EDIT(40,16,70)
		/*49*/     FDI_CHECK(40,17,   FMSG(MLogDir))                   //Log DIR contents
		/*50*/     FDI_CHECK(40,18,   FMSG(MConfigPassiveMode))

		/*51*/     FDI_HLINE(5,19)

		/*52*/FDI_GDEFBUTTON(0,20,   FMSG(MOk))
		/*53*/   FDI_GBUTTON(0,20,   FMSG(MCancel))
		/*54*/   FDI_GBUTTON(0,20,   FMSG(MExtOpt))
		{FFDI_NONE,0,0,0,0,0,NULL}
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
	FarDialogItem DialogItems[(sizeof(InitItems)/sizeof(InitItems[0])-1)];
	int           IdleColor    = Opt.IdleColor,
	              ProcessColor = Opt.ProcessColor;
	int           rc;
	FP_InitDialogItems(InitItems,DialogItems);
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
	StrCpy(DialogItems[CFG_DESC].Data,Opt.DescriptionNames);
	StrCpy(DialogItems[CFG_PASS].Data,Opt.DefaultPassword);
	StrCpy(DialogItems[CFG_FIRE].Data,Opt.Firewall);
	sprintf(DialogItems[CFG_LOGLIMIT].Data,"%d",Opt.CmdLogLimit);
	sprintf(DialogItems[CFG_LOGFILE].Data,"%s",Opt.CmdLogFile);
	DialogItems[CFG_LOGDIR].Selected      = Opt.LogOutput;
	DialogItems[CFG_PASV].Selected        = Opt.PassiveMode;

	do
	{
		rc = FDialog(76,23,"Config",DialogItems,(sizeof(InitItems)/sizeof(InitItems[0])-1));

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
	if(DialogItems[CFG_IDLE_SCREEN].Selected)  Opt.IdleMode = IDLE_CONSOLE; else if(DialogItems[CFG_IDLE_CAPTION].Selected) Opt.IdleMode = IDLE_CAPTION; else

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
	StrCpy(Opt.DescriptionNames,DialogItems[CFG_DESC].Data);
	StrCpy(Opt.DefaultPassword,DialogItems[CFG_PASS].Data);
	StrCpy(Opt.Firewall,DialogItems[CFG_FIRE].Data);
	Opt.CmdLogLimit        = atoi(DialogItems[CFG_LOGLIMIT].Data);
	StrCpy(Opt.CmdLogFile, DialogItems[CFG_LOGFILE].Data);
	Opt.LogOutput          = DialogItems[CFG_LOGDIR].Selected;
	Opt.PassiveMode        = DialogItems[CFG_PASV].Selected;
//Write to REG
	WriteCfg();

	if(FTPPanels[0]) FTPPanels[0]->Invalidate();

	if(FTPPanels[1]) FTPPanels[1]->Invalidate();

	return TRUE;
}
