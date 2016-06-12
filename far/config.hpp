#ifndef CONFIG_HPP_E468759B_688C_4D45_A5BA_CF1D4FCC9A08
#define CONFIG_HPP_E468759B_688C_4D45_A5BA_CF1D4FCC9A08
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

#include "panel.hpp"
#include "palette.hpp"

class GeneralConfig;
class RegExp;

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
	//EXCLUDECMDHISTORY_NOTAPPLYCMD   = 0x00000010,  // не помещать в историю команды выполнения из "Apply Command"
};

enum QUOTEDNAMETYPE
{
	QUOTEDNAME_INSERT         = 0x00000001,            // кавычить при сбросе в командную строку, в диалогах и редакторе
	QUOTEDNAME_CLIPBOARD      = 0x00000002,            // кавычить при помещении в буфер обмена
};

enum
{
	DMOUSEBUTTON_LEFT = 0x00000001,
	DMOUSEBUTTON_RIGHT = 0x00000002,
};

enum
{
	VMENUCLICK_IGNORE = 0,
	VMENUCLICK_CANCEL = 1,
	VMENUCLICK_APPLY = 2,
};

enum DIZUPDATETYPE
{
	DIZ_NOT_UPDATE,
	DIZ_UPDATE_IF_DISPLAYED,
	DIZ_UPDATE_ALWAYS
};

enum disk_menu_mode
{
	DRIVE_SHOW_TYPE = 0x00000001,
	DRIVE_SHOW_NETNAME = 0x00000002,
	DRIVE_SHOW_LABEL = 0x00000004,
	DRIVE_SHOW_FILESYSTEM = 0x00000008,
	DRIVE_SHOW_SIZE = 0x00000010,
	DRIVE_SHOW_REMOVABLE = 0x00000020,
	DRIVE_SHOW_PLUGINS = 0x00000040,
	DRIVE_SHOW_CDROM = 0x00000080,
	DRIVE_SHOW_SIZE_FLOAT = 0x00000100,
	DRIVE_SHOW_REMOTE = 0x00000200,
	DRIVE_SORT_PLUGINS_BY_HOTKEY = 0x00000400,
};

struct column;
struct FARConfigItem;

class Option
{
public:
	template<class T>
	Option(const T& Value): m_Value(Value) {}
	virtual ~Option() = default;

	virtual string toString() const = 0;
	virtual void fromString(const string& value) = 0;
	virtual string ExInfo() const = 0;
	virtual string typeToString() const = 0;
	virtual bool IsDefault(const any& Default) const = 0;
	virtual void SetDefault(const any& Default) = 0;
	virtual bool Edit(class DialogBuilder* Builder, int Width, int Param) = 0;
	virtual void Export(FarSettingsItem& To) const = 0;

	bool Changed() const { return m_Value.touched(); }

protected:
	template<class T>
	const T& GetT() const { return any_cast<T>(m_Value); }
	template<class T>
	void SetT(const T& NewValue) { if (GetT<T>() != NewValue) m_Value = NewValue; }

private:
	friend class Options;

	virtual bool StoreValue(GeneralConfig* Storage, const string& KeyName, const string& ValueName, bool always) const = 0;
	virtual bool ReceiveValue(GeneralConfig* Storage, const string& KeyName, const string& ValueName, const any& Default) = 0;

	void MakeUnchanged() { m_Value.forget(); }

	monitored<any> m_Value;
};

template<class base_type, class derived>
class OptionImpl: public Option
{
public:
	typedef base_type underlying_type;

	OptionImpl(): Option(base_type()) {}
	OptionImpl(const base_type& Value): Option(Value) {}
	OptionImpl(const derived& Value): Option(Value.Get()) {}

	const base_type& Get() const { return GetT<base_type>(); }
	void Set(const base_type& Value) { SetT(Value); }

	virtual bool IsDefault(const any& Default) const override { return Get() == any_cast<base_type>(Default); }
	virtual void SetDefault(const any& Default) override { Set(any_cast<base_type>(Default)); }

