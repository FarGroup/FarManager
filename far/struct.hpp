#ifndef __FARSTRUCT_HPP__
#define __FARSTRUCT_HPP__
/*
struct.hpp

все независимые структуры (которые содержат только простые типы)

*/

/* Revision: 1.62 22.06.2001 $ */

/*
Modify:
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
  /* SVS $*/
  /* $ 04.11.2000 SVS
     В Opt добавлены альтернативные клавиши, вызывающие функцию Xlat
  */
  int XLatAltEditorKey;
  int XLatAltCmdLineKey;
  int XLatAltDialogKey;
  /* SVS $*/
  /* $ 25.11.2000 IS
     Разграничитель слов из реестра для функции Xlat
  */
  char WordDivForXlat[256];
  /* IS $ */
  BYTE Table[2][81]; // [0] non-english буквы, [1] english буквы
  BYTE Rules[3][60]; // 3 по 30 правил:
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
  int CursorBeyondEOL;
  int BSLikeDel;
  int CharCodeBase;
};
/* IS $ */

/* $ 29.03.2001 IS
     Тут следует хранить "локальные" настройки для программы просмотра
*/
struct ViewerOptions
{
  int TabSize;
  int AutoDetectTable;
  /* $ 18.07.2000 tran
    + пара настроек для viewer*/
  int ShowScrollbar;
  int ShowArrows;
  /* tran 18.07.2000 $ */
};
/* IS $ */

// "Полиция"
struct PoliciesOptions {
  int DisabledOptions;  // разрешенность меню конфигурации
  int ShowHiddenDrives; // показывать скрытые логические диски
};

