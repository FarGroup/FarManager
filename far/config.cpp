/*
config.cpp

Конфигурация

*/

/* Revision: 1.27 11.10.2000 $ */

/*
Modify:
  11.10.2000 SVS
   + Opt.EditorBSLikeDel - если = 0, то BS действует как в FAR 1.65
  05.10.2000 SVS
    ! Все новые фишки (из TechInfo) только читаем...
  27.09.2000 SVS
    + HelpURLRules
    - XLat-таблицы только читаем.
    ! Ctrl-Alt-Shift - реагируем, если надо.
  24.09.2000 SVS
    + Opt.MaxPositionCache
    + Opt.SaveViewerShortPos & Opt.SaveEditorShortPos
    + Opt.CmdHistoryRule задает поведение Esc для командной строки:
       =1 - Не изменять положение в History, если после Ctrl-E/Ctrl-X
            нажали ESC (поведение - аля VC).
       =0 - поведение как и было - изменять положение в History
    ! "Editor\XLat" -> "XLat" - как самостоятельный раздел.
    + Клавиши вызывающие функцию Xlat
      Opt.XLatEditorKey, Opt.XLatCmdLineKey, Opt.XLatDialogKey
  20.09.2000 SVS
    + Opt.SubstPluginPrefix - 1 = подстанавливать префикс плагина
      для Ctrl-[ и ему подобные
  19.09.2000 SVS
    + Opt.PanelCtrlAltShiftRule задает поведение Ctrl-Alt-Shift для панелей.
  15.09.2000 IS
    + Отключение автоопределения таблицы символов, если отсутствует таблица с
      распределением частот символов
  15.09.2000 SVS
    ! RightClickRule по умолчанию ставится в положение 2
  14.09.2000 SVS
    ! Ошибка в названии XLAT_SWITCHKEYBLAYOUT.
  12.09.2000 SVS
    ! Разделение Wrap/WWrap/UnWrap на 2 составляющих -
      Состояние (Wrap/UnWrap) и тип (Wrap/WWrap)
    + Panel/RightClickRule в реестре - задает поведение правой клавиши
      мыши (это по поводу Bug#17)
  11.09.2000 SVS
    + если Far\Dialog\EULBsClear = 1, то BS в диалогах для UnChanged строки
      удаляет такую строку также, как и Del
  10.09.2000 SVS
    ! Наконец-то нашлось приемлемое имя для QWERTY -> Xlat.
  08.09.2000 SVS
    ! QWED_SWITCHKEYBLAYER -> EDTR_SWITCHKEYBLAYER
  07.09.2000 tran 1.14
    + Config//Current File
  05.09.2000 SVS 1.13
    + QWERTY - поддержка
  31.08.2000 SVS
    + Теперь FAR помнит тип Wrap
  04.08.2000 SVS
    ! WordDiv выкинул - будет описан в TechInfo.txt
      Но пустую строку все равно (даже в реестре) ввести нельзя!
  03.08.2000 SVS
    + WordDiv в случае пустого (юзвер убил, значит, случайно) ставится
      стандартный набор
  03.08.2000 SVS
    + WordDiv внесен в Options|Editor settings
  01.08.2000 SVS
    ! Добавка в виде задания дополнительного пути для поиска плагинов
      расширяется на месте - не имеет флаг по умолчанию!
    + "Вспомним" путь для дополнительных плагинов по шаблону!!!
      Сам шаблон берем из ключика в реестре!
  27.07.2000 SVS
    ! Opt.AutoComplete по умолчанию не активизирован,
      дабы не шокировать публику :-)
  26.07.2000 SVS
    + Opt.AutoComplete & дополнение диалога настройки интерфейса
  18.07.2000 tran 1.05
    + Opt.ShowViewerArrows, Opt.ShowViewerScrollbar
      изменил диалог ViewerSetting
  15.07.2000 tran
    + Opt.ShowKeyBarViewer
  15.07.2000 SVS
    + Добавка в виде задания дополнительного пути для поиска плагинов
  11.07.2000 SVS
    ! Последниие 5 индексов внаглую перезаписываются (если на этих местах
      стоят нули)
  04.07.2000 SVS
    ! ScrollBar Setting for Menus переехал из Options|Panel settings
      в Options|Interface settings
  30.06.2000 SVS
    - Кнопки залезли на рамку :-) в диалоге Options|Panel settings
  29.06.2000 SVS
    + Показывать ли ScrollBar для Menu
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#include "headers.hpp"
#pragma hdrstop

/* $ 30.06.2000 IS
   Стандартные заголовки
*/
#include "internalheaders.hpp"
/* IS $ */

/* $ 03.08.2000 SVS
   Стандартный набор разделителей
*/
static char WordDiv0[]="!%^&*()+|{}:\"<>?`-=\\[];',./";
/* SVS $ */

void SystemSettings()
{
  /* $ 15.07.2000 SVS
     + Добавка в виде задания дополнительного пути для поиска плагинов
  */
  static struct DialogData CfgDlgData[]={
  /*  0 */  DI_DOUBLEBOX,3,1,52,19,0,0,0,0,(char *)MConfigSystemTitle,
  /*  1 */  DI_CHECKBOX,5,2,0,0,1,0,0,0,(char *)MConfigRO,
  /*  2 */  DI_CHECKBOX,5,3,0,0,0,0,0,0,(char *)MConfigRecycleBin,
  /*  3 */  DI_CHECKBOX,5,4,0,0,0,0,0,0,(char *)MConfigSystemCopy,
  /*  4 */  DI_CHECKBOX,5,5,0,0,0,0,0,0,(char *)MConfigCopySharing,
  /*  5 */  DI_CHECKBOX,5,6,0,0,0,0,0,0,(char *)MConfigCreateUppercaseFolders,
  /*  6 */  DI_CHECKBOX,5,7,0,0,0,0,0,0,(char *)MConfigInactivity,
  /*  7 */  DI_FIXEDIT,9,8,11,6,0,0,0,0,"",
  /*  8 */  DI_TEXT,13,8,0,0,0,0,0,0,(char *)MConfigInactivityMinutes,
  /*  9 */  DI_CHECKBOX,5,9,0,0,0,0,0,0,(char *)MConfigSaveHistory,
  /* 10 */  DI_CHECKBOX,5,10,0,0,0,0,0,0,(char *)MConfigSaveFoldersHistory,
  /* 11 */  DI_CHECKBOX,5,11,0,0,0,0,0,0,(char *)MConfigSaveViewHistory,
  /* 12 */  DI_CHECKBOX,5,12,0,0,0,0,0,0,(char *)MConfigRegisteredTypes,
  /* 13 */  DI_CHECKBOX,5,13,0,0,0,0,0,0,(char *)MConfigSubstPluginPrefix,
  /* 14 */  DI_CHECKBOX,5,14,0,0,0,0,0,0,(char *)MConfigAutoSave,

  /* 15 */  DI_TEXT,5,15,0,0,0,0,0,0,(char *)MConfigPersonalPath,
  /* 16 */  DI_EDIT,5,16,50,15,0,0,0,0,"",

  /* 17 */  DI_TEXT,5,17,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
  /* 18 */  DI_BUTTON,0,18,0,0,0,0,DIF_CENTERGROUP,1,(char *)MOk,
  /* 19 */  DI_BUTTON,0,18,0,0,0,0,DIF_CENTERGROUP,0,(char *)MCancel
  };
  MakeDialogItems(CfgDlgData,CfgDlg);

  CfgDlg[1].Selected=Opt.ClearReadOnly;
  CfgDlg[2].Selected=Opt.DeleteToRecycleBin;
  CfgDlg[3].Selected=Opt.UseSystemCopy;
  CfgDlg[4].Selected=Opt.CopyOpened;
  CfgDlg[5].Selected=Opt.CreateUppercaseFolders;

  CfgDlg[6].Selected=Opt.InactivityExit;
  sprintf(CfgDlg[7].Data,"%d",Opt.InactivityExitTime);
  CfgDlg[9].Selected=Opt.SaveHistory;
  CfgDlg[10].Selected=Opt.SaveFoldersHistory;
  CfgDlg[11].Selected=Opt.SaveViewHistory;
  CfgDlg[12].Selected=Opt.UseRegisteredTypes;
  CfgDlg[13].Selected=Opt.SubstPluginPrefix;
  CfgDlg[14].Selected=Opt.AutoSaveSetup;
  strcpy(CfgDlg[16].Data,Opt.PersonalPluginsPath);

  {
    Dialog Dlg(CfgDlg,sizeof(CfgDlg)/sizeof(CfgDlg[0]));
    Dlg.SetHelp("SystemSettings");
    Dlg.SetPosition(-1,-1,56,21);
    Dlg.Process();
    if (Dlg.GetExitCode()!=18)
      return;
  }

  Opt.ClearReadOnly=CfgDlg[1].Selected;
  Opt.DeleteToRecycleBin=CfgDlg[2].Selected;
  Opt.UseSystemCopy=CfgDlg[3].Selected;
  Opt.CopyOpened=CfgDlg[4].Selected;
  Opt.CreateUppercaseFolders=CfgDlg[5].Selected;
  Opt.InactivityExit=CfgDlg[6].Selected;
  if ((Opt.InactivityExitTime=atoi(CfgDlg[7].Data))<=0)
    Opt.InactivityExit=Opt.InactivityExitTime=0;
  Opt.SaveHistory=CfgDlg[9].Selected;
  Opt.SaveFoldersHistory=CfgDlg[10].Selected;
  Opt.SaveViewHistory=CfgDlg[11].Selected;
  Opt.UseRegisteredTypes=CfgDlg[12].Selected;
  Opt.SubstPluginPrefix=CfgDlg[13].Selected;
  Opt.AutoSaveSetup=CfgDlg[14].Selected;
  strncpy(Opt.PersonalPluginsPath,CfgDlg[16].Data,sizeof(Opt.PersonalPluginsPath));
  /* SVS $ */
}


