/*
infolist.cpp

Информационная панель

*/

#include "headers.hpp"
#pragma hdrstop

#include "infolist.hpp"
#include "macroopcode.hpp"
#include "global.hpp"
#include "fn.hpp"
#include "flink.hpp"
#include "colors.hpp"
#include "lang.hpp"
#include "keys.hpp"
#include "ctrlobj.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "help.hpp"
#include "fileview.hpp"
#include "fileedit.hpp"
#include "manager.hpp"

static int LastDizWrapMode = -1;
static int LastDizWrapType = -1;
/* $ 27.04.2001 DJ
   запоминаем, был ли включен скроллбар
*/
static int LastDizShowScrollbar = -1;
/* DJ $ */

InfoList::InfoList()
{
	Type=INFO_PANEL;
	DizView=NULL;
	PrevMacroMode=-1;
	*DizFileName=0;

	if (LastDizWrapMode < 0)
	{
		LastDizWrapMode = Opt.ViOpt.ViewerIsWrap;
		LastDizWrapType = Opt.ViOpt.ViewerWrap;
		/* $ 27.04.2001 DJ
		   запоминаем, был ли включен скроллбар
		*/
		LastDizShowScrollbar = Opt.ViOpt.ShowScrollbar;
		/* DJ $ */
	}
}

InfoList::~InfoList()
{
	/* $ 30.04.2001 DJ
	   CloseDizFile() -> CloseFile()
	*/
	CloseFile();
	/* DJ $ */
	SetMacroMode(TRUE);
}

/* $ 26.03.2002 DJ
   перерисовка, только если мы текущий фрейм
*/
void InfoList::Update(int Mode)
{
	if (!EnableUpdate)
		return;

	if (CtrlObject->Cp() == FrameManager->GetCurrentFrame())
		Redraw();
}

const char *InfoList::GetTitle(char *lTitle,int LenTitle,int TruncSize)
{
	char Title[512];
	sprintf(Title," %s ",MSG(MInfoTitle));
	TruncStr(Title,X2-X1-3);
	xstrncpy(lTitle,Title,LenTitle);
	return lTitle;
}

