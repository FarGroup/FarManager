/*
config.cpp

Конфигурация

*/

/* Revision: 1.124 24.01.2002 $ */

/*
Modify:
  24.01.2002 SVS
    - BugZ#264 - Show hidden files
  23.01.2002 SVS
    ! Нафиг по умолчанию отрубим ScreenSaver ну и некоторые другие.
  16.01.2002 SVS
    ! SEARCH_ROOT -> SEARCH_FROM_CURRENT
  03.01.2001 IS
    ! Устранение "двойного отрицания" в EditorConfig
  03.01.2002 SVS
    - BugZ#220 - Auto save setup нельзя включить
  27.12.2001 IS
    ! По умолчанию CloseConsoleRule=1, т.е. если автосохранение включено, то
      FAR попытается сохранить настройки, если его закрыли "по кресту".
      Сделано в соответствии с мнением большинства (голосование в farbugs).
  26.12.2001 SVS
    + Opt.CloseConsoleRule
    ! При закрытии окна "по кресту"... теперь настраиваемо! По умолчанию - как
      и раньше - без сохранения!
    + Обработка RO-файлов описаний
    + Свой курсор для Overtype режима
  21.12.2001 SVS
    + Opt.RestoreCPAfterExecute
    ! AutoSave в диалоге настройки системы перенесем ниже - так логичнее будет.
  17.12.2001 IS
    + Работа с PanelMiddleClickRule
  14.12.2001 SVS
    + Вызовем SaveHiData() в SaveConfig() для сохранения раскраски
  10.12.2001 SVS
    ! DM_SETTEXTLENGTH -> DM_SETMAXTEXTLENGTH
  07.12.2001 IS
    - BugZ#154 - неверно отображается диалог Option->Panel setting в
      нерегистреном фаре.
    - Забыли дисаблить редактор DLG_PANEL_AUTOUPDATELIMITVAL, если
      Opt.AutoUpdateLimit = 0
  07.12.2001 IS
    + Работа с Opt.MultiMakeDir
    ! Opt.MultiCopy хранится не в NKeyInterface, а в NKeySystem
  03.12.2001 IS
    + Считываем Opt.EditorUndoSize - размер буфера undo
  27.11.2001 SVS
    ! В при сохранении настроек запоминаем каталог и текущий объект в любом
      случае, независимо от типа панели - плагин или простая панель
  27.11.2001 DJ
    + параметр Local у EditorConfig и ViewerConfig (при вызове настроек
      конкретного редактора или вьюера не показываем глобальные
      системные настройки)
  21.11.2001 SVS
    ! Внедрение DIF_AUTOMATION (с удаленем диалоговых процедур)
  20.11.2001 SVS
    ! Перетасовка в некоторых настройках
    + Логика блокировок в диалогах (DIF_DISABLE)
  16.11.2001 SVS
    ! уточняем системную политику (добавлена ветка в HKLM)
      - для дисков HKCU может только отменять показ
      - для опций HKCU может только добавлять блокироку пунктов
  01.11.2001 SVS
    ! уберем Opt.CPAJHefuayor ;-(
  30.10.2001 SVS
    + Opt.CPAJHefuayor - по умолчанию отключено!
  29.10.2001 IS
    !  SaveEditorPos и SaveEditorShortPos переехали в EditorOptions
  24.10.2001 KM
    + Opt.FindFolders. Запомнить флаг разрешения поиска каталогов в Alt-F7
  15.10.2001 SVS
    + Opt.DlgSelectFromHistory
  12.10.2001 SVS
    ! Ну охренеть (Opt.FolderSetAttr165!!!) - уже и так есть то, что надо:
      Opt.SetAttrFolderRules!
  11.10.2001 SVS
    + Opt.FolderSetAttr165; // поведение для каталогов как у 1.65
  07.10.2001 SVS
    + Opt.HelpTabSize - размер табуляции по умолчанию.
  02.10.2001 SVS
    - В таблице однажды забыл для Opt.AltF9 выставить дефолт =-1
  27.09.2001 IS
    - Левый размер при использовании strncpy
  26.09.2001 SVS
    + Opt.AutoUpdateLimit -  выше этого количество не обновлять панели.
      По умолчанию = 0 (всегда делать автоапдейт)
  16.09.2001 SVS
    ! Opt.ExceptCallDebugger удален в связи со сменой статуса опции
      Opt.ExceptRules
  09.09.2001 IS
    + обновим настройки ком.строки при выходе из Interface settings, чтобы
      установился правильный режим "постоянные блоки"
  08.09.2001 VVM
    + Настройка для блоков в строках ввода F9/Options/Interface settings
  09.08.2001 OT
    - F9|Options|Panel settings|Highlight files - не перерисовывается
  03.08.2001 IS
    + Учтем опцию "разрешить мультикопирование/перемещение/создание связей".
      По умолчанию она отключена.
  24.07.2001 SVS
    + Учтем новую опцию Opt.Confirm.HistoryClear при очистки истории
    + Opt.PgUpChangeDisk
  22.07.2001 SVS
    + Опция про релоад файла в редакторе
  19.07.2001 SVS
    ! Про хелп. Юзаем то, что есть в Opt и не пладим сущностей.
  19.07.2001 OT
    ! AltF9 - к первоначальному положению :)
  17.07.2001 SVS
    ! Opt.AltF9 - уточнение поведения для разных платформ
  16.07.2001 SVS
    ! Opt.AltF9 - по умолчанию = 0, т.е. отключено (подробнее см. описалово)
  10.07.2001 SKV
    ! Redraw после изменения конфига.
  04.07.2001 SVS
    + Opt.LCIDSort
  24.06.2001 KM
    ! Вернулся назад дефолт открытия read-only файлов: не предупреждать и не блокировать,
      уж слишком много копий было сломано из-за этого в последнее время.
  22.06.2001 SVS
    + Opt.DateFormat
  14.06.2001 OT
    ! "Бунт" ;-)
  28.05.2001 SVS
    ! Основные XLat-клавиши по умолчанию не ставятся в CtrlShiftX - дадим
      возможность отключить эту фичу.
  24.05.2001 SVS
    ! Задание размера табуляции в EditorConfig перенесено ниже (так лучше
      смотрится :-)
  21.05.2001 OT
    + Opt.AltF9
    + Opt.Confirmation.AllowReedit
  20.05.2001 IS
    + Задаем поведение для файлов с атрибутом r/o в настройках редактора
       лочить или нет
       орать или нет
    + Opt.ReadOnlyLock теперь сохраняется в реестре
  18.05.2001 DJ
    + #include "colors.hpp"
  14.05.2001 SVS
    + Opt.ShowCheckingFile - щоб управлять мельканием в заголовке...
      По умолчанию - отлючено
  06.05.2001 DJ
    ! перетрях #include
  29.04.2001 ОТ
    + Внедрение NWZ от Третьякова
  30.04.2001 DJ
    * не было history в SetFolderInfoFiles; не обновлялись инфо-панели
      после его изменения
  28.04.2001 VVM
    + Opt.SubstNameRule битовая маска:
      0 - если установлен, то опрашивать сменные диски при GetSubstName()
      1 - если установлен, то опрашивать все остальные при GetSubstName()
  16.04.2001 VVM
    + Opt.MsWheelDeltaView - задает смещение для прокрутки вьюера.
    + Opt.MsWheelDeltaEdit - задает смещение для прокрутки редактора.
    ! Opt.MouseWheelDelta -> Opt.MsWheelDelta
  22.04.2001 SVS
    + Opt.QuotedSymbols - разделители для QuoteSpace()
    ! ConsoleDetachKey - по умолчанию назначается "CtrlAltTab"
  16.04.2001 VVM
    + Opt.MouseWheelDelta - задает смещение для прокрутки. Сколько раз посылать UP/DOWN
  02.04.2001 VVM
    + Opt.FlagPosixSemantics будет влиять на:
        добавление файлов в историю с разным регистром
        добавление LastPositions в редакторе и вьюере
  30.03.2001 SVS
    + Opt.Policies.*
    ! ViewerConfig, EditorConfig - ограничение на размер поля ввода
  29.03.2001 IS
    + По аналогии с редактором часть настроек переехала в ViewerOptions,
      соответственно произведены замены по всему файлу типа
      Opt.ViewerTabSize -> Opt.ViOpt.TabSize и т.п.
  28.03.2001 VVM
    + Opt.RememberLogicalDrives = запоминать логические диски и не опрашивать
      каждый раз. Для предотвращения "просыпания" "зеленых" винтов.
  26.03.2001 SVS
    + SystemSettings() - путь к персональным плагинам - DIF_VAREDIT
    ! Выставляем ограничение в SetDizConfig() в sizeof(DizOptions.ListNames)
      для поля ввода.
  20.03.2001 SVS
    + В стандартные раздилители WordDiv0 добавлена тильда '~'
  20.03.2001 SVS
    ! основательная переделка SaveConfig и ReadConfig: введена структура
      FARConfig в которой описывается ВСЕ, что проходит по линии реестра
      (в основном все)
  16.03.2001 SVS
    ! В конфирм-диалоге операция Exit должна по смыслу стоять последней
    + Opt.ChangeDriveDisconnetMode
  15.03.2001 SVS
    + Opt.Confirm.RemoveConnection - подтверждение для удаления мапленных дисков
  12.03.2001 SVS
    + Opt.DeleteSymbolWipe -> Opt.WipeSymbol
  12.03.2001 SVS
    + Opt.DeleteSymbolWipe символ заполнитель для "ZAP-операции"
  27.02.2001 SVS
    + Opt.EdOpt.CharCodeBase - В каком виде представлять в редакторе
      в статусной строке код текущего символа
  26.02.2001 IS
    - Недочет в EditorConfig
  26.02.2001 VVM
    + Opt.ExceptCallDebugger
  21.02.2001 IS
    + Работа в EditorConfig идет со структурой EditorOptions
    ! Opt.EditorBSLikeDel -> Opt.EdOpt.BSLikeDel
      Opt.TabSize -> Opt.EdOpt.TabSize
      Opt.EditorExpandTabs -> Opt.EdOpt.ExpandTabs
      Opt.EditorCursorBeyondEOL -> Opt.EdOpt.CursorBeyondEOL
      Opt.EditorAutoDetectTable -> Opt.EdOpt.AutoDetectTable
      Opt.EditorAutoIndent -> Opt.EdOpt.AutoIndent
      Opt.EditorPersistentBlocks -> Opt.EdOpt.PersistentBlocks
      Opt.EditorDelRemovesBlocks -> Opt.EdOpt.DelRemovesBlocks
  20.02.2001 VVM
    ! Сохранение параметра "Тип врапа"
  12.02.2001 SKV
    + ConsoleDetachKey
  09.02.2001 IS
    + сохраним/считаем состояние опции "помеченное вперед"
    + Опция подтверждения нажатия Esc. По умолчанию отключена.
  30.01.2001 VVM
    + Показывает время копирования,оставшееся время и среднюю скорость.
      Зависит от настроек в реестре CopyTimeRule
  22.01.2001 SVS
    + Opt.CursorSize - Размер курсора ФАРа :-)
  19.01.2001 SVS
    + Opt.MacroReuseRules - Правило на счет повторно использования забинденных
      клавиш
  17.01.2001 SVS
    ! Opt.ShiftsKeyRules
  07.01.2001 SVS
    ! Opt.EditorReadOnlyLock = 2, т.е. выдавать предупреждение.
  16.12.2000 IS
    - баг: забыли считать опцию DLG_VIEW_AUTODETECT из диалога
  13.12.2000 SVS
    ! Уточняем алгоритм "взятия" палитры.
  10.12.2000 IS
    ! Убрал из WordDivForXlat кавычки и квадратные скобки
  29.11.2000 SVS
    + Opt.EditorReadOnlyLock - лочить файл при открытии в редакторе, если
      он имеет атрибуты R|S|H
    + Opt.EditorFileSizeLimit - минимально допустимый размер файла, после
      которого будет выдан диалог о целесообразности открытия подобного
      файла на редактирование
  28.11.2000 SVS
    + Opt.EditorF7Rules - Правило на счет поиска в редакторе
  27.11.2000 SVS
    + Opt.ExceptRules - Правило на счет вызова исключений
  25.11.2000 IS
    + Стандартный набор разделителей для функции Xlat (WordDivForXlat)
  24.11.2000 SVS
    - Проблема с Alt* при XLat
    + SetAttrFolderRules задает поведение Ctrl-A на каталоге:
      1 - отображать со снятой опцией "для подкаталогов" (по умолчанию)
      0 - как и ранее
  16.11.2000 SVS
    ! Клавиши, вызывающие Xlat - теперь хранятся в реестре в текстовом виде
  05.11.2000 SVS
    - В настройках вьювера вместо Opt.SaveViewerShortPos стоялО
      Opt.SaveEditorShortPos :-(((
  04.11.2000 SVS
    + XLat - добавление альтернативных клавиш:
        XLatAltEditorKey, XLatAltCmdLineKey, XLatAltDialogKey;
  20.10.2000 SVS
    + Opt.PanelCtrlFRule
      Panel/CtrlFRule в реестре - задает поведение Ctrl-F
      Если = 0, то штампуется файл как есть, иначе - с учетом
      отображения на панели
  17.10.2000 SVS
    ! WordDiv имеет размер 256;
  16.10.2000 SVS
    ! System\CopyOpened по умолчанию установлен в 1 (разрешен)
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

#include "global.hpp"
#include "lang.hpp"
#include "fn.hpp"
#include "keys.hpp"
#include "colors.hpp"
#include "cmdline.hpp"
#include "ctrlobj.hpp"
#include "dialog.hpp"
#include "filepanels.hpp"
#include "filelist.hpp"
#include "panel.hpp"
#include "help.hpp"
#include "filter.hpp"
#include "poscache.hpp"
#include "findfile.hpp"
#include "hilight.hpp"

/* $ 03.08.2000 SVS
   Стандартный набор разделителей
*/
static char WordDiv0[257]="~!%^&*()+|{}:\"<>?`-=\\[];',./";
/* SVS $ */

