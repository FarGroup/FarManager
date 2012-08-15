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
#include "configdb.hpp"

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

class Option
{
public:
	explicit Option(const string& Value):sValue(new string(Value)), ValueChanged(false), IsString(true){}
	explicit Option(const int Value):iValue(Value), ValueChanged(false), IsString(false){}
	virtual ~Option(){if(IsString) delete sValue;}
	bool Changed(){return ValueChanged;}
	virtual bool StoreValue(const wchar_t* KeyName, const wchar_t* ValueName) = 0;
	virtual const string toString() = 0;
protected:
	const string& GetString() const {return *sValue;}
	const int GetInt() const {return iValue;}
	void Set(const string& NewValue) {if(*sValue != NewValue) {*sValue = NewValue; ValueChanged = true;}}
	void Set(const int NewValue) {if(iValue != NewValue) {iValue = NewValue; ValueChanged = true;}}
	virtual bool ReceiveValue(const wchar_t* KeyName, const wchar_t* ValueName, const void* Default) = 0;
private:
	void MakeUnchanged(){ValueChanged = false;}
	union
	{
		string* sValue;
		int iValue;
	};
	bool ValueChanged;
	bool IsString;
	friend void ReadConfig();
};

class BoolOption:public Option
{
public:
	BoolOption():Option(false){}
	BoolOption(const bool& Value):Option(Value){}
	BoolOption& operator=(bool Value){Set(Value); return *this;}
	const bool Get() const {return GetInt() != false;}
	operator bool() const {return GetInt() != false;}
	bool ReceiveValue(const wchar_t* KeyName, const wchar_t* ValueName, bool Default);
	virtual bool StoreValue(const wchar_t* KeyName, const wchar_t* ValueName);
	virtual const string toString(){return Get()? L"true":L"false";}
private:
	virtual bool ReceiveValue(const wchar_t* KeyName, const wchar_t* ValueName, const void* Default) {return ReceiveValue(KeyName, ValueName, reinterpret_cast<intptr_t>(Default) != 0);}

};

class Bool3Option:public Option
{
public:
	Bool3Option():Option(0){}
	Bool3Option(const int& Value):Option(Value % 3){}
	const int Get() const {return GetInt() % 3;}
	Bool3Option& operator=(int Value){Set(Value % 3); return *this;}
	Bool3Option& operator--(){Set((GetInt()+2) % 3); return *this;}
	Bool3Option& operator++(){Set((GetInt()+1) % 3); return *this;}
	Bool3Option operator--(int){int Current = GetInt() % 3; Set((Current+2) % 3); return Current;}
	Bool3Option operator++(int){int Current = GetInt() % 3; Set((Current+1) % 3); return Current;}
	operator int() const {return GetInt() % 3;}
	bool ReceiveValue(const wchar_t* KeyName, const wchar_t* ValueName, int Default);
	virtual bool StoreValue(const wchar_t* KeyName, const wchar_t* ValueName);
	virtual const string toString(){ int v = Get(); return v ? (v == 1 ? L"True" : L"Other") : L"False"; }
private:
	virtual bool ReceiveValue(const wchar_t* KeyName, const wchar_t* ValueName, const void* Default) {return ReceiveValue(KeyName, ValueName, static_cast<int>(reinterpret_cast<intptr_t>(Default)));}
};

