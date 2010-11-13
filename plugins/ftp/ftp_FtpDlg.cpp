#include <all_far.h>
#pragma hdrstop

#include "ftp_Int.h"

//---------------------------------------------------------------------------------
BOOL WINAPI AskSaveList(SaveListInfo* sli)
{
	static FP_DialogItem InitItems[]=
	{
		/*00*/    FDI_CONTROL(DI_DOUBLEBOX, 3, 1,56,14, 0, FMSG(MFLTitle))

		/*01*/      FDI_LABEL(4, 2,    FMSG(MFLSaveTo))
		/*02*/   FDI_HISTEDIT(4, 3,54, "SaveListHistory")
		/*03*/      FDI_HLINE(1, 4)

		/*04*/ FDI_STARTRADIO(4, 5,    FMSG(MFLUrls))
		/*05*/      FDI_RADIO(4, 6,    FMSG(MFLTree))
		/*06*/      FDI_RADIO(32, 5,    FMSG(MFLGroups))
		/*07*/      FDI_HLINE(1, 7)

		/*08*/      FDI_CHECK(4, 8,    FMSG(MFLPrefix))
		/*09*/      FDI_CHECK(4, 9,    FMSG(MFLPass))
		/*10*/      FDI_CHECK(4,10,    FMSG(MFLQuote))
		/*11*/      FDI_CHECK(4,11,    FMSG(MFLSizes))

		/*12*/      FDI_LABEL(28,11,    FMSG(MFLSizeBound))
		/*13*/       FDI_EDIT(44,11,47)
		/*14*/      FDI_HLINE(1,12)

		/*15*/      FDI_CHECK(32, 8,    FMSG(MFAppendList))

		/*16*/ FDI_GDEFBUTTON(0,13,    FMSG(MFtpSave))
		/*17*/    FDI_GBUTTON(0,13,    FMSG(MDownloadCancel))
		{FFDI_NONE,0,0,0,0,0,NULL}
	};
	FarDialogItem  DialogItems[(sizeof(InitItems)/sizeof(InitItems[0])-1)];
	FP_InitDialogItems(InitItems,DialogItems);
	StrCpy(DialogItems[ 2].Data, sli->path, sizeof(DialogItems[0].Data));
	DialogItems[ 4].Selected = sli->ListType == sltUrlList;
	DialogItems[ 5].Selected = sli->ListType == sltTree;
	DialogItems[ 6].Selected = sli->ListType == sltGroup;
	DialogItems[ 8].Selected = sli->AddPrefix;
	DialogItems[ 9].Selected = sli->AddPasswordAndUser;
	DialogItems[10].Selected = sli->Quote;
	DialogItems[11].Selected = sli->Size;
	itoa(sli->RightBound, DialogItems[13].Data, 10);
	DialogItems[15].Selected = sli->Append;

	if(FDialog(60,16,"FTPSaveList",DialogItems,(sizeof(InitItems)/sizeof(InitItems[0])-1)) != 16)
		return FALSE;

	StrCpy(sli->path, DialogItems[ 2].Data, MAX_PATH);
	if(DialogItems[ 4].Selected) sli->ListType = sltUrlList; else if(DialogItems[ 5].Selected) sli->ListType = sltTree;    else if(DialogItems[ 6].Selected) sli->ListType = sltGroup;

	sli->AddPrefix          = DialogItems[ 8].Selected;
	sli->AddPasswordAndUser = DialogItems[ 9].Selected;
	sli->Quote              = DialogItems[10].Selected;
	sli->Size               = DialogItems[11].Selected;
	sli->RightBound         = Max(30,atoi(DialogItems[13].Data));
	sli->Append             = DialogItems[15].Selected;
	return TRUE;
}

