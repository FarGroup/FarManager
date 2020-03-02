#include <all_far.h>
#pragma hdrstop

#include "Int.h"

void SetTitles(char *cols[], LPCSTR fmt,int cn)
{
	int n = 0;

	if(!fmt || *fmt == 0 || !cn)
		return;

	do
	{
		if(*fmt == 'C' || *fmt == 'c')
		{
			fmt++;

			switch(*fmt)
			{
				case '0':
					cols[n] = (char*)FP_GetMsg(MFileMode);
					break;
				case '1':
					cols[n] = (char*)FP_GetMsg(MLink);
					break;
				default:
					cols[n] = (char*)"<unk>";
			}
		}
		else
			cols[n] = NULL;

		n++;

		if(n >= cn)
			return;

		while(true)
		{
			if(*fmt == 0)
				return;

			if(*fmt == ',')
			{
				fmt++;
				break;
			}

			fmt++;
		}
	}
	while(true);
}

void FTP::GetOpenPluginInfo(struct OpenPluginInfo *pi)
{
	PROC(("GetOpenPluginInfo","%s cc: %d pi: %p hC:%p",ShowHosts?"HOSTS":"FTP",CallLevel,pi,hConnect));
	PanelInfo thisPInfo = { 0 };
	static int inside = 0;
	inside++;
	memset(pi, 0, sizeof(*pi));
	pi->StructSize = sizeof(*pi);

//---------------- FLAGS
	if(!hConnect || ShowHosts)
		pi->Flags = OPIF_SHOWPRESERVECASE | OPIF_USEHIGHLIGHTING;
	else
		pi->Flags = OPIF_ADDDOTS | OPIF_USEFILTER | OPIF_USESORTGROUPS | OPIF_USEHIGHLIGHTING |
		            OPIF_SHOWPRESERVECASE;

	Log(("SetFlags %08X", pi->Flags));
//---------------- HOST, CURDIR
	static String curDir;
	GetCurPath(curDir);
	pi->HostFile = NULL;
	pi->CurDir   = curDir.c_str();
//---------------- TITLE
	pi->PanelTitle = PanelTitle;

	if(ShowHosts)
		_snprintf(PanelTitle, ARRAYSIZE(PanelTitle),
		          " FTP: %s ", pi->CurDir);
	else
		_snprintf(PanelTitle, ARRAYSIZE(PanelTitle),
		          (hConnect&&hConnect->Host.ServerType==FTP_TYPE_MVS)? " FTP: %s@%s/%s ": " FTP: %s@%s%s ",
		          Host.User, Host.Host, pi->CurDir);

	if(inside > 1)
	{
		inside--;
		return;
	}

//---------------- FORMAT
	static String Format;

	if(ShowHosts)
		Format = "//Hosts/";
	else
		Format.printf("//%s/",Host.Host);

	Format.cat(pi->CurDir + (*pi->CurDir == '/' || *pi->CurDir == '\\'));
	pi->Format = Format.c_str();
//---------------- INFO LINES
	static struct InfoPanelLine InfoLines[7];
	char *m;
	memset(InfoLines, 0, sizeof(InfoLines));
//Client
	StrCpy(InfoLines[0].Text,FP_GetMsg(MFtpInfoFTPClient),ARRAYSIZE(InfoLines[0].Text));
	InfoLines[0].Separator = TRUE;
	StrCpy(InfoLines[1].Text,FP_GetMsg(MFtpInfoHostName),ARRAYSIZE(InfoLines[0].Text));
	StrCpy(InfoLines[1].Data,Host.HostName,ARRAYSIZE(InfoLines[0].Data));
	StrCpy(InfoLines[2].Text,FP_GetMsg(MFtpInfoHostDescr),ARRAYSIZE(InfoLines[0].Text));
	StrCpy(InfoLines[2].Data,Host.HostDescr,ARRAYSIZE(InfoLines[0].Data));
	StrCpy(InfoLines[3].Text,FP_GetMsg(MFtpInfoHostType),ARRAYSIZE(InfoLines[0].Text));

	if(hConnect)
		FtpSystemInfo(hConnect,InfoLines[3].Data,ARRAYSIZE(InfoLines[0].Data));
	else
		InfoLines[3].Data[0] = 0;

//Titles
	StrCpy(InfoLines[4].Text,FP_GetMsg(MFtpInfoFtpTitle),ARRAYSIZE(InfoLines[0].Text));
	InfoLines[4].Separator=TRUE;
	InfoLines[5].Text[0] = 0;

	if(hConnect)
		StrCpy(InfoLines[5].Data, hConnect->GetStartReply(), ARRAYSIZE(InfoLines[5].Data));
	else
		InfoLines[5].Data[0] = 0;

	m = strpbrk(InfoLines[5].Data,"\n\r");

	if(m) *m = 0;

	StrCpy(InfoLines[6].Text, FP_GetMsg(MResmResume), ARRAYSIZE(InfoLines[0].Text));

	if(hConnect)
		StrCpy(InfoLines[6].Data, FP_GetMsg(FtpIsResume(hConnect)?MResmSupport:MResmNotSupport), ARRAYSIZE(InfoLines[0].Data));
	else
		StrCpy(InfoLines[6].Data, FP_GetMsg(MResmNotConnected), ARRAYSIZE(InfoLines[0].Data));

	pi->InfoLines       = InfoLines;
	pi->InfoLinesNumber = 7;
//---------------- DESCR
	static char *DescrFiles[32],
	       DescrFilesString[256];
	StrCpy(DescrFilesString, Opt.DescriptionNames, ARRAYSIZE(DescrFilesString));
	int   DescrFilesNumber = 0;
	char *NamePtr          = DescrFilesString;

	while(DescrFilesNumber < (int)ARRAYSIZE(DescrFiles))
	{
		while(isspace(*NamePtr)) NamePtr++;

		if(*NamePtr == 0) break;

		DescrFiles[DescrFilesNumber++] = NamePtr;

		if((NamePtr=strchr(NamePtr,',')) == NULL) break;

		*(NamePtr++)=0;
	}

	pi->DescrFiles=DescrFiles;

	if(!Opt.ReadDescriptions)
		pi->DescrFilesNumber=0;
	else
		pi->DescrFilesNumber=DescrFilesNumber;

//---------------- SHORTCUT
	static String ShortcutData;

	if(ShowHosts)
	{
		/*
		HOSTSTS
		  Hostspath
		*/
		ShortcutData.printf("HOST:%s", HostsPath);
	}
	else
	{
		/*
		FTP
		  Host
		  1
		  AskLogin    + 3
		  AsciiMode   + 3
		  PassiveMode + 3
		  UseFirewall + 3
		  HostTable
		  1
		  User
		  1
		  Password
		  1
		  ExtCmdView + 3
		  IOBuffSize (atoi)
		  1
		  FFDup + '0'
		  DecodeCmdLine + '0'
		  1
		*/
		ShortcutData.printf("FTP:%s\x1%c%c%c%c%d\x1%s\x1%s\x1%s\x1%c%d\x1%c\x1%c\x1",
		                    Host.Host,
		                    Host.AskLogin+3, Host.AsciiMode+3, Host.PassiveMode+3, Host.UseFirewall+3, Host.ServerType,
		                    Host.HostTable,
		                    Host.User,
		                    Host.Password,
		                    Host.ExtCmdView+3, Host.IOBuffSize,
		                    '0'+Host.FFDup, '0'+Host.DecodeCmdLine);
	}

	pi->ShortcutData = ShortcutData.c_str();

//---------------- PANEL MODES
//HOSTST
	if(ShowHosts)
	{
		static struct PanelMode PanelModesArray[10];
		static char *ColumnTitles[4] = { NULL };
		static char *ColumnTitles2[4] = { NULL };
		static char  Mode[ 20 ], ModeSz[20], ModeSz2[20];
		int      dizLen = 0,
		         nmLen  = 0,
		         hLen   = 0,
		         hstLen = 0;
		int      n,num;
		FTPHost* p;
		memset(&PanelModesArray,0,sizeof(PanelModesArray));

		if(!thisPInfo.PanelItems)
			FP_Info->Control(this, FCTL_GETPANELINFO, &thisPInfo);

		for(n = 0; n < thisPInfo.ItemsNumber; n++)
		{
			p = FTPHost::Convert(&thisPInfo.PanelItems[n]);

			if(!p) continue;

			dizLen = Max(dizLen,(int)strlen(p->HostDescr));
			nmLen  = Max(nmLen, (int)strlen(p->User));
			hLen   = Max(hLen, (int)strlen(p->Home));
			hstLen = Max(hstLen,(int)strlen(p->Host));
		}

		ColumnTitles[0] = (char*)FP_GetMsg(MHostColumn);
		//==1
		PanelModesArray[1].ColumnTypes   = (char *)"C0";
		PanelModesArray[1].ColumnWidths  = (char *)"0";
		//==2
		num = 1;
		n   = (thisPInfo.PanelRect.right-thisPInfo.PanelRect.left)/2;
		//HOST
		strcpy(Mode,"C0");
		sprintf(ModeSz,hLen || nmLen || dizLen ? "%d" : "0",hstLen);

		//HOME
		if(hLen)
		{
			strcat(Mode,",C1");

			if(hLen < n && (nmLen || dizLen))
			{
				strcat(ModeSz,Message(",%d",hLen));
				n -= hLen;
			}
			else
				strcat(ModeSz,",0");

			ColumnTitles[num++] = (char*)FP_GetMsg(MHomeColumn);
		}

		//UNAME
		if(nmLen)
		{
			strcat(Mode,",C2");

			if(nmLen < n && dizLen)
			{
				strcat(ModeSz,Message(",%d",nmLen));
			}
			else
				strcat(ModeSz,",0");

			ColumnTitles[num++] = (char*)FP_GetMsg(MUserColumn);
		}

		//DIZ
		if(dizLen)
		{
			strcat(Mode,",Z");
			strcat(ModeSz,",0");
			ColumnTitles[num] = (char*)FP_GetMsg(MDescColumn);
		}

		PanelModesArray[2].ColumnTypes   = Mode;
		PanelModesArray[2].ColumnWidths  = ModeSz;
		PanelModesArray[2].ColumnTitles  = ColumnTitles;
		ColumnTitles2[0] = (char*)FP_GetMsg(MHostColumn);
		ColumnTitles2[1] = (char*)FP_GetMsg(MDescColumn);

		if(!dizLen)
		{
			PanelModesArray[3].ColumnTypes   = (char *)"C0";
			PanelModesArray[3].ColumnWidths  = (char *)"0";
		}
		else
		{
			PanelModesArray[3].ColumnTypes   = (char *)"C0,Z";
			PanelModesArray[3].ColumnWidths  = ModeSz2;
			sprintf(ModeSz2,"%d,0",Min((int)(thisPInfo.PanelRect.right-thisPInfo.PanelRect.left)/2,hstLen));
		}

		PanelModesArray[3].ColumnTitles  = ColumnTitles2;
		pi->PanelModesArray  = PanelModesArray;
		pi->PanelModesNumber = ARRAYSIZE(PanelModesArray);
		pi->StartPanelMode   = 0;

		for(n = 1; n <= 3; n++)
		{
			PanelModesArray[n].StatusColumnTypes   = PanelModesArray[n].ColumnTypes;
			PanelModesArray[n].StatusColumnWidths  = PanelModesArray[n].ColumnWidths;
		}
	}
	else
	{
//FTP
		static struct PanelMode PanelModesArray[10];
		memset(PanelModesArray, 0, sizeof(PanelModesArray));
		static char *ColumnTitles[10];
		SetTitles(ColumnTitles, FP_GetMsg(MColumn9), ARRAYSIZE(ColumnTitles));
		PanelModesArray[9].ColumnTypes  = (char*)FP_GetMsg(MColumn9);
		PanelModesArray[9].ColumnWidths = (char*)FP_GetMsg(MSizes9);
		PanelModesArray[9].ColumnTitles = ColumnTitles;
		PanelModesArray[9].FullScreen   = atoi(FP_GetMsg(MFullScreen9));
		static char *ColumnTitles1[10];
		SetTitles(ColumnTitles1, FP_GetMsg(MColumn0), ARRAYSIZE(ColumnTitles));
		PanelModesArray[0].ColumnTypes  = (char*)FP_GetMsg(MColumn0);
		PanelModesArray[0].ColumnWidths = (char*)FP_GetMsg(MSizes0);
		PanelModesArray[0].ColumnTitles = ColumnTitles1;
		PanelModesArray[0].FullScreen   = atoi(FP_GetMsg(MFullScreen0));
		pi->PanelModesArray  = PanelModesArray;
		pi->PanelModesNumber = ARRAYSIZE(PanelModesArray);
	}

//---------------- KEYBAR
	static struct KeyBarTitles KeyBar;
	memset(&KeyBar,0,sizeof(KeyBar));
	KeyBar.ShiftTitles[1-1] = (char *)"";
	KeyBar.ShiftTitles[2-1] = (char *)"";
	KeyBar.ShiftTitles[3-1] = (char *)"";
	KeyBar.AltTitles[6-1]   = (char*)FP_GetMsg(MAltF6);

	if(ShowHosts)
	{
		KeyBar.ShiftTitles[1-1] = (char*)FP_GetMsg(MShiftF1);
		KeyBar.ShiftTitles[4-1] = ShowHosts ? (char*)FP_GetMsg(MShiftF4):NULL;
	}
	else
	{
		KeyBar.ShiftTitles[1-1] = (char*)FP_GetMsg(MShiftF1);
		KeyBar.ShiftTitles[7-1] = (char*)FP_GetMsg(MShiftF7);
	}

	pi->KeyBar=&KeyBar;

//---------------- RESTORE SCREEN
	if(!SkipRestoreScreen && CurrentState != fcsExpandList && CurrentState != fcsOperation && !IS_SILENT(FP_LastOpMode))
		FP_Screen::FullRestore();

//Back
	inside--;
}