void InfoList::DisplayObject()
{
	char Title[NM],OutStr[200];
	Panel *AnotherPanel;
	char DriveRoot[NM],VolumeName[NM],FileSystemName[NM];
	DWORD MaxNameLength,FileSystemFlags,VolumeNumber;
	CloseFile();
	Box(X1,Y1,X2,Y2,COL_PANELBOX,DOUBLE_BOX);
	SetScreen(X1+1,Y1+1,X2-1,Y2-1,' ',COL_PANELTEXT);
	SetColor(Focus ? COL_PANELSELECTEDTITLE:COL_PANELTITLE);
	GetTitle(Title,sizeof(Title)-1);

	if (*Title)
	{
		GotoXY(X1+(X2-X1+1-(int)strlen(Title))/2,Y1);
		Text(Title);
	}

	DrawSeparator(Y1+3);
	DrawSeparator(Y1+8);
	SetColor(COL_PANELTEXT);
	{
		char ComputerName[64],UserName[256];
		DWORD ComputerNameSize=sizeof(ComputerName),UserNameSize=sizeof(UserName);
		*ComputerName=*UserName=0;
		GetComputerName(ComputerName,&ComputerNameSize);
		/*
			http://msdn.microsoft.com/en-us/library/ms724268%28VS.85%29.aspx
			 0 = NameUnknown           An unknown name type.
			 1 = NameFullyQualifiedDN  The fully-qualified distinguished name (for example, CN=Jeff Smith,OU=Users,DC=Engineering,DC=Microsoft,DC=Com).
			 2 = NameSamCompatible     A legacy account name (for example, Engineering\JSmith). The domain-only version includes trailing backslashes (\\).
			 3 = NameDisplay           A "friendly" display name (for example, Jeff Smith). The display name is not necessarily the defining relative distinguished name (RDN).
			 6 = NameUniqueId          A GUID string that the IIDFromString function returns (for example, {4fa050f0-f561-11cf-bdd9-00aa003a77b6}).
			 7 = NameCanonical         The complete canonical name (for example, engineering.microsoft.com/software/someone). The domain-only version includes a trailing forward slash (/).
			 8 = NameUserPrincipal     The user principal name (for example, someone@example.com).
			 9 = NameCanonicalEx       The same as NameCanonical except that the rightmost forward slash (/) is replaced with a new line character (\n), even in a domain-only case (for example, engineering.microsoft.com/software\nJSmith).
			10 = NameServicePrincipal  The generalized service principal name (for example, www/www.microsoft.com@microsoft.com).
			12 = NameDnsDomain         The DNS domain name followed by a backward-slash and the SAM username.
		*/
		FAR_GetUserNameEx(8,UserName,&UserNameSize);
		FAR_CharToOem(ComputerName,ComputerName);
		FAR_CharToOem(UserName,UserName);
		GotoXY(X1+2,Y1+1);
		PrintText(MInfoCompName);
		PrintInfo(ComputerName);
		GotoXY(X1+2,Y1+2);
		PrintText(MInfoUserName);
		PrintInfo(UserName);
	}
	AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);
	AnotherPanel->GetCurDir(CurDir);

	if (*CurDir==0)
		FarGetCurDir(sizeof(CurDir),CurDir);

	/* $ 01.02.2001 SVS
	   В Win2K корректно отображать инфу при заходе в Juction каталог
	   Здесь Рут-диск может быть другим
	*/
	if ((GetFileAttributes(CurDir)&FILE_ATTRIBUTE_REPARSE_POINT) == FILE_ATTRIBUTE_REPARSE_POINT)
	{
		char JuncName[NM];

		if (GetReparsePointInfo(CurDir,JuncName,sizeof(JuncName)))
		{
			int offset = 0;

			if (!strncmp(JuncName,"\\??\\",4))
				offset = 4;

			GetPathRoot(JuncName+offset,DriveRoot); //"\??\D:\Junc\Src\"
		}
	}
	else
		GetPathRoot(CurDir,DriveRoot);

	/* SVS $ */

	if (GetVolumeInformation(DriveRoot,VolumeName,sizeof(VolumeName),
	                         &VolumeNumber,&MaxNameLength,&FileSystemFlags,
	                         FileSystemName,sizeof(FileSystemName)))
	{
		char LocalName[8], DiskType[100], RemoteName[NM], DiskName[NM];
		int ShowRealPath=FALSE;
		int DriveType=FAR_GetDriveType(DriveRoot,NULL,TRUE);
		sprintf(LocalName,"%c:",*DriveRoot);

		if (DriveRoot[0] && DriveRoot[1]==':')
			sprintf(DiskName,"%c:",toupper(*DriveRoot));
		else
			strcpy(DiskName,DriveRoot);

		int IdxMsgID=-1;
		*DiskType=0;

		switch (DriveType)
		{
			case DRIVE_REMOVABLE:
				IdxMsgID=MInfoRemovable;
				break;
			case DRIVE_FIXED:
				IdxMsgID=MInfoFixed;
				break;
			case DRIVE_REMOTE:
				IdxMsgID=MInfoNetwork;
				break;
			case DRIVE_CDROM:
				IdxMsgID=MInfoCDROM;
				break;
			case DRIVE_RAMDISK:
				IdxMsgID=MInfoRAM;
				break;
			default:

				if (IsDriveTypeCDROM(DriveType))
					IdxMsgID=DriveType-DRIVE_CD_RW+MInfoCD_RW;
				else
					*DiskType=0;

				break;
		}

		if (IdxMsgID != -1)
			strcpy(DiskType,MSG(IdxMsgID));

		{
			if (GetSubstName(DriveType,LocalName,RemoteName,sizeof(RemoteName)))
			{
				strcpy(DiskType,MSG(MInfoSUBST));
				DriveType=DRIVE_SUBSTITUTE;
			}
		}
		sprintf(Title," %s %s %s (%s) ",DiskType,MSG(MInfoDisk),DiskName,FileSystemName);

		if (DriveType==DRIVE_REMOTE)
		{
			DWORD RemoteNameSize=sizeof(RemoteName);

			if (WNetGetConnection(LocalName,RemoteName,&RemoteNameSize)==NO_ERROR)
				ShowRealPath=TRUE;
		}
		else if (DriveType == DRIVE_SUBSTITUTE)
			ShowRealPath=TRUE;

		if (ShowRealPath)
		{
			FAR_CharToOem(RemoteName,RemoteName);
			strcat(Title,RemoteName);
			strcat(Title," ");
		}

		TruncStr(Title,X2-X1-3);
		GotoXY(X1+(X2-X1+1-(int)strlen(Title))/2,Y1+3);
		PrintText(Title);
		unsigned __int64 TotalSize,TotalFree,UserFree;

		if (GetDiskSize(DriveRoot,&TotalSize,&TotalFree,&UserFree))
		{
			GotoXY(X1+2,Y1+4);
			PrintText(MInfoDiskTotal);
			InsertCommas(TotalSize,OutStr,sizeof(OutStr));
			PrintInfo(OutStr);
			GotoXY(X1+2,Y1+5);
			PrintText(MInfoDiskFree);
			InsertCommas(UserFree,OutStr,sizeof(OutStr));
			PrintInfo(OutStr);
		}

		GotoXY(X1+2,Y1+6);
		PrintText(MInfoDiskLabel);
		PrintInfo(VolumeName);
		GotoXY(X1+2,Y1+7);
		PrintText(MInfoDiskNumber);
		sprintf(OutStr,"%04X-%04X",VolumeNumber>>16,VolumeNumber & 0xffff);
		PrintInfo(OutStr);
	}

	strcpy(Title,MSG(MInfoMemory));
	GotoXY(X1+(X2-X1-(int)strlen(Title))/2,Y1+8);
	PrintText(Title);
	MEMORYSTATUSEX ms;
	FAR_GlobalMemoryStatusEx(&ms);

	if (ms.dwMemoryLoad==0)
		ms.dwMemoryLoad=100-ToPercent64(ms.ullAvailPhys+ms.ullAvailPageFile,ms.ullTotalPhys+ms.ullTotalPageFile);

	GotoXY(X1+2,Y1+9);
	PrintText(MInfoMemoryLoad);
	sprintf(OutStr,"%d%%",ms.dwMemoryLoad);
	PrintInfo(OutStr);
	GotoXY(X1+2,Y1+10);
	PrintText(MInfoMemoryTotal);
	InsertCommas(static_cast<unsigned __int64>(ms.ullTotalPhys),OutStr,sizeof(OutStr));
	PrintInfo(OutStr);
	GotoXY(X1+2,Y1+11);
	PrintText(MInfoMemoryFree);
	InsertCommas(static_cast<unsigned __int64>(ms.ullAvailPhys),OutStr,sizeof(OutStr));
	PrintInfo(OutStr);
	GotoXY(X1+2,Y1+12);
	PrintText(MInfoVirtualTotal);
	InsertCommas(static_cast<unsigned __int64>(ms.ullTotalPageFile),OutStr,sizeof(OutStr));
	PrintInfo(OutStr);
	GotoXY(X1+2,Y1+13);
	PrintText(MInfoVirtualFree);
	InsertCommas(static_cast<unsigned __int64>(ms.ullAvailPageFile),OutStr,sizeof(OutStr));
	PrintInfo(OutStr);
	ShowDirDescription();
	ShowPluginDescription();
}