	virtual bool ReceiveValue(GeneralConfig* Storage, const string& KeyName, const string& ValueName, const any& Default) override;
	virtual bool StoreValue(GeneralConfig* Storage, const string& KeyName, const string& ValueName, bool always) const override;

	//operator const base_type&() const { return Get(); }
};

class BoolOption: public OptionImpl<bool, BoolOption>
{
public:
	using OptionImpl<bool, BoolOption>::OptionImpl;

	virtual string toString() const override { return Get() ? L"true"s : L"false"s; }
	virtual void fromString(const string& value) override;
	virtual string ExInfo() const override { return {}; }
	virtual string typeToString() const override { return L"boolean"s; }
	virtual bool Edit(class DialogBuilder* Builder, int Width, int Param) override;
	virtual void Export(FarSettingsItem& To) const override;

	template<class T>
	BoolOption& operator=(const T& Value) { Set(Value); return *this; }

	operator bool() const { return Get(); }
};

class Bool3Option: public OptionImpl<long long, Bool3Option>
{
public:
	using OptionImpl<long long, Bool3Option>::OptionImpl;

	virtual string toString() const override { int v = Get(); return v ? (v == 1 ? L"true"s : L"other"s) : L"false"s; }
	virtual void fromString(const string& value) override;
	virtual string ExInfo() const override { return {}; }
	virtual string typeToString() const override { return L"3-state"s; }
	virtual bool Edit(class DialogBuilder* Builder, int Width, int Param) override;
	virtual void Export(FarSettingsItem& To) const override;

	template<class T>
	Bool3Option& operator=(const T& Value) { Set(Value); return *this; }

	operator FARCHECKEDSTATE() const { return static_cast<FARCHECKEDSTATE>(Get()); }
};

class IntOption: public OptionImpl<long long, IntOption>
{
public:
	using OptionImpl<long long, IntOption>::OptionImpl;

	virtual string toString() const override { return std::to_wstring(Get()); }
	virtual void fromString(const string& value) override;
	virtual string ExInfo() const override;
	virtual string typeToString() const override { return L"integer"s; }
	virtual bool Edit(class DialogBuilder* Builder, int Width, int Param) override;
	virtual void Export(FarSettingsItem& To) const override;

	template<class T>
	IntOption& operator=(const T& Value) { Set(Value); return *this; }

	IntOption& operator|=(long long Value){ Set(Get() | Value); return *this; }
	IntOption& operator&=(long long Value){Set(Get()&Value); return *this;}
	IntOption& operator%=(long long Value){Set(Get()%Value); return *this;}
	IntOption& operator^=(long long Value){Set(Get()^Value); return *this;}
	IntOption& operator--(){Set(Get()-1); return *this;}
	IntOption& operator++(){Set(Get()+1); return *this;}
	IntOption operator--(int){long long Current = Get(); Set(Current-1); return Current;}
	IntOption operator++(int){long long Current = Get(); Set(Current+1); return Current;}

	operator long long() const { return Get(); }
};

class StringOption: public OptionImpl<string, StringOption>
{
public:
	using OptionImpl<string, StringOption>::OptionImpl;

	virtual string toString() const override { return Get(); }
	virtual void fromString(const string& value) override { Set(value); }
	virtual string ExInfo() const override { return {}; }
	virtual string typeToString() const override { return L"string"s; }
	virtual bool Edit(class DialogBuilder* Builder, int Width, int Param) override;
	virtual void Export(FarSettingsItem& To) const override;

	template<class T>
	StringOption& operator=(const T& Value) { Set(Value); return *this; }

	StringOption& operator+=(const string& Value) {Set(Get()+Value); return *this;}
	wchar_t operator[] (size_t index) const { return Get()[index]; }
	const wchar_t* data() const { return Get().data(); }
	void clear() { Set({}); }
	bool empty() const { return Get().empty(); }
	size_t size() const { return Get().size(); }

	operator const string&() const { return Get(); }
};

class Options: noncopyable
{
	enum farconfig_mode
	{
		cfg_roaming,
		cfg_local,
	};

public:
	struct ViewerOptions;
	struct EditorOptions;

