/*
fileview.cpp

Просмотр файла - надстройка над viewer.cpp

*/

/* Revision: 1.00 25.06.2000 $ */

/*
Modify:
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#include "headers.hpp"
#pragma hdrstop

#ifndef __FARCONST_HPP__
#include "farconst.hpp"
#endif
#ifndef __KEYS_HPP__
#include "keys.hpp"
#endif
#ifndef __FARLANG_HPP__
#include "lang.hpp"
#endif
#ifndef __COLOROS_HPP__
#include "colors.hpp"
#endif
#ifndef __FARSTRUCT_HPP__
#include "struct.hpp"
#endif
#ifndef __PLUGIN_HPP__
#include "plugin.hpp"
#endif
#ifndef __CLASSES_HPP__
#include "classes.hpp"
#endif
#ifndef __FARFUNC_HPP__
#include "fn.hpp"
#endif
#ifndef __FARGLOBAL_HPP__
#include "global.hpp"
#endif

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

  char *FViewKeys[]={MSG(MViewF1),MSG(MViewF2),MSG(MViewF3),MSG(MViewF4),"",DisableEdit ? "":MSG(MViewF6),MSG(MViewF7),MSG(MViewF8),"",MSG(MViewF10),MSG(MViewF11),MSG(MViewF12)};
  char *FViewShiftKeys[]={"","","","","","",MSG(MViewShiftF7),MSG(MViewShiftF8),"",""};
  char *FViewAltKeys[]={"","","","","","","",MSG(MViewAltF8),"",""};
  ViewKeyBar.Set(FViewKeys,sizeof(FViewKeys)/sizeof(FViewKeys[0]));
  ViewKeyBar.SetShift(FViewShiftKeys,sizeof(FViewShiftKeys)/sizeof(FViewShiftKeys[0]));
  ViewKeyBar.SetAlt(FViewAltKeys,sizeof(FViewAltKeys)/sizeof(FViewAltKeys[0]));
  SetKeyBar(&ViewKeyBar);
  View.SetPosition(X1,Y1,X2,Y2-1);
  View.SetViewKeyBar(&ViewKeyBar);
  if (!View.OpenFile(Name))
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
    ViewKeyBar.SetPosition(0,ScrY,ScrX,ScrY);
    ViewKeyBar.Redraw();
    SetPosition(0,0,ScrX,ScrY-1);
    View.SetPosition(0,0,ScrX,ScrY-1);
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
