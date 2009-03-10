/*
fileview.cpp

Просмотр файла - надстройка над viewer.cpp

*/

#include "headers.hpp"
#pragma hdrstop

#include "fileview.hpp"
#include "global.hpp"
#include "fn.hpp"
#include "lang.hpp"
#include "keys.hpp"
#include "ctrlobj.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "history.hpp"
#include "manager.hpp"
#include "fileedit.hpp"
#include "cmdline.hpp"
#include "savescr.hpp"

FileViewer::FileViewer(const char *Name,int EnableSwitch,int DisableHistory,
                       int DisableEdit,long ViewStartPos,char *PluginData,
                       NamesList *ViewNamesList,int ToSaveAs)
{
  _OT(SysLog("[%p] FileViewer::FileViewer(I variant...)", this));
  FileViewer::DisableEdit=DisableEdit;
  SetPosition(0,0,ScrX,ScrY);
  FullScreen=TRUE;
  Init(Name,EnableSwitch,DisableHistory,ViewStartPos,PluginData,ViewNamesList,ToSaveAs);
}


FileViewer::FileViewer(const char *Name,int EnableSwitch,int DisableHistory,
                       const char *Title, int X1,int Y1,int X2,int Y2)
{
  _OT(SysLog("[%p] FileViewer::FileViewer(II variant...)", this));
  DisableEdit=TRUE;
  /* $ 02.11.2001 IS
       отрицательные координаты левого верхнего угла заменяются на нулевые
  */
  if(X1 < 0)
    X1=0;
  if(X2 < 0 || X2 > ScrX)
    X2=ScrX;
  if(Y1 < 0)
    Y1=0;
  if(Y2 < 0 || Y2 > ScrY)
    Y2=ScrY;
  if(X1 >= X2)
  {
    X1=0;
    X2=ScrX;
  }
  if(Y1 >= Y2)
  {
    Y1=0;
    Y2=ScrY;
  }
  /* IS $ */
  SetPosition(X1,Y1,X2,Y2);
  FullScreen=(X1==0 && Y1==0 && X2==ScrX && Y2==ScrY);
  View.SetTitle(Title);
  Init(Name,EnableSwitch,DisableHistory,-1,"",NULL,FALSE);
}


void FileViewer::Init(const char *name,int EnableSwitch,int disableHistory, ///
                      long ViewStartPos,char *PluginData,
                      NamesList *ViewNamesList,int ToSaveAs)
{
  RedrawTitle = FALSE;
  ViewKeyBar.SetOwner(this);
  ViewKeyBar.SetPosition(X1,Y2,X2,Y2);
  KeyBarVisible = Opt.ViOpt.ShowKeyBar;
  TitleBarVisible = Opt.ViOpt.ShowTitleBar;

  int OldMacroMode=CtrlObject->Macro.GetMode();
  MacroMode = MACRO_VIEWER;
  CtrlObject->Macro.SetMode(MACRO_VIEWER);

  View.SetPluginData(PluginData);
  View.SetHostFileViewer(this);

  DisableHistory=disableHistory; ///
  xstrncpy(Name,name,sizeof(Name)-1); ///
  SetCanLoseFocus(EnableSwitch);

  /* $ 17.08.2001 KM
    Добавлено для поиска по AltF7. При редактировании найденного файла из
    архива для клавиши F2 сделать вызов ShiftF2.
  */
  SaveToSaveAs=ToSaveAs;
  /* KM $ */

  InitKeyBar();

  if (!View.OpenFile(Name,TRUE)) // $ 04.07.2000 tran + add TRUE as 'warning' parameter
  {
    DisableHistory = TRUE; // $ 26.03.2002 DJ - при неудаче открытия - не пишем мусор в историю
    // FrameManager->DeleteFrame(this); // ЗАЧЕМ? Вьювер то еще не помещен в очередь манагера!
    ExitCode=FALSE;
    CtrlObject->Macro.SetMode(OldMacroMode);
    return;
  }

  if (ViewStartPos!=-1)
    View.SetFilePos(ViewStartPos);
  if (ViewNamesList)
    View.SetNamesList(ViewNamesList);
  ExitCode=TRUE;
  ViewKeyBar.Show();
  if ( Opt.ViOpt.ShowKeyBar==0 )
    ViewKeyBar.Hide0();

  sprintf(NewTitle,MSG(MInViewer),PointToName(Name));
  SetFarTitle(NewTitle);
  ShowConsoleTitle();
  F3KeyOnly=TRUE;
  if (EnableSwitch) {
    FrameManager->InsertFrame(this);
  } else {
    FrameManager->ExecuteFrame(this);
  }
}