class IntOption:public Option
{
public:
	IntOption():Option(0){}
	IntOption(const int& Value):Option(Value){}
	const int Get() const {return GetInt();}
	IntOption& operator=(int Value){Set(Value); return *this;}
	IntOption& operator|=(const int& Value){Set(GetInt()|Value); return *this;}
	IntOption& operator&=(const int& Value){Set(GetInt()&Value); return *this;}
	IntOption& operator%=(const int& Value){Set(GetInt()%Value); return *this;}
	IntOption& operator^=(const int& Value){Set(GetInt()^Value); return *this;}
	IntOption& operator--(){Set(GetInt()-1); return *this;}
	IntOption& operator++(){Set(GetInt()+1); return *this;}
	IntOption operator--(int){int Current = GetInt(); Set(Current-1); return Current;}
	IntOption operator++(int){int Current = GetInt(); Set(Current+1); return Current;}
	operator int() const {return GetInt();}
	bool ReceiveValue(const wchar_t* KeyName, const wchar_t* ValueName, int Default);
	virtual bool StoreValue(const wchar_t* KeyName, const wchar_t* ValueName);
	virtual const string toString(){FormatString s; s << Get(); return s;}
private:
	virtual bool ReceiveValue(const wchar_t* KeyName, const wchar_t* ValueName, const void* Default) {return ReceiveValue(KeyName, ValueName, static_cast<int>(reinterpret_cast<intptr_t>(Default)));}
};

class StringOption:public Option
{
public:
	StringOption():Option(L""){}
	StringOption(const StringOption& Value):Option(Value){}
	StringOption(const string& Value):Option(Value){}
	const string& Get() const {return GetString();}
	operator const wchar_t *() const {return GetString();}
	operator const string&() const {return GetString();}
	void Clear() {Set(L"");}
	bool IsEmpty() const {return GetString().IsEmpty();}
	size_t GetLength() const {return Get().GetLength();}
	wchar_t At(size_t Pos) const {return Get().At(Pos);}
	StringOption& operator=(const wchar_t* Value) {Set(Value); return *this;}
	StringOption& operator=(const string& Value) {Set(Value); return *this;}
	StringOption& operator=(const StringOption& Value) {Set(Value); return *this;}
	StringOption& operator+=(const string& Value) {Set(Get()+Value); return *this;}
	StringOption& operator+=(wchar_t Value) {Set(Get()+Value); return *this;}
	bool ReceiveValue(const wchar_t* KeyName, const wchar_t* ValueName, const wchar_t* Default);
	virtual bool ReceiveValue(const wchar_t* KeyName, const wchar_t* ValueName, const void* Default) {return ReceiveValue(KeyName, ValueName, static_cast<const wchar_t*>(Default));}
	virtual bool StoreValue(const wchar_t* KeyName, const wchar_t* ValueName);
	virtual const string toString(){return Get();}
};

struct PanelOptions
{
	IntOption Type;
	BoolOption Visible;
	BoolOption Focus;
	IntOption ViewMode;
	IntOption SortMode;
	IntOption SortOrder;
	BoolOption SortGroups;
	BoolOption ShowShortNames;
	BoolOption NumericSort;
	BoolOption CaseSensitiveSort;
	BoolOption DirectoriesFirst;
};

struct AutoCompleteOptions
{
	BoolOption ShowList;
	BoolOption ModalList;
	BoolOption AppendCompletion;

	Bool3Option UseFilesystem;
	Bool3Option UseHistory;
	Bool3Option UsePath;
};


struct PluginConfirmation
{
	Bool3Option OpenFilePlugin;
	BoolOption StandardAssociation;
	BoolOption EvenIfOnlyOnePlugin;
	BoolOption SetFindList;
	BoolOption Prefix;
};

struct Confirmation
{
	BoolOption Copy;
	BoolOption Move;
	BoolOption RO;
	BoolOption Drag;
	BoolOption Delete;
	BoolOption DeleteFolder;
	BoolOption Exit;
	BoolOption Esc;  // Для CheckForEsc
	/* $ 12.03.2002 VVM
	  + Opt.EscTwiceToInterrupt
	    Определяет поведение при прерывании длительной операции
	    0 - второй ESC продолжает операцию
	    1 - второй ESC прерывает операцию */
	BoolOption EscTwiceToInterrupt;
	BoolOption RemoveConnection;
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
	BoolOption AllowReedit;
	BoolOption HistoryClear;
	BoolOption RemoveSUBST;
	BoolOption RemoveHotPlug;
	BoolOption DetachVHD;
};

