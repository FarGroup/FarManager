/*
qview.cpp

Quick view panel

*/

#include "headers.hpp"
#pragma hdrstop

#include "qview.hpp"
#include "macroopcode.hpp"
#include "fn.hpp"
#include "flink.hpp"
#include "lang.hpp"
#include "colors.hpp"
#include "keys.hpp"
#include "global.hpp"
#include "filepanels.hpp"
#include "help.hpp"
#include "viewer.hpp"
#include "cmdline.hpp"
#include "ctrlobj.hpp"

/* $ 12.07.2000 SVS
    ! ƒл€ возможности 3-х позиционного Wrap`а статическа€ переменна€
      LastWrapMode имеет не булевое значение, а обычный int
*/
/* $ 20.02.2001 VVM
    ! ¬рап хранитс€ в 2х переменных. */
static int LastWrapMode = -1;
static int LastWrapType = -1;
/* VVM $ */
/* SVS $ */

QuickView::QuickView()
{
	Type=QVIEW_PANEL;
	QView=NULL;
	*CurFileName=0;
	*CurFileType=0;
	*TempName=0;
	Directory=0;
	PrevMacroMode = -1;

	/* $ 20.02.2001 VVM
	  + ѕроинициализируем режим врап-а */
	if (LastWrapMode < 0)
	{
		LastWrapMode = Opt.ViOpt.ViewerIsWrap;
		LastWrapType = Opt.ViOpt.ViewerWrap;
	}

	/* VVM $ */
}


QuickView::~QuickView()
{
	CloseFile();
	SetMacroMode(TRUE);
}


const char *QuickView::GetTitle(char *lTitle,int LenTitle,int TruncSize)
{
	char Title[512];
	sprintf(Title," %s ",MSG(MQuickViewTitle));
	TruncStr(Title,X2-X1-3);
	xstrncpy(lTitle,Title,LenTitle);
	return lTitle;
}

