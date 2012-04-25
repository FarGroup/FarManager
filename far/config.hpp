#pragma once

/*
config.hpp

Конфигурация
*/
/*
Copyright © 1996 Eugene Roshal
Copyright © 2000 Far Group
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "panelctype.hpp"

//  +CASR_* Поведение Ctrl-Alt-Shift для AllCtrlAltShiftRule
enum
{
	CASR_PANEL  = 0x0001,
	CASR_EDITOR = 0x0002,
	CASR_VIEWER = 0x0004,
	CASR_HELP   = 0x0008,
	CASR_DIALOG = 0x0010,
};

enum ExcludeCmdHistoryType
{
	EXCLUDECMDHISTORY_NOTWINASS    = 0x00000001,  // не помещать в историю команды ассоциаций Windows
	EXCLUDECMDHISTORY_NOTFARASS    = 0x00000002,  // не помещать в историю команды выполнения ассоциаций файлов
	EXCLUDECMDHISTORY_NOTPANEL     = 0x00000004,  // не помещать в историю команды выполнения с панели
	EXCLUDECMDHISTORY_NOTCMDLINE   = 0x00000008,  // не помещать в историю команды выполнения с ком.строки
	//EXCLUDECMDHISTORY_NOTAPPLYCMD   = 0x00000010,  // не помещать в историю команды выполнения из "Apply Commang"
};

// для Opt.QuotedName
enum QUOTEDNAMETYPE
{
	QUOTEDNAME_INSERT         = 0x00000001,            // кавычить при сбросе в командную строку, в диалогах и редакторе
	QUOTEDNAME_CLIPBOARD      = 0x00000002,            // кавычить при помещении в буфер обмена
};

//Для Opt.Dialogs.MouseButton
#define DMOUSEBUTTON_LEFT   0x00000001
#define DMOUSEBUTTON_RIGHT  0x00000002

//Для Opt.VMenu.xBtnClick
#define VMENUCLICK_IGNORE 0
#define VMENUCLICK_CANCEL 1
#define VMENUCLICK_APPLY  2

//Для Opt.Diz.UpdateMode
enum DIZUPDATETYPE
{
	DIZ_NOT_UPDATE,
	DIZ_UPDATE_IF_DISPLAYED,
	DIZ_UPDATE_ALWAYS
};

enum FarPoliciesFlags
{
	FFPOL_MAINMENUSYSTEM        = 0x00000001,
	FFPOL_MAINMENUPANEL         = 0x00000002,
	FFPOL_MAINMENUINTERFACE     = 0x00000004,
	FFPOL_MAINMENULANGUAGE      = 0x00000008,
	FFPOL_MAINMENUPLUGINS       = 0x00000010,
	FFPOL_MAINMENUDIALOGS       = 0x00000020,
	FFPOL_MAINMENUCONFIRMATIONS = 0x00000040,
	FFPOL_MAINMENUPANELMODE     = 0x00000080,
	FFPOL_MAINMENUFILEDESCR     = 0x00000100,
	FFPOL_MAINMENUFOLDERDESCR   = 0x00000200,
	FFPOL_MAINMENUVIEWER        = 0x00000800,
	FFPOL_MAINMENUEDITOR        = 0x00001000,
	FFPOL_MAINMENUCOLORS        = 0x00004000,
	FFPOL_MAINMENUHILIGHT       = 0x00008000,
	FFPOL_MAINMENUSAVEPARAMS    = 0x00020000,

	FFPOL_CREATEMACRO           = 0x00040000,
	FFPOL_USEPSWITCH            = 0x00080000,
	FFPOL_PERSONALPATH          = 0x00100000,
	FFPOL_KILLTASK              = 0x00200000,
	FFPOL_SHOWHIDDENDRIVES      = 0x80000000,
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
	int CaseSensitiveSort;
	int DirectoriesFirst;
};

struct AutoCompleteOptions
{
	int ShowList;
	int ModalList;
	int AppendCompletion;
};


struct PluginConfirmation
{
	int OpenFilePlugin;
	int StandardAssociation;
	int EvenIfOnlyOnePlugin;
	int SetFindList;
	int Prefix;
};

struct Confirmation
{
	int Copy;
	int Move;
	int RO;
	int Drag;
	int Delete;
	int DeleteFolder;
	int Exit;
	int Esc;  // Для CheckForEsc
	/* $ 12.03.2002 VVM
	  + Opt.EscTwiceToInterrupt
	    Определяет поведение при прерывании длительной операции
	    0 - второй ESC продолжает операцию
	    1 - второй ESC прерывает операцию */
	int EscTwiceToInterrupt;
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
	int DetachVHD;
};