//---------------------------------------------------------------------------------
BOOL FTP::CopyAskDialog(BOOL Move, BOOL Download, FTPCopyInfo* ci)
{
	static FP_DialogItem InitItems[]=
	{
		/*00*/    FDI_CONTROL(DI_DOUBLEBOX, 3, 1,72,17, 0, NULL)

		/*01*/      FDI_LABEL(5, 2,    NULL)
		/*02*/   FDI_HISTEDIT(5, 3,70, NULL)

		/*03*/      FDI_LABEL(5, 4,    FMSG(MInclude))
		/*04*/   FDI_HISTEDIT(5, 5,70, FTP_INCHISTORY)

		/*05*/      FDI_LABEL(5, 6,    FMSG(MExclude))
		/*06*/   FDI_HISTEDIT(5, 7,70, FTP_EXCHISTORY)

		/*07*/      FDI_HLINE(3, 8)

		/*08*/      FDI_CHECK(5, 9,    FMSG(MDownloadAscii))
		/*09*/      FDI_CHECK(5,10,    FMSG(MDoNotScan))
		/*10*/      FDI_CHECK(5,11,    FMSG(MSelectFromList))
		/*11*/      FDI_CHECK(5,12,    FMSG(MAddQueue))
		/*12*/      FDI_CHECK(5,13,    FMSG(MConfigUploadLowCase))
		/*13*/      FDI_CHECK(5,14,    FMSG(MDownloadOnServer))

		/*14*/      FDI_LABEL(40, 9,    FMSG(MDefOverwrite))
		/*15*/ FDI_STARTRADIO(41,10,    FMSG(MOverAsk))
		/*16*/      FDI_RADIO(41,11,    FMSG(MOverOver))
		/*17*/      FDI_RADIO(41,12,    FMSG(MOverSkip))
		/*18*/      FDI_RADIO(41,13,    FMSG(MOverResume))
		/*19*/      FDI_RADIO(41,14,    FMSG(MOverNewer))

		/*20*/      FDI_HLINE(3,15)
		/*21*/ FDI_GDEFBUTTON(0,16,    NULL)
		/*22*/    FDI_GBUTTON(0,16,    FMSG(MDownloadCancel))

		{FFDI_NONE,0,0,0,0,0,NULL}
	};
	enum
	{
		MID_TITLE    = 0,
		MID_DEST     = 1,
		MID_PATH     = 2,
		MID_INC      = 4,
		MID_EXC      = 6,
		MID_ASCII    = 8,
		MID_NOTSCAN  = 9,
		MID_SHLIST   = 10,
		MID_ADDQ     = 11,
		MID_UPPER    = 12,
		MID_FTPR     = 13,
		MID_ASK      = 15,
		MID_OVER     = 16,
		MID_SKIP     = 17,
		MID_RESUME   = 18,
		MID_NEWER    = 19,
		MID_DEFB     = 21
	};
	FarDialogItem  di[(sizeof(InitItems)/sizeof(InitItems[0])-1)];
	char          *m;

//Set values
	//Title
	if(ci->Download)
	{
		InitItems[ MID_TITLE ].Text = Move ? FMSG(MRenameTitle) : FMSG(MDownloadTitle);
		InitItems[ MID_DEST  ].Text = FMSG(MDownloadTo);
		InitItems[ MID_PATH  ].Text = FTP_GETHISTORY;
		InitItems[ MID_DEFB  ].Text = Move ? FMSG(MRenameTitle) : FMSG(MDownload);
	}
	else
	{
		InitItems[ MID_TITLE ].Text = FMSG(MUploadTitle);
		InitItems[ MID_DEST  ].Text = FMSG(MUploadTo);
		InitItems[ MID_PATH  ].Text = FTP_PUTHISTORY;
		InitItems[ MID_DEFB  ].Text = FMSG(MUpload);
	}

//Create items
	FP_InitDialogItems(InitItems,di);

//Set flags
	//Gray resume
	if(!hConnect->ResumeSupport && Download)
		SET_FLAG(di[MID_RESUME].Flags,DIF_DISABLE);

	if(!Download || !Move)
		SET_FLAG(di[MID_FTPR].Flags,DIF_DISABLE);

	//Gray do not scan files
	SET_FLAG(di[MID_NOTSCAN].Flags,DIF_DISABLE);
	//Path
	di[MID_PATH].Flags        |= DIF_VAREDIT;
	di[MID_PATH].Ptr.PtrFlags  = 0;
	di[MID_PATH].Ptr.PtrLength = DIALOG_EDIT_SIZE;
	di[MID_PATH].Ptr.PtrData   = DialogEditBuffer;
	StrCpy(DialogEditBuffer, ci->DestPath.c_str(), DIALOG_EDIT_SIZE);
	//Include
	TStrCpy(di[MID_INC].Data,  "*");
	//Exclude
	StrCpy(di[MID_EXC].Data,  "");
	//Flags
	di[MID_ASCII].Selected = ci->asciiMode;
	di[MID_UPPER].Selected = ci->UploadLowCase;

	if(Move)
		SET_FLAG(di[MID_ADDQ].Flags,DIF_DISABLE);

	//Gray UPPER on download
	if(Download)
		SET_FLAG(di[MID_UPPER].Flags,DIF_DISABLE);

	//Mode
	switch(ci->MsgCode)
	{
		case   ocOverAll: di[MID_OVER  ].Selected = TRUE; break;
		case   ocSkipAll: di[MID_SKIP  ].Selected = TRUE; break;
		case ocResumeAll: di[MID_RESUME].Selected = TRUE; break;
		case  ocNewerAll: di[MID_NEWER ].Selected = TRUE; break;
		default: di[MID_ASK   ].Selected = TRUE; break;
	}

//Dialog
	if(FDialog(76,19,"FTPCmd",di,(sizeof(InitItems)/sizeof(InitItems[0])-1)) != MID_DEFB)
	{
		if(LongBeep) FP_PeriodReset(LongBeep);

		return FALSE;
	}

	if(LongBeep) FP_PeriodReset(LongBeep);

//Get paras
	//Path
	ci->DestPath = DialogEditBuffer;

	if(!ci->DestPath[0])
		return FALSE;

	if(ci->DestPath.Cmp(FTP_CMDPREFIX,FTP_CMDPREFIX_SIZE) &&
	        ci->DestPath[FTP_CMDPREFIX_SIZE] == ':')
		ci->DestPath.Del(0, FTP_CMDPREFIX_SIZE+1);

	//Include
	m = di[MID_INC].Data;

	while(*m && isspace(*m)) m++;

	TStrCpy(IncludeMask, m);
	m = IncludeMask + strLen(IncludeMask)-1;

	while(m > IncludeMask && isspace(*m)) { *m = 0; m--; }

	//Exclude
	m = di[MID_EXC].Data;

	while(*m && isspace(*m)) m++;

	TStrCpy(ExcludeMask, m);
	m = ExcludeMask + strLen(ExcludeMask)-1;

	while(m > IncludeMask && isspace(*m)) { *m = 0; m--; }

	//Flags
	ci->asciiMode       = di[ MID_ASCII  ].Selected;
	ci->ShowProcessList = di[ MID_SHLIST ].Selected;
	ci->AddToQueque     = di[ MID_ADDQ   ].Selected;
	ci->UploadLowCase   = di[ MID_UPPER  ].Selected;
	ci->FTPRename       = di[ MID_FTPR   ].Selected;

	//Mode
	if(di[MID_OVER  ].Selected) ci->MsgCode = ocOverAll; else if(di[MID_SKIP  ].Selected) ci->MsgCode = ocSkipAll; else if(di[MID_RESUME].Selected) ci->MsgCode = ocResumeAll; else if(di[MID_NEWER ].Selected) ci->MsgCode = ocNewerAll;

	return TRUE;
}
//---------------------------------------------------------------------------------
void FTP::SaveURL()
{
	FTPHost h;
	String  str, path;
	h = Host;
	FtpGetCurrentDirectory(hConnect, path);
	Host.MkUrl(str, path.c_str(), NULL, TRUE);

	if(h.SetHostName(str.c_str(),NULL,NULL) &&
	        GetHost(MSaveFTPTitle,&h,FALSE))
		h.Write(HostsPath);
}

