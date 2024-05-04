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

// Internal:
#include "palette.hpp"
#include "plugin.hpp"
#include "string_utils.hpp"

// Platform:

// Common:
#include "common/multifunction.hpp"
#include "common/monitored.hpp"
#include "common/string_utils.hpp"
#include "common/utility.hpp"

// External:

//----------------------------------------------------------------------------

struct FarSettingsItem;
class GeneralConfig;
class RegExp;
class DialogBuilder;
struct PanelViewSettings;
struct column;
struct FARConfigItem;

enum class panel_sort: int
{
	UNSORTED,
	BY_NAME,
	BY_EXT,
	BY_MTIME,
	BY_CTIME,
	BY_ATIME,
	BY_SIZE,
	BY_DIZ,
	BY_OWNER,
	BY_COMPRESSEDSIZE,
	BY_NUMLINKS,
	BY_NUMSTREAMS,
	BY_STREAMSSIZE,
	BY_NAMEONLY,
	BY_CHTIME,

	COUNT,

	BY_USER = 100000
};

enum class sort_order: int
{
	first,

	flip_or_default = first,
	keep,
	ascend,
	descend,

	last = descend
};

enum
{
	CASR_PANEL  = 0_bit,
	CASR_EDITOR = 1_bit,
	CASR_VIEWER = 2_bit,
	CASR_HELP   = 3_bit,
	CASR_DIALOG = 4_bit,
};

enum ExcludeCmdHistoryType
{
	EXCLUDECMDHISTORY_NOTWINASS    = 0_bit, // не помещать в историю команды ассоциаций Windows
	EXCLUDECMDHISTORY_NOTFARASS    = 1_bit, // не помещать в историю команды выполнения ассоциаций файлов
	EXCLUDECMDHISTORY_NOTPANEL     = 2_bit, // не помещать в историю команды выполнения с панели
	EXCLUDECMDHISTORY_NOTCMDLINE   = 3_bit, // не помещать в историю команды выполнения с ком.строки
};

enum QUOTEDNAMETYPE
{
	QUOTEDNAME_INSERT         = 0_bit, // кавычить при сбросе в командную строку, в диалогах и редакторе
	QUOTEDNAME_CLIPBOARD      = 1_bit, // кавычить при помещении в буфер обмена
};

enum
{
	DMOUSEBUTTON_LEFT  = 0_bit,
	DMOUSEBUTTON_RIGHT = 1_bit,
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
	DRIVE_SHOW_TYPE              = 0_bit,
	DRIVE_SHOW_ASSOCIATED_PATH   = 1_bit,
	DRIVE_SHOW_LABEL             = 2_bit,
	DRIVE_SHOW_FILESYSTEM        = 3_bit,
	DRIVE_SHOW_SIZE              = 4_bit,
	DRIVE_SHOW_REMOVABLE         = 5_bit,
	DRIVE_SHOW_PLUGINS           = 6_bit,
	DRIVE_SHOW_CDROM             = 7_bit,
	DRIVE_SHOW_SIZE_FLOAT        = 8_bit,
	DRIVE_SHOW_REMOTE            = 9_bit,
	DRIVE_SORT_PLUGINS_BY_HOTKEY = 10_bit,
	DRIVE_SHOW_LABEL_USE_SHELL   = 11_bit,
	DRIVE_SHOW_VIRTUAL           = 12_bit,
	DRIVE_SHOW_UNMOUNTED_VOLUMES = 13_bit,
};

class Option
{
public:
	using variant = std::variant<long long, string, bool>;

	virtual ~Option() = default;

	[[nodiscard]]
	virtual string toString() const = 0;
	[[nodiscard]]
	virtual bool TryParse(string_view value) = 0;
	[[nodiscard]]
	virtual string ExInfo() const = 0;
	[[nodiscard]]
	virtual string_view GetType() const = 0;
	[[nodiscard]]
	virtual bool IsDefault(const variant& Default) const = 0;
	virtual void SetDefault(const variant& Default) = 0;
	[[nodiscard]]
	virtual bool Edit(DialogBuilder& Builder, int Param) = 0;
	virtual void Export(FarSettingsItem& To) const = 0;