struct DizOptions
{
	string strListNames;
	int ROUpdate;
	int UpdateMode;
	int SetHidden;
	int StartPos;
	int AnsiByDefault;
	int SaveInUTF;
};

struct CodeXLAT
{
	HKL Layouts[10];
	string Rules[3]; // правила:
	// [0] "если предыдущий символ латинский"
	// [1] "если предыдущий символ нелатинский символ"
	// [2] "если предыдущий символ не рус/lat"
	string Table[2]; // [0] non-english буквы, [1] english буквы
	string strWordDivForXlat;
	DWORD Flags;       // дополнительные флаги
	int CurrentLayout;
};

struct EditorOptions
{
	int TabSize;
	int ExpandTabs;
	int PersistentBlocks;
	int DelRemovesBlocks;
	int AutoIndent;
	int AutoDetectCodePage;
	int AnsiCodePageForNewFile;
	int AnsiCodePageAsDefault;
	int CursorBeyondEOL;
	int BSLikeDel;
	int CharCodeBase;
	int SavePos;
	int SaveShortPos;
	int F7Rules; // $ 28.11.2000 SVS - Правило на счет поиска в редакторе
	int AllowEmptySpaceAfterEof; // $ 21.06.2005 SKV - разрешить показывать пустое пространство после последней строки редактируемого файла.
	int ReadOnlyLock; // $ 29.11.2000 SVS - лочить файл при открытии в редакторе, если он имеет атрибуты R|S|H
	int UndoSize; // $ 03.12.2001 IS - размер буфера undo в редакторе
	int UseExternalEditor;
	DWORD FileSizeLimitLo;
	DWORD FileSizeLimitHi;
	int ShowKeyBar;
	int ShowTitleBar;
	int ShowScrollBar;
	int EditOpenedForWrite;
	int SearchSelFound;
	int SearchRegexp;
	int SearchPickUpWord;
	int ShowWhiteSpace;

	string strWordDiv;

	void Clear()
	{
		TabSize = 0;
		ExpandTabs = 0;
		PersistentBlocks = 0;
		DelRemovesBlocks = 0;
		AutoIndent = 0;
		AutoDetectCodePage = 0;
		AnsiCodePageForNewFile = 0;
		AnsiCodePageAsDefault = 0;
		CursorBeyondEOL = 0;
		BSLikeDel = 0;
		CharCodeBase = 0;
		SavePos = 0;
		SaveShortPos = 0;
		F7Rules = 0;
		AllowEmptySpaceAfterEof = 0;
		ReadOnlyLock = 0;
		UndoSize = 0;
		UseExternalEditor = 0;
		ShowKeyBar = 0;
		ShowTitleBar = 0;
		ShowScrollBar=0;
		EditOpenedForWrite=0;
		SearchSelFound=0;
		SearchRegexp=0;
		SearchPickUpWord=0;
		ShowWhiteSpace=0;
		FileSizeLimitLo = 0;
		FileSizeLimitHi = 0;
		strWordDiv.Clear();
	}

	void CopyTo(EditorOptions &dest)
	{
		dest.TabSize = TabSize;
		dest.ExpandTabs = ExpandTabs;
		dest.PersistentBlocks = PersistentBlocks;
		dest.DelRemovesBlocks = DelRemovesBlocks;
		dest.AutoIndent = AutoIndent;
		dest.AutoDetectCodePage = AutoDetectCodePage;
		dest.AnsiCodePageForNewFile = AnsiCodePageForNewFile;
		dest.AnsiCodePageAsDefault = AnsiCodePageAsDefault;
		dest.CursorBeyondEOL = CursorBeyondEOL;
		dest.BSLikeDel = BSLikeDel;
		dest.CharCodeBase = CharCodeBase;
		dest.SavePos = SavePos;
		dest.SaveShortPos = SaveShortPos;
		dest.F7Rules = F7Rules;
		dest.AllowEmptySpaceAfterEof = AllowEmptySpaceAfterEof;
		dest.ReadOnlyLock = ReadOnlyLock;
		dest.UndoSize = UndoSize;
		dest.UseExternalEditor = UseExternalEditor;
		dest.ShowKeyBar = ShowKeyBar;
		dest.ShowTitleBar = ShowTitleBar;
		dest.ShowScrollBar=ShowScrollBar;
		dest.EditOpenedForWrite = EditOpenedForWrite;
		dest.SearchSelFound=SearchSelFound;
		dest.SearchRegexp=SearchRegexp;
		dest.SearchPickUpWord=SearchPickUpWord;
		dest.ShowWhiteSpace=ShowWhiteSpace;
		dest.FileSizeLimitLo = FileSizeLimitLo;
		dest.FileSizeLimitHi = FileSizeLimitHi;
		dest.strWordDiv = strWordDiv;
	}
};