//---------------------------------------------------------------------------------
void QueryExHostOptions(FTP* ftp,FTPHost* p)
{
	static FP_DialogItem InitItems[]=
	{
		/*00*/    FDI_CONTROL(DI_DOUBLEBOX, 3, 1,72,17, 0, FMSG(MEHTitle))

		/*01*/      FDI_CHECK(5, 2,    FMSG(MDupFF))
		/*02*/      FDI_CHECK(5, 3,    FMSG(MUndupFF))
		/*03*/      FDI_CHECK(5, 4,    FMSG(MEHDecodeCmd))
		/*04*/      FDI_CHECK(5, 5,    FMSG(MSendAllo))
		/*05*/      FDI_CHECK(5, 6,    FMSG(MUseStartSpaces))

		/*06*/      FDI_HLINE(3,15)

		/*07*/ FDI_GDEFBUTTON(0,16,    FMSG(MOk))
		/*08*/    FDI_GBUTTON(0,16,    FMSG(MCancel))
		{FFDI_NONE,0,0,0,0,0,NULL}
	};
	enum
	{
		dDupFF    = 1,
		dUndupFF  = 2,
		dCmdLine  = 3,
		dSendAllo = 4,
		dUseStartSpaces = 5,

		dOk       = 7
	};
	int           rc;
	FarDialogItem di[(sizeof(InitItems)/sizeof(InitItems[0])-1)];
	FP_InitDialogItems(InitItems,di);
	di[dDupFF].Selected               = p->FFDup;
	di[dUndupFF].Selected             = p->UndupFF;
	di[dCmdLine].Selected             = p->DecodeCmdLine;
	di[dSendAllo].Selected            = p->SendAllo;
	di[dUseStartSpaces].Selected   = p->UseStartSpaces;

	do
	{
		rc = FDialog(76,19,"FTPExtHost",di,(sizeof(InitItems)/sizeof(InitItems[0])-1));

		if(rc == -1)
			return;

		if(ftp->LongBeep) FP_PeriodReset(ftp->LongBeep);

		di[rc].Focus    = FALSE;

		if(rc != dOk)
			return;

		break;
	}
	while(1);

	p->FFDup              = di[dDupFF].Selected;
	p->UndupFF            = di[dUndupFF].Selected;
	p->DecodeCmdLine      = di[dCmdLine].Selected;
	p->SendAllo           = di[dSendAllo].Selected;
	p->UseStartSpaces  = di[dUseStartSpaces].Selected;
}