__int64 InfoList::VMProcess(int OpCode,void *vParam,__int64 iParam)
{
	if (DizView!=NULL)
		return DizView->VMProcess(OpCode,vParam,iParam);

	switch (OpCode)
	{
		case MCODE_C_EMPTY:
			return _i64(1);
	}

	return _i64(0);
}

int InfoList::ProcessKey(int Key)
{
	if (!IsVisible())
		return(FALSE);

	if (ProcessShortcutFolder(Key,FALSE))
		return(TRUE);

	switch (Key)
	{
			/* $ 30.04.2001 DJ
			   показываем правильную тему хелпа
			*/
		case KEY_F1:
		{
			Help Hlp("InfoPanel");
		}
		return TRUE;
		/* DJ $ */
		case KEY_F3:
		case KEY_NUMPAD5:  case KEY_SHIFTNUMPAD5:

			if (*DizFileName)
			{
				CtrlObject->Cp()->GetAnotherPanel(this)->GetCurDir(CurDir);
				FarChDir(CurDir);
				new FileViewer(DizFileName,TRUE);//OT
			}

			/* $ 20.07.2000 tran
			   после показа перерисовываем панели */
			CtrlObject->Cp()->Redraw();
			/* tran 20.07.2000 $ */
			return(TRUE);
		case KEY_F4:
			/* $ 30.04.2001 DJ
			   не показываем редактор, если ничего не задано в именах файлов;
			   не редактируем имена описаний со звездочками;
			   убираем лишнюю перерисовку панелей
			*/
		{
			Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);
			AnotherPanel->GetCurDir(CurDir);
			FarChDir(CurDir);

			if (*DizFileName)
				new FileEditor(DizFileName,FFILEEDIT_ENABLEF6);
			else if (*Opt.FolderInfoFiles)
			{
				char ArgName[NM];
				const char *p = Opt.FolderInfoFiles;

				while ((p = GetCommaWord(p,ArgName)) != NULL)
				{
					if (!strpbrk(ArgName, "*?"))
					{
						new FileEditor(ArgName,FFILEEDIT_CANNEWFILE|FFILEEDIT_ENABLEF6);
						break;
					}
				}
			}

			AnotherPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
//        AnotherPanel->Redraw();
			Update(0);
		}
		/* DJ $ */
		/* $ 20.07.2000 tran
		   после показа перерисовываем панели */
		CtrlObject->Cp()->Redraw();
		/* tran 20.07.2000 $ */
		return(TRUE);
		case KEY_CTRLR:
			Redraw();
			return(TRUE);
	}

	/* $ 30.04.2001 DJ
	   обновляем кейбар после нажатия F8, F2 или Shift-F2
	*/
	if (DizView!=NULL && Key>=256)
	{
		int ret = DizView->ProcessKey(Key);

		if (Key == KEY_F8 || Key == KEY_F2 || Key == KEY_SHIFTF2)
		{
			DynamicUpdateKeyBar();
			CtrlObject->MainKeyBar->Redraw();
		}

		if (Key == KEY_F7 || Key == KEY_SHIFTF7)
		{
			__int64 Pos, Length;
			DWORD Flags;
			DizView->GetSelectedParam(Pos,Length,Flags);
//      ShellUpdatePanels(NULL,FALSE);
			DizView->InRecursion++;
			Redraw();
			CtrlObject->Cp()->GetAnotherPanel(this)->Redraw();
			DizView->SelectText(Pos,Length,Flags|1);
			DizView->InRecursion--;
		}

		return(ret);
	}

	/* DJ $ */
	return(FALSE);
}