struct DizOptions
{
	StringOption strListNames;
	BoolOption ROUpdate;
	IntOption UpdateMode;
	BoolOption SetHidden;
	IntOption StartPos;
	BoolOption AnsiByDefault;
	BoolOption SaveInUTF;
};

struct CodeXLAT
{
	HKL Layouts[10];
	StringOption strLayouts;
	StringOption Rules[3]; // правила:
	// [0] "если предыдущий символ латинский"
	// [1] "если предыдущий символ нелатинский символ"
	// [2] "если предыдущий символ не рус/lat"
	StringOption Table[2]; // [0] non-english буквы, [1] english буквы
	StringOption strWordDivForXlat;
	IntOption Flags;       // дополнительные флаги
	int CurrentLayout;
};

struct EditorOptions
{
	IntOption TabSize;
	IntOption ExpandTabs;
	BoolOption PersistentBlocks;
	BoolOption DelRemovesBlocks;
	BoolOption AutoIndent;
	BoolOption AutoDetectCodePage;
	BoolOption AnsiCodePageForNewFile;
	BoolOption AnsiCodePageAsDefault;
	BoolOption CursorBeyondEOL;
	BoolOption BSLikeDel;
	IntOption CharCodeBase;
	BoolOption SavePos;
	BoolOption SaveShortPos;
	BoolOption F7Rules; // $ 28.11.2000 SVS - Правило на счет поиска в редакторе
	BoolOption AllowEmptySpaceAfterEof; // $ 21.06.2005 SKV - разрешить показывать пустое пространство после последней строки редактируемого файла.
	IntOption ReadOnlyLock; // $ 29.11.2000 SVS - лочить файл при открытии в редакторе, если он имеет атрибуты R|S|H
	IntOption UndoSize; // $ 03.12.2001 IS - размер буфера undo в редакторе
	BoolOption UseExternalEditor;
	IntOption FileSizeLimitLo;
	IntOption FileSizeLimitHi;
	BoolOption ShowKeyBar;
	BoolOption ShowTitleBar;
	BoolOption ShowScrollBar;
	BoolOption EditOpenedForWrite;
	BoolOption SearchSelFound;
	BoolOption SearchRegexp;
	BoolOption SearchPickUpWord;
	Bool3Option ShowWhiteSpace;

	StringOption strWordDiv;

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

	IntOption TabSize;
	BoolOption AutoDetectCodePage;
	BoolOption ShowScrollbar;     // $ 18.07.2000 tran пара настроек для viewer
	BoolOption ShowArrows;
	BoolOption PersistentBlocks; // $ 14.05.2002 VVM Постоянные блоки во вьюере
	BoolOption ViewerIsWrap; // (Wrap|WordWarp)=1 | UnWrap=0
	BoolOption ViewerWrap; // Wrap=0|WordWarp=1
	BoolOption SavePos;
	BoolOption SaveCodepage;
	BoolOption SaveWrapMode;
	BoolOption SaveShortPos;
	BoolOption UseExternalViewer;
	BoolOption ShowKeyBar; // $ 15.07.2000 tran + ShowKeyBar
	BoolOption AnsiCodePageAsDefault;
	BoolOption ShowTitleBar;
	BoolOption SearchRegexp;
	IntOption MaxLineSize; // 100..100000, default=10000
	BoolOption SearchEditFocus; // auto-focus on edit text/hex window
	BoolOption Visible0x00;
	IntOption ZeroChar;
};

// "Полиция"
struct PoliciesOptions
{
	IntOption DisabledOptions;  // разрешенность меню конфигурации
	BoolOption ShowHiddenDrives; // показывать скрытые логические диски
};