BOOL FTP::GetHost(int title,FTPHost* p,BOOL ToDescription)
{
	static FP_DialogItem InitItems[]=
	{
		/*00*/    FDI_CONTROL(DI_DOUBLEBOX, 3, 1,72,17, 0, NULL)
		/*01*/      FDI_LABEL(5, 2,    FMSG(MFtpName))
		/*02*/   FDI_HISTEDIT(7, 3,70, FTP_HOSTHISTORY)

		/*03*/      FDI_HLINE(4, 4)

		/*04*/      FDI_LABEL(5, 5,    FMSG(MUser))
		/*05*/   FDI_HISTEDIT(18, 5,70, FTP_USERHISTORY)

		/*06*/      FDI_LABEL(5, 6,    FMSG(MPassword))
		/*07*/    FDI_PSWEDIT(18, 6,70)

		/*08*/      FDI_LABEL(5, 7,    FMSG(MHostDescr))
		/*09*/       FDI_EDIT(18, 7,70)

		/*10*/      FDI_HLINE(3, 8)

		/*11*/      FDI_CHECK(5, 9,    FMSG(MAskLogin))
		/*12*/      FDI_CHECK(5,10,    FMSG(MAsciiMode))
		/*13*/      FDI_CHECK(5,11,    FMSG(MPassiveMode))
		/*14*/      FDI_CHECK(5,12,    FMSG(MUseFirewall))

		/*15*/      FDI_CHECK(5,13,    FMSG(MDecodeCommands))
		/*16*/      FDI_CHECK(40, 9,    FMSG(MExtWindow))
		/*17*/      FDI_CHECK(40,10,    FMSG(MExtList))
		/*18*/       FDI_EDIT(44,11, 59)
		/*19*/      FDI_LABEL(40,12,    FMSG(MHostIOSize))
		/*20*/       FDI_EDIT(60,12, 70)

		/*21*/      FDI_HLINE(3,14)

		/*22*/    FDI_GBUTTON(0,15,    FMSG(MFtpSelectTable))
		/*23*/    FDI_GBUTTON(0,15,    FMSG(MServerType))
		/*24*/    FDI_GBUTTON(0,15,    FMSG(MExtOpt))
		/*25*/ FDI_GDEFBUTTON(0,16,    FMSG(MFtpSave))
		/*26*/    FDI_GBUTTON(0,16,    FMSG(MFtpConnect))
		/*27*/    FDI_GBUTTON(0,16,    FMSG(MCancel))
		{FFDI_NONE,0,0,0,0,0,NULL}
	};
#define dHOST    2
#define dUSER    5
#define dPSWD    7
#define dDESC    9
#define dLOGIN   11
#define dASCII   12
#define dPASV    13
#define dFIRW    14
#define dCODECMD 15
#define dEXTCMD  16
#define dEXTLIST 17
#define dLISTCMD 18
#define dIOSIZE  20
#define dTABLE   22
#define dSERVER  23
#define dEXTBUT  24
#define dSAVE    25
#define dCONNECT 26
	int           rc;
	char          TableName[100];
	char         *Name;
	FarDialogItem di[(sizeof(InitItems)/sizeof(InitItems[0])-1)];
	FTPHost       tmp;
	tmp = *p;
	FP_InitDialogItems(InitItems,di);
	StrCpy(di[0].Data, FP_GetMsg(title));

	if(ToDescription)
		di[dDESC].Focus = TRUE;
	else
		di[dHOST].Focus = TRUE;

	StrCpy(di[dHOST].Data,p->HostName);
	StrCpy(di[dUSER].Data,p->User);
	StrCpy(di[dPSWD].Data,p->Password);
	StrCpy(di[dDESC].Data,p->HostDescr);
	Size2Str(di[dIOSIZE].Data,p->IOBuffSize);
	di[dLOGIN].Selected   = p->AskLogin;
	di[dASCII].Selected   = p->AsciiMode;
	di[dPASV].Selected    = p->PassiveMode;
	di[dFIRW].Selected    = p->UseFirewall;
	di[dEXTCMD].Selected  = p->ExtCmdView;
	di[dEXTLIST].Selected = p->ExtList;
	di[dCODECMD].Selected = p->CodeCmd;
	StrCpy(di[dLISTCMD].Data,p->ListCMD);
	StrCpy(TableName,p->HostTable);

	do
	{
		rc = FDialog(76,19,"FTPConnect",di,(sizeof(InitItems)/sizeof(InitItems[0])-1));

		if(rc == -1)
			return FALSE;

		if(LongBeep) FP_PeriodReset(LongBeep);

		di[rc].Focus    = FALSE;
		di[dHOST].Focus = TRUE;

		if(rc == dTABLE)
		{
			SelectFileTable(TableName);
			continue;
		}
		else if(rc == dSERVER)
		{
			tmp.ServerType = SelectServerType(p->ServerType);
			continue;
		}
		else if(rc == dEXTBUT)
		{
			QueryExHostOptions(this,&tmp);
			continue;
		}
		else if(rc != dSAVE && rc != dCONNECT)
			return FALSE;

		Name = di[dHOST].Data;

		while(isspace(*Name)) Name++;

		if(!Name[0]) continue;

		if(*Name == '\"' && FP_FSF && FP_FSF->Unquote)
		{
			FP_FSF->Unquote(Name);

			if(!Name[0]) continue;

			while(isspace(*Name)) Name++;

			if(!Name[0]) continue;
		}

		break;
	}
	while(1);

	*p = tmp;
	p->SetHostName(Name,
	               di[dUSER].Data,
	               di[dPSWD].Data);
	StrCpy(p->HostDescr,di[dDESC].Data,sizeof(p->HostDescr));
	StrCpy(p->HostTable,TableName);
	p->AskLogin    = di[dLOGIN].Selected;
	p->AsciiMode   = di[dASCII].Selected;
	p->PassiveMode = di[dPASV].Selected;
	p->UseFirewall = di[dFIRW].Selected;
	p->ExtCmdView  = di[dEXTCMD].Selected;
	p->ExtList     = di[dEXTLIST].Selected;
	p->CodeCmd     = di[dCODECMD].Selected;
	p->IOBuffSize  = Max((DWORD)FTR_MINBUFFSIZE,Str2Size(di[dIOSIZE].Data));
	StrCpy(p->ListCMD,di[dLISTCMD].Data,sizeof(p->ListCMD));

	if(rc == dCONNECT)
	{
		Host.Assign(p);
		FullConnect();
		return FALSE;
	}

	return TRUE;
}

