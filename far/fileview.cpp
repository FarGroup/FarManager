/*
fileview.cpp

Просмотр файла - надстройка над viewer.cpp

*/

/* Revision: 1.37 14.06.2001 $ */

/*
Modify:
  14.06.2001 OT
    ! "Бунт" ;-)
  06.06.2001 OT
    ! отменен OnChangeFocus за отсутствием состава ... необходимости :)
    + добавлен деструктор ~FileViewer()... с косметическими целями
  05.06.2001 tran
    + класс FileView - добавлен OnChangeFocus
  27.05.2001 DJ
    - Не делаем DeleteFrame() в случае ошибки открытия
  26.05.2001 OT
    - Вьюер возможно запускать в модальном режиме
  20.05.2001 DJ
    - починим макросы
  15.05.2001 OT
    ! NWZ -> NFZ
  14.05.2001 OT
    ! Изменение порядка вызова параметров ReplaceFrame (для единообразия и удобства)
  12.05.2001 DJ
    ! отрисовка по OnChangeFocus перенесена в Frame
    ! убран дублирующийся ExitCode
  11.05.2001 OT
    ! Отрисовка Background
  10.05.2001 DJ
    + Alt-F11 - view/edit history
    + Ctrl-F10 всегда переключается на панели
  07.05.2001 SVS
    ! SysLog(); -> _D(SysLog());
  07.05.2001 DJ
    - кейбар не обновлялся
  06.05.2001 DJ
    ! перетрях #include
    + обработка F6 под NWZ
  06.05.2001 ОТ
    ! Переименование Window в Frame :)
  05.05.2001 DJ
    + перетрях NWZ
  29.04.2001 ОТ
    + Внедрение NWZ от Третьякова
  28.04.2001 VVM
    + KeyBar тоже умеет обрабатывать клавиши.
  10.04.2001 IS
    ! Не делаем SetCurDir при ctrl-f10, если нужный путь уже есть на открытых
      панелях, тем самым добиваемся того, что выделение с элементов
      панелей не сбрасывается.
  29.03.2001 IS
    + Работа с локальной копией ViewerOptions при KEY_ALTSHIFTF9
  22.03.2001 SVS
    - "Залипание" кейбара после исполнения макроса
  03.01.2001 SVS
    ! для KEY_ALTSHIFTF9 забыли сделать Show()
  19.12.2000 SVS
    + Alt-Shift-F9 - Вызов диалога настроек (с подачи IS)
    - [*] Забыли "застолбить" место в LNG-файлах под клавишу F9 :-)
      застолбить -застолбили, но не показывает.
  16.12.2000 tran 1.14
    ! Ctrl-F10 смотрит на пассивную панель
  03.11.2000 OT
    ! Введение проверки возвращаемого значения
  02.11.2000 OT
    ! Введение проверки на длину буфера, отведенного под имя файла.
  27.09.2000 SVS
    + Печать файла с использованием плагина PrintMan
    ! Ctrl-Alt-Shift - реагируем, если надо.
  15.09.2000 tran 1.09
    - FKL bug
  14.09.2000 SVS
    - Bug #NN1 - Непонятки  поведением KeyBar (см. описание к Patch#191)
  24.08.2000 SVS
    + Добавляем реакцию показа бакграунда на клавишу CtrlAltShift
  07.08.2000 SVS
    + добавил названия расширенных функциональных клавиш
  22.07.2000 tran 1.06
    + Ctrl-F10 выходит с установкой на файл на текущей панели
  21.07.2000 tran 1.05
      - артефакт при CtrlO при выключенном кейбаре
  15.07.2000 tran
      + CtrlB выключает/включает keybar
  04.07.2000 tran
    + не показывать мессаг бакс при невозвожности открыть файл
  29.06.2000 tran
    + названия всех функциональных клавиш
  28.06.2000 tran
    - NT Console resize
      adding SetScreenPosition
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
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

FileViewer::FileViewer(char *Name,int EnableSwitch,int DisableHistory,
                       int DisableEdit,long ViewStartPos,char *PluginData,
                       NamesList *ViewNamesList)
{
  _OT(SysLog("[%p] FileViewer::FileViewer(I variant...)", this));
  FileViewer::DisableEdit=DisableEdit;
  SetPosition(0,0,ScrX,ScrY);
  FullScreen=TRUE;
  Init(Name,EnableSwitch,DisableHistory,ViewStartPos,PluginData,ViewNamesList);
}


FileViewer::FileViewer(char *Name,int EnableSwitch,char *Title,
                       int X1,int Y1,int X2,int Y2)
{
  _OT(SysLog("[%p] FileViewer::FileViewer(II variant...)", this));
  DisableEdit=TRUE;
  SetPosition(X1,Y1,X2,Y2);
  FullScreen=(X1==0 && Y1==0 && X2==ScrX && Y2==ScrY);
  View.SetTitle(Title);
  Init(Name,EnableSwitch,TRUE,-1,"",NULL);
}


void FileViewer::Init(char *name,int EnableSwitch,int disableHistory,  ///
                      long ViewStartPos,char *PluginData,
                      NamesList *ViewNamesList)
{
  ViewKeyBar.SetOwner(this);
  ViewKeyBar.SetPosition(X1,Y2,X2,Y2);
  /* $ 07.05.2001 DJ */
  KeyBarVisible = Opt.ShowKeyBarViewer;
  /* DJ $ */
  /* $ 20.05.2001 DJ */
  MacroMode = MACRO_VIEWER;
  /* DJ $ */
  View.SetPluginData(PluginData);
  /* $ 07.08.2000 SVS
  */
  View.SetHostFileViewer(this);
  /* SVS $ */

  DisableHistory=disableHistory; ///
  strcpy(Name,name); ///
  SetCanLoseFocus(EnableSwitch);

  /* $ 07.08.2000 SVS
    ! Код, касаемый KeyBar вынесен в отдельную функцию */
  InitKeyBar();
  /* SVS $*/

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
  /* $ 29.06.2000 tran
     добавил названия всех функциональных клавиш */
  char *FViewKeys[]={MSG(MViewF1),MSG(MViewF2),MSG(MViewF3),MSG(MViewF4),MSG(MViewF5),DisableEdit ? "":MSG(MViewF6),MSG(MViewF7),MSG(MViewF8),MSG(MViewF9),MSG(MViewF10),MSG(MViewF11),MSG(MViewF12)};
  char *FViewShiftKeys[]={MSG(MViewShiftF1),MSG(MViewShiftF2),MSG(MViewShiftF3),MSG(MViewShiftF4),MSG(MViewShiftF5),MSG(MViewShiftF6),MSG(MViewShiftF7),MSG(MViewShiftF8),MSG(MViewShiftF9),MSG(MViewShiftF10),MSG(MViewShiftF11),MSG(MViewShiftF12)};
  char *FViewAltKeys[]={MSG(MViewAltF1),MSG(MViewAltF2),MSG(MViewAltF3),MSG(MViewAltF4),MSG(MViewAltF5),MSG(MViewAltF6),MSG(MViewAltF7),MSG(MViewAltF8),MSG(MViewAltF9),MSG(MViewAltF10),MSG(MViewAltF11),MSG(MViewAltF12)};
  char *FViewCtrlKeys[]={MSG(MViewCtrlF1),MSG(MViewCtrlF2),MSG(MViewCtrlF3),MSG(MViewCtrlF4),MSG(MViewCtrlF5),MSG(MViewCtrlF6),MSG(MViewCtrlF7),MSG(MViewCtrlF8),MSG(MViewCtrlF9),MSG(MViewCtrlF10),MSG(MViewCtrlF11),MSG(MViewCtrlF12)};

  if(CtrlObject->Plugins.FindPlugin(SYSID_PRINTMANAGER) == -1)
    FViewAltKeys[5-1]="";

  /* $ 07.08.2000 SVS
     добавил названия расширенных функциональных клавиш */
  char *FViewAltShiftKeys[]={MSG(MViewAltShiftF1),MSG(MViewAltShiftF2),MSG(MViewAltShiftF3),MSG(MViewAltShiftF4),MSG(MViewAltShiftF5),MSG(MViewAltShiftF6),MSG(MViewAltShiftF7),MSG(MViewAltShiftF8),MSG(MViewAltShiftF9),MSG(MViewAltShiftF10),MSG(MViewAltShiftF11),MSG(MViewAltShiftF12)};
  char *FViewCtrlShiftKeys[]={MSG(MViewCtrlShiftF1),MSG(MViewCtrlShiftF2),MSG(MViewCtrlShiftF3),MSG(MViewCtrlShiftF4),MSG(MViewCtrlShiftF5),MSG(MViewCtrlShiftF6),MSG(MViewCtrlShiftF7),MSG(MViewCtrlShiftF8),MSG(MViewCtrlShiftF9),MSG(MViewCtrlShiftF10),MSG(MViewCtrlShiftF11),MSG(MViewCtrlShiftF12)};
  char *FViewCtrlAltKeys[]={MSG(MViewCtrlAltF1),MSG(MViewCtrlAltF2),MSG(MViewCtrlAltF3),MSG(MViewCtrlAltF4),MSG(MViewCtrlAltF5),MSG(MViewCtrlAltF6),MSG(MViewCtrlAltF7),MSG(MViewCtrlAltF8),MSG(MViewCtrlAltF9),MSG(MViewCtrlAltF10),MSG(MViewCtrlAltF11),MSG(MViewCtrlAltF12)};
  /* SVS $*/

  ViewKeyBar.Set(FViewKeys,sizeof(FViewKeys)/sizeof(FViewKeys[0]));
  ViewKeyBar.SetShift(FViewShiftKeys,sizeof(FViewShiftKeys)/sizeof(FViewShiftKeys[0]));
  ViewKeyBar.SetAlt(FViewAltKeys,sizeof(FViewAltKeys)/sizeof(FViewAltKeys[0]));
  ViewKeyBar.SetCtrl(FViewCtrlKeys,sizeof(FViewCtrlKeys)/sizeof(FViewCtrlKeys[0]));
  /* $ 07.08.2000 SVS
     добавил названия расширенных функциональных клавиш */
  ViewKeyBar.SetCtrlAlt(FViewCtrlAltKeys,sizeof(FViewCtrlAltKeys)/sizeof(FViewCtrlAltKeys[0]));
  ViewKeyBar.SetCtrlShift(FViewCtrlShiftKeys,sizeof(FViewCtrlShiftKeys)/sizeof(FViewCtrlShiftKeys[0]));
  ViewKeyBar.SetAltShift(FViewAltShiftKeys,sizeof(FViewAltShiftKeys)/sizeof(FViewAltShiftKeys[0]));
  /* SVS $ */
  /* tran $ */

  SetKeyBar(&ViewKeyBar);
  /* $ 15.07.2000 tran
     ShowKeyBarViewer support*/
  View.SetPosition(X1,Y1,X2,Y2-(Opt.ShowKeyBarViewer?1:0));
  /* tran 15.07.2000 $ */

  View.SetViewKeyBar(&ViewKeyBar);
}
/* SVS $ */

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
    /* $ 22.07.2000 tran
       + выход по ctrl-f10 с установкой курсора на файл */
    case KEY_CTRLF10:
      {
        if(GetCanLoseFocus())
        {
          char DirTmp[NM],ADir[NM],PDir[NM],*NameTmp,FileName[NM];
          View.GetFileName(FileName);
          ProcessKey(KEY_F10);
          if(strchr(FileName,'\\') || strchr(FileName,'/'))
          {
            strncpy(DirTmp,FileName,NM);
            NameTmp=PointToName(DirTmp);
            if(NameTmp>DirTmp)NameTmp[-1]=0;
            CtrlObject->Cp()->GetAnotherPanel(CtrlObject->Cp()->ActivePanel)->GetCurDir(PDir);
            CtrlObject->Cp()->ActivePanel->GetCurDir(ADir);
            /* $ 10.04.2001 IS
                 Не делаем SetCurDir, если нужный путь уже есть на открытых
                 панелях, тем самым добиваемся того, что выделение с элементов
                 панелей не сбрасывается.
            */
            BOOL AExist=LocalStricmp(ADir,DirTmp)==0,
                 PExist=LocalStricmp(PDir,DirTmp)==0;
            // если нужный путь есть на пассивной панели
            if ( !AExist && PExist)
            {
                CtrlObject->Cp()->ProcessKey(KEY_TAB);
            }
            if(!AExist && !PExist)
                CtrlObject->Cp()->ActivePanel->SetCurDir(DirTmp,TRUE);
            /* IS */
            CtrlObject->Cp()->ActivePanel->GoToFile(NameTmp);
            /* $ 10.05.2001 DJ
               переключаемся на панели
            */
            FrameManager->SwitchToPanels();
            /* DJ $ */
          }
          /*else
          {
            CtrlObject->Cp()->ActivePanel->SetCurDir(StartDir,TRUE);
            CtrlObject->Cp()->ActivePanel->GoToFile(FileName);
          } */
        }
        return (TRUE);
      }
    /* tran 22.07.2000 $ */
    /* $ 15.07.2000 tran
       + CtrlB switch KeyBar*/
    case KEY_CTRLB:
      Opt.ShowKeyBarViewer=!Opt.ShowKeyBarViewer;
      if ( Opt.ShowKeyBarViewer )
        ViewKeyBar.Show();
      else
        ViewKeyBar.Hide0(); // 0 mean - Don't purge saved screen
      Show();
      /* $ 07.05.2001 DJ */
      KeyBarVisible = Opt.ShowKeyBarViewer;
      /* DJ $ */
      return (TRUE);
    /* tran 15.07.2000 $ */
    /* $ 24.08.2000 SVS
       + Добавляем реакцию показа бакграунда на клавишу CtrlAltShift
    */
    case KEY_CTRLALTSHIFTPRESS:
      if(!(Opt.AllCtrlAltShiftRule & CASR_VIEWER))
        return TRUE;
    case KEY_CTRLO:
      FrameManager->ShowBackground();
      WaitKey(Key==KEY_CTRLALTSHIFTPRESS?KEY_CTRLALTSHIFTRELEASE:-1);
        Show();
      return(TRUE);
    /* SVS $ */
    case KEY_F3:
    case KEY_NUMPAD5:
      if (F3KeyOnly)
        return(TRUE);
    case KEY_ESC:
    case KEY_F10:
      FrameManager->DeleteFrame();
      return(TRUE);
    case KEY_F6:
      if (!DisableEdit)
      {
        SetExitCode(0);
        char ViewFileName[NM];
        View.GetFileName(ViewFileName);
        long FilePos=View.GetFilePos();
        /* $ 06.05.2001 DJ обработка F6 под NWZ */
        FileEditor *ShellEditor = new FileEditor (ViewFileName, FALSE, GetCanLoseFocus(),
          -2, FilePos, FALSE);
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
      if(CtrlObject->Plugins.FindPlugin(SYSID_PRINTMANAGER) != -1)
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

      ViewerConfig(ViOpt);

      View.SetTabSize(ViOpt.TabSize);
      View.SetAutoDetectTable(ViOpt.AutoDetectTable);
      View.SetShowScrollbar(ViOpt.ShowScrollbar);
      View.SetShowArrows(ViOpt.ShowArrows);
      /* IS $ */
      if ( Opt.ShowKeyBarViewer )
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
          if ( Opt.ShowKeyBarViewer )
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
}


void FileViewer::SetTempViewName(char *Name)
{
  View.SetTempViewName(Name);
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
    if (ConvertNameToFull(Name,FullFileName, sizeof(FullFileName)) >= sizeof(FullFileName))
      return ;
    CtrlObject->ViewHistory->AddToHistory(FullFileName,MSG(MHistoryView),0);
  }
}
