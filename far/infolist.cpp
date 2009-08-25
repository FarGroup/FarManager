/*
infolist.cpp

Информационная панель
*/
/*
Copyright (c) 1996 Eugene Roshal
Copyright (c) 2000 Far Group
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "headers.hpp"
#pragma hdrstop

#include "imports.hpp"
#include "infolist.hpp"
#include "macroopcode.hpp"
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
#include "cddrv.hpp"
#include "syslog.hpp"
#include "interf.hpp"
#include "drivemix.hpp"
#include "dirmix.hpp"
#include "pathmix.hpp"
#include "strmix.hpp"
#include "mix.hpp"

static int LastDizWrapMode = -1;
static int LastDizWrapType = -1;
static int LastDizShowScrollbar = -1;

InfoList::InfoList()
{
  Type=INFO_PANEL;
  DizView=NULL;
  PrevMacroMode=-1;

  if (LastDizWrapMode < 0)
  {
    LastDizWrapMode = Opt.ViOpt.ViewerIsWrap;
    LastDizWrapType = Opt.ViOpt.ViewerWrap;
    LastDizShowScrollbar = Opt.ViOpt.ShowScrollbar;
  }
}

InfoList::~InfoList()
{
  CloseFile();
  SetMacroMode(TRUE);
}

// перерисовка, только если мы текущий фрейм
void InfoList::Update (int Mode)
{
  if (!EnableUpdate)
    return;
  if (CtrlObject->Cp() == FrameManager->GetCurrentFrame())
    Redraw();
}

string &InfoList::GetTitle(string &strTitle,int SubLen,int TruncSize)
{
  strTitle.Format (L" %s ", MSG(MInfoTitle));
  TruncStr(strTitle,X2-X1-3);
  return strTitle;
}

void InfoList::DisplayObject()
{
	string strTitle;
	string strOutStr;

	Panel *AnotherPanel;
	string strDriveRoot;

	string strVolumeName, strFileSystemName;

	DWORD MaxNameLength,FileSystemFlags,VolumeNumber;

	CloseFile();
	Box(X1,Y1,X2,Y2,COL_PANELBOX,DOUBLE_BOX);
	SetScreen(X1+1,Y1+1,X2-1,Y2-1,L' ',COL_PANELTEXT);
	SetColor(Focus ? COL_PANELSELECTEDTITLE:COL_PANELTITLE);

	GetTitle(strTitle);
	if ( !strTitle.IsEmpty() )
	{
		GotoXY(X1+(X2-X1+1-(int)strTitle.GetLength())/2,Y1);
		Text(strTitle);
	}

	DrawSeparator(Y1+3);
	DrawSeparator(Y1+8);

	SetColor(COL_PANELTEXT);

	{
		string strComputerName, strUserName;

		DWORD dwSize = MAX_COMPUTERNAME_LENGTH+1;

		wchar_t *ComputerName = strComputerName.GetBuffer (dwSize);
		GetComputerName(ComputerName, &dwSize);
		strComputerName.ReleaseBuffer ();

		dwSize = 256; //UNLEN

		wchar_t *UserName = strUserName.GetBuffer (dwSize);

		if(!GetUserNameEx(NameUserPrincipal,UserName, &dwSize))
		{
			GetUserName(UserName, &dwSize);
		}
		strUserName.ReleaseBuffer ();

		GotoXY(X1+2,Y1+1);
		PrintText(MInfoCompName);
		PrintInfo(strComputerName);
		GotoXY(X1+2,Y1+2);
		PrintText(MInfoUserName);
		PrintInfo(strUserName);
	}

	AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);
	AnotherPanel->GetCurDir(strCurDir);

	if ( strCurDir.IsEmpty() )
		apiGetCurrentDirectory(strCurDir);

	/*
		Корректно отображать инфу при заходе в Juction каталог
		Рут-диск может быть другим
	*/
	if((apiGetFileAttributes(strCurDir)&FILE_ATTRIBUTE_REPARSE_POINT) == FILE_ATTRIBUTE_REPARSE_POINT)
	{
		string strJuncName;
		if(GetReparsePointInfo(strCurDir, strJuncName))
		{
			NormalizeSymlinkName(strJuncName);
			GetPathRoot(strJuncName,strDriveRoot); //"\??\D:\Junc\Src\"
		}

	}
	else
		GetPathRoot(strCurDir, strDriveRoot);

	if ( apiGetVolumeInformation (strDriveRoot,&strVolumeName,
			&VolumeNumber,&MaxNameLength,&FileSystemFlags,
			&strFileSystemName))
	{
		int IdxMsgID=-1;
		int DriveType=FAR_GetDriveType(strDriveRoot,NULL,TRUE);
		switch(DriveType)
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
				if(IsDriveTypeCDROM(DriveType))
					IdxMsgID=DriveType-DRIVE_CD_RW+MInfoCD_RW;
				break;
		}

		LPCWSTR DiskType=(IdxMsgID!=-1)?MSG(IdxMsgID):L"";

		wchar_t LocalName[]={strDriveRoot.At(0),L':',L'\0'};
		string strRemoteName;
		if(GetSubstName(DriveType,LocalName,strRemoteName))
		{
			DiskType = MSG(MInfoSUBST);
			DriveType=DRIVE_SUBSTITUTE;
		}

		strTitle=string(L" ")+DiskType+L" "+MSG(MInfoDisk)+L" "+((!strDriveRoot.IsEmpty() && strDriveRoot.At(1)==L':')?LocalName:strDriveRoot)+L" ("+strFileSystemName+L") ";

		if (DriveType==DRIVE_REMOTE)
		{
			apiWNetGetConnection(LocalName, strRemoteName);
		}
		else if(DriveType == DRIVE_SUBSTITUTE)
		{
			strTitle += strRemoteName;
			strTitle += L" ";
		}

		TruncStr(strTitle,X2-X1-3);
		GotoXY(X1+(X2-X1+1-(int)strTitle.GetLength())/2,Y1+3);
		PrintText(strTitle);

		unsigned __int64 TotalSize,TotalFree,UserFree;
		if (apiGetDiskSize(strCurDir,&TotalSize,&TotalFree,&UserFree))
		{
			GotoXY(X1+2,Y1+4);
			PrintText(MInfoDiskTotal);
			InsertCommas(TotalSize,strOutStr);
			PrintInfo(strOutStr);
			GotoXY(X1+2,Y1+5);
			PrintText(MInfoDiskFree);
			InsertCommas(UserFree,strOutStr);
			PrintInfo(strOutStr);
		}

		GotoXY(X1+2,Y1+6);
		PrintText(MInfoDiskLabel);
		PrintInfo(strVolumeName);
		GotoXY(X1+2,Y1+7);
		PrintText(MInfoDiskNumber);
		strOutStr.Format (L"%04X-%04X",VolumeNumber>>16,VolumeNumber & 0xffff);
		PrintInfo(strOutStr);
	}

	strTitle = MSG(MInfoMemory);
	TruncStr(strTitle,X2-X1-3);
	GotoXY(X1+(X2-X1+1-(int)strTitle.GetLength())/2,Y1+8);
	PrintText(strTitle);
	MEMORYSTATUSEX ms={sizeof(ms)};

	if(GlobalMemoryStatusEx(&ms))
	{
		if (ms.dwMemoryLoad==0)
			ms.dwMemoryLoad=100-ToPercent64(ms.ullAvailPhys+ms.ullAvailPageFile,ms.ullTotalPhys+ms.ullTotalPageFile);
		GotoXY(X1+2,Y1+9);
		PrintText(MInfoMemoryLoad);
		strOutStr.Format (L"%d%%",ms.dwMemoryLoad);
		PrintInfo(strOutStr);
		GotoXY(X1+2,Y1+10);
		PrintText(MInfoMemoryTotal);
		InsertCommas((__int64)ms.ullTotalPhys,strOutStr);
		PrintInfo(strOutStr);
		GotoXY(X1+2,Y1+11);
		PrintText(MInfoMemoryFree);
		InsertCommas((__int64)ms.ullAvailPhys,strOutStr);
		PrintInfo(strOutStr);
		GotoXY(X1+2,Y1+12);
		PrintText(MInfoVirtualTotal);
		InsertCommas((__int64)ms.ullTotalPageFile,strOutStr);
		PrintInfo(strOutStr);
		GotoXY(X1+2,Y1+13);
		PrintText(MInfoVirtualFree);
		InsertCommas((__int64)ms.ullAvailPageFile,strOutStr);
		PrintInfo(strOutStr);
	}

	ShowDirDescription();
	ShowPluginDescription();
}