/* $ 29.03.2001 IS
     Тут следует хранить "локальные" настройки для программы просмотра
*/
struct ViewerOptions
{
	enum EViewerLineSize
	{
		eMinLineSize = 100,
		eDefLineSize = 10*1000,
		eMaxLineSize = 100*1000
	};

	int TabSize;
	int AutoDetectCodePage;
	int ShowScrollbar;     // $ 18.07.2000 tran пара настроек для viewer
	int ShowArrows;
	int PersistentBlocks; // $ 14.05.2002 VVM Постоянные блоки во вьюере
	int ViewerIsWrap; // (Wrap|WordWarp)=1 | UnWrap=0
	int ViewerWrap; // Wrap=0|WordWarp=1
	int SavePos;
	int SaveShortPos;
	int UseExternalViewer;
	int ShowKeyBar; // $ 15.07.2000 tran + ShowKeyBar
	int AnsiCodePageAsDefault;
	int ShowTitleBar;
	int SearchRegexp;
	int MaxLineSize; // 100..100000, default=10000
	int SearchEditFocus; // auto-focus on edit text/hex window
	int Visible0x00;
	int ZeroChar;
};

// "Полиция"
struct PoliciesOptions
{
	int DisabledOptions;  // разрешенность меню конфигурации
	int ShowHiddenDrives; // показывать скрытые логические диски
};

struct DialogsOptions
{
	int   EditBlock;            // Постоянные блоки в строках ввода
	int   EditHistory;          // Добавлять в историю?
	int   AutoComplete;         // Разрешено автодополнение?
	int   EULBsClear;           // = 1 - BS в диалогах для UnChanged строки удаляет такую строку также, как и Del
	int   SelectFromHistory;    // = 0 then (ctrl-down в строке с историей курсор устанавливался на самую верхнюю строку)
	DWORD EditLine;             // общая информация о строке ввода (сейчас это пока... позволяет управлять выделением)
	int   MouseButton;          // Отключение восприятие правой/левой кнопки мышы как команд закрытия окна диалога
	int   DelRemovesBlocks;
	int   CBoxMaxHeight;        // максимальный размер открываемого списка (по умолчанию=8)
};

struct VMenuOptions
{
	int   LBtnClick;
	int   RBtnClick;
	int   MBtnClick;
};

struct CommandLineOptions
{
	int EditBlock;
	int DelRemovesBlocks;
	int AutoComplete;
	int UsePromptFormat;
	string strPromptFormat;
};

struct NowellOptions
{
	int MoveRO;               // перед операцией Move снимать R/S/H атрибуты, после переноса - выставлять обратно
};

struct ScreenSizes
{
	COORD DeltaXY;            // на сколько поз. изменить размеры для распахнутого экрана
	int WScreenSizeSet;
	COORD WScreenSize[4];
};

struct LoadPluginsOptions
{
	string strCustomPluginsPath;  // путь для поиска плагинов, указанный в /p
	string strPersonalPluginsPath;
//  DWORD TypeLoadPlugins;       // see TYPELOADPLUGINSOPTIONS
	int MainPluginDir; // TRUE - использовать стандартный путь к основным плагинам
	int PluginsCacheOnly; // seting by '/co' switch, not saved in registry
	int PluginsPersonal;

	int SilentLoadPlugin; // при загрузке плагина с кривым...
#ifndef NO_WRAPPER
	int OEMPluginsSupport;
#endif // NO_WRAPPER
	int ScanSymlinks;
};

struct FindFileOptions
{
	int FileSearchMode;
	bool FindFolders;
	bool FindSymLinks;
	bool CollectFiles;
	bool UseFilter;
	bool FindAlternateStreams;
	string strSearchInFirstSize;

