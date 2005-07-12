#ifndef __FARSTRUCT_HPP__
#define __FARSTRUCT_HPP__
/*
struct.hpp

все независимые структуры (которые содержат только простые типы)

*/

/* Revision: 1.133 12.07.2005 $ */

/*
Modify:
  12.07.2005 SVS
    ! опции, ответственные за копирование вынесены в отдельную структуру CopyMoveOptions
    + Opt.CMOpt.CopySecurityOptions - что делать с опцией "Copy access rights"? (набор битов)
  07.07.2005 SVS
    ! Вьюверные настройки собраны в одно место
  05.07.2005 SVS
    ! Все настройки, относящиеся к редактору внесены в структуру EditorOptions
  21.06.2005 SKV
    + Opt.AllowEmptySpaceAfterEof
  14.06.2005 SVS
    + Opt.ShowTimeoutDACLFiles
  30.05.2005 SVS
    ! временно откатим проект про USB
  06.05.2005 SVS
    + Confirmation.RemoveSUBST, Confirmation.RemoveUSB, Confirmation.AfterRemoveUSB
  14.04.2005 SVS
    + Opt.UsePrintManager
  12.04.2005 KM
    ! Дополнена структура FindFileOptions двумя параметрами:
      Opt.FindOpt.SearchInFirst и Opt.FindOpt.SearchInFirstSize
  06.04.2005 SVS
    ! Opt.EdOpt.ExpandTabColor свое отслужил, выкидываем :-)
  05.04.2005 SVS
    + Opt.EdOpt.ExpandTabColor
  03.03.2005 SVS
    + Opt.MsWheelDeltaHelp
  01.03.2005 SVS
    + struct TreeOptions
    ! Opt.AutoChangeFolder -> Opt.Tree.AutoChangeFolder
  02.02.2005 SVS
    + DialogsOptions.CBoxMaxHeight - максимальный размер открываемого списка (по умолчанию=8)
  30.08.2004 SVS
    + Opt.IgnoreErrorBadPathName - Игнорировать ошибку ERROR_BAD_PATHNAME под масдаем, по умолчанию = 0
  18.05.2004 SVS
    + В структуру PanelOptions добавлен член NumericSort
  07.05.2004 SVS
    + DialogsOptions.DelRemovesBlocks
  01.03.2004 SVS
    + FAR_ANSI
  28.02.2004 SVS
    + Opt.AutoUpdateRemoteDrive - управление автоапдейтом сетевых дисков
  15.01.2004 SVS
    + Opt.ExcludeCmdHistory - в историю только то, что вводили с клавиатуры
  07.01.2004 SVS
    + XLat.XLatFastFindKey и XLat.XLatAltFastFindKey - транслитерация для FastFind
    + Opt.ExecuteShowErrorMessage
  18.12.2003 SVS
    + DialogsOptions.MouseButton - Отключение восприятие правой/левой кнопки мышы как команд закрытия окна диалога
      Бит есть - функция работает
    ! перечисления FSizeType и FDateType переехали из struct.hpp в farconst.hpp
    + Opt.HistoryCount, FoldersHistoryCount, ViewHistoryCount, DialogsHistoryCount.
    + Opt.CASRule
  29.10.2003 SVS
    + LoadPluginsOptions.SilentLoadPlugin - тихий режим загрузки плагинов
  14.10.2003 SVS
    ! Opt.FileSearchMode и Opt.FindFolders вынесены в отдельную структуру struct FindFileOptions
    + FindFileOptions.CollectFiles - собирать NamesList для поисковика (когда жмем F3 в диалоге результатов поиска)
  13.10.2003 SVS
    ! переименование:
      Opt.KeyMacroRecord1  -> Opt.KeyMacroCtrlDot
      Opt.KeyMacroRecord2  -> Opt.KeyMacroCtrlShiftDot
  10.10.2003 SVS
    + EditorOptions.WordDiv
  05.10.2003 KM
    + struct FilterParams, Opt.OpFilter
  04.10.2003 SVS
    + Opt.KeyMacroRecord1 аля KEY_CTRLDOT и Opt.KeyMacroRecord2  аля KEY_CTRLSHIFTDOT
  17.09.2003 KM
    + Opt.CharTable
  25.08.2003 SVS
    ! Opt.QuotedName - DWORD, т.к. битовые флаги
  25.08.2003 SVS
    + Opt.QuotedName - заключать имена файлов/папок в кавычки
  23.07.2003 SVS
    + Opt.ScrSize.DeltaXY - уточнение размера для "распаховки" консоли
  06.06.2003 SVS
    ! Все, что связано с загрузкой плагинов объединено в структуру LoadPluginsOptions
  19.05.2003 SVS
    - неправильные комментарии в struct DialogsOptions
    ! DialogsOptions.SelectedType -> DialogsOptions.EditLine
  14.05.2003 VVM
    + ViewerOptions.PersistentBlocks - постоянные блоки во вьюере
  06.05.2003 SVS
    ! Opt.UseTTFFont заменена на Opt.UseUnicodeConsole - так вернее
  21.04.2003 SVS
    + Opt.DelThreadPriority
    + struct ScreenSizes, Opt.ScrSize - для отладки "Alt-Enter"
  05.03.2003 SVS
    + Opt.ScanJunction - сканировать так же симлинки.
  10.02.2003 SVS
    + Opt.ShowTimeoutDelFiles; // тайаут в процессе удаления (в ms)
  13.01.2003 SVS
    + Opt.SortFolderExt
  23.12.2002 SVS
    ! OnlyEditorViewerUsed стал частью структуры Options
  07.10.2002 SVS
    + Opt.SetupArgv - количество каталогов в комюстроке ФАРа
  12.08.2002 SVS
   + Opt.ExecuteUseAppPath
  30.05.2002 SVS
    + Opt.UseTTFFont
  25.05.2002 IS
    ! внедрение const
  24.05.2002 SVS
    + Opt.UseNumPad
  22.05.2002 SVS
    + Opt.CloseCDGate
  05.04.2002 SVS
    + Opt.PluginMaxReadData
  01.04.2002 SVS
    + struct NowellOptions
  18.03.2002 SVS
    ! Опции, ответственные за диалоги вынесены в отдельную
      структуру DialogsOptions
    + Opt.Dialogs.SelectedType
  12.03.2002 VVM
    + Opt.EscTwiceToInterrupt
      Определяет поведение при прерывании длительной операции
      0 - второй ESC продолжает операцию
      1 - второй ESC прерывает операцию
  19.02.2002 SVS
    ! В таблицах и правилах Opt.XLat - первый байт = размер таблицы.
    ! Opt.XLat.Rules - 80 байт (по 40 правил)
  26.12.2001 SVS
    + Opt.CloseConsoleRule, Opt.Diz.ROUpdate
    ! Opt.CursorSize - плюс еще 2 позиции под Overide-режим
  21.12.2001 SVS
    + Opt.RestoreCPAfterExecute
  17.12.2001 IS
    + Opt.PanelMiddleClickRule - поведение средней кнопки мыши в панелях
  07.12.2001 IS
    + Opt.MultiMakeDir - опция создания нескольких каталогов за один сеанс
  03.12.2001 IS
    + Opt.EditorUndoSize - размер буфера undo в редакторе
  01.11.2001 SVS
    ! уберем Opt.CPAJHefuayor ;-(
  30.10.2001 SVS
    + Opt.CPAJHefuayor
  29.10.2001 IS
    ! SaveEditorPos и SaveEditorShortPos переехали в EditorOptions
  26.10.2001 KM
    + Opt.FindFolders. Запомнить флаг разрешения поиска каталогов в Alt-F7
  19.10.2001 SVS
    + struct PreRedrawParamStruct - для исправления BugZ#85
  15.10.2001 SVS
    + Opt.DlgSelectFromHistory
  12.10.2001 SVS
    ! Ну охренеть (Opt.FolderSetAttr165!!!) - уже и так есть то, что надо:
      Opt.SetAttrFolderRules!
  11.10.2001 SVS
    + Opt.FolderSetAttr165; // поведение для каталогов как у 1.65
  07.10.2001 SVS
    + Opt.HelpTabSize - размер табуляции по умолчанию.
  26.09.2001 SVS
    + Opt.AutoUpdateLimit -  выше этого количество автоматически
      не обновлять панели.
  08.09.2001 VVM
    + Постоянные блоки в строках ввода - Opt.DialogsEditBlock;
      0 - Блоки непостоянные
      1 - Блоки постоянные
  05.09.2001 SVS
    + сктруктура HighlightDataColor
  03.08.2001 IS
    + опция "разрешить мультикопирование/перемещение/создание связей":
      Opt.MultiCopy
  24.07.2001 SVS
    + Opt.PgUpChangeDisk
    + Opt.Confirmation.HistoryClear
  04.07.2001 SVS
    + Opt.LCIDSort
  22.06.2001 SVS
    + Opt.DateFormat
  21.05.2001 OT
    + Opt.AltF9
    + Opt.Confirmation.AllowReedit
  14.05.2001 SVS
    + Opt.ShowCheckingFile - щоб управлять мельканием в заголовке...
  06.05.2001 DJ
    ! перетрях #include
  04.05.2001 SVS
    ! Наконец то дошли руки до DI_LISTBOX ;-) - новый член FarDialogItem.ListPos
  28.04.2001 VVM
    + Opt.SubstNameRule битовая маска:
      0 - если установлен, то опрашивать сменные диски при GetSubstName()
      1 - если установлен, то опрашивать все остальные при GetSubstName()
  26.04.2001 VVM
    + Opt.MsWheelDeltaView - задает смещение для прокрутки вьюера.
    + Opt.MsWheelDeltaEdit - задает смещение для прокрутки редактора.
    + Opt.MouseWheelDelta -> Opt.MsWheelDelta
  23.04.2001 SVS
    ! Новые члены GroupSortData.OriginalMasks и HighlightData.OriginalMasks
      новый вгляд на %PATHEXT% - то что редактируем и то, что юзаем -
      разные сущности.
  22.04.2001 SVS
    + Opt.QuotedSymbols - разделители для QuoteSpace()
  16.04.2001 VVM
    + Opt.MouseWheelDelta - задает смещение для прокрутки.
  02.04.2001 VVM
    + Opt.FlagPosixSemantics будет влиять на:
        добавление файлов в историю с разным регистром
        добавление LastPositions в редакторе и вьюере
  30.03.2001 SVS
   + Opt.Policies - прообраз полиции.
  29.03.2001 IS
   + struct ViewerOptions
  28.03.2001 VVM
   + Opt.RememberLogicalDrives = запоминать логические диски и не опрашивать
     каждый раз. Для предотвращения "просыпания" "зеленых" винтов.
  20.03.2001 SVS
   + Opt.FullScreenHelp - для унификации интерфейса конфигурации
  16.03.2001 SVS
   + Opt.ChangeDriveDisconnetMode - для запоминания - как удалять мапленные
     диски (для CheckBox`а)
  15.03.2001 SVS
   + Opt.Confirm.RemoveConnection - подтверждение для удаления мапленных дисков
  12.03.2001 SVS
   + Opt.DeleteSymbolWipe -> Opt.WipeSymbol
  12.03.2001 SVS
   + Opt.DeleteSymbolWipe символ заполнитель для "ZAP-операции"
  27.02.2001 SVS
   + EditorOptions.CharCodeBase - база вывода кода символа
  26.02.2001 VVM
   + Opt.ExceptCallDebugger - вызывать дебаггер или нет
   ! Opt.ExceptRules - битовая маска
  21.02.2001 IS
   ! Часть настроек редактора переехала в EditorOptions
  12.02.2001 SKV
   + ConsoleDetachKey. Клавиша отстёгивания консоли фара от
     долгоиграющего процесса в ней запущенного.
  11.02.2001 SVS
   ! Изменения структур DialogItem и DialogData (DIF_VAREDIT)
   ! HighlightData.Masks - сделан ссылкой (DIF_VAREDIT)
   ! GroupSortData.Masks - сделан ссылкой (DIF_VAREDIT)
   ! FilterDataRecord.Masks - сделан ссылкой (DIF_VAREDIT)
  11.02.2001 SVS
   ! Изменения в MenuItem.
  09.02.2001 IS
   + RightSelectedFirst, LeftSelectedFirst;
   + Confirmation.Esc
  30.01.2001 VVM
   + Показывает время копирования,оставшееся время и среднюю скорость.
      Зависит от настроек в реестре CopyTimeRule
  22.01.2001 SVS
   + Opt.CursorSize[2] - Размер курсора ФАРа :-)
  19.01.2001 SVS
   + Opt.MacroReuseRules - повторное использование.
  09.01.2001 SVS
   + Opt.ShiftsKeyRules - Правило на счет выбора механизма трансляции
     Alt-Буква для нелатинским буковок и символов "`-=[]\;',./" с
     модификаторами Alt-, Ctrl-, Alt-Shift-, Ctrl-Shift-, Ctrl-Alt-
  28.12.2000 SVS
   + Opt.HotkeyRules - Правило на счет выбора механизма хоткеев
  21.12.2000 SVS
   ! структура MacroRecord перенесена в macro.hpp
  08.12.2000 SVS
   ! изменения в структуре DialogItem.
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
   + Разграничитель слов из реестра для функции Xlat (WordDivForXlat)
  24.11.2000 SVS
   ! XLat.Rules - 3 по 30 правил, вместо 10
   + Правило на счет установки атрибутов на каталоги
  04.11.2000 SVS
   ! XLat - все что относится к XLat - в одну структуру.
   + XLat - добавление альтернативных клавиш:
       XLatAltEditorKey, XLatAltCmdLineKey, XLatAltDialogKey;
  20.10.2000 SVS
   + Opt.PanelCtrlFRule
      Panel/CtrlFRule в реестре - задает поведение Ctrl-F
      Если = 0, то штампуется файл как есть, иначе - с учетом
      отображения на панели
  17.10.2000 SVS
   ! WordDiv имеет размер 256;
  10.10.2000 SVS
   + Opt.EditorBSLikeDel
  27.09.2000 SVS
   + Opt.HelpURLRules - =0 отключить возможность запуска URL-приложений
   + Opt.AllCtrlAltShiftRule - битовые флаги, задают поведение Ctrl-Alt-Shift
     бит установлен - функция включена:
     0 - Panel
     1 - Edit
     2 - View
     3 - Help
     4 - Dialog
  24.09.2000 SVS
   + Opt.MaxPositionCache - количество позиций в кэше сохранения
   + Opt.SaveViewerShortPos - запоминание позиций во вьювере по Ctrl-0..9
   + Opt.SaveEditorShortPos - запоминание позиций в редакторе по Ctrl-0..9
   + Opt.CmdHistoryRule задает поведение Esc для командной строки.
   + Клавиши для вызова функции Xlat:
     Opt.XLatEditorKey, Opt.XLatCmdLineKey, Opt.XLatDialogKey
  20.09.2000 SVS
   + Opt.SubstPluginPrefix - 1 = подстанавливать префикс плагина
     для Ctrl-[ и ему подобные
  19.09.2000 SVS
   + Opt.PanelCtrlAltShiftRule задает поведение Ctrl-Alt-Shift для панелей.
  12.09.2000 SVS
   + Добавлена переменная Options.ViewerWrap
   ! ViewerTypeWrap переименована в ViewerIsWrap
       Разделение Wrap/WWrap/UnWrap на 2 составляющих -
       Состояние (Wrap/UnWrap) и тип (Wrap/WWrap)
        ViewerIsWrap  =  UnWrap=0  | Перенос=1
        ViewerWrap    =  Wrap=0    | WordWarp=1
   + Opt.PanelRightClickRule задает поведение правой клавиши мыши
     (это по поводу Bug#17)
  11.09.2000 SVS
   + В Opt добавлена переменная DlgEULBsClear
     если = 1, то BS в диалогах для UnChanged строки удаляет такую
     строку также, как и Del
  09.09.2000 SVS 1.14
   + CHAR_INFO *VBuf; в элементах диалога
  07.09.2000 tran 1.13
   + Config//Current File
  05.09.2000 SVS 1.12
   + Структура CodeQWERTY, описывающая QWERTY-перекодировщик
   ! В Opt добавлен блок переменный, касаемых QWERTY-перекодировки
  01.09.2000 tran 1.11
   + Options.PluginsCacheOnly - грузить плагины только из кеша
  31.08.2000 SVS
   ! DialogItem.Flags, DialogData.Flags - тип DWORD
   - не сохраняется тип врапа.
     Добавлена переменная Options.ViewerTypeWrap
  12.08.2000 KM 1.09
   ! В структурах DialogItem и DialogData новое поле, включенное
     в union, char *Mask.
  18.08.2000 SVS
   ! struct FarListItems -> struct FarList, а то совсем запутался :-)
  03.08.2000 SVS
   ! WordDiv -> Opt.WordDiv
  03.08.2000 SVS
   + Добавка в Options: MainPluginDir - использовать основной путь для
     поиска плагинов...
  01.08.2000 SVS
   ! Изменения в структурах Dialog*
  26.07.2000 SVS
   + Opt.AutoComplete
  18.07.2000 tran 1.04
   + Opt.ViewerShowScrollBar, Opt.ViewerShowArrows
  15.07.2000 tran
   + добавлен аттрибут показа KeyBar в Viewer - Options::ShowKeyBarViewer
  15.07.2000 SVS
   + Opt.PersonalPluginsPath - путь для поиска персональных плагинов
  29.06.2000 SVS
   + Добавлен атрибут показа Scroll Bar в меню - Options::ShowMenuScrollbar
  25.06.2000 SVS
   ! Подготовка Master Copy
   ! Выделение в качестве самостоятельного модуля
*/