int InfoList::ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent)
{
	int RetCode;

	if (Panel::PanelProcessMouse(MouseEvent,RetCode))
		return(RetCode);

	/* $ 29.05.2001 tran
	   DizView может быть равен 0 */
	if (MouseEvent->dwMousePosition.Y>=14 && DizView!=NULL)
	{
		/* $ 27.04.2001 DJ
		   позволяем использовать скроллбар, если он включен
		*/
		_tran(SysLog("InfoList::ProcessMouse() DizView = %p",DizView));
		/* $ 12.10.2001 SKV
		  одноко аккуратно посчитаем окошко DizView,
		  и оставим 2 символа на скроллинг мышой.
		*/
		int DVX1,DVX2,DVY1,DVY2;
		DizView->GetPosition(DVX1,DVY1,DVX2,DVY2);

		if ((MouseEvent->dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) &&
		        MouseEvent->dwMousePosition.X > DVX1+1 &&
		        MouseEvent->dwMousePosition.X < DVX2 - DizView->GetShowScrollbar() - 1 &&
		        MouseEvent->dwMousePosition.Y > DVY1+1 &&
		        MouseEvent->dwMousePosition.Y < DVY2-1
		   )
		{
			ProcessKey(KEY_F3);
			return(TRUE);
		}

		/* SKV$*/

		/* DJ $ */
		if (MouseEvent->dwButtonState & RIGHTMOST_BUTTON_PRESSED)
		{
			ProcessKey(KEY_F4);
			return(TRUE);
		}
	}

	SetFocus();

	if (DizView!=NULL)
		return(DizView->ProcessMouse(MouseEvent));

	return(TRUE);
}


void InfoList::PrintText(const char *Str)
{
	if (WhereY()>Y2-1)
		return;

	mprintf("%.*s",X2-WhereX(),Str);
}


void InfoList::PrintText(int MsgID)
{
	PrintText(MSG(MsgID));
}


void InfoList::PrintInfo(const char *str)
{
	if (WhereY()>Y2-1)
		return;

	int SaveColor=GetColor(),MaxLength=X2-WhereX()-2;

	if (MaxLength<0)
		MaxLength=0;

	/* $ 25.04.2002 IS
	   Скопируем данные во внутренний буфер, т.к. TruncStr изменяет их, а этого
	   делать нельзя, т.к. str может указывать и на r/o-строку
	*/
	char Str[NM*2];
	xstrncpy(Str,str,sizeof(Str));
	/* IS $ */
	TruncStr(Str,MaxLength);
	int Length=(int)strlen(Str);
	int NewX=X2-Length-1;

	if (NewX>X1 && NewX>WhereX())
	{
		GotoXY(NewX,WhereY());
		SetColor(COL_PANELINFOTEXT);
		mprintf("%s ",Str);
		SetColor(SaveColor);
	}
}