//---------------------------------------------------------------------------------
BOOL FTP::EditDirectory(String& Name,char *Desc,BOOL newDir)
{
	static FP_DialogItem InitItems[]=
	{
		/*00*/      FDI_CONTROL(DI_DOUBLEBOX, 3, 1,72,6, 0, NULL)
		/*01*/        FDI_LABEL(5, 2,    FMSG(MMkdirName))
		/*02*/     FDI_HISTEDIT(5, 3,70, FTP_FOLDHISTORY)
		/*03*/        FDI_LABEL(5, 4,    FMSG(MHostDescr))
		/*04*/         FDI_EDIT(5, 5,70)
		{FFDI_NONE,0,0,0,0,0,NULL}
	};
	InitItems[0].Text = newDir ? FMSG(MMkdirTitle) : FMSG(MChDir);
	FarDialogItem di[(sizeof(InitItems)/sizeof(InitItems[0])-1)];
	FP_InitDialogItems(InitItems,di);
	//Name
	di[2].Flags        |= DIF_VAREDIT;
	di[2].Ptr.PtrFlags  = 0;
	di[2].Ptr.PtrLength = DIALOG_EDIT_SIZE;
	di[2].Ptr.PtrData   = DialogEditBuffer;
	StrCpy(DialogEditBuffer, Name.c_str(), DIALOG_EDIT_SIZE);

	if(!newDir)
		SET_FLAG(di[2].Flags, DIF_DISABLE);

	//Description
	if(Desc)
		StrCpy(di[4].Data,Desc);
	else
	{
		SET_FLAG(di[3].Flags, DIF_DISABLE);
		SET_FLAG(di[4].Flags, DIF_DISABLE);
	}

	//Exec
	int AskCode = FDialog(76,8,"FTPCmd",di,(sizeof(InitItems)/sizeof(InitItems[0])-1));

	if(AskCode < 0 || !DialogEditBuffer[0])
		return FALSE;

	//Fields -> Data
	Name = DialogEditBuffer;

	if(Desc) StrCpy(Desc,di[4].Data);

	return TRUE;
}