#include "farconst.hpp"

struct FilterParams
{
  struct
  {
    DWORD Used;
    char Mask[NM*2];
  } FMask;
  struct
  {
    DWORD Used;
    FDateType DateType;
    FILETIME DateAfter;
    FILETIME DateBefore;
  } FDate;
  struct
  {
    DWORD Used;
    FSizeType SizeType;
    __int64 SizeAbove;
    __int64 SizeBelow;
  } FSize;
  struct
  {
    DWORD Used;
    DWORD AttrSet;
    DWORD AttrClear;
  } FAttr;
};

struct PanelOptions
{
  int Type;
  int Visible;
  int Focus;
  int ViewMode;
  int SortMode;
  int SortOrder;
  int SortGroups;
  int ShowShortNames;
  int NumericSort;
};

struct Confirmation
{
  int Copy;
  int Move;
  int Drag;
  int Delete;
  int DeleteFolder;
  int Exit;
  /* $ 09.02.2001 IS
     Для CheckForEsc
  */
  int Esc;
  /* IS $ */
  /* $ 12.03.2002 VVM
    + Opt.EscTwiceToInterrupt
      Определяет поведение при прерывании длительной операции
      0 - второй ESC продолжает операцию
      1 - второй ESC прерывает операцию */
  int EscTwiceToInterrupt;
  /* VVM $ */
  int RemoveConnection;
  /* $ 23.05.2001
    +  Opt.Confirmation.AllowReedit - Флаг, который изменяет поведение открытия
      файла на редактирование если, данный файл уже редактируется. По умолчанию - 1
      0 - Если уже открытый файл не был изменен, то происходит переход к открытому редактору
          без дополнительных вопросов. Если файл был изменен, то задается вопрос, и в случае
          если выбрана вариант Reload, то загружается новая копия файла, при этом сделанные
          изменения теряются.
      1 - Так как было раньше. Задается вопрос и происходит переход либо уже к открытому файлу
          либо загружается новая версия редактора.
      */
  int AllowReedit;
  int HistoryClear;
  int RemoveSUBST;
};