void QuickView::DisplayObject()
{
	if (Flags.Check(FSCROBJ_ISREDRAWING))
		return;

	Flags.Set(FSCROBJ_ISREDRAWING);
	char Title[NM];

	if (QView==NULL && !ProcessingPluginCommand)
		CtrlObject->Cp()->GetAnotherPanel(this)->UpdateViewPanel();

	if (QView!=NULL)
		QView->SetPosition(X1+1,Y1+1,X2-1,Y2-3);

	Box(X1,Y1,X2,Y2,COL_PANELBOX,DOUBLE_BOX);
	SetScreen(X1+1,Y1+1,X2-1,Y2-1,' ',COL_PANELTEXT);
	SetColor(Focus ? COL_PANELSELECTEDTITLE:COL_PANELTITLE);
	GetTitle(Title,sizeof(Title)-1);

	if (*Title)
	{
		GotoXY(X1+(X2-X1+1-(int)strlen(Title))/2,Y1);
		Text(Title);
	}

	DrawSeparator(Y2-2);
	SetColor(COL_PANELTEXT);
	GotoXY(X1+1,Y2-1);
	mprintf("%-*.*s",X2-X1-1,X2-X1-1,PointToName(CurFileName));

	if (*CurFileType)
	{
		char TypeText[sizeof(CurFileType)];
		sprintf(TypeText," %s ",CurFileType);
		TruncStr(TypeText,X2-X1-1);
		SetColor(COL_PANELSELECTEDINFO);
		GotoXY(X1+(X2-X1+1-(int)strlen(TypeText))/2,Y2-2);
		Text(TypeText);
	}

	if (Directory)
	{
		char Msg[NM*2];
		sprintf(Msg,MSG(MQuickViewFolder),CurFileName);
		TruncStr(Msg,X2-X1-4);
		SetColor(COL_PANELTEXT);
		GotoXY(X1+2,Y1+2);
		PrintText(Msg);

		/* $ 01.02.2001 SVS
		   ¬ панели "Quick view" добавим инфу про Junction
		*/
		if ((GetFileAttributes(CurFileName)&FILE_ATTRIBUTE_REPARSE_POINT) == FILE_ATTRIBUTE_REPARSE_POINT)
		{
			char JuncName[NM*2];
			DWORD ReparseTag=0;

			if (GetReparsePointInfo(CurFileName,JuncName,sizeof(JuncName),&ReparseTag)) //"\??\D:\Junc\Src\"
			{
				int ID_Msg=MQuickViewJunction;
				int offset=0;

				if (!strncmp(JuncName,"\\??\\",4))
					offset = 4;

				if (ReparseTag==IO_REPARSE_TAG_MOUNT_POINT)
				{
					if (IsLocalVolumePath(JuncName) && !JuncName[49])
					{
						char JuncRoot[NM*2];
						JuncRoot[0]=JuncRoot[1]=0;
						GetPathRootOne(JuncName+offset,JuncRoot);

						if (JuncRoot[1] == ':')
							strcpy(JuncName+offset,JuncRoot);

						ID_Msg=MQuickViewVolMount;
					}
				}
				else if (ReparseTag==IO_REPARSE_TAG_SYMLINK)
				{
					ID_Msg=MQuickViewSymlink;
				}

				sprintf(Msg,MSG(ID_Msg),TruncPathStr(JuncName+offset,X2-X1-1-(int)strlen(MSG(ID_Msg))));
				//TruncStr(Msg,X2-X1-1);
				SetColor(COL_PANELTEXT);
				GotoXY(X1+2,Y1+3);
				PrintText(Msg);
			}
		}

		/* SVS $ */

		if (Directory==1 || Directory==4)
		{
			char SlackMsg[100];
			GotoXY(X1+2,Y1+4);
			PrintText(MSG(MQuickViewContains));
			GotoXY(X1+2,Y1+6);
			PrintText(MSG(MQuickViewFolders));
			SetColor(COL_PANELINFOTEXT);
			sprintf(Msg,"%d",DirCount);
			PrintText(Msg);
			SetColor(COL_PANELTEXT);
			GotoXY(X1+2,Y1+7);
			PrintText(MSG(MQuickViewFiles));
			SetColor(COL_PANELINFOTEXT);
			sprintf(Msg,"%d",FileCount);
			PrintText(Msg);
			SetColor(COL_PANELTEXT);
			GotoXY(X1+2,Y1+8);
			PrintText(MSG(MQuickViewBytes));
			SetColor(COL_PANELINFOTEXT);
			InsertCommas(FileSize,Msg,sizeof(Msg));
			PrintText(Msg);
			SetColor(COL_PANELTEXT);
			GotoXY(X1+2,Y1+9);
			PrintText(MSG(MQuickViewCompressed));
			SetColor(COL_PANELINFOTEXT);
			InsertCommas(CompressedFileSize,Msg,sizeof(Msg));
			PrintText(Msg);
			SetColor(COL_PANELTEXT);
			GotoXY(X1+2,Y1+10);
			PrintText(MSG(MQuickViewRatio));
			SetColor(COL_PANELINFOTEXT);
			sprintf(SlackMsg,"%d%%",ToPercent64(CompressedFileSize,FileSize));
			PrintText(SlackMsg);

			if (Directory!=4 && RealFileSize>=CompressedFileSize)
			{
				SetColor(COL_PANELTEXT);
				GotoXY(X1+2,Y1+12);
				PrintText(MSG(MQuickViewCluster));
				SetColor(COL_PANELINFOTEXT);
				InsertCommas(ClusterSize,Msg,sizeof(Msg));
				PrintText(Msg);
				SetColor(COL_PANELTEXT);
				GotoXY(X1+2,Y1+13);
				PrintText(MSG(MQuickViewRealSize));
				SetColor(COL_PANELINFOTEXT);
				InsertCommas(RealFileSize,Msg,sizeof(Msg));
				PrintText(Msg);
				SetColor(COL_PANELTEXT);
				GotoXY(X1+2,Y1+14);
				PrintText(MSG(MQuickViewSlack));
				SetColor(COL_PANELINFOTEXT);
				InsertCommas(RealFileSize-CompressedFileSize,Msg,sizeof(Msg));
				__int64 Size1=RealFileSize-CompressedFileSize;
				__int64 Size2=RealFileSize;
				sprintf(SlackMsg,"%s (%d%%)",Msg,ToPercent64(Size1,Size2));
				PrintText(SlackMsg);
			}
		}
	}
	else if (QView!=NULL)
		QView->Show();

	Flags.Clear(FSCROBJ_ISREDRAWING);
}


