/*
infolist.cpp

Информационная панель

*/

#include "headers.hpp"
#pragma hdrstop

#include "infolist.hpp"
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
  DizPresent=FALSE;

  if (LastDizWrapMode < 0)
  {
    LastDizWrapMode = Opt.ViOpt.ViewerIsWrap;
    LastDizWrapType = Opt.ViOpt.ViewerWrap;
    LastDizShowScrollbar = Opt.ViOpt.ShowScrollbar;
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
void InfoList::Update (int Mode)
{
  if (!EnableUpdate)
    return;
  if (CtrlObject->Cp() == FrameManager->GetCurrentFrame())
    Redraw();
}
/* DJ $ */

void InfoList::GetTitle(string &strTitle,int SubLen,int TruncSize)
{
  strTitle.Format (L" %s ", UMSG(MInfoTitle));
  TruncStrW(strTitle,X2-X1-3);
}

void InfoList::DisplayObject()
{
  //char Title[NM],OutStr[200];
  string strTitle;
  string strOutStr;

  Panel *AnotherPanel;
  string strDriveRoot;

  string strVolumeName, strFileSystemName;

  DWORD MaxNameLength,FileSystemFlags,VolumeNumber;
  /* $ 30.04.2001 DJ
     CloseDizFile() -> CloseFile()
  */
  CloseFile();
  /* DJ $ */
  Box(X1,Y1,X2,Y2,COL_PANELBOX,DOUBLE_BOX);
  SetScreen(X1+1,Y1+1,X2-1,Y2-1,L' ',COL_PANELTEXT);
  SetColor(Focus ? COL_PANELSELECTEDTITLE:COL_PANELTITLE);

  GetTitle(strTitle);
  if ( !strTitle.IsEmpty() )
  {
    GotoXY(X1+(X2-X1+1-strTitle.GetLength())/2,Y1);
    TextW(strTitle);
  }

  DrawSeparator(Y1+3);
  DrawSeparator(Y1+8);

  SetColor(COL_PANELTEXT);
  {
    string strComputerName, strUserName;

    DWORD dwSize = MAX_COMPUTERNAME_LENGTH+1;

    wchar_t *ComputerName = strComputerName.GetBuffer (dwSize);
    GetComputerNameW (ComputerName, &dwSize);
    strComputerName.ReleaseBuffer ();

    dwSize = 256; //UNLEN

    wchar_t *UserName = strUserName.GetBuffer (dwSize);
    GetUserNameW (UserName, &dwSize);
    strUserName.ReleaseBuffer ();

    GotoXY(X1+2,Y1+1);
    PrintTextW(MInfoCompName);
    PrintInfoW(strComputerName);
    GotoXY(X1+2,Y1+2);
    PrintTextW(MInfoUserName);
    PrintInfoW(strUserName);
  }

  AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);
  AnotherPanel->GetCurDirW(strCurDir);

  if ( strCurDir.IsEmpty() )
    FarGetCurDirW(strCurDir);

  /* $ 01.02.2001 SVS
     В Win2K корректно отображать инфу при заходе в Juction каталог
     Здесь Рут-диск может быть другим
  */
  if((GetFileAttributesW(strCurDir)&FILE_ATTRIBUTE_REPARSE_POINT) == FILE_ATTRIBUTE_REPARSE_POINT)
  {
    string strJuncName;
    if(GetJunctionPointInfoW(strCurDir, strJuncName))
      GetPathRootW((const wchar_t*)strJuncName+4, strDriveRoot); //"\??\D:\Junc\Src\"
  }
  else
     GetPathRootW(strCurDir, strDriveRoot);
  /* SVS $ */

  if ( apiGetVolumeInformation (strDriveRoot,&strVolumeName,
                            &VolumeNumber,&MaxNameLength,&FileSystemFlags,
                            &strFileSystemName))
  {
    string strLocalName, strDiskType, strRemoteName, strDiskName;
    int ShowRealPath=FALSE;
    int DriveType=FAR_GetDriveTypeW(strDriveRoot,NULL,TRUE);
    strLocalName.Format (L"%c:", strDriveRoot.At(0));

    if ( !strDriveRoot.IsEmpty() && strDriveRoot.At(1)==L':')
      strDiskName.Format (L"%c:",LocalUpperW(strDriveRoot.At(0)));
    else
      strDiskName = strDriveRoot;

    int IdxMsgID=-1;
    strDiskType=L"";
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
        else
          strDiskType=L"";
        break;
    }
    if(IdxMsgID != -1)
      strDiskType= UMSG(IdxMsgID);

    {
      if(GetSubstNameW(DriveType,strLocalName,strRemoteName))
      {
        strDiskType = UMSG(MInfoSUBST);
        DriveType=DRIVE_SUBSTITUTE;
      }
    }

    strTitle.Format (L" %s %s %s (%s) ", (const wchar_t*)strDiskType, UMSG(MInfoDisk), (const wchar_t*)strDiskName, (const wchar_t*)strFileSystemName);

    if (DriveType==DRIVE_REMOTE)
    {
        DWORD dwRemoteNameSize = 0;

        WNetGetConnectionW (strLocalName, NULL, &dwRemoteNameSize);

        wchar_t *RemoteName = strRemoteName.GetBuffer (dwRemoteNameSize);

        if ( WNetGetConnectionW(strLocalName, RemoteName, &dwRemoteNameSize) == NO_ERROR )
            ShowRealPath=TRUE;

        strRemoteName.ReleaseBuffer ();
    }
    else if(DriveType == DRIVE_SUBSTITUTE)
        ShowRealPath=TRUE;

    if(ShowRealPath)
    {
      strTitle += strRemoteName;
      strTitle += L" ";
    }

    TruncStrW(strTitle,X2-X1-3);
    GotoXY(X1+(X2-X1+1-strTitle.GetLength())/2,Y1+3);
    PrintTextW(strTitle);

    unsigned __int64 TotalSize,TotalFree,UserFree;
    if (GetDiskSizeW(strDriveRoot,&TotalSize,&TotalFree,&UserFree))
    {
      GotoXY(X1+2,Y1+4);
      PrintTextW(MInfoDiskTotal);
      InsertCommasW(TotalSize,strOutStr);
      PrintInfoW(strOutStr);
      GotoXY(X1+2,Y1+5);
      PrintTextW(MInfoDiskFree);
      InsertCommasW(UserFree,strOutStr);
      PrintInfoW(strOutStr);
    }

    GotoXY(X1+2,Y1+6);
    PrintTextW(MInfoDiskLabel);
    PrintInfoW(strVolumeName);
    GotoXY(X1+2,Y1+7);
    PrintTextW(MInfoDiskNumber);
    strOutStr.Format (L"%04X-%04X",VolumeNumber>>16,VolumeNumber & 0xffff);
    PrintInfoW(strOutStr);
  }

  strTitle = UMSG(MInfoMemory);
  GotoXY(X1+(X2-X1-strTitle.GetLength())/2,Y1+8);
  PrintTextW(strTitle);
  MEMORYSTATUSEX ms;
  FAR_GlobalMemoryStatusEx(&ms);
  if (ms.dwMemoryLoad==0)
    ms.dwMemoryLoad=100-ToPercent64(ms.ullAvailPhys+ms.ullAvailPageFile,ms.ullTotalPhys+ms.ullTotalPageFile);
  GotoXY(X1+2,Y1+9);
  PrintTextW(MInfoMemoryLoad);
  strOutStr.Format (L"%d%%",ms.dwMemoryLoad);
  PrintInfoW(strOutStr);
  GotoXY(X1+2,Y1+10);
  PrintTextW(MInfoMemoryTotal);
  InsertCommasW((__int64)ms.ullTotalPhys,strOutStr);
  PrintInfoW(strOutStr);
  GotoXY(X1+2,Y1+11);
  PrintTextW(MInfoMemoryFree);
  InsertCommasW((__int64)ms.ullAvailPhys,strOutStr);
  PrintInfoW(strOutStr);
  GotoXY(X1+2,Y1+12);
  PrintTextW(MInfoVirtualTotal);
  InsertCommasW((__int64)ms.ullTotalPageFile,strOutStr);
  PrintInfoW(strOutStr);
  GotoXY(X1+2,Y1+13);
  PrintTextW(MInfoVirtualFree);
  InsertCommasW((__int64)ms.ullAvailPageFile,strOutStr);
  PrintInfoW(strOutStr);
  ShowDirDescription();
  ShowPluginDescription();
}