struct RegInfo
{
  char RegName[256];
  char RegCode[256];
  int Done;
};


struct DizOptions
{
  char ListNames[NM];
  int ROUpdate;
  int UpdateMode;
  int SetHidden;
  int StartPos;
};

/* $ 05.09.2000 SVS
   Структура CodeQWERTY, описывающая QWERTY-перекодировщик
*/
struct CodeXLAT{
  DWORD Flags;       // дополнительные флаги
  /* $ 05.09.2000 SVS
     В Opt добавлены клавиши, вызывающие функцию Xlat
  */
  int XLatEditorKey;
  int XLatCmdLineKey;
  int XLatDialogKey;
  int XLatFastFindKey;
  /* SVS $*/
  /* $ 04.11.2000 SVS
     В Opt добавлены альтернативные клавиши, вызывающие функцию Xlat
  */
  int XLatAltEditorKey;
  int XLatAltCmdLineKey;
  int XLatAltDialogKey;
  int XLatAltFastFindKey;
  /* SVS $*/
  /* $ 25.11.2000 IS
     Разграничитель слов из реестра для функции Xlat
  */
  char WordDivForXlat[256];
  /* IS $ */
  // первый байт - размер таблицы
  BYTE Table[2][81]; // [0] non-english буквы, [1] english буквы
  BYTE Rules[3][81]; // 3 по 40 правил:
                    //  [0] "если предыдущий символ латинский"
                    //  [1] "если предыдущий символ нелатинский символ"
                    //  [2] "если предыдущий символ не рус/lat"
};
/* SVS $*/