__int64 InfoList::VMProcess(int OpCode,void *vParam,__int64 iParam)
{
  if (DizView!=NULL)
    return DizView->VMProcess(OpCode,vParam,iParam);

  switch(OpCode)
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

  if(ProcessShortcutFolder(Key,FALSE))
    return(TRUE);

  switch(Key)
  {
    case KEY_F1:
      {
        Help Hlp (L"InfoPanel");
      }
      return TRUE;

    case KEY_F3:
    case KEY_NUMPAD5:  case KEY_SHIFTNUMPAD5:
      if ( !strDizFileName.IsEmpty() )
      {
        CtrlObject->Cp()->GetAnotherPanel(this)->GetCurDir(strCurDir);
        FarChDir(strCurDir);

        new FileViewer(strDizFileName,TRUE);//OT
      }
      CtrlObject->Cp()->Redraw();
      return(TRUE);
    case KEY_F4:
      /* $ 30.04.2001 DJ
         не показываем редактор, если ничего не задано в именах файлов;
         не редактируем имена описаний со звездочками;
         убираем лишнюю перерисовку панелей
      */
      {
        Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);
        AnotherPanel->GetCurDir(strCurDir);
        FarChDir(strCurDir);
        if ( !strDizFileName.IsEmpty() )
        {
          new FileEditor(strDizFileName,CP_AUTODETECT,FFILEEDIT_ENABLEF6);
        }
        else if ( !Opt.strFolderInfoFiles.IsEmpty() )
        {
          string strArgName;
          const wchar_t *p = Opt.strFolderInfoFiles;
          while ((p = GetCommaWord(p,strArgName)) != NULL)
          {
            if (!wcspbrk (strArgName, L"*?"))
            {
              new FileEditor(strArgName,CP_AUTODETECT,FFILEEDIT_CANNEWFILE|FFILEEDIT_ENABLEF6);
              break;
            }
          }
        }
        AnotherPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
//        AnotherPanel->Redraw();
        Update(0);
      }
      CtrlObject->Cp()->Redraw();
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

  return(FALSE);
}


