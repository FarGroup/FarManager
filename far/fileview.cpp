/*
fileview.cpp

Просмотр файла - надстройка над viewer.cpp

*/

/* Revision: 1.04 15.07.2000 $ */

/*
Modify:
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
  28.06.2000 tran
    - NT Console resize
      adding SetScreenPosition
  29.06.2000 tran
    + названия всех функциональных клавиш
  04.07.2000 tran
    + не показывать мессаг бакс при невозвожности открыть файл
  15.07.2000 tran
    + CtrlB выключает/включает keybar
*/

#include "headers.hpp"
#pragma hdrstop

/* $ 30.06.2000 IS
   Стандартные заголовки
*/
#include "internalheaders.hpp"
/* IS $ */

FileViewer::FileViewer(char *Name,int EnableSwitch,int DisableHistory,
                       int DisableEdit,long ViewStartPos,char *PluginData,
                       NamesList *ViewNamesList)
{
  FileViewer::DisableEdit=DisableEdit;
  SetPosition(0,0,ScrX,ScrY);
  FullScreen=TRUE;
  Init(Name,EnableSwitch,DisableHistory,ViewStartPos,PluginData,ViewNamesList);
}


FileViewer::FileViewer(char *Name,int EnableSwitch,char *Title,
                       int X1,int Y1,int X2,int Y2)
{
  DisableEdit=TRUE;
  SetPosition(X1,Y1,X2,Y2);
  FullScreen=(X1==0 && Y1==0 && X2==ScrX && Y2==ScrY);
  View.SetTitle(Title);
  Init(Name,EnableSwitch,TRUE,-1,"",NULL);
}


void FileViewer::Init(char *Name,int EnableSwitch,int DisableHistory,
                      long ViewStartPos,char *PluginData,
                      NamesList *ViewNamesList)
{
  ViewKeyBar.SetOwner(this);
  ViewKeyBar.SetPosition(X1,Y2,X2,Y2);
  View.SetPluginData(PluginData);

  SetEnableSwitch(EnableSwitch);

  /* $ 29.06.2000 tran
     добавил названия всех функциональных клавиш */
  char *FViewKeys[]={MSG(MViewF1),MSG(MViewF2),MSG(MViewF3),MSG(MViewF4),MSG(MViewF5),DisableEdit ? "":MSG(MViewF6),MSG(MViewF7),MSG(MViewF8),"",MSG(MViewF10),MSG(MViewF11),MSG(MViewF12)};
  char *FViewShiftKeys[]={MSG(MViewShiftF1),MSG(MViewShiftF2),MSG(MViewShiftF3),MSG(MViewShiftF4),MSG(MViewShiftF5),MSG(MViewShiftF6),MSG(MViewShiftF7),MSG(MViewShiftF8),MSG(MViewShiftF9),MSG(MViewShiftF10),MSG(MViewShiftF11),MSG(MViewShiftF12)};
  char *FViewAltKeys[]={MSG(MViewAltF1),MSG(MViewAltF2),MSG(MViewAltF3),MSG(MViewAltF4),MSG(MViewAltF5),MSG(MViewAltF6),MSG(MViewAltF7),MSG(MViewAltF8),MSG(MViewAltF9),MSG(MViewAltF10),MSG(MViewAltF11),MSG(MViewAltF12)};
  char *FViewCtrlKeys[]={MSG(MViewCtrlF1),MSG(MViewCtrlF2),MSG(MViewCtrlF3),MSG(MViewCtrlF4),MSG(MViewCtrlF5),MSG(MViewCtrlF6),MSG(MViewCtrlF7),MSG(MViewCtrlF8),MSG(MViewCtrlF9),MSG(MViewCtrlF10),MSG(MViewCtrlF11),MSG(MViewCtrlF12)};

  ViewKeyBar.Set(FViewKeys,sizeof(FViewKeys)/sizeof(FViewKeys[0]));
  ViewKeyBar.SetShift(FViewShiftKeys,sizeof(FViewShiftKeys)/sizeof(FViewShiftKeys[0]));
  ViewKeyBar.SetAlt(FViewAltKeys,sizeof(FViewAltKeys)/sizeof(FViewAltKeys[0]));
  ViewKeyBar.SetCtrl(FViewCtrlKeys,sizeof(FViewCtrlKeys)/sizeof(FViewCtrlKeys[0]));
  /* tran $ */
  SetKeyBar(&ViewKeyBar);
  /* $ 15.07.2000 tran
     ShowKeyBarViewer support*/
  View.SetPosition(X1,Y1,X2,Y2-(Opt.ShowKeyBarViewer?1:0));
  /* tran 15.07.2000 $ */

  View.SetViewKeyBar(&ViewKeyBar);
  /* $ 04.07.2000 tran
     + add TRUE as 'warning' parameter */
  if (!View.OpenFile(Name,TRUE))
  /* tran 04.07.2000 $ */
  {
    ExitCode=FALSE;
    return;
  }

  if (ViewStartPos!=-1)
    View.SetFilePos(ViewStartPos);
  if (ViewNamesList)
    View.SetNamesList(ViewNamesList);
  ExitCode=TRUE;
  ViewKeyBar.Show();
  /* $ 15.07.2000 tran
     dirty trick :( */
  if ( Opt.ShowKeyBarViewer==0 )
    ViewKeyBar.Hide0();
  /* tran 15.07.2000 $ */


  sprintf(NewTitle,MSG(MInViewer),PointToName(Name));
  SetFarTitle(NewTitle);
  ShowConsoleTitle();
  F3KeyOnly=TRUE;
  Process();
  if (!DisableHistory && (CtrlObject->ActivePanel!=NULL || strcmp(Name,"-")!=0))
  {
    char FullFileName[NM];
    ConvertNameToFull(Name,FullFileName);
    CtrlObject->ViewHistory->AddToHistory(FullFileName,MSG(MHistoryView),0);
  }
}