	[[nodiscard]]
	bool Changed() const { return m_Value.touched(); }

protected:
	COPY_CONSTRUCTIBLE(Option);
	COPY_ASSIGNABLE_DEFAULT(Option);

	explicit Option(const auto& Value): m_Value(Value) {}

	template<class T>
	[[nodiscard]]
	const T& GetT() const { return std::get<T>(m_Value.value()); }

	template<class T>
	void SetT(const T& NewValue) { if (GetT<T>() != NewValue) m_Value = NewValue; }

private:
	friend class Options;

	virtual void StoreValue(GeneralConfig* Storage, string_view KeyName, string_view ValueName, bool always) const = 0;
	virtual bool ReceiveValue(const GeneralConfig* Storage, string_view KeyName, string_view ValueName, const variant& Default) = 0;

	void MakeUnchanged() { m_Value.forget(); }

	monitored<variant> m_Value;
};

namespace option
{
	class validator_tag{};
	class notifier_tag{};

	auto validator(auto&& Callable)
	{
		return overload
		{
			[Callable = FWD(Callable)](validator_tag, const auto& Value){ return Callable(Value); },
			[](notifier_tag, const auto&){}
		};
	}

	auto notifier(auto&& Callable)
	{
		return overload
		{
			[](validator_tag, const auto& Value){ return Value; },
			[Callable = FWD(Callable)](notifier_tag, const auto& Value){ Callable(Value); }
		};
	}
}

namespace detail
{
	template<class base_type, class derived>
	class OptionImpl: public Option
	{
	public:
		using underlying_type = base_type;
		using impl_type = OptionImpl<base_type, derived>;

		using callback_type = multifunction<
			base_type(option::validator_tag, const base_type&),
			void(option::notifier_tag, const base_type&)
		>;

		void SetCallback(const callback_type& Callback)
		{
			assert(!m_Callback);
			m_Callback = Callback;
		}

		[[nodiscard]]
		const auto& Get() const
		{
			return GetT<base_type>();
		}

		void Set(const base_type& Value)
		{
			const auto Validated = Validate(Value);
			if (Validated == Get())
				return;

			SetT(Validated);
			Notify();
		}

		[[nodiscard]]
		bool TrySet(const base_type& Value)
		{
			if (Validate(Value) != Value)
				return false;

			SetT(Value);
			Notify();
			return true;
		}

		[[nodiscard]]
		string ExInfo() const override { return {}; }

		[[nodiscard]]
		bool IsDefault(const variant& Default) const override { return Get() == std::get<base_type>(Default); }
		void SetDefault(const variant& Default) override { Set(std::get<base_type>(Default)); }

		[[nodiscard]]
		bool ReceiveValue(const GeneralConfig* Storage, string_view KeyName, string_view ValueName, const variant& Default) override;
		void StoreValue(GeneralConfig* Storage, string_view KeyName, string_view ValueName, bool always) const override;

		//operator const base_type&() const { return Get(); }

	protected:
		OptionImpl():
			Option(base_type())
		{
			static_assert(std::derived_from<derived, OptionImpl>);
		}

		auto& operator=(const base_type& Value)
		{
			Set(Value);
			return static_cast<derived&>(*this);
		}

	private:
		[[nodiscard]]
		base_type Validate(const base_type& Value) const
		{
			return m_Callback?
				m_Callback(option::validator_tag{}, Value) :
				Value;
		}

		void Notify() const
		{
			if (m_Callback)
				m_Callback(option::notifier_tag{}, Get());
		}

		callback_type m_Callback;
	};
}

class BoolOption final: public detail::OptionImpl<bool, BoolOption>
{
public:
	using impl_type::OptionImpl;
	using impl_type::operator=;

	[[nodiscard]]
	string toString() const override { return Get() ? L"true"s : L"false"s; }
	[[nodiscard]]
	bool TryParse(string_view value) override;
	[[nodiscard]]
	string_view GetType() const override { return L"boolean"sv; }
	[[nodiscard]]
	bool Edit(DialogBuilder& Builder, int Param) override;
	void Export(FarSettingsItem& To) const override;