void PanelSettings()
{
  static struct DialogData CfgDlgData[]={
    DI_DOUBLEBOX,3,1,52,17,0,0,0,0,(char *)MConfigPanelTitle,
    DI_CHECKBOX,5,2,0,0,1,0,0,0,(char *)MConfigHidden,
    DI_CHECKBOX,5,3,0,0,0,0,0,0,(char *)MConfigHighlight,
    DI_CHECKBOX,5,4,0,0,0,0,0,0,(char *)MConfigAutoChange,
    DI_CHECKBOX,5,5,0,0,0,0,0,0,(char *)MConfigSelectFolders,
    DI_CHECKBOX,5,6,0,0,0,0,0,0,(char *)MConfigReverseSort,
    DI_TEXT,3,7,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
    DI_CHECKBOX,5,8,0,0,0,0,0,0,(char *)MConfigShowColumns,
    DI_CHECKBOX,5,9,0,0,0,0,0,0,(char *)MConfigShowStatus,
    DI_CHECKBOX,5,10,0,0,0,0,0,0,(char *)MConfigShowTotal,
    DI_CHECKBOX,5,11,0,0,0,0,0,0,(char *)MConfigShowFree,
    DI_CHECKBOX,5,12,0,0,0,0,0,0,(char *)MConfigShowScrollbar,
    DI_CHECKBOX,5,13,0,0,0,0,0,0,(char *)MConfigShowScreensNumber,
    DI_CHECKBOX,5,14,0,0,0,0,0,0,(char *)MConfigShowSortMode,
    DI_TEXT,3,15,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
    DI_BUTTON,0,16,0,0,0,0,DIF_CENTERGROUP,1,(char *)MOk,
    DI_BUTTON,0,16,0,0,0,0,DIF_CENTERGROUP,0,(char *)MCancel
  };
  MakeDialogItems(CfgDlgData,CfgDlg);

  CfgDlg[1].Selected=Opt.ShowHidden;
  CfgDlg[2].Selected=Opt.Highlight;
  CfgDlg[3].Selected=Opt.AutoChangeFolder;
  CfgDlg[4].Selected=Opt.SelectFolders;
  CfgDlg[5].Selected=Opt.ReverseSort;

  CfgDlg[7].Selected=Opt.ShowColumnTitles;
  CfgDlg[8].Selected=Opt.ShowPanelStatus;
  CfgDlg[9].Selected=Opt.ShowPanelTotals;
  CfgDlg[10].Selected=Opt.ShowPanelFree;
  CfgDlg[11].Selected=Opt.ShowPanelScrollbar;
  CfgDlg[12].Selected=Opt.ShowScreensNumber;
  CfgDlg[13].Selected=Opt.ShowSortMode;

  {
    Dialog Dlg(CfgDlg,sizeof(CfgDlg)/sizeof(CfgDlg[0]));
    Dlg.SetHelp("PanelSettings");
    Dlg.SetPosition(-1,-1,56,19);
    Dlg.Process();
    if (Dlg.GetExitCode()!=15)
      return;
  }

  Opt.ShowHidden=CfgDlg[1].Selected;
  Opt.Highlight=CfgDlg[2].Selected;
  Opt.AutoChangeFolder=CfgDlg[3].Selected;
  Opt.SelectFolders=CfgDlg[4].Selected;
  Opt.ReverseSort=CfgDlg[5].Selected;

  Opt.ShowColumnTitles=CfgDlg[7].Selected;
  Opt.ShowPanelStatus=CfgDlg[8].Selected;
  Opt.ShowPanelTotals=CfgDlg[9].Selected;
  Opt.ShowPanelFree=CfgDlg[10].Selected;
  Opt.ShowPanelScrollbar=CfgDlg[11].Selected;
  Opt.ShowScreensNumber=CfgDlg[12].Selected;
  Opt.ShowSortMode=CfgDlg[13].Selected;

  CtrlObject->LeftPanel->Update(UPDATE_KEEP_SELECTION);
  CtrlObject->RightPanel->Update(UPDATE_KEEP_SELECTION);
  CtrlObject->SetScreenPositions();
}