void FileViewer::Process()
{
  ChangeMacroMode MacroMode(MACRO_VIEWER);
  Modal::Process();
}


void FileViewer::Show()
{
  if (FullScreen)
  {
    /* $ 15.07.2000 tran
       + keybar hide/show support */
    if ( Opt.ShowKeyBarViewer )
    {
        ViewKeyBar.SetPosition(0,ScrY,ScrX,ScrY);
        ViewKeyBar.Redraw();
    }
    SetPosition(0,0,ScrX,ScrY-(Opt.ShowKeyBarViewer?1:0));
    View.SetPosition(0,0,ScrX,ScrY-(Opt.ShowKeyBarViewer?1:0));
    /* tran 15.07.2000 $ */
  }
  ScreenObject::Show();
}


void FileViewer::DisplayObject()
{
  View.Show();
}


int FileViewer::ProcessKey(int Key)
{
  if (Key!=KEY_F3 && Key!=KEY_NUMPAD5)
    F3KeyOnly=FALSE;
  switch(Key)
  {
    /* $ 15.07.2000 tran
       + CtrlB switch KeyBar*/
    case KEY_CTRLB:
      Opt.ShowKeyBarViewer=!Opt.ShowKeyBarViewer;
      if ( Opt.ShowKeyBarViewer )
        ViewKeyBar.Show();
      else
        ViewKeyBar.Hide0(); // 0 mean - Don't purge saved screen
      Show();
      return (TRUE);
    /* tran 15.07.2000 $ */
    case KEY_CTRLO:
      Hide();
      if (CtrlObject->LeftPanel!=CtrlObject->RightPanel)
        CtrlObject->ModalManager.ShowBackground();
      else
      {
        ViewKeyBar.Hide();
        WaitKey();
      }
      ViewKeyBar.Show();
      Show();
      return(TRUE);
    case KEY_CTRLTAB:
    case KEY_CTRLSHIFTTAB:
    case KEY_F12:
      if (GetEnableSwitch())
      {
        View.KeepInitParameters();
        if (Key==KEY_CTRLSHIFTTAB)
          SetExitCode(3);
        else
          SetExitCode(Key==KEY_CTRLTAB ? 1:2);
      }
      return(TRUE);
    case KEY_F3:
    case KEY_NUMPAD5:
      if (F3KeyOnly)
        return(TRUE);
    case KEY_ESC:
    case KEY_F10:
      SetExitCode(0);
      return(TRUE);
    case KEY_F6:
      if (!DisableEdit)
      {
        SetExitCode(0);
        char ViewFileName[NM];
        View.GetFileName(ViewFileName);
        long FilePos=View.GetFilePos();
        CtrlObject->ModalManager.SetNextWindow(FALSE,ViewFileName,FilePos);
        ShowTime(2);
      }
      return(TRUE);
    default:
      return(View.ProcessKey(Key));
  }
}


int FileViewer::ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent)
{
  F3KeyOnly=FALSE;
  if (!View.ProcessMouse(MouseEvent))
    if (!ViewKeyBar.ProcessMouse(MouseEvent))
      return(FALSE);
  return(TRUE);
}


int FileViewer::GetTypeAndName(char *Type,char *Name)
{
  strcpy(Type,MSG(MScreensView));
  View.GetFileName(Name);
  return(MODALTYPE_VIEWER);
}


void FileViewer::ShowConsoleTitle()
{
  View.ShowConsoleTitle();
}


void FileViewer::SetTempViewName(char *Name)
{
  View.SetTempViewName(Name);
}


int FileViewer::GetExitCode()
{
  return(ExitCode);
}

/* $ 28.06.2000 tran
 (NT Console resize)
 resize viewer */
void FileViewer::SetScreenPosition()
{
  if (FullScreen)
  {
    SetPosition(0,0,ScrX,ScrY);
    Show();
  }
}
/* tran $ */