/* $ 21.02.2001 IS
     Новая структура: настройки редактора
*/
struct EditorOptions
{
  int TabSize;
  int ExpandTabs;
  int PersistentBlocks;
  int DelRemovesBlocks;
  int AutoIndent;
  int AutoDetectTable;
  int AnsiTableForNewFile;
  int CursorBeyondEOL;
  int BSLikeDel;
  int CharCodeBase;
  int SavePos;
  int SaveShortPos;
  int F7Rules; // $ 28.11.2000 SVS - Правило на счет поиска в редакторе
  int AllowEmptySpaceAfterEof; // $ 21.06.2005 SKV - разрешить показывать пустое пространство после последней строки редактируемого файла.
  char WordDiv[256];

  int ReadOnlyLock; // $ 29.11.2000 SVS - лочить файл при открытии в редакторе, если он имеет атрибуты R|S|H
  int UndoSize; // $ 03.12.2001 IS - размер буфера undo в редакторе
  int UseExternalEditor;
  /* $ 29.11.2000 SVS
   + Opt.EditorFileSizeLimit - минимально допустимый размер файла, после
     которого будет выдан диалог о целесообразности открытия подобного
     файла на редактирование
  */
  DWORD FileSizeLimitLo;
  DWORD FileSizeLimitHi;
  /* SVS $ */
};
/* IS $ */