	Options();
	~Options();
	void ShellOptions(bool LastCommand, const MOUSE_EVENT_RECORD *MouseEvent);
	void Load(const std::vector<std::pair<string, string>>& Overridden);
	void Save(bool Manual);
	const Option* GetConfigValue(const wchar_t *Key, const wchar_t *Name) const;
	const Option* GetConfigValue(size_t Root, const wchar_t* Name) const;
	bool AdvancedConfig(farconfig_mode Mode = cfg_roaming);
	void LocalViewerConfig(ViewerOptions &ViOptRef) {return ViewerConfig(ViOptRef, true);}
	void LocalEditorConfig(EditorOptions &EdOptRef) {return EditorConfig(EdOptRef, true);}
	void SetSearchColumns(const string& Columns, const string& Widths);

	struct PanelOptions
	{
		IntOption m_Type;
		BoolOption Visible;
		IntOption ViewMode;
		IntOption SortMode;
		BoolOption ReverseSortOrder;
		BoolOption SortGroups;
		BoolOption ShowShortNames;
		BoolOption NumericSort;
		BoolOption CaseSensitiveSort;
		BoolOption SelectedFirst;
		BoolOption DirectoriesFirst;
		StringOption Folder;
		StringOption CurFile;
	};

	struct AutoCompleteOptions
	{
		BoolOption ShowList;
		BoolOption ModalList;
		BoolOption AppendCompletion;

		Bool3Option UseFilesystem;
		Bool3Option UseHistory;
		Bool3Option UsePath;
		Bool3Option UseEnvironment;
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
		BoolOption Esc;
		BoolOption EscTwiceToInterrupt;
		BoolOption RemoveConnection;
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
		CodeXLAT(): Layouts(), CurrentLayout() {}

		HKL Layouts[10];
		StringOption strLayouts;
		StringOption Rules[3]; // правила:
		// [0] "если предыдущий символ латинский"
		// [1] "если предыдущий символ нелатинский символ"
		// [2] "если предыдущий символ не рус/lat"
		StringOption Table[2]; // [0] non-english буквы, [1] english буквы
		StringOption strWordDivForXlat;
		IntOption Flags;
		mutable int CurrentLayout;
	};

	struct EditorOptions
	{
		IntOption TabSize;
		IntOption ExpandTabs;
		BoolOption PersistentBlocks;
		BoolOption DelRemovesBlocks;
		BoolOption AutoIndent;
		BoolOption AutoDetectCodePage;
		IntOption DefaultCodePage;
		StringOption strF8CPs;
		BoolOption CursorBeyondEOL;
		BoolOption BSLikeDel;
		IntOption CharCodeBase;
		BoolOption SavePos;
		BoolOption SaveShortPos;
		BoolOption AllowEmptySpaceAfterEof;
		IntOption ReadOnlyLock;
		IntOption UndoSize;
		BoolOption UseExternalEditor;
		IntOption FileSizeLimit;
		BoolOption ShowKeyBar;
		BoolOption ShowTitleBar;
		BoolOption ShowScrollBar;
		BoolOption EditOpenedForWrite;
		BoolOption SearchSelFound;
		BoolOption SearchCursorAtEnd;
		BoolOption SearchRegexp;
		Bool3Option ShowWhiteSpace;

		StringOption strWordDiv;

		BoolOption KeepEOL;
		BoolOption AddUnicodeBOM;
		BoolOption NewFileUnixEOL;
	};

	struct ViewerOptions
	{
		enum
		{
			eMinLineSize = 1*1000,
			eDefLineSize = 10*1000,
			eMaxLineSize = 100*1000
		};