	string strSearchOutFormat;
	string strSearchOutFormatWidth;
	int OutColumnCount;
	unsigned __int64 OutColumnTypes[PANEL_COLUMNCOUNT];
	int OutColumnWidths[PANEL_COLUMNCOUNT];
	int OutColumnWidthType[PANEL_COLUMNCOUNT];
};

struct InfoPanelOptions
{
	COMPUTER_NAME_FORMAT ComputerNameFormat;
	EXTENDED_NAME_FORMAT UserNameFormat;
	int ShowPowerStatus;
	string strShowStatusInfo;
	string strFolderInfoFiles;
};

struct TreeOptions
{
	int MinTreeCount;         // Минимальное количество папок для сохранения дерева в файле.
	int AutoChangeFolder;     // Автосмена папок при перемещении по дереву
	DWORD TreeFileAttr;       // Файловые атрибуты для файлов-деревях

#if defined(TREEFILE_PROJECT)
	int LocalDisk;            // Хранить файл структуры папок для локальных дисков
	int NetDisk;              // Хранить файл структуры папок для сетевых дисков
	int NetPath;              // Хранить файл структуры папок для сетевых путей
	int RemovableDisk;        // Хранить файл структуры папок для сменных дисков
	int CDDisk;               // Хранить файл структуры папок для CD/DVD/BD/etc дисков

	string strLocalDisk;      // шаблон имени файла-деревяхи для локальных дисков
	string strNetDisk;        // шаблон имени файла-деревяхи для сетевых дисков
	string strNetPath;        // шаблон имени файла-деревяхи для сетевых путей
	string strRemovableDisk;  // шаблон имени файла-деревяхи для сменных дисков
	string strCDDisk;         // шаблон имени файла-деревяхи для CD/DVD/BD/etc дисков

	string strExceptPath;     // для перечисленных здесь не хранить

	string strSaveLocalPath;  // сюда сохраняем локальные диски
	string strSaveNetPath;    // сюда сохраняем сетевые диски
#endif
};

struct CopyMoveOptions
{
	int UseSystemCopy;         // использовать системную функцию копирования
	int CopyOpened;            // копировать открытые на запись файлы
	int CopyShowTotal;         // показать общий индикатор копирования
	int MultiCopy;             // "разрешить мультикопирование/перемещение/создание связей"
	DWORD CopySecurityOptions; // для операции Move - что делать с опцией "Copy access rights"
	int CopyTimeRule;          // $ 30.01.2001 VVM  Показывает время копирования,оставшееся время и среднюю скорость
	size_t BufferSize;
};

struct DeleteOptions
{
	int DelShowTotal;         // показать общий индикатор удаления
};

struct MacroOptions
{
	int MacroReuseRules; // Правило на счет повторно использования забинденных клавиш
	DWORD DisableMacro; // параметры /m или /ma или /m....
	DWORD KeyMacroCtrlDot, KeyMacroRCtrlDot; // аля KEY_CTRLDOT/KEY_RCTRLDOT
	DWORD KeyMacroCtrlShiftDot, KeyMacroRCtrlShiftDot; // аля KEY_CTRLSHIFTDOT/KEY_RCTRLSHIFTDOT
	string strMacroCONVFMT; // формат преобразования double в строку
	string strDateFormat; // Для $Date
};

struct KnownModulesIDs
{
	GUID Network;
	GUID Emenu;
};

struct ExecuteOptions
{
	int RestoreCPAfterExecute;
	int ExecuteUseAppPath;
	int ExecuteFullTitle;
	int ExecuteSilentExternal;
	string strExecuteBatchType;
	string strExcludeCmds;
	string strHomeDir; // cd ~
};

struct Options
{
	palette Palette;
	int Clock;
	int Mouse;
	int ShowKeyBar;
	int ScreenSaver;
	int ScreenSaverTime;
	int UseVk_oem_x;
	int ShowHidden;
	int Highlight;

	string strLeftFolder;
	string strRightFolder;

	string strLeftCurFile;
	string strRightCurFile;

	int RightSelectedFirst;
	int LeftSelectedFirst;
	int SelectFolders;
	int ReverseSort;
	int SortFolderExt;
	int DeleteToRecycleBin;         // удалять в корзину?
	int DeleteToRecycleBinKillLink; // перед удалением папки в корзину кильнем вложенные симлинки.
	int WipeSymbol; // символ заполнитель для "ZAP-операции"