	[[nodiscard]]
	explicit(false) operator bool() const { return Get(); }
};

class Bool3Option final: public detail::OptionImpl<long long, Bool3Option>
{
public:
	using impl_type::OptionImpl;
	using impl_type::operator=;

	[[nodiscard]]
	string toString() const override { const auto v = Get(); return v == BSTATE_CHECKED? L"true"s : v == BSTATE_UNCHECKED? L"false"s : L"other"s; }
	[[nodiscard]]
	bool TryParse(string_view value) override;
	[[nodiscard]]
	string_view GetType() const override { return L"3-state"sv; }
	[[nodiscard]]
	bool Edit(DialogBuilder& Builder, int Param) override;
	void Export(FarSettingsItem& To) const override;

	[[nodiscard]]
	explicit(false) operator FARCHECKEDSTATE() const { return static_cast<FARCHECKEDSTATE>(Get()); }
};

class IntOption final: public detail::OptionImpl<long long, IntOption>
{
public:
	using impl_type::OptionImpl;
	using impl_type::operator=;

	[[nodiscard]]
	string toString() const override;
	[[nodiscard]]
	bool TryParse(string_view value) override;
	[[nodiscard]]
	string ExInfo() const override;
	[[nodiscard]]
	string_view GetType() const override { return L"integer"sv; }
	[[nodiscard]]
	bool Edit(DialogBuilder& Builder, int Param) override;
	void Export(FarSettingsItem& To) const override;

	IntOption& operator|=(long long Value){Set(Get()|Value); return *this;}
	IntOption& operator&=(long long Value){Set(Get()&Value); return *this;}
	IntOption& operator%=(long long Value){Set(Get()%Value); return *this;}
	IntOption& operator^=(long long Value){Set(Get()^Value); return *this;}
	IntOption& operator--(){Set(Get()-1); return *this;}
	IntOption& operator++(){Set(Get()+1); return *this;}

	[[nodiscard]]
	explicit(false) operator long long() const { return Get(); }
};

class StringOption final: public detail::OptionImpl<string, StringOption>
{
public:
	using impl_type::OptionImpl;
	using impl_type::operator=;

	[[nodiscard]]
	string toString() const override { return Get(); }
	[[nodiscard]]
	bool TryParse(string_view value) override { Set(string(value)); return true; }
	[[nodiscard]]
	string_view GetType() const override { return L"string"sv; }
	[[nodiscard]]
	bool Edit(DialogBuilder& Builder, int Param) override;
	void Export(FarSettingsItem& To) const override;

	StringOption& operator+=(const string& Value) {Set(Get()+Value); return *this;}
	[[nodiscard]]
	wchar_t operator[] (size_t index) const { return Get()[index]; }
	[[nodiscard]]
	const wchar_t* c_str() const { return Get().c_str(); }
	void clear() { Set({}); }
	[[nodiscard]]
	bool empty() const { return Get().empty(); }
	[[nodiscard]]
	size_t size() const { return Get().size(); }

	[[nodiscard]]
	explicit(false) operator const string&() const { return Get(); }
	[[nodiscard]]
	explicit(false) operator string_view() const { return Get(); }
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
	using overrides = unordered_string_map_icase<string_view>;
	void Load(overrides&& Overrides);
	void Save(bool Manual);
	const Option* GetConfigValue(string_view Key, string_view Name) const;
	const Option* GetConfigValue(size_t Root, string_view Name) const;
	bool AdvancedConfig(config_type Mode = config_type::roaming);
	void LocalViewerConfig(ViewerOptions &ViOptRef) {return ViewerConfig(ViOptRef, true);}
	void LocalEditorConfig(EditorOptions &EdOptRef) {return EditorConfig(EdOptRef, true);}
	void SetSearchColumns(string_view Columns, string_view Widths);
	void SetFilePanelModes();
	static void MaskGroupsSettings();

	struct SortingOptions
	{
		enum class collation
		{
			ordinal    = 0,
			invariant  = 1,
			linguistic = 2,
		};