__int64 QuickView::VMProcess(int OpCode,void *vParam,__int64 iParam)
{
	if (!Directory && QView!=NULL)
		return QView->VMProcess(OpCode,vParam,iParam);

	switch (OpCode)
	{
		case MCODE_C_EMPTY:
			return _i64(1);
	}

	return _i64(0);
}

int QuickView::ProcessKey(int Key)
{
	if (!IsVisible())
		return(FALSE);

	if (ProcessShortcutFolder(Key,FALSE))
		return(TRUE);

	/* $ 30.04.2001 DJ
	   показываем правильный help topic
	*/
	if (Key == KEY_F1)
	{
		Help Hlp("QViewPanel");
		return TRUE;
	}

	/* DJ $ */
	if (Key==KEY_F3 || Key==KEY_NUMPAD5 || Key == KEY_SHIFTNUMPAD5)
	{
		Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);

		if (AnotherPanel->GetType()==FILE_PANEL)
			AnotherPanel->ProcessKey(KEY_F3);

		return(TRUE);
	}

	/* $ 04.08.2000 tran
	   Gray+, Gray- передвигают курсор на другой панели*/
	if (Key==KEY_ADD || Key==KEY_SUBTRACT)
	{
		Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);

		if (AnotherPanel->GetType()==FILE_PANEL)
			AnotherPanel->ProcessKey(Key==KEY_ADD?KEY_DOWN:KEY_UP);

		return(TRUE);
	}

	/* tran 04.08.2000 $ */

	/* $ 30.04.2001 DJ
	   обновл€ем кейбар
	*/
	if (QView!=NULL && !Directory && Key>=256)
	{
		int ret = QView->ProcessKey(Key);

		if (Key == KEY_F4 || Key == KEY_F8 || Key == KEY_F2 || Key == KEY_SHIFTF2)
		{
			DynamicUpdateKeyBar();
			CtrlObject->MainKeyBar->Redraw();
		}

		if (Key == KEY_F7 || Key == KEY_SHIFTF7)
		{
			/*
			__int64 Pos;
			int Length;
			DWORD Flags;
			QView->GetSelectedParam(Pos,Length,Flags);
			*/
			Redraw();
			CtrlObject->Cp()->GetAnotherPanel(this)->Redraw();
			//QView->SelectText(Pos,Length,Flags|1);
		}

		return ret;
	}

	return(FALSE);
}


int QuickView::ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent)
{
	int RetCode;

	if (!IsVisible())
		return(FALSE);

	if (Panel::PanelProcessMouse(MouseEvent,RetCode))
		return(RetCode);

	SetFocus();

	if (QView!=NULL && !Directory)
		return(QView->ProcessMouse(MouseEvent));

	return(FALSE);
}

#if defined(__BORLANDC__)
#pragma warn -par
#endif
void QuickView::Update(int Mode)
{
	if (!EnableUpdate)
		return;

	if (*CurFileName==0)
		CtrlObject->Cp()->GetAnotherPanel(this)->UpdateViewPanel();

	Redraw();
}
#if defined(__BORLANDC__)
#pragma warn +par
#endif