struct DialogsOptions
{
	BoolOption EditBlock;            // Постоянные блоки в строках ввода
	BoolOption EditHistory;          // Добавлять в историю?
	BoolOption AutoComplete;         // Разрешено автодополнение?
	BoolOption EULBsClear;           // = 1 - BS в диалогах для UnChanged строки удаляет такую строку также, как и Del
	BoolOption SelectFromHistory;    // = 0 then (ctrl-down в строке с историей курсор устанавливался на самую верхнюю строку)
	IntOption EditLine;             // общая информация о строке ввода (сейчас это пока... позволяет управлять выделением)
	IntOption MouseButton;          // Отключение восприятие правой/левой кнопки мышы как команд закрытия окна диалога
	BoolOption DelRemovesBlocks;
	IntOption CBoxMaxHeight;        // максимальный размер открываемого списка (по умолчанию=8)
};

struct VMenuOptions
{
	IntOption LBtnClick;
	IntOption RBtnClick;
	IntOption MBtnClick;
};

struct CommandLineOptions
{
	BoolOption EditBlock;
	BoolOption DelRemovesBlocks;
	BoolOption AutoComplete;
	BoolOption UsePromptFormat;
	StringOption strPromptFormat;
};

struct NowellOptions
{
	BoolOption MoveRO;               // перед операцией Move снимать R/S/H атрибуты, после переноса - выставлять обратно
};

struct ScreenSizes
{
	// на сколько поз. изменить размеры для распахнутого экрана
	IntOption DeltaX;
	IntOption DeltaY;
};

struct LoadPluginsOptions
{
	string strCustomPluginsPath;  // путь для поиска плагинов, указанный в /p
	string strPersonalPluginsPath;
//  IntOption TypeLoadPlugins;       // see TYPELOADPLUGINSOPTIONS
	bool MainPluginDir; // TRUE - использовать стандартный путь к основным плагинам
	bool PluginsCacheOnly; // seting by '/co' switch, not saved in registry
	bool PluginsPersonal;

	BoolOption SilentLoadPlugin; // при загрузке плагина с кривым...
#ifndef NO_WRAPPER
	BoolOption OEMPluginsSupport;
#endif // NO_WRAPPER
	BoolOption ScanSymlinks;
};

struct FindFileOptions
{
	IntOption FileSearchMode;
	BoolOption FindFolders;
	BoolOption FindSymLinks;
	BoolOption CollectFiles;
	BoolOption UseFilter;
	BoolOption FindAlternateStreams;
	StringOption strSearchInFirstSize;

	StringOption strSearchOutFormat;
	StringOption strSearchOutFormatWidth;
	int OutColumnCount;
	unsigned __int64 OutColumnTypes[PANEL_COLUMNCOUNT];
	int OutColumnWidths[PANEL_COLUMNCOUNT];
	int OutColumnWidthType[PANEL_COLUMNCOUNT];
};

struct InfoPanelOptions
{
	IntOption ComputerNameFormat;
	IntOption UserNameFormat;
	BoolOption ShowPowerStatus;
	StringOption strShowStatusInfo;
	StringOption strFolderInfoFiles;
	BoolOption ShowCDInfo;
};

struct TreeOptions
{
	IntOption MinTreeCount;         // Минимальное количество папок для сохранения дерева в файле.
	BoolOption AutoChangeFolder;     // Автосмена папок при перемещении по дереву
	IntOption TreeFileAttr;       // Файловые атрибуты для файлов-деревях

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
	BoolOption UseSystemCopy;         // использовать системную функцию копирования
	BoolOption CopyOpened;            // копировать открытые на запись файлы
	BoolOption CopyShowTotal;         // показать общий индикатор копирования
	BoolOption MultiCopy;             // "разрешить мультикопирование/перемещение/создание связей"
	IntOption CopySecurityOptions; // для операции Move - что делать с опцией "Copy access rights"
	IntOption CopyTimeRule;          // $ 30.01.2001 VVM  Показывает время копирования,оставшееся время и среднюю скорость
	IntOption BufferSize;
};

struct DeleteOptions
{
	BoolOption DelShowTotal;         // показать общий индикатор удаления
};