void FileViewer::InitKeyBar(void)
{
  ViewKeyBar.ReadRegGroup("Viewer",Opt.Language);

  ViewKeyBar.SetAllGroup (KBL_MAIN,         Opt.OnlyEditorViewerUsed?MSingleViewF1:MViewF1, 12);
  ViewKeyBar.SetAllGroup (KBL_SHIFT,        Opt.OnlyEditorViewerUsed?MSingleViewShiftF1:MViewShiftF1, 12);
  ViewKeyBar.SetAllGroup (KBL_ALT,          Opt.OnlyEditorViewerUsed?MSingleViewAltF1:MViewAltF1, 12);
  ViewKeyBar.SetAllGroup (KBL_CTRL,         Opt.OnlyEditorViewerUsed?MSingleViewCtrlF1:MViewCtrlF1, 12);
  ViewKeyBar.SetAllGroup (KBL_CTRLSHIFT,    Opt.OnlyEditorViewerUsed?MSingleViewCtrlShiftF1:MViewCtrlShiftF1, 12);
  ViewKeyBar.SetAllGroup (KBL_CTRLALT,      Opt.OnlyEditorViewerUsed?MSingleViewCtrlAltF1:MViewCtrlAltF1, 12);
  ViewKeyBar.SetAllGroup (KBL_ALTSHIFT,     Opt.OnlyEditorViewerUsed?MSingleViewAltShiftF1:MViewAltShiftF1, 12);
  ViewKeyBar.SetAllGroup (KBL_CTRLALTSHIFT, Opt.OnlyEditorViewerUsed?MSingleViewCtrlAltShiftF1:MViewCtrlAltShiftF1, 12);

  if(DisableEdit)
    ViewKeyBar.Change(KBL_MAIN,"",6-1);

  if(!GetCanLoseFocus())
    ViewKeyBar.Change(KBL_MAIN,"",12-1);

  if(!GetCanLoseFocus())
    ViewKeyBar.Change(KBL_ALT,"",11-1);

  if(!Opt.UsePrintManager || CtrlObject->Plugins.FindPlugin(SYSID_PRINTMANAGER) == -1)
    ViewKeyBar.Change(KBL_ALT,"",5-1);

  ViewKeyBar.SetAllRegGroup();

  SetKeyBar(&ViewKeyBar);
  // $ 15.07.2000 tran - ShowKeyBarViewer support
  View.SetPosition(X1,Y1,X2,Y2-(Opt.ViOpt.ShowKeyBar?1:0));
  View.SetViewKeyBar(&ViewKeyBar);
}
/* SVS $ */

void FileViewer::Show()
{
 _SVS(SysLog("FileViewer::Show()"));
  if (FullScreen)
  {
    /* $ 15.07.2000 tran
       + keybar hide/show support */
    if ( Opt.ViOpt.ShowKeyBar )
    {
        ViewKeyBar.SetPosition(0,ScrY,ScrX,ScrY);
        ViewKeyBar.Redraw();
    }
    SetPosition(0,0,ScrX,ScrY-(Opt.ViOpt.ShowKeyBar?1:0));
    View.SetPosition(0,(Opt.ViOpt.ShowTitleBar?1:0),ScrX,ScrY-(Opt.ViOpt.ShowKeyBar?1:0));
    /* tran 15.07.2000 $ */
  }
  ScreenObject::Show();
  ShowStatus();
}


void FileViewer::DisplayObject()
{
  View.Show();
}


__int64 FileViewer::VMProcess(int OpCode,void *vParam,__int64 iParam)
{
  return View.VMProcess(OpCode,vParam,iParam);
}

