/*
qview.cpp

Quick view panel

*/

#include "headers.hpp"
#pragma hdrstop

#include "qview.hpp"
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
  Directory=0;
  PrevMacroMode = -1;
  /* $ 20.02.2001 VVM
    + ѕроинициализируем режим врап-а */
  if (LastWrapMode < 0) {
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


void QuickView::GetTitle(string &strTitle,int SubLen,int TruncSize)
{
  strTitle.Format (L" %s ", UMSG(MQuickViewTitle));
  TruncStr(strTitle,X2-X1-3);
}

void QuickView::DisplayObject()
{
  if (Flags.Check(FSCROBJ_ISREDRAWING))
    return;
  Flags.Set(FSCROBJ_ISREDRAWING);

  string strTitle;
  if (QView==NULL && !ProcessingPluginCommand)
    CtrlObject->Cp()->GetAnotherPanel(this)->UpdateViewPanel();
  if (QView!=NULL)
    QView->SetPosition(X1+1,Y1+1,X2-1,Y2-3);
  Box(X1,Y1,X2,Y2,COL_PANELBOX,DOUBLE_BOX);
  SetScreen(X1+1,Y1+1,X2-1,Y2-1,L' ',COL_PANELTEXT);
  SetColor(Focus ? COL_PANELSELECTEDTITLE:COL_PANELTITLE);
  GetTitle(strTitle);
  if ( !strTitle.IsEmpty() )
  {
    GotoXY(X1+(X2-X1+1-(int)strTitle.GetLength())/2,Y1);
    Text(strTitle);
  }
  DrawSeparator(Y2-2);
  SetColor(COL_PANELTEXT);
  GotoXY(X1+1,Y2-1);
  mprintf(L"%-*.*s",X2-X1-1,X2-X1-1,PointToName(strCurFileName));

  if ( !strCurFileType.IsEmpty() )
  {
    string strTypeText;
    strTypeText.Format (L" %s ", (const wchar_t*)strCurFileType);
    TruncStr(strTypeText,X2-X1-1);
    SetColor(COL_PANELSELECTEDINFO);
    GotoXY(X1+(X2-X1+1-(int)strTypeText.GetLength())/2,Y2-2);
    Text(strTypeText);
  }
  if (Directory)
  {
    string strMsg;

    strMsg.Format (UMSG(MQuickViewFolder),(const wchar_t*)strCurFileName);

    TruncStr(strMsg,X2-X1-4);
    SetColor(COL_PANELTEXT);
    GotoXY(X1+2,Y1+2);
    PrintText(strMsg);
    /* $ 01.02.2001 SVS
       ¬ панели "Quick view" добавим инфу про Junction
    */
    if((GetFileAttributesW(strCurFileName)&FILE_ATTRIBUTE_REPARSE_POINT) == FILE_ATTRIBUTE_REPARSE_POINT)
    {
      string strJuncName;
      int ID_Msg, Width;
      if(GetJunctionPointInfo(strCurFileName, strJuncName)) //"\??\D:\Junc\Src\"
      {
        strJuncName.RShift (4);

        if(!wcsncmp(strJuncName,L"Volume{",7))
        {
          string strJuncRoot;
          GetPathRootOne(strJuncName, strJuncRoot);
          if( strJuncRoot.At(1) == L':')
          {
              strJuncName = strJuncRoot;
          }
          ID_Msg=MQuickViewVolMount;
          Width=20;
        }
        else
        {
          ID_Msg=MQuickViewJunction;
          Width=9;
        }

        TruncPathStr(strJuncName,X2-X1-4-Width);

        strMsg.Format (UMSG(ID_Msg), (const wchar_t*)strJuncName);
        TruncStr(strMsg,X2-X1-4);
        SetColor(COL_PANELTEXT);
        GotoXY(X1+2,Y1+3);
        PrintText(strMsg);
      }
    }
    /* SVS $ */

    if (Directory==1 || Directory==4)
    {
      string strSlackMsg;

      GotoXY(X1+2,Y1+4);
      PrintText(UMSG(MQuickViewContains));
      GotoXY(X1+2,Y1+6);
      PrintText(UMSG(MQuickViewFolders));
      SetColor(COL_PANELINFOTEXT);
      strMsg.Format (L"%d",DirCount);
      PrintText(strMsg);
      SetColor(COL_PANELTEXT);
      GotoXY(X1+2,Y1+7);
      PrintText(UMSG(MQuickViewFiles));
      SetColor(COL_PANELINFOTEXT);
      strMsg.Format (L"%d",FileCount);
      PrintText(strMsg);
      SetColor(COL_PANELTEXT);
      GotoXY(X1+2,Y1+8);
      PrintText(UMSG(MQuickViewBytes));
      SetColor(COL_PANELINFOTEXT);
      InsertCommas(FileSize,strMsg);
      PrintText(strMsg);
      SetColor(COL_PANELTEXT);
      GotoXY(X1+2,Y1+9);
      PrintText(UMSG(MQuickViewCompressed));
      SetColor(COL_PANELINFOTEXT);
      InsertCommas(CompressedFileSize,strMsg);
      PrintText(strMsg);

      SetColor(COL_PANELTEXT);
      GotoXY(X1+2,Y1+10);
      PrintText(UMSG(MQuickViewRatio));
      SetColor(COL_PANELINFOTEXT);
      strSlackMsg.Format (L"%d%%",ToPercent64(CompressedFileSize,FileSize));
      PrintText(strSlackMsg);

      if (Directory!=4 && RealFileSize>=CompressedFileSize)
      {
        SetColor(COL_PANELTEXT);
        GotoXY(X1+2,Y1+12);
        PrintText(UMSG(MQuickViewCluster));
        SetColor(COL_PANELINFOTEXT);
        InsertCommas(ClusterSize,strMsg);
        PrintText(strMsg);
        SetColor(COL_PANELTEXT);
        GotoXY(X1+2,Y1+13);
        PrintText(UMSG(MQuickViewRealSize));
        SetColor(COL_PANELINFOTEXT);
        InsertCommas(RealFileSize,strMsg);
        PrintText(strMsg);
        SetColor(COL_PANELTEXT);
        GotoXY(X1+2,Y1+14);
        PrintText(UMSG(MQuickViewSlack));
        SetColor(COL_PANELINFOTEXT);
        InsertCommas(RealFileSize-CompressedFileSize,strMsg);
        unsigned __int64 Size1=RealFileSize-CompressedFileSize;
        unsigned __int64 Size2=RealFileSize;

        while ( (Size2 >> 32) !=0)
        {
          Size1=Size1>>1;
          Size2=Size2>>1;
        }
        strSlackMsg.Format (L"%s (%d%%)",(const wchar_t*)strMsg,ToPercent((DWORD)Size1, (DWORD)Size2));
        PrintText(strSlackMsg);
      }
    }
  }
  else
    if (QView!=NULL)
      QView->Show();
 Flags.Clear(FSCROBJ_ISREDRAWING);
}


int QuickView::ProcessKey(int Key)
{
  if (!IsVisible())
    return(FALSE);

  if(ProcessShortcutFolder(Key,FALSE))
    return(TRUE);

  /* $ 30.04.2001 DJ
     показываем правильный help topic
  */
  if (Key == KEY_F1)
  {
    Help Hlp (L"QViewPanel");
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
      //__int64 Pos;
      //int Length;
      //DWORD Flags;
      //QView->GetSelectedParam(Pos,Length,Flags);
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
  if ( strCurFileName.IsEmpty() )
    CtrlObject->Cp()->GetAnotherPanel(this)->UpdateViewPanel();
  Redraw();
}
#if defined(__BORLANDC__)
#pragma warn +par
#endif


void QuickView::ShowFile(const wchar_t *FileName,int TempFile,HANDLE hDirPlugin)
{
  const wchar_t *ExtPtr;
  int FileAttr;
  CloseFile();
  QView=NULL;

  strCurFileName = L"";

  if (!IsVisible())
    return;
  if (FileName==NULL)
  {
    ProcessingPluginCommand++;
    Show();
    ProcessingPluginCommand--;
    return;
  }
  QView=new Viewer;
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

  strCurFileName = FileName;

  if ((ExtPtr=wcsrchr(strCurFileName,L'.'))!=NULL)
  {
    string strValue;

    if ( RegQueryStringValue (HKEY_CLASSES_ROOT, ExtPtr, strValue)==ERROR_SUCCESS)
    {
      if (RegQueryStringValue (HKEY_CLASSES_ROOT, strValue,strCurFileType)!=ERROR_SUCCESS)
        strCurFileType=L"";
    }
  }
  if (hDirPlugin || (FileAttr=GetFileAttributesW(strCurFileName))!=-1 && (FileAttr & FA_DIREC))
  {
    /* $ 28.06.2000 IS
     Ќе показывать тип файла дл€ каталогов в "Ѕыстром просмотре" /
    */
    strCurFileType=L"";
    /* IS $ */
    if (hDirPlugin)
    {
      int ExitCode=GetPluginDirInfo(hDirPlugin,strCurFileName,DirCount,
                   FileCount,FileSize,CompressedFileSize);
      if (ExitCode)
        Directory=4;
      else
        Directory=3;
    }
    else
    {
      int ExitCode=GetDirInfo(UMSG(MQuickViewTitle),strCurFileName,DirCount,
                   FileCount,FileSize,CompressedFileSize,RealFileSize,
                   ClusterSize,500,NULL,GETDIRINFO_ENHBREAK|GETDIRINFO_SCANSYMLINKDEF);
      if (ExitCode==1)
        Directory=1;
      else
        if (ExitCode==-1)
          Directory=2;
        else
          Directory=3;
    }
  }
  else
    if ( !strCurFileName.IsEmpty() )
      QView->OpenFile(strCurFileName,FALSE);

  if (TempFile)
    ConvertNameToFull (strCurFileName, strTempName);

  Redraw();

  if (CtrlObject->Cp()->ActivePanel == this)
  {
    DynamicUpdateKeyBar();
    CtrlObject->MainKeyBar->Redraw();
  }
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

  strCurFileType = L"";

  QViewDelTempName();
  Directory=0;
}


void QuickView::QViewDelTempName()
{
  if ( !strTempName.IsEmpty() )
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

    SetFileAttributesW (strTempName, FILE_ATTRIBUTE_ARCHIVE); //was chmod(TempName,S_IREAD|S_IWRITE);
    DeleteFileW (strTempName); //BUGBUG

    CutToSlash(strTempName);
    apiRemoveDirectory(strTempName);

    strTempName=L"";
  }
}


void QuickView::PrintText(const wchar_t *Str)
{
  if (WhereY()>Y2-3 || WhereX()>X2-2)
    return;

  mprintf(L"%.*s",X2-2-WhereX()+1,Str);
}


int QuickView::UpdateIfChanged(int UpdateMode)
{
  if (IsVisible() && !strCurFileName.IsEmpty() && Directory==2)
  {
    string strViewName = strCurFileName;
    ShowFile(strViewName, !strTempName.IsEmpty() ,NULL);
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
    string strTitleDir = L"{";
    if ( !strCurFileName.IsEmpty() )
    {
      strTitleDir += strCurFileName;
      strTitleDir += L" - QuickView";
    }
    else
    {
      string strCmdText;
      CtrlObject->CmdLine->GetString(strCmdText);

      strTitleDir += strCmdText;
    }

    strTitleDir += L"}";

    strLastFarTitle = strTitleDir;
    SetFarTitle(strTitleDir);
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

int QuickView::GetCurName(string &strName, string &strShortName)
{
  if ( !strCurFileName.IsEmpty() )
  {
    strName = strCurFileName;
    strShortName = strName;
    return (TRUE);
  }
  return (FALSE);
}

BOOL QuickView::UpdateKeyBar()
{
  KeyBar *KB = CtrlObject->MainKeyBar;
  KB->SetAllGroup (KBL_MAIN, MQViewF1, 12);
  KB->SetAllGroup (KBL_SHIFT, MQViewShiftF1, 12);
  KB->SetAllGroup (KBL_ALT, MQViewAltF1, 12);
  KB->SetAllGroup (KBL_CTRL, MQViewCtrlF1, 12);
  KB->ClearGroup (KBL_CTRLSHIFT);
  KB->ClearGroup (KBL_CTRLALT);
  KB->ClearGroup (KBL_ALTSHIFT);

  DynamicUpdateKeyBar();

  return TRUE;
}

void QuickView::DynamicUpdateKeyBar()
{
  KeyBar *KB = CtrlObject->MainKeyBar;
  if (Directory || !QView)
  {
    KB->Change (UMSG(MF2), 2-1);
    KB->Change (L"", 4-1);
    KB->Change (L"", 8-1);
    KB->Change (KBL_SHIFT, L"", 2-1);
    KB->Change (KBL_SHIFT, L"", 8-1);
    KB->Change (KBL_ALT, UMSG(MAltF8), 8-1); // стандартный дл€ панели - "хистори"
  }
  else {
    if (QView->GetHexMode())
      KB->Change (UMSG(MViewF4Text), 4-1);
    else
      KB->Change (UMSG(MQViewF4), 4-1);

    if (QView->GetAnsiMode())
      KB->Change (UMSG(MViewF8DOS), 8-1);
    else
      KB->Change (UMSG(MQViewF8), 8-1);

    if (!QView->GetWrapMode())
    {
      if (QView->GetWrapType())
        KB->Change (UMSG(MViewShiftF2), 2-1);
      else
        KB->Change (UMSG(MViewF2), 2-1);
    }
    else
      KB->Change (UMSG(MViewF2Unwrap), 2-1);

    if (QView->GetWrapType())
      KB->Change (KBL_SHIFT, UMSG(MViewF2), 2-1);
    else
      KB->Change (KBL_SHIFT, UMSG(MViewShiftF2), 2-1);
  }

  KB->ReadRegGroup(L"QView",Opt.strLanguage);
  KB->SetAllRegGroup();
}