		BoolOption AutoDetectCodePage;
		IntOption   DefaultCodePage;
		StringOption strF8CPs;
		IntOption   MaxLineSize; // 1000..100000, default=10000
		BoolOption PersistentBlocks;
		BoolOption  SaveCodepage;
		BoolOption SavePos;
		BoolOption  SaveShortPos;
		BoolOption SaveWrapMode;
		BoolOption  SearchEditFocus; // auto-focus on edit text/hex window
		BoolOption  SearchRegexp;
		Bool3Option SearchWrapStop; // [NonStop] / {Start-End} / [Full Cycle]
		BoolOption  ShowArrows;
		BoolOption ShowKeyBar;
		BoolOption  ShowScrollbar;
		BoolOption ShowTitleBar;
		IntOption   TabSize;
		BoolOption  UseExternalViewer;
		BoolOption  ViewerIsWrap; // (Wrap|WordWarp)=1 | UnWrap=0
		BoolOption  ViewerWrap; // Wrap=0|WordWarp=1
		BoolOption Visible0x00;
		IntOption  ZeroChar;
	};

	struct PoliciesOptions
	{
		BoolOption ShowHiddenDrives; // показывать скрытые логические диски
	};

	struct DialogsOptions
	{
		BoolOption EditBlock;            // Постоянные блоки в строках ввода
		BoolOption EditHistory;          // Добавлять в историю?
		BoolOption AutoComplete;         // Разрешено автодополнение?
		BoolOption EULBsClear;           // = 1 - BS в диалогах для UnChanged строки удаляет такую строку также, как и Del
		IntOption MouseButton;          // Отключение восприятие правой/левой кнопки мыши как команд закрытия окна диалога
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
		// перед операцией Move снимать R/S/H атрибуты, после переноса - выставлять обратно
		BoolOption MoveRO;
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
		bool MainPluginDir; // true - использовать стандартный путь к основным плагинам
		bool PluginsCacheOnly; // set by '/co' switch, not saved
		bool PluginsPersonal;

		BoolOption SilentLoadPlugin;
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
		BoolOption UseFilter;
		BoolOption FindAlternateStreams;
		StringOption strSearchInFirstSize;

		StringOption strSearchOutFormat;
		StringOption strSearchOutFormatWidth;

		std::vector<column> OutColumns;
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
		BoolOption TurnOffCompletely;   // Turn OFF SlowlyAndBuglyTreeView

		IntOption MinTreeCount;         // Минимальное количество папок для сохранения дерева в файле.
		BoolOption AutoChangeFolder;    // Автосмена папок при перемещении по дереву
		IntOption TreeFileAttr;         // Файловые атрибуты для файлов-деревях

#if defined(TREEFILE_PROJECT)
		BoolOption LocalDisk;           // Хранить файл структуры папок для локальных дисков
		BoolOption NetDisk;             // Хранить файл структуры папок для сетевых дисков
		BoolOption NetPath;             // Хранить файл структуры папок для сетевых путей
		BoolOption RemovableDisk;       // Хранить файл структуры папок для сменных дисков
		BoolOption CDDisk;              // Хранить файл структуры папок для CD/DVD/BD/etc дисков

		StringOption strLocalDisk;      // шаблон имени файла-деревяхи для локальных дисков
		StringOption strNetDisk;        // шаблон имени файла-деревяхи для сетевых дисков
		StringOption strNetPath;        // шаблон имени файла-деревяхи для сетевых путей
		StringOption strRemovableDisk;  // шаблон имени файла-деревяхи для сменных дисков
		StringOption strCDDisk;         // шаблон имени файла-деревяхи для CD/DVD/BD/etc дисков

		StringOption strExceptPath;     // для перечисленных здесь не хранить

		StringOption strSaveLocalPath;  // сюда сохраняем локальные диски
		StringOption strSaveNetPath;    // сюда сохраняем сетевые диски
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
		BoolOption ShowTotal;         // показать общий индикатор удаления
		BoolOption HighlightSelected;
		IntOption  ShowSelected;
	};