struct MacroOptions
{
	BoolOption MacroReuseRules; // Правило на счет повторно использования забинденных клавиш
	int DisableMacro; // параметры /m или /ma или /m....
	// config
	StringOption strKeyMacroCtrlDot, strKeyMacroRCtrlDot; // аля KEY_CTRLDOT/KEY_RCTRLDOT
	StringOption strKeyMacroCtrlShiftDot, strKeyMacroRCtrlShiftDot; // аля KEY_CTRLSHIFTDOT/KEY_RCTRLSHIFTDOT
	// internal
	int KeyMacroCtrlDot, KeyMacroRCtrlDot;
	int KeyMacroCtrlShiftDot, KeyMacroRCtrlShiftDot;
	StringOption strMacroCONVFMT; // формат преобразования double в строку
	StringOption strDateFormat; // Для $Date
};

struct KnownModulesIDs
{
	GUID Network;
	StringOption NetworkGuidStr;
	GUID Emenu;
	StringOption EmenuGuidStr;
};

struct ExecuteOptions
{
	BoolOption RestoreCPAfterExecute;
	BoolOption ExecuteUseAppPath;
	BoolOption ExecuteFullTitle;
	BoolOption ExecuteSilentExternal;
	StringOption strExecuteBatchType;
	StringOption strExcludeCmds;
	BoolOption    UseHomeDir; // cd ~
	StringOption strHomeDir; // cd ~
};

struct Options
{
	palette Palette;
	BoolOption Clock;
	BoolOption Mouse;
	BoolOption ShowKeyBar;
	BoolOption ScreenSaver;
	IntOption ScreenSaverTime;
	BoolOption UseVk_oem_x;
	BoolOption ShowHidden;
	BoolOption Highlight;

	StringOption strLeftFolder;
	StringOption strRightFolder;

	StringOption strLeftCurFile;
	StringOption strRightCurFile;

	BoolOption RightSelectedFirst;
	BoolOption LeftSelectedFirst;
	BoolOption SelectFolders;
	BoolOption ReverseSort;
	BoolOption SortFolderExt;
	BoolOption DeleteToRecycleBin;         // удалять в корзину?
	BoolOption DeleteToRecycleBinKillLink; // перед удалением папки в корзину кильнем вложенные симлинки.
	IntOption WipeSymbol; // символ заполнитель для "ZAP-операции"

	CopyMoveOptions CMOpt;

	DeleteOptions DelOpt;

	BoolOption MultiMakeDir; // Опция создания нескольких каталогов за один сеанс

	BoolOption CreateUppercaseFolders;
	BoolOption UseRegisteredTypes;

	BoolOption ViewerEditorClock;
	BoolOption OnlyEditorViewerUsed; // =1, если старт был /e или /v
	BoolOption SaveViewHistory;
	IntOption ViewHistoryCount;
	IntOption ViewHistoryLifetime;

	StringOption strExternalEditor;
	EditorOptions EdOpt;
	StringOption strExternalViewer;
	ViewerOptions ViOpt;


	StringOption strWordDiv; // $ 03.08.2000 SVS Разграничитель слов из реестра
	StringOption strQuotedSymbols;
	IntOption QuotedName;
	BoolOption AutoSaveSetup;
	IntOption ChangeDriveMode;
	BoolOption ChangeDriveDisconnectMode;

	BoolOption SaveHistory;
	IntOption HistoryCount;
	IntOption HistoryLifetime;
	BoolOption SaveFoldersHistory;
	IntOption FoldersHistoryCount;
	IntOption FoldersHistoryLifetime;
	IntOption DialogsHistoryCount;
	IntOption DialogsHistoryLifetime;

	FindFileOptions FindOpt;

	IntOption LeftHeightDecrement;
	IntOption RightHeightDecrement;
	IntOption WidthDecrement;

	BoolOption ShowColumnTitles;
	BoolOption ShowPanelStatus;
	BoolOption ShowPanelTotals;
	BoolOption ShowPanelFree;
	BoolOption PanelDetailedJunction;
	BoolOption ShowUnknownReparsePoint;
	BoolOption HighlightColumnSeparator;
	BoolOption DoubleGlobalColumnSeparator;