/* $ 29.03.2001 IS
     Тут следует хранить "локальные" настройки для программы просмотра
*/
struct ViewerOptions
{
  int TabSize;
  int AutoDetectTable;
  int ShowScrollbar;     // $ 18.07.2000 tran пара настроек для viewer
  int ShowArrows;
  int PersistentBlocks; // $ 14.05.2002 VVM Постоянные блоки во вьюере
  int ViewerIsWrap; // (Wrap|WordWarp)=1 | UnWrap=0
  int ViewerWrap; // Wrap=0|WordWarp=1
  int SaveViewerPos;
  int SaveViewerShortPos;
  int UseExternalViewer;
  int ShowKeyBarViewer; // $ 15.07.2000 tran + ShowKeyBarViewer
};
/* IS $ */

// "Полиция"
struct PoliciesOptions {
  int DisabledOptions;  // разрешенность меню конфигурации
  int ShowHiddenDrives; // показывать скрытые логические диски
};

struct DialogsOptions{
  int   EditBlock;          // Постоянные блоки в строках ввода
  int   EditHistory;        // Добавлять в историю?
  int   AutoComplete;       // Разрешено автодополнение?
  int   EULBsClear;         // = 1 - BS в диалогах для UnChanged строки удаляет такую строку также, как и Del
  int   SelectFromHistory;  // = 0 then (ctrl-down в строке с историей курсор устанавливался на самую верхнюю строку)
  DWORD EditLine;           // общая информация о строке ввода (сейчас это пока... позволяет управлять выделением)
  int   MouseButton;        // Отключение восприятие правой/левой кнопки мышы как команд закрытия окна диалога
  int   DelRemovesBlocks;
  int   CBoxMaxHeight;      // максимальный размер открываемого списка (по умолчанию=8)
};

