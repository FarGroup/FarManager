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

#include "palette.hpp"

class GeneralConfig;
class RegExp;
struct PanelViewSettings;
struct hash_icase;
struct equal_to_icase;
struct column;
struct FARConfigItem;

enum
{
	CASR_PANEL  = bit(0),
	CASR_EDITOR = bit(1),
	CASR_VIEWER = bit(2),
	CASR_HELP   = bit(3),
	CASR_DIALOG = bit(4),
};

enum ExcludeCmdHistoryType
{
	EXCLUDECMDHISTORY_NOTWINASS    = bit(0), // не помещать в историю команды ассоциаций Windows
	EXCLUDECMDHISTORY_NOTFARASS    = bit(1), // не помещать в историю команды выполнения ассоциаций файлов
	EXCLUDECMDHISTORY_NOTPANEL     = bit(2), // не помещать в историю команды выполнения с панели
	EXCLUDECMDHISTORY_NOTCMDLINE   = bit(3), // не помещать в историю команды выполнения с ком.строки
	//EXCLUDECMDHISTORY_NOTAPPLYCMD   = bit(4), // не помещать в историю команды выполнения из "Apply Command"
};

enum QUOTEDNAMETYPE
{
	QUOTEDNAME_INSERT         = bit(0), // кавычить при сбросе в командную строку, в диалогах и редакторе
	QUOTEDNAME_CLIPBOARD      = bit(1), // кавычить при помещении в буфер обмена
};

enum
{
	DMOUSEBUTTON_LEFT = bit(0),
	DMOUSEBUTTON_RIGHT = bit(1),
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
	DRIVE_SHOW_TYPE              = bit(0),
	DRIVE_SHOW_PATH              = bit(1),
	DRIVE_SHOW_LABEL             = bit(2),
	DRIVE_SHOW_FILESYSTEM        = bit(3),
	DRIVE_SHOW_SIZE              = bit(4),
	DRIVE_SHOW_REMOVABLE         = bit(5),
	DRIVE_SHOW_PLUGINS           = bit(6),
	DRIVE_SHOW_CDROM             = bit(7),
	DRIVE_SHOW_SIZE_FLOAT        = bit(8),
	DRIVE_SHOW_REMOTE            = bit(9),
	DRIVE_SORT_PLUGINS_BY_HOTKEY = bit(10),
};

class Option
{
public:
	virtual ~Option() = default;

	virtual string toString() const = 0;
	virtual bool TryParse(const string& value) = 0;
	virtual string ExInfo() const = 0;
	virtual const wchar_t* GetType() const = 0;
	virtual bool IsDefault(const any& Default) const = 0;
	virtual void SetDefault(const any& Default) = 0;
	virtual bool Edit(class DialogBuilder* Builder, int Width, int Param) = 0;
	virtual void Export(FarSettingsItem& To) const = 0;

	bool Changed() const { return m_Value.touched(); }

protected:
	template<class T>
	explicit Option(const T& Value): m_Value(Value) {}

	template<class T>
	const T& GetT() const { return any_cast<T>(m_Value); }

	template<class T>
	void SetT(const T& NewValue) { if (GetT<T>() != NewValue) m_Value = NewValue; }

private:
	friend class Options;

	virtual bool StoreValue(GeneralConfig* Storage, const string& KeyName, const string& ValueName, bool always) const = 0;
	virtual bool ReceiveValue(const GeneralConfig* Storage, const string& KeyName, const string& ValueName, const any& Default) = 0;

	void MakeUnchanged() { m_Value.forget(); }

	monitored<any> m_Value;
};

namespace detail
{
	template<class base_type, class derived>
	class OptionImpl: public Option
	{
	public:
		using underlying_type = base_type;
		using validator_type = std::function<base_type(const base_type&)>;
		using impl_type = OptionImpl<base_type, derived>;

		auto& operator=(const base_type& Value) { Set(Value); return static_cast<derived&>(*this); }

		void SetValidator(const validator_type& Validator) { m_Validator = Validator; }

		const auto& Get() const { return GetT<base_type>(); }
		void Set(const base_type& Value) { SetT(Validate(Value)); }
		bool TrySet(const base_type& Value)
		{
			if (Validate(Value) != Value)
			{
				return false;
			}
			SetT(Value);
			return true;
		}

		virtual string ExInfo() const override { return {}; }

		virtual bool IsDefault(const any& Default) const override { return Get() == any_cast<base_type>(Default); }
		virtual void SetDefault(const any& Default) override { Set(any_cast<base_type>(Default)); }

		virtual bool ReceiveValue(const GeneralConfig* Storage, const string& KeyName, const string& ValueName, const any& Default) override;
		virtual bool StoreValue(GeneralConfig* Storage, const string& KeyName, const string& ValueName, bool always) const override;

		//operator const base_type&() const { return Get(); }

	protected:
		OptionImpl(): Option(base_type())
		{
			static_assert((std::is_base_of_v<OptionImpl, derived>));
		}

	private:
		base_type Validate(const base_type& Value) const { return m_Validator? m_Validator(Value) : Value; }

		validator_type m_Validator;
	};
}

class BoolOption: public detail::OptionImpl<bool, BoolOption>
{
public:
	using impl_type::OptionImpl;
	using impl_type::operator=;

