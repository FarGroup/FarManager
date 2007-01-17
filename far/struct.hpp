#ifndef __FARSTRUCT_HPP__
#define __FARSTRUCT_HPP__
/*
struct.hpp

все независимые структуры (которые содержат только простые типы)

*/

#include "farconst.hpp"
#include "UnicodeString.hpp"

struct FilterParams
{
  struct __FMask
  {
    DWORD Used;
    string strMask;
  } FMask;
  struct __FDate
  {
    DWORD Used;
    FDateType DateType;
    FILETIME DateAfter;
    FILETIME DateBefore;
  } FDate;
  struct __FSize
  {
    DWORD Used;
    FSizeType SizeType;
    __int64 SizeAbove;
    __int64 SizeBelow;
  } FSize;
  struct __FAttr
  {
    DWORD Used;
    DWORD AttrSet;
    DWORD AttrClear;
  } FAttr;

  /*
  void Clear()
  {
  	FMask.Used=0;
  	FMask.strMask=L"";
  	memset(&FDate,0,sizeof(FDate));
  	memset(&FSize,0,sizeof(FSize));
  	memset(&FAttr,0,sizeof(FAttr));
  }
  */

  FilterParams& operator=(const FilterParams &fpCopy)
  {
  	FMask.Used=fpCopy.FMask.Used;
  	FMask.strMask=fpCopy.FMask.strMask;
  	memcpy(&FDate,&fpCopy.FDate,sizeof(FDate));
  	memcpy(&FSize,&fpCopy.FSize,sizeof(FSize));
  	memcpy(&FAttr,&fpCopy.FAttr,sizeof(FAttr));
  	return *this;
  }

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
  string strListNames;
  int ROUpdate;
  int UpdateMode;
  int SetHidden;
  int StartPos;

  /*
  void Clear()
  {
    strListNames=L"";
    ROUpdate=0;
    UpdateMode=0;
    SetHidden=0;
    StartPos=0;
  }
  */
};

/* $ 05.09.2000 SVS
   Структура CodeQWERTY, описывающая QWERTY-перекодировщик
*/
struct CodeXLAT
{
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
  string strWordDivForXlat;
  /* IS $ */
  // первый байт - размер таблицы
  BYTE Table[2][81]; // [0] non-english буквы, [1] english буквы
  BYTE Rules[3][81]; // 3 по 40 правил:
                    //  [0] "если предыдущий символ латинский"
                    //  [1] "если предыдущий символ нелатинский символ"
                    //  [2] "если предыдущий символ не рус/lat"

  /*
  void Clear()
  {
    Flags=0;
    XLatEditorKey=0;
    XLatCmdLineKey=0;
    XLatDialogKey=0;
    XLatFastFindKey=0;
    XLatAltEditorKey=0;
    XLatAltCmdLineKey=0;
    XLatAltDialogKey=0;
    XLatAltFastFindKey=0;
    strWordDivForXlat=L"";
    memset(Table,0,sizeof(Table));
    memset(Rules,0,sizeof(Rules));
  }
  */
};
/* SVS $*/

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
	int ReadOnlyLock; // $ 29.11.2000 SVS - лочить файл при открытии в редакторе, если он имеет атрибуты R|S|H
	int UndoSize; // $ 03.12.2001 IS - размер буфера undo в редакторе
	int UseExternalEditor;
	DWORD FileSizeLimitLo;
	DWORD FileSizeLimitHi;
	int ShowKeyBar;

	string strWordDiv;

	void Clear ()
	{
		TabSize = 0;
		ExpandTabs = 0;
		PersistentBlocks = 0;
		DelRemovesBlocks = 0;
		AutoIndent = 0;
		AutoDetectTable = 0;
		AnsiTableForNewFile = 0;
		AnsiTableAsDefault = 0;
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

		FileSizeLimitLo = 0;
		FileSizeLimitHi = 0;

		strWordDiv = L"";
	}

	void CopyTo (EditorOptions &dest)
	{
		dest.TabSize = TabSize;
		dest.ExpandTabs = ExpandTabs;
		dest.PersistentBlocks = PersistentBlocks;
		dest.DelRemovesBlocks = DelRemovesBlocks;
		dest.AutoIndent = AutoIndent;
		dest.AutoDetectTable = AutoDetectTable;
		dest.AnsiTableForNewFile = AnsiTableForNewFile;
		dest.AnsiTableAsDefault = AnsiTableAsDefault;
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
		dest.strWordDiv = strWordDiv;

		dest.FileSizeLimitLo = FileSizeLimitLo;
		dest.FileSizeLimitHi = FileSizeLimitHi;
	}
};

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

  string strCustomPluginsPath;  // путь для поиска плагинов, указанный в /p
  string strPersonalPluginsPath;
  /* SVS $*/
  int SilentLoadPlugin; // при загрузке плагина с кривым...

  /*
  void Clear()
  {
    MainPluginDir=0;
    PluginsCacheOnly=0;
    PluginsPersonal=0;

    strCustomPluginsPath=L"";
    strPersonalPluginsPath=L"";
    SilentLoadPlugin=0;
  }
  */
};