struct NowellOptions{
  int MoveRO;               // перед операцией Move снимать R/S/H атрибуты, после переноса - выставлять обратно
};

// Хранилище параметров поиска character table
struct FindCharTable
{
  int AllTables;
  int AnsiTable;
  int UnicodeTable;
  int TableNum;
};

struct ScreenSizes{
  COORD DeltaXY;            // на сколько поз. изменить размеры для распахнутого экрана
#if defined(DETECT_ALT_ENTER)
  /*
    Opt.WScreenSize - Windowed/Full Screen Size
       COORD[0].X - Windowed Width  mode 1
       COORD[0].Y - Windowed Height mode 1
       COORD[1].X - Windowed Width  mode 2
       COORD[1].Y - Windowed Height mode 2

       COORD[2].X - FullScreen Width  mode 1
       COORD[2].Y - FullScreen Height mode 1
       COORD[3].X - FullScreen Width  mode 2
       COORD[3].Y - FullScreen Height mode 2
  */
  int WScreenSizeSet;
  COORD WScreenSize[4];
#endif
};

struct LoadPluginsOptions{
//  DWORD TypeLoadPlugins;       // see TYPELOADPLUGINSOPTIONS
  /* $ 03.08.2000 SVS
     TRUE - использовать стандартный путь к основным плагинам
  */
  int MainPluginDir;
  /* SVS $*/
  /* $ 01.09.2000 tran
     seting by '/co' switch, not saved in registry. */
  int PluginsCacheOnly;
  /* tran $ */
  int PluginsPersonal;

  char CustomPluginsPath[NM];  // путь для поиска плагинов, указанный в /p
  /* $ 15.07.2000 SVS
    + путь для поиска персональных плагинов, большой размер из-за того,
      что здесь может стоять сетевой путь...
  */
  char PersonalPluginsPath[1024];
  /* SVS $*/
  int SilentLoadPlugin; // при загрузке плагина с кривым...
};

struct FindFileOptions{
  int FindFolders;
  int CollectFiles;
  int FileSearchMode;
  int SearchInFirst;
  char SearchInFirstSize[NM];
};

struct TreeOptions{
  int LocalDisk;         // Хранить файл структуры папок для локальных дисков
  int NetDisk;           // Хранить файл структуры папок для сетевых дисков
  int NetPath;           // Хранить файл структуры папок для сетевых путей
  int RemovableDisk;     // Хранить файл структуры папок для сменных дисков
  int MinTreeCount;      // Минимальное количество папок для сохранения дерева в файле.
  int AutoChangeFolder;  // автосмена папок при перемещении по дереву
};

struct CopyMoveOptions{
  int UseSystemCopy;         // использовать системную функцию копирования
  int CopyOpened;            // копировать открытые на запись файлы
  int CopyShowTotal;         // показать общий индикатор копирования
  int MultiCopy;             // "разрешить мультикопирование/перемещение/создание связей"
  DWORD CopySecurityOptions; // для операции Move - что делать с опцией "Copy access rights"
  int CopyTimeRule;          // $ 30.01.2001 VVM  Показывает время копирования,оставшееся время и среднюю скорость
};

struct Options
{
  int Clock;
  int Mouse;
  int ShowKeyBar;
  int ScreenSaver;
  int ScreenSaverTime;
  int UsePromptFormat;
  char PromptFormat[80];
  int AltGr;
  int InactivityExit;
  int InactivityExitTime;
  int ShowHidden;
  int Highlight;
  char LeftFolder[NM];
  char RightFolder[NM];
  /* $ 07.09.2000 tran
     + Config//Current File */
  char LeftCurFile[NM];
  char RightCurFile[NM];
  /* tran 07.09.2000 $ */

  /* $ 09.02.2001 IS
     состояние режима "помеченное вперед"
  */
  int RightSelectedFirst;
  int LeftSelectedFirst;
  /* IS $ */
  int SelectFolders;
  int ReverseSort;
  int ClearReadOnly;
  int SortFolderExt;
  int DeleteToRecycleBin;
  int WipeSymbol; // символ заполнитель для "ZAP-операции"

  struct CopyMoveOptions CMOpt;

  /* IS $ */
  /* $ 07.12.2001 IS Опция создания нескольких каталогов за один сеанс */
  int MultiMakeDir;
  /* IS $ */
  int CreateUppercaseFolders;
  int UseRegisteredTypes;

  int ViewerEditorClock;
  int OnlyEditorViewerUsed; // =1, если старт был /e или /v
  int SaveViewHistory;
  int ViewHistoryCount;

  char ExternalEditor[NM];
  struct EditorOptions EdOpt;
  char ExternalViewer[NM];
  struct ViewerOptions ViOpt;