void QuickView::ShowFile(char *FileName,int TempFile,HANDLE hDirPlugin)
{
	char *ExtPtr;
	int FileAttr;
	CloseFile();
	QView=NULL;
	*CurFileName=0;

	if (!IsVisible())
		return;

	if (FileName==NULL)
	{
		ProcessingPluginCommand++;
		Show();
		ProcessingPluginCommand--;
		return;
	}

	strcpy(CurFileName,FileName);

	if ((ExtPtr=strrchr(CurFileName,'.'))!=NULL)
	{
		char Value[1024];

		if (GetShellType(ExtPtr,Value,sizeof(Value)))
		{
			LONG ValueSize=sizeof(CurFileType);

			if (RegQueryValue(HKEY_CLASSES_ROOT,Value,(LPTSTR)CurFileType,&ValueSize)!=ERROR_SUCCESS)
				*CurFileType=0;

			FAR_CharToOem(CurFileType,CurFileType);
		}
	}

	if (hDirPlugin || (FileAttr=GetFileAttributes(CurFileName))!=-1 && (FileAttr & FA_DIREC))
	{
		/* $ 28.06.2000 IS
		 Ќе показывать тип файла дл€ каталогов в "Ѕыстром просмотре" /
		*/
		*CurFileType=0;

		/* IS $ */
		if (hDirPlugin)
		{
			int ExitCode=GetPluginDirInfo(hDirPlugin,CurFileName,DirCount,
			                              FileCount,FileSize,CompressedFileSize);

			if (ExitCode)
				Directory=4;
			else
				Directory=3;
		}
		else
		{
			int ExitCode=GetDirInfo(MSG(MQuickViewTitle),CurFileName,DirCount,
			                        FileCount,FileSize,CompressedFileSize,RealFileSize,
			                        ClusterSize,500,
			                        NULL,
			                        GETDIRINFO_ENHBREAK|GETDIRINFO_USEDALTFOLDERNAME|GETDIRINFO_SCANSYMLINKDEF|GETDIRINFO_DONTREDRAWFRAME);

			if (ExitCode==1)
				Directory=1;
			else if (ExitCode==-1)
				Directory=2;
			else
				Directory=3;
		}
	}
	else
	{
		if (*CurFileName)
		{
			QView=new Viewer(true);
			QView->SetRestoreScreenMode(FALSE);
			QView->SetPosition(X1+1,Y1+1,X2-1,Y2-3);
			QView->SetStatusMode(0);
			QView->EnableHideCursor(0);
			/* $ 20.02.2001 VVM
			    + «апомнить старое состо€ние врапа и потом восстановить. */
			OldWrapMode = QView->GetWrapMode();
			OldWrapType = QView->GetWrapType();
			QView->SetWrapMode(LastWrapMode);
			QView->SetWrapType(LastWrapType);
			/* VVM $ */
			QView->OpenFile(CurFileName,FALSE);
		}
	}

	if (TempFile)
	{
//    ConvertNameToFull(CurFileName,TempName, sizeof(TempName));
		if (ConvertNameToFull(CurFileName,TempName, sizeof(TempName)) >= sizeof(TempName))
		{
			return;
		}
	}

	Redraw();

	/* $ 30.04.2001 DJ
	   обновл€ем кейбар
	*/
	if (CtrlObject->Cp()->ActivePanel == this)
	{
		DynamicUpdateKeyBar();
		CtrlObject->MainKeyBar->Redraw();
	}

	/* DJ $ */
}


void QuickView::CloseFile()
{
	if (QView!=NULL)
	{
		/* $ 20.02.2001 VVM
		    ! ¬осстановить старое значение врапа */
		LastWrapMode=QView->GetWrapMode();
		LastWrapType=QView->GetWrapType();
		QView->SetWrapMode(OldWrapMode);
		QView->SetWrapType(OldWrapType);
		/* VVM $ */
		delete QView;
		QView=NULL;
	}

	QViewDelTempName();
	*CurFileType=0;
	Directory=0;
}


void QuickView::QViewDelTempName()
{
	if (*TempName)
	{
		if (QView!=NULL)
		{
			/* $ 20.02.2001 VVM
			    ! ¬осстановить старое значение врапа */
			LastWrapMode=QView->GetWrapMode();
			LastWrapType=QView->GetWrapType();
			QView->SetWrapMode(OldWrapMode);
			QView->SetWrapType(OldWrapType);
			/* VVM $ */
			delete QView;
			QView=NULL;
		}

		chmod(TempName,S_IREAD|S_IWRITE);
		remove(TempName);
		*PointToName(TempName)=0;
		FAR_RemoveDirectory(TempName);
		*TempName=0;
	}
}


void QuickView::PrintText(char *Str)
{
	if (WhereY()>Y2-3 || WhereX()>X2-2)
		return;

	mprintf("%.*s",X2-2-WhereX()+1,Str);
}


int QuickView::UpdateIfChanged(int UpdateMode)
{
	if (IsVisible() && *CurFileName && Directory==2)
	{
		char ViewName[NM+30];
		strcpy(ViewName,CurFileName);
		ShowFile(ViewName,*TempName,NULL);
		return(TRUE);
	}

	return(FALSE);
}