	CopyMoveOptions CMOpt;

	DeleteOptions DelOpt;

	int MultiMakeDir; // Опция создания нескольких каталогов за один сеанс

	int CreateUppercaseFolders;
	int UseRegisteredTypes;

	int ViewerEditorClock;
	int OnlyEditorViewerUsed; // =1, если старт был /e или /v
	int SaveViewHistory;
	int ViewHistoryCount;

	string strExternalEditor;
	EditorOptions EdOpt;
	string strExternalViewer;
	ViewerOptions ViOpt;


	string strWordDiv; // $ 03.08.2000 SVS Разграничитель слов из реестра
	string strQuotedSymbols;
	DWORD QuotedName;
	int AutoSaveSetup;
	int SetupArgv; // количество каталогов в комюстроке ФАРа
	int ChangeDriveMode;
	int ChangeDriveDisconnectMode;

	int SaveHistory;
	int HistoryCount;
	int SaveFoldersHistory;
	int FoldersHistoryCount;
	int DialogsHistoryCount;

	FindFileOptions FindOpt;

	int LeftHeightDecrement;
	int RightHeightDecrement;
	int WidthDecrement;

	int ShowColumnTitles;
	int ShowPanelStatus;
	int ShowPanelTotals;
	int ShowPanelFree;
	int PanelDetailedJunction;
	int ShowUnknownReparsePoint;
	int HighlightColumnSeparator;
	int DoubleGlobalColumnSeparator;

	int ShowPanelScrollbar;
	int ShowMenuScrollbar; // $ 29.06.2000 SVS Добавлен атрибут показа Scroll Bar в меню.
	int ShowScreensNumber;
	int ShowSortMode;
	int ShowMenuBar;
	int FormatNumberSeparators;
	int CleanAscii;
	int NoGraphics;

	Confirmation Confirm;
	PluginConfirmation PluginConfirm;

	DizOptions Diz;

	int ShellRightLeftArrowsRule;
	PanelOptions LeftPanel;
	PanelOptions RightPanel;

	AutoCompleteOptions AutoComplete;

	DWORD  AutoUpdateLimit; // выше этого количество автоматически не обновлять панели.
	int AutoUpdateRemoteDrive;

	string strLanguage;
	int SmallIcon;
#ifndef NO_WRAPPER
	string strRegRoot;
#endif // NO_WRAPPER
	int PanelRightClickRule; // задает поведение правой клавиши мыши
	int PanelCtrlAltShiftRule; // задает поведение Ctrl-Alt-Shift для панелей.
	// Panel/CtrlFRule в реестре - задает поведение Ctrl-F. Если = 0, то штампуется файл как есть, иначе - с учетом отображения на панели
	int PanelCtrlFRule;
	/*
	  битовые флаги, задают поведение Ctrl-Alt-Shift
	   бит установлен - функция включена:
	   0 - Panel
	   1 - Edit
	   2 - View
	   3 - Help
	   4 - Dialog
	*/
	int AllCtrlAltShiftRule;

	int CASRule; // 18.12.2003 - Пробуем различать левый и правый CAS (попытка #1).
	/*
	  задает поведение Esc для командной строки:
	    =1 - Не изменять положение в History, если после Ctrl-E/Ctrl/-X
	         нажали ESC (поведение - аля VC).
	    =0 - поведение как и было - изменять положение в History
	*/
	int CmdHistoryRule;

	DWORD ExcludeCmdHistory;
	int SubstPluginPrefix; // 1 = подстанавливать префикс плагина (для Ctrl-[ и ему подобные)
	int MaxPositionCache; // количество позиций в кэше сохранения
	int SetAttrFolderRules; // Правило на счет установки атрибутов на каталоги
	int ExceptRules; // Правило на счет вызова исключений
	int ExceptCallDebugger; // вызывать дебаггер при исключении
	int ExceptUsed;
	string strExceptEventSvc;
	/*
	 + Opt.ShiftsKeyRules - Правило на счет выбора механизма трансляции
	   Alt-Буква для нелатинским буковок и символов "`-=[]\;',./" с
	   модификаторами Alt-, Ctrl-, Alt-Shift-, Ctrl-Shift-, Ctrl-Alt-
	*/
	int ShiftsKeyRules;
	int CursorSize[4];   // Размер курсора ФАРа