void InterfaceSettings()
{
  static struct DialogData CfgDlgData[]={
    /* $ 04.07.2000 SVS
       + Показывать ли ScrollBar для Menu|Options|Interface settings
    */
    /* $ 26.07.2000 SVS
       + Разрешить ли автодополнение в строках ввода
    */
    DI_DOUBLEBOX,3,1,52,18,0,0,0,0,(char *)MConfigInterfaceTitle,
    DI_CHECKBOX,5,2,0,0,1,0,0,0,(char *)MConfigClock,
    DI_CHECKBOX,5,3,0,0,0,0,0,0,(char *)MConfigViewerEditorClock,
    DI_CHECKBOX,5,4,0,0,0,0,0,0,(char *)MConfigMouse,
    DI_CHECKBOX,5,5,0,0,0,0,0,0,(char *)MConfigKeyBar,
    DI_CHECKBOX,5,6,0,0,0,0,0,0,(char *)MConfigMenuBar,
    DI_CHECKBOX,5,7,0,0,0,0,0,0,(char *)MConfigSaver,
    DI_FIXEDIT,9,8,11,5,0,0,0,0,"",
    DI_TEXT,13,8,0,0,0,0,0,0,(char *)MConfigSaverMinutes,
    DI_CHECKBOX,5,9,0,0,0,0,0,0,(char *)MConfigDialogsEditHistory,
    DI_CHECKBOX,5,10,0,0,0,0,0,0,(char *)MConfigUsePromptFormat,
    DI_EDIT,9,11,24,10,0,0,0,0,"",
    DI_CHECKBOX,5,12,0,0,0,0,0,0,(char *)MConfigAltGr,
    DI_CHECKBOX,5,13,0,0,0,0,0,0,(char *)MConfigCopyTotal,
    DI_CHECKBOX,5,14,0,0,0,0,0,0,(char *)MConfigShowMenuScrollbar,
    DI_CHECKBOX,5,15,0,0,0,0,0,0,(char *)MConfigAutoComplete,
    DI_TEXT,3,16,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
    DI_BUTTON,0,17,0,0,0,0,DIF_CENTERGROUP,1,(char *)MOk,
    DI_BUTTON,0,17,0,0,0,0,DIF_CENTERGROUP,0,(char *)MCancel
  };
  MakeDialogItems(CfgDlgData,CfgDlg);

  if (!RegVer)
  {
    CfgDlg[2].Type=DI_TEXT;
    sprintf(CfgDlg[2].Data," *  %s",MSG(MRegOnlyShort));
  }

  CfgDlg[1].Selected=Opt.Clock;
  CfgDlg[2].Selected=Opt.ViewerEditorClock;
  CfgDlg[3].Selected=Opt.Mouse;
  CfgDlg[4].Selected=Opt.ShowKeyBar;
  CfgDlg[5].Selected=Opt.ShowMenuBar;
  CfgDlg[6].Selected=Opt.ScreenSaver;
  sprintf(CfgDlg[7].Data,"%d",Opt.ScreenSaverTime);
  CfgDlg[9].Selected=Opt.DialogsEditHistory;
  CfgDlg[10].Selected=Opt.UsePromptFormat;
  strcpy(CfgDlg[11].Data,Opt.PromptFormat);
  CfgDlg[12].Selected=Opt.AltGr;
  CfgDlg[13].Selected=Opt.CopyShowTotal;
  CfgDlg[14].Selected=Opt.ShowMenuScrollbar;
  CfgDlg[15].Selected=Opt.AutoComplete;

  {
    Dialog Dlg(CfgDlg,sizeof(CfgDlg)/sizeof(CfgDlg[0]));
    Dlg.SetHelp("InterfSettings");
    Dlg.SetPosition(-1,-1,56,20);
    Dlg.Process();
    if (Dlg.GetExitCode()!=17)
      return;
  }

  Opt.Clock=CfgDlg[1].Selected;
  Opt.ViewerEditorClock=CfgDlg[2].Selected;
  Opt.Mouse=CfgDlg[3].Selected;
  Opt.ShowKeyBar=CfgDlg[4].Selected;
  Opt.ShowMenuBar=CfgDlg[5].Selected;
  Opt.ScreenSaver=CfgDlg[6].Selected;
  Opt.DialogsEditHistory=CfgDlg[9].Selected;
  if ((Opt.ScreenSaverTime=atoi(CfgDlg[7].Data))<=0)
    Opt.ScreenSaver=Opt.ScreenSaverTime=0;
  Opt.UsePromptFormat=CfgDlg[10].Selected;
  strncpy(Opt.PromptFormat,CfgDlg[11].Data,sizeof(Opt.PromptFormat));
  Opt.AltGr=CfgDlg[12].Selected;
  Opt.CopyShowTotal=CfgDlg[13].Selected;
  Opt.ShowMenuScrollbar=CfgDlg[14].Selected;
  Opt.AutoComplete=CfgDlg[15].Selected;
  /* SVS $ */
  SetFarConsoleMode();
  CtrlObject->LeftPanel->Update(UPDATE_KEEP_SELECTION);
  CtrlObject->RightPanel->Update(UPDATE_KEEP_SELECTION);
  CtrlObject->SetScreenPositions();
}


void SetConfirmations()
{
  static struct DialogData ConfDlgData[]={
    DI_DOUBLEBOX,3,1,41,10,0,0,0,0,(char *)MSetConfirmTitle,
    DI_CHECKBOX,5,2,0,0,1,0,0,0,(char *)MSetConfirmCopy,
    DI_CHECKBOX,5,3,0,0,0,0,0,0,(char *)MSetConfirmMove,
    DI_CHECKBOX,5,4,0,0,0,0,0,0,(char *)MSetConfirmDrag,
    DI_CHECKBOX,5,5,0,0,0,0,0,0,(char *)MSetConfirmDelete,
    DI_CHECKBOX,5,6,0,0,0,0,0,0,(char *)MSetConfirmDeleteFolders,
    DI_CHECKBOX,5,7,0,0,0,0,0,0,(char *)MSetConfirmExit,
    DI_TEXT,3,8,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
    DI_BUTTON,0,9,0,0,0,0,DIF_CENTERGROUP,1,(char *)MOk,
    DI_BUTTON,0,9,0,0,0,0,DIF_CENTERGROUP,0,(char *)MCancel
  };
  MakeDialogItems(ConfDlgData,ConfDlg);
  ConfDlg[1].Selected=Opt.Confirm.Copy;
  ConfDlg[2].Selected=Opt.Confirm.Move;
  ConfDlg[3].Selected=Opt.Confirm.Drag;
  ConfDlg[4].Selected=Opt.Confirm.Delete;
  ConfDlg[5].Selected=Opt.Confirm.DeleteFolder;
  ConfDlg[6].Selected=Opt.Confirm.Exit;
  Dialog Dlg(ConfDlg,sizeof(ConfDlg)/sizeof(ConfDlg[0]));
  Dlg.SetHelp("ConfirmDlg");
  Dlg.SetPosition(-1,-1,45,12);
  Dlg.Process();
  if (Dlg.GetExitCode()!=8)
    return;
  Opt.Confirm.Copy=ConfDlg[1].Selected;
  Opt.Confirm.Move=ConfDlg[2].Selected;
  Opt.Confirm.Drag=ConfDlg[3].Selected;
  Opt.Confirm.Delete=ConfDlg[4].Selected;
  Opt.Confirm.DeleteFolder=ConfDlg[5].Selected;
  Opt.Confirm.Exit=ConfDlg[6].Selected;
}


void SetDizConfig()
{
  static struct DialogData DizDlgData[]=
  {
    DI_DOUBLEBOX,3,1,72,13,0,0,0,0,(char *)MCfgDizTitle,
    DI_TEXT,5,2,0,0,0,0,0,0,(char *)MCfgDizListNames,
    DI_EDIT,5,3,70,3,1,0,0,0,"",
    DI_TEXT,3,4,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
    DI_CHECKBOX,5,5,0,0,0,0,0,0,(char *)MCfgDizSetHidden,
    DI_FIXEDIT,5,6,7,6,0,0,0,0,"",
    DI_TEXT,9,6,0,0,0,0,0,0,(char *)MCfgDizStartPos,
    DI_TEXT,3,7,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
    DI_RADIOBUTTON,5,8,0,0,0,0,DIF_GROUP,0,(char *)MCfgDizNotUpdate,
    DI_RADIOBUTTON,5,9,0,0,0,0,0,0,(char *)MCfgDizUpdateIfDisplayed,
    DI_RADIOBUTTON,5,10,0,0,0,0,0,0,(char *)MCfgDizAlwaysUpdate,
    DI_TEXT,3,11,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
    DI_BUTTON,0,12,0,0,0,0,DIF_CENTERGROUP,1,(char *)MOk,
    DI_BUTTON,0,12,0,0,0,0,DIF_CENTERGROUP,0,(char *)MCancel
  };
  MakeDialogItems(DizDlgData,DizDlg);
  Dialog Dlg(DizDlg,sizeof(DizDlg)/sizeof(DizDlg[0]));
  Dlg.SetPosition(-1,-1,76,15);
  Dlg.SetHelp("FileDiz");
  strcpy(DizDlg[2].Data,Opt.Diz.ListNames);
  if (Opt.Diz.UpdateMode==DIZ_NOT_UPDATE)
    DizDlg[8].Selected=TRUE;
  else
    if (Opt.Diz.UpdateMode==DIZ_UPDATE_IF_DISPLAYED)
      DizDlg[9].Selected=TRUE;
    else
      DizDlg[10].Selected=TRUE;
  DizDlg[4].Selected=Opt.Diz.SetHidden;
  sprintf(DizDlg[5].Data,"%d",Opt.Diz.StartPos);

  Dlg.Process();
  if (Dlg.GetExitCode()!=12)
    return;
  strcpy(Opt.Diz.ListNames,DizDlg[2].Data);
  if (DizDlg[8].Selected)
    Opt.Diz.UpdateMode=DIZ_NOT_UPDATE;
  else
    if (DizDlg[9].Selected)
      Opt.Diz.UpdateMode=DIZ_UPDATE_IF_DISPLAYED;
    else
      Opt.Diz.UpdateMode=DIZ_UPDATE_ALWAYS;
  Opt.Diz.SetHidden=DizDlg[4].Selected;
  Opt.Diz.StartPos=atoi(DizDlg[5].Data);
}

/* $ 18.07.2000 tran
   настройка двух новых параметров для вьювера */