struct Options
{
  /* $ 03.08.2000 SVS
     TRUE - использовать стандартный путь к основным плагинам
  */
  int MainPluginDir;
  /* SVS $*/
  int Clock;
  int ViewerEditorClock;
  int Mouse;
  int ShowKeyBar;
  int ScreenSaver;
  int ScreenSaverTime;
  int DialogsEditHistory;
  int UsePromptFormat;
  char PromptFormat[80];
  int AltGr;
  int InactivityExit;
  int InactivityExitTime;
  int ShowHidden;
  int Highlight;
  int AutoChangeFolder;
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
  int RightSelectedFirst, LeftSelectedFirst;
  /* IS $ */
  int SelectFolders;
  int ReverseSort;
  int ClearReadOnly;
  int DeleteToRecycleBin;
  int WipeSymbol; // символ заполнитель для "ZAP-операции"
  int UseSystemCopy;
  int CopyOpened;
  int CopyShowTotal;
  int CreateUppercaseFolders;
  int UseRegisteredTypes;
  int UseExternalViewer;
  char ExternalViewer[NM];
  int UseExternalEditor;
  char ExternalEditor[NM];
  int SaveViewerPos;
  int SaveViewerShortPos;
  /* $ 21.02.2001 IS
       Переменные для редактора переехали в соответствующую структуру
  */
  struct EditorOptions EdOpt;
  /* IS $ */
  /* $ 29.03.2001 IS
       Некоторые переменные для вьюера переехали в соответствующую структуру
  */
  struct ViewerOptions ViOpt;
  /* IS $ */
  /* $ 29.11.2000 SVS
   + Opt.EditorReadOnlyLock - лочить файл при открытии в редакторе, если
     он имеет атрибуты R|S|H
  */
  int EditorReadOnlyLock;
  /* SVS $ */
  /* $ 29.11.2000 SVS
   + Opt.EditorFileSizeLimit - минимально допустимый размер файла, после
     которого будет выдан диалог о целесообразности открытия подобного
     файла на редактирование
  */
  DWORD EditorFileSizeLimitLo;
  DWORD EditorFileSizeLimitHi;
  /* SVS $ */
  /* $ 03.08.2000 SVS
     Разграничитель слов из реестра
  */
  char WordDiv[256];
  /* SVS $ */
  char QuotedSymbols[32];
  int SaveEditorPos;
  int SaveEditorShortPos;
  int SaveHistory;
  int SaveFoldersHistory;
  int SaveViewHistory;
  int AutoSaveSetup;
  int ChangeDriveMode;
  int ChangeDriveDisconnetMode;
  int FileSearchMode;
  char TempPath[NM];
  int HeightDecrement;
  int WidthDecrement;
  char PassiveFolder[NM];
  int ShowColumnTitles;
  int ShowPanelStatus;
  int ShowPanelTotals;
  int ShowPanelFree;
  int ShowPanelScrollbar;
  /* $ 29.06.2000 SVS
    Добавлен атрибут показа Scroll Bar в меню.
  */
  int ShowMenuScrollbar;
  /* SVS $*/
  int ShowScreensNumber;
  int ShowSortMode;
  int ShowMenuBar;
  /* $ 15.07.2000 tran
    + ShowKeyBarViewer*/
  int ShowKeyBarViewer;
  /* tran 15.07.2000 $ */
  int CleanAscii;
  int NoGraphics;
  char FolderInfoFiles[1024];
  struct Confirmation Confirm;
  struct DizOptions Diz;
  struct PanelOptions LeftPanel;
  struct PanelOptions RightPanel;
  char Language[80];
  char HelpLanguage[80];
  int SmallIcon;
  char RegRoot[NM];
  /* $ 15.07.2000 SVS
    + путь для поиска персональных плагинов, большой размер из-за того,
      что здесь может стоять сетевой путь...
  */
  char PersonalPluginsPath[1024];
  /* SVS $*/
  /* $ 26.07.2000 SVS
     Разрешение для функции автозавершения в строках ввода в диалогах
     имеющих History
  */
  int AutoComplete;
  /* SVS $*/
  /* $ 31.08.2000 SVS
     Добавлена переменная Options.ViewerTypeWrap
  */
  /* $ 12.09.2000 SVS
     Добавлена переменная Options.ViewerWrap
     ViewerTypeWrap переименована в ViewerIsWrap
  */
  int ViewerIsWrap; // (Wrap|WordWarp)=1 | UnWrap=0
  int ViewerWrap; // Wrap=0|WordWarp=1
  /* SVS 12.09.2000 $*/
  /* SVS $*/
  /* $ 01.09.2000 tran
     seting by '/co' switch, not saved in registry. */
  int PluginsCacheOnly;
  /* tran $ */
  /* $ 11.09.2000 SVS
     В Opt добавлена переменная DlgEULBsClear
     если = 1, то BS в диалогах для UnChanged строки удаляет такую
     строку также, как и Del
  */
  int DlgEULBsClear;
  /* SVS $*/
  /* $ 12.09.2000 SVS
   + Opt.PanelRightClickRule задает поведение правой клавиши мыши
     (это по поводу Bug#17)
  */
  int PanelRightClickRule;
  /* SVS $*/
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
  /* $ 24.09.2000 SVS
   + Opt.CmdHistoryRule задает поведение Esc для командной строки:
      =1 - Не изменять положение в History, если после Ctrl-E/Ctrl/-X
           нажали ESC (поведение - аля VC).
      =0 - поведение как и было - изменять положение в History
  */
  int CmdHistoryRule;
  /* SVS $*/
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
  /* $ 27.09.2000 SVS
   + Opt.HelpURLRules - =0 отключить возможность запуска URL-приложений
  */
  int HelpURLRules;
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
  /* $ 28.11.2000 SVS
   + Opt.EditorF7Rules - Правило на счет поиска в редакторе */
  int EditorF7Rules;
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
  /* $ 22.01.2001 SVS
   + Opt.CursorSize - Размер курсора ФАРа :-)
     клавиш */
  int CursorSize[2];
  /* SVS $ */
  /* $ 05.09.2000 SVS
     В Opt добавлен блок переменный, касаемых QWERTY-перекодировки
  */
  struct CodeXLAT XLat;
  /* SVS $*/
  /* $ 30.01.2001 VVM
    + Показывает время копирования,оставшееся время и среднюю скорость.
      Зависит от настроек в реестре CopyTimeRule */
  int CopyTimeRule;
  /* VVM $ */
  /*$ 08.02.2001 SKV
    Комбинация клавиш для детача Far'овской консоли
    от длятельного неинтерактивного процесса в ней запущенного.
  */
  int ConsoleDetachKey;
  /* SKV$*/
  int FullScreenHelp;
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
  int ShowCheckingFile;

  char DateFormat[80]; // Для $Date
  struct PoliciesOptions Policies;
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

#endif // __FARSTRUCT_HPP__