int InfoList::ProcessKey(int Key)
{
  if (!IsVisible())
    return(FALSE);

  if(ProcessShortcutFolder(Key,FALSE))
    return(TRUE);

  switch(Key)
  {
    /* $ 30.04.2001 DJ
       показываем правильную тему хелпа
    */
    case KEY_F1:
      {
        Help Hlp (L"InfoPanel");
      }
      return TRUE;
    /* DJ $ */
    case KEY_F3:
    case KEY_NUMPAD5:  case KEY_SHIFTNUMPAD5:
        if ( !strDizFileName.IsEmpty() )
      {
        CtrlObject->Cp()->GetAnotherPanel(this)->GetCurDirW(strCurDir);
        FarChDirW(strCurDir);

        new FileViewer(strDizFileName,TRUE);//OT
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
        AnotherPanel->GetCurDirW(strCurDir);
        FarChDirW(strCurDir);
        if ( !strDizFileName.IsEmpty() )
        {
          new FileEditor(strDizFileName,-1,FFILEEDIT_ENABLEF6);
        }
        else if ( !Opt.strFolderInfoFiles.IsEmpty() )
        {
          string strArgName;
          const wchar_t *p = Opt.strFolderInfoFiles;
          while ((p = GetCommaWordW(p,strArgName)) != NULL)
          {
            if (!wcspbrk (strArgName, L"*?"))
            {
              new FileEditor(strArgName,-1,FFILEEDIT_CANNEWFILE|FFILEEDIT_ENABLEF6);
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
      Redraw();
      CtrlObject->Cp()->GetAnotherPanel(this)->Redraw();
      DizView->SelectText(Pos,Length,Flags|1);
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
    _tran(SysLog(L"InfoList::ProcessMouse() DizView = %p",DizView));
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


void InfoList::PrintTextW(const wchar_t *Str)
{
    if (WhereY()>Y2-1)
        return;
    mprintfW(L"%.*s",X2-WhereX(),Str);
}


void InfoList::PrintTextW(int MsgID)
{
    PrintTextW(UMSG(MsgID));
}


void InfoList::PrintInfoW(const wchar_t *str)
{
    if (WhereY()>Y2-1)
        return;
    int SaveColor=GetColor(),MaxLength=X2-WhereX()-2;
    if (MaxLength<0)
        MaxLength=0;

    string strStr = str;
    TruncStrW(strStr,MaxLength);
    int Length=strStr.GetLength();
    int NewX=X2-Length-1;
    if (NewX>X1 && NewX>WhereX())
    {
        GotoXY(NewX,WhereY());
        SetColor(COL_PANELINFOTEXT);
        mprintfW(L"%s ",(const wchar_t*)strStr);
        SetColor(SaveColor);
    }
}


void InfoList::PrintInfoW(int MsgID)
{
    PrintInfoW(UMSG(MsgID));
}


void InfoList::ShowDirDescription()
{
  string strDizDir;
  int Length;
  Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);
  DrawSeparator(Y1+14);
  if (AnotherPanel->GetMode()!=FILE_PANEL)
  {
    SetColor(COL_PANELTEXT);
    GotoXY(X1+2,Y1+15);
    PrintTextW(MInfoDizAbsent);
    return;
  }
  AnotherPanel->GetCurDirW(strDizDir);
  if ((Length=strDizDir.GetLength())>0 && strDizDir.At(Length-1)!=L'\\')
    strDizDir += L"\\";

  string strArgName;

  const wchar_t *NamePtr = Opt.strFolderInfoFiles;
  while ((NamePtr=GetCommaWordW(NamePtr,strArgName))!=NULL)
  {
    string strFullDizName;
    strFullDizName = strDizDir;
    strFullDizName += strArgName;

    FAR_FIND_DATA_EX FindData;

    if ( !apiGetFindDataEx (strFullDizName,&FindData) )
      continue;

    CutToSlashW (strFullDizName, false);
    strFullDizName += FindData.strFileName;

    if (OpenDizFile(strFullDizName))
      return;
  }
  /* $ 30.04.2001 DJ
     CloseDizFile() -> CloseFile()
  */
  CloseFile();
  /* DJ $ */
  SetColor(COL_PANELTEXT);
  GotoXY(X1+2,Y1+15);
  PrintTextW(MInfoDizAbsent);
}


void InfoList::ShowPluginDescription()
{
  Panel *AnotherPanel;
  static wchar_t VertcalLine[2]={0xBA,0x00}; //BUGBUG
  AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(this);
  if (AnotherPanel->GetMode()!=PLUGIN_PANEL)
    return;
  /* $ 30.04.2001 DJ
     CloseDizFile() -> CloseFile()
  */
  CloseFile();
  /* DJ $ */
  struct OpenPluginInfoW Info;
  AnotherPanel->GetOpenPluginInfo(&Info);
  for (int I=0;I<Info.InfoLinesNumber;I++)
  {
    int Y=Y2-Info.InfoLinesNumber+I;
    if (Y<=Y1)
      continue;
    const struct InfoPanelLineW *InfoLine=&Info.InfoLines[I];
    GotoXY(X1,Y);
    SetColor(COL_PANELBOX);
    TextW(VertcalLine);
    SetColor(COL_PANELTEXT);
    mprintfW(L"%*s",X2-X1-1,L"");
    SetColor(COL_PANELBOX);
    TextW(VertcalLine);
    GotoXY(X1+2,Y);
    if (InfoLine->Separator)
    {
      string strTitle;
      if (InfoLine->Text!=NULL && *InfoLine->Text)
        strTitle.Format (L" %s ",InfoLine->Text);
      else
        strTitle=L"";
      DrawSeparator(Y);
      TruncStrW(strTitle,X2-X1-3);
      GotoXY(X1+(X2-X1-strTitle.GetLength())/2,Y);
      PrintTextW(strTitle);
    }
    else
    {
      PrintTextW(NullToEmptyW(InfoLine->Text));
      PrintInfoW(NullToEmptyW(InfoLine->Data));
    }
  }
}

/* $ 30.04.2001 DJ
   CloseDizFile() -> CloseFile()
*/
void InfoList::CloseFile()
{
  if (DizView!=NULL)
  {
    /* $ 12.10.2001 SKV
      Если идёт вызов метода DizView,
      то не надо делать delete...
    */
    if(DizView->InRecursion)return;
    /* SKV$*/
    LastDizWrapMode=DizView->GetWrapMode();
    LastDizWrapType=DizView->GetWrapType();
    /* $ 27.04.2001 DJ
       запоминаем, был ли включен скроллбар
    */
    LastDizShowScrollbar=DizView->GetShowScrollbar();
    /* DJ $ */
    DizView->SetWrapMode(OldWrapMode);
    DizView->SetWrapType(OldWrapType);
    delete DizView;
    DizView=NULL;
  }
  strDizFileName=L"";
}
/* DJ $ */

int InfoList::OpenDizFile(const wchar_t *DizFile)
{
  _tran(SysLog(L"InfoList::OpenDizFile([%s]",DizFile));
  if (DizView == NULL)
  {
    /* $ 12.10.2001 SKV
      Теперь это не просто Viewer, а DizViewer :)
    */
    DizView=new DizViewer;
    /* SKV$*/
    _tran(SysLog(L"InfoList::OpenDizFile() create new Viewer = %p",DizView));
    DizView->SetRestoreScreenMode(FALSE);
    DizView->SetPosition(X1+1,Y1+15,X2-1,Y2-1);
    DizView->SetStatusMode(0);
    DizView->EnableHideCursor(0);
    OldWrapMode = DizView->GetWrapMode();
    OldWrapType = DizView->GetWrapType();
    DizView->SetWrapMode(LastDizWrapMode);
    DizView->SetWrapType(LastDizWrapType);
    /* $ 27.04.2001 DJ
       если скроллбар был включен, включаем
    */
    DizView->SetShowScrollbar (LastDizShowScrollbar);
    /* DJ $ */
  }

  if (!DizView->OpenFile(DizFile,FALSE))
  {
    DizPresent=FALSE;
    return(FALSE);
  }
  DizView->Show();

  strDizFileName = DizFile;
  string strTitle;
  strTitle.Format (L" %s ", (const wchar_t*)PointToNameW(strDizFileName));
  TruncStrW(strTitle,X2-X1-3);
  GotoXY(X1+(X2-X1-strTitle.GetLength())/2,Y1+14);
  SetColor(COL_PANELTEXT);
  PrintTextW(strTitle);
  DizPresent=TRUE;
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


int InfoList::GetCurNameW(string &strName, string &strShortName)
{
  strName = strDizFileName;

  ConvertNameToShortW(strName, strShortName);

  return (TRUE);
}

/* IS $ */
/* $ 30.04.2001 DJ
   свой кейбар
*/

BOOL InfoList::UpdateKeyBar()
{
  KeyBar *KB = CtrlObject->MainKeyBar;
  KB->SetAllGroup (KBL_MAIN, MInfoF1, 12);
  KB->SetAllGroup (KBL_SHIFT, MInfoShiftF1, 12);
  KB->SetAllGroup (KBL_ALT, MInfoAltF1, 12);
  KB->SetAllGroup (KBL_CTRL, MInfoCtrlF1, 12);
  KB->ClearGroup (KBL_CTRLSHIFT);
  KB->ClearGroup (KBL_CTRLALT);
  KB->ClearGroup (KBL_ALTSHIFT);
  DynamicUpdateKeyBar();

  return TRUE;
}

void InfoList::DynamicUpdateKeyBar()
{
  KeyBar *KB = CtrlObject->MainKeyBar;
  if (DizView && DizPresent)
  {
    KB->Change (UMSG(MInfoF3), 3-1);

    if (DizView->GetAnsiMode())
      KB->Change (UMSG(MViewF8DOS), 7);
    else
      KB->Change (UMSG(MInfoF8), 7);

    if (!DizView->GetWrapMode())
    {
      if (DizView->GetWrapType())
        KB->Change (UMSG(MViewShiftF2), 2-1);
      else
        KB->Change (UMSG(MViewF2), 2-1);
    }
    else
      KB->Change (UMSG(MViewF2Unwrap), 2-1);

    if (DizView->GetWrapType())
      KB->Change (KBL_SHIFT, UMSG(MViewF2), 2-1);
    else
      KB->Change (KBL_SHIFT, UMSG(MViewShiftF2), 2-1);
  }
  else
  {
    KB->Change (UMSG(MF2), 2-1);
    KB->Change (KBL_SHIFT, L"", 2-1);
    KB->Change (L"", 3-1);
    KB->Change (L"", 8-1);
    KB->Change (KBL_SHIFT, L"", 8-1);
    KB->Change (KBL_ALT, UMSG(MAltF8), 8-1); // стандартный для панели - "хистори"
  }
}
/* DJ $ */