	CodeXLAT XLat;

	int ConsoleDetachKey; // Комбинация клавиш для детача Far'овской консоли от длятельного неинтерактивного процесса в ней запущенного.

	string strHelpLanguage;
	int FullScreenHelp;
	int HelpTabSize;

	int HelpURLRules; // =0 отключить возможность запуска URL-приложений

	// запоминать логические диски и не опрашивать каждый раз. Для предотвращения "просыпания" "зеленых" винтов.
	int RememberLogicalDrives;
	/*
	  будет влиять на:
	      добавление файлов в историю с разным регистром
	      добавление LastPositions в редакторе и вьюере
	*/
	int FlagPosixSemantics;

	int MsWheelDelta; // задает смещение для прокрутки
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

	/* $ 23.05.2001 AltF9
	  + Opt.AltF9 Флаг позволяет выбрать механизм  работы комбинации Alt-F9
	       (Изменение размера экрана) в оконном режиме. По умолчанию - 1.
	    0 - использовать механизм, совместимый с FAR версии 1.70 beta 3 и
	       ниже, т.е. переключение 25/50 линий.
	    1 - использовать усовершенствованный механизм - окно FAR Manager
	       будет переключаться с нормального на максимально доступный размер
	       консольного окна и обратно.*/
	int AltF9;

	int ClearType;

	int PgUpChangeDisk;
	int ShowDotsInRoot;
	int ShowCheckingFile;
	int CloseCDGate;       // автомонтирование CD
	int UpdateEnvironment;

	ExecuteOptions Exec;

	DWORD PluginMaxReadData;
	int ScanJunction;

	DWORD ShowTimeoutDelFiles; // тайаут в процессе удаления (в ms)
	DWORD ShowTimeoutDACLFiles;
	int DelThreadPriority; // приоритет процесса удаления, по умолчанию = THREAD_PRIORITY_NORMAL

	LoadPluginsOptions LoadPlug;

	DialogsOptions Dialogs;
	VMenuOptions VMenu;
	CommandLineOptions CmdLine;
	PoliciesOptions Policies;
	NowellOptions Nowell;
	ScreenSizes ScrSize;
	MacroOptions Macro;

	int FindCodePage;

	TreeOptions Tree;
	InfoPanelOptions InfoPanel;

	DWORD CPMenuMode;
	string strNoAutoDetectCP;
	// Перечисленные здесь кодовые страницы будут исключены из детектирования nsUniversalDetectorEx.
	// Автодетект юникодных страниц от этого не зависит, поэтому UTF-8 будет определяться даже если
	// 65001 здесь присутствует. Если UniversalDetector выдаст страницу из этого списка, она будет
	// заменена на умолчательную ANSI или OEM, в зависимости от настроек.
	// пример: L"1250,1252,1253,1255,855,10005,28592,28595,28597,28598,38598,65001"
	// Если строка пустая никакой фильтрации кодовых страниц в UCD детекте не будет.
	// Если "-1", то в зависимости CPMenuMode (Ctrl-H в меню кодовых страниц фильтрация UCD либо будет
	// отключена, либо будут разрешенны только 'любимые' и системные (OEM ANSI) кодовые страницы.

	bool IsUserAdmin;
	string strTitleAddons;

	int ElevationMode;
	int CurrentElevationMode;

	BOOL WindowMode;

	string ProfilePath;
	string LocalProfilePath;
	string GlobalUserMenuDir;
	KnownModulesIDs KnownIDs;

	string strBoxSymbols;

};

extern Options Opt;

void SystemSettings();
void PanelSettings();
void InterfaceSettings();
void DialogSettings();
void VMenuSettings();
void CmdlineSettings();
void SetConfirmations();
void PluginsManagerSettings();
void SetDizConfig();
void ViewerConfig(ViewerOptions &ViOpt,bool Local=false);
void EditorConfig(EditorOptions &EdOpt,bool Local=false);
void ReadConfig();
void SaveConfig(int Ask);
void SetFolderInfoFiles();
void InfoPanelSettings();
void MaskGroupsSettings();
void AutoCompleteSettings();
void TreeSettings();

bool GetConfigValue(const wchar_t *Key, const wchar_t *Name, string &Value);
bool GetConfigValue(size_t Root,const wchar_t* Name,DWORD& Type,void*& Data);

bool AdvancedConfig();