/* $ 12.10.2000 IS
   Стандартный набор разделителей для функции Xlat
*/
static char WordDivForXlat0[257]=" \t!#$%^&*()+|=\\/@?";
/* IS $ */

char PersonalPluginsPath[1024];
char KeyNameConsoleDetachKey[64];
char szCtrlShiftX[]="CtrlShiftX";

// KeyName
char NKeyColors[]="Colors";
char NKeyScreen[]="Screen";
char NKeyInterface[]="Interface";
char NKeyViewer[]="Viewer";
char NKeyDialog[]="Dialog";
char NKeyEditor[]="Editor";
char NKeyXLat[]="XLat";
char NKeySystem[]="System";
char NKeySystemExecutor[]="System\\Executor";
char NKeyHelp[]="Help";
char NKeyLanguage[]="Language";
char NKeyConfirmations[]="Confirmations";
char NKeyPanel[]="Panel";
char NKeyPanelLeft[]="Panel\\Left";
char NKeyPanelRight[]="Panel\\Right";
char NKeyPanelLayout[]="Panel\\Layout";
char NKeyLayout[]="Layout";
char NKeyDescriptions[]="Descriptions";
char NKeyKeyMacros[]="KeyMacros";
char NKeyPolicies[]="Policies";

void SystemSettings()
{
  const char *HistoryName="PersPath";
  char PersonalPluginsPath[sizeof(Opt.PersonalPluginsPath)];
  /* $ 15.07.2000 SVS
     + Добавка в виде задания дополнительного пути для поиска плагинов
  */
  static struct DialogData CfgDlgData[]={
  /*  0 */  DI_DOUBLEBOX,3,1,52,18,0,0,0,0,(char *)MConfigSystemTitle,
  /*  1 */  DI_CHECKBOX,5,2,0,0,1,0,0,0,(char *)MConfigRO,
  /*  2 */  DI_CHECKBOX,5,3,0,0,0,0,0,0,(char *)MConfigRecycleBin,
  /*  3 */  DI_CHECKBOX,5,4,0,0,0,0,0,0,(char *)MConfigSystemCopy,
  /*  4 */  DI_CHECKBOX,5,5,0,0,0,0,0,0,(char *)MConfigCopySharing,
  /*  5 */  DI_CHECKBOX,5,6,0,0,0,0,0,0,(char *)MConfigCreateUppercaseFolders,
  /*  6 */  DI_CHECKBOX,5,7,0,0,0,0,DIF_AUTOMATION,0,(char *)MConfigInactivity,
  /*  7 */  DI_FIXEDIT,9,8,11,6,0,0,0,0,"",
  /*  8 */  DI_TEXT,13,8,0,0,0,0,0,0,(char *)MConfigInactivityMinutes,
  /*  9 */  DI_CHECKBOX,5,9,0,0,0,0,0,0,(char *)MConfigSaveHistory,
  /* 10 */  DI_CHECKBOX,5,10,0,0,0,0,0,0,(char *)MConfigSaveFoldersHistory,
  /* 11 */  DI_CHECKBOX,5,11,0,0,0,0,0,0,(char *)MConfigSaveViewHistory,
  /* 12 */  DI_CHECKBOX,5,12,0,0,0,0,0,0,(char *)MConfigRegisteredTypes,
  /* 13 */  DI_TEXT,5,13,0,0,0,0,0,0,(char *)MConfigPersonalPath,
  /* 14 */  DI_EDIT,5,14,50,14,0,(DWORD)HistoryName,DIF_HISTORY|DIF_VAREDIT,0,"",
  /* 15 */  DI_CHECKBOX,5,15,0,0,0,0,0,0,(char *)MConfigAutoSave,
  /* 16 */  DI_TEXT,5,16,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
  /* 17 */  DI_BUTTON,0,17,0,0,0,0,DIF_CENTERGROUP,1,(char *)MOk,
  /* 18 */  DI_BUTTON,0,17,0,0,0,0,DIF_CENTERGROUP,0,(char *)MCancel
  };
  MakeDialogItems(CfgDlgData,CfgDlg);

  CfgDlg[1].Selected=Opt.ClearReadOnly;
  CfgDlg[2].Selected=Opt.DeleteToRecycleBin;
  CfgDlg[3].Selected=Opt.UseSystemCopy;
  CfgDlg[4].Selected=Opt.CopyOpened;
  CfgDlg[5].Selected=Opt.CreateUppercaseFolders;

  CfgDlg[6].Selected=Opt.InactivityExit;
  sprintf(CfgDlg[7].Data,"%d",Opt.InactivityExitTime);
  if(!Opt.InactivityExit)
  {
    CfgDlg[7].Flags|=DIF_DISABLE;
    CfgDlg[8].Flags|=DIF_DISABLE;
  }

  CfgDlg[9].Selected=Opt.SaveHistory;
  CfgDlg[10].Selected=Opt.SaveFoldersHistory;
  CfgDlg[11].Selected=Opt.SaveViewHistory;
  CfgDlg[12].Selected=Opt.UseRegisteredTypes;

  strcpy(PersonalPluginsPath,Opt.PersonalPluginsPath);
  CfgDlg[14].Ptr.PtrData=PersonalPluginsPath;
  CfgDlg[14].Ptr.PtrLength=sizeof(PersonalPluginsPath);
  CfgDlg[15].Selected=Opt.AutoSaveSetup;

  {
    Dialog Dlg(CfgDlg,sizeof(CfgDlg)/sizeof(CfgDlg[0]));
    Dlg.SetHelp("SystemSettings");
    Dlg.SetPosition(-1,-1,56,20);
    Dlg.SetAutomation(6,7,DIF_DISABLE,0,0,DIF_DISABLE);
    Dlg.SetAutomation(6,8,DIF_DISABLE,0,0,DIF_DISABLE);
    Dlg.Process();
    if (Dlg.GetExitCode()!=17)
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
  Opt.AutoSaveSetup=CfgDlg[15].Selected;
  strcpy(Opt.PersonalPluginsPath,PersonalPluginsPath);
  /* SVS $ */
}


#define DLG_PANEL_HIDDEN              1
#define DLG_PANEL_HIGHLIGHT           2
#define DLG_PANEL_CHANGEFOLDER        3
#define DLG_PANEL_SELECTFOLDERS       4
#define DLG_PANEL_REVERSESORT         5
#define DLG_PANEL_AUTOUPDATELIMIT     6
#define DLG_PANEL_AUTOUPDATELIMIT2    7
#define DLG_PANEL_AUTOUPDATELIMITVAL  8

#define DLG_PANEL_SHOWCOLUMNTITLES   10
#define DLG_PANEL_SHOWPANELSTATUS    11
#define DLG_PANEL_SHOWPANELTOTALS    12
#define DLG_PANEL_SHOWPANELFREE      13
#define DLG_PANEL_SHOWPANELSCROLLBAR 14
#define DLG_PANEL_SHOWSCREENSNUMBER  15
#define DLG_PANEL_SHOWSORTMODE       16
#define DLG_PANEL_OK                 18

void PanelSettings()
{
  static struct DialogData CfgDlgData[]={
  /* 00 */DI_DOUBLEBOX,3,1,52,19,0,0,0,0,(char *)MConfigPanelTitle,
  /* 01 */DI_CHECKBOX,5,2,0,0,1,0,0,0,(char *)MConfigHidden,
  /* 02 */DI_CHECKBOX,5,3,0,0,0,0,0,0,(char *)MConfigHighlight,
  /* 03 */DI_CHECKBOX,5,4,0,0,0,0,0,0,(char *)MConfigAutoChange,
  /* 04 */DI_CHECKBOX,5,5,0,0,0,0,0,0,(char *)MConfigSelectFolders,
  /* 05 */DI_CHECKBOX,5,6,0,0,0,0,0,0,(char *)MConfigReverseSort,
  /* 06 */DI_CHECKBOX,5,7,0,0,0,0,DIF_AUTOMATION,0,(char *)MConfigAutoUpdateLimit,
  /* 07 */DI_TEXT,9,8,0,0,0,0,0,0,(char *)MConfigAutoUpdateLimit2,
  /* 08 */DI_EDIT,9,8,15,8,0,0,0,0,"",
  /* 09 */DI_TEXT,3,9,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
  /* 10 */DI_CHECKBOX,5,10,0,0,0,0,0,0,(char *)MConfigShowColumns,
  /* 11 */DI_CHECKBOX,5,11,0,0,0,0,0,0,(char *)MConfigShowStatus,
  /* 12 */DI_CHECKBOX,5,12,0,0,0,0,0,0,(char *)MConfigShowTotal,
  /* 13 */DI_CHECKBOX,5,13,0,0,0,0,0,0,(char *)MConfigShowFree,
  /* 14 */DI_CHECKBOX,5,14,0,0,0,0,0,0,(char *)MConfigShowScrollbar,
  /* 15 */DI_CHECKBOX,5,15,0,0,0,0,0,0,(char *)MConfigShowScreensNumber,
  /* 16 */DI_CHECKBOX,5,16,0,0,0,0,0,0,(char *)MConfigShowSortMode,
  /* 17 */DI_TEXT,3,17,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
  /* 18 */DI_BUTTON,0,18,0,0,0,0,DIF_CENTERGROUP,1,(char *)MOk,
  /* 19 */DI_BUTTON,0,18,0,0,0,0,DIF_CENTERGROUP,0,(char *)MCancel
  };
  MakeDialogItems(CfgDlgData,CfgDlg);

  CfgDlg[DLG_PANEL_HIDDEN].Selected=Opt.ShowHidden;
  CfgDlg[DLG_PANEL_HIGHLIGHT].Selected=Opt.Highlight;
  CfgDlg[DLG_PANEL_CHANGEFOLDER].Selected=Opt.AutoChangeFolder;
  CfgDlg[DLG_PANEL_SELECTFOLDERS].Selected=Opt.SelectFolders;
  CfgDlg[DLG_PANEL_REVERSESORT].Selected=Opt.ReverseSort;

  CfgDlg[DLG_PANEL_SHOWCOLUMNTITLES].Selected=Opt.ShowColumnTitles;
  CfgDlg[DLG_PANEL_SHOWPANELSTATUS].Selected=Opt.ShowPanelStatus;
  CfgDlg[DLG_PANEL_SHOWPANELTOTALS].Selected=Opt.ShowPanelTotals;
  CfgDlg[DLG_PANEL_SHOWPANELFREE].Selected=Opt.ShowPanelFree;
  CfgDlg[DLG_PANEL_SHOWPANELSCROLLBAR].Selected=Opt.ShowPanelScrollbar;
  CfgDlg[DLG_PANEL_SHOWSCREENSNUMBER].Selected=Opt.ShowScreensNumber;
  CfgDlg[DLG_PANEL_SHOWSORTMODE].Selected=Opt.ShowSortMode;
  CfgDlg[DLG_PANEL_AUTOUPDATELIMITVAL].X1+=strlen(MSG(MConfigAutoUpdateLimit2))+1;
  CfgDlg[DLG_PANEL_AUTOUPDATELIMITVAL].X2+=strlen(MSG(MConfigAutoUpdateLimit2))+1;
  CfgDlg[DLG_PANEL_AUTOUPDATELIMIT].Selected=Opt.AutoUpdateLimit!=0;
  if (!RegVer)
  {
    CfgDlg[DLG_PANEL_AUTOUPDATELIMITVAL].Type=DI_TEXT;
    CfgDlg[DLG_PANEL_AUTOUPDATELIMIT].Type=DI_TEXT;
    sprintf(CfgDlg[DLG_PANEL_AUTOUPDATELIMIT].Data," *  %s",MSG(MRegOnlyShort));
    CfgDlg[DLG_PANEL_AUTOUPDATELIMIT2].Data[0]=0;
    CfgDlg[DLG_PANEL_AUTOUPDATELIMITVAL].Data[0]=0;
  }
  else
    ultoa(Opt.AutoUpdateLimit,CfgDlg[DLG_PANEL_AUTOUPDATELIMITVAL].Data,10);

  if(Opt.AutoUpdateLimit==0)
    CfgDlg[DLG_PANEL_AUTOUPDATELIMITVAL].Flags|=DIF_DISABLE;

  {
    Dialog Dlg(CfgDlg,sizeof(CfgDlg)/sizeof(CfgDlg[0]));
    Dlg.SetHelp("PanelSettings");
    Dlg.SetPosition(-1,-1,56,21);
    Dlg.SetAutomation(DLG_PANEL_AUTOUPDATELIMIT,DLG_PANEL_AUTOUPDATELIMITVAL,DIF_DISABLE,0,0,DIF_DISABLE);
    Dlg.Process();
    if (Dlg.GetExitCode() != DLG_PANEL_OK)
      return;
  }

  Opt.ShowHidden=CfgDlg[DLG_PANEL_HIDDEN].Selected;
  Opt.Highlight=CfgDlg[DLG_PANEL_HIGHLIGHT].Selected;
  Opt.AutoChangeFolder=CfgDlg[DLG_PANEL_CHANGEFOLDER].Selected;
  Opt.SelectFolders=CfgDlg[DLG_PANEL_SELECTFOLDERS].Selected;
  Opt.ReverseSort=CfgDlg[DLG_PANEL_REVERSESORT].Selected;
  if(!CfgDlg[DLG_PANEL_AUTOUPDATELIMIT].Selected)
    Opt.AutoUpdateLimit=0;
  else
  {
    char *endptr;
    Opt.AutoUpdateLimit=strtoul(CfgDlg[DLG_PANEL_AUTOUPDATELIMITVAL].Data, &endptr, 10);
  }
  Opt.ShowColumnTitles=CfgDlg[DLG_PANEL_SHOWCOLUMNTITLES].Selected;
  Opt.ShowPanelStatus=CfgDlg[DLG_PANEL_SHOWPANELSTATUS].Selected;
  Opt.ShowPanelTotals=CfgDlg[DLG_PANEL_SHOWPANELTOTALS].Selected;
  Opt.ShowPanelFree=CfgDlg[DLG_PANEL_SHOWPANELFREE].Selected;
  Opt.ShowPanelScrollbar=CfgDlg[DLG_PANEL_SHOWPANELSCROLLBAR].Selected;
  Opt.ShowScreensNumber=CfgDlg[DLG_PANEL_SHOWSCREENSNUMBER].Selected;
  Opt.ShowSortMode=CfgDlg[DLG_PANEL_SHOWSORTMODE].Selected;
//  FrameManager->RefreshFrame();
  CtrlObject->Cp()->LeftPanel->Update(UPDATE_KEEP_SELECTION);
  CtrlObject->Cp()->RightPanel->Update(UPDATE_KEEP_SELECTION);
  CtrlObject->Cp()->Redraw();

}


#define DLG_INTERF_CLOCK                1
#define DLG_INTERF_VIEWEREDITORCLOCK    2
#define DLG_INTERF_MOUSE                3
#define DLG_INTERF_MOUSEPMCLICKRULE     4
#define DLG_INTERF_SHOWKEYBAR           5
#define DLG_INTERF_SHOWMENUBAR          6
#define DLG_INTERF_SCREENSAVER          7
#define DLG_INTERF_SCREENSAVERTIME      8
#define DLG_INTERF_SAVERMINUTES         9
#define DLG_INTERF_DIALOGSEDITHISTORY  10
#define DLG_INTERF_DIALOGSEDITBLOCK    11
#define DLG_INTERF_USEPROMPTFORMAT     12
#define DLG_INTERF_PROMPTFORMAT        13
#define DLG_INTERF_ALTGR               14
#define DLG_INTERF_COPYSHOWTOTAL       15
#define DLG_INTERF_COPYTIMERULE        16
#define DLG_INTERF_AUTOCOMPLETE        17
#define DLG_INTERF_PGUPCHANGEDISK      18
#define DLG_INTERF_OK                  20

/* $ 17.12.2001 IS
   Настройка средней кнопки мыши для панелей. Воткнем пока сюда, потом надо
   переехать в специальный диалог по программированию мыши.
*/
void InterfaceSettings()
{
  static struct DialogData CfgDlgData[]={
    /* $ 04.07.2000 SVS
       + Показывать ли ScrollBar для Menu|Options|Interface settings
    */
    /* $ 26.07.2000 SVS
       + Разрешить ли автодополнение в строках ввода
    */
  /* 00 */DI_DOUBLEBOX,3,1,54,21,0,0,0,0,(char *)MConfigInterfaceTitle,
  /* 01 */DI_CHECKBOX,5,2,0,0,1,0,0,0,(char *)MConfigClock,
  /* 02 */DI_CHECKBOX,5,3,0,0,0,0,0,0,(char *)MConfigViewerEditorClock,
  /* 03 */DI_CHECKBOX,5,4,0,0,0,0,DIF_AUTOMATION,0,(char *)MConfigMouse,
  /* 04 */DI_CHECKBOX,9,5,0,0,0,0,0,0,(char *)MConfigMousePanelMClickRule,
  /* 05 */DI_CHECKBOX,5,6,0,0,0,0,0,0,(char *)MConfigKeyBar,
  /* 06 */DI_CHECKBOX,5,7,0,0,0,0,0,0,(char *)MConfigMenuBar,
  /* 07 */DI_CHECKBOX,5,8,0,0,0,0,DIF_AUTOMATION,0,(char *)MConfigSaver,
  /* 08 */DI_FIXEDIT,9,9,11,8,0,0,0,0,"",
  /* 09 */DI_TEXT,13,9,0,0,0,0,0,0,(char *)MConfigSaverMinutes,
  /* 10 */DI_CHECKBOX,5,10,0,0,0,0,0,0,(char *)MConfigDialogsEditHistory,
  /* 11 */DI_CHECKBOX,5,11,0,0,0,0,0,0,(char *)MConfigDialogsEditBlock,
  /* 12 */DI_CHECKBOX,5,12,0,0,0,0,DIF_AUTOMATION,0,(char *)MConfigUsePromptFormat,
  /* 13 */DI_EDIT,9,13,24,12,0,0,0,0,"",
  /* 14 */DI_CHECKBOX,5,14,0,0,0,0,0,0,(char *)MConfigAltGr,
  /* 15 */DI_CHECKBOX,5,15,0,0,0,0,0,0,(char *)MConfigCopyTotal,
  /* 16 */DI_CHECKBOX,5,16,0,0,0,0,0,0,(char *)MConfigCopyTimeRule,
  /* 17 */DI_CHECKBOX,5,17,0,0,0,0,0,0,(char *)MConfigAutoComplete,
  /* 18 */DI_CHECKBOX,5,18,0,0,0,0,0,0,(char *)MConfigPgUpChangeDisk,
  /* 19 */DI_TEXT,3,19,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
  /* 20 */DI_BUTTON,0,20,0,0,0,0,DIF_CENTERGROUP,1,(char *)MOk,
  /* 21 */DI_BUTTON,0,20,0,0,0,0,DIF_CENTERGROUP,0,(char *)MCancel
  };
  MakeDialogItems(CfgDlgData,CfgDlg);

  if (!RegVer)
  {
    CfgDlg[DLG_INTERF_VIEWEREDITORCLOCK].Type=DI_TEXT;
    sprintf(CfgDlg[DLG_INTERF_VIEWEREDITORCLOCK].Data," *  %s",MSG(MRegOnlyShort));
  }

  CfgDlg[DLG_INTERF_CLOCK].Selected=Opt.Clock;
  CfgDlg[DLG_INTERF_VIEWEREDITORCLOCK].Selected=Opt.ViewerEditorClock;
  CfgDlg[DLG_INTERF_MOUSE].Selected=Opt.Mouse;
  CfgDlg[DLG_INTERF_MOUSEPMCLICKRULE].Selected=Opt.PanelMiddleClickRule;
  if(!Opt.Mouse)
    CfgDlg[DLG_INTERF_MOUSEPMCLICKRULE].Flags|=DIF_DISABLE;
  CfgDlg[DLG_INTERF_SHOWKEYBAR].Selected=Opt.ShowKeyBar;
  CfgDlg[DLG_INTERF_SHOWMENUBAR].Selected=Opt.ShowMenuBar;
  CfgDlg[DLG_INTERF_SCREENSAVER].Selected=Opt.ScreenSaver;
  sprintf(CfgDlg[DLG_INTERF_SCREENSAVERTIME].Data,"%d",Opt.ScreenSaverTime);
  if(!Opt.ScreenSaver)
  {
    CfgDlg[DLG_INTERF_SCREENSAVERTIME].Flags|=DIF_DISABLE;
    CfgDlg[DLG_INTERF_SAVERMINUTES].Flags|=DIF_DISABLE;
  }
  CfgDlg[DLG_INTERF_DIALOGSEDITHISTORY].Selected=Opt.DialogsEditHistory;
  CfgDlg[DLG_INTERF_DIALOGSEDITBLOCK].Selected=Opt.DialogsEditBlock;
  CfgDlg[DLG_INTERF_USEPROMPTFORMAT].Selected=Opt.UsePromptFormat;
  strcpy(CfgDlg[DLG_INTERF_PROMPTFORMAT].Data,Opt.PromptFormat);
  if(!Opt.UsePromptFormat)
    CfgDlg[DLG_INTERF_PROMPTFORMAT].Flags|=DIF_DISABLE;
  CfgDlg[DLG_INTERF_ALTGR].Selected=Opt.AltGr;
  CfgDlg[DLG_INTERF_COPYSHOWTOTAL].Selected=Opt.CopyShowTotal;

  CfgDlg[DLG_INTERF_COPYTIMERULE].Selected=Opt.CopyTimeRule!=0;

  CfgDlg[DLG_INTERF_AUTOCOMPLETE].Selected=Opt.AutoComplete;
  CfgDlg[DLG_INTERF_PGUPCHANGEDISK].Selected=Opt.PgUpChangeDisk;

  {
    Dialog Dlg(CfgDlg,sizeof(CfgDlg)/sizeof(CfgDlg[0]));
    Dlg.SetHelp("InterfSettings");
    Dlg.SetPosition(-1,-1,58,23);
    Dlg.SetAutomation(DLG_INTERF_SCREENSAVER,DLG_INTERF_SCREENSAVERTIME,DIF_DISABLE,0,0,DIF_DISABLE);
    Dlg.SetAutomation(DLG_INTERF_SCREENSAVER,DLG_INTERF_SAVERMINUTES,DIF_DISABLE,0,0,DIF_DISABLE);
    Dlg.SetAutomation(DLG_INTERF_USEPROMPTFORMAT,DLG_INTERF_PROMPTFORMAT,DIF_DISABLE,0,0,DIF_DISABLE);
    Dlg.SetAutomation(DLG_INTERF_MOUSE,DLG_INTERF_MOUSEPMCLICKRULE,DIF_DISABLE,0,0,DIF_DISABLE);
    Dlg.Process();
    if (Dlg.GetExitCode() != DLG_INTERF_OK)
      return;
  }

  Opt.Clock=CfgDlg[DLG_INTERF_CLOCK].Selected;
  Opt.ViewerEditorClock=CfgDlg[DLG_INTERF_VIEWEREDITORCLOCK].Selected;
  Opt.Mouse=CfgDlg[DLG_INTERF_MOUSE].Selected;
  Opt.PanelMiddleClickRule=CfgDlg[DLG_INTERF_MOUSEPMCLICKRULE].Selected;
  Opt.ShowKeyBar=CfgDlg[DLG_INTERF_SHOWKEYBAR].Selected;
  Opt.ShowMenuBar=CfgDlg[DLG_INTERF_SHOWMENUBAR].Selected;
  Opt.ScreenSaver=CfgDlg[DLG_INTERF_SCREENSAVER].Selected;
  Opt.DialogsEditHistory=CfgDlg[DLG_INTERF_DIALOGSEDITHISTORY].Selected;
  Opt.DialogsEditBlock=CfgDlg[DLG_INTERF_DIALOGSEDITBLOCK].Selected;
  if ((Opt.ScreenSaverTime=atoi(CfgDlg[DLG_INTERF_SCREENSAVERTIME].Data))<=0)
    Opt.ScreenSaver=Opt.ScreenSaverTime=0;
  Opt.UsePromptFormat=CfgDlg[DLG_INTERF_USEPROMPTFORMAT].Selected;
  strncpy(Opt.PromptFormat,CfgDlg[DLG_INTERF_PROMPTFORMAT].Data,sizeof(Opt.PromptFormat)-1);
  Opt.AltGr=CfgDlg[DLG_INTERF_ALTGR].Selected;
  Opt.CopyShowTotal=CfgDlg[DLG_INTERF_COPYSHOWTOTAL].Selected;
  Opt.AutoComplete=CfgDlg[DLG_INTERF_AUTOCOMPLETE].Selected;
  Opt.PgUpChangeDisk=CfgDlg[DLG_INTERF_PGUPCHANGEDISK].Selected;
  Opt.CopyTimeRule=0;
  if(CfgDlg[DLG_INTERF_COPYTIMERULE].Selected)
    Opt.CopyTimeRule=3;

  SetFarConsoleMode();
  CtrlObject->Cp()->LeftPanel->Update(UPDATE_KEEP_SELECTION);
  CtrlObject->Cp()->RightPanel->Update(UPDATE_KEEP_SELECTION);
  CtrlObject->Cp()->SetScreenPosition();
  /*$ 10.07.2001 SKV
    ! надо это делать, иначе если кейбар спрятали,
      будет полный рамс.
  */
  CtrlObject->Cp()->Redraw();
  /* SKV$*/
  /* $ 09.09.2001 IS обновим настройки ком.строки */
  CtrlObject->CmdLine->SetPersistentBlocks(Opt.DialogsEditBlock);
  /* IS $ */
}
/* IS 17.12.2001 $ */

/* $ 09.02.2001 IS
   Опция Esc
*/
/* $ 15.03.2001 SVS
    Подтверждение удаления мапленных дисков из меню дисков
*/
void SetConfirmations()
{
  static struct DialogData ConfDlgData[]={
  /* 00 */DI_DOUBLEBOX,3,1,46,14,0,0,0,0,(char *)MSetConfirmTitle,
  /* 01 */DI_CHECKBOX,5,2,0,0,1,0,0,0,(char *)MSetConfirmCopy,
  /* 02 */DI_CHECKBOX,5,3,0,0,0,0,0,0,(char *)MSetConfirmMove,
  /* 03 */DI_CHECKBOX,5,4,0,0,0,0,0,0,(char *)MSetConfirmDrag,
  /* 04 */DI_CHECKBOX,5,5,0,0,0,0,0,0,(char *)MSetConfirmDelete,
  /* 05 */DI_CHECKBOX,5,6,0,0,0,0,0,0,(char *)MSetConfirmDeleteFolders,
  /* 06 */DI_CHECKBOX,5,7,0,0,0,0,0,0,(char *)MSetConfirmEsc,
  /* 07 */DI_CHECKBOX,5,8,0,0,0,0,0,0,(char *)MSetConfirmRemoveConnection,
  /* 08 */DI_CHECKBOX,5,9,0,0,0,0,0,0,(char *)MSetConfirmAllowReedit,
  /* 08 */DI_CHECKBOX,5,10,0,0,0,0,0,0,(char *)MSetConfirmHistoryClear,
  /* 09 */DI_CHECKBOX,5,11,0,0,0,0,0,0,(char *)MSetConfirmExit,
  /* 11 */DI_TEXT,3,12,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
  /* 12 */DI_BUTTON,0,13,0,0,0,0,DIF_CENTERGROUP,1,(char *)MOk,
  /* 13 */DI_BUTTON,0,13,0,0,0,0,DIF_CENTERGROUP,0,(char *)MCancel

  };
  MakeDialogItems(ConfDlgData,ConfDlg);
  ConfDlg[1].Selected=Opt.Confirm.Copy;
  ConfDlg[2].Selected=Opt.Confirm.Move;
  ConfDlg[3].Selected=Opt.Confirm.Drag;
  ConfDlg[4].Selected=Opt.Confirm.Delete;
  ConfDlg[5].Selected=Opt.Confirm.DeleteFolder;
  ConfDlg[6].Selected=Opt.Confirm.Esc;
  ConfDlg[7].Selected=Opt.Confirm.RemoveConnection;
  ConfDlg[8].Selected=Opt.Confirm.AllowReedit;
  ConfDlg[9].Selected=Opt.Confirm.HistoryClear;
  ConfDlg[10].Selected=Opt.Confirm.Exit;

  Dialog Dlg(ConfDlg,sizeof(ConfDlg)/sizeof(ConfDlg[0]));
  Dlg.SetHelp("ConfirmDlg");
  Dlg.SetPosition(-1,-1,50,16);
  Dlg.Process();
  if (Dlg.GetExitCode()!=12)
    return;
  Opt.Confirm.Copy=ConfDlg[1].Selected;
  Opt.Confirm.Move=ConfDlg[2].Selected;
  Opt.Confirm.Drag=ConfDlg[3].Selected;
  Opt.Confirm.Delete=ConfDlg[4].Selected;
  Opt.Confirm.DeleteFolder=ConfDlg[5].Selected;
  Opt.Confirm.Esc=ConfDlg[6].Selected;
  Opt.Confirm.RemoveConnection=ConfDlg[7].Selected;
  Opt.Confirm.AllowReedit=ConfDlg[8].Selected;
  Opt.Confirm.HistoryClear=ConfDlg[9].Selected;
  Opt.Confirm.Exit=ConfDlg[10].Selected;
}
/* SVS $ */
/* IS $ */

void SetDizConfig()
{
  static struct DialogData DizDlgData[]=
  {
  /* 00 */DI_DOUBLEBOX,3,1,72,14,0,0,0,0,(char *)MCfgDizTitle,
  /* 01 */DI_TEXT,5,2,0,0,0,0,0,0,(char *)MCfgDizListNames,
  /* 02 */DI_EDIT,5,3,70,3,1,0,0,0,"",
  /* 03 */DI_TEXT,3,4,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
  /* 04 */DI_CHECKBOX,5,5,0,0,0,0,0,0,(char *)MCfgDizSetHidden,
  /* 05 */DI_CHECKBOX,5,6,0,0,0,0,0,0,(char *)MCfgDizROUpdate,
  /* 06 */DI_FIXEDIT,5,7,7,7,0,0,0,0,"",
  /* 07 */DI_TEXT,9,7,0,0,0,0,0,0,(char *)MCfgDizStartPos,
  /* 08 */DI_TEXT,3,8,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
  /* 09 */DI_RADIOBUTTON,5,9,0,0,0,0,DIF_GROUP,0,(char *)MCfgDizNotUpdate,
  /* 10 */DI_RADIOBUTTON,5,10,0,0,0,0,0,0,(char *)MCfgDizUpdateIfDisplayed,
  /* 11 */DI_RADIOBUTTON,5,11,0,0,0,0,0,0,(char *)MCfgDizAlwaysUpdate,
  /* 12 */DI_TEXT,3,12,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
  /* 13 */DI_BUTTON,0,13,0,0,0,0,DIF_CENTERGROUP,1,(char *)MOk,
  /* 14 */DI_BUTTON,0,13,0,0,0,0,DIF_CENTERGROUP,0,(char *)MCancel
  };
  MakeDialogItems(DizDlgData,DizDlg);
  Dialog Dlg(DizDlg,sizeof(DizDlg)/sizeof(DizDlg[0]));
  Dlg.SetPosition(-1,-1,76,16);
  Dlg.SetHelp("FileDiz");

  // ограничим размер поля ввода.
  Dialog::SendDlgMessage((HANDLE)&Dlg,DM_SETMAXTEXTLENGTH,2,sizeof(Opt.Diz.ListNames)-1);
  strcpy(DizDlg[2].Data,Opt.Diz.ListNames);
  if (Opt.Diz.UpdateMode==DIZ_NOT_UPDATE)
    DizDlg[9].Selected=TRUE;
  else
    if (Opt.Diz.UpdateMode==DIZ_UPDATE_IF_DISPLAYED)
      DizDlg[10].Selected=TRUE;
    else
      DizDlg[11].Selected=TRUE;
  DizDlg[4].Selected=Opt.Diz.SetHidden;
  DizDlg[5].Selected=Opt.Diz.ROUpdate;
  sprintf(DizDlg[6].Data,"%d",Opt.Diz.StartPos);

  Dlg.Process();
  if (Dlg.GetExitCode()!=13)
    return;
  strncpy(Opt.Diz.ListNames,DizDlg[2].Data,sizeof(Opt.Diz.ListNames)-1);
  if (DizDlg[9].Selected)
    Opt.Diz.UpdateMode=DIZ_NOT_UPDATE;
  else
    if (DizDlg[10].Selected)
      Opt.Diz.UpdateMode=DIZ_UPDATE_IF_DISPLAYED;
    else
      Opt.Diz.UpdateMode=DIZ_UPDATE_ALWAYS;
  Opt.Diz.SetHidden=DizDlg[4].Selected;
  Opt.Diz.ROUpdate=DizDlg[5].Selected;
  Opt.Diz.StartPos=atoi(DizDlg[6].Data);
}

/* $ 18.07.2000 tran
   настройка двух новых параметров для вьювера */
/*
 +----------------- Viewer ------------------+
 | + External viewer ----------------------+ |
 | | ( ) Use for F3                        | |
 | | () Use for Alt-F3                    | |
 | | Viewer command:                       | |
 | |                                       | |
 | +---------------------------------------+ |
 | + Internal viewer ----------------------+ |
 | | [x] Save file position                | |
 | | [x] Save shortcut position            | |
 | | [x] Autodetect character table        | |
 | | 8   Tab size                          | |
 | | [x] Show scrollbar                    | |
 | | [x] Show arrows                       | |
 | +---------------------------------------+ |
 |            [ Ok ]  [ Cancel ]             |
 +-------------------------------------------+
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

/* $ 29.03.2001 IS
  + По аналогии с редактором часть настроек переехала в ViewerOptions
*/
/* $ 27.11.2001 DJ
   параметр Local
*/
void ViewerConfig(struct ViewerOptions &ViOpt,int Local)
{
  static struct DialogData CfgDlgData[]={
  /*  0 */  DI_DOUBLEBOX , 3, 1,47,17,0,0,0,0,(char *)MViewConfigTitle,                  //   0
  /*  1 */  DI_SINGLEBOX , 5, 2,45, 7,0,0,DIF_LEFTTEXT,0,(char *)MViewConfigExternal,    //   1
  /*  2 */  DI_RADIOBUTTON,7, 3, 0, 0,1,0,DIF_GROUP,0,(char *)MViewConfigExternalF3,     //   2
  /*  3 */  DI_RADIOBUTTON,7, 4, 0, 0,0,0,0,0,(char *)MViewConfigExternalAltF3,          //   3
  /*  4 */  DI_TEXT      , 7, 5, 0, 0,0,0,0,0,(char *)MViewConfigExternalCommand,        //   4
  /*  5 */  DI_EDIT      , 7, 6,43, 6,0,0,0,0,"",                                        //   5
  /*  6 */  DI_SINGLEBOX , 5, 8,45,15,0,0,DIF_LEFTTEXT,0,(char *)MViewConfigInternal,    //   6
  /*  7 */  DI_CHECKBOX  , 7, 9, 0, 0,0,0,0,0,(char *)MViewConfigSavePos,                //   7
  /*  8 */  DI_CHECKBOX  , 7,10, 0, 0,0,0,0,0,(char *)MViewConfigSaveShortPos,           //   8
  /*  9 */  DI_CHECKBOX  , 7,11, 0, 0,0,0,0,0,(char *)MViewAutoDetectTable,              //   9
  /* 10 */  DI_FIXEDIT   , 7,12, 9,15,0,0,0,0,"",                                        //  10
  /* 11 */  DI_TEXT      ,11,12, 0, 0,0,0,0,0,(char *)MViewConfigTabSize,                //  11
  /* 12 */  DI_CHECKBOX  , 7,13, 0, 0,0,0,0,0,(char *)MViewConfigScrollbar,              //  12 *new
  /* 13 */  DI_CHECKBOX  , 7,14, 0, 0,0,0,0,0,(char *)MViewConfigArrows,                 //  13 *new
  /* 14 */  DI_BUTTON    , 0,16, 0, 0,0,0,DIF_CENTERGROUP,1,(char *)MOk,                 //  14 , was 11
  /* 15 */  DI_BUTTON    , 0,16, 0, 0,0,0,DIF_CENTERGROUP,0,(char *)MCancel              //  15 , was 12
  };
  MakeDialogItems(CfgDlgData,CfgDlg);

  CfgDlg[DLG_VIEW_USE_F3].Selected=Opt.UseExternalViewer;
  CfgDlg[DLG_VIEW_USE_ALTF3].Selected=!Opt.UseExternalViewer;
  strcpy(CfgDlg[DLG_VIEW_EXTERNAL].Data,Opt.ExternalViewer);
  CfgDlg[DLG_VIEW_SAVEFILEPOS].Selected=Opt.SaveViewerPos;
  CfgDlg[DLG_VIEW_SAVESHORTPOS].Selected=Opt.SaveViewerShortPos;
  /* 15.09.2000 IS
     Отключение автоопределения таблицы символов, если отсутствует таблица с
     распределением частот символов
  */
  CfgDlg[DLG_VIEW_AUTODETECT].Selected=ViOpt.AutoDetectTable&&DistrTableExist();
  /* IS $ */
  CfgDlg[DLG_VIEW_SCROLLBAR].Selected=ViOpt.ShowScrollbar;
  CfgDlg[DLG_VIEW_ARROWS].Selected=ViOpt.ShowArrows;
  sprintf(CfgDlg[DLG_VIEW_TABSIZE].Data,"%d",ViOpt.TabSize);

  if (!RegVer)
  {
    CfgDlg[DLG_VIEW_TABSIZE].Type=CfgDlg[10].Type=DI_TEXT;
    sprintf(CfgDlg[DLG_VIEW_TABSIZE].Data," *  %s",MSG(MRegOnlyShort));
    *CfgDlg[10].Data=0;
  }

  int DialogHeight=19;
  if (Local)
  {
    int i;
    for (i=1; i<=5; i++)
      CfgDlg[i].Flags |= DIF_HIDDEN;
    for (i=6; i<=15; i++)
      CfgDlg[i].Y1 -= 6;
    CfgDlg[0].Y2 -= 6;
    CfgDlg[6].Y2 -= 6;
    DialogHeight -= 6;
  }

  {
    Dialog Dlg(CfgDlg,sizeof(CfgDlg)/sizeof(CfgDlg[0]));
    Dlg.SetHelp("ViewerSettings");
    Dlg.SetPosition(-1,-1,51,DialogHeight);
    Dlg.Process();
    if (Dlg.GetExitCode()!=DLG_VIEW_OK)
      return;
  }

  if (!Local)
  {
    Opt.UseExternalViewer=CfgDlg[DLG_VIEW_USE_F3].Selected;
    strncpy(Opt.ExternalViewer,CfgDlg[DLG_VIEW_EXTERNAL].Data,sizeof(Opt.ExternalViewer)-1);
  }
  Opt.SaveViewerPos=CfgDlg[DLG_VIEW_SAVEFILEPOS].Selected;
  Opt.SaveViewerShortPos=CfgDlg[DLG_VIEW_SAVESHORTPOS].Selected;
  /* $ 16.12.2000 IS
    - баг: забыли считать опцию DLG_VIEW_AUTODETECT из диалога
  */
  ViOpt.AutoDetectTable=CfgDlg[DLG_VIEW_AUTODETECT].Selected;
  /* IS $ */
  /* 15.09.2000 IS
     Отключение автоопределения таблицы символов, если отсутствует таблица с
     распределением частот символов
  */
  if(!DistrTableExist() && ViOpt.AutoDetectTable)
  {
    ViOpt.AutoDetectTable=0;
    Message(MSG_WARNING,1,MSG(MWarning),
              MSG(MDistributionTableWasNotFound),MSG(MAutoDetectWillNotWork),
              MSG(MOk));
  }
  /* IS $ */
  ViOpt.TabSize=atoi(CfgDlg[DLG_VIEW_TABSIZE].Data);
  ViOpt.ShowScrollbar=CfgDlg[DLG_VIEW_SCROLLBAR].Selected;
  ViOpt.ShowArrows=CfgDlg[DLG_VIEW_ARROWS].Selected;
  if (ViOpt.TabSize<1 || ViOpt.TabSize>512)
    ViOpt.TabSize=8;
}
/* DJ $ */
/* IS $ */
/* tran 18.07.2000 $ */

/* $ 04.08.2000 SVS
   ! WordDiv выкинул - будет описан в TechInfo.txt
*/
/* $ 03.08.2000 SVS
  + WordDiv внесен в Options|Editor settings
*/
/* $ 21.02.2001 IS
  + Работа идет со структурой EditorOptions
*/
/* $ 20.05.2001 IS
  + Задаем поведение для файлов с атрибутом r/o:
    лочить или нет
    орать или нет
*/
/* $ 27.11.2001 DJ
   параметр Local и соответствующие модификации в коде
*/
/* $ 03.01.2001 IS
  ! Устранение "двойного отрицания".
*/
void EditorConfig(struct EditorOptions &EdOpt,int Local)
{
  static struct DialogData CfgDlgData[]={
  /*  0 */  DI_DOUBLEBOX,3,1,63,22,0,0,0,0,(char *)MEditConfigTitle,
  /*  1 */  DI_SINGLEBOX,5,2,61,7,0,0,DIF_LEFTTEXT,0,(char *)MEditConfigExternal,
  /*  2 */  DI_RADIOBUTTON,7,3,0,0,1,0,DIF_GROUP,0,(char *)MEditConfigEditorF4,
  /*  3 */  DI_RADIOBUTTON,7,4,0,0,0,0,0,0,(char *)MEditConfigEditorAltF4,
  /*  4 */  DI_TEXT,7,5,0,0,0,0,0,0,(char *)MEditConfigEditorCommand,
  /*  5 */  DI_EDIT,7,6,59,6,0,0,0,0,"",
  /*  6 */  DI_SINGLEBOX,5,8,61,20,0,0,DIF_LEFTTEXT,0,(char *)MEditConfigInternal,
  /*  7 */  DI_CHECKBOX,7,9,0,0,0,0,0,0,(char *)MEditConfigTabsToSpaces,
  /*  8 */  DI_CHECKBOX,7,10,0,0,0,0,0,0,(char *)MEditConfigPersistentBlocks,
  /*  9 */  DI_CHECKBOX,7,11,0,0,0,0,0,0,(char *)MEditConfigDelRemovesBlocks,
  /* 10 */  DI_CHECKBOX,7,12,0,0,0,0,0,0,(char *)MEditConfigAutoIndent,
  /* 11 */  DI_CHECKBOX,7,13,0,0,0,0,0,0,(char *)MEditConfigSavePos,
  /* 12 */  DI_CHECKBOX,7,14,0,0,0,0,0,0,(char *)MEditConfigSaveShortPos,
  /* 13 */  DI_CHECKBOX,7,15,0,0,0,0,0,0,(char *)MEditAutoDetectTable,
  /* 14 */  DI_CHECKBOX,7,16,0,0,0,0,0,0,(char *)MEditCursorBeyondEnd,
  /* 15 */  DI_CHECKBOX,7,17,0,0,0,0,0,0,(char *)MEditLockROFileModification,
  /* 16 */  DI_CHECKBOX,7,18,0,0,0,0,0,0,(char *)MEditWarningBeforeOpenROFile,
  /* 17 */  DI_FIXEDIT,7,19,9,19,0,0,0,0,"",
  /* 18 */  DI_TEXT,11,19,0,0,0,0,0,0,(char *)MEditConfigTabSize,
  /* 19 */  DI_BUTTON,0,21,0,0,0,0,DIF_CENTERGROUP,1,(char *)MOk,
  /* 20 */  DI_BUTTON,0,21,0,0,0,0,DIF_CENTERGROUP,0,(char *)MCancel
  };
  MakeDialogItems(CfgDlgData,CfgDlg);

  CfgDlg[2].Selected=Opt.UseExternalEditor;
  CfgDlg[3].Selected=!Opt.UseExternalEditor;
  strcpy(CfgDlg[5].Data,Opt.ExternalEditor);
  CfgDlg[7].Selected=EdOpt.ExpandTabs;
  CfgDlg[8].Selected=EdOpt.PersistentBlocks;
  CfgDlg[9].Selected=EdOpt.DelRemovesBlocks;
  CfgDlg[10].Selected=EdOpt.AutoIndent;
  CfgDlg[11].Selected=EdOpt.SavePos;
  CfgDlg[12].Selected=EdOpt.SaveShortPos;
  /* 15.09.2000 IS
     Отключение автоопределения таблицы символов, если отсутствует таблица с
     распределением частот символов
  */
  CfgDlg[13].Selected=EdOpt.AutoDetectTable&&DistrTableExist();
  /* IS $ */
  sprintf(CfgDlg[17].Data,"%d",EdOpt.TabSize);
  CfgDlg[14].Selected=EdOpt.CursorBeyondEOL;
  CfgDlg[15].Selected=!(Opt.EditorReadOnlyLock & 1);
  CfgDlg[16].Selected=Opt.EditorReadOnlyLock & 2;

  if (!RegVer)
  {
    CfgDlg[17].Type=CfgDlg[18].Type=DI_TEXT;
    sprintf(CfgDlg[17].Data," *  %s",MSG(MRegOnlyShort));
    *CfgDlg[18].Data=0;
  }

  int DialogHeight=24;
  if (Local)
  {
    int i;
    for (i=1; i<=5; i++)
      CfgDlg[i].Flags |= DIF_HIDDEN;
    for (i=6; i<=20; i++)
      CfgDlg[i].Y1 -= 6;
    CfgDlg[0].Y2 -= 6;
    CfgDlg[6].Y2 -= 6;
    DialogHeight -= 6;
  }

  {
    Dialog Dlg(CfgDlg,sizeof(CfgDlg)/sizeof(CfgDlg[0]));
    Dlg.SetHelp("EditorSettings");
    Dlg.SetPosition(-1,-1,67,DialogHeight);
    Dlg.Process();
    if (Dlg.GetExitCode()!=19)
      return;
  }

  if (!Local)
  {
    Opt.UseExternalEditor=CfgDlg[2].Selected;
    strncpy(Opt.ExternalEditor,CfgDlg[5].Data,sizeof(Opt.ExternalEditor)-1);
  }
  EdOpt.ExpandTabs=CfgDlg[7].Selected;
  EdOpt.PersistentBlocks=CfgDlg[8].Selected;
  EdOpt.DelRemovesBlocks=CfgDlg[9].Selected;
  EdOpt.AutoIndent=CfgDlg[10].Selected;
  EdOpt.SavePos=CfgDlg[11].Selected;
  EdOpt.SaveShortPos=CfgDlg[12].Selected;
  EdOpt.AutoDetectTable=CfgDlg[13].Selected;
  /* 15.09.2000 IS
     Отключение автоопределения таблицы символов, если отсутствует таблица с
     распределением частот символов
  */
  if(!DistrTableExist() && EdOpt.AutoDetectTable)
  {
    EdOpt.AutoDetectTable=0;
    Message(MSG_WARNING,1,MSG(MWarning),
              MSG(MDistributionTableWasNotFound),MSG(MAutoDetectWillNotWork),
              MSG(MOk));
  }
  /* IS $ */
  EdOpt.TabSize=atoi(CfgDlg[17].Data);
  if (EdOpt.TabSize<1 || EdOpt.TabSize>512)
    EdOpt.TabSize=8;
  EdOpt.CursorBeyondEOL=CfgDlg[14].Selected;
  Opt.EditorReadOnlyLock&=~3;
  if(!CfgDlg[15].Selected) Opt.EditorReadOnlyLock|=1;
  if(CfgDlg[16].Selected) Opt.EditorReadOnlyLock|=2;
}
/* IS 03.01.2002 $ */
/* DJ $ */
/* IS $ */
/* IS $ */
/* SVS $ */


void SetFolderInfoFiles()
{
  /* 30.04.2001 DJ
     добавлена history; обновляем инфо-панель после изменения
  */
  if (GetString(MSG(MSetFolderInfoTitle),MSG(MSetFolderInfoNames),"FolderInfoFiles",
      Opt.FolderInfoFiles,Opt.FolderInfoFiles,sizeof(Opt.FolderInfoFiles),"OptMenu",FIB_ENABLEEMPTY))
  {
    if (CtrlObject->Cp()->LeftPanel->GetType() == INFO_PANEL)
      CtrlObject->Cp()->LeftPanel->Update(0);
    if (CtrlObject->Cp()->RightPanel->GetType() == INFO_PANEL)
      CtrlObject->Cp()->RightPanel->Update(0);
  }
  /* DJ $ */
}


// Структура, описывающая всю конфигурацию(!)
static struct FARConfig{
  int   IsSave;   // =1 - будет записываться в SaveConfig()
  DWORD ValType;  // REG_DWORD, REG_SZ, REG_BINARY
  char *KeyName;  // Имя ключа
  char *ValName;  // Имя параметра
  void *ValPtr;   // адрес переменной, куда помещаем данные
  DWORD DefDWord; // он же размер данных для REG_SZ и REG_BINARY
  char *DefStr;   // строка/данные по умолчанию
} CFG[]={
  {1, REG_BINARY, NKeyColors,"CurrentPalette",(char*)Palette,SizeArrayPalette,(char*)DefaultPalette},

  {1, REG_DWORD,  NKeyScreen, "Clock", &Opt.Clock, 1, 0},
  {1, REG_DWORD,  NKeyScreen, "ViewerEditorClock",&Opt.ViewerEditorClock,0, 0},
  {1, REG_DWORD,  NKeyScreen, "KeyBar",&Opt.ShowKeyBar,1, 0},
  {1, REG_DWORD,  NKeyScreen, "ScreenSaver",&Opt.ScreenSaver, 0, 0},
  {1, REG_DWORD,  NKeyScreen, "ScreenSaverTime",&Opt.ScreenSaverTime,5, 0},
  {1, REG_DWORD,  NKeyScreen, "UsePromptFormat", &Opt.UsePromptFormat,0, 0},
  {1, REG_SZ,     NKeyScreen, "PromptFormat",Opt.PromptFormat,sizeof(Opt.PromptFormat),"$p>"},

  {1, REG_DWORD,  NKeyInterface, "DialogsEditHistory",&Opt.DialogsEditHistory,1, 0},
  {1, REG_DWORD,  NKeyInterface, "DialogsEditBlock",&Opt.DialogsEditBlock,0, 0},
  {1, REG_DWORD,  NKeyInterface, "Mouse",&Opt.Mouse,1, 0},
  {1, REG_DWORD,  NKeyInterface, "AltGr",&Opt.AltGr,1, 0},
  {1, REG_DWORD,  NKeyInterface, "CopyShowTotal",&Opt.CopyShowTotal,0, 0},
  {1, REG_DWORD,  NKeyInterface, "ShowMenuBar",&Opt.ShowMenuBar,0, 0},
  {1, REG_DWORD,  NKeyInterface, "AutoComplete",&Opt.AutoComplete,1, 0},
  {0, REG_DWORD,  NKeyInterface, "CursorSize1",&Opt.CursorSize[0],15, 0},
  {0, REG_DWORD,  NKeyInterface, "CursorSize2",&Opt.CursorSize[1],10, 0},
  {0, REG_DWORD,  NKeyInterface, "CursorSize3",&Opt.CursorSize[2],99, 0},
  {0, REG_DWORD,  NKeyInterface, "CursorSize4",&Opt.CursorSize[3],99, 0},
  {0, REG_DWORD,  NKeyInterface, "ShiftsKeyRules",&Opt.ShiftsKeyRules,1, 0},
  {0, REG_DWORD,  NKeyInterface, "AltF9",&Opt.AltF9, -1, 0},
  {1, REG_DWORD,  NKeyInterface, "CtrlPgUp",&Opt.PgUpChangeDisk, 1, 0},
  /* $ 24.10.2001 KM
     Запомнить флаг разрешения поиска каталогов в Alt-F7
  */
  {1, REG_DWORD,  NKeyInterface, "FindFolders",&Opt.FindFolders, 1, 0},
  /* KM $ */

  {1, REG_SZ,     NKeyViewer,"ExternalViewerName",Opt.ExternalViewer,sizeof(Opt.ExternalViewer),""},
  {1, REG_DWORD,  NKeyViewer,"UseExternalViewer",&Opt.UseExternalViewer,0, 0},
  {1, REG_DWORD,  NKeyViewer,"SaveViewerPos",&Opt.SaveViewerPos,1, 0},
  {1, REG_DWORD,  NKeyViewer,"SaveViewerShortPos",&Opt.SaveViewerShortPos,1, 0},
  {1, REG_DWORD,  NKeyViewer,"AutoDetectTable",&Opt.ViOpt.AutoDetectTable,0, 0},
  {1, REG_DWORD,  NKeyViewer,"TabSize",&Opt.ViOpt.TabSize,8, 0},
  {1, REG_DWORD,  NKeyViewer,"ShowKeyBar",&Opt.ShowKeyBarViewer,1, 0},
  {1, REG_DWORD,  NKeyViewer,"ShowArrows",&Opt.ViOpt.ShowArrows,1, 0},
  {1, REG_DWORD,  NKeyViewer,"ShowScrollbar",&Opt.ViOpt.ShowScrollbar,0, 0},
  {1, REG_DWORD,  NKeyViewer,"IsWrap",&Opt.ViewerIsWrap,1, 0},
  {1, REG_DWORD,  NKeyViewer,"Wrap",&Opt.ViewerWrap,0, 0},

  {0, REG_DWORD,  NKeyDialog,"EULBsClear",&Opt.DlgEULBsClear,0, 0},
  {0, REG_DWORD,  NKeyDialog,"SelectFromHistory",&Opt.DlgSelectFromHistory,0, 0},

  {1, REG_SZ,     NKeyEditor,"ExternalEditorName",Opt.ExternalEditor,sizeof(Opt.ExternalEditor),""},
  {1, REG_DWORD,  NKeyEditor,"UseExternalEditor",&Opt.UseExternalEditor,0, 0},
  {1, REG_DWORD,  NKeyEditor,"ExpandTabs",&Opt.EdOpt.ExpandTabs,0, 0},
  {1, REG_DWORD,  NKeyEditor,"TabSize",&Opt.EdOpt.TabSize,8, 0},
  {1, REG_DWORD,  NKeyEditor,"PersistentBlocks",&Opt.EdOpt.PersistentBlocks,1, 0},
  {1, REG_DWORD,  NKeyEditor,"DelRemovesBlocks",&Opt.EdOpt.DelRemovesBlocks,0, 0},
  {1, REG_DWORD,  NKeyEditor,"AutoIndent",&Opt.EdOpt.AutoIndent,0, 0},
  {1, REG_DWORD,  NKeyEditor,"SaveEditorPos",&Opt.EdOpt.SavePos,1, 0},
  {1, REG_DWORD,  NKeyEditor,"SaveEditorShortPos",&Opt.EdOpt.SaveShortPos,1, 0},
  {1, REG_DWORD,  NKeyEditor,"AutoDetectTable",&Opt.EdOpt.AutoDetectTable,0, 0},
  {1, REG_DWORD,  NKeyEditor,"EditorCursorBeyondEOL",&Opt.EdOpt.CursorBeyondEOL,1, 0},
  {1, REG_DWORD,  NKeyEditor,"ReadOnlyLock",&Opt.EditorReadOnlyLock,0, 0}, // Вернём назад дефолт 1.65 - не предупреждать и не блокировать
  /* $ 03.12.2001 IS размер буфера undo в редакторе */
  {0, REG_DWORD,  NKeyEditor,"EditorUndoSize",&Opt.EditorUndoSize,2048,0},
  /* IS $ */
  {0, REG_SZ,     NKeyEditor,"WordDiv",Opt.WordDiv,sizeof(Opt.WordDiv),WordDiv0},
  {0, REG_DWORD,  NKeyEditor,"BSLikeDel",&Opt.EdOpt.BSLikeDel,1, 0},
  {0, REG_DWORD,  NKeyEditor,"EditorF7Rules",&Opt.EditorF7Rules,1, 0},
  {0, REG_DWORD,  NKeyEditor,"FileSizeLimit",&Opt.EditorFileSizeLimitLo,(DWORD)0, 0},
  {0, REG_DWORD,  NKeyEditor,"FileSizeLimitHi",&Opt.EditorFileSizeLimitHi,(DWORD)0, 0},
  {0, REG_DWORD,  NKeyEditor,"CharCodeBase",&Opt.EdOpt.CharCodeBase,1, 0},

  {0, REG_DWORD,  NKeyXLat,"Flags",&Opt.XLat.Flags,(DWORD)XLAT_SWITCHKEYBLAYOUT, 0},
  {0, REG_BINARY, NKeyXLat,"Table1",Opt.XLat.Table[0],sizeof(Opt.XLat.Table[0]),NULL},
  {0, REG_BINARY, NKeyXLat,"Table2",Opt.XLat.Table[1],sizeof(Opt.XLat.Table[1]),NULL},
  {0, REG_BINARY, NKeyXLat,"Rules1",Opt.XLat.Rules[0],sizeof(Opt.XLat.Rules[0]),NULL},
  {0, REG_BINARY, NKeyXLat,"Rules2",Opt.XLat.Rules[1],sizeof(Opt.XLat.Rules[1]),NULL},
  {0, REG_BINARY, NKeyXLat,"Rules3",Opt.XLat.Rules[2],sizeof(Opt.XLat.Rules[2]),NULL},
  {0, REG_SZ,     NKeyXLat,"WordDivForXlat",Opt.XLat.WordDivForXlat,sizeof(Opt.XLat.WordDivForXlat),WordDivForXlat0},

  {1, REG_DWORD,  NKeySystem,"SaveHistory",&Opt.SaveHistory,1, 0},
  {1, REG_DWORD,  NKeySystem,"SaveFoldersHistory",&Opt.SaveFoldersHistory,1, 0},
  {1, REG_DWORD,  NKeySystem,"SaveViewHistory",&Opt.SaveViewHistory,1, 0},
  {1, REG_DWORD,  NKeySystem,"UseRegisteredTypes",&Opt.UseRegisteredTypes,1, 0},
  {1, REG_DWORD,  NKeySystem,"AutoSaveSetup",&Opt.AutoSaveSetup,0, 0},
  {1, REG_DWORD,  NKeySystem,"ClearReadOnly",&Opt.ClearReadOnly,0, 0},
  {1, REG_DWORD,  NKeySystem,"DeleteToRecycleBin",&Opt.DeleteToRecycleBin,1, 0},
  {0, REG_DWORD,  NKeySystem,"WipeSymbol",&Opt.WipeSymbol,0, 0},
  {1, REG_DWORD,  NKeySystem,"UseSystemCopy",&Opt.UseSystemCopy,0, 0},
  {1, REG_DWORD,  NKeySystem,"CopyOpened",&Opt.CopyOpened,1, 0},
  {1, REG_DWORD,  NKeySystem,"CreateUppercaseFolders",&Opt.CreateUppercaseFolders,0, 0},
  {1, REG_DWORD,  NKeySystem,"InactivityExit",&Opt.InactivityExit,0, 0},
  {1, REG_DWORD,  NKeySystem,"InactivityExitTime",&Opt.InactivityExitTime,15, 0},
  {1, REG_DWORD,  NKeySystem,"DriveMenuMode",&Opt.ChangeDriveMode,DRIVE_SHOW_TYPE|DRIVE_SHOW_PLUGINS, 0},
  {1, REG_DWORD,  NKeySystem,"DriveDisconnetMode",&Opt.ChangeDriveDisconnetMode,1, 0},
  {1, REG_DWORD,  NKeySystem,"FileSearchMode",&Opt.FileSearchMode,SEARCH_FROM_CURRENT, 0},
  {1, REG_SZ,     NKeySystem,"FolderInfo",Opt.FolderInfoFiles,sizeof(Opt.FolderInfoFiles),"DirInfo,File_Id.diz,Descript.ion,ReadMe,Read.Me,ReadMe.txt,ReadMe.*"},
  {0, REG_DWORD,  NKeySystem,"SubstPluginPrefix",&Opt.SubstPluginPrefix, 0, 0},
  {0, REG_DWORD,  NKeySystem,"CmdHistoryRule",&Opt.CmdHistoryRule,0, 0},
  {0, REG_DWORD,  NKeySystem,"SetAttrFolderRules",&Opt.SetAttrFolderRules,1, 0},
  {0, REG_DWORD,  NKeySystem,"MaxPositionCache",&Opt.MaxPositionCache,64, 0},
  {0, REG_DWORD,  NKeySystem,"AllCtrlAltShiftRule",&Opt.AllCtrlAltShiftRule,0x0000FFFF, 0},
  {1, REG_DWORD,  NKeySystem,"CopyTimeRule",  &Opt.CopyTimeRule, 3, 0},
  {0, REG_SZ,     NKeySystem,"ConsoleDetachKey", KeyNameConsoleDetachKey, sizeof(KeyNameConsoleDetachKey),"CtrlAltTab"},
  {1, REG_SZ,     NKeySystem,"PersonalPluginsPath",Opt.PersonalPluginsPath,sizeof(Opt.PersonalPluginsPath),PersonalPluginsPath},
  /* $ 07.12.2001 IS
     ! опция "разрешить мультикопирование/перемещение/создание связей"
     + опция "создание нескольких каталогов за один раз"
  */
  {1, REG_DWORD,  NKeySystem, "MultiCopy",&Opt.MultiCopy,0, 0},
  {1, REG_DWORD,  NKeySystem, "MultiMakeDir",&Opt.MultiMakeDir,0, 0},
  /* IS $ */
  /* $ 02.04.2001 VVM
    + Будет влиять на:
        добавление файлов в историю с разным регистром
        добавление LastPositions в редакторе и вьюере */
  {0, REG_DWORD,  NKeySystem,"FlagPosixSemantics", &Opt.FlagPosixSemantics, 1, 0},
  /* VVM $ */
  /* $ 16.04.2001 VVM
    + Opt.MsWheelDelta - задает смещение для прокрутки в панелях. */
  {0, REG_DWORD,  NKeySystem,"MsWheelDelta", &Opt.MsWheelDelta, 1, 0},
  /* VVM $ */
  /* VVM $ */
  /* $ 26.04.2001 VVM
    + Opt.MsWheelDeltaView - задает смещение для прокрутки во вьюере.
    + Opt.MsWheelDeltaEdit - задает смещение для прокрутки в редакторе. */
  {0, REG_DWORD,  NKeySystem,"MsWheelDeltaView", &Opt.MsWheelDeltaView, 1, 0},
  {0, REG_DWORD,  NKeySystem,"MsWheelDeltaEdit", &Opt.MsWheelDeltaEdit, 1, 0},
  /* VVM $ */
  /* $ 28.04.2001 VVM
    + Opt.SubstNameRule битовая маска:
      0 - если установлен, то опрашивать сменные диски при GetSubstName()
      1 - если установлен, то опрашивать все остальные при GetSubstName() */
  {0, REG_DWORD,  NKeySystem,"SubstNameRule", &Opt.SubstNameRule, 2, 0},
  /* VVM $ */
  {0, REG_DWORD,  NKeySystem,"ShowCheckingFile", &Opt.ShowCheckingFile, 0, 0},

  {0, REG_SZ,     NKeySystem,"QuotedSymbols",Opt.QuotedSymbols,sizeof(Opt.QuotedSymbols)," &+,"},
  {0, REG_DWORD,  NKeySystem,"LCID",&Opt.LCIDSort,LOCALE_USER_DEFAULT, 0},
  //{0, REG_DWORD,  NKeySystem,"CPAJHefuayor",&Opt.CPAJHefuayor,0, 0},
  {0, REG_DWORD,  NKeySystem,"CloseConsoleRule",&Opt.CloseConsoleRule,1, 0},

  {0, REG_DWORD,  NKeySystemExecutor,"RestoreCP",&Opt.RestoreCPAfterExecute,1, 0},

  {0, REG_DWORD,  NKeyHelp,"ActivateURL",&Opt.HelpURLRules,1, 0},

  {1, REG_SZ,     NKeyLanguage,"Main",Opt.Language,sizeof(Opt.Language),"English"},
  {1, REG_SZ,     NKeyLanguage,"Help",Opt.HelpLanguage,sizeof(Opt.HelpLanguage),"English"},

  {1, REG_DWORD,  NKeyConfirmations,"Copy",&Opt.Confirm.Copy,1, 0},
  {1, REG_DWORD,  NKeyConfirmations,"Move",&Opt.Confirm.Move,1, 0},
  {1, REG_DWORD,  NKeyConfirmations,"Drag",&Opt.Confirm.Drag,1, 0},
  {1, REG_DWORD,  NKeyConfirmations,"Delete",&Opt.Confirm.Delete,1, 0},
  {1, REG_DWORD,  NKeyConfirmations,"DeleteFolder",&Opt.Confirm.DeleteFolder,1, 0},
  {1, REG_DWORD,  NKeyConfirmations,"Esc",&Opt.Confirm.Esc,0, 0},
  {1, REG_DWORD,  NKeyConfirmations,"RemoveConnection",&Opt.Confirm.RemoveConnection,1, 0},
  {1, REG_DWORD,  NKeyConfirmations,"AllowReedit",&Opt.Confirm.AllowReedit,1, 0},
  {1, REG_DWORD,  NKeyConfirmations,"HistoryClear",&Opt.Confirm.HistoryClear,1, 0},
  {1, REG_DWORD,  NKeyConfirmations,"Exit",&Opt.Confirm.Exit,1, 0},

  {1, REG_DWORD,  NKeyPanel,"ShowHidden",&Opt.ShowHidden,1, 0},
  {1, REG_DWORD,  NKeyPanel,"Highlight",&Opt.Highlight,1, 0},
  {1, REG_DWORD,  NKeyPanel,"AutoChangeFolder",&Opt.AutoChangeFolder,0, 0},
  {1, REG_DWORD,  NKeyPanel,"SelectFolders",&Opt.SelectFolders,0, 0},
  {1, REG_DWORD,  NKeyPanel,"ReverseSort",&Opt.ReverseSort,1, 0},
  {0, REG_DWORD,  NKeyPanel,"RightClickRule",&Opt.PanelRightClickRule,2, 0},
  {0, REG_DWORD,  NKeyPanel,"CtrlFRule",&Opt.PanelCtrlFRule,1, 0},
  {0, REG_DWORD,  NKeyPanel,"CtrlAltShiftRule",&Opt.PanelCtrlAltShiftRule,0, 0},
  {0, REG_DWORD,  NKeyPanel,"RememberLogicalDrives",&Opt.RememberLogicalDrives, 0, 0},
  {1, REG_DWORD,  NKeyPanel,"AutoUpdateLimit",&Opt.AutoUpdateLimit, 0, 0},
  /* $ 17.12.2001 IS поведение средней кнопки мыши в панелях */
  {1, REG_DWORD,  NKeyPanel,"MiddleClickRule",&Opt.PanelMiddleClickRule,1, 0},
  /* IS $ */

  {1, REG_DWORD,  NKeyPanelLeft,"Type",&Opt.LeftPanel.Type,0, 0},
  {1, REG_DWORD,  NKeyPanelLeft,"Visible",&Opt.LeftPanel.Visible,1, 0},
  {1, REG_DWORD,  NKeyPanelLeft,"Focus",&Opt.LeftPanel.Focus,1, 0},
  {1, REG_DWORD,  NKeyPanelLeft,"ViewMode",&Opt.LeftPanel.ViewMode,2, 0},
  {1, REG_DWORD,  NKeyPanelLeft,"SortMode",&Opt.LeftPanel.SortMode,1, 0},
  {1, REG_DWORD,  NKeyPanelLeft,"SortOrder",&Opt.LeftPanel.SortOrder,1, 0},
  {1, REG_DWORD,  NKeyPanelLeft,"SortGroups",&Opt.LeftPanel.SortGroups,0, 0},
  {1, REG_DWORD,  NKeyPanelLeft,"ShortNames",&Opt.LeftPanel.ShowShortNames,0, 0},
  {1, REG_SZ,     NKeyPanelLeft,"Folder",Opt.LeftFolder,sizeof(Opt.LeftFolder),""},
  {1, REG_SZ,     NKeyPanelLeft,"CurFile",Opt.LeftCurFile,sizeof(Opt.LeftCurFile),""},
  {1, REG_DWORD,  NKeyPanelLeft,"SelectedFirst",&Opt.LeftSelectedFirst,0,0},

  {1, REG_DWORD,  NKeyPanelRight,"Type",&Opt.RightPanel.Type,0, 0},
  {1, REG_DWORD,  NKeyPanelRight,"Visible",&Opt.RightPanel.Visible,1, 0},
  {1, REG_DWORD,  NKeyPanelRight,"Focus",&Opt.RightPanel.Focus,0, 0},
  {1, REG_DWORD,  NKeyPanelRight,"ViewMode",&Opt.RightPanel.ViewMode,2, 0},
  {1, REG_DWORD,  NKeyPanelRight,"SortMode",&Opt.RightPanel.SortMode,1, 0},
  {1, REG_DWORD,  NKeyPanelRight,"SortOrder",&Opt.RightPanel.SortOrder,1, 0},
  {1, REG_DWORD,  NKeyPanelRight,"SortGroups",&Opt.RightPanel.SortGroups,0, 0},
  {1, REG_DWORD,  NKeyPanelRight,"ShortNames",&Opt.RightPanel.ShowShortNames,0, 0},
  {1, REG_SZ,     NKeyPanelRight,"Folder",Opt.RightFolder,sizeof(Opt.RightFolder),""},
  {1, REG_SZ,     NKeyPanelRight,"CurFile",Opt.RightCurFile,sizeof(Opt.RightCurFile),""},
  {1, REG_DWORD,  NKeyPanelRight,"SelectedFirst",&Opt.RightSelectedFirst,0, 0},

  {1, REG_DWORD,  NKeyPanelLayout,"ColumnTitles",&Opt.ShowColumnTitles,1, 0},
  {1, REG_DWORD,  NKeyPanelLayout,"StatusLine",&Opt.ShowPanelStatus,1, 0},
  {1, REG_DWORD,  NKeyPanelLayout,"TotalInfo",&Opt.ShowPanelTotals,1, 0},
  {1, REG_DWORD,  NKeyPanelLayout,"FreeInfo",&Opt.ShowPanelFree,0, 0},
  {1, REG_DWORD,  NKeyPanelLayout,"Scrollbar",&Opt.ShowPanelScrollbar,0, 0},
  {0, REG_DWORD,  NKeyPanelLayout,"ScrollbarMenu",&Opt.ShowMenuScrollbar,1, 0},
  {1, REG_DWORD,  NKeyPanelLayout,"ScreensNumber",&Opt.ShowScreensNumber,1, 0},
  {1, REG_DWORD,  NKeyPanelLayout,"SortMode",&Opt.ShowSortMode,1, 0},

  {1, REG_DWORD,  NKeyLayout,"HeightDecrement",&Opt.HeightDecrement,0, 0},
  {1, REG_DWORD,  NKeyLayout,"WidthDecrement",&Opt.WidthDecrement,0, 0},
  {1, REG_SZ,     NKeyLayout,"PassiveFolder",Opt.PassiveFolder,sizeof(Opt.PassiveFolder),""},
  {1, REG_DWORD,  NKeyLayout,"FullscreenHelp",&Opt.FullScreenHelp,0, 0},

  {1, REG_SZ,     NKeyDescriptions,"ListNames",Opt.Diz.ListNames,sizeof(Opt.Diz.ListNames),"Descript.ion,Files.bbs"},
  {1, REG_DWORD,  NKeyDescriptions,"UpdateMode",&Opt.Diz.UpdateMode,DIZ_UPDATE_IF_DISPLAYED, 0},
  {1, REG_DWORD,  NKeyDescriptions,"ROUpdate",&Opt.Diz.ROUpdate,0, 0},
  {1, REG_DWORD,  NKeyDescriptions,"SetHidden",&Opt.Diz.SetHidden,TRUE, 0},
  {1, REG_DWORD,  NKeyDescriptions,"StartPos",&Opt.Diz.StartPos,0, 0},

  {0, REG_DWORD,  NKeyKeyMacros,"MacroReuseRules",&Opt.MacroReuseRules,0, 0},

  {0, REG_DWORD,  NKeyPolicies,"ShowHiddenDrives",&Opt.Policies.ShowHiddenDrives,1, 0},
  {0, REG_DWORD,  NKeyPolicies,"DisabledOptions",&Opt.Policies.DisabledOptions,0, 0},

  {0, REG_SZ,     NKeyKeyMacros,"DateFormat",Opt.DateFormat,sizeof(Opt.DateFormat),"%a %b %d %H:%M:%S %Z %Y"},
};


void ReadConfig()
{
  int I;
  DWORD OptPolicies_ShowHiddenDrives,  OptPolicies_DisabledOptions;
  char KeyNameFromReg[34];

  /* <ПРЕПРОЦЕССЫ> *************************************************** */
  // "Вспомним" путь для дополнительного поиска плагинов
  SetRegRootKey(HKEY_LOCAL_MACHINE);
  GetRegKey(NKeySystem,"TemplatePluginsPath",PersonalPluginsPath,"",sizeof(Opt.PersonalPluginsPath));
  OptPolicies_ShowHiddenDrives=GetRegKey(NKeyPolicies,"ShowHiddenDrives",1)&1;
  OptPolicies_DisabledOptions=GetRegKey(NKeyPolicies,"ShowHiddenDrives",0);
  SetRegRootKey(HKEY_CURRENT_USER);
  if(Opt.ExceptRules == -1)
    GetRegKey("System","ExceptRules",Opt.ExceptRules,1);
  /* *************************************************** </ПРЕПРОЦЕССЫ> */

  for(I=0; I < sizeof(CFG)/sizeof(CFG[0]); ++I)
  {
    switch(CFG[I].ValType)
    {
      case REG_DWORD:
       GetRegKey(CFG[I].KeyName,CFG[I].ValName,*(int *)CFG[I].ValPtr,(DWORD)CFG[I].DefDWord);
       break;
      case REG_SZ:
       GetRegKey(CFG[I].KeyName,CFG[I].ValName,(char*)CFG[I].ValPtr,CFG[I].DefStr,CFG[I].DefDWord);
       break;
      case REG_BINARY:
       GetRegKey(CFG[I].KeyName,CFG[I].ValName,(BYTE*)CFG[I].ValPtr,(BYTE*)CFG[I].DefStr,CFG[I].DefDWord);
       break;
    }
  }

  /* <ПОСТПРОЦЕССЫ> *************************************************** */
  /* $ 02.04.2001 VVM
    + Opt.FlagPosixSemantics не пашет под 9x */
  if (WinVer.dwPlatformId!=VER_PLATFORM_WIN32_NT)
    Opt.FlagPosixSemantics=0;
  /* VVM $ */

  // Умолчание разное для разных платформ.
  if(Opt.AltF9 == -1)
    Opt.AltF9=WinVer.dwPlatformId==VER_PLATFORM_WIN32_NT?1:0;

  Opt.HelpTabSize=8; // пока жестко пропишем...

  //   Уточняем алгоритм "взятия" палитры.
  for(I=COL_PRIVATEPOSITION_FOR_XRENZNAETCHEGO-COL_FIRSTPALETTECOLOR+1;
      I < (COL_LASTPALETTECOLOR-COL_FIRSTPALETTECOLOR);
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
  /* SVS 13.12.2000 $ */

  Opt.ViewerIsWrap&=1;
  if(RegVer) Opt.ViewerWrap&=1; else Opt.ViewerWrap=0;

  /* $ 03.12.2001 IS
      Если EditorUndoSize слишком маленькое или слишком большое,
      то сделаем размер undo такой же, как и в старых версиях
  */
  if(Opt.EditorUndoSize<64 || Opt.EditorUndoSize>(0x7FFFFFFF-2))
    Opt.EditorUndoSize=64;
  /* IS $ */

  // Исключаем случайное стирание разделителей ;-)
  if(!strlen(Opt.WordDiv))
     strcpy(Opt.WordDiv,WordDiv0);
  // Исключаем случайное стирание разделителей
  if(!strlen(Opt.XLat.WordDivForXlat))
     strcpy(Opt.XLat.WordDivForXlat,WordDivForXlat0);
  if(Opt.MaxPositionCache < 16 || Opt.MaxPositionCache > 128)
    Opt.MaxPositionCache=64;
  Opt.PanelRightClickRule%=3;
  Opt.PanelCtrlAltShiftRule%=3;
  Opt.ConsoleDetachKey=KeyNameToKey(KeyNameConsoleDetachKey);
  if (Opt.EdOpt.TabSize<1 || Opt.EdOpt.TabSize>512)
    Opt.EdOpt.TabSize=8;
  if (Opt.ViOpt.TabSize<1 || Opt.ViOpt.TabSize>512)
    Opt.ViOpt.TabSize=8;

  GetRegKey(NKeyXLat,"EditorKey",KeyNameFromReg,szCtrlShiftX,sizeof(KeyNameFromReg)-1);
  if((Opt.XLat.XLatEditorKey=KeyNameToKey(KeyNameFromReg)) == -1)
    Opt.XLat.XLatEditorKey=0;
  GetRegKey(NKeyXLat,"CmdLineKey",KeyNameFromReg,szCtrlShiftX,sizeof(KeyNameFromReg)-1);
  if((Opt.XLat.XLatCmdLineKey=KeyNameToKey(KeyNameFromReg)) == -1)
    Opt.XLat.XLatCmdLineKey=0;
  GetRegKey(NKeyXLat,"DialogKey",KeyNameFromReg,szCtrlShiftX,sizeof(KeyNameFromReg)-1);
  if((Opt.XLat.XLatDialogKey=KeyNameToKey(KeyNameFromReg)) == -1)
    Opt.XLat.XLatDialogKey=0;
  GetRegKey(NKeyXLat,"AltEditorKey",KeyNameFromReg,szCtrlShiftX,sizeof(KeyNameFromReg)-1);
  if((Opt.XLat.XLatAltEditorKey=KeyNameToKey(KeyNameFromReg)) == -1)
    Opt.XLat.XLatAltEditorKey=0;
  GetRegKey(NKeyXLat,"AltCmdLineKey",KeyNameFromReg,szCtrlShiftX,sizeof(KeyNameFromReg)-1);
  if((Opt.XLat.XLatAltCmdLineKey=KeyNameToKey(KeyNameFromReg)) == -1)
    Opt.XLat.XLatAltCmdLineKey=0;
  GetRegKey(NKeyXLat,"AltDialogKey",KeyNameFromReg,szCtrlShiftX,sizeof(KeyNameFromReg)-1);
  if((Opt.XLat.XLatAltDialogKey=KeyNameToKey(KeyNameFromReg)) == -1)
    Opt.XLat.XLatAltDialogKey=0;

  FileList::ReadPanelModes();
  GetTempPath(sizeof(Opt.TempPath),Opt.TempPath);
  RemoveTrailingSpaces(Opt.TempPath);
  AddEndSlash(Opt.TempPath);
  CtrlObject->EditorPosCache->Read("Editor\\LastPositions");
  CtrlObject->ViewerPosCache->Read("Viewer\\LastPositions");

  // уточняем системную политику
  // для дисков HKCU может только отменять показ
  Opt.Policies.ShowHiddenDrives&=OptPolicies_ShowHiddenDrives;
  // для опций HKCU может только добавлять блокироку пунктов
  Opt.Policies.DisabledOptions|=OptPolicies_DisabledOptions;

  /* *************************************************** </ПОСТПРОЦЕССЫ> */
}


void SaveConfig(int Ask)
{
  char OutText2[NM];
  int I;

  if (Ask && Message(0,2,MSG(MSaveSetupTitle),MSG(MSaveSetupAsk1),MSG(MSaveSetupAsk2),MSG(MSaveSetup),MSG(MCancel))!=0)
    return;

  /* <ПРЕПРОЦЕССЫ> *************************************************** */
  Panel *LeftPanel=CtrlObject->Cp()->LeftPanel;
  Panel *RightPanel=CtrlObject->Cp()->RightPanel;

  Opt.LeftPanel.Focus=LeftPanel->GetFocus();
  Opt.LeftPanel.Visible=LeftPanel->IsVisible();
  Opt.RightPanel.Focus=RightPanel->GetFocus();
  Opt.RightPanel.Visible=RightPanel->IsVisible();

  CtrlObject->Cp()->GetAnotherPanel(CtrlObject->Cp()->ActivePanel)->GetCurDir(Opt.PassiveFolder);

  if (LeftPanel->GetMode()==NORMAL_PANEL)
  {
    Opt.LeftPanel.Type=LeftPanel->GetType();
    Opt.LeftPanel.ViewMode=LeftPanel->GetViewMode();
    Opt.LeftPanel.SortMode=LeftPanel->GetSortMode();
    Opt.LeftPanel.SortOrder=LeftPanel->GetSortOrder();
    Opt.LeftPanel.SortGroups=LeftPanel->GetSortGroups();
    Opt.LeftPanel.ShowShortNames=LeftPanel->GetShowShortNamesMode();
    Opt.LeftSelectedFirst=LeftPanel->GetSelectedFirstMode();
  }
  LeftPanel->GetCurDir(Opt.LeftFolder);
  LeftPanel->GetCurBaseName(Opt.LeftCurFile,OutText2);

  if (RightPanel->GetMode()==NORMAL_PANEL)
  {
    Opt.RightPanel.Type=RightPanel->GetType();
    Opt.RightPanel.ViewMode=RightPanel->GetViewMode();
    Opt.RightPanel.SortMode=RightPanel->GetSortMode();
    Opt.RightPanel.SortOrder=RightPanel->GetSortOrder();
    Opt.RightPanel.SortGroups=RightPanel->GetSortGroups();
    Opt.RightPanel.ShowShortNames=RightPanel->GetShowShortNamesMode();
    Opt.RightSelectedFirst=RightPanel->GetSelectedFirstMode();
  }
  RightPanel->GetCurDir(Opt.RightFolder);
  RightPanel->GetCurBaseName(Opt.RightCurFile,OutText2);
  CtrlObject->HiFiles->SaveHiData();
  /* *************************************************** </ПРЕПРОЦЕССЫ> */

  for(I=0; I < sizeof(CFG)/sizeof(CFG[0]); ++I)
  {
    if(CFG[I].IsSave)
      switch(CFG[I].ValType)
      {
        case REG_DWORD:
         SetRegKey(CFG[I].KeyName,CFG[I].ValName,*(int *)CFG[I].ValPtr);
         break;
        case REG_SZ:
         SetRegKey(CFG[I].KeyName,CFG[I].ValName,(char*)CFG[I].ValPtr);
         break;
        case REG_BINARY:
         SetRegKey(CFG[I].KeyName,CFG[I].ValName,(BYTE*)CFG[I].ValPtr,CFG[I].DefDWord);
         break;
      }
  }

  /* <ПОСТПРОЦЕССЫ> *************************************************** */
  PanelFilter::SaveSelection();
  FileList::SavePanelModes();
  if (Ask)
    CtrlObject->Macro.SaveMacros();
  /* *************************************************** </ПОСТПРОЦЕССЫ> */
}