  char WordDiv[256]; // $ 03.08.2000 SVS Разграничитель слов из реестра
  char QuotedSymbols[32];
  DWORD QuotedName;
  int SaveHistory;
  int SaveFoldersHistory;
  int AutoSaveSetup;
  int SetupArgv; // количество каталогов в комюстроке ФАРа
  int ChangeDriveMode;
  int ChangeDriveDisconnetMode;

  int HistoryCount;
  int FoldersHistoryCount;
  int DialogsHistoryCount;

  struct FindFileOptions FindOpt;

  char TempPath[NM];
  int HeightDecrement;
  int WidthDecrement;
  char PassiveFolder[NM];
  int ShowColumnTitles;
  int ShowPanelStatus;
  int ShowPanelTotals;
  int ShowPanelFree;
  int ShowPanelScrollbar;
  int ShowMenuScrollbar; // $ 29.06.2000 SVS Добавлен атрибут показа Scroll Bar в меню.
  int ShowScreensNumber;
  int ShowSortMode;
  int ShowMenuBar;

  int CleanAscii;
  int NoGraphics;
  char FolderInfoFiles[1024];

  struct Confirmation Confirm;
  struct DizOptions Diz;
  struct PanelOptions LeftPanel;
  struct PanelOptions RightPanel;

  DWORD  AutoUpdateLimit; // выше этого количество автоматически не обновлять панели.
  int AutoUpdateRemoteDrive;

  char Language[80];
  int SmallIcon;
  char RegRoot[NM];
  /* $ 12.09.2000 SVS
   + Opt.PanelRightClickRule задает поведение правой клавиши мыши
     (это по поводу Bug#17)
  */
  int PanelRightClickRule;
  /* SVS $*/
  /* $ 17.12.2001 IS поведение средней кнопки мыши в панелях */
  int PanelMiddleClickRule;
  /* IS $ */
  /* $ 19.09.2000 SVS
   + Opt.PanelCtrlAltShiftRule задает поведение Ctrl-Alt-Shift для панелей.
  */
  int PanelCtrlAltShiftRule;
  /* SVS $*/
  /* $ 20.10.2000 SVS
    Panel/CtrlFRule в реестре - задает поведение Ctrl-F
    Если = 0, то штампуется файл как есть, иначе - с учетом
    отображения на панели
  */
  int PanelCtrlFRule;
  /* SVS $*/
  /* $ 27.09.2000 SVS
   + Opt.AllCtrlAltShiftRule - битовые флаги, задают поведение Ctrl-Alt-Shift
     бит установлен - функция включена:
     0 - Panel
     1 - Edit
     2 - View
     3 - Help
     4 - Dialog
  */
  int AllCtrlAltShiftRule;
  /* SVS $*/
  int CASRule; // 18.12.2003 - Пробуем различать левый и правый CAS (попытка #1).
  /* $ 24.09.2000 SVS
   + Opt.CmdHistoryRule задает поведение Esc для командной строки:
      =1 - Не изменять положение в History, если после Ctrl-E/Ctrl/-X
           нажали ESC (поведение - аля VC).
      =0 - поведение как и было - изменять положение в History
  */
  int CmdHistoryRule;
  /* SVS $*/
  DWORD ExcludeCmdHistory;
  /* $ 20.09.2000 SVS
   + Opt.SubstPluginPrefix - 1 = подстанавливать префикс плагина
     для Ctrl-[ и ему подобные
  */
  int SubstPluginPrefix;
  /* SVS $*/
  /* $ 24.09.2000 SVS
   + Opt.MaxPositionCache - количество позиций в кэше сохранения
  */
  int MaxPositionCache;
  /* SVS $*/
  /* $ 22.11.2000 SVS
   + Правило на счет установки атрибутов на каталоги*/
  int SetAttrFolderRules;
  /* SVS $ */
  /* $ 27.11.2000 SVS
   + Opt.ExceptRules - Правило на счет вызова исключений */
  int ExceptRules;
  /* $ 26.02.2001 VVM
   + Opt.ExceptCallDebugger - вызывать дебаггер при исключении */
  int ExceptCallDebugger;
  /* VVM $ */
  /* SVS $ */
  /* $ 28.12.2000 SVS
   + Opt.HotkeyRules - Правило на счет выбора механизма хоткеев */
  int HotkeyRules;
  /* SVS $ */
  /* $ 09.01.2001 SVS
   + Opt.ShiftsKeyRules - Правило на счет выбора механизма трансляции
     Alt-Буква для нелатинским буковок и символов "`-=[]\;',./" с
     модификаторами Alt-, Ctrl-, Alt-Shift-, Ctrl-Shift-, Ctrl-Alt- */
  int ShiftsKeyRules;
  /* SVS $ */
  /* $ 19.01.2001 SVS
   + Opt.MacroReuseRules - Правило на счет повторно использования забинденных
     клавиш */
  int MacroReuseRules;
  /* SVS $ */
  int IgnoreErrorBadPathName;

