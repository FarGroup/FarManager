/*
fileview.cpp

Просмотр файла - надстройка над viewer.cpp

*/

/* Revision: 1.62 26.12.2002 $ */

/*
Modify:
    - BugZ#754 - открытие редатора с большИми у2, х2
      Проверим координаты в Init
  23.12.2002 SVS
    + Wish - В LNG-файлах отдельные позиции лейбаков для /e и /v
  10.12.2002 SVS
    - BugZ#720 - far /v file + Ctrl-O
  14.06.2002 IS
    + Дополнительный параметр у SetTempViewName - DeleteFolder
  04.06.2002 SVS
    - BugZ#546 - Editor валит фар (здесь такая же фигня, что и в редакторе!)
  24.05.2002 SVS
    ! Уточнения в FileViewer::ViewerControl для логов
  22.05.2002 SVS
    + ViewerControl()
  13.05.2002 VVM
    + Перерисуем заголовок консоли после позиционирования на файл.
  26.03.2002 DJ
    ! при неудаче открытия - не пишем мусор в историю
  22.03.2002 SVS
    - strcpy - Fuck!
  19.03.2002 SVS
    - BugZ#373 - F3 Ctrl-O - виден курсор
  28.01.2002 OT
    - При неудачном открытии файла не удалялся фрейм
  28.12.2001 DJ
    ! унифицируем обработку Ctrl-F10
  17.12.2001 KM
    ! Если !GetCanLoseFocus() тогда на Alt-F11 рисуем пустую строку.
  08.12.2001 OT
    Bugzilla #144 Заходим в архив, F4 на файле, Ctrl-F10.
  27.11.2001 DJ
    + Local в ViewerConfig
  14.11.2001 SVS
    ! Ctrl-F10 не выходит, а только позиционирует
  02.11.2001 IS
    - отрицательные координаты левого верхнего угла заменяются на нулевые
  12.10.2001 VVM
    ! Неправильно запоминалось имя файла в истории.
  11.10.2001 IS
    ! Если просили удалить файл при закрытии и переключаемся в редактор
      по F6, то удалять файл уже не нужно.
  27.09.2001 IS
    - Левый размер при использовании strncpy
  08.09.2001 IS
    + Дополнительный параметр у второго конструктора: DisableHistory
  17.08.2001 KM
    + Добавлена функция SetSaveToSaveAs для установки дефолтной реакции
      на клавишу F2 в вызов ShiftF2 для поиска, в случае редактирования
      найденного файла из архива.
    ! Изменён конструктор и функция Init для работы SaveToSaveAs.
    - Убрана в KeyBar надпись на клавишу F12 при CanLoseFocus=TRUE
  11.07.2001 OT
    Перенос CtrlAltShift в Manager
  25.06.2001 IS
   ! Внедрение const
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
  strncpy(Name,name,sizeof(Name)-1); ///
  SetCanLoseFocus(EnableSwitch);

  /* $ 17.08.2001 KM
    Добавлено для поиска по AltF7. При редактировании найденного файла из
    архива для клавиши F2 сделать вызов ShiftF2.
  */
  SaveToSaveAs=ToSaveAs;
  /* KM $ */

  /* $ 07.08.2000 SVS
    ! Код, касаемый KeyBar вынесен в отдельную функцию */
  InitKeyBar();
  /* SVS $*/

  /* $ 04.07.2000 tran
     + add TRUE as 'warning' parameter */
  if (!View.OpenFile(Name,TRUE))
  /* tran 04.07.2000 $ */
  {
    /* $ 26.03.2002 DJ
       при неудаче открытия - не пишем мусор в историю
    */
    DisableHistory = TRUE;
    /* DJ $ */
    // FrameManager->DeleteFrame(this); // ЗАЧЕМ? Вьювер то еще не помещен в очередь манагера!
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
  char *FViewKeys[12];
  int I,J;

  for(I=0; I < 7; ++I)
  {
    for(J=1; J <= 12; ++J)
    {
      FViewKeys[J-1]=MSG(IKeyLabel[Opt.OnlyEditorViewerUsed][I][J]);
    }
    switch(IKeyLabel[Opt.OnlyEditorViewerUsed][I][0])
    {
      case KBL_MAIN:
        if(DisableEdit)
          FViewKeys[6-1]="";
        if(!GetCanLoseFocus())
          FViewKeys[12-1]="";
        break;
      case KBL_ALT:
        // $ 17.12.2001 KM  - Если !GetCanLoseFocus() тогда на Alt-F11 рисуем пустую строку.
        if(!GetCanLoseFocus())
          FViewKeys[11-1]="";
        if(CtrlObject->Plugins.FindPlugin(SYSID_PRINTMANAGER) == -1)
          FViewKeys[5-1]="";
        break;
    }
    ViewKeyBar.SetGroup(IKeyLabel[Opt.OnlyEditorViewerUsed][I][0],FViewKeys,sizeof(FViewKeys)/sizeof(FViewKeys[0]));
  }

  SetKeyBar(&ViewKeyBar);
  // $ 15.07.2000 tran - ShowKeyBarViewer support
  View.SetPosition(X1,Y1,X2,Y2-(Opt.ShowKeyBarViewer?1:0));
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
  if (RedrawTitle && ((Key & 0x00ffffff) < KEY_END_FKEY))
    ShowConsoleTitle();

  if (Key!=KEY_F3 && !(Key==KEY_NUMPAD5||Key==KEY_SHIFTNUMPAD5))
    F3KeyOnly=FALSE;
  switch(Key)
  {
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
/* $ KEY_CTRLALTSHIFTPRESS унесено в manager OT */
    case KEY_CTRLO:
      if(!Opt.OnlyEditorViewerUsed)
      {
        FrameManager->ShowBackground();
        SetCursorType(FALSE,0);
        WaitKey();
        FrameManager->RefreshFrame();
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
        /* $ 11.10.2001 IS
            Если переключаемся в редактор, то удалять файл уже не
            нужно
        */
        SetTempViewName("");
        /* IS $ */
        SetExitCode(0);
        char ViewFileName[NM];
        View.GetFileName(ViewFileName);
        long FilePos=View.GetFilePos();
        /* $ 06.05.2001 DJ обработка F6 под NWZ */
        FileEditor *ShellEditor = new FileEditor (ViewFileName, FALSE, GetCanLoseFocus(),
          -2, FilePos, FALSE, NULL, SaveToSaveAs);
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

      /* $ 27.11.2001 DJ
         Local в ViewerConfig
      */
      ViewerConfig(ViOpt,1);
      /* DJ $ */

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
