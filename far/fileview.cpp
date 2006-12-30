/*
fileview.cpp

Просмотр файла - надстройка над viewer.cpp

*/

/* Revision: 1.87 07.07.2006 $ */

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

FileViewer::FileViewer(const wchar_t *Name,int EnableSwitch,int DisableHistory,
                       int DisableEdit,long ViewStartPos,const wchar_t *PluginData,
                       NamesList *ViewNamesList,int ToSaveAs)
{
  _OT(SysLog("[%p] FileViewer::FileViewer(I variant...)", this));
  FileViewer::DisableEdit=DisableEdit;
  SetPosition(0,0,ScrX,ScrY);
  FullScreen=TRUE;
  Init(Name,EnableSwitch,DisableHistory,ViewStartPos,PluginData,ViewNamesList,ToSaveAs);
}


FileViewer::FileViewer(const wchar_t *Name,int EnableSwitch,int DisableHistory,
                       const wchar_t *Title, int X1,int Y1,int X2,int Y2)
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
  Init(Name,EnableSwitch,DisableHistory,-1,L"",NULL,FALSE);
}


void FileViewer::Init(const wchar_t *name,int EnableSwitch,int disableHistory, ///
                      long ViewStartPos,const wchar_t *PluginData,
                      NamesList *ViewNamesList,int ToSaveAs)
{
  RedrawTitle = FALSE;
  ViewKeyBar.SetOwner(this);
  ViewKeyBar.SetPosition(X1,Y2,X2,Y2);
  KeyBarVisible = Opt.ViOpt.ShowKeyBar;

  int OldMacroMode=CtrlObject->Macro.GetMode();
  MacroMode = MACRO_VIEWER;
  CtrlObject->Macro.SetMode(MACRO_VIEWER);

  View.SetPluginData(PluginData);
  View.SetHostFileViewer(this);

  DisableHistory=disableHistory; ///

  strName = name;

  SetCanLoseFocus(EnableSwitch);

  /* $ 17.08.2001 KM
    Добавлено для поиска по AltF7. При редактировании найденного файла из
    архива для клавиши F2 сделать вызов ShiftF2.
  */
  SaveToSaveAs=ToSaveAs;
  /* KM $ */

  InitKeyBar();

  if (!View.OpenFile(strName,TRUE)) // $ 04.07.2000 tran + add TRUE as 'warning' parameter
  {
    DisableHistory = TRUE;  // $ 26.03.2002 DJ - при неудаче открытия - не пишем мусор в историю
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


  ShowConsoleTitle();

  F3KeyOnly=TRUE;
  if (EnableSwitch) {
    FrameManager->InsertFrame(this);
  } else {
    FrameManager->ExecuteFrame(this);
  }
}


/* $ 07.08.2000 SVS
  Функция инициализации KeyBar Labels
*/
void FileViewer::InitKeyBar(void)
{
  int IKeyLabel[2][7][13]=
  {
    // Обычный редактор
    {
      /* (empty)   */ {KBL_MAIN,MViewF1,MViewF2,MViewF3,MViewF4,MViewF5,MViewF6,MViewF7,MViewF8,MViewF9,MViewF10,MViewF11,MViewF12},
      /* Shift     */ {KBL_SHIFT,MViewShiftF1,MViewShiftF2,MViewShiftF3,MViewShiftF4,MViewShiftF5,MViewShiftF6,MViewShiftF7,MViewShiftF8,MViewShiftF9,MViewShiftF10,MViewShiftF11,MViewShiftF12},
      /* Alt       */ {KBL_ALT,MViewAltF1,MViewAltF2,MViewAltF3,MViewAltF4,MViewAltF5,MViewAltF6,MViewAltF7,MViewAltF8,MViewAltF9,MViewAltF10,MViewAltF11,MViewAltF12},
      /* Ctrl      */ {KBL_CTRL,MViewCtrlF1,MViewCtrlF2,MViewCtrlF3,MViewCtrlF4,MViewCtrlF5,MViewCtrlF6,MViewCtrlF7,MViewCtrlF8,MViewCtrlF9,MViewCtrlF10,MViewCtrlF11,MViewCtrlF12},
      /* AltShift  */ {KBL_ALTSHIFT,MViewAltShiftF1,MViewAltShiftF2,MViewAltShiftF3,MViewAltShiftF4,MViewAltShiftF5,MViewAltShiftF6,MViewAltShiftF7,MViewAltShiftF8,MViewAltShiftF9,MViewAltShiftF10,MViewAltShiftF11,MViewAltShiftF12},
      /* CtrlShift */ {KBL_CTRLSHIFT,MViewCtrlShiftF1,MViewCtrlShiftF2,MViewCtrlShiftF3,MViewCtrlShiftF4,MViewCtrlShiftF5,MViewCtrlShiftF6,MViewCtrlShiftF7,MViewCtrlShiftF8,MViewCtrlShiftF9,MViewCtrlShiftF10,MViewCtrlShiftF11,MViewCtrlShiftF12},
      /* CtrlAlt   */ {KBL_CTRLALT,MViewCtrlAltF1,MViewCtrlAltF2,MViewCtrlAltF3,MViewCtrlAltF4,MViewCtrlAltF5,MViewCtrlAltF6,MViewCtrlAltF7,MViewCtrlAltF8,MViewCtrlAltF9,MViewCtrlAltF10,MViewCtrlAltF11,MViewCtrlAltF12},
    },
    // одиночный редактор
    {
      /* (empty)   */ {KBL_MAIN,MSingleViewF1,MSingleViewF2,MSingleViewF3,MSingleViewF4,MSingleViewF5,MSingleViewF6,MSingleViewF7,MSingleViewF8,MSingleViewF9,MSingleViewF10,MSingleViewF11,MSingleViewF12},
      /* Shift     */ {KBL_SHIFT,MSingleViewShiftF1,MSingleViewShiftF2,MSingleViewShiftF3,MSingleViewShiftF4,MSingleViewShiftF5,MSingleViewShiftF6,MSingleViewShiftF7,MSingleViewShiftF8,MSingleViewShiftF9,MSingleViewShiftF10,MSingleViewShiftF11,MSingleViewShiftF12},
      /* Alt       */ {KBL_ALT,MSingleViewAltF1,MSingleViewAltF2,MSingleViewAltF3,MSingleViewAltF4,MSingleViewAltF5,MSingleViewAltF6,MSingleViewAltF7,MSingleViewAltF8,MSingleViewAltF9,MSingleViewAltF10,MSingleViewAltF11,MSingleViewAltF12},
      /* Ctrl      */ {KBL_CTRL,MSingleViewCtrlF1,MSingleViewCtrlF2,MSingleViewCtrlF3,MSingleViewCtrlF4,MSingleViewCtrlF5,MSingleViewCtrlF6,MSingleViewCtrlF7,MSingleViewCtrlF8,MSingleViewCtrlF9,MSingleViewCtrlF10,MSingleViewCtrlF11,MSingleViewCtrlF12},
      /* AltShift  */ {KBL_ALTSHIFT,MSingleViewAltShiftF1,MSingleViewAltShiftF2,MSingleViewAltShiftF3,MSingleViewAltShiftF4,MSingleViewAltShiftF5,MSingleViewAltShiftF6,MSingleViewAltShiftF7,MSingleViewAltShiftF8,MSingleViewAltShiftF9,MSingleViewAltShiftF10,MSingleViewAltShiftF11,MSingleViewAltShiftF12},
      /* CtrlShift */ {KBL_CTRLSHIFT,MSingleViewCtrlShiftF1,MSingleViewCtrlShiftF2,MSingleViewCtrlShiftF3,MSingleViewCtrlShiftF4,MSingleViewCtrlShiftF5,MSingleViewCtrlShiftF6,MSingleViewCtrlShiftF7,MSingleViewCtrlShiftF8,MSingleViewCtrlShiftF9,MSingleViewCtrlShiftF10,MSingleViewCtrlShiftF11,MSingleViewCtrlShiftF12},
      /* CtrlAlt   */ {KBL_CTRLALT,MSingleViewCtrlAltF1,MSingleViewCtrlAltF2,MSingleViewCtrlAltF3,MSingleViewCtrlAltF4,MSingleViewCtrlAltF5,MSingleViewCtrlAltF6,MSingleViewCtrlAltF7,MSingleViewCtrlAltF8,MSingleViewCtrlAltF9,MSingleViewCtrlAltF10,MSingleViewCtrlAltF11,MSingleViewCtrlAltF12},
    }
  };
  wchar_t *FViewKeys[12];
  int I,J;

  for(I=0; I < 7; ++I)
  {
    for(J=1; J <= 12; ++J)
    {
      FViewKeys[J-1]=UMSG(IKeyLabel[Opt.OnlyEditorViewerUsed][I][J]);
    }
    switch(IKeyLabel[Opt.OnlyEditorViewerUsed][I][0])
    {
      case KBL_MAIN:
        if(DisableEdit)
          FViewKeys[6-1]=L"";
        if(!GetCanLoseFocus())
          FViewKeys[12-1]=L"";
        break;
      case KBL_ALT:
        // $ 17.12.2001 KM  - Если !GetCanLoseFocus() тогда на Alt-F11 рисуем пустую строку.
        if(!GetCanLoseFocus())
          FViewKeys[11-1]=L"";
        if(!Opt.UsePrintManager || CtrlObject->Plugins.FindPlugin(SYSID_PRINTMANAGER) == -1)
          FViewKeys[5-1]=L"";
        break;
    }
    ViewKeyBar.SetGroup(IKeyLabel[Opt.OnlyEditorViewerUsed][I][0],FViewKeys,sizeof(FViewKeys)/sizeof(FViewKeys[0]));
  }

  SetKeyBar(&ViewKeyBar);
  // $ 15.07.2000 tran - ShowKeyBarViewer support
  View.SetPosition(X1,Y1,X2,Y2-(Opt.ViOpt.ShowKeyBar?1:0));
  View.SetViewKeyBar(&ViewKeyBar);
}
/* SVS $ */

void FileViewer::Show()
{
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
    View.SetPosition(0,0,ScrX,ScrY-(Opt.ViOpt.ShowKeyBar?1:0));
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
  if (RedrawTitle && ((Key & 0x00ffffff) < KEY_END_FKEY))
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
        string strFileName;

        View.GetFileName(strFileName);

        CtrlObject->Cp()->GoToFileW (strFileName);

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
        string strViewFileName;

        View.GetFileName(strViewFileName);

        HANDLE hEdit=INVALID_HANDLE_VALUE;
        if(WinVer.dwPlatformId==VER_PLATFORM_WIN32_NT)
          hEdit=FAR_CreateFileW(strViewFileName,GENERIC_READ,FILE_SHARE_READ,NULL,
            OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN|FILE_FLAG_POSIX_SEMANTICS,
            NULL);
        if(hEdit==INVALID_HANDLE_VALUE)
          hEdit=FAR_CreateFileW(strViewFileName,GENERIC_READ,FILE_SHARE_READ,NULL,
            OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN, NULL);

        if (hEdit==INVALID_HANDLE_VALUE)
        {
          MessageW(MSG_WARNING|MSG_ERRORTYPE,1,UMSG(MEditTitle),UMSG(MEditCannotOpen),strViewFileName,UMSG(MOk));
          return(TRUE);
        }
        CloseHandle(hEdit);

        /* $ 11.10.2001 IS
            Если переключаемся в редактор, то удалять файл уже не
            нужно
        */
        SetTempViewName(L"");
        /* IS $ */
        SetExitCode(0);
        __int64 FilePos=View.GetFilePos();
        /* $ 06.05.2001 DJ обработка F6 под NWZ */

        /* $ 07.07.2006 IS
           Тут косяк, замеченный при чтении warnings - FilePos теряет информацию при преобразовании __int64 -> int
           Надо бы поправить FileEditor на этот счет.
        */
        FileEditor *ShellEditor = new FileEditor (strViewFileName, -1, FALSE, GetCanLoseFocus(),
          -2, static_cast<int>(FilePos), FALSE, NULL, SaveToSaveAs);
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
      if(Opt.UsePrintManager && CtrlObject->Plugins.FindPlugin(SYSID_PRINTMANAGER) != -1)
        CtrlObject->Plugins.CallPlugin(SYSID_PRINTMANAGER,OPEN_VIEWER,0); // printman
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


int FileViewer::GetTypeAndName(string &strType, string &strName)
{
   strType = UMSG(MScreensView);
   View.GetFileName (strName);

   return(MODALTYPE_VIEWER);
}


void FileViewer::ShowConsoleTitle()
{
  View.ShowConsoleTitle();
  RedrawTitle = FALSE;
}


void FileViewer::SetTempViewName(const wchar_t *Name, BOOL DeleteFolder)
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
  if (!DisableHistory && (CtrlObject->Cp()->ActivePanel!=NULL || wcscmp (strName, L"-")!=0))
  {
    string strFullFileName;
    View.GetFileName(strFullFileName);

    CtrlObject->ViewHistory->AddToHistory(strFullFileName,UMSG(MHistoryView),0);
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

void FileViewer::GetTitle(string &Title,int LenTitle,int TruncSize)
{
  View.GetTitle(Title,LenTitle,TruncSize);
}

__int64 FileViewer::GetViewFileSize() const
{
  return View.GetViewFileSize();
}

__int64 FileViewer::GetViewFilePos() const
{
  return View.GetViewFilePos();
}