	BoolOption ShowPanelScrollbar;
	BoolOption ShowMenuScrollbar; // $ 29.06.2000 SVS Добавлен атрибут показа Scroll Bar в меню.
	BoolOption ShowScreensNumber;
	BoolOption ShowSortMode;
	BoolOption ShowMenuBar;
	IntOption FormatNumberSeparators;
	BoolOption CleanAscii;
	BoolOption NoGraphics;

	Confirmation Confirm;
	PluginConfirmation PluginConfirm;

	DizOptions Diz;

	BoolOption ShellRightLeftArrowsRule;
	PanelOptions LeftPanel;
	PanelOptions RightPanel;

	AutoCompleteOptions AutoComplete;

	IntOption  AutoUpdateLimit; // выше этого количество автоматически не обновлять панели.
	BoolOption AutoUpdateRemoteDrive;

	StringOption strLanguage;
	BoolOption SetIcon;
	BoolOption SetAdminIcon;
#ifndef NO_WRAPPER
	string strRegRoot;
#endif // NO_WRAPPER
	IntOption PanelRightClickRule; // задает поведение правой клавиши мыши
	IntOption PanelCtrlAltShiftRule; // задает поведение Ctrl-Alt-Shift для панелей.
	// Panel/CtrlFRule в реестре - задает поведение Ctrl-F. Если = 0, то штампуется файл как есть, иначе - с учетом отображения на панели
	BoolOption PanelCtrlFRule;
	/*
	  битовые флаги, задают поведение Ctrl-Alt-Shift
	   бит установлен - функция включена:
	   0 - Panel
	   1 - Edit
	   2 - View
	   3 - Help
	   4 - Dialog
	*/
	IntOption AllCtrlAltShiftRule;

	IntOption CASRule; // 18.12.2003 - Пробуем различать левый и правый CAS (попытка #1).
	/*
	  задает поведение Esc для командной строки:
	    =1 - Не изменять положение в History, если после Ctrl-E/Ctrl/-X
	         нажали ESC (поведение - аля VC).
	    =0 - поведение как и было - изменять положение в History
	*/
	BoolOption CmdHistoryRule;

	IntOption ExcludeCmdHistory;
	BoolOption SubstPluginPrefix; // 1 = подстанавливать префикс плагина (для Ctrl-[ и ему подобные)
	IntOption MaxPositionCache; // количество позиций в кэше сохранения
	BoolOption SetAttrFolderRules; // Правило на счет установки атрибутов на каталоги

	int UseExceptionHandler;
	BoolOption StoredExceptRules; // Правило на счет вызова исключений
	int ExceptRules;

	BoolOption ExceptUsed;
	StringOption strExceptEventSvc;
	/*
	 + Opt.ShiftsKeyRules - Правило на счет выбора механизма трансляции
	   Alt-Буква для нелатинским буковок и символов "`-=[]\;',./" с
	   модификаторами Alt-, Ctrl-, Alt-Shift-, Ctrl-Shift-, Ctrl-Alt-
	*/
	BoolOption ShiftsKeyRules;
	IntOption CursorSize[4];   // Размер курсора ФАРа

	CodeXLAT XLat;

	StringOption ConsoleDetachKey; // Комбинация клавиш для детача Far'овской консоли от длятельного неинтерактивного процесса в ней запущенного.

	StringOption strHelpLanguage;
	BoolOption FullScreenHelp;
	IntOption HelpTabSize;

	IntOption HelpURLRules; // =0 отключить возможность запуска URL-приложений

	// запоминать логические диски и не опрашивать каждый раз. Для предотвращения "просыпания" "зеленых" винтов.
	BoolOption RememberLogicalDrives;
	/*
	  будет влиять на:
	      добавление файлов в историю с разным регистром
	      добавление LastPositions в редакторе и вьюере
	*/
	BoolOption FlagPosixSemantics;