	virtual string toString() const override { return Get() ? L"true"s : L"false"s; }
	virtual bool TryParse(const string& value) override;
	virtual const wchar_t* GetType() const override { return L"boolean"; }
	virtual bool Edit(class DialogBuilder* Builder, int Width, int Param) override;
	virtual void Export(FarSettingsItem& To) const override;

	operator bool() const { return Get(); }
};

class Bool3Option: public detail::OptionImpl<long long, Bool3Option>
{
public:
	using impl_type::OptionImpl;
	using impl_type::operator=;

	virtual string toString() const override { const auto v = Get(); return v == BSTATE_CHECKED? L"true"s : v == BSTATE_UNCHECKED? L"false"s : L"other"s; }
	virtual bool TryParse(const string& value) override;
	virtual const wchar_t* GetType() const override { return L"3-state"; }
	virtual bool Edit(class DialogBuilder* Builder, int Width, int Param) override;
	virtual void Export(FarSettingsItem& To) const override;

	operator FARCHECKEDSTATE() const { return static_cast<FARCHECKEDSTATE>(Get()); }
};

class IntOption: public detail::OptionImpl<long long, IntOption>
{
public:
	using impl_type::OptionImpl;
	using impl_type::operator=;

	virtual string toString() const override { return str(Get()); }
	virtual bool TryParse(const string& value) override;
	virtual string ExInfo() const override;
	virtual const wchar_t* GetType() const override { return L"integer"; }
	virtual bool Edit(class DialogBuilder* Builder, int Width, int Param) override;
	virtual void Export(FarSettingsItem& To) const override;

	IntOption& operator|=(long long Value){Set(Get()|Value); return *this;}
	IntOption& operator&=(long long Value){Set(Get()&Value); return *this;}
	IntOption& operator%=(long long Value){Set(Get()%Value); return *this;}
	IntOption& operator^=(long long Value){Set(Get()^Value); return *this;}
	IntOption& operator--(){Set(Get()-1); return *this;}
	IntOption& operator++(){Set(Get()+1); return *this;}

	operator long long() const { return Get(); }
};

class StringOption: public detail::OptionImpl<string, StringOption>
{
public:
	using impl_type::OptionImpl;
	using impl_type::operator=;

	virtual string toString() const override { return Get(); }
	virtual bool TryParse(const string& value) override { Set(value); return true; }
	virtual const wchar_t* GetType() const override { return L"string"; }
	virtual bool Edit(class DialogBuilder* Builder, int Width, int Param) override;
	virtual void Export(FarSettingsItem& To) const override;

	StringOption& operator+=(const string& Value) {Set(Get()+Value); return *this;}
	wchar_t operator[] (size_t index) const { return Get()[index]; }
	const wchar_t* data() const { return Get().data(); }
	void clear() { Set({}); }
	bool empty() const { return Get().empty(); }
	size_t size() const { return Get().size(); }

	operator const string&() const { return Get(); }
	operator string_view() const { return Get(); }
};

class Options: noncopyable
{
	enum class config_type
	{
		roaming,
		local,
	};

public:
	struct ViewerOptions;
	struct EditorOptions;

	Options();
	~Options();
	void ShellOptions(bool LastCommand, const MOUSE_EVENT_RECORD *MouseEvent);
	void Load(std::unordered_map<string, string, hash_icase, equal_to_icase>&& Overrides);
	void Save(bool Manual);
	const Option* GetConfigValue(const wchar_t *Key, const wchar_t *Name) const;
	const Option* GetConfigValue(size_t Root, const wchar_t* Name) const;
	bool AdvancedConfig(config_type Mode = config_type::roaming);
	void LocalViewerConfig(ViewerOptions &ViOptRef) {return ViewerConfig(ViOptRef, true);}
	void LocalEditorConfig(EditorOptions &EdOptRef) {return EditorConfig(EdOptRef, true);}
	static void SetSearchColumns(const string& Columns, const string& Widths);

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
		StringOption Comspec;
		StringOption ComspecArguments;
		struct
		{
			string Pattern;
			std::unique_ptr<RegExp> Re;
		}
		ComspecConditionRe;
		StringOption ComspecCondition;
		BoolOption   UseHomeDir; // cd ~
		StringOption strHomeDir; // cd ~
	};

	palette Palette;
	BoolOption Clock;
	BoolOption Mouse;
	BoolOption ShowKeyBar;
	BoolOption ScreenSaver;
	IntOption ScreenSaverTime;
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
	Bool3Option ShowScreensNumber;
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
	BoolOption WindowModeStickyX;
	BoolOption WindowModeStickyY;

	const std::vector<PanelViewSettings>& ViewSettings;

	class farconfig;

private:
	void InitConfigs();
	void InitConfigsData();
	farconfig& GetConfig(config_type Type);
	const farconfig& GetConfig(config_type Type) const;
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

	std::vector<farconfig> m_Configs;
	std::vector<string>* m_ConfigStrings;
	config_type m_CurrentConfigType;
	std::vector<PanelViewSettings> m_ViewSettings;
	bool m_ViewSettingsChanged;
};

string GetFarIniString(const string& AppName, const string& KeyName, const string& Default);
int GetFarIniInt(const string& AppName, const string& KeyName, int Default);

std::chrono::steady_clock::duration GetRedrawTimeout();

#endif // CONFIG_HPP_E468759B_688C_4D45_A5BA_CF1D4FCC9A08