int FileViewer::ProcessKey(int Key)
{
  if (RedrawTitle && (((unsigned int)Key & 0x00ffffff) < KEY_END_FKEY || IS_INTERNAL_KEY_REAL((unsigned int)Key & 0x00ffffff)))
    ShowConsoleTitle();

  if (Key!=KEY_F3 && !(Key==KEY_NUMPAD5||Key==KEY_SHIFTNUMPAD5))
    F3KeyOnly=FALSE;
  switch(Key)
  {
#if 0
    /* $ 30.05.2003 SVS
       Фича :-) Shift-F4 в редакторе/вьювере позволяет открывать другой редактор/вьювер
       Пока закомментим
    */
    case KEY_SHIFTF4:
    {
      if(!Opt.OnlyEditorViewerUsed)
        CtrlObject->Cp()->ActivePanel->ProcessKey(Key);
      return TRUE;
    }
    /* $ SVS */
#endif
    /* $ 22.07.2000 tran
       + выход по ctrl-f10 с установкой курсора на файл */
    case KEY_CTRLF10:
      {
        if (View.isTemporary()){
          return(TRUE);
        }
        SaveScreen Sc;
        /* $ 28.12.2001 DJ
           унифицируем обработку Ctrl-F10
        */
        char FileName[NM];
        View.GetFileName(FileName);
        CtrlObject->Cp()->GoToFile (FileName);
        RedrawTitle = TRUE;
        /* DJ $ */
        return (TRUE);
      }
    /* tran 22.07.2000 $ */
    /* $ 15.07.2000 tran
       + CtrlB switch KeyBar*/
    case KEY_CTRLB:
      Opt.ViOpt.ShowKeyBar=!Opt.ViOpt.ShowKeyBar;
      if ( Opt.ViOpt.ShowKeyBar )
        ViewKeyBar.Show();
      else
        ViewKeyBar.Hide0(); // 0 mean - Don't purge saved screen
      Show();
      /* $ 07.05.2001 DJ */
      KeyBarVisible = Opt.ViOpt.ShowKeyBar;
      /* DJ $ */
      return (TRUE);
    /* tran 15.07.2000 $ */

    case KEY_CTRLSHIFTB:
    {
      Opt.ViOpt.ShowTitleBar=!Opt.ViOpt.ShowTitleBar;
      TitleBarVisible = Opt.ViOpt.ShowTitleBar;
      Show();
      return (TRUE);
    }

    /* $ 24.08.2000 SVS
       + Добавляем реакцию показа бакграунда на клавишу CtrlAltShift
    */
/* $ KEY_CTRLALTSHIFTPRESS унесено в manager OT */
    case KEY_CTRLO:
      if(!Opt.OnlyEditorViewerUsed)
      {
        if(FrameManager->ShowBackground())
        {
          SetCursorType(FALSE,0);
          WaitKey();
          FrameManager->RefreshFrame();
        }
      }
      return(TRUE);
    /* SVS $ */
    case KEY_F3:
    case KEY_NUMPAD5:  case KEY_SHIFTNUMPAD5:
      if (F3KeyOnly)
        return(TRUE);
    case KEY_ESC:
    case KEY_F10:
      FrameManager->DeleteFrame();
      return(TRUE);
    case KEY_F6:
      if (!DisableEdit)
      {
        char ViewFileName[NM];
        View.GetFileName(ViewFileName);
/*
        HANDLE hEdit=FAR_CreateFile(ViewFileName,GENERIC_READ,FILE_SHARE_READ,NULL,
                                    OPEN_EXISTING,
                                    FILE_FLAG_SEQUENTIAL_SCAN|(WinVer.dwPlatformId==VER_PLATFORM_WIN32_NT?FILE_FLAG_POSIX_SEMANTICS:0),
                                    NULL);
*/
        HANDLE hEdit=INVALID_HANDLE_VALUE;
        if(WinVer.dwPlatformId==VER_PLATFORM_WIN32_NT)
          hEdit=FAR_CreateFile(ViewFileName,GENERIC_READ,FILE_SHARE_READ,NULL,
            OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN|FILE_FLAG_POSIX_SEMANTICS,
            NULL);
        if(hEdit==INVALID_HANDLE_VALUE)
          hEdit=FAR_CreateFile(ViewFileName,GENERIC_READ,FILE_SHARE_READ,NULL,
            OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN, NULL);

        if (hEdit==INVALID_HANDLE_VALUE)
        {
          Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MEditTitle),MSG(MEditCannotOpen),ViewFileName,MSG(MOk));
          return(TRUE);
        }
        CloseHandle(hEdit);

        /* $ 11.10.2001 IS
            Если переключаемся в редактор, то удалять файл уже не
            нужно
        */
        SetTempViewName("");
        /* IS $ */
        SetExitCode(0);
        __int64 FilePos=View.GetFilePos();
        /* $ 06.05.2001 DJ обработка F6 под NWZ */
        /* $ 04.07.2006 IS
           Тут косяк, замеченный при чтении warnings - FilePos теряет информацию при преобразовании __int64 -> int
           Надо бы поправить FileEditor на этот счет.
        */
        FileEditor *ShellEditor = new FileEditor (ViewFileName, (GetCanLoseFocus()?FFILEEDIT_ENABLEF6:0)|(SaveToSaveAs?FFILEEDIT_SAVETOSAVEAS:0),-2, static_cast<int>(FilePos), NULL);
        /* IS $ */
        ShellEditor->SetEnableF6 (TRUE);
        /* $ 07.05.2001 DJ сохраняем NamesList */
        ShellEditor->SetNamesList (View.GetNamesList());
        /* DJ $ */
        /* DJ $ */
        FrameManager->DeleteFrame(this); // Insert уже есть внутри конструктора
        ShowTime(2);
      }
      return(TRUE);

    /* $ 27.09.2000 SVS
       + Печать файла с использованием плагина PrintMan
    */
    case KEY_ALTF5:
    {
      if(Opt.UsePrintManager)
        CtrlObject->Plugins.CallPlugin(CtrlObject->Plugins.FindPlugin(SYSID_PRINTMANAGER),OPEN_VIEWER); // printman

      return TRUE;
    }
    /* SVS $*/

    /* $ 19.12.2000 SVS
       Вызов диалога настроек (с подачи IS)
    */
    case KEY_ALTSHIFTF9:
      /* $ 29.03.2001 IS
           Работа с локальной копией ViewerOptions
      */
      struct ViewerOptions ViOpt;

      ViOpt.TabSize=View.GetTabSize();
      ViOpt.AutoDetectTable=View.GetAutoDetectTable();
      ViOpt.ShowScrollbar=View.GetShowScrollbar();
      ViOpt.ShowArrows=View.GetShowArrows();
      ViOpt.PersistentBlocks=View.GetPersistentBlocks();

      /* $ 27.11.2001 DJ
         Local в ViewerConfig
      */
      ViewerConfig(ViOpt,1);
      /* DJ $ */

      View.SetTabSize(ViOpt.TabSize);
      View.SetAutoDetectTable(ViOpt.AutoDetectTable);
      View.SetShowScrollbar(ViOpt.ShowScrollbar);
      View.SetShowArrows(ViOpt.ShowArrows);
      View.SetPersistentBlocks(ViOpt.PersistentBlocks);
      /* IS $ */
      if ( Opt.ViOpt.ShowKeyBar )
        ViewKeyBar.Show();
      View.Show();
      return TRUE;
    /* SVS $ */

    /* $ 10.05.2001 DJ
       Alt-F11 - show view/edit history
    */
    case KEY_ALTF11:
      if (GetCanLoseFocus())
        CtrlObject->CmdLine->ShowViewEditHistory();
      return TRUE;
    /* DJ $ */

    default:
//      Этот кусок - на будущее (по аналогии с редактором :-)
//      if (CtrlObject->Macro.IsExecuting() || !View.ProcessViewerInput(&ReadRec))
      {
        /* $ 22.03.2001 SVS
           Это помогло от залипания :-)
        */
        if (!CtrlObject->Macro.IsExecuting())
          if ( Opt.ViOpt.ShowKeyBar )
              ViewKeyBar.Show();
        /* SVS $ */
        if (!ViewKeyBar.ProcessKey(Key))
          return(View.ProcessKey(Key));
      }
      return(TRUE);
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
  if ( Type ) strcpy(Type,MSG(MScreensView));
  if ( Name ) View.GetFileName(Name);
  return(MODALTYPE_VIEWER);
}


void FileViewer::ShowConsoleTitle()
{
  View.ShowConsoleTitle();
  RedrawTitle = FALSE;
}


void FileViewer::SetTempViewName(const char *Name, BOOL DeleteFolder)
{
  View.SetTempViewName(Name, DeleteFolder);
}


FileViewer::~FileViewer()
{
  _OT(SysLog("[%p] ~FileViewer::FileViewer()",this));
}

void FileViewer::OnDestroy()
{
  _OT(SysLog("[%p] FileViewer::OnDestroy()",this));
  if (!DisableHistory && (CtrlObject->Cp()->ActivePanel!=NULL || strcmp(Name,"-")!=0))
  {
    char FullFileName[NM];
    View.GetFileName(FullFileName);
    CtrlObject->ViewHistory->AddToHistory(FullFileName,MSG(MHistoryView),0);
  }
}