void InfoList::PrintInfo(int MsgID)
{
	PrintInfo(MSG(MsgID));
}


void InfoList::ShowDirDescription()
{
	char DizDir[NM];
	int Length;
	Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);
	DrawSeparator(Y1+14);

	if (AnotherPanel->GetMode()==FILE_PANEL)
	{
		AnotherPanel->GetCurDir(DizDir);

		if ((Length=(int)strlen(DizDir))>0 && DizDir[Length-1]!='\\')
			strcat(DizDir,"\\");

		char ArgName[NM];
		const char *NamePtr=Opt.FolderInfoFiles;

		while ((NamePtr=GetCommaWord(NamePtr,ArgName))!=NULL)
		{
			char FullDizName[2048];
			xstrncpy(FullDizName,DizDir,sizeof(FullDizName)-1);

			if (strlen(FullDizName)+strlen(ArgName) < sizeof(FullDizName))
				strcat(FullDizName,ArgName);
			else
				return;

			WIN32_FIND_DATA FindData;

			if (!GetFileWin32FindData(FullDizName,&FindData))
				continue;

			strcpy(PointToName(FullDizName),FindData.cFileName);

			if (OpenDizFile(FullDizName))
				return;
		}
	}

	CloseFile();
	SetColor(COL_PANELTEXT);
	GotoXY(X1+2,Y1+15);
	PrintText(MInfoDizAbsent);
}


void InfoList::ShowPluginDescription()
{
	Panel *AnotherPanel;
	static char VerticalLine[2]={0xBA,0x00};
	AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);

	if (AnotherPanel->GetMode()!=PLUGIN_PANEL)
		return;

	CloseFile();
	struct OpenPluginInfo Info;
	AnotherPanel->GetOpenPluginInfo(&Info);

	for (int I=0; I<Info.InfoLinesNumber; I++)
	{
		int Y=Y2-Info.InfoLinesNumber+I;

		if (Y<=Y1)
			continue;

		const struct InfoPanelLine *InfoLine=&Info.InfoLines[I];
		GotoXY(X1,Y);
		SetColor(COL_PANELBOX);
		//Text(VerticalLine);
		BoxText((WORD)(Opt.UseUnicodeConsole?BoxSymbols[VerticalLine[0]-0x0B0]:VerticalLine[0]));
		SetColor(COL_PANELTEXT);
		mprintf("%*s",X2-X1-1,"");
		SetColor(COL_PANELBOX);
		//Text(VerticalLine);
		BoxText((WORD)(Opt.UseUnicodeConsole?BoxSymbols[VerticalLine[0]-0x0B0]:VerticalLine[0]));
		GotoXY(X1+2,Y);

		if (InfoLine->Separator)
		{
			char Title[200];

			if (InfoLine->Text!=NULL && *InfoLine->Text)
				sprintf(Title," %s ",InfoLine->Text);
			else
				*Title=0;

			DrawSeparator(Y);
			TruncStr(Title,X2-X1-3);
			GotoXY(X1+(X2-X1-(int)strlen(Title))/2,Y);
			PrintText(Title);
		}
		else
		{
			PrintText(NullToEmpty(InfoLine->Text));
			PrintInfo(NullToEmpty(InfoLine->Data));
		}
	}
}

void InfoList::CloseFile()
{
	if (DizView!=NULL)
	{
		/* $ 12.10.2001 SKV
		  Если идёт вызов метода DizView,
		  то не надо делать delete...
		*/
		if (DizView->InRecursion)
			return;

		LastDizWrapMode=DizView->GetWrapMode();
		LastDizWrapType=DizView->GetWrapType();
		LastDizShowScrollbar=DizView->GetShowScrollbar();
		DizView->SetWrapMode(OldWrapMode);
		DizView->SetWrapType(OldWrapType);
		delete DizView;
		DizView=NULL;
	}

	*DizFileName=0;
}