int InfoList::ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent)
{
  int RetCode;
  if (Panel::PanelProcessMouse(MouseEvent,RetCode))
    return(RetCode);

  if (MouseEvent->dwMousePosition.Y>=14 && DizView!=NULL)
  {
    _tran(SysLog(L"InfoList::ProcessMouse() DizView = %p",DizView));
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


void InfoList::PrintText(const wchar_t *Str)
{
    if (WhereY()>Y2-1)
        return;
    mprintf(L"%.*s",X2-WhereX(),Str);
}


void InfoList::PrintText(int MsgID)
{
    PrintText(MSG(MsgID));
}


void InfoList::PrintInfo(const wchar_t *str)
{
    if (WhereY()>Y2-1)
        return;
    int SaveColor=GetColor(),MaxLength=X2-WhereX()-2;
    if (MaxLength<0)
        MaxLength=0;

    string strStr = str;
    TruncStr(strStr,MaxLength);
    int Length=(int)strStr.GetLength();
    int NewX=X2-Length-1;
    if (NewX>X1 && NewX>WhereX())
    {
        GotoXY(NewX,WhereY());
        SetColor(COL_PANELINFOTEXT);
        mprintf(L"%s ",(const wchar_t*)strStr);
        SetColor(SaveColor);
    }
}


void InfoList::PrintInfo(int MsgID)
{
    PrintInfo(MSG(MsgID));
}


void InfoList::ShowDirDescription()
{
  Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);
  DrawSeparator(Y1+14);
  if (AnotherPanel->GetMode()==FILE_PANEL)
  {
		string strDizDir;
    AnotherPanel->GetCurDir(strDizDir);
		if(!strDizDir.IsEmpty())
			AddEndSlash(strDizDir);

    string strArgName;

    const wchar_t *NamePtr = Opt.strFolderInfoFiles;
    while ((NamePtr=GetCommaWord(NamePtr,strArgName))!=NULL)
    {
      string strFullDizName;
      strFullDizName = strDizDir;
      strFullDizName += strArgName;

      FAR_FIND_DATA_EX FindData;

      if ( !apiGetFindDataEx (strFullDizName,&FindData) )
        continue;

      CutToSlash(strFullDizName, false);
      strFullDizName += FindData.strFileName;

      if (OpenDizFile(strFullDizName))
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
  static wchar_t VertcalLine[2]={BoxSymbols[BS_V2],0};
  AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);
  if (AnotherPanel->GetMode()!=PLUGIN_PANEL)
    return;
  CloseFile();
	OpenPluginInfo Info;
  AnotherPanel->GetOpenPluginInfo(&Info);
  for (int I=0;I<Info.InfoLinesNumber;I++)
  {
    int Y=Y2-Info.InfoLinesNumber+I;
    if (Y<=Y1)
      continue;
		const InfoPanelLine *InfoLine=&Info.InfoLines[I];
    GotoXY(X1,Y);
    SetColor(COL_PANELBOX);
    Text(VertcalLine);
    SetColor(COL_PANELTEXT);
    mprintf(L"%*s",X2-X1-1,L"");
    SetColor(COL_PANELBOX);
    Text(VertcalLine);
    GotoXY(X1+2,Y);
    if (InfoLine->Separator)
    {
      string strTitle;
      if (InfoLine->Text!=NULL && *InfoLine->Text)
        strTitle.Format (L" %s ",InfoLine->Text);
      else
        strTitle=L"";
      DrawSeparator(Y);
      TruncStr(strTitle,X2-X1-3);
      GotoXY(X1+(X2-X1-(int)strTitle.GetLength())/2,Y);
      PrintText(strTitle);
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
    if(DizView->InRecursion)
      return;
    LastDizWrapMode=DizView->GetWrapMode();
    LastDizWrapType=DizView->GetWrapType();
    LastDizShowScrollbar=DizView->GetShowScrollbar();
    DizView->SetWrapMode(OldWrapMode);
    DizView->SetWrapType(OldWrapType);
    delete DizView;
    DizView=NULL;
  }
  strDizFileName=L"";
}

int InfoList::OpenDizFile(const wchar_t *DizFile)
{
  bool bOK=true;

  _tran(SysLog(L"InfoList::OpenDizFile([%s]",DizFile));
  if (DizView == NULL)
  {
    DizView=new DizViewer;
    if (!DizView)
      return FALSE;
    _tran(SysLog(L"InfoList::OpenDizFile() create new Viewer = %p",DizView));
    DizView->SetRestoreScreenMode(FALSE);
    DizView->SetPosition(X1+1,Y1+15,X2-1,Y2-1);
    DizView->SetStatusMode(0);
    DizView->EnableHideCursor(0);
    OldWrapMode = DizView->GetWrapMode();
    OldWrapType = DizView->GetWrapType();
    DizView->SetWrapMode(LastDizWrapMode);
    DizView->SetWrapType(LastDizWrapType);
    DizView->SetShowScrollbar (LastDizShowScrollbar);
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
    strDizFileName = DizFile;
  }

  DizView->Show();

  string strTitle;
  strTitle.Format (L" %s ", (const wchar_t*)PointToName(strDizFileName));
  TruncStr(strTitle,X2-X1-3);
  GotoXY(X1+(X2-X1-(int)strTitle.GetLength())/2,Y1+14);
  SetColor(COL_PANELTEXT);
  PrintText(strTitle);
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


int InfoList::GetCurName(string &strName, string &strShortName)
{
  strName = strDizFileName;

  ConvertNameToShort(strName, strShortName);

  return (TRUE);
}

BOOL InfoList::UpdateKeyBar()
{
  KeyBar *KB = CtrlObject->MainKeyBar;
  KB->SetAllGroup (KBL_MAIN, MInfoF1, 12);
  KB->SetAllGroup (KBL_SHIFT, MInfoShiftF1, 12);
  KB->SetAllGroup (KBL_ALT, MInfoAltF1, 12);
  KB->SetAllGroup (KBL_CTRL, MInfoCtrlF1, 12);
  KB->SetAllGroup (KBL_CTRLSHIFT, MInfoCtrlShiftF1, 12);
  KB->SetAllGroup (KBL_CTRLALT, MInfoCtrlAltF1, 12);
  KB->SetAllGroup (KBL_ALTSHIFT, MInfoAltShiftF1, 12);
  KB->SetAllGroup (KBL_CTRLALTSHIFT, MInfoCtrlAltShiftF1, 12);
  DynamicUpdateKeyBar();

  return TRUE;
}

void InfoList::DynamicUpdateKeyBar()
{
  KeyBar *KB = CtrlObject->MainKeyBar;
  if (DizView)
  {
    KB->Change (MSG(MInfoF3), 3-1);

    if (DizView->GetCodePage() != GetOEMCP())
      KB->Change (MSG(MViewF8DOS), 7);
    else
      KB->Change (MSG(MInfoF8), 7);

    if (!DizView->GetWrapMode())
    {
      if (DizView->GetWrapType())
        KB->Change (MSG(MViewShiftF2), 2-1);
      else
        KB->Change (MSG(MViewF2), 2-1);
    }
    else
      KB->Change (MSG(MViewF2Unwrap), 2-1);

    if (DizView->GetWrapType())
      KB->Change (KBL_SHIFT, MSG(MViewF2), 2-1);
    else
      KB->Change (KBL_SHIFT, MSG(MViewShiftF2), 2-1);
  }
  else
  {
    KB->Change (MSG(MF2), 2-1);
    KB->Change (KBL_SHIFT, L"", 2-1);
    KB->Change (L"", 3-1);
    KB->Change (L"", 8-1);
    KB->Change (KBL_SHIFT, L"", 8-1);
    KB->Change (KBL_ALT, MSG(MAltF8), 8-1); // стандартный для панели - "хистори"
  }

  KB->ReadRegGroup(L"Info",Opt.strLanguage);
  KB->SetAllRegGroup();
}