//---------------------------------------------------------------------------------
int FTP::TableNameToValue(char *TableName)
{
	if(StrCmp(TableName,".win")==0)
		return(0);

	if(StrCmp(TableName,".dos")==0)
		return(1);

	if(StrCmp(TableName,".auto")==0)
		return(2);

	if(StrCmp(TableName,".utf8")==0)
		return(3);

	for(int I=0;; I++)
	{
		CharTableSet TableSet;

		if(FP_Info->CharTable(I,(char*)&TableSet,sizeof(TableSet))==-1)
			break;

		if(StrCmp(TableName,TableSet.TableName)==0)
			return(I+4);
	}

	return(0);
}

void FTP::SelectTable()
{
	struct FarMenuItem MenuItems[16];
	memset(MenuItems, 0, sizeof(MenuItems));
	MenuItems[hConnect->TableNum].Selected = TRUE;
	StrCpy(MenuItems[0].Text,"Windows");
	StrCpy(MenuItems[1].Text,"DOS");
	StrCpy(MenuItems[2].Text,FP_GetMsg(MTableAuto));
	StrCpy(MenuItems[3].Text,"UTF-8");
	int TableNum=4;

	while(TableNum < (int)(sizeof(MenuItems)/sizeof(MenuItems[0])))
	{
		CharTableSet TableSet;

		if(FP_Info->CharTable(TableNum-4,(char*)&TableSet,sizeof(TableSet))==-1)
			break;

		StrCpy(MenuItems[TableNum++].Text,TableSet.TableName);
	}

	int ExitCode=FP_Info->Menu(FP_Info->ModuleNumber,-1,-1,0,FMENU_AUTOHIGHLIGHT,
	                           FP_GetMsg(MTableTitle),NULL,NULL,NULL,NULL,MenuItems,TableNum);

	if(ExitCode<0)
		return;

	if(hConnect)
		((Connection *)hConnect)->SetTable(ExitCode);

	char TableName[100];

	switch(ExitCode)
	{
		case 0:
			StrCpy(TableName,".win");
			break;
		case 1:
			StrCpy(TableName,".dos");
			break;
		case 2:
			StrCpy(TableName,".auto");
			break;
		case 3:
			StrCpy(TableName,".utf8");
			break;
		default:
			StrCpy(TableName,MenuItems[ExitCode].Text);
			break;
	}

	FP_SetRegKey("CharTable",TableName);
}