int InfoList::OpenDizFile(char *DizFile)
{
	bool bOK=true;
	_tran(SysLog("InfoList::OpenDizFile([%s]",DizFile));

	if (DizView == NULL)
	{
		DizView=new DizViewer;

		if (!DizView)
			return FALSE;

		_tran(SysLog("InfoList::OpenDizFile() create new Viewer = %p",DizView));
		DizView->SetRestoreScreenMode(FALSE);
		DizView->SetPosition(X1+1,Y1+15,X2-1,Y2-1);
		DizView->SetStatusMode(0);
		DizView->EnableHideCursor(0);
		OldWrapMode = DizView->GetWrapMode();
		OldWrapType = DizView->GetWrapType();
		DizView->SetWrapMode(LastDizWrapMode);
		DizView->SetWrapType(LastDizWrapType);
		DizView->SetShowScrollbar(LastDizShowScrollbar);
	}
	else
	{
		//не будем менять внутренности если мы посреди операции со вьювером.
		bOK = DizView->InRecursion==0;
	}

	if (bOK)
	{
		if (!DizView->OpenFile(DizFile,FALSE))
		{
			delete DizView;
			DizView = NULL;
			return(FALSE);
		}

		strcpy(DizFileName,DizFile);
	}

	DizView->Show();
	char Title[NM];
	sprintf(Title," %s ",PointToName(DizFileName));
	TruncStr(Title,X2-X1-3);
	GotoXY(X1+(X2-X1-(int)strlen(Title))/2,Y1+14);
	SetColor(COL_PANELTEXT);
	PrintText(Title);
	return(TRUE);
}

void InfoList::SetFocus()
{
	Panel::SetFocus();
	SetMacroMode(FALSE);
}

void InfoList::KillFocus()
{
	Panel::KillFocus();
	SetMacroMode(TRUE);
}

void InfoList::SetMacroMode(int Restore)
{
	if (CtrlObject == NULL)
		return;

	if (PrevMacroMode == -1)
		PrevMacroMode = CtrlObject->Macro.GetMode();

	CtrlObject->Macro.SetMode(Restore ? PrevMacroMode:MACRO_INFOPANEL);
}

/* $ 02.01.2002 IS дадим информацию об имени просматриваемого diz-файла
   Если возвращается пустая строка, то значит, что в текущий момент
   diz-файл не просматривается.
*/
int InfoList::GetCurName(char *Name,char *ShortName)
{
	if (Name && ShortName)
	{
		strcpy(Name, DizFileName);
		ConvertNameToShort(DizFileName,ShortName,NM-1); // BUG!
		return (TRUE);
	}

	return (FALSE);
}

BOOL InfoList::UpdateKeyBar()
{
	KeyBar *KB = CtrlObject->MainKeyBar;
	KB->SetAllGroup(KBL_MAIN, MInfoF1, 12);
	KB->SetAllGroup(KBL_SHIFT, MInfoShiftF1, 12);
	KB->SetAllGroup(KBL_ALT, MInfoAltF1, 12);
	KB->SetAllGroup(KBL_CTRL, MInfoCtrlF1, 12);
	KB->SetAllGroup(KBL_CTRLSHIFT, MInfoCtrlShiftF1, 12);
	KB->SetAllGroup(KBL_CTRLALT, MInfoCtrlAltF1, 12);
	KB->SetAllGroup(KBL_ALTSHIFT, MInfoAltShiftF1, 12);
	KB->SetAllGroup(KBL_CTRLALTSHIFT, MInfoCtrlAltShiftF1, 12);
	DynamicUpdateKeyBar();
	return TRUE;
}

void InfoList::DynamicUpdateKeyBar()
{
	KeyBar *KB = CtrlObject->MainKeyBar;

	if (DizView)
	{
		KB->Change(MSG(MInfoF3), 3-1);

		if (DizView->GetAnsiMode())
			KB->Change(MSG(MViewF8DOS), 7);
		else
			KB->Change(MSG(MInfoF8), 7);

		if (!DizView->GetWrapMode())
		{
			if (DizView->GetWrapType())
				KB->Change(MSG(MViewShiftF2), 2-1);
			else
				KB->Change(MSG(MViewF2), 2-1);
		}
		else
			KB->Change(MSG(MViewF2Unwrap), 2-1);

		if (DizView->GetWrapType())
			KB->Change(KBL_SHIFT, MSG(MViewF2), 2-1);
		else
			KB->Change(KBL_SHIFT, MSG(MViewShiftF2), 2-1);
	}
	else
	{
		KB->Change(MSG(MF2), 2-1);
		KB->Change(KBL_SHIFT, "", 2-1);
		KB->Change("", 3-1);
		KB->Change("", 8-1);
		KB->Change(KBL_SHIFT, "", 8-1);
		KB->Change(KBL_ALT, MSG(MAltF8), 8-1);  // стандартный для панели - "хистори"
	}

	KB->ReadRegGroup("Info",Opt.Language);
	KB->SetAllRegGroup();
}