		IntOption Collation;
		BoolOption DigitsAsNumbers;
		BoolOption CaseSensitive;
	};

	struct PanelOptions
	{
		IntOption m_Type;
		BoolOption Visible;
		IntOption ViewMode;
		IntOption SortMode;
		BoolOption ReverseSortOrder;
		BoolOption SortGroups;
		BoolOption ShowShortNames;
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
		BoolOption ValidateConversion;
	};

	struct CodeXLAT
	{
		HKL Layouts[10]{};
		StringOption strLayouts;
		StringOption Rules[3]; // правила:
		// [0] "если предыдущий символ латинский"
		// [1] "если предыдущий символ нелатинский символ"
		// [2] "если предыдущий символ не рус/lat"
		StringOption Table[2]; // [0] non-english буквы, [1] english буквы
		StringOption strWordDivForXlat;
		IntOption Flags;
		mutable int CurrentLayout{};
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
		Bool3Option ShowWhiteSpace;

		StringOption strWordDiv;

		BoolOption KeepEOL;
		BoolOption AddUnicodeBOM;
		BoolOption NewFileUnixEOL;
		BoolOption SaveSafely;
		BoolOption CreateBackups;
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
		BoolOption DetectDumpMode;
		IntOption DefaultCodePage;
		StringOption strF8CPs;
		IntOption MaxLineSize; // 1000..100000, default=10000
		BoolOption PersistentBlocks;
		BoolOption SaveCodepage;
		BoolOption SavePos;
		BoolOption SaveShortPos;
		BoolOption SaveViewMode;
		BoolOption SaveWrapMode;
		Bool3Option SearchWrapStop; // [NonStop] / {Start-End} / [Full Cycle]
		BoolOption ShowArrows;
		BoolOption ShowKeyBar;
		BoolOption ShowScrollbar;
		BoolOption ShowTitleBar;
		IntOption TabSize;
		BoolOption UseExternalViewer;
		BoolOption ViewerIsWrap; // (Wrap|WordWarp)=1 | UnWrap=0
		BoolOption ViewerWrap; // Wrap=0|WordWarp=1
		BoolOption Visible0x00;
		IntOption ZeroChar;
	};

	struct PoliciesOptions
	{
		BoolOption ShowHiddenDrives; // показывать скрытые логические диски
	};

	struct DialogsOptions
	{
		BoolOption EditBlock;            // Постоянные блоки в строках ввода
		Bool3Option EditHistory;          // Добавлять в историю?
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
		// путь для поиска плагинов, указанный в /p
		string strCustomPluginsPath;
		string strPersonalPluginsPath;
		// true - использовать стандартный путь к основным плагинам
		bool MainPluginDir{};
		// set by '/co' switch, not saved
		bool PluginsCacheOnly{};
		bool PluginsPersonal{};

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
		BoolOption PreserveTimestamps;
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
		// параметры /m или /ma или /m....
		int DisableMacro{};
		// config
		StringOption strKeyMacroCtrlDot, strKeyMacroRCtrlDot; // аля KEY_CTRLDOT/KEY_RCTRLDOT
		StringOption strKeyMacroCtrlShiftDot, strKeyMacroRCtrlShiftDot; // аля KEY_CTRLSHIFTDOT/KEY_RCTRLSHIFTDOT
		// internal
		unsigned
			KeyMacroCtrlDot{},
			KeyMacroRCtrlDot{},
			KeyMacroCtrlShiftDot{},
			KeyMacroRCtrlShiftDot{};
		StringOption strDateFormat; // Для $Date
		BoolOption ShowPlayIndicator; // показать вывод 'P' во время проигрывания макроса
	};

	struct KnownModulesIDs
	{
		struct UuidOption
		{
			UUID Id{};
			StringOption StrId;
			string_view Default;
		}
		Network,
		Emenu,
		Arclite,
		Luamacro,
		Netbox,
		ProcList,
		TmpPanel;
	};

	struct ExecuteOptions
	{
		BoolOption RestoreCPAfterExecute;
		StringOption strExecuteBatchType;
		StringOption strExcludeCmds;
		std::vector<string_view> ExcludeCmds;
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
		BoolOption UseAssociations;
	};