/* $ 20.07.2000 tran
   два метода - установка заголовка*/
void QuickView::SetTitle()
{
	if (GetFocus())
	{
		char TitleDir[NM+30];

		if (*CurFileName)
			sprintf(TitleDir,"{%.*s - QuickView}",NM-1,CurFileName);
		else
		{
			char CmdText[512];
			CtrlObject->CmdLine->GetString(CmdText,sizeof(CmdText));
			sprintf(TitleDir,"{%.*s}",sizeof(TitleDir)-3,CmdText);
		}

		strcpy(LastFarTitle,TitleDir);
		SetFarTitle(TitleDir);
	}
}
// и его показ в случае получени€ фокуса
void QuickView::SetFocus()
{
	Panel::SetFocus();
	SetTitle();
	SetMacroMode(FALSE);
}
/* tran 20.07.2000 $ */

void QuickView::KillFocus()
{
	Panel::KillFocus();
	SetMacroMode(TRUE);
}

void QuickView::SetMacroMode(int Restore)
{
	if (CtrlObject == NULL)
		return;

	if (PrevMacroMode == -1)
		PrevMacroMode = CtrlObject->Macro.GetMode();

	CtrlObject->Macro.SetMode(Restore ? PrevMacroMode:MACRO_QVIEWPANEL);
}

int QuickView::GetCurName(char *Name,char *ShortName)
{
	if (Name && ShortName && *CurFileName)
	{
		strcpy(Name, CurFileName);
		strcpy(ShortName, Name);
		return (TRUE);
	}

	return (FALSE);
}


BOOL QuickView::UpdateKeyBar()
{
	KeyBar *KB = CtrlObject->MainKeyBar;
	KB->SetAllGroup(KBL_MAIN, MQViewF1, 12);
	KB->SetAllGroup(KBL_SHIFT, MQViewShiftF1, 12);
	KB->SetAllGroup(KBL_ALT, MQViewAltF1, 12);
	KB->SetAllGroup(KBL_CTRL, MQViewCtrlF1, 12);
	KB->SetAllGroup(KBL_CTRLSHIFT, MQViewCtrlShiftF1, 12);
	KB->SetAllGroup(KBL_CTRLALT, MQViewCtrlAltF1, 12);
	KB->SetAllGroup(KBL_ALTSHIFT, MQViewAltShiftF1, 12);
	KB->SetAllGroup(KBL_CTRLALTSHIFT, MQViewCtrlAltShiftF1, 12);
	DynamicUpdateKeyBar();
	return TRUE;
}

void QuickView::DynamicUpdateKeyBar()
{
	KeyBar *KB = CtrlObject->MainKeyBar;

	if (Directory || !QView)
	{
		KB->Change(MSG(MF2), 2-1);
		KB->Change("", 4-1);
		KB->Change("", 8-1);
		KB->Change(KBL_SHIFT, "", 2-1);
		KB->Change(KBL_SHIFT, "", 8-1);
		KB->Change(KBL_ALT, MSG(MAltF8), 8-1);  // стандартный дл€ панели - "хистори"
	}
	else
	{
		if (QView->GetHexMode())
			KB->Change(MSG(MViewF4Text), 4-1);
		else
			KB->Change(MSG(MQViewF4), 4-1);

		if (QView->GetAnsiMode())
			KB->Change(MSG(MViewF8DOS), 8-1);
		else
			KB->Change(MSG(MQViewF8), 8-1);

		if (!QView->GetWrapMode())
		{
			if (QView->GetWrapType())
				KB->Change(MSG(MViewShiftF2), 2-1);
			else
				KB->Change(MSG(MViewF2), 2-1);
		}
		else
			KB->Change(MSG(MViewF2Unwrap), 2-1);

		if (QView->GetWrapType())
			KB->Change(KBL_SHIFT, MSG(MViewF2), 2-1);
		else
			KB->Change(KBL_SHIFT, MSG(MViewShiftF2), 2-1);
	}

	KB->ReadRegGroup("QView",Opt.Language);
	KB->SetAllRegGroup();
}