struct FindFileOptions
{
  int FindFolders;
  int CollectFiles;
  int FileSearchMode;
  int SearchInFirst;
  string strSearchInFirstSize;

  /*
  void Clear()
  {
    FindFolders=0;
    CollectFiles=0;
    FileSearchMode=0;
    SearchInFirst=0;
    strSearchInFirstSize=L"";
  }
  */
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
  string strPromptFormat;
  int AltGr;
  int UseVk_oem_x;
  int InactivityExit;
  int InactivityExitTime;
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

  string strExternalEditor;
  struct EditorOptions EdOpt;
  string strExternalViewer;
  struct ViewerOptions ViOpt;


  string strWordDiv; // $ 03.08.2000 SVS Разграничитель слов из реестра
  string strQuotedSymbols;
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

  string strTempPath;
  int HeightDecrement;
  int WidthDecrement;

  string strPassiveFolder;

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
  string strFolderInfoFiles;

  struct Confirmation Confirm;
  struct DizOptions Diz;
  struct PanelOptions LeftPanel;
  struct PanelOptions RightPanel;

  DWORD  AutoUpdateLimit; // выше этого количество автоматически не обновлять панели.
  int AutoUpdateRemoteDrive;

  string strLanguage;
  int SmallIcon;
  string strRegRoot;
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

  string strHelpLanguage;
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
  int ExecuteFullTitle;
  string strExecuteBatchType;

#if defined(FAR_ANSI)
  int FarAnsi;
#endif
  DWORD PluginMaxReadData;
  int UseNumPad;
  int ScanJunction;

  DWORD ShowTimeoutDelFiles; // тайаут в процессе удаления (в ms)
  DWORD ShowTimeoutDACLFiles;
  int DelThreadPriority; // приоритет процесса удаления, по умолчанию = THREAD_PRIORITY_NORMAL

  //int CPAJHefuayor; // производное от "Close Plugin And Jump:
                  // Highly experimental feature, use at your own risk"

  string strDateFormat; // Для $Date
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