	struct MacroOptions
	{
		int DisableMacro; // параметры /m или /ma или /m....
		// config
		StringOption strKeyMacroCtrlDot, strKeyMacroRCtrlDot; // аля KEY_CTRLDOT/KEY_RCTRLDOT
		StringOption strKeyMacroCtrlShiftDot, strKeyMacroRCtrlShiftDot; // аля KEY_CTRLSHIFTDOT/KEY_RCTRLSHIFTDOT
		// internal
		DWORD KeyMacroCtrlDot, KeyMacroRCtrlDot;
		DWORD KeyMacroCtrlShiftDot, KeyMacroRCtrlShiftDot;
		StringOption strMacroCONVFMT; // формат преобразования double в строку
		StringOption strDateFormat; // Для $Date
		BoolOption ShowPlayIndicator; // показать вывод 'P' во время проигрывания макроса
	};

	struct KnownModulesIDs
	{
		struct GuidOption
		{
			GUID Id;
			StringOption StrId;
			const wchar_t* Default;
		};

		GuidOption Network;
		GuidOption Emenu;
		GuidOption Arclite;
		GuidOption Luamacro;
		GuidOption Netbox;
	};

	struct ExecuteOptions
	{
		BoolOption RestoreCPAfterExecute;
		BoolOption ExecuteUseAppPath;
		BoolOption ExecuteFullTitle;
		StringOption strExecuteBatchType;
		StringOption strExcludeCmds;
		struct
		{
			string Pattern;
			std::unique_ptr<RegExp> Re;
		}
		ComspecConditionRe;
		StringOption ComspecCondition;
		StringOption ComspecArguments;
		BoolOption   UseHomeDir; // cd ~
		StringOption strHomeDir; // cd ~
	};

	palette Palette;
	BoolOption Clock;
	BoolOption Mouse;
	BoolOption ShowKeyBar;
	BoolOption ScreenSaver;
	IntOption ScreenSaverTime;
	BoolOption UseVk_oem_x;
	BoolOption ShowHidden;
	BoolOption ShortcutAlwaysChdir;
	BoolOption Highlight;
	BoolOption RightClickSelect;
	BoolOption ShowBytes;

	BoolOption SelectFolders;
	BoolOption ReverseSort;
	BoolOption SortFolderExt;
	BoolOption DeleteToRecycleBin;
	IntOption WipeSymbol; // символ заполнитель для "ZAP-операции"

	CopyMoveOptions CMOpt;

	DeleteOptions DelOpt;

	BoolOption MultiMakeDir; // Опция создания нескольких каталогов за один сеанс

	BoolOption UseRegisteredTypes;

	BoolOption ViewerEditorClock;
	BoolOption SaveViewHistory;
	IntOption ViewHistoryCount;
	IntOption ViewHistoryLifetime;

	StringOption strExternalEditor;
	EditorOptions EdOpt;
	StringOption strExternalViewer;
	ViewerOptions ViOpt;

	// alias for EdOpt.strWordDiv
	StringOption& strWordDiv;
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
	BoolOption ShowMenuScrollbar;
	BoolOption ShowScreensNumber;
	BoolOption ShowSortMode;
	BoolOption ShowMenuBar;
	StringOption FormatNumberSeparators;
	BoolOption CleanAscii;
	BoolOption NoGraphics;

	Confirmation Confirm;
	PluginConfirmation PluginConfirm;

	DizOptions Diz;

	BoolOption ShellRightLeftArrowsRule;
	PanelOptions LeftPanel;
	PanelOptions RightPanel;
	BoolOption LeftFocus;

	AutoCompleteOptions AutoComplete;

	// выше этого количество автоматически не обновлять панели.
	IntOption  AutoUpdateLimit;
	BoolOption AutoUpdateRemoteDrive;

	StringOption strLanguage;
	BoolOption SetIcon;
	BoolOption SetAdminIcon;
	IntOption PanelRightClickRule;
	IntOption PanelCtrlAltShiftRule;
	// поведение Ctrl-F. Если = 0, то штампуется файл как есть, иначе - с учетом отображения на панели
	BoolOption PanelCtrlFRule;
	/*
	поведение Ctrl-Alt-Shift
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
	BoolOption SetAttrFolderRules;

	BoolOption ExceptUsed;
	StringOption strExceptEventSvc;
	/*
	Правило на счет выбора механизма трансляции
	Alt-Буква для нелатинским буковок и символов "`-=[]\;',./" с
	модификаторами Alt-, Ctrl-, Alt-Shift-, Ctrl-Shift-, Ctrl-Alt-
	*/
	BoolOption ShiftsKeyRules;
	IntOption CursorSize[4];