int FileViewer::FastHide()
{
  return Opt.AllCtrlAltShiftRule & CASR_VIEWER;
}

int FileViewer::ViewerControl(int Command,void *Param)
{
  _VCTLLOG(CleverSysLog SL("FileViewer::ViewerControl()"));
  _VCTLLOG(SysLog("(Command=%s, Param=[%d/0x%08X])",_VCTL_ToName(Command),(int)Param,Param));
  return View.ViewerControl(Command,Param);
}

const char *FileViewer::GetTitle(char *Title,int LenTitle,int TruncSize)
{
  return View.GetTitle(Title,LenTitle,TruncSize);
}

__int64 FileViewer::GetViewFileSize() const
{
  return View.GetViewFileSize();
}

__int64 FileViewer::GetViewFilePos() const
{
  return View.GetViewFilePos();
}

void FileViewer::ShowStatus()
{
 _SVS(SysLog("FileViewer::ShowStatus()"));
  char Status[4096],Name[4096];
 _SVS(SysLog("IsTitleBarVisible()=%d",IsTitleBarVisible()));
  if (!IsTitleBarVisible())
    return;
  /* $ 22.06.2000 IS
    Показывать полное имя файла во вьюере
    Was: strcpy(Name,*Title ? Title:FileName);
  */
  GetTitle(Name,sizeof(Name)-1,0);
  /* IS $  */
  int NameLength=ScrX-43; //???41
  if (Opt.ViewerEditorClock && IsFullScreen())
    NameLength-=6;
  if (NameLength<20)
    NameLength=20;
  /* $ 01.10.2000 IS
     ! Показывать букву диска в статусной строке
  */
  TruncPathStr(Name,NameLength);
  /* IS $ */
  char *TableName;
  char TmpTableName[32];
  if (View.VM.Unicode)
    TableName="Unicode";
  else if (View.VM.UseDecodeTable)
  {
    xstrncpy(TmpTableName,View.TableSet.TableName,sizeof(TmpTableName));
    TableName=RemoveChar(TmpTableName,'&',TRUE);
  }
  else if (View.VM.AnsiMode)
    TableName="Win";
  else
    TableName="DOS";

  const char *StatusFormat="%-*s %10.10s %13I64u %7.7s %-4I64d %s%3d%%";
  sprintf(Status,StatusFormat,
          NameLength,Name,TableName,
          View.FileSize,MSG(MViewerStatusCol),View.LeftPos,
          Opt.ViewerEditorClock ? "":" ",
          (View.LastPage ? 100:ToPercent64(View.FilePos,View.FileSize)));
  SetColor(COL_VIEWERSTATUS);
  GotoXY(X1,Y1);
  /* $ 31.08.2000 SVS
     Бага - без часиков неверно отображается верхний статус
  */
  mprintf("%-*.*s",View.Width+(View.ViOpt.ShowScrollbar?1:0),
                   View.Width+(View.ViOpt.ShowScrollbar?1:0),Status);
  /* SVS $ */
  if (Opt.ViewerEditorClock && IsFullScreen())
    ShowTime(FALSE);
}

void FileViewer::OnChangeFocus (int focus)
{
  Frame::OnChangeFocus (focus);
  CtrlObject->Plugins.CurViewer=&View;
  int FCurViewerID=View.ViewerID;
  CtrlObject->Plugins.ProcessViewerEvent(focus?VE_GOTFOCUS:VE_KILLFOCUS,&FCurViewerID);
}