/*
 ╔═════════════════ Viewer ══════════════════╗
 ║ ┌ External viewer ──────────────────────┐ ║
 ║ │ ( ) Use for F3                        │ ║
 ║ │ () Use for Alt-F3                    │ ║
 ║ │ Viewer command:                       │ ║
 ║ │                                       │ ║
 ║ └───────────────────────────────────────┘ ║
 ║ ┌ Internal viewer ──────────────────────┐ ║
 ║ │ [x] Save file position                │ ║
 ║ │ [x] Save shortcut position            │ ║
 ║ │ [x] Autodetect character table        │ ║
 ║ │ 8   Tab size                          │ ║
 ║ │ [x] Show scrollbar                    │ ║
 ║ │ [x] Show arrows                       │ ║
 ║ └───────────────────────────────────────┘ ║
 ║            [ Ok ]  [ Cancel ]             ║
 ╚═══════════════════════════════════════════╝
*/
#define DLG_VIEW_USE_F3      2
#define DLG_VIEW_USE_ALTF3   3
#define DLG_VIEW_EXTERNAL    5
#define DLG_VIEW_SAVEFILEPOS 7
#define DLG_VIEW_SAVESHORTPOS 8
#define DLG_VIEW_AUTODETECT  9
#define DLG_VIEW_TABSIZE    10
#define DLG_VIEW_SCROLLBAR  12
#define DLG_VIEW_ARROWS     13
#define DLG_VIEW_OK         14

void ViewerConfig()
{
  static struct DialogData CfgDlgData[]={
    DI_DOUBLEBOX , 3, 1,47,17,0,0,0,0,(char *)MViewConfigTitle,                  //   0
    DI_SINGLEBOX , 5, 2,45, 7,0,0,DIF_LEFTTEXT,0,(char *)MViewConfigExternal,    //   1
    DI_RADIOBUTTON,7, 3, 0, 0,1,0,DIF_GROUP,0,(char *)MViewConfigExternalF3,     //   2
    DI_RADIOBUTTON,7, 4, 0, 0,0,0,0,0,(char *)MViewConfigExternalAltF3,          //   3
    DI_TEXT      , 7, 5, 0, 0,0,0,0,0,(char *)MViewConfigExternalCommand,        //   4
    DI_EDIT      , 7, 6,43, 6,0,0,0,0,"",                                        //   5
    DI_SINGLEBOX , 5, 8,45,15,0,0,DIF_LEFTTEXT,0,(char *)MViewConfigInternal,    //   6
    DI_CHECKBOX  , 7, 9, 0, 0,0,0,0,0,(char *)MViewConfigSavePos,                //   7
    DI_CHECKBOX  , 7,10, 0, 0,0,0,0,0,(char *)MViewConfigSaveShortPos,           //   8
    DI_CHECKBOX  , 7,11, 0, 0,0,0,0,0,(char *)MViewAutoDetectTable,              //   9
    DI_FIXEDIT   , 7,12, 9,15,0,0,0,0,"",                                        //  10
    DI_TEXT      ,11,12, 0, 0,0,0,0,0,(char *)MViewConfigTabSize,                //  11
    DI_CHECKBOX  , 7,13, 0, 0,0,0,0,0,(char *)MViewConfigScrollbar,              //  12 *new
    DI_CHECKBOX  , 7,14, 0, 0,0,0,0,0,(char *)MViewConfigArrows,                 //  13 *new
    DI_BUTTON    , 0,16, 0, 0,0,0,DIF_CENTERGROUP,1,(char *)MOk,                 //  14 , was 11
    DI_BUTTON    , 0,16, 0, 0,0,0,DIF_CENTERGROUP,0,(char *)MCancel              //  15 , was 12
  };
  MakeDialogItems(CfgDlgData,CfgDlg);

  CfgDlg[DLG_VIEW_USE_F3].Selected=Opt.UseExternalViewer;
  CfgDlg[DLG_VIEW_USE_ALTF3].Selected=!Opt.UseExternalViewer;
  strcpy(CfgDlg[DLG_VIEW_EXTERNAL].Data,Opt.ExternalViewer);
  CfgDlg[DLG_VIEW_SAVEFILEPOS].Selected=Opt.SaveViewerPos;
  CfgDlg[DLG_VIEW_SAVESHORTPOS].Selected=Opt.SaveEditorShortPos;
  /* 15.09.2000 IS
     Отключение автоопределения таблицы символов, если отсутствует таблица с
     распределением частот символов
  */
  CfgDlg[DLG_VIEW_AUTODETECT].Selected=Opt.ViewerAutoDetectTable&&DistrTableExist();
  /* IS $ */
  CfgDlg[DLG_VIEW_SCROLLBAR].Selected=Opt.ViewerShowScrollbar;
  CfgDlg[DLG_VIEW_ARROWS].Selected=Opt.ViewerShowArrows;
  sprintf(CfgDlg[DLG_VIEW_TABSIZE].Data,"%d",Opt.ViewTabSize);

  if (!RegVer)
  {
    CfgDlg[DLG_VIEW_TABSIZE].Type=CfgDlg[10].Type=DI_TEXT;
    sprintf(CfgDlg[DLG_VIEW_TABSIZE].Data," *  %s",MSG(MRegOnlyShort));
    *CfgDlg[10].Data=0;
  }

  {
    Dialog Dlg(CfgDlg,sizeof(CfgDlg)/sizeof(CfgDlg[0]));
    Dlg.SetHelp("ViewerSettings");
    Dlg.SetPosition(-1,-1,51,19);
    Dlg.Process();
    if (Dlg.GetExitCode()!=DLG_VIEW_OK)
      return;
  }

  Opt.UseExternalViewer=CfgDlg[DLG_VIEW_USE_F3].Selected;
  strcpy(Opt.ExternalViewer,CfgDlg[DLG_VIEW_EXTERNAL].Data);
  Opt.SaveViewerPos=CfgDlg[DLG_VIEW_SAVEFILEPOS].Selected;
  Opt.SaveEditorShortPos=CfgDlg[DLG_VIEW_SAVESHORTPOS].Selected;
  /* 15.09.2000 IS
     Отключение автоопределения таблицы символов, если отсутствует таблица с
     распределением частот символов
  */
  if(!DistrTableExist() && Opt.ViewerAutoDetectTable)
  {
    Opt.ViewerAutoDetectTable=0;
    Message(MSG_WARNING,1,MSG(MWarning),
              MSG(MDistributionTableWasNotFound),MSG(MAutoDetectWillNotWork),
              MSG(MOk));
  }
  /* IS $ */
  Opt.ViewTabSize=atoi(CfgDlg[DLG_VIEW_TABSIZE].Data);
  Opt.ViewerShowScrollbar=CfgDlg[DLG_VIEW_SCROLLBAR].Selected;
  Opt.ViewerShowArrows=CfgDlg[DLG_VIEW_ARROWS].Selected;
  if (Opt.ViewTabSize<1 || Opt.ViewTabSize>512)
    Opt.ViewTabSize=8;
}
/* tran 18.07.2000 $ */