void FTP::SelectFileTable(char *TableName)
{
	struct FarMenuItem MenuItems[50];
	memset(MenuItems, 0, sizeof(MenuItems));
	StrCpy(MenuItems[0].Text,FP_GetMsg(MTableDefault));
	StrCpy(MenuItems[1].Text,"Windows");
	StrCpy(MenuItems[2].Text,"DOS");
	StrCpy(MenuItems[3].Text,FP_GetMsg(MTableAuto));
	StrCpy(MenuItems[4].Text,"UTF-8");

	if(StrCmp(TableName,".win")==0)
		MenuItems[1].Selected=TRUE;

	if(StrCmp(TableName,".dos")==0)
		MenuItems[2].Selected=TRUE;

	if(StrCmp(TableName,".auto")==0)
		MenuItems[3].Selected=TRUE;

	if(StrCmp(TableName,".utf8")==0)
		MenuItems[4].Selected=TRUE;

	int TableNum=5;

	while(TableNum < (int)ARRAY_SIZE(MenuItems))
	{
		CharTableSet TableSet;

		if(FP_Info->CharTable(TableNum-5,(char*)&TableSet,sizeof(TableSet))==-1)
			break;

		if(StrCmp(TableName,TableSet.TableName)==0)
			MenuItems[TableNum].Selected=TRUE;

		StrCpy(MenuItems[TableNum++].Text,TableSet.TableName);
	}

	int ExitCode=FP_Info->Menu(FP_Info->ModuleNumber,-1,-1,0,FMENU_AUTOHIGHLIGHT,
	                           FP_GetMsg(MTableTitle),NULL,NULL,NULL,NULL,MenuItems,TableNum);

	if(ExitCode<0)
		return;

	switch(ExitCode)
	{
		case 0:
			*TableName=0;
			break;
		case 1:
			StrCpy(TableName,".win");
			break;
		case 2:
			StrCpy(TableName,".dos");
			break;
		case 3:
			StrCpy(TableName,".auto");
			break;
		case 4:
			StrCpy(TableName,".utf8");
			break;
		default:
			StrCpy(TableName,MenuItems[ExitCode].Text);
			break;
	}
}
//---------------------------------------------------------------------------------
void FTP::SetAttributes()
{
	static FP_DialogItem InitItems[]=
	{
		/*00*/    FDI_CONTROL(DI_DOUBLEBOX, 3, 1,35,6, 0, FMSG(MChmodTitle))
		/*01*/      FDI_LABEL(6, 2,    FMSG("R  W  X   R  W  X   R  W  X"))
		/*02*/      FDI_CHECK(5, 3,    NULL)
		/*03*/      FDI_CHECK(8, 3,    NULL)
		/*04*/      FDI_CHECK(11, 3,    NULL)
		/*05*/      FDI_CHECK(15, 3,    NULL)
		/*06*/      FDI_CHECK(18, 3,    NULL)
		/*07*/      FDI_CHECK(21, 3,    NULL)
		/*08*/      FDI_CHECK(25, 3,    NULL)
		/*09*/      FDI_CHECK(28, 3,    NULL)
		/*10*/      FDI_CHECK(31, 3,    NULL)
		/*11*/      FDI_HLINE(3, 4)
		/*12*/ FDI_GDEFBUTTON(0, 5,   FMSG(MOk))
		/*13*/    FDI_GBUTTON(0, 5,   FMSG(MCancel))
		{FFDI_NONE,0,0,0,0,0,NULL}
	};
	FarDialogItem di[(sizeof(InitItems)/sizeof(InitItems[0])-1)];
	PanelInfo     PInfo;
	DWORD         Mode=0;
	int           n,AskCode;
	char         *m;
	FP_InitDialogItems(InitItems,di);
	FP_Info->Control(this,FCTL_GETPANELINFO,&PInfo);

	if(PInfo.SelectedItemsNumber == 1)
	{
		m = PInfo.SelectedItems[0].CustomColumnData[ FTP_COL_MODE ];

		if(!m[0])
			return;

		for(n = 2; n <= 10; n++)
			di[ n ].Selected = m[ n-1 ] != '-';
	}

	AskCode = FDialog(39,8,"FTPCmd",di,(sizeof(InitItems)/sizeof(InitItems[0])-1));

	if(AskCode != 12)
		return;

	FP_Screen _scr;

	for(n = 2; n <= 10; n++)
		Mode = (Mode<<1) | di[n].Selected;

	AskCode = TRUE;

	for(n = 0; n < PInfo.SelectedItemsNumber; n++)
	{
		m = FTP_FILENAME(&PInfo.SelectedItems[n]);

		if(FtpChmod(hConnect,m,Mode))
			continue;

		if(!AskCode)
			continue;

		switch(FtpConnectMessage(hConnect,MCannotChmod,m,-MCopySkip,MCopySkipAll))
		{
			/*skip*/     case 0: break;
			/*skip all*/ case 1: AskCode = FALSE; break;
			default: SetLastError(ERROR_CANCELLED);
				return;
		}
	}
}
//---------------------------------------------------------------------------------
BOOL WINAPI GetLoginData(char *User, char *Password, BOOL forceAsk)
{
	static FP_DialogItem InitItems[]=
	{
		/*00*/    FDI_CONTROL(DI_DOUBLEBOX, 3, 1,72,8, 0, FMSG(MLoginInfo))
		/*01*/      FDI_LABEL(5, 2,    FMSG(MUserName))
		/*02*/   FDI_HISTEDIT(5, 3,70, FTP_USERHISTORY)
		/*01*/      FDI_LABEL(5, 4,    FMSG(MUserPassword))
		/*16*/   FDI_PSWEDIT(5, 5,70)
		/*03*/      FDI_HLINE(5, 6)
		/*06*/ FDI_GDEFBUTTON(0, 7,   FMSG(MOk))
		/*07*/    FDI_GBUTTON(0, 7,   FMSG(MCancel))
		{FFDI_NONE,0,0,0,0,0,NULL}
	};

	if(User[0] && !Password[0])
		forceAsk = TRUE;

	//Default user
	if(!User[0] && Opt.AutoAnonymous)
		StrCpy(User,"anonymous");

	//Default passw
	if(!Password[0] && stricmp(User,"anonymous") == 0)
		StrCpy(Password, Opt.DefaultPassword, FAR_MAX_NAME);

	if(!forceAsk)
		return TRUE;

	FarDialogItem di[(sizeof(InitItems)/sizeof(InitItems[0])-1)];
	FP_InitDialogItems(InitItems,di);

	if(*User==0 || *Password!=0)
		di[2].Focus = TRUE;
	else
		di[4].Focus = TRUE;

	StrCpy(di[2].Data, User);
	StrCpy(di[4].Data, Password);

	do
	{
		int AskCode = FP_Info->Dialog(FP_Info->ModuleNumber,-1,-1,
		                              76,10,"FTPConnect",
		                              di,(sizeof(InitItems)/sizeof(InitItems[0])-1));

		//Cancel
		if(AskCode == -1 || AskCode == 7)
		{
			SetLastError(ERROR_CANCELLED);
			return FALSE;
		}

		//Empty user name
		if(!di[2].Data[0])
			continue;

		StrCpy(User,     di[2].Data, FAR_MAX_NAME);
		StrCpy(Password, di[4].Data, FAR_MAX_NAME);
		return TRUE;
	}
	while(1);
}