  /*
  void Clear()
  {
    Clock=0;
    Mouse=0;
    ShowKeyBar=0;
    ScreenSaver=0;
    ScreenSaverTime=0;
    UsePromptFormat=0;
    strPromptFormat=L"";
    AltGr=0;
    UseVk_oem_x=0;
    InactivityExit=0;
    InactivityExitTime=0;
    ShowHidden=0;
    Highlight=0;

    strLeftFolder=L"";
    strRightFolder=L"";

    strLeftCurFile=L"";
    strRightCurFile=L"";

    RightSelectedFirst=0;
    LeftSelectedFirst=0;
    SelectFolders=0;
    ReverseSort=0;
    ClearReadOnly=0;
    SortFolderExt=0;
    DeleteToRecycleBin=0;
    WipeSymbol=0;

    memset(&CMOpt,0,sizeof(CMOpt));

    MultiMakeDir=0;
    CreateUppercaseFolders=0;
    UseRegisteredTypes=0;

    ViewerEditorClock=0;
    OnlyEditorViewerUsed=0;
    SaveViewHistory=0;
    ViewHistoryCount=0;

    strExternalEditor=L"";
    EdOpt.Clear();
    strExternalViewer=L"";
    memset(&ViOpt,0,sizeof(ViOpt));


    strWordDiv=L"";
    strQuotedSymbols=L"";
    QuotedName=0;
    AutoSaveSetup=0;
    SetupArgv=0;
    ChangeDriveMode=0;
    ChangeDriveDisconnetMode=0;

    SaveHistory=0;
    HistoryCount=0;
    SaveFoldersHistory=0;
    SavePluginFoldersHistory=0;
    FoldersHistoryCount=0;
    DialogsHistoryCount=0;

    FindOpt.Clear();

    strTempPath=L"";
    HeightDecrement=0;
    WidthDecrement=0;

    strPassiveFolder=L"";

    ShowColumnTitles=0;
    ShowPanelStatus=0;
    ShowPanelTotals=0;
    ShowPanelFree=0;
    ShowPanelScrollbar=0;
    ShowMenuScrollbar=0;
    ShowScreensNumber=0;
    ShowSortMode=0;
    ShowMenuBar=0;

    CleanAscii=0;
    NoGraphics=0;
    strFolderInfoFiles=L"";

    memset(&Confirm,0,sizeof(Confirm));
    Diz.Clear();
    memset(&LeftPanel,0,sizeof(LeftPanel));
    memset(&RightPanel,0,sizeof(RightPanel));

    AutoUpdateLimit=0;
    AutoUpdateRemoteDrive=0;

    strLanguage=L"";
    SmallIcon=0;
    strRegRoot=L"";
    PanelRightClickRule=0;
    PanelMiddleClickRule=0;
    PanelCtrlAltShiftRule=0;
    PanelCtrlFRule=0;
    AllCtrlAltShiftRule=0;
    CASRule=0;
    CmdHistoryRule=0;
    ExcludeCmdHistory=0;
    SubstPluginPrefix=0;
    MaxPositionCache=0;
    SetAttrFolderRules=0;
    ExceptRules=0;
    ExceptCallDebugger=0;
    HotkeyRules=0;
    ShiftsKeyRules=0;
    MacroReuseRules=0;
    IgnoreErrorBadPathName=0;

    KeyMacroCtrlDot=0;
    KeyMacroCtrlShiftDot=0;
    memset(CursorSize,0,sizeof(CursorSize));
    XLat.Clear();
    ConsoleDetachKey=0;

    UsePrintManager=0;

    strHelpLanguage=L"";
    FullScreenHelp=0;
    HelpTabSize=0;
    HelpURLRules=0;

    RememberLogicalDrives=0;
    FlagPosixSemantics=0;
    MsWheelDelta=0;
    MsWheelDeltaView=0;
    MsWheelDeltaEdit=0;
    MsWheelDeltaHelp=0;
    SubstNameRule=0;

    AltF9=0;
    PgUpChangeDisk=0;
    ShowCheckingFile=0;
    CloseConsoleRule=0;
    CloseCDGate=0;

    LCIDSort=0;
    RestoreCPAfterExecute=0;
    ExecuteShowErrorMessage=0;
    ExecuteUseAppPath=0;
    ExecuteFullTitle=0;
    strExecuteBatchType=L"";

  #if defined(FAR_ANSI)
    FarAnsi=0;
  #endif
    PluginMaxReadData=0;
    UseNumPad=0;
    ScanJunction=0;

    ShowTimeoutDelFiles=0;
    ShowTimeoutDACLFiles=0;
    DelThreadPriority=0;

    strDateFormat=L"";
    LoadPlug.Clear();

    memset(&Dialogs,0,sizeof(Dialogs));
    memset(&Policies,0,sizeof(Policies));
    memset(&Nowell,0,sizeof(Nowell));
    memset(&ScrSize,0,sizeof(ScrSize));
    memset(&CharTable,0,sizeof(CharTable));
    OpFilter.Clear();
    memset(&Tree,0,sizeof(Tree));
  }
  */
};

struct PluginHandle
{
  HANDLE hPlugin;
  class Plugin *pPlugin;
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
  wchar_t MarkChar;
  BYTE Reserved[10];
};

struct PreRedrawParamStruct
{
  DWORD Flags;
  void *Param1;
  const void *Param2;
  const void *Param3;
  void *Param4;
  __int64 Param5;
};

struct FAR_FIND_DATA_EX
{
	DWORD    dwFileAttributes;
	FILETIME ftCreationTime;
	FILETIME ftLastAccessTime;
	FILETIME ftLastWriteTime;
	unsigned __int64 nFileSize;

	union {
		unsigned __int64 nPackSize; //same as reserved
		struct {
			DWORD dwReserved0;
			DWORD dwReserved1;
		};
	};

	string   strFileName;
	string   strAlternateFileName;

	void Clear()
	{
		dwFileAttributes=0;
		memset(&ftCreationTime,0,sizeof(ftCreationTime));
		memset(&ftLastAccessTime,0,sizeof(ftLastAccessTime));
		memset(&ftLastWriteTime,0,sizeof(ftLastWriteTime));
		nFileSize=_ui64(0);
		nPackSize=_ui64(0);
		strFileName=L"";
		strAlternateFileName=L"";
	}

	FAR_FIND_DATA_EX& operator=(const FAR_FIND_DATA_EX &ffdexCopy)
	{
		dwFileAttributes=ffdexCopy.dwFileAttributes;
		memcpy(&ftCreationTime,&ffdexCopy.ftCreationTime,sizeof(ftCreationTime));
		memcpy(&ftLastAccessTime,&ffdexCopy.ftLastAccessTime,sizeof(ftLastAccessTime));
		memcpy(&ftLastWriteTime,&ffdexCopy.ftLastWriteTime,sizeof(ftLastWriteTime));
		nFileSize=ffdexCopy.nFileSize;
		nPackSize=ffdexCopy.nPackSize;
		strFileName=ffdexCopy.strFileName;
		strAlternateFileName=ffdexCopy.strAlternateFileName;
		return *this;
	}
};


#endif // __FARSTRUCT_HPP__