/* $ 04.08.2000 SVS
   ! WordDiv выкинул - будет описан в TechInfo.txt
*/
/* $ 03.08.2000 SVS
  + WordDiv внесен в Options|Editor settings
*/
void EditorConfig()
{
  static struct DialogData CfgDlgData[]={
  /*  0 */  DI_DOUBLEBOX,3,1,47,20,0,0,0,0,(char *)MEditConfigTitle,
  /*  1 */  DI_SINGLEBOX,5,2,45,7,0,0,DIF_LEFTTEXT,0,(char *)MEditConfigExternal,
  /*  2 */  DI_RADIOBUTTON,7,3,0,0,1,0,DIF_GROUP,0,(char *)MEditConfigEditorF4,
  /*  3 */  DI_RADIOBUTTON,7,4,0,0,0,0,0,0,(char *)MEditConfigEditorAltF4,
  /*  4 */  DI_TEXT,7,5,0,0,0,0,0,0,(char *)MEditConfigEditorCommand,
  /*  5 */  DI_EDIT,7,6,43,6,0,0,0,0,"",
  /*  6 */  DI_SINGLEBOX,5,8,45,18,0,0,DIF_LEFTTEXT,0,(char *)MEditConfigInternal,
  /*  7 */  DI_CHECKBOX,7,9,0,0,0,0,0,0,(char *)MEditConfigTabsToSpaces,
  /*  8 */  DI_CHECKBOX,7,10,0,0,0,0,0,0,(char *)MEditConfigPersistentBlocks,
  /*  9 */  DI_CHECKBOX,7,11,0,0,0,0,0,0,(char *)MEditConfigDelRemovesBlocks,
  /* 10 */  DI_CHECKBOX,7,12,0,0,0,0,0,0,(char *)MEditConfigAutoIndent,
  /* 11 */  DI_CHECKBOX,7,13,0,0,0,0,0,0,(char *)MEditConfigSavePos,
  /* 12 */  DI_CHECKBOX,7,14,0,0,0,0,0,0,(char *)MEditConfigSaveShortPos,
  /* 13 */  DI_CHECKBOX,7,15,0,0,0,0,0,0,(char *)MEditAutoDetectTable,
  /* 14 */  DI_FIXEDIT,7,16,9,15,0,0,0,0,"",
  /* 15 */  DI_TEXT,11,16,0,0,0,0,0,0,(char *)MEditConfigTabSize,
  /* 16 */  DI_CHECKBOX,7,17,0,0,0,0,0,0,(char *)MEditCursorBeyondEnd,
  /* 17 */  DI_BUTTON,0,19,0,0,0,0,DIF_CENTERGROUP,1,(char *)MOk,
  /* 18 */  DI_BUTTON,0,19,0,0,0,0,DIF_CENTERGROUP,0,(char *)MCancel
  };
  MakeDialogItems(CfgDlgData,CfgDlg);

  CfgDlg[2].Selected=Opt.UseExternalEditor;
  CfgDlg[3].Selected=!Opt.UseExternalEditor;
  strcpy(CfgDlg[5].Data,Opt.ExternalEditor);
  CfgDlg[7].Selected=Opt.EditorExpandTabs;
  CfgDlg[8].Selected=Opt.EditorPersistentBlocks;
  CfgDlg[9].Selected=Opt.EditorDelRemovesBlocks;
  CfgDlg[10].Selected=Opt.EditorAutoIndent;
  CfgDlg[11].Selected=Opt.SaveEditorPos;
  CfgDlg[12].Selected=Opt.SaveEditorShortPos;
  /* 15.09.2000 IS
     Отключение автоопределения таблицы символов, если отсутствует таблица с
     распределением частот символов
  */
  CfgDlg[13].Selected=Opt.EditorAutoDetectTable&&DistrTableExist();
  /* IS $ */
  sprintf(CfgDlg[14].Data,"%d",Opt.TabSize);
  CfgDlg[16].Selected=Opt.EditorCursorBeyondEOL;

  if (!RegVer)
  {
    CfgDlg[14].Type=CfgDlg[15].Type=DI_TEXT;
    sprintf(CfgDlg[14].Data," *  %s",MSG(MRegOnlyShort));
    *CfgDlg[15].Data=0;
  }

  {
    Dialog Dlg(CfgDlg,sizeof(CfgDlg)/sizeof(CfgDlg[0]));
    Dlg.SetHelp("EditorSettings");
    Dlg.SetPosition(-1,-1,51,22);
    Dlg.Process();
    if (Dlg.GetExitCode()!=17)
      return;
  }

  Opt.UseExternalEditor=CfgDlg[2].Selected;
  strcpy(Opt.ExternalEditor,CfgDlg[5].Data);
  Opt.EditorExpandTabs=CfgDlg[7].Selected;
  Opt.EditorPersistentBlocks=CfgDlg[8].Selected;
  Opt.EditorDelRemovesBlocks=CfgDlg[9].Selected;
  Opt.EditorAutoIndent=CfgDlg[10].Selected;
  Opt.SaveEditorPos=CfgDlg[11].Selected;
  Opt.SaveEditorShortPos=CfgDlg[12].Selected;
  Opt.EditorAutoDetectTable=CfgDlg[13].Selected;
  /* 15.09.2000 IS
     Отключение автоопределения таблицы символов, если отсутствует таблица с
     распределением частот символов
  */
  if(!DistrTableExist() && Opt.EditorAutoDetectTable)
  {
    Opt.EditorAutoDetectTable=0;
    Message(MSG_WARNING,1,MSG(MWarning),
              MSG(MDistributionTableWasNotFound),MSG(MAutoDetectWillNotWork),
              MSG(MOk));
  }
  /* IS $ */
  Opt.TabSize=atoi(CfgDlg[14].Data);
  if (Opt.TabSize<1 || Opt.TabSize>512)
    Opt.TabSize=8;
  Opt.EditorCursorBeyondEOL=CfgDlg[16].Selected;
}
/* SVS $ */


void SetFolderInfoFiles()
{
  GetString(MSG(MSetFolderInfoTitle),MSG(MSetFolderInfoNames),NULL,Opt.FolderInfoFiles,Opt.FolderInfoFiles,sizeof(Opt.FolderInfoFiles),"OptMenu",FIB_ENABLEEMPTY);
}


