#include <all_far.h>
#pragma hdrstop

#include "Int.h"

//---------------------------------------------------------------------------------
BOOL WINAPI AskSaveList(SaveListInfo* sli)
{
	InitDialogItem InitItems[]=
	{
		{DI_DOUBLEBOX, 3, 1,56,14, 0,0,0,0, FMSG(MFLTitle)},

		{DI_TEXT,4, 2,0,0,0,  0,0,0,  FMSG(MFLSaveTo)},
		{DI_EDIT,4, 3,54, 3,0,(DWORD_PTR)"SaveListHistory",DIF_HISTORY, 0,NULL},
		{DI_TEXT,1, 4,1, 4,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,NULL },

		{DI_RADIOBUTTON,4, 5,0,0,0,0,DIF_GROUP,   0, FMSG(MFLUrls)},
		{DI_RADIOBUTTON,4, 6,0,0,0,   0,0,0, FMSG(MFLTree)},
		{DI_RADIOBUTTON,32, 5,0,0,0, 0,0,0,   FMSG(MFLGroups)},
		{DI_TEXT,1, 7,1, 7,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,NULL },

		{DI_CHECKBOX,4, 8,0,0,0,  0,0,0,  FMSG(MFLPrefix)},
		{DI_CHECKBOX,4, 9,0,0,0,  0,0,0,  FMSG(MFLPass)},
		{DI_CHECKBOX,4,10,0,0,0,  0,0,0,  FMSG(MFLQuote)},
		{DI_CHECKBOX,4,11,0,0,0,  0,0,0,  FMSG(MFLSizes)},

		{DI_TEXT,28,11,0,0,0,   0,0,0, FMSG(MFLSizeBound)},
		{DI_EDIT,44,11,47,11,0,0,0,0,NULL},
		{DI_TEXT,1,12,1,12,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,NULL },

		{DI_CHECKBOX,32, 8,0,0,0,   0,0,0, FMSG(MFAppendList)},

		{DI_BUTTON,0,13,0,0,0,0,DIF_CENTERGROUP, 1,FMSG(MFtpSave)},
	};
	FarDialogItem DialogItems[ARRAYSIZE(InitItems)];
	InitDialogItems(InitItems,DialogItems,ARRAYSIZE(DialogItems));
	StrCpy(DialogItems[ 2].Data, sli->path, ARRAYSIZE(DialogItems[0].Data));
	DialogItems[ 4].Selected = sli->ListType == sltUrlList;
	DialogItems[ 5].Selected = sli->ListType == sltTree;
	DialogItems[ 6].Selected = sli->ListType == sltGroup;
	DialogItems[ 8].Selected = sli->AddPrefix;
	DialogItems[ 9].Selected = sli->AddPasswordAndUser;
	DialogItems[10].Selected = sli->Quote;
	DialogItems[11].Selected = sli->Size;
	itoa(sli->RightBound, DialogItems[13].Data, 10);
	DialogItems[15].Selected = sli->Append;

	if(FDialog(60,16,"FTPSaveList",DialogItems,ARRAYSIZE(DialogItems)) != 16)
		return FALSE;

	StrCpy(sli->path, DialogItems[ 2].Data, MAX_PATH);

	if(DialogItems[ 4].Selected) sli->ListType = sltUrlList;
	else if(DialogItems[ 5].Selected) sli->ListType = sltTree;
	else if(DialogItems[ 6].Selected) sli->ListType = sltGroup;

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
	InitDialogItem InitItems[]=
	{
		{DI_DOUBLEBOX, 3, 1,72,17, 0,0,0,0, NULL},

		{DI_TEXT,5, 2,0,0,0,  0,0,0,  NULL},
		{DI_EDIT,5, 3,70, 3,0, 0,DIF_HISTORY,0,NULL},

		{DI_TEXT,5, 4,0,0,0, 0,0,0, FMSG(MInclude)},
		{DI_EDIT,5, 5,70, 5,0, 0,DIF_HISTORY,0,FTP_INCHISTORY},

		{DI_TEXT,5, 6,0,0,0, 0,0,0,   FMSG(MExclude)},
		{DI_EDIT,5, 7,70, 7,0,0,DIF_HISTORY,0, FTP_EXCHISTORY},

		{DI_TEXT,3, 8,3, 8,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,NULL },

		{DI_CHECKBOX,5, 9,0,0,0,  0,0,0,  FMSG(MDownloadAscii)},
		{DI_CHECKBOX,5,10,0,0,0,  0,0,0,  FMSG(MDoNotScan)},
		{DI_CHECKBOX,5,11,0,0,0,  0,0,0,  FMSG(MSelectFromList)},
		{DI_CHECKBOX,5,12,0,0,0,  0,0,0,  FMSG(MAddQueue)},
		{DI_CHECKBOX,5,13,0,0,0,  0,0,0,  FMSG(MConfigUploadLowCase)},
		{DI_CHECKBOX,5,14,0,0,0,  0,0,0,  FMSG(MDownloadOnServer)},

		{DI_TEXT,40, 9,0,0,0,  0,0,0,  FMSG(MDefOverwrite)},
		{DI_RADIOBUTTON,41,10,0,0,0,0,DIF_GROUP,  0,  FMSG(MOverAsk)},
		{DI_RADIOBUTTON,41,11,0,0,0, 0,0,0,   FMSG(MOverOver)},
		{DI_RADIOBUTTON,41,12,0,0,0, 0,0,0,   FMSG(MOverSkip)},
		{DI_RADIOBUTTON,41,13,0,0,0, 0,0,0,   FMSG(MOverResume)},
		{DI_RADIOBUTTON,41,14,0,0,0, 0,0,0,   FMSG(MOverNewer)},

		{DI_TEXT,3,15,3,15,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,NULL },
		{DI_BUTTON,0,16,0,0,0,0,DIF_CENTERGROUP, 1,   NULL},
		{DI_BUTTON,0,16,0,0,0,0,DIF_CENTERGROUP, 0,   FMSG(MDownloadCancel)},
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
	FarDialogItem di[ARRAYSIZE(InitItems)];
	char          *m;

//Set values
	//Title
	if(ci->Download)
	{
		InitItems[ MID_TITLE ].Data = Move ? FMSG(MRenameTitle) : FMSG(MDownloadTitle);
		InitItems[ MID_DEST  ].Data = FMSG(MDownloadTo);
		InitItems[ MID_PATH  ].Data = FTP_GETHISTORY;
		InitItems[ MID_DEFB  ].Data = Move ? FMSG(MRenameTitle) : FMSG(MDownload);
	}
	else
	{
		InitItems[ MID_TITLE ].Data = FMSG(MUploadTitle);
		InitItems[ MID_DEST  ].Data = FMSG(MUploadTo);
		InitItems[ MID_PATH  ].Data = FTP_PUTHISTORY;
		InitItems[ MID_DEFB  ].Data = FMSG(MUpload);
	}

//Create items
	InitDialogItems(InitItems,di,ARRAYSIZE(di));

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
	StrCpy(di[MID_INC].Data, "*", ARRAYSIZE(di[MID_INC].Data));
	//Exclude
	*di[MID_EXC].Data = 0;
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
		case   ocOverAll:
			di[MID_OVER  ].Selected = TRUE;
			break;
		case   ocSkipAll:
			di[MID_SKIP  ].Selected = TRUE;
			break;
		case ocResumeAll:
			di[MID_RESUME].Selected = TRUE;
			break;
		case  ocNewerAll:
			di[MID_NEWER ].Selected = TRUE;
			break;
		default:
			di[MID_ASK   ].Selected = TRUE;
			break;
	}

//Dialog
	if(FDialog(76,19,"FTPCmd",di,ARRAYSIZE(di)) != MID_DEFB)
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

	StrCpy(IncludeMask, m, ARRAYSIZE(IncludeMask));
	m = IncludeMask + strlen(IncludeMask)-1;

	while(m > IncludeMask && isspace(*m))
	{
		*m = 0;
		m--;
	}

	//Exclude
	m = di[MID_EXC].Data;

	while(*m && isspace(*m)) m++;

	StrCpy(ExcludeMask, m, ARRAYSIZE(ExcludeMask));
	m = ExcludeMask + strlen(ExcludeMask)-1;

	while(m > IncludeMask && isspace(*m))
	{
		*m = 0;
		m--;
	}

	//Flags
	ci->asciiMode       = di[ MID_ASCII  ].Selected;
	ci->ShowProcessList = di[ MID_SHLIST ].Selected;
	ci->AddToQueque     = di[ MID_ADDQ   ].Selected;
	ci->UploadLowCase   = di[ MID_UPPER  ].Selected;
	ci->FTPRename       = di[ MID_FTPR   ].Selected;

	//Mode
	if(di[MID_OVER  ].Selected) ci->MsgCode = ocOverAll;
	else if(di[MID_SKIP  ].Selected) ci->MsgCode = ocSkipAll;
	else if(di[MID_RESUME].Selected) ci->MsgCode = ocResumeAll;
	else if(di[MID_NEWER ].Selected) ci->MsgCode = ocNewerAll;

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
	InitDialogItem InitItems[]=
	{
		{DI_DOUBLEBOX, 3, 1,72,17, 0, 0,0,0,FMSG(MEHTitle)},
		{DI_CHECKBOX,5, 2,0,0,0,    0,0,0,FMSG(MDupFF)},
		{DI_CHECKBOX,5, 3,0,0,0,    0,0,0,FMSG(MUndupFF)},
		{DI_CHECKBOX,5, 4,0,0,0,    0,0,0,FMSG(MEHDecodeCmd)},
		{DI_CHECKBOX,5, 5,0,0,0,    0,0,0,FMSG(MSendAllo)},
		{DI_CHECKBOX,5, 6,0,0,0,    0,0,0,FMSG(MUseStartSpaces)},
		{DI_TEXT,3,15,3,15,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,NULL },
		{DI_BUTTON,0,16,0,0,0,0,DIF_CENTERGROUP, 1,   FMSG(MOk)},
		{DI_BUTTON,0,16,0,0,0,0,DIF_CENTERGROUP, 0,   FMSG(MCancel)},
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
	FarDialogItem di[ARRAYSIZE(InitItems)];
	InitDialogItems(InitItems,di,ARRAYSIZE(di));
	di[dDupFF].Selected               = p->FFDup;
	di[dUndupFF].Selected             = p->UndupFF;
	di[dCmdLine].Selected             = p->DecodeCmdLine;
	di[dSendAllo].Selected            = p->SendAllo;
	di[dUseStartSpaces].Selected   = p->UseStartSpaces;

	do
	{
		rc = FDialog(76,19,"FTPExtHost",di,ARRAYSIZE(di));

		if(rc == -1)
			return;

		if(ftp->LongBeep) FP_PeriodReset(ftp->LongBeep);

		di[rc].Focus    = FALSE;

		if(rc != dOk)
			return;

		break;
	}
	while(true);

	p->FFDup              = di[dDupFF].Selected;
	p->UndupFF            = di[dUndupFF].Selected;
	p->DecodeCmdLine      = di[dCmdLine].Selected;
	p->SendAllo           = di[dSendAllo].Selected;
	p->UseStartSpaces  = di[dUseStartSpaces].Selected;
}

BOOL FTP::GetHost(int title,FTPHost* p,BOOL ToDescription)
{
	InitDialogItem InitItems[]=
	{
		{DI_DOUBLEBOX, 3, 1,72,17, 0, 0,0,0,NULL},
		{DI_TEXT,5, 2,0,0,0,    0,0,0,FMSG(MFtpName)},
		{DI_EDIT,7, 3,70, 3,0, (DWORD_PTR)FTP_HOSTHISTORY,DIF_HISTORY,0,NULL},

		{DI_TEXT,4, 4,4, 4,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,NULL },

		{DI_TEXT,5, 5,0,0,0,    0,0,0,FMSG(MUser)},
		{DI_EDIT,18, 5,70, 5,0,(DWORD_PTR)FTP_USERHISTORY,DIF_HISTORY,0,NULL },

		{DI_TEXT,5, 6,0,0,0, 0,0,0,   FMSG(MPassword)},
		{DI_PSWEDIT,18, 6,70, 6,0,0,0,0,NULL},

		{DI_TEXT,5, 7,0,0,0, 0,0,0,   FMSG(MHostDescr)},
		{DI_EDIT,18, 7,70, 7,0,0,0,0,NULL},

		{DI_TEXT,3, 8,3, 8,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,NULL },

		{DI_CHECKBOX,5, 9,0,0,0,  0,0,0,  FMSG(MAskLogin)},
		{DI_CHECKBOX,5,10,0,0,0,  0,0,0,  FMSG(MAsciiMode)},
		{DI_CHECKBOX,5,11,0,0,0,  0,0,0,  FMSG(MPassiveMode)},
		{DI_CHECKBOX,5,12,0,0,0,  0,0,0,  FMSG(MUseFirewall)},

		{DI_CHECKBOX,5,13,0,0,0,  0,0,0,  FMSG(MDecodeCommands)},
		{DI_CHECKBOX,40, 9,0,0,0, 0,0,0,   FMSG(MExtWindow)},
		{DI_CHECKBOX,40,10,0,0,0, 0,0,0,   FMSG(MExtList)},
		{DI_EDIT,44,11, 59,11,0,0,0,0,NULL},
		{DI_TEXT,40,12,0,0,0, 0,0,0,   FMSG(MHostIOSize)},
		{DI_EDIT,60,12, 70,12,0,0,0,0,NULL},

		{DI_TEXT,3,14,3,14,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,NULL },

		{DI_BUTTON,0,15,0,0,0,0,DIF_CENTERGROUP, 0,   FMSG(MFtpSelectTable)},
		{DI_BUTTON,0,15,0,0,0,0,DIF_CENTERGROUP, 0,   FMSG(MServerType)},
		{DI_BUTTON,0,15,0,0,0,0,DIF_CENTERGROUP, 0,   FMSG(MExtOpt)},
		{DI_BUTTON,0,16,0,0,0,0,DIF_CENTERGROUP,  1,  FMSG(MFtpSave)},
		{DI_BUTTON,0,16,0,0,0,0,DIF_CENTERGROUP,  0,  FMSG(MFtpConnect)},
		{DI_BUTTON,0,16,0,0,0,0,DIF_CENTERGROUP, 0,   FMSG(MCancel)},
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
	FarDialogItem di[ARRAYSIZE(InitItems)];
	FTPHost       tmp;
	tmp = *p;
	InitDialogItems(InitItems,di,ARRAYSIZE(di));
	strcpy(di[0].Data, FP_GetMsg(title));

	if(ToDescription)
		di[dDESC].Focus = TRUE;
	else
		di[dHOST].Focus = TRUE;

	strcpy(di[dHOST].Data,p->HostName);
	strcpy(di[dUSER].Data,p->User);
	strcpy(di[dPSWD].Data,p->Password);
	strcpy(di[dDESC].Data,p->HostDescr);
	Size2Str(di[dIOSIZE].Data,p->IOBuffSize);
	di[dLOGIN].Selected   = p->AskLogin;
	di[dASCII].Selected   = p->AsciiMode;
	di[dPASV].Selected    = p->PassiveMode;
	di[dFIRW].Selected    = p->UseFirewall;
	di[dEXTCMD].Selected  = p->ExtCmdView;
	di[dEXTLIST].Selected = p->ExtList;
	di[dCODECMD].Selected = p->CodeCmd;
	strcpy(di[dLISTCMD].Data,p->ListCMD);
	strcpy(TableName,p->HostTable);

	do
	{
		rc = FDialog(76,19,"FTPConnect",di,ARRAYSIZE(di));

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
	while(true);

	*p = tmp;
	p->SetHostName(Name,
	               di[dUSER].Data,
	               di[dPSWD].Data);
	StrCpy(p->HostDescr,di[dDESC].Data,ARRAYSIZE(p->HostDescr));
	strcpy(p->HostTable,TableName);
	p->AskLogin    = di[dLOGIN].Selected;
	p->AsciiMode   = di[dASCII].Selected;
	p->PassiveMode = di[dPASV].Selected;
	p->UseFirewall = di[dFIRW].Selected;
	p->ExtCmdView  = di[dEXTCMD].Selected;
	p->ExtList     = di[dEXTLIST].Selected;
	p->CodeCmd     = di[dCODECMD].Selected;
	p->IOBuffSize  = Max((DWORD)FTR_MINBUFFSIZE,Str2Size(di[dIOSIZE].Data));
	StrCpy(p->ListCMD,di[dLISTCMD].Data,ARRAYSIZE(p->ListCMD));

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
	InitDialogItem InitItems[]=
	{
		{DI_DOUBLEBOX, 3, 1,72,6, 0, 0,0,0,NULL},
		{DI_TEXT,5, 2,0,0,0, 0,0,0,   FMSG(MMkdirName)},
		{DI_EDIT,5, 3,70, 3,0,(DWORD_PTR)FTP_FOLDHISTORY,DIF_HISTORY, 0,},
		{DI_TEXT,5, 4,0,0,0, 0,0,0,   FMSG(MHostDescr)},
		{DI_EDIT,5, 5,70, 5,0,0,0,0,NULL},
	};
	InitItems[0].Data = newDir ? FMSG(MMkdirTitle) : FMSG(MChDir);
	FarDialogItem di[ARRAYSIZE(InitItems)];
	InitDialogItems(InitItems,di,ARRAYSIZE(di));
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
		strcpy(di[4].Data,Desc);
	else
	{
		SET_FLAG(di[3].Flags, DIF_DISABLE);
		SET_FLAG(di[4].Flags, DIF_DISABLE);
	}

	//Exec
	int AskCode = FDialog(76,8,"FTPCmd",di,ARRAYSIZE(di));

	if(AskCode < 0 || !DialogEditBuffer[0])
		return FALSE;

	//Fields -> Data
	Name = DialogEditBuffer;

	if(Desc) strcpy(Desc,di[4].Data);

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
	strcpy(MenuItems[0].Text,"Windows");
	strcpy(MenuItems[1].Text,"DOS");
	strcpy(MenuItems[2].Text,FP_GetMsg(MTableAuto));
	strcpy(MenuItems[3].Text,"UTF-8");
	int TableNum=4;

	while(TableNum < (int)(ARRAYSIZE(MenuItems)))
	{
		CharTableSet TableSet;

		if(FP_Info->CharTable(TableNum-4,(char*)&TableSet,sizeof(TableSet))==-1)
			break;

		strcpy(MenuItems[TableNum++].Text,TableSet.TableName);
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
			strcpy(TableName,".win");
			break;
		case 1:
			strcpy(TableName,".dos");
			break;
		case 2:
			strcpy(TableName,".auto");
			break;
		case 3:
			strcpy(TableName,".utf8");
			break;
		default:
			strcpy(TableName,MenuItems[ExitCode].Text);
			break;
	}

	FP_SetRegKey("CharTable",TableName);
}

void FTP::SelectFileTable(char *TableName)
{
	struct FarMenuItem MenuItems[50];
	memset(MenuItems, 0, sizeof(MenuItems));
	strcpy(MenuItems[0].Text,FP_GetMsg(MTableDefault));
	strcpy(MenuItems[1].Text,"Windows");
	strcpy(MenuItems[2].Text,"DOS");
	strcpy(MenuItems[3].Text,FP_GetMsg(MTableAuto));
	strcpy(MenuItems[4].Text,"UTF-8");

	if(StrCmp(TableName,".win")==0)
		MenuItems[1].Selected=TRUE;

	if(StrCmp(TableName,".dos")==0)
		MenuItems[2].Selected=TRUE;

	if(StrCmp(TableName,".auto")==0)
		MenuItems[3].Selected=TRUE;

	if(StrCmp(TableName,".utf8")==0)
		MenuItems[4].Selected=TRUE;

	int TableNum=5;

	while(TableNum < (int)ARRAYSIZE(MenuItems))
	{
		CharTableSet TableSet;

		if(FP_Info->CharTable(TableNum-5,(char*)&TableSet,sizeof(TableSet))==-1)
			break;

		if(StrCmp(TableName,TableSet.TableName)==0)
			MenuItems[TableNum].Selected=TRUE;

		strcpy(MenuItems[TableNum++].Text,TableSet.TableName);
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
			strcpy(TableName,".win");
			break;
		case 2:
			strcpy(TableName,".dos");
			break;
		case 3:
			strcpy(TableName,".auto");
			break;
		case 4:
			strcpy(TableName,".utf8");
			break;
		default:
			strcpy(TableName,MenuItems[ExitCode].Text);
			break;
	}
}
//---------------------------------------------------------------------------------
void FTP::SetAttributes()
{
	InitDialogItem InitItems[]=
	{
		{DI_DOUBLEBOX, 3, 1,35,6, 0, 0,0,0,FMSG(MChmodTitle)},
		{DI_TEXT,6, 2,0,0,0,    0,0,0,FMSG("R  W  X   R  W  X   R  W  X")},
		{DI_CHECKBOX,5, 3,0,0,0, 0,0,0,   NULL},
		{DI_CHECKBOX,8, 3,0,0,0,  0,0,0,  NULL},
		{DI_CHECKBOX,11, 3,0,0,0,  0,0,0,  NULL},
		{DI_CHECKBOX,15, 3,0,0,0, 0,0,0,   NULL},
		{DI_CHECKBOX,18, 3,0,0,0,  0,0,0,  NULL},
		{DI_CHECKBOX,21, 3,0,0,0,  0,0,0,  NULL},
		{DI_CHECKBOX,25, 3,0,0,0,  0,0,0,  NULL},
		{DI_CHECKBOX,28, 3,0,0,0, 0,0,0,   NULL},
		{DI_CHECKBOX,31, 3,0,0,0,  0,0,0,  NULL},
		{DI_TEXT,3, 4,3, 4,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,NULL },
		{DI_BUTTON,0, 5,0,0,0,0,DIF_CENTERGROUP, 1,  FMSG(MOk)},
		{DI_BUTTON,0, 5,0,0,0,0,DIF_CENTERGROUP, 0,  FMSG(MCancel)},
	};
	FarDialogItem di[ARRAYSIZE(InitItems)];
	PanelInfo     PInfo;
	DWORD         Mode=0;
	int           n,AskCode;
	char         *m;
	InitDialogItems(InitItems,di,ARRAYSIZE(di));
	FP_Info->Control(this,FCTL_GETPANELINFO,&PInfo);

	if(PInfo.SelectedItemsNumber == 1)
	{
		m = PInfo.SelectedItems[0].CustomColumnData[ FTP_COL_MODE ];

		if(!m[0])
			return;

		for(n = 2; n <= 10; n++)
			di[ n ].Selected = m[ n-1 ] != '-';
	}

	AskCode = FDialog(39,8,"FTPCmd",di,ARRAYSIZE(di));

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
			/*skip*/     case 0:
				break;
				/*skip all*/
			case 1:
				AskCode = FALSE;
				break;
			default:
				SetLastError(ERROR_CANCELLED);
				return;
		}
	}
}
//---------------------------------------------------------------------------------
BOOL WINAPI GetLoginData(char *User, char *Password, BOOL forceAsk)
{
	InitDialogItem InitItems[]=
	{
		{DI_DOUBLEBOX, 3, 1,72,8, 0,0,0,0, FMSG(MLoginInfo)},
		{DI_TEXT,5, 2,0,0,0,  0,0,0,  FMSG(MUserName)},
		{DI_EDIT,5, 3,70, 3,0,(DWORD_PTR)FTP_USERHISTORY,DIF_HISTORY, 0,NULL},
		{DI_TEXT,5, 4,0,0,0,  0,0,0,  FMSG(MUserPassword)},
		{DI_PSWEDIT,5, 5,70, 5,0,0,0,0,NULL},
		{DI_TEXT,5, 6,5, 6,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,NULL },
		{DI_BUTTON,0, 7,0,0,0,0,DIF_CENTERGROUP, 1,  FMSG(MOk)},
		{DI_BUTTON,0, 7,0,0,0,0,DIF_CENTERGROUP, 0,  FMSG(MCancel)},
	};

	if(User[0] && !Password[0])
		forceAsk = TRUE;

	//Default user
	if(!User[0] && Opt.AutoAnonymous)
		strcpy(User,"anonymous");

	//Default passw
	if(!Password[0] && stricmp(User,"anonymous") == 0)
		StrCpy(Password, Opt.DefaultPassword, FAR_MAX_NAME);

	if(!forceAsk)
		return TRUE;

	FarDialogItem di[ARRAYSIZE(InitItems)];
	InitDialogItems(InitItems,di,ARRAYSIZE(di));

	if(*User==0 || *Password!=0)
		di[2].Focus = TRUE;
	else
		di[4].Focus = TRUE;

	strcpy(di[2].Data, User);
	strcpy(di[4].Data, Password);

	do
	{
		int AskCode = FP_Info->Dialog(FP_Info->ModuleNumber,-1,-1,
		                              76,10,"FTPConnect",
		                              di,ARRAYSIZE(di));

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
	while(true);
}
