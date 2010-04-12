#ifndef __FARSTRUCT_HPP__
#define __FARSTRUCT_HPP__
/*
struct.hpp

все независимые структуры (которые содержат только простые типы)

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
	int RemoveHotPlug;
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
struct CodeXLAT
{
	DWORD Flags;       // дополнительные флаги
	char WordDivForXlat[256];
	HKL Layouts[10];
	int CurrentLayout;
	// первый байт - размер таблицы
	BYTE Table[2][81]; // [0] non-english буквы, [1] english буквы
	BYTE Rules[3][81]; // 3 по 40 правил:
	//  [0] "если предыдущий символ латинский"
	//  [1] "если предыдущий символ нелатинский символ"
	//  [2] "если предыдущий символ не рус/lat"
};

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
	int AnsiTableAsDefault;
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
	int ShowKeyBar;
	int ShowTitleBar;
	int ShowScrollBar;
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
	int ShowKeyBar; // $ 15.07.2000 tran + ShowKeyBar
	int AnsiTableAsDefault;
	int ShowTitleBar;
};
/* IS $ */

// "Полиция"
struct PoliciesOptions
{
	int DisabledOptions;  // разрешенность меню конфигурации
	int ShowHiddenDrives; // показывать скрытые логические диски
};

struct DialogsOptions
{
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

struct NowellOptions
{
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

struct ScreenSizes
{
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

struct LoadPluginsOptions
{
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

struct FindFileOptions
{
	int FindFolders;
	int FindSymLinks;
	int CollectFiles;
	int FileSearchMode;
	int UseFilter;
	char SearchInFirstSize[NM];
};

struct TreeOptions
{
	int LocalDisk;         // Хранить файл структуры папок для локальных дисков
	int NetDisk;           // Хранить файл структуры папок для сетевых дисков
	int NetPath;           // Хранить файл структуры папок для сетевых путей
	int RemovableDisk;     // Хранить файл структуры папок для сменных дисков
	int MinTreeCount;      // Минимальное количество папок для сохранения дерева в файле.
	int AutoChangeFolder;  // автосмена папок при перемещении по дереву
	DWORD TreeFileAttr;      // файловые атрибуты для файлов-деревях
};

struct CopyMoveOptions
{
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
	int UseVk_oem_x;
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
	int DeleteToRecycleBin;         // удалять в корзину?
	int DeleteToRecycleBinKillLink; // перед удалением папки в корзину кильнем вложенные симлинки.
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
	int AutoSaveSetup;
	int SetupArgv; // количество каталогов в комюстроке ФАРа
	int ChangeDriveMode;
	int ChangeDriveDisconnetMode;

	int SaveHistory;
	int HistoryCount;
	int SaveFoldersHistory;
	int SavePluginFoldersHistory;
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
	int FolderDeepScan;
	int FormatNumberSeparators;
	int CleanAscii;
	int NoGraphics;
	char FolderInfoFiles[1024];

	struct Confirmation Confirm;
	struct DizOptions Diz;

	int ShellRightLeftArrowsRule;
	struct PanelOptions LeftPanel;
	struct PanelOptions RightPanel;

	DWORD  AutoUpdateLimit; // выше этого количество автоматически не обновлять панели.
	int AutoUpdateRemoteDrive;

	char Language[LANGUAGENAME_SIZE];
	int SmallIcon;
	char RegRoot[NM];
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
	DWORD DisableMacro; // параметры /m или /ma или /m....

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
	// горизонтальная прокрутка
	int MsHWheelDelta;
	int MsHWheelDeltaView;
	int MsHWheelDeltaEdit;
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
	int ExecuteFullTitle;
	char ExecuteBatchType[256];

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

enum enumHighlightDataColor
{
	HIGHLIGHTCOLOR_NORMAL = 0,
	HIGHLIGHTCOLOR_SELECTED,
	HIGHLIGHTCOLOR_UNDERCURSOR,
	HIGHLIGHTCOLOR_SELECTEDUNDERCURSOR,

	HIGHLIGHTCOLORTYPE_FILE = 0,
	HIGHLIGHTCOLORTYPE_MARKCHAR = 1,
};

struct HighlightDataColor
{
	WORD Color[2][4];  // [0=file, 1=mark][0=normal,1=selected,2=undercursor,3=selectedundercursor]; if HIBYTE == 0xFF then transparent
	WORD MarkChar;
};

#endif // __FARSTRUCT_HPP__