void ReadConfig()
{
  //                                                    было sizeof(Palette)
  GetRegKey("Colors","CurrentPalette",Palette,DefaultPalette,SizeArrayPalette);
  /* $ 11.07.2000 SVS
     Последниие несколько индексов внаглую перезаписываются (если на этих
     местах стоят нули)
  */
  int I;
  for(I=COL_DIALOGMENUSCROLLBAR-COL_FIRSTPALETTECOLOR;
      I < (COL_DIALOGMENUSCROLLBAR-COL_FIRSTPALETTECOLOR)+4;
      ++I)
  {
    if(!Palette[I])
      if(!Palette[COL_PRIVATEPOSITION_FOR_XRENZNAETCHEGO-COL_FIRSTPALETTECOLOR])
        Palette[I]=DefaultPalette[I];
      else if(Palette[COL_PRIVATEPOSITION_FOR_XRENZNAETCHEGO-COL_FIRSTPALETTECOLOR] == 1)
        Palette[I]=BlackPalette[I];
      /*
      else
        в других случаях нифига ничего не делаем, т.к.
        есть другие палитры...
      */
  }
  /* SVS $ */

  GetRegKey("Screen","Clock",Opt.Clock,1);
  GetRegKey("Screen","ViewerEditorClock",Opt.ViewerEditorClock,0);
  GetRegKey("Screen","KeyBar",Opt.ShowKeyBar,1);
  GetRegKey("Screen","ScreenSaver",Opt.ScreenSaver,1);
  GetRegKey("Screen","ScreenSaverTime",Opt.ScreenSaverTime,5);
  GetRegKey("Screen","UsePromptFormat",Opt.UsePromptFormat,0);
  GetRegKey("Screen","PromptFormat",Opt.PromptFormat,"$p>",sizeof(Opt.PromptFormat));

  GetRegKey("Interface","DialogsEditHistory",Opt.DialogsEditHistory,1);
  GetRegKey("Interface","Mouse",Opt.Mouse,1);
  GetRegKey("Interface","AltGr",Opt.AltGr,1);
  GetRegKey("Interface","CopyShowTotal",Opt.CopyShowTotal,0);
  GetRegKey("Interface","ShowMenuBar",Opt.ShowMenuBar,0);
  /* $ 27.07.2000 SVS
     ! Opt.AutoComplete по умолчанию не активизирован,
       дабы не шокировать публику :-)
  */
  GetRegKey("Interface","AutoComplete",Opt.AutoComplete,0);
  /* SVS $*/

  GetRegKey("Viewer","ExternalViewerName",Opt.ExternalViewer,"",sizeof(Opt.ExternalViewer));
  GetRegKey("Viewer","UseExternalViewer",Opt.UseExternalViewer,0);
  GetRegKey("Viewer","SaveViewerPos",Opt.SaveViewerPos,0);
  GetRegKey("Viewer","SaveViewerShortPos",Opt.SaveViewerShortPos,0);
  GetRegKey("Viewer","AutoDetectTable",Opt.ViewerAutoDetectTable,0);
  GetRegKey("Viewer","TabSize",Opt.ViewTabSize,8);
  /* $ 15.07.2000 tran
     + Opt.ShowKeyBarViewer */
  GetRegKey("Viewer","ShowKeyBar",Opt.ShowKeyBarViewer,1);
  /* tran 15.07.2000 $ */
  /* $ 18.07.2000 tran
     + Opt.ViewerShowArrows, Opt.ViewerShowScrollbar*/
  GetRegKey("Viewer","ShowArrows",Opt.ViewerShowArrows,1);
  GetRegKey("Viewer","ShowScrollbar",Opt.ViewerShowScrollbar,0);
  /* tran 18.07.2000 $ */
  /* $ 31.08.2000 SVS
     ! Opt.ViewerIsWrap
  */
  GetRegKey("Viewer","IsWrap",Opt.ViewerIsWrap,1);
  Opt.ViewerIsWrap&=1;
  GetRegKey("Viewer","Wrap",Opt.ViewerWrap,0);
  if(RegVer) Opt.ViewerWrap&=1; else Opt.ViewerWrap=0;
  /* SVS $*/

  /* $ 11.09.2000 SVS
     если EULBsClear = 1, то BS в диалогах для UnChanged строки
     удаляет такую строку также, как и Del
  */
  GetRegKey("Dialog","EULBsClear",Opt.DlgEULBsClear,0);
  /* SVS $*/

  GetRegKey("Editor","ExternalEditorName",Opt.ExternalEditor,"",sizeof(Opt.ExternalEditor));
  GetRegKey("Editor","UseExternalEditor",Opt.UseExternalEditor,0);
  GetRegKey("Editor","ExpandTabs",Opt.EditorExpandTabs,0);
  GetRegKey("Editor","TabSize",Opt.TabSize,8);
  GetRegKey("Editor","PersistentBlocks",Opt.EditorPersistentBlocks,1);
  GetRegKey("Editor","DelRemovesBlocks",Opt.EditorDelRemovesBlocks,0);
  GetRegKey("Editor","AutoIndent",Opt.EditorAutoIndent,0);
  GetRegKey("Editor","SaveEditorPos",Opt.SaveEditorPos,0);
  GetRegKey("Editor","SaveEditorShortPos",Opt.SaveEditorShortPos,0);
  GetRegKey("Editor","AutoDetectTable",Opt.EditorAutoDetectTable,0);
  GetRegKey("Editor","EditorCursorBeyondEOL",Opt.EditorCursorBeyondEOL,1);
  /* $ 03.08.2000 SVS
     Записать разграничитель слов из реестра
  */
  GetRegKey("Editor","WordDiv",Opt.WordDiv,WordDiv0,79);
  /* SVS $ */
  GetRegKey("Editor","BSLikeDel",Opt.EditorBSLikeDel,1);

  /* $ 03.08.2000 SVS
     Исключаем случайное стирание разделителей ;-)
  */
  if(!strlen(Opt.WordDiv))
     strcpy(Opt.WordDiv,WordDiv0);
  /* SVS $ */
  /* $ 05.09.2000 SVS
     CodeXLat - описывающая XLat-перекодировщик
  */
  GetRegKey("XLat","Flags",(int&)Opt.XLat.Flags,(DWORD)XLAT_SWITCHKEYBLAYOUT);
  GetRegKey("XLat","Table1",(BYTE*)Opt.XLat.Table[0],(BYTE*)NULL,sizeof(Opt.XLat.Table[0]));
  GetRegKey("XLat","Table2",(BYTE*)Opt.XLat.Table[1],(BYTE*)NULL,sizeof(Opt.XLat.Table[1]));
  GetRegKey("XLat","Rules1",(BYTE*)Opt.XLat.Rules[0],(BYTE*)NULL,sizeof(Opt.XLat.Rules[0]));
  GetRegKey("XLat","Rules2",(BYTE*)Opt.XLat.Rules[1],(BYTE*)NULL,sizeof(Opt.XLat.Rules[1]));
  GetRegKey("XLat","Rules3",(BYTE*)Opt.XLat.Rules[2],(BYTE*)NULL,sizeof(Opt.XLat.Rules[2]));
  /* SVS $ */
  /* $ 24.09.2000 SVS
     Клавиши, вызывающие Xlat
  */
  GetRegKey("XLat","EditorKey",Opt.XLatEditorKey,KEY_CTRLSHIFTX);
  GetRegKey("XLat","CmdLineKey",Opt.XLatCmdLineKey,KEY_CTRLSHIFTX);
  GetRegKey("XLat","DialogKey",Opt.XLatDialogKey,KEY_CTRLSHIFTX);
  /* SVS $ */

  GetRegKey("System","SaveHistory",Opt.SaveHistory,0);
  GetRegKey("System","SaveFoldersHistory",Opt.SaveFoldersHistory,0);
  GetRegKey("System","SaveViewHistory",Opt.SaveViewHistory,0);
  GetRegKey("System","UseRegisteredTypes",Opt.UseRegisteredTypes,1);
  GetRegKey("System","AutoSaveSetup",Opt.AutoSaveSetup,0);

  GetRegKey("Help","ActivateURL",Opt.HelpURLRules,1);

  /* $ 15.07.2000 SVS
     "Вспомним" путь для дополнительного поиска плагинов
  */
  {
   /* $ 01.08.2000 SVS
      "Вспомним" путь по шаблону!!!
   */
   SetRegRootKey(HKEY_LOCAL_MACHINE);
   char PersonalPluginsPath[1024];
   GetRegKey("System","TemplatePluginsPath",PersonalPluginsPath,"",sizeof(Opt.PersonalPluginsPath));
   SetRegRootKey(HKEY_CURRENT_USER); //???????????????????????
   GetRegKey("System","PersonalPluginsPath",Opt.PersonalPluginsPath,PersonalPluginsPath,sizeof(Opt.PersonalPluginsPath));
   /* 01.08.2000 SVS $ */
  }
  /* SVS $ */
  GetRegKey("System","ClearReadOnly",Opt.ClearReadOnly,0);
  GetRegKey("System","DeleteToRecycleBin",Opt.DeleteToRecycleBin,1);
  GetRegKey("System","UseSystemCopy",Opt.UseSystemCopy,0);
  GetRegKey("System","CopyOpened",Opt.CopyOpened,0);
  GetRegKey("System","CreateUppercaseFolders",Opt.CreateUppercaseFolders,0);
  GetRegKey("System","InactivityExit",Opt.InactivityExit,0);
  GetRegKey("System","InactivityExitTime",Opt.InactivityExitTime,15);
  GetRegKey("System","DriveMenuMode",Opt.ChangeDriveMode,DRIVE_SHOW_TYPE|DRIVE_SHOW_PLUGINS);
  GetRegKey("System","FileSearchMode",Opt.FileSearchMode,SEARCH_ROOT);
  GetRegKey("System","FolderInfo",Opt.FolderInfoFiles,"DirInfo,File_Id.diz,Descript.ion,ReadMe,Read.Me,ReadMe.txt,ReadMe.*",sizeof(Opt.FolderInfoFiles));
  GetRegKey("System","SubstPluginPrefix",Opt.SubstPluginPrefix,0);
  /* $ 24.09.2000 SVS
     + CmdHistoryRule задает поведение Esc для командной строки
  */
  GetRegKey("System","CmdHistoryRule",Opt.CmdHistoryRule,0);
  /* SVS $ */
  GetRegKey("System","MaxPositionCache",Opt.MaxPositionCache,64);
  if(Opt.MaxPositionCache < 16 || Opt.MaxPositionCache > 128)
    Opt.MaxPositionCache=64;
  /* $ 27.09.2000 SVS
    + Opt.AllCtrlAltShiftRule битовые флаги, задают поведение Ctrl-Alt-Shift
      По умолчанию все разрешено.
  */
  GetRegKey("System","AllCtrlAltShiftRule",Opt.AllCtrlAltShiftRule,0x0000FFFF);
  /* SVS $ */

  GetRegKey("Language","Main",Opt.Language,"English",sizeof(Opt.Language));
  GetRegKey("Language","Help",Opt.HelpLanguage,"English",sizeof(Opt.HelpLanguage));

  GetRegKey("Confirmations","Copy",Opt.Confirm.Copy,1);
  GetRegKey("Confirmations","Move",Opt.Confirm.Move,1);
  GetRegKey("Confirmations","Drag",Opt.Confirm.Drag,1);
  GetRegKey("Confirmations","Delete",Opt.Confirm.Delete,1);
  GetRegKey("Confirmations","DeleteFolder",Opt.Confirm.DeleteFolder,1);
  GetRegKey("Confirmations","Exit",Opt.Confirm.Exit,1);

  GetRegKey("Panel","ShowHidden",Opt.ShowHidden,1);
  GetRegKey("Panel","Highlight",Opt.Highlight,1);
  GetRegKey("Panel","AutoChangeFolder",Opt.AutoChangeFolder,0);
  GetRegKey("Panel","SelectFolders",Opt.SelectFolders,0);
  GetRegKey("Panel","ReverseSort",Opt.ReverseSort,1);
  /* $ 12.09.2000 SVS
    + Panel/RightClickRule в реестре - задает поведение правой клавиши
      мыши (это по поводу Bug#17)
  */
  GetRegKey("Panel","RightClickRule",Opt.PanelRightClickRule,2);
  Opt.PanelRightClickRule%=3;
  /* SVS $ */
  /* $ 19.09.2000 SVS
    + Opt.PanelCtrlAltShiftRule задает поведение Ctrl-Alt-Shift для панелей.
  */
  GetRegKey("Panel","CtrlAltShiftRule",Opt.PanelCtrlAltShiftRule,0);
  Opt.PanelCtrlAltShiftRule%=3;
  /* SVS $ */

  GetRegKey("Panel\\Left","Type",Opt.LeftPanel.Type,0);
  GetRegKey("Panel\\Left","Visible",Opt.LeftPanel.Visible,1);
  GetRegKey("Panel\\Left","Focus",Opt.LeftPanel.Focus,1);
  GetRegKey("Panel\\Left","ViewMode",Opt.LeftPanel.ViewMode,2);
  GetRegKey("Panel\\Left","SortMode",Opt.LeftPanel.SortMode,1);
  GetRegKey("Panel\\Left","SortOrder",Opt.LeftPanel.SortOrder,1);
  GetRegKey("Panel\\Left","SortGroups",Opt.LeftPanel.SortGroups,0);
  GetRegKey("Panel\\Left","ShortNames",Opt.LeftPanel.ShowShortNames,0);
  GetRegKey("Panel\\Left","Folder",Opt.LeftFolder,"",sizeof(Opt.LeftFolder));
  /* $ 07.09.2000 tran
     + Config//Current File*/
  GetRegKey("Panel\\Left","CurFile",Opt.LeftCurFile,"",sizeof(Opt.LeftCurFile));
  /* tran 07.09.2000 $ */

  GetRegKey("Panel\\Right","Type",Opt.RightPanel.Type,0);
  GetRegKey("Panel\\Right","Visible",Opt.RightPanel.Visible,1);
  GetRegKey("Panel\\Right","Focus",Opt.RightPanel.Focus,0);
  GetRegKey("Panel\\Right","ViewMode",Opt.RightPanel.ViewMode,2);
  GetRegKey("Panel\\Right","SortMode",Opt.RightPanel.SortMode,1);
  GetRegKey("Panel\\Right","SortOrder",Opt.RightPanel.SortOrder,1);
  GetRegKey("Panel\\Right","SortGroups",Opt.RightPanel.SortGroups,0);
  GetRegKey("Panel\\Right","ShortNames",Opt.RightPanel.ShowShortNames,0);
  GetRegKey("Panel\\Right","Folder",Opt.RightFolder,"",sizeof(Opt.RightFolder));
  /* $ 07.09.2000 tran
    + Config//Current File */
  GetRegKey("Panel\\Right","CurFile",Opt.RightCurFile,"",sizeof(Opt.RightCurFile));
  /* tran 07.09.2000 $ */

  GetRegKey("Panel\\Layout","ColumnTitles",Opt.ShowColumnTitles,1);
  GetRegKey("Panel\\Layout","StatusLine",Opt.ShowPanelStatus,1);
  GetRegKey("Panel\\Layout","TotalInfo",Opt.ShowPanelTotals,1);
  GetRegKey("Panel\\Layout","FreeInfo",Opt.ShowPanelFree,0);
  GetRegKey("Panel\\Layout","Scrollbar",Opt.ShowPanelScrollbar,0);
  /* $ 29.06.2000 SVS
     + Показывать ли ScrollBar для Menu
  */
  GetRegKey("Panel\\Layout","ScrollbarMenu",Opt.ShowMenuScrollbar,0);
  /* SVS $ */
  GetRegKey("Panel\\Layout","ScreensNumber",Opt.ShowScreensNumber,1);
  GetRegKey("Panel\\Layout","SortMode",Opt.ShowSortMode,1);

  GetRegKey("Layout","HeightDecrement",Opt.HeightDecrement,0);
  GetRegKey("Layout","WidthDecrement",Opt.WidthDecrement,0);
  Help::SetFullScreenMode(GetRegKey("Layout","FullscreenHelp",0));

  GetRegKey("Layout","PassiveFolder",Opt.PassiveFolder,"",sizeof(Opt.PassiveFolder));

  GetRegKey("Descriptions","ListNames",Opt.Diz.ListNames,"Descript.ion,Files.bbs",sizeof(Opt.Diz.ListNames));
  GetRegKey("Descriptions","UpdateMode",Opt.Diz.UpdateMode,DIZ_UPDATE_IF_DISPLAYED);
  GetRegKey("Descriptions","SetHidden",Opt.Diz.SetHidden,TRUE);
  GetRegKey("Descriptions","StartPos",Opt.Diz.StartPos,0);

  FileList::ReadPanelModes();
  GetTempPath(sizeof(Opt.TempPath),Opt.TempPath);
  RemoveTrailingSpaces(Opt.TempPath);
  AddEndSlash(Opt.TempPath);
  CtrlObject->EditorPosCache.Read("Editor\\LastPositions");
  CtrlObject->ViewerPosCache.Read("Viewer\\LastPositions");
  if (Opt.TabSize<1 || Opt.TabSize>512)
    Opt.TabSize=8;
  if (Opt.ViewTabSize<1 || Opt.ViewTabSize>512)
    Opt.ViewTabSize=8;
}