	IntOption MsWheelDelta; // задает смещение для прокрутки
	IntOption MsWheelDeltaView;
	IntOption MsWheelDeltaEdit;
	IntOption MsWheelDeltaHelp;
	// горизонтальная прокрутка
	IntOption MsHWheelDelta;
	IntOption MsHWheelDeltaView;
	IntOption MsHWheelDeltaEdit;

	/* $ 28.04.2001 VVM
	  + Opt.SubstNameRule битовая маска:
	    0 - если установлен, то опрашивать сменные диски при GetSubstName()
	    1 - если установлен, то опрашивать все остальные при GetSubstName() */
	IntOption SubstNameRule;

	/* $ 23.05.2001 AltF9
	  + Opt.AltF9 Флаг позволяет выбрать механизм  работы комбинации Alt-F9
	       (Изменение размера экрана) в оконном режиме. По умолчанию - 1.
	    0 - использовать механизм, совместимый с FAR версии 1.70 beta 3 и
	       ниже, т.е. переключение 25/50 линий.
	    1 - использовать усовершенствованный механизм - окно FAR Manager
	       будет переключаться с нормального на максимально доступный размер
	       консольного окна и обратно.*/
	BoolOption AltF9;

	BoolOption ClearType;

	Bool3Option PgUpChangeDisk;
	BoolOption ShowDotsInRoot;
	BoolOption ShowCheckingFile;
	BoolOption CloseCDGate;       // автомонтирование CD
	BoolOption UpdateEnvironment;

	ExecuteOptions Exec;

	IntOption PluginMaxReadData;
	BoolOption ScanJunction;

	IntOption ShowTimeoutDelFiles; // тайаут в процессе удаления (в ms)
	IntOption ShowTimeoutDACLFiles;
	IntOption DelThreadPriority; // приоритет процесса удаления, по умолчанию = THREAD_PRIORITY_NORMAL

	LoadPluginsOptions LoadPlug;

	DialogsOptions Dialogs;
	VMenuOptions VMenu;
	CommandLineOptions CmdLine;
	PoliciesOptions Policies;
	NowellOptions Nowell;
	ScreenSizes ScrSize;
	MacroOptions Macro;

	IntOption FindCodePage;

	TreeOptions Tree;
	InfoPanelOptions InfoPanel;

	BoolOption CPMenuMode;
	StringOption strNoAutoDetectCP;
	// Перечисленные здесь кодовые страницы будут исключены из детектирования nsUniversalDetectorEx.
	// Автодетект юникодных страниц от этого не зависит, поэтому UTF-8 будет определяться даже если
	// 65001 здесь присутствует. Если UniversalDetector выдаст страницу из этого списка, она будет
	// заменена на умолчательную ANSI или OEM, в зависимости от настроек.
	// пример: L"1250,1252,1253,1255,855,10005,28592,28595,28597,28598,38598,65001"
	// Если строка пустая никакой фильтрации кодовых страниц в UCD детекте не будет.
	// Если "-1", то в зависимости CPMenuMode (Ctrl-H в меню кодовых страниц фильтрация UCD либо будет
	// отключена, либо будут разрешенны только 'любимые' и системные (OEM ANSI) кодовые страницы.

	bool IsUserAdmin;
	StringOption strTitleAddons;
	StringOption strEditorTitleFormat;
	StringOption strViewerTitleFormat;

	IntOption StoredElevationMode;
	int ElevationMode;

	BoolOption StoredWindowMode;
	int WindowMode;

	int ReadOnlyConfig;

	string ProfilePath;
	string LocalProfilePath;
	string GlobalUserMenuDir;
	KnownModulesIDs KnownIDs;

	StringOption strBoxSymbols;

	BoolOption SmartFolderMonitor; // def: 0=always monitor panel folder(s), 1=only when FAR has input focus
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
bool GetConfigValue(size_t Root, const wchar_t* Name, GeneralConfig::OptionType& Type, Option*& Data);

bool AdvancedConfig();