  DWORD KeyMacroCtrlDot; // аля KEY_CTRLDOT
  DWORD KeyMacroCtrlShiftDot; // аля KEY_CTRLSHIFTDOT
  /* $ 22.01.2001 SVS
   + Opt.CursorSize - Размер курсора ФАРа :-)
     клавиш */
  int CursorSize[4];
  /* SVS $ */
  /* $ 05.09.2000 SVS
     В Opt добавлен блок переменный, касаемых QWERTY-перекодировки
  */
  struct CodeXLAT XLat;
  /* SVS $*/
  /*$ 08.02.2001 SKV
    Комбинация клавиш для детача Far'овской консоли
    от длятельного неинтерактивного процесса в ней запущенного.
  */
  int ConsoleDetachKey;
  /* SKV$*/

  int UsePrintManager;

  char HelpLanguage[80];
  int FullScreenHelp;
  int HelpTabSize;
  /* $ 27.09.2000 SVS
   + Opt.HelpURLRules - =0 отключить возможность запуска URL-приложений
  */
  int HelpURLRules;

  /* SVS $*/
  /* $ 28.03.2001 VVM
    + RememberLogicalDrives = запоминать логические диски и не опрашивать
      каждый раз. Для предотвращения "просыпания" "зеленых" винтов. */
  int RememberLogicalDrives;
  /* VVM $ */
  /* $ 02.04.2001 VVM
    + Opt.FlagPosixSemantics будет влиять на:
        добавление файлов в историю с разным регистром
        добавление LastPositions в редакторе и вьюере */
  int FlagPosixSemantics;
  /* VVM $ */
  /* $ 16.04.2001 VVM
    + Opt.MouseWheelDelta - задает смещение для прокрутки. */
  int MsWheelDelta;
  /* VVM $ */
  int MsWheelDeltaView;
  int MsWheelDeltaEdit;
  int MsWheelDeltaHelp;
  /* $ 28.04.2001 VVM
    + Opt.SubstNameRule битовая маска:
      0 - если установлен, то опрашивать сменные диски при GetSubstName()
      1 - если установлен, то опрашивать все остальные при GetSubstName() */
  int SubstNameRule;
  /* VVM $ */

  /* $ 23.05.2001 AltF9
    + Opt.AltF9 Флаг позволяет выбрать механизм  работы комбинации Alt-F9
         (Изменение размера экрана) в оконном режиме. По умолчанию - 1.
      0 - использовать механизм, совместимый с FAR версии 1.70 beta 3 и
         ниже, т.е. переключение 25/50 линий.
      1 - использовать усовершенствованный механизм - окно FAR Manager
         будет переключаться с нормального на максимально доступный размер
         консольного окна и обратно.*/
  int AltF9;
  /* OT $ */
  int PgUpChangeDisk;
  int ShowCheckingFile;
  int CloseConsoleRule;
  int CloseCDGate;       // автомонтирование CD

  DWORD LCIDSort;
  int RestoreCPAfterExecute;
  int ExecuteShowErrorMessage;
  int ExecuteUseAppPath;

#if defined(FAR_ANSI)
  int FarAnsi;
#endif
  DWORD PluginMaxReadData;
  int UseNumPad;
  int UseUnicodeConsole;
  int ScanJunction;

  DWORD ShowTimeoutDelFiles; // тайаут в процессе удаления (в ms)
  DWORD ShowTimeoutDACLFiles;
  int DelThreadPriority; // приоритет процесса удаления, по умолчанию = THREAD_PRIORITY_NORMAL

  //int CPAJHefuayor; // производное от "Close Plugin And Jump:
                  // Highly experimental feature, use at your own risk"

  char DateFormat[80]; // Для $Date
  struct LoadPluginsOptions LoadPlug;

  struct DialogsOptions Dialogs;
  struct PoliciesOptions Policies;
  struct NowellOptions Nowell;
  struct ScreenSizes ScrSize;
  /* $ 17.09.2003 KM
       Структура для запоминания параметров таблиц символов в поиске
  */
  struct FindCharTable CharTable;
  /* KM $ */
  /* $ 28.09.2003 KM
     OpFilter - параметр для запоминания свойств файлового фильтра
  */
  struct FilterParams OpFilter;
  /* KM $ */
  struct TreeOptions Tree;
};


struct PluginHandle
{
  HANDLE InternalHandle;
  int PluginNumber;
};

// for class Edit
struct ColorItem
{
  int StartPos;
  int EndPos;
  int Color;
};

/* ВРЕМЕННОЕ!!! воплощение будущих надежд :-))
   Структура должна быть 16 байт!
*/
struct HighlightDataColor
{
  BYTE Color;
  BYTE SelColor;
  BYTE CursorColor;
  BYTE CursorSelColor;
  BYTE MarkChar;
  BYTE Reserved[11];
};

struct PreRedrawParamStruct
{
  DWORD Flags;
  void *Param1;
  const void *Param2;
  const void *Param3;
  void *Param4;
};

#endif // __FARSTRUCT_HPP__