void SaveConfig(int Ask)
{
  char OutText[NM],OutText2[NM];

  if (Ask && Message(0,2,MSG(MSaveSetupTitle),MSG(MSaveSetupAsk1),MSG(MSaveSetupAsk2),MSG(MSaveSetup),MSG(MCancel))!=0)
    return;

  //                                       было sizeof(Palette)
  SetRegKey("Colors","CurrentPalette",Palette,SizeArrayPalette);
  SetRegKey("Screen","Clock",Opt.Clock);
  SetRegKey("Screen","ViewerEditorClock",Opt.ViewerEditorClock);
  SetRegKey("Screen","KeyBar",Opt.ShowKeyBar);
  SetRegKey("Screen","ScreenSaver",Opt.ScreenSaver);
  SetRegKey("Screen","ScreenSaverTime",Opt.ScreenSaverTime);
  SetRegKey("Screen","UsePromptFormat",Opt.UsePromptFormat);
  SetRegKey("Screen","PromptFormat",Opt.PromptFormat);

  SetRegKey("Interface","DialogsEditHistory",Opt.DialogsEditHistory);
  SetRegKey("Interface","Mouse",Opt.Mouse);
  SetRegKey("Interface","AltGr",Opt.AltGr);
  SetRegKey("Interface","CopyShowTotal",Opt.CopyShowTotal);
  SetRegKey("Interface","ShowMenuBar",Opt.ShowMenuBar);
  SetRegKey("Interface","AutoComplete",Opt.AutoComplete);

  SetRegKey("Viewer","ExternalViewerName",Opt.ExternalViewer);
  SetRegKey("Viewer","UseExternalViewer",Opt.UseExternalViewer);
  SetRegKey("Viewer","SaveViewerPos",Opt.SaveViewerPos);
  SetRegKey("Viewer","SaveViewerShortPos",Opt.SaveViewerShortPos);
  SetRegKey("Viewer","AutoDetectTable",Opt.ViewerAutoDetectTable);
  SetRegKey("Viewer","TabSize",Opt.ViewTabSize);
  /* $ 15.07.2000 tran
     + Opt.ShowKeyBarViewer */
  SetRegKey("Viewer","ShowKeyBar",Opt.ShowKeyBarViewer);
  /* tran 15.07.2000 $ */
  /* $ 18.07.2000 tran
     + Opt.ViewerShowArrows, Opt.ViewerShowScrollbar*/
  SetRegKey("Viewer","ShowArrows",Opt.ViewerShowArrows);
  SetRegKey("Viewer","ShowScrollbar",Opt.ViewerShowScrollbar);
  /* tran 18.07.2000 $ */
  /* $ 31.08.2000 SVS
     ! Opt.ViewerIsWrap
  */
  SetRegKey("Viewer","IsWrap",Opt.ViewerIsWrap);
  GetRegKey("Viewer","Wrap",Opt.ViewerWrap);
  /* SVS $*/

  SetRegKey("Editor","ExternalEditorName",Opt.ExternalEditor);
  SetRegKey("Editor","UseExternalEditor",Opt.UseExternalEditor);
  SetRegKey("Editor","ExpandTabs",Opt.EditorExpandTabs);
  SetRegKey("Editor","TabSize",Opt.TabSize);
  SetRegKey("Editor","PersistentBlocks",Opt.EditorPersistentBlocks);
  SetRegKey("Editor","DelRemovesBlocks",Opt.EditorDelRemovesBlocks);
  SetRegKey("Editor","AutoIndent",Opt.EditorAutoIndent);
  SetRegKey("Editor","SaveEditorPos",Opt.SaveEditorPos);
  SetRegKey("Editor","SaveEditorShortPos",Opt.SaveEditorShortPos);
  SetRegKey("Editor","AutoDetectTable",Opt.EditorAutoDetectTable);
  SetRegKey("Editor","EditorCursorBeyondEOL",Opt.EditorCursorBeyondEOL);

  SetRegKey("System","SaveHistory",Opt.SaveHistory);
  SetRegKey("System","SaveFoldersHistory",Opt.SaveFoldersHistory);
  SetRegKey("System","SaveViewHistory",Opt.SaveViewHistory);
  SetRegKey("System","UseRegisteredTypes",Opt.UseRegisteredTypes);
  SetRegKey("System","AutoSaveSetup",Opt.AutoSaveSetup);
  /* $ 15.07.2000 SVS
     Сохраняем путь для дополнительного поиска плагинов
  */
  SetRegKey("System","PersonalPluginsPath",Opt.PersonalPluginsPath);
  /* SVS $ */
  SetRegKey("System","ClearReadOnly",Opt.ClearReadOnly);
  SetRegKey("System","DeleteToRecycleBin",Opt.DeleteToRecycleBin);
  SetRegKey("System","UseSystemCopy",Opt.UseSystemCopy);
  SetRegKey("System","CopyOpened",Opt.CopyOpened);
  SetRegKey("System","CreateUppercaseFolders",Opt.CreateUppercaseFolders);
  SetRegKey("System","InactivityExit",Opt.InactivityExit);
  SetRegKey("System","InactivityExitTime",Opt.InactivityExitTime);
  SetRegKey("System","DriveMenuMode",Opt.ChangeDriveMode);
  SetRegKey("System","FileSearchMode",Opt.FileSearchMode);
  SetRegKey("System","FolderInfo",Opt.FolderInfoFiles);
  SetRegKey("System","SubstPluginPrefix",Opt.SubstPluginPrefix);

  SetRegKey("Language","Main",Opt.Language);
  SetRegKey("Language","Help",Opt.HelpLanguage);

  SetRegKey("Confirmations","Copy",Opt.Confirm.Copy);
  SetRegKey("Confirmations","Move",Opt.Confirm.Move);
  SetRegKey("Confirmations","Drag",Opt.Confirm.Drag);
  SetRegKey("Confirmations","Delete",Opt.Confirm.Delete);
  SetRegKey("Confirmations","DeleteFolder",Opt.Confirm.DeleteFolder);
  SetRegKey("Confirmations","Exit",Opt.Confirm.Exit);

  SetRegKey("Panel","ShowHidden",Opt.ShowHidden);
  SetRegKey("Panel","Highlight",Opt.Highlight);
  SetRegKey("Panel","AutoChangeFolder",Opt.AutoChangeFolder);
  SetRegKey("Panel","SelectFolders",Opt.SelectFolders);
  SetRegKey("Panel","ReverseSort",Opt.ReverseSort);

  Panel *LeftPanel=CtrlObject->LeftPanel;
  SetRegKey("Panel\\Left","Visible",LeftPanel->IsVisible());
  SetRegKey("Panel\\Left","Focus",LeftPanel->GetFocus());
  if (LeftPanel->GetMode()==NORMAL_PANEL)
  {
    SetRegKey("Panel\\Left","Type",LeftPanel->GetType());
    SetRegKey("Panel\\Left","ViewMode",LeftPanel->GetViewMode());
    SetRegKey("Panel\\Left","SortMode",LeftPanel->GetSortMode());
    SetRegKey("Panel\\Left","SortOrder",LeftPanel->GetSortOrder());
    SetRegKey("Panel\\Left","SortGroups",LeftPanel->GetSortGroups());
    SetRegKey("Panel\\Left","ShortNames",LeftPanel->GetShowShortNamesMode());
    LeftPanel->GetCurDir(OutText);
    SetRegKey("Panel\\Left","Folder",OutText);
    /* $ 07.09.2000 tran
       + Config//Current File */
    LeftPanel->GetCurName(OutText,OutText2);
    SetRegKey("Panel\\Left","CurFile",OutText);
    /* tran 07.09.2000 $ */
  }

  Panel *RightPanel=CtrlObject->RightPanel;
  SetRegKey("Panel\\Right","Visible",RightPanel->IsVisible());
  SetRegKey("Panel\\Right","Focus",RightPanel->GetFocus());
  if (RightPanel->GetMode()==NORMAL_PANEL)
  {
    SetRegKey("Panel\\Right","Type",RightPanel->GetType());
    SetRegKey("Panel\\Right","ViewMode",RightPanel->GetViewMode());
    SetRegKey("Panel\\Right","SortMode",RightPanel->GetSortMode());
    SetRegKey("Panel\\Right","SortOrder",RightPanel->GetSortOrder());
    SetRegKey("Panel\\Right","SortGroups",RightPanel->GetSortGroups());
    SetRegKey("Panel\\Right","ShortNames",RightPanel->GetShowShortNamesMode());
    RightPanel->GetCurDir(OutText);
    SetRegKey("Panel\\Right","Folder",OutText);
    /* $ 07.09.2000 tran
       + Config//Current File*/
    RightPanel->GetCurName(OutText,OutText2);
    SetRegKey("Panel\\Right","CurFile",OutText);
    /* tran 07.09.2000 $ */
  }

  SetRegKey("Panel\\Layout","ColumnTitles",Opt.ShowColumnTitles);
  SetRegKey("Panel\\Layout","StatusLine",Opt.ShowPanelStatus);
  SetRegKey("Panel\\Layout","TotalInfo",Opt.ShowPanelTotals);
  SetRegKey("Panel\\Layout","FreeInfo",Opt.ShowPanelFree);
  SetRegKey("Panel\\Layout","Scrollbar",Opt.ShowPanelScrollbar);
  /* $ 29.06.2000 SVS
     + Показывать ли ScrollBar для Menu
  */
  SetRegKey("Panel\\Layout","ScrollbarMenu",Opt.ShowMenuScrollbar);
  /* SVS $ */
  SetRegKey("Panel\\Layout","ScreensNumber",Opt.ShowScreensNumber);
  SetRegKey("Panel\\Layout","SortMode",Opt.ShowSortMode);

  SetRegKey("Layout","HeightDecrement",Opt.HeightDecrement);
  SetRegKey("Layout","WidthDecrement",Opt.WidthDecrement);
  SetRegKey("Layout","FullscreenHelp",Help::GetFullScreenMode());

  CtrlObject->GetAnotherPanel(CtrlObject->ActivePanel)->GetCurDir(OutText);
  SetRegKey("Layout","PassiveFolder",OutText);

  SetRegKey("Descriptions","ListNames",Opt.Diz.ListNames);
  SetRegKey("Descriptions","UpdateMode",Opt.Diz.UpdateMode);
  SetRegKey("Descriptions","SetHidden",Opt.Diz.SetHidden);
  SetRegKey("Descriptions","StartPos",Opt.Diz.StartPos);

  PanelFilter::SaveSelection();
  FileList::SavePanelModes();
  if (Ask)
    CtrlObject->Macro.SaveMacros();
}