	CodeXLAT XLat;

	StringOption ConsoleDetachKey; // Комбинация клавиш для детача Far'овской консоли от длятельного неинтерактивного процесса в ней запущенного.

	StringOption strHelpLanguage;
	BoolOption FullScreenHelp;
	IntOption HelpTabSize;

	IntOption HelpURLRules; // =0 отключить возможность запуска URL-приложений
	BoolOption HelpSearchRegexp;

	// запоминать логические диски и не опрашивать каждый раз. Для предотвращения "просыпания" "зеленых" винтов.
	BoolOption RememberLogicalDrives;
	BoolOption FlagPosixSemantics;

	IntOption MsWheelDelta; // задает смещение для прокрутки
	IntOption MsWheelDeltaView;
	IntOption MsWheelDeltaEdit;
	IntOption MsWheelDeltaHelp;
	// горизонтальная прокрутка
	IntOption MsHWheelDelta;
	IntOption MsHWheelDeltaView;
	IntOption MsHWheelDeltaEdit;

	/*
	битовая маска:
	    0 - если установлен, то опрашивать сменные диски при GetSubstName()
	    1 - если установлен, то опрашивать все остальные при GetSubstName()
	*/
	IntOption SubstNameRule;

	/* $ 23.05.2001 AltF9
	  + Флаг позволяет выбрать механизм  работы комбинации Alt-F9
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

	IntOption RedrawTimeout;
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
	// отключена, либо будут разрешенны только избранные и системные (OEM ANSI) кодовые страницы.

	StringOption strTitleAddons;
	StringOption strEditorTitleFormat;
	StringOption strViewerTitleFormat;

	IntOption StoredElevationMode;

	BoolOption StoredWindowMode;

	string ProfilePath;
	string LocalProfilePath;
	string TemplateProfilePath;
	string GlobalUserMenuDir;
	KnownModulesIDs KnownIDs;

	StringOption strBoxSymbols;

	BoolOption SmartFolderMonitor; // def: 0=always monitor panel folder(s), 1=only when FAR has input focus

	int ReadOnlyConfig;
	int UseExceptionHandler;
	int ElevationMode;
	int WindowMode;

	const std::vector<PanelViewSettings>& ViewSettings;

	class farconfig;

private:
	void InitConfig();
	void InitConfigData();
	intptr_t AdvancedConfigDlgProc(class Dialog* Dlg, intptr_t Msg, intptr_t Param1, void* Param2);
	void SystemSettings();
	void PanelSettings();
	void InterfaceSettings();
	void DialogSettings();
	void VMenuSettings();
	void CmdlineSettings();
	void SetConfirmations();
	void PluginsManagerSettings();
	void SetDizConfig();
	void ViewerConfig(ViewerOptions &ViOptRef, bool Local = false);
	void EditorConfig(EditorOptions &EdOptRef, bool Local = false);
	void SetFolderInfoFiles();
	void InfoPanelSettings();
	static void MaskGroupsSettings();
	void AutoCompleteSettings();
	void TreeSettings();
	void SetFilePanelModes();
	void SetViewSettings(size_t Index, PanelViewSettings&& Data);
	void AddViewSettings(size_t Index, PanelViewSettings&& Data);
	void DeleteViewSettings(size_t Index);
	void ReadPanelModes();
	void SavePanelModes(bool always);

	std::vector<farconfig> Config;
	farconfig_mode CurrentConfig;
	std::vector<PanelViewSettings> m_ViewSettings;
	bool m_ViewSettingsChanged;
};

string GetFarIniString(const string& AppName, const string& KeyName, const string& Default);
int GetFarIniInt(const string& AppName, const string& KeyName, int Default);

clock_t GetRedrawTimeout();

#endif // CONFIG_HPP_E468759B_688C_4D45_A5BA_CF1D4FCC9A08