	SortingOptions Sort;

	palette Palette;
	BoolOption SetPalette;

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
	BoolOption AllowReverseSort;
	BoolOption ReverseSortCharCompat;
	BoolOption SortFolderExt;
	BoolOption DeleteToRecycleBin;
	IntOption WipeSymbol; // символ заполнитель для "ZAP-операции"

	CopyMoveOptions CMOpt;

	DeleteOptions DelOpt;

	BoolOption MultiMakeDir; // Опция создания нескольких каталогов за один сеанс

	BoolOption UseRegisteredTypes;

	Bool3Option SaveViewHistory;
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

	Bool3Option SaveHistory;
	IntOption HistoryCount;
	IntOption HistoryLifetime;
	Bool3Option SaveFoldersHistory;
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

	BoolOption ShowPanelScrollbar;
	BoolOption ShowMenuScrollbar;
	Bool3Option ShowScreensNumber;
	BoolOption ShowSortMode;
	BoolOption ShowMenuBar;
	StringOption FormatNumberSeparators;

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
	IntOption IconIndex;
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
	IntOption CursorSize[4];
	IntOption GrabberCursorSize;

	CodeXLAT XLat;

	StringOption ConsoleDetachKey; // Комбинация клавиш для детача Far'овской консоли от длятельного неинтерактивного процесса в ней запущенного.

	StringOption strHelpLanguage;
	BoolOption FullScreenHelp;

	IntOption HelpURLRules; // =0 отключить возможность запуска URL-приложений

	// запоминать логические диски и не опрашивать каждый раз. Для предотвращения "просыпания" "зеленых" винтов.
	BoolOption RememberLogicalDrives;

	IntOption MsWheelDelta; // задает смещение для прокрутки
	IntOption MsWheelDeltaView;
	IntOption MsWheelDeltaEdit;
	IntOption MsWheelDeltaHelp;
	// горизонтальная прокрутка
	IntOption MsHWheelDelta;
	IntOption MsHWheelDeltaView;
	IntOption MsHWheelDeltaEdit;

	// How many ticks constitute an event
	IntOption MsWheelThreshold;

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

	BoolOption VirtualTerminalRendering;
	Bool3Option ClearType;
	Bool3Option FullWidthAwareRendering;

	Bool3Option PgUpChangeDisk;
	BoolOption ShowDotsInRoot;
	BoolOption ShowCheckingFile;
	BoolOption UpdateEnvironment;

	ExecuteOptions Exec;

	IntOption PluginMaxReadData;
	BoolOption ScanJunction;

	IntOption RedrawTimeout;

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
	// Если "-1", то в зависимости от CPMenuMode (Ctrl-H в меню кодовых страниц) фильтрация UCD либо будет
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

	int ReadOnlyConfig{-1};
	int UseExceptionHandler{};
	long long ElevationMode{};
	int WindowMode{-1};
	BoolOption WindowModeStickyX;
	BoolOption WindowModeStickyY;

	BoolOption ClipboardUnicodeWorkaround;

	std::vector<std::vector<std::pair<panel_sort, sort_order>>> PanelSortLayers;

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
	void AutoCompleteSettings();
	void TreeSettings();
	void SetViewSettings(size_t Index, PanelViewSettings&& Data);
	void AddViewSettings(size_t Index, PanelViewSettings&& Data);
	void DeleteViewSettings(size_t Index);
	void ReadPanelModes();
	void SavePanelModes(bool always);
	void SetDriveMenuHotkeys();
	void ReadSortLayers();
	void SaveSortLayers(bool Always);

	std::vector<farconfig> m_Configs;
	std::vector<PanelViewSettings> m_ViewSettings;
	bool m_ViewSettingsChanged{};
	bool m_HideUnchanged{};
};

string GetFarIniString(string_view AppName, string_view KeyName, string_view Default);
int GetFarIniInt(string_view AppName, string_view KeyName, int Default);

std::chrono::milliseconds GetRedrawTimeout() noexcept;

#endif // CONFIG_HPP_E468759B_688C_4D45_A5BA_CF1D4FCC9A08
