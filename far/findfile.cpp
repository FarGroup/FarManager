/*
findfile.cpp

Поиск (Alt-F7)
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

// BUGBUG
#include "platform.headers.hpp"

// Self:
#include "findfile.hpp"

// Internal:
#include "flink.hpp"
#include "keys.hpp"
#include "ctrlobj.hpp"
#include "dialog.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "fileview.hpp"
#include "fileedit.hpp"
#include "filelist.hpp"
#include "cmdline.hpp"
#include "namelist.hpp"
#include "scantree.hpp"
#include "manager.hpp"
#include "filemasks.hpp"
#include "filefilter.hpp"
#include "encoding.hpp"
#include "taskbar.hpp"
#include "interf.hpp"
#include "message.hpp"
#include "notification.hpp"
#include "delete.hpp"
#include "datetime.hpp"
#include "pathmix.hpp"
#include "exitcode.hpp"
#include "strmix.hpp"
#include "mix.hpp"
#include "constitle.hpp"
#include "uuids.far.dialogs.hpp"
#include "console.hpp"
#include "wakeful.hpp"
#include "panelmix.hpp"
#include "keyboard.hpp"
#include "plugins.hpp"
#include "lang.hpp"
#include "filestr.hpp"
#include "panelctype.hpp"
#include "filetype.hpp"
#include "diskmenu.hpp"
#include "string_utils.hpp"
#include "vmenu.hpp"
#include "exception_handler.hpp"
#include "drivemix.hpp"
#include "global.hpp"
#include "cvtname.hpp"
#include "log.hpp"
#include "stddlg.hpp"
#include "codepage.hpp"

// Platform:
#include "platform.hpp"
#include "platform.concurrency.hpp"
#include "platform.debug.hpp"
#include "platform.env.hpp"
#include "platform.fs.hpp"

// Common:
#include "common/bytes_view.hpp"
#include "common/enum_tokens.hpp"
#include "common/scope_exit.hpp"

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

namespace
{
	constexpr string_view AllFilesMask{ L"*.*"sv };

	const auto DM_REFRESH = DM_USER + 1;

	// Список архивов. Если файл найден в архиве, то FindList->ArcIndex указывает сюда.
	struct ArcListItem
	{
		MOVE_CONSTRUCTIBLE(ArcListItem);

		ArcListItem() = default;

		string strArcName;
		plugin_panel* hPlugin; // Plugin handle
		unsigned long long Flags; // OpenPanelInfo.Flags
		string strRootPath; // Root path in plugin after opening.
	};

	// Список найденных файлов. Индекс из списка хранится в меню.
	struct FindListItem
	{
		MOVE_CONSTRUCTIBLE(FindListItem);

		FindListItem() = default;

		os::fs::find_data FindData;
		ArcListItem* Arc{};
		DWORD Used{};
		UserDataItem UserData{};
	};

	namespace messages
	{
		struct menu_data
		{
			MOVE_CONSTRUCTIBLE(menu_data);

			menu_data(string_view FullName, const os::fs::find_data& FindData, UserDataItem const& UserData, ArcListItem* Arc):
				m_FindData(FindData),
				m_FullName(FullName),
				m_UserData(UserData),
				m_Arc(Arc)
			{}

			os::fs::find_data m_FindData;
			string m_FullName;
			UserDataItem m_UserData{};
			ArcListItem* m_Arc{};
		};

		struct push {};
		struct pop {};

		struct status
		{
			string Value;
		};

		struct percent
		{
			unsigned Value;
		};
	}

	using message = std::variant<
		messages::menu_data,
		messages::push,
		messages::pop,
		messages::status,
		messages::percent
	>;

	struct FindFilesOptions
	{
		bool NotContaining{};
		bool SearchInArchives{};
	};
}

class InterThreadData;

// BUGBUG Cleanup
class FindFiles: noncopyable
{
public:
	FindFiles();
	~FindFiles();

	const std::unique_ptr<filemasks>& GetFileMask() const { return FileMaskForFindFile; }
	const std::unique_ptr<multifilter>& GetFilter() const { return Filter; }
	static bool IsWordDiv(wchar_t symbol);
	// BUGBUG
	void AddMenuRecord(Dialog* Dlg, string_view FullName, const os::fs::find_data& FindData, UserDataItem const& UserData, ArcListItem* Arc);

	std::unique_ptr<InterThreadData> itd;
	os::synced_queue<message> m_Messages;
	std::queue<message> m_ExtractedMessages;
	string m_Status;
	unsigned m_Percent{};
	FindFilesOptions m_Options;
	SearchReplaceDlgParams m_SearchDlgParams;

	// BUGBUG
	[[nodiscard]]
	auto ScopedLock()
	{
		return make_raii_wrapper<
			&os::critical_section::lock,
			&os::critical_section::unlock
		>(&PluginCS);
	}

private:
	string &PrepareDriveNameStr(string &strSearchFromRoot) const;
	void AdvancedDialog() const;
	intptr_t MainDlgProc(Dialog* Dlg, intptr_t Msg, intptr_t Param1, void* Param2);
	intptr_t FindDlgProc(Dialog* Dlg, intptr_t Msg, intptr_t Param1, void* Param2);
	void OpenFile(string_view SearchFileName, int OpenKey, const FindListItem* FindItem, Dialog* Dlg) const;
	bool FindFilesProcess();
	void ProcessMessage(message& Message);
	void SetPluginDirectory(string_view DirName, const plugin_panel* hPlugin, bool UpdatePanel, const UserDataItem *UserData);
	bool GetPluginFile(ArcListItem const* ArcItem, const os::fs::find_data& FindData, const string& DestPath, string &strResultName, const UserDataItem* UserData);
	void stop_and_discard(Dialog* Dlg);
	FindListItem& AddFindListItem(const os::fs::find_data& FindData, UserDataItem const& UserData);
	void ClearFindList();

	static intptr_t AdvancedDlgProc(Dialog* Dlg, intptr_t Msg, intptr_t Param1, void* Param2);

	// BUGBUG
	bool AnySetFindList{};
	bool UseFilter{};
	bool FindFoldersChanged{};
	bool SearchFromChanged{};
	bool FindPositionChanged{};
	bool Finalized{};
	bool PluginMode{};
	FINDAREA SearchMode{ FINDAREA_ALL };
	int favoriteCodePages{};
	uintptr_t CodePage{ CP_DEFAULT };
	struct FindListItem* FindExitItem{};
	string m_FindMask;
	std::unique_ptr<filemasks> FileMaskForFindFile;
	std::unique_ptr<multifilter> Filter;

	std::unique_ptr<delayed_deleter> m_DelayedDeleter;

	int m_FileCount{};
	int m_DirCount{};
	int m_LastFoundNumber{};

	os::critical_section PluginCS;

	// BUGBUG
	class background_searcher* m_Searcher{};
	std::exception_ptr m_ExceptionPtr;
	std::stack<string> m_LastDir;
	string m_LastDirName;
	Dialog* m_ResultsDialogPtr{};
	bool m_EmptyArc{};

	time_check m_TimeCheck{ time_check::mode::immediate };
	os::concurrency::timer m_UpdateTimer;
	std::list<FindListItem> m_FindList;
	bool m_IsHexActive{};
	// This flag is set to true if the hotkey of either of Text/Hex radio is pressed.
	// If true, the handler of Text/Hex radio button click will set focus on the Text/Hex editor
	bool m_IsTextOrHexHotkeyUsed{};
};

// TODO BUGBUG DELETE THIS
class InterThreadData
{
private:
	mutable os::critical_section DataCS;

	std::list<ArcListItem> ArcList;

public:
	InterThreadData() {Init();}
	~InterThreadData() { ClearAllLists(); }

	void Init()
	{
		SCOPED_ACTION(std::scoped_lock)(DataCS);
		ArcList.clear();
	}

	void ClearAllLists()
	{
		SCOPED_ACTION(std::scoped_lock)(DataCS);
		ArcList.clear();
	}

	ArcListItem& AddArcListItem(string_view const ArcName, plugin_panel* const hPlugin, unsigned long long const Flags, string_view const RootPath)
	{
		SCOPED_ACTION(std::scoped_lock)(DataCS);

		ArcListItem NewItem;
		NewItem.strArcName = ArcName;
		NewItem.hPlugin = hPlugin;
		NewItem.Flags = Flags;
		NewItem.strRootPath = RootPath;
		AddEndSlash(NewItem.strRootPath);

		ArcList.emplace_back(std::move(NewItem));
		return ArcList.back();
	}
};


enum
{
	FIND_EXIT_NONE,
	FIND_EXIT_SEARCHAGAIN,
	FIND_EXIT_GOTO,
	FIND_EXIT_PANEL
};

enum ADVANCEDDLG
{
	AD_DOUBLEBOX,
	AD_TEXT_SEARCHFIRST,
	AD_EDIT_SEARCHFIRST,
	AD_SEPARATOR1,
	AD_TEXT_COLUMNSFORMAT,
	AD_EDIT_COLUMNSFORMAT,
	AD_TEXT_COLUMNSWIDTH,
	AD_EDIT_COLUMNSWIDTH,
	AD_SEPARATOR2,
	AD_BUTTON_OK,
	AD_BUTTON_CANCEL,

	AD_COUNT
};

enum FINDASKDLG
{
	FAD_DOUBLEBOX,
	FAD_TEXT_MASK,
	FAD_EDIT_MASK,
	FAD_SEPARATOR0,
	FAD_TEXT_CONTAINING,
	FAD_EDIT_TEXT,
	FAD_EDIT_HEX,
	FAD_RADIO_TEXT,
	FAD_RADIO_HEX,
	FAD_TEXT_CP,
	FAD_COMBOBOX_CP,
	FAD_SEPARATOR1,
	FAD_CHECKBOX_CASE,
	FAD_CHECKBOX_WHOLEWORDS,
	FAD_CHECKBOX_FUZZY,
	FAD_CHECKBOX_NOTCONTAINING,
	FAD_CHECKBOX_ARC,
	FAD_CHECKBOX_DIRS,
	FAD_CHECKBOX_LINKS,
	FAD_CHECKBOX_STREAMS,
	FAD_SEPARATOR_2,
	FAD_SEPARATOR_3,
	FAD_TEXT_WHERE,
	FAD_COMBOBOX_WHERE,
	FAD_CHECKBOX_FILTER,
	FAD_SEPARATOR_4,
	FAD_BUTTON_FIND,
	FAD_BUTTON_DRIVE,
	FAD_BUTTON_FILTER,
	FAD_BUTTON_ADVANCED,
	FAD_BUTTON_CANCEL,

	FAD_COUNT
};

enum FINDDLG
{
	FD_DOUBLEBOX,
	FD_LISTBOX,
	FD_SEPARATOR1,
	FD_TEXT_STATUS,
	FD_TEXT_STATUS_PERCENTS,
	FD_SEPARATOR2,
	FD_BUTTON_NEW,
	FD_BUTTON_GOTO,
	FD_BUTTON_VIEW,
	FD_BUTTON_PANEL,
	FD_BUTTON_STOP,

	FD_COUNT
};

class background_searcher: noncopyable
{
public:
	background_searcher(
		FindFiles* Owner,
		FINDAREA SearchMode,
		ArcListItem* FindFileArcItem,
		uintptr_t CodePage,
		unsigned long long SearchInFirst,
		bool UseFilter,
		bool PluginMode
	);

	void Search();
	void Pause() const { PauseEvent.reset(); }
	void Resume() const { PauseEvent.set(); }
	void Stop() const { PauseEvent.set(); StopEvent.set(); }
	bool Stopped() const { return StopEvent.is_signaled(); }
	bool Finished() const { return m_Finished || m_ExceptionPtr || m_SehException.is_signaled(); }

	const auto& ExceptionPtr() const { return m_ExceptionPtr; }
	auto& SehException() { return m_SehException; }

private:
	void InitInFileSearch();
	void ReleaseInFileSearch();

	bool LookForString(string_view FileName);
	bool IsFileIncluded(PluginPanelItem* FileItem, string_view FullName, os::fs::attributes FileAttr, string_view DisplayName);
	void DoPrepareFileList();
	void DoPreparePluginListImpl();
	void DoPreparePluginList();
	void ArchiveSearch(string_view ArcName);
	void DoScanTree(string_view strRoot);
	void ScanPluginTree(plugin_panel* hPlugin, unsigned long long Flags, int& RecurseLevel);
	void AddMenuRecord(string_view FullName, PluginPanelItem& FindData) const;

	FindFiles* const m_Owner;
	const string m_EventName;

	std::vector<std::byte> readBufferA;
	std::vector<wchar_t> readBuffer;
	struct CodePageInfo;
	std::vector<CodePageInfo> m_CodePages;
	string strPluginSearchPath;

	bool m_Autodetection;

	const FINDAREA SearchMode;
	ArcListItem* m_FindFileArcItem;
	const uintptr_t CodePage;
	size_t m_MaxCharSize{};
	const unsigned long long SearchInFirst;
	const FindFilesOptions& m_Options;
	const SearchReplaceDlgParams& m_SearchDlgParams;
	const bool UseFilter;
	const bool m_PluginMode;

	os::event PauseEvent;
	os::event StopEvent;
	std::atomic_bool m_Finished{};

	std::exception_ptr m_ExceptionPtr;
	seh_exception m_SehException;

	searchers m_TextSearchers;
	i_searcher const* m_TextSearcher;

	using hex_searcher = std::boyer_moore_horspool_searcher<bytes::const_iterator>;
	std::optional<hex_searcher> m_HexSearcher;

	std::optional<taskbar::indeterminate> m_TaskbarProgress{ std::in_place };
};

struct background_searcher::CodePageInfo
{
	explicit CodePageInfo(uintptr_t CodePage):
		CodePage(CodePage)
	{
	}

	uintptr_t CodePage;
	size_t MaxCharSize{};
	wchar_t LastSymbol{};
	bool WordFound{};
	size_t BytesToSkip{};

	void initialize()
	{
		if (IsUtf16CodePage(CodePage))
			MaxCharSize = 2;
		else
		{
			CPINFO cpi;

			if (!GetCPInfo(CodePage, &cpi))
				cpi.MaxCharSize = 0; //Считаем, что ошибка и потом такие таблицы в поиске пропускаем

			MaxCharSize = cpi.MaxCharSize;
		}

		LastSymbol = 0;
		WordFound = false;
	}
};

void background_searcher::InitInFileSearch()
{
	if (m_SearchDlgParams.IsSearchPatternEmpty())
		return;

	// Инициализируем буферы чтения из файла
	const size_t readBufferSize = 65536;

	readBufferA.resize(readBufferSize);
	readBuffer.resize(readBufferSize);

	if (!m_SearchDlgParams.Hex.value())
	{
		m_TextSearcher = &init_searcher(m_TextSearchers, m_SearchDlgParams.CaseSensitive.value(), m_SearchDlgParams.Fuzzy.value(), m_SearchDlgParams.SearchStr, false);

		// Формируем список кодовых страниц
		if (CodePage == CP_ALL)
		{
			// Проверяем наличие выбранных страниц символов
			const auto CpEnum = codepages::GetFavoritesEnumerator();
			const auto hasSelected = std::ranges::any_of(CpEnum, [](auto const& i){ return (i.second & CPST_FIND) != 0; });

			if (hasSelected)
			{
				m_CodePages.clear();
			}
			else
			{
				// system codepages

				// Windows 10-specific madness
				const auto AnsiCp = encoding::codepage::ansi();
				if (AnsiCp != CP_UTF8)
				{
					m_CodePages.emplace_back(AnsiCp);
				}

				const auto OemCp = encoding::codepage::oem();
				if (OemCp != AnsiCp && OemCp != CP_UTF8)
				{
					m_CodePages.emplace_back(OemCp);
				}

				m_CodePages.emplace_back(CP_UTF8);
				m_CodePages.emplace_back(CP_UTF16LE);
				m_CodePages.emplace_back(CP_UTF16BE);
			}

			// Добавляем избранные таблицы символов
			for (const auto [Name, Value]: CpEnum)
			{
				if (Value & (hasSelected? CPST_FIND : CPST_FAVORITE))
				{
					// Проверяем дубли
					if (hasSelected || std::ranges::find(m_CodePages, Name, &CodePageInfo::CodePage) == m_CodePages.cend())
						m_CodePages.emplace_back(Name);
				}
			}
		}
		else
		{
			m_CodePages.emplace_back(CodePage);
			m_Autodetection = CodePage == CP_DEFAULT;
		}

		for (auto& i: m_CodePages)
		{
			i.initialize();
			m_MaxCharSize = std::max(m_MaxCharSize, i.MaxCharSize);
		}
	}
	else
	{
		// Инициализируем данные для аглоритма поиска
		m_HexSearcher.emplace(ALL_CONST_RANGE(m_SearchDlgParams.SearchBytes.value()));
	}
}

void background_searcher::ReleaseInFileSearch()
{
	clear_and_shrink(readBufferA);
	clear_and_shrink(readBuffer);
	m_CodePages.clear();
}

string& FindFiles::PrepareDriveNameStr(string &strSearchFromRoot) const
{
	auto strCurDir = GetPathRoot(Global->CtrlObject->CmdLine()->GetCurDir());
	DeleteEndSlash(strCurDir);

	if (
		strCurDir.empty()||
		(Global->CtrlObject->Cp()->ActivePanel()->GetMode() == panel_mode::PLUGIN_PANEL && Global->CtrlObject->Cp()->ActivePanel()->IsVisible())
	)
	{
		strSearchFromRoot = msg(lng::MFindFileSearchFromRootFolder);
	}
	else
	{
		strSearchFromRoot = concat(msg(lng::MFindFileSearchFromRootOfDrive), L' ', strCurDir);
	}

	return strSearchFromRoot;
}

// Проверяем символ на принадлежность разделителям слов
bool FindFiles::IsWordDiv(const wchar_t symbol)
{
	// Также разделителем является конец строки и пробельные символы
	return !symbol || std::iswspace(symbol) || ::IsWordDiv(Global->Opt->strWordDiv,symbol);
}

void FindFiles::SetPluginDirectory(string_view const DirName, const plugin_panel* const hPlugin, bool const UpdatePanel, const UserDataItem* const UserData)
{
	if (!DirName.empty())
	{
		//const wchar_t* DirPtr = ;
		const auto NamePtr = PointToName(DirName);

		if (NamePtr.size() != DirName.size())
		{
			const auto Dir = DeleteEndSlash(DirName.substr(0, DirName.size() - NamePtr.size()));

			// force plugin to update its file list (that can be empty at this time)
			// if not done SetDirectory may fail
			{
				std::span<PluginPanelItem> PanelData;

				SCOPED_ACTION(std::scoped_lock)(PluginCS);
				if (Global->CtrlObject->Plugins->GetFindData(hPlugin, PanelData, OPM_SILENT))
					Global->CtrlObject->Plugins->FreeFindData(hPlugin, PanelData, true);
			}

			SCOPED_ACTION(std::scoped_lock)(PluginCS);
			Global->CtrlObject->Plugins->SetDirectory(hPlugin, Dir.empty()? L"\\"s : string(Dir), OPM_SILENT, Dir.empty()? nullptr : UserData);
		}

		// Отрисуем панель при необходимости.
		if (UpdatePanel)
		{
			Global->CtrlObject->Cp()->ActivePanel()->Update(UPDATE_KEEP_SELECTION);
			Global->CtrlObject->Cp()->ActivePanel()->GoToFile(NamePtr);
			Global->CtrlObject->Cp()->ActivePanel()->Show();
		}
	}
}

intptr_t FindFiles::AdvancedDlgProc(Dialog* Dlg, intptr_t Msg, intptr_t Param1, void* Param2)
{
	switch (Msg)
	{
		case DN_CLOSE:

			if (Param1==AD_BUTTON_OK)
			{
				const auto Data = std::bit_cast<const wchar_t*>(Dlg->SendMessage(DM_GETCONSTTEXTPTR, AD_EDIT_SEARCHFIRST, nullptr));

				if (Data && *Data && !CheckFileSizeStringFormat(Data))
				{
					Message(MSG_WARNING,
						msg(lng::MFindFileAdvancedTitle),
						{
							msg(lng::MBadFileSizeFormat)
						},
						{ lng::MOk });
					return FALSE;
				}
			}

			break;
		default:
			break;
	}

	return Dlg->DefProc(Msg,Param1,Param2);
}

void FindFiles::AdvancedDialog() const
{
	auto AdvancedDlg = MakeDialogItems<AD_COUNT>(
	{
		{ DI_DOUBLEBOX, {{3,  1 }, {52, 11}}, DIF_NONE, msg(lng::MFindFileAdvancedTitle), },
		{ DI_TEXT,      {{5,  2 }, {0,  2 }}, DIF_NONE, msg(lng::MFindFileSearchFirst), },
		{ DI_EDIT,      {{5,  3 }, {50, 3 }}, DIF_NONE, Global->Opt->FindOpt.strSearchInFirstSize, },
		{ DI_TEXT,      {{-1, 4 }, {0,  4 }}, DIF_SEPARATOR, },
		{ DI_TEXT,      {{5,  5 }, {0,  5 }}, DIF_NONE, msg(lng::MFindAlternateModeTypes), },
		{ DI_EDIT,      {{5,  6 }, {50, 6 }}, DIF_NONE, Global->Opt->FindOpt.strSearchOutFormat, },
		{ DI_TEXT,      {{5,  7 }, {0,  7 }}, DIF_NONE, msg(lng::MFindAlternateModeWidths), },
		{ DI_EDIT,      {{5,  8 }, {50, 8 }}, DIF_NONE, Global->Opt->FindOpt.strSearchOutFormatWidth, },
		{ DI_TEXT,      {{-1, 9 }, {0,  9 }}, DIF_SEPARATOR, },
		{ DI_BUTTON,    {{0,  10}, {0,  10}}, DIF_CENTERGROUP | DIF_DEFAULTBUTTON, msg(lng::MOk), },
		{ DI_BUTTON,    {{0,  10}, {0,  10}}, DIF_CENTERGROUP, msg(lng::MCancel), },
	});

	const auto Dlg = Dialog::create(AdvancedDlg, &FindFiles::AdvancedDlgProc);
	Dlg->SetHelp(L"FindFileAdvanced"sv);
	Dlg->SetPosition({ -1, -1, 52 + 4, 13 });
	Dlg->Process();

	if (Dlg->GetExitCode() == AD_BUTTON_OK)
	{
		Global->Opt->FindOpt.strSearchInFirstSize = AdvancedDlg[AD_EDIT_SEARCHFIRST].strData;
		Global->Opt->SetSearchColumns(AdvancedDlg[AD_EDIT_COLUMNSFORMAT].strData, AdvancedDlg[AD_EDIT_COLUMNSWIDTH].strData);
	}
}

intptr_t FindFiles::MainDlgProc(Dialog* Dlg, intptr_t Msg, intptr_t Param1, void* Param2)
{
	const auto SetAllCpTitle = [&]
	{
		const int TitlePosition = 1;
		const auto CpEnum = codepages::GetFavoritesEnumerator();
		const auto Title = msg(std::ranges::any_of(CpEnum, [](auto const& i){ return i.second & CPST_FIND; })? lng::MFindFileSelectedCodePages : lng::MFindFileAllCodePages);
		Dlg->GetAllItem()[FAD_COMBOBOX_CP].ListPtr->at(TitlePosition).Name = Title;
		FarListPos Position{ sizeof(Position) };
		Dlg->SendMessage(DM_LISTGETCURPOS, FAD_COMBOBOX_CP, &Position);
		if (Position.SelectPos == TitlePosition)
			Dlg->SendMessage(DM_SETTEXTPTR, FAD_COMBOBOX_CP, UNSAFE_CSTR(Title));
	};

	switch (Msg)
	{
		case DN_INITDIALOG:
		{
			const auto Hex{ Dlg->SendMessage(DM_GETCHECK, FAD_RADIO_HEX, nullptr) == BSTATE_CHECKED };
			Dlg->SendMessage(DM_SHOWITEM,FAD_EDIT_TEXT,ToPtr(!Hex));
			Dlg->SendMessage(DM_SHOWITEM,FAD_EDIT_HEX,ToPtr(Hex));
			Dlg->SendMessage(DM_ENABLE,FAD_TEXT_CP,ToPtr(!Hex));
			Dlg->SendMessage(DM_ENABLE,FAD_COMBOBOX_CP,ToPtr(!Hex));
			Dlg->SendMessage(DM_ENABLE,FAD_CHECKBOX_CASE,ToPtr(!Hex));
			Dlg->SendMessage(DM_ENABLE,FAD_CHECKBOX_WHOLEWORDS,ToPtr(!Hex));
			Dlg->SendMessage(DM_ENABLE,FAD_CHECKBOX_FUZZY,ToPtr(!Hex));
			Dlg->SendMessage(DM_EDITUNCHANGEDFLAG,FAD_EDIT_TEXT,ToPtr(1));
			Dlg->SendMessage(DM_EDITUNCHANGEDFLAG,FAD_EDIT_HEX,ToPtr(1));
			Dlg->SendMessage(DM_SETTEXTPTR,FAD_TEXT_CP,const_cast<wchar_t*>(msg(lng::MFindFileCodePage).c_str()));
			Dlg->SendMessage(DM_SETCOMBOBOXEVENT,FAD_COMBOBOX_CP,ToPtr(CBET_KEY));
			const auto BottomLine = KeysToLocalizedText(KEY_SPACE, KEY_INS);
			FarListTitles Titles{ sizeof(Titles), 0, nullptr, 0, BottomLine.c_str() };
			Dlg->SendMessage(DM_LISTSETTITLES,FAD_COMBOBOX_CP,&Titles);
			// Установка запомненных ранее параметров
			CodePage = Global->Opt->FindCodePage;
			favoriteCodePages = static_cast<int>(codepages::instance().FillCodePagesList(Dlg, FAD_COMBOBOX_CP, CodePage, true, true, false, true, false));
			SetAllCpTitle();

			// Текущее значение в списке выбора кодовых страниц в общем случае может не совпадать с CodePage,
			// так что получаем CodePage из списка выбора
			FarListPos Position{ sizeof(Position) };
			Dlg->SendMessage( DM_LISTGETCURPOS, FAD_COMBOBOX_CP, &Position);
			FarListGetItem Item{ sizeof(Item), Position.SelectPos };
			Dlg->SendMessage( DM_LISTGETITEM, FAD_COMBOBOX_CP, &Item);
			CodePage = Item.Item.UserData;
			return TRUE;
		}
		case DN_CLOSE:
		{
			switch (Param1)
			{
				case FAD_BUTTON_FIND:
				{
					string Mask(std::bit_cast<const wchar_t*>(Dlg->SendMessage(DM_GETCONSTTEXTPTR, FAD_EDIT_MASK, nullptr)));

					if (Mask.empty())
						Mask = AllFilesMask;

					return FileMaskForFindFile->assign(Mask);
				}
				case FAD_BUTTON_DRIVE:
				{
					ChangeDisk(Global->CtrlObject->Cp()->ActivePanel());
					// Ну что ж, раз пошла такая пьянка рефрешить окна
					// будем таким способом.
					Global->WindowManager->ResizeAllWindows();
					string strSearchFromRoot;
					PrepareDriveNameStr(strSearchFromRoot);
					FarListGetItem item{ sizeof(item), FINDAREA_ROOT };
					Dlg->SendMessage(DM_LISTGETITEM,FAD_COMBOBOX_WHERE,&item);
					item.Item.Text=strSearchFromRoot.c_str();
					Dlg->SendMessage(DM_LISTUPDATE,FAD_COMBOBOX_WHERE,&item);
					PluginMode = Global->CtrlObject->Cp()->ActivePanel()->GetMode() == panel_mode::PLUGIN_PANEL;
					item.ItemIndex = FINDAREA_ROOT;
					Dlg->SendMessage(DM_LISTGETITEM,FAD_COMBOBOX_WHERE,&item);

					if (PluginMode)
						item.Item.Flags|=LIF_GRAYED;
					else
						item.Item.Flags&=~LIF_GRAYED;

					Dlg->SendMessage(DM_LISTUPDATE,FAD_COMBOBOX_WHERE,&item);
					item.ItemIndex = FINDAREA_ALL_BUTNETWORK;
					Dlg->SendMessage(DM_LISTGETITEM,FAD_COMBOBOX_WHERE,&item);

					if (PluginMode)
						item.Item.Flags|=LIF_GRAYED;
					else
						item.Item.Flags&=~LIF_GRAYED;

					Dlg->SendMessage(DM_LISTUPDATE,FAD_COMBOBOX_WHERE,&item);
				}
				break;
				case FAD_BUTTON_FILTER:
					filters::EditFilters(Filter->area(), Filter->panel());
					break;
				case FAD_BUTTON_ADVANCED:
					AdvancedDialog();
					break;
				case -2:
				case -1:
				case FAD_BUTTON_CANCEL:
					return TRUE;
			}

			return FALSE;
		}
		case DN_BTNCLICK:
		{
			switch (Param1)
			{
				case FAD_RADIO_TEXT:
				case FAD_RADIO_HEX:
				{
					if (!Param2) break;

					SCOPED_ACTION(Dialog::suppress_redraw)(Dlg);

					const auto NewHex{ Param1 == FAD_RADIO_HEX };

					if (NewHex != m_IsHexActive)
					{
						m_IsHexActive = NewHex;

						const auto strDataStr = ConvertHexString(get_dialog_item_text(Dlg, NewHex? FAD_EDIT_TEXT : FAD_EDIT_HEX), CodePage, !NewHex);
						set_dialog_item_text(Dlg, NewHex? FAD_EDIT_HEX : FAD_EDIT_TEXT, strDataStr);

						Dlg->SendMessage(DM_SHOWITEM, FAD_EDIT_TEXT, ToPtr(!NewHex));
						Dlg->SendMessage(DM_SHOWITEM, FAD_EDIT_HEX, ToPtr(NewHex));
						Dlg->SendMessage(DM_ENABLE, FAD_TEXT_CP, ToPtr(!NewHex));
						Dlg->SendMessage(DM_ENABLE, FAD_COMBOBOX_CP, ToPtr(!NewHex));
						Dlg->SendMessage(DM_ENABLE, FAD_CHECKBOX_CASE, ToPtr(!NewHex));
						Dlg->SendMessage(DM_ENABLE, FAD_CHECKBOX_WHOLEWORDS, ToPtr(!NewHex));
						Dlg->SendMessage(DM_ENABLE, FAD_CHECKBOX_FUZZY, ToPtr(!NewHex));

						if (!strDataStr.empty())
						{
							const auto UnchangeFlag = static_cast<int>(Dlg->SendMessage(DM_EDITUNCHANGEDFLAG, NewHex ? FAD_EDIT_TEXT : FAD_EDIT_HEX, ToPtr(-1)));
							Dlg->SendMessage(DM_EDITUNCHANGEDFLAG, NewHex ? FAD_EDIT_HEX : FAD_EDIT_TEXT, ToPtr(UnchangeFlag));
						}
					}

					if (m_IsTextOrHexHotkeyUsed)
					{
						m_IsTextOrHexHotkeyUsed = false;
						Dlg->SendMessage(DM_SETFOCUS, NewHex ? FAD_EDIT_HEX : FAD_EDIT_TEXT, nullptr);
					}
				}
				break;
			}

			break;
		}
		case DN_CONTROLINPUT:
		{
			const auto record = static_cast<const INPUT_RECORD*>(Param2);
			if (record->EventType!=KEY_EVENT) break;
			int key = InputRecordToKey(record);
			switch (Param1)
			{
				case FAD_COMBOBOX_CP:
				{
					switch (key)
					{
						case KEY_INS:
						case KEY_NUMPAD0:
						case KEY_SPACE:
						{
							// Обработка установки/снятия флажков для стандартных и избранных таблиц символов
							// Получаем текущую позицию в выпадающем списке таблиц символов
							FarListPos Position{ sizeof(Position) };
							Dlg->SendMessage( DM_LISTGETCURPOS, FAD_COMBOBOX_CP, &Position);
							// Получаем номер выбранной таблицы символов
							FarListGetItem Item{ sizeof(Item), Position.SelectPos };
							Dlg->SendMessage( DM_LISTGETITEM, FAD_COMBOBOX_CP, &Item);
							const auto SelectedCodePage = Item.Item.UserData;
							// Разрешаем отмечать только стандартные и избранные таблицы символов
							int FavoritesIndex = 2 + StandardCPCount + 2;

							if (Position.SelectPos > 1)
							{
								// Получаем текущее состояние флага в реестре
								long long SelectType = codepages::GetFavorite(SelectedCodePage);

								// Отмечаем/разотмечаем таблицу символов
								if (Item.Item.Flags & LIF_CHECKED)
								{
									// Для стандартных таблиц символов просто удаляем значение из реестра, для
									// избранных же оставляем в реестре флаг, что таблица символов избранная
									if (SelectType & CPST_FAVORITE)
										codepages::SetFavorite(SelectedCodePage, CPST_FAVORITE);
									else
										codepages::DeleteFavorite(SelectedCodePage);

									Item.Item.Flags &= ~LIF_CHECKED;
								}
								else
								{
									codepages::SetFavorite(SelectedCodePage, CPST_FIND | (SelectType & CPST_FAVORITE ? CPST_FAVORITE : 0));
									Item.Item.Flags |= LIF_CHECKED;
								}

								SetAllCpTitle();

								// Обновляем текущий элемент в выпадающем списке
								Dlg->SendMessage( DM_LISTUPDATE, FAD_COMBOBOX_CP, &Item);

								FarListPos Pos{ sizeof(Pos), Position.SelectPos + 1, Position.TopPos };
								Dlg->SendMessage( DM_LISTSETCURPOS, FAD_COMBOBOX_CP,&Pos);

								// Обрабатываем случай, когда таблица символов может присутствовать, как в стандартных, так и в избранных,
								// т.е. выбор/снятие флага автоматически происходит у обоих элементов
								bool bStandardCodePage = Position.SelectPos < FavoritesIndex;

								for (int Index = bStandardCodePage ? FavoritesIndex : 0; Index < (bStandardCodePage ? FavoritesIndex + favoriteCodePages : FavoritesIndex); Index++)
								{
									// Получаем элемент таблицы символов
									FarListGetItem CheckItem{ sizeof(CheckItem), Index };
									Dlg->SendMessage( DM_LISTGETITEM, FAD_COMBOBOX_CP, &CheckItem);

									// Обрабатываем только таблицы символов
									if (!(CheckItem.Item.Flags&LIF_SEPARATOR))
									{
										if (SelectedCodePage == CheckItem.Item.UserData)
										{
											if (Item.Item.Flags & LIF_CHECKED)
												CheckItem.Item.Flags |= LIF_CHECKED;
											else
												CheckItem.Item.Flags &= ~LIF_CHECKED;

											Dlg->SendMessage( DM_LISTUPDATE, FAD_COMBOBOX_CP, &CheckItem);
											break;
										}
									}
								}
							}
						}
						break;
					}
				}
				break;
			}

			break;
		}
		case DN_EDITCHANGE:
		{
			switch (Param1)
			{
				case FAD_COMBOBOX_CP:
					// Получаем выбранную в выпадающем списке таблицу символов
					CodePage = Dlg->GetListItemSimpleUserData(FAD_COMBOBOX_CP, Dlg->SendMessage(DM_LISTGETCURPOS, FAD_COMBOBOX_CP, nullptr));
					return TRUE;

				case FAD_COMBOBOX_WHERE:
					SearchFromChanged = true;
					return TRUE;
			}
			break;
		}
		case DN_HOTKEY:
			m_IsTextOrHexHotkeyUsed = Param1 == FAD_RADIO_TEXT || Param1 == FAD_RADIO_HEX;
			break;

		default:
			break;
	}

	return Dlg->DefProc(Msg,Param1,Param2);
}

bool FindFiles::GetPluginFile(ArcListItem const* const ArcItem, const os::fs::find_data& FindData, const string& DestPath, string &strResultName, const UserDataItem* const UserData)
{
	SCOPED_ACTION(std::scoped_lock)(PluginCS);
	OpenPanelInfo Info;

	Global->CtrlObject->Plugins->GetOpenPanelInfo(ArcItem->hPlugin,&Info);
	string strSaveDir = NullToEmpty(Info.CurDir);
	AddEndSlash(strSaveDir);
	Global->CtrlObject->Plugins->SetDirectory(ArcItem->hPlugin, L"\\"s, OPM_SILENT);
	SetPluginDirectory(FindData.FileName,ArcItem->hPlugin,false,UserData);
	const auto FileNameToFind = PointToName(FindData.FileName);
	const auto FileNameToFindShort = FindData.HasAlternateFileName()? PointToName(FindData.AlternateFileName()) : string_view{};
	std::span<PluginPanelItem> Items;
	bool nResult=false;

	if (Global->CtrlObject->Plugins->GetFindData(ArcItem->hPlugin, Items, OPM_SILENT))
	{
		const auto It = std::ranges::find_if(Items, [&](const auto& Item)
		{
			return FileNameToFind == NullToEmpty(Item.FileName) && FileNameToFindShort == NullToEmpty(Item.AlternateFileName);
		});

		if (It != Items.end())
		{
			nResult = Global->CtrlObject->Plugins->GetFile(ArcItem->hPlugin, std::to_address(It), DestPath, strResultName, OPM_SILENT) != 0;
		}

		Global->CtrlObject->Plugins->FreeFindData(ArcItem->hPlugin, Items, true);
	}

	Global->CtrlObject->Plugins->SetDirectory(ArcItem->hPlugin, L"\\"s, OPM_SILENT);
	SetPluginDirectory(strSaveDir, ArcItem->hPlugin, false, nullptr);
	return nResult;
}

bool background_searcher::LookForString(string_view const FileName)
{
	const os::fs::file File(FileName, FILE_READ_DATA, os::fs::file_share_all, nullptr, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN);
	if(!File)
	{
		return false;
	}

	if (m_Autodetection)
	{
		m_CodePages.front().CodePage = GetFileCodepage(File, encoding::codepage::ansi());
		m_CodePages.front().initialize();
		m_MaxCharSize = m_CodePages.front().MaxCharSize;
	}

	// Количество считанных из файла байт
	size_t readBlockSize = 0;
	// Количество прочитанных из файла байт
	unsigned long long alreadyRead = 0;

	// Step back a little more just in case (lengths of the pattern and the actually found string could be different)
	const auto ArbitraryOverlapBytesCountJustInCase = 128;
	static_assert(!(ArbitraryOverlapBytesCountJustInCase & 1));

	// Смещение на которое мы отступили при переходе между блоками
	const intptr_t StepBackOffset = m_SearchDlgParams.Hex.value()?
		m_SearchDlgParams.SearchBytes.value().size() - 1 :
		(m_MaxCharSize * m_SearchDlgParams.SearchStr.size()) + ArbitraryOverlapBytesCountJustInCase;

	unsigned long long FileSize = 0;
	// BUGBUG check result
	if (!File.GetSize(FileSize))
	{
		LOGWARNING(L"GetSize({}): {}"sv, File.GetName(), os::last_error());
	}

	if (SearchInFirst)
	{
		FileSize=std::min(SearchInFirst,FileSize);
	}

	unsigned LastPercents{};

	// Основной цикл чтения из файла
	while (!Stopped() && File.Read(readBufferA.data(), (!SearchInFirst || alreadyRead + readBufferA.size() <= SearchInFirst)? readBufferA.size() : SearchInFirst - alreadyRead, readBlockSize))
	{
		const auto IsLastBlock = readBlockSize < readBuffer.size();

		const auto Percents = ToPercent(alreadyRead, FileSize);

		if (Percents!=LastPercents)
		{
			m_Owner->m_Messages.emplace(messages::percent{ Percents });
			LastPercents=Percents;
		}

		// Увеличиваем счётчик прочитаннх байт
		alreadyRead += readBlockSize;

		// Для hex и обыкновенного поиска разные ветки
		if (m_SearchDlgParams.Hex.value())
		{
			// Выходим, если прочитали мало
			if (readBlockSize < m_SearchDlgParams.SearchBytes.value().size())
				return false;

			// Ищем
			const auto Begin = readBufferA.cbegin(), End = Begin + readBlockSize;
			if (std::search(Begin, End, *m_HexSearcher) != End)
				return true;
		}
		else
		{
			bool ErrorState = false;
			for (auto& i: m_CodePages)
			{
				if (alreadyRead == readBlockSize)
					i.BytesToSkip = 0;

				ErrorState = false;
				// Пропускаем ошибочные кодовые страницы
				if (!i.MaxCharSize)
				{
					ErrorState = true;
					continue;
				}

				// Если начало файла очищаем информацию о поиске по словам
				if (m_SearchDlgParams.WholeWords.value() && alreadyRead == readBlockSize)
				{
					i.WordFound = false;
					i.LastSymbol = 0;
				}

				// Если ничего не прочитали
				if (!readBlockSize)
				{
					// Если поиск по словам и в конце предыдущего блока было что-то найдено,
					// то считаем, что нашли то, что нужно
					if(m_SearchDlgParams.WholeWords.value() && i.WordFound)
						return true;
					else
					{
						ErrorState = true;
						continue;
					}
					// Выходим, если прочитали меньше размера строки поиска и нет поиска по словам
				}

				if (readBlockSize < m_SearchDlgParams.SearchStr.size() && !(m_SearchDlgParams.WholeWords.value() && i.WordFound))
				{
					ErrorState = true;
					continue;
				}

				// Количество символов в выходном буфере
				size_t bufferCount;

				// Буфер для поиска
				wchar_t const *buffer;

				// Перегоняем буфер в UTF-16
				if (IsUtf16CodePage(i.CodePage))
				{
					// Вычисляем размер буфера в UTF-16
					bufferCount = readBlockSize/sizeof(wchar_t);

					// Выходим, если размер буфера меньше длины строки поиска
					if (bufferCount < m_SearchDlgParams.SearchStr.size())
					{
						ErrorState = true;
						continue;
					}

					// Копируем буфер чтения в буфер сравнения
					if (i.CodePage== CP_UTF16BE)
					{
						// Для UTF-16 (big endian) преобразуем буфер чтения в буфер сравнения
						static_assert(std::endian::native == std::endian::little, "No way");
						const auto EvenSize = readBlockSize / sizeof(char16_t) * sizeof(char16_t);
						swap_bytes(readBufferA.data(), readBuffer.data(), EvenSize, sizeof(char16_t));
						if (readBlockSize & 1)
						{
							readBuffer[EvenSize / sizeof(char16_t)] = make_integer<char16_t>('\0', static_cast<char>(readBufferA[readBlockSize - 1]));
							++bufferCount;
						}
						// Устанавливаем буфер сравнения
						buffer = readBuffer.data();
					}
					else
					{
						// Если поиск в UTF-16 (little endian), то используем исходный буфер
						buffer = std::bit_cast<wchar_t*>(readBufferA.data());
					}
				}
				else
				{
					// Конвертируем буфер чтения из кодировки поиска в UTF-16
					encoding::diagnostics Diagnostics{ encoding::diagnostics::not_enough_data };
					bufferCount = encoding::get_chars(i.CodePage, { readBufferA.data() + i.BytesToSkip, readBlockSize - i.BytesToSkip }, readBuffer, &Diagnostics);

					// Выходим, если нам не удалось сконвертировать строку
					if (!bufferCount)
					{
						ErrorState = true;
						continue;
					}

					if (!IsLastBlock)
						bufferCount -= Diagnostics.PartialOutput;

					// Если у нас поиск по словам и в конце предыдущего блока было вхождение
					if (m_SearchDlgParams.WholeWords.value() && i.WordFound)
					{
						// Если конец файла, то считаем, что есть разделитель в конце
						if (bufferCount < m_SearchDlgParams.SearchStr.size())
							return true;

						// Проверяем первый символ текущего блока с учётом обратного смещения, которое делается
						// при переходе между блоками
						i.LastSymbol = readBuffer[m_SearchDlgParams.SearchStr.size() - 1];

						if (FindFiles::IsWordDiv(i.LastSymbol))
							return true;

						// Если размер буфера меньше размера слова, то выходим
						if (readBlockSize < m_SearchDlgParams.SearchStr.size())
						{
							ErrorState = true;
							continue;
						}
					}

					// Устанавливаем буфер сравнения
					buffer = readBuffer.data();
				}

				i.WordFound = false;

				string_view Where{ buffer, bufferCount };

				const auto Next = [&](size_t const Offset)
				{
					Where.remove_prefix(Offset + 1);
				};

				while (!Where.empty())
				{
					const auto FoundPosition = m_TextSearcher->find_in(Where);
					if (!FoundPosition)
						break;

					if (!m_SearchDlgParams.WholeWords.value())
						return true;

					const auto [FoundOffset, FoundSize] = *FoundPosition;

					const auto AbsoluteOffset = bufferCount - Where.size() + FoundOffset;

					// Если идёт поиск по словам, то делаем соответствующие проверки
					bool firstWordDiv = false;

					// Если мы находимся вначале блока
					if (!AbsoluteOffset)
					{
						// Если мы находимся вначале файла, то считаем, что разделитель есть
						// Если мы находимся вначале блока, то проверяем является
						// или нет последний символ предыдущего блока разделителем
						if (alreadyRead == readBlockSize || FindFiles::IsWordDiv(i.LastSymbol))
							firstWordDiv = true;
					}
					else
					{
						// Проверяем является или нет предыдущий найденному символ блока разделителем

						i.LastSymbol = buffer[AbsoluteOffset - 1];

						if (FindFiles::IsWordDiv(i.LastSymbol))
							firstWordDiv = true;
					}

					// Проверяем разделитель в конце, только если найден разделитель в начале
					if (firstWordDiv)
					{
						// Если блок выбран не до конца
						if (AbsoluteOffset + FoundSize != bufferCount)
						{
							// Проверяем является или нет последующий за найденным символ блока разделителем
							i.LastSymbol = buffer[AbsoluteOffset + FoundSize];

							if (FindFiles::IsWordDiv(i.LastSymbol))
								return true;
						}
						else
							i.WordFound = true;
					}

					Next(FoundOffset);
				}

				// Выходим, если мы вышли за пределы количества байт разрешённых для поиска
				if (SearchInFirst && alreadyRead >= SearchInFirst)
				{
					ErrorState = true;
					continue;
				}

				if (!IsLastBlock)
				{
					if (IsUtf16CodePage(i.CodePage))
					{
						i.LastSymbol = readBuffer[bufferCount - StepBackOffset / sizeof(wchar_t) - 1];
					}
					else
					{
						// HERE BE DRAGONS

						// We can't just go back an arbitrary number of bytes in case of UTF-8, because we might land in the middle of a character.
						// Decoding it will succeed, but will produce something completely unrelated to the original character.
						// This could lead to rare false positives in search (if the user looks for such invalid characters for any reason)
						// and have other undesirable effects. In particular, such characters break the current FoldString-based implementation of fuzzy search.

						// To address this, we take a few bytes and decode them. The diagnostics will report the number of undecoded bytes,
						// so everything else is the number of decoded bytes which can be safely skipped on the next iteration.

						// * 2 To make sure that we can decode at least one
						bytes_view const TestStr(readBufferA.data() + readBlockSize - StepBackOffset, m_MaxCharSize * 2);

						encoding::diagnostics Diagnostics{ encoding::diagnostics::not_enough_data };
						const auto TestStrChars = encoding::get_chars(i.CodePage, TestStr, readBuffer, &Diagnostics);

						i.BytesToSkip = TestStr.size() - Diagnostics.PartialInput;

						// Запоминаем последний символ блока
						i.LastSymbol = readBuffer[TestStrChars - Diagnostics.PartialOutput];
					}
				}
			}

			if (ErrorState)
				return false;
		}

		// Если мы потенциально прочитали не весь файл
		if (!IsLastBlock)
		{
			// Отступаем назад на длину слова поиска минус 1
			if (!File.SetPointer(-StepBackOffset, nullptr, FILE_CURRENT))
				return false;
			alreadyRead -= StepBackOffset;
		}
	}

	return false;
}

bool background_searcher::IsFileIncluded(PluginPanelItem* FileItem, string_view const FullName, os::fs::attributes FileAttr, string_view const DisplayName)
{
	if (!m_Owner->GetFileMask()->check(PointToName(FullName)))
		return false;

	const auto hPlugin = m_FindFileArcItem? m_FindFileArcItem->hPlugin : nullptr;

	if (FileAttr & FILE_ATTRIBUTE_DIRECTORY)
		return Global->Opt->FindOpt.FindFolders && m_SearchDlgParams.IsSearchPatternEmpty();

	if (m_SearchDlgParams.IsSearchPatternEmpty())
		return true;

	m_Owner->m_Messages.emplace(messages::status{ string(DisplayName) });

	string strSearchFileName;
	bool RemoveTemp = false;

	SCOPE_EXIT
	{
		if (RemoveTemp)
			DeleteFileWithFolder(strSearchFileName);
	};

	if (!hPlugin)
	{
		strSearchFileName = FullName;
	}
	else
	{
		const auto UseInternalCommand = [&]
		{
			SCOPED_ACTION(auto)(m_Owner->ScopedLock());
			OpenPanelInfo Info;
			Global->CtrlObject->Plugins->GetOpenPanelInfo(hPlugin, &Info);
			return PluginManager::UseInternalCommand(hPlugin, PLUGIN_FARGETFILES, Info);
		};

		if (UseInternalCommand())
		{
			strSearchFileName = strPluginSearchPath + FullName;
		}
		else
		{
			const auto strTempDir = MakeTemp();
			if (!os::fs::create_directory(strTempDir))
				return false;

			const auto GetFile = [&]
			{
				SCOPED_ACTION(auto)(m_Owner->ScopedLock());
				return Global->CtrlObject->Plugins->GetFile(hPlugin, FileItem, strTempDir, strSearchFileName, OPM_SILENT | OPM_FIND) != FALSE;
			};

			if (!GetFile())
			{
				// BUGBUG check result
				if (!os::fs::remove_directory(strTempDir))
				{
					LOGWARNING(L"remove_directory({}): {}"sv, strTempDir, os::last_error());
				}

				return false;
			}

			RemoveTemp = true;
		}
	}

	return LookForString(strSearchFileName) ^ m_Options.NotContaining;
}

static void clear_queue(std::queue<message>&& Messages)
{
	for (; !Messages.empty(); Messages.pop())
	{
		std::visit(overload
		{
			[&](messages::menu_data const& Data)
			{
				FreePluginPanelItemUserData(Data.m_Arc? Data.m_Arc->hPlugin : nullptr, Data.m_UserData);
			},
			[](messages::push const&){},
			[](messages::pop const&){},
			[](messages::status const&){},
			[](messages::percent const&){}
		}, Messages.front());
	}
}

void FindFiles::stop_and_discard(Dialog* Dlg)
{
	if (Finalized)
		return;

	m_Searcher->Stop();
	clear_queue(std::move(m_ExtractedMessages));

	// The request to stop might arrive in the middle of something and searcher can still pump some messages
	do
	{
		clear_queue(m_Messages.pop_all());
	}
	while (!m_Searcher->Finished());

	Dlg->SendMessage(DM_REFRESH, 0, {});
}

FindListItem& FindFiles::AddFindListItem(const os::fs::find_data& FindData, UserDataItem const& UserData)
{
	FindListItem NewItem;
	NewItem.FindData = FindData;
	NewItem.UserData = UserData;

	m_FindList.emplace_back(std::move(NewItem));
	return m_FindList.back();
}

void FindFiles::ClearFindList()
{
	if (m_FindList.empty())
		return;

	for (const auto& i: m_FindList)
	{
		FreePluginPanelItemUserData(i.Arc? i.Arc->hPlugin : nullptr, i.UserData);
	}

	m_FindList.clear();
}

intptr_t FindFiles::FindDlgProc(Dialog* Dlg, intptr_t Msg, intptr_t Param1, void* Param2)
{
	if (!m_ExceptionPtr)
	{
		m_ExceptionPtr = m_Searcher->ExceptionPtr();
		if (m_ExceptionPtr)
		{
			Dlg->SendMessage(DM_CLOSE, 0, nullptr);
			return TRUE;
		}
	}

	auto& ListBox = Dlg->GetAllItem()[FD_LISTBOX].ListPtr;

	switch (Msg)
	{
	case DM_REFRESH:
		{
			if (Finalized)
				break;

			if (os::handle::is_signaled(console.GetInputHandle()))
				break;

			const auto refresh_status = [&]
			{
				const auto strDataStr = far::vformat(msg(lng::MFindFileFound), m_FileCount, m_DirCount);
				Dlg->SendMessage(DM_SETTEXTPTR, FD_SEPARATOR1, UNSAFE_CSTR(strDataStr));

				if (m_Searcher->Finished())
				{
					Dlg->SendMessage(DM_SETTEXTPTR, FD_TEXT_STATUS, {});
				}
				else
				{
					auto strFM = m_Status;

					if (!m_SearchDlgParams.IsSearchPatternEmpty())
					{
						strFM = far::vformat(
							msg(lng::MFindFileSearchingIn),
							quote_unconditional(
								truncate_right(m_SearchDlgParams.Hex.value()
									? BlobToHexString(m_SearchDlgParams.SearchBytes.value(), 0)
									: m_SearchDlgParams.SearchStr, 10)))
							+ L' ' + strFM;

						Dlg->SendMessage(DM_SETTEXTPTR, FD_TEXT_STATUS_PERCENTS, UNSAFE_CSTR(far::format(L"{:3}%"sv, m_Percent)));
					}

					SMALL_RECT Rect;
					Dlg->SendMessage(DM_GETITEMPOSITION, FD_TEXT_STATUS, &Rect);

					inplace::truncate_center(strFM, Rect.Right - Rect.Left + 1);
					Dlg->SendMessage(DM_SETTEXTPTR, FD_TEXT_STATUS, UNSAFE_CSTR(strFM));
				}

				if (m_LastFoundNumber)
				{
					m_LastFoundNumber = 0;

					if (ListBox->UpdateRequired())
						Dlg->SendMessage(DM_SHOWITEM, FD_LISTBOX, ToPtr(1));
				}

				Dlg->SendMessage(DM_ENABLEREDRAW, 1, nullptr);
				Dlg->SendMessage(DM_ENABLEREDRAW, 0, nullptr);
			};

			SCOPED_ACTION(Dialog::suppress_redraw)(Dlg);

			refresh_status();

			if (m_ExtractedMessages.empty())
				m_ExtractedMessages = m_Messages.pop_all();

			for (; !m_ExtractedMessages.empty(); m_ExtractedMessages.pop())
			{
				if (os::handle::is_signaled(console.GetInputHandle()))
					break;

				ProcessMessage(m_ExtractedMessages.front());

				if (m_TimeCheck)
					refresh_status();
			}

			if (m_Searcher->Finished() && m_Messages.empty() && m_ExtractedMessages.empty())
			{
				m_UpdateTimer = {};

				Finalized = true;

				SCOPED_ACTION(Dialog::suppress_redraw)(Dlg);
				const auto strMessage = far::vformat(msg(lng::MFindFileDone), m_FileCount, m_DirCount);
				Dlg->SendMessage(DM_SETTEXTPTR, FD_SEPARATOR1, nullptr);
				Dlg->SendMessage(DM_SETTEXTPTR, FD_TEXT_STATUS, UNSAFE_CSTR(strMessage));
				Dlg->SendMessage(DM_SETTEXTPTR, FD_TEXT_STATUS_PERCENTS, nullptr);
				Dlg->SendMessage(DM_SETTEXTPTR, FD_BUTTON_STOP, const_cast<wchar_t*>(msg(lng::MFindFileCancel).c_str()));
				ConsoleTitle::SetFarTitle(strMessage);

				Dlg->SendMessage(DM_ENABLEREDRAW, 1, nullptr);
				Dlg->SendMessage(DM_ENABLEREDRAW, 0, nullptr);
			}
		}
		break;

	case DN_INITDIALOG:
		{
			Dlg->GetAllItem()[FD_LISTBOX].ListPtr->SetMenuFlags(VMENU_NOMERGEBORDER);
			Dlg->SendMessage(DM_SETCOMBOBOXEVENT, FD_LISTBOX, ToPtr(CBET_KEY | CBET_MOUSE));
		}
		break;

	case DN_DRAWDLGITEMDONE: //???
	case DN_DRAWDIALOGDONE:
		Dlg->DefProc(Msg,Param1,Param2);

		// Переместим фокус на кнопку [Go To]
		if ((m_DirCount || m_FileCount) && !FindPositionChanged)
		{
			FindPositionChanged=true;
			Dlg->SendMessage(DM_SETFOCUS, FD_BUTTON_GOTO, nullptr);
		}
		return TRUE;

	case DN_CONTROLINPUT:
		{
			const auto record = static_cast<const INPUT_RECORD*>(Param2);
			if (none_of(record->EventType, KEY_EVENT, MOUSE_EVENT))
				break;

			switch (const auto key = InputRecordToKey(record); key)
			{
			case KEY_ESC:
			case KEY_F10:
				{
					if (!Finalized)
					{
						if (!m_Searcher->Finished())
							m_Searcher->Pause();

						if (ConfirmAbort())
						{
							stop_and_discard(Dlg);
						}
						else
						{
							if (!m_Searcher->Finished())
								m_Searcher->Resume();
						}

						return TRUE;
					}
				}
				break;

			case KEY_RIGHT:
			case KEY_NUMPAD6:
			case KEY_TAB:
				if (Param1==FD_BUTTON_STOP)
				{
					FindPositionChanged=true;
					Dlg->SendMessage(DM_SETFOCUS, FD_BUTTON_NEW, nullptr);
					return TRUE;
				}
				break;

			case KEY_LEFT:
			case KEY_NUMPAD4:
			case KEY_SHIFTTAB:
				if (Param1==FD_BUTTON_NEW)
				{
					FindPositionChanged=true;
					Dlg->SendMessage(DM_SETFOCUS, FD_BUTTON_STOP, nullptr);
					return TRUE;
				}
				break;

			case KEY_UP:
			case KEY_DOWN:
			case KEY_NUMPAD8:
			case KEY_NUMPAD2:
			case KEY_PGUP:
			case KEY_PGDN:
			case KEY_NUMPAD9:
			case KEY_NUMPAD3:
			case KEY_HOME:
			case KEY_END:
			case KEY_NUMPAD7:
			case KEY_NUMPAD1:
			case KEY_MSWHEEL_UP:
			case KEY_MSWHEEL_DOWN:
			case KEY_ALTLEFT:
			case KEY_RALTLEFT:
			case KEY_ALT|KEY_NUMPAD4:
			case KEY_RALT|KEY_NUMPAD4:
			case KEY_MSWHEEL_LEFT:
			case KEY_ALTRIGHT:
			case KEY_RALTRIGHT:
			case KEY_ALT|KEY_NUMPAD6:
			case KEY_RALT|KEY_NUMPAD6:
			case KEY_MSWHEEL_RIGHT:
			case KEY_ALTSHIFTLEFT:
			case KEY_RALTSHIFTLEFT:
			case KEY_ALT|KEY_SHIFT|KEY_NUMPAD4:
			case KEY_RALT|KEY_SHIFT|KEY_NUMPAD4:
			case KEY_ALTSHIFTRIGHT:
			case KEY_RALTSHIFTRIGHT:
			case KEY_ALT|KEY_SHIFT|KEY_NUMPAD6:
			case KEY_RALT|KEY_SHIFT|KEY_NUMPAD6:
			case KEY_ALTHOME:
			case KEY_RALTHOME:
			case KEY_ALT|KEY_NUMPAD7:
			case KEY_RALT|KEY_NUMPAD7:
			case KEY_ALTEND:
			case KEY_RALTEND:
			case KEY_ALT|KEY_NUMPAD1:
			case KEY_RALT|KEY_NUMPAD1:
				ListBox->ProcessKey(Manager::Key(key));
				return TRUE;

			/*
			case KEY_CTRLA:
			case KEY_RCTRLA:
			{
				if (!ListBox->GetItemCount())
				{
					return TRUE;
				}

				size_t ItemIndex = *static_cast<size_t*>(ListBox->GetUserData(nullptr,0));

				FINDLIST FindItem;
				itd->GetFindListItem(ItemIndex, FindItem);

				if (ShellSetFileAttributes(nullptr,FindItem.FindData.strFileName))
				{
					itd->SetFindListItem(ItemIndex, FindItem);
					Dlg->SendMessage(DM_REDRAW,0,0);
				}
				return TRUE;
			}
			*/

			case KEY_F3:
			case KEY_ALTF3:
			case KEY_RALTF3:
			case KEY_CTRLSHIFTF3:
			case KEY_RCTRLSHIFTF3:
			case KEY_NUMPAD5:
			case KEY_SHIFTNUMPAD5:
			case KEY_F4:
			case KEY_ALTF4:
			case KEY_RALTF4:
			case KEY_CTRLSHIFTF4:
			case KEY_RCTRLSHIFTF4:
				{
					if (ListBox->empty())
					{
						return TRUE;
					}

					const auto FindItem = *ListBox->GetComplexUserDataPtr<FindListItem*>();
					bool RemoveTemp=false;
					string strSearchFileName;

					if (FindItem->FindData.Attributes & FILE_ATTRIBUTE_DIRECTORY)
					{
						return TRUE;
					}

					bool real_name = true;

					if(FindItem->Arc)
					{
						if(!(FindItem->Arc->Flags & OPIF_REALNAMES))
						{
							std::unique_ptr<plugin_panel> PluginPanelPtr;
							real_name = false;

							string strFindArcName = FindItem->Arc->strArcName;
							if(!FindItem->Arc->hPlugin)
							{
								const auto SavePluginsOutput = std::exchange(Global->DisablePluginsOutput, true);
								{
									SCOPED_ACTION(std::scoped_lock)(PluginCS);
									PluginPanelPtr = Global->CtrlObject->Plugins->OpenFilePlugin(&strFindArcName, OPM_NONE, OFP_SEARCH);
									FindItem->Arc->hPlugin = PluginPanelPtr.get();
								}
								Global->DisablePluginsOutput=SavePluginsOutput;

								if (!PluginPanelPtr)
								{
									return TRUE;
								}
							}

							const auto strTempDir = MakeTemp();
							if (!os::fs::create_directory(strTempDir))
								return false;

							const auto bGet = GetPluginFile(FindItem->Arc, FindItem->FindData, strTempDir, strSearchFileName, &FindItem->UserData);

							if (PluginPanelPtr)
							{
								SCOPED_ACTION(std::scoped_lock)(PluginCS);
								Global->CtrlObject->Plugins->ClosePanel(std::move(PluginPanelPtr));
								FindItem->Arc->hPlugin = nullptr;
							}

							if (!bGet)
							{
								// BUGBUG check result
								if (!os::fs::remove_directory(strTempDir))
								{
									LOGWARNING(L"remove_directory({}): {}"sv, strTempDir, os::last_error());
								}

								return FALSE;
							}

							RemoveTemp=true;
						}
					}

					if (real_name)
					{
						strSearchFileName = FindItem->FindData.FileName;
						if (!os::fs::exists(strSearchFileName) && os::fs::exists(FindItem->FindData.AlternateFileName()))
							strSearchFileName = FindItem->FindData.AlternateFileName();
					}

					OpenFile(strSearchFileName, key, FindItem, Dlg);

					if (RemoveTemp)
					{
						// external editor may not have enough time to open this file, so defer deletion
						if (!m_DelayedDeleter)
						{
							m_DelayedDeleter = std::make_unique<delayed_deleter>(true);
						}
						m_DelayedDeleter->add(strSearchFileName);
					}
					return TRUE;
				}

			default:
				break;
			}
		}
		break;

	case DN_BTNCLICK:
		{
			FindPositionChanged = true;
			switch (Param1)
			{
			case FD_BUTTON_NEW:
				stop_and_discard(Dlg);
				return FALSE;

			case FD_BUTTON_STOP:
				// As Stop
				if (!Finalized)
				{
					stop_and_discard(Dlg);
					return TRUE;
				}
				// As Cancel
				return FALSE;

			case FD_BUTTON_VIEW:
				{
					INPUT_RECORD key;
					KeyToInputRecord(KEY_F3,&key);
					FindDlgProc(Dlg,DN_CONTROLINPUT,FD_LISTBOX,&key);
					return TRUE;
				}

			case FD_BUTTON_GOTO:
			case FD_BUTTON_PANEL:
				// Переход и посыл на панель будем делать не в диалоге, а после окончания поиска.
				// Иначе возможна ситуация, когда мы ищем на панели, потом ее грохаем и создаем новую
				// (а поиск-то идет!) и в результате ФАР трапается.
				if(ListBox->empty())
				{
					return TRUE;
				}
				FindExitItem = *ListBox->GetComplexUserDataPtr<FindListItem*>();
				return FALSE;

			default:
				break;
			}
		}
		break;

	case DN_CLOSE:
		{
			BOOL Result = TRUE;
			if (Param1==FD_LISTBOX)
			{
				if(!ListBox->empty())
				{
					FindDlgProc(Dlg, DN_BTNCLICK, FD_BUTTON_GOTO, nullptr); // emulates a [ Go to ] button pressing;
				}
				else
				{
					Result = FALSE;
				}
			}
			if(Result)
			{
				stop_and_discard(Dlg);
			}
			return Result;
		}

	case DN_RESIZECONSOLE:
		{
			SCOPED_ACTION(Dialog::suppress_redraw)(Dlg);

			auto& Coord = *static_cast<COORD*>(Param2);
			SMALL_RECT DlgRect;
			Dlg->SendMessage( DM_GETDLGRECT, 0, &DlgRect);
			int DlgWidth=DlgRect.Right-DlgRect.Left+1;
			int DlgHeight=DlgRect.Bottom-DlgRect.Top+1;
			const auto IncX = Coord.X - DlgWidth - 2;
			const auto IncY = Coord.Y - DlgHeight - 2;

			if ((IncX > 0) || (IncY > 0))
			{
				Coord.X = DlgWidth + (IncX > 0? IncX : 0);
				Coord.Y = DlgHeight + (IncY > 0? IncY : 0);
				Dlg->SendMessage( DM_RESIZEDIALOG, 0, &Coord);
			}

			DlgWidth += IncX;
			DlgHeight += IncY;

			for (const auto i: std::views::iota(0uz, static_cast<size_t>(FD_SEPARATOR1)))
			{
				SMALL_RECT rect;
				Dlg->SendMessage( DM_GETITEMPOSITION, i, &rect);
				rect.Right += IncX;
				rect.Bottom += IncY;
				Dlg->SendMessage( DM_SETITEMPOSITION, i, &rect);
			}

			for (const auto i: std::views::iota(FD_SEPARATOR1 + 0, FD_BUTTON_STOP + 1))
			{
				SMALL_RECT rect;
				Dlg->SendMessage( DM_GETITEMPOSITION, i, &rect);

				if (i == FD_SEPARATOR1)
				{
					// Center text
					rect.Left = rect.Right = -1;
				}
				else if (i == FD_TEXT_STATUS)
				{
					rect.Right += IncX;
				}
				else if (i==FD_TEXT_STATUS_PERCENTS)
				{
					rect.Right+=IncX;
					rect.Left+=IncX;
				}

				rect.Top += IncY;
				rect.Bottom += IncY;
				Dlg->SendMessage( DM_SETITEMPOSITION, i, &rect);
			}

			if ((IncX <= 0) || (IncY <= 0))
			{
				Coord.X = DlgWidth;
				Coord.Y = DlgHeight;
				Dlg->SendMessage( DM_RESIZEDIALOG, 0, &Coord);
			}

			return TRUE;
		}

	default:
		break;
	}

	return Dlg->DefProc(Msg,Param1,Param2);
}

void FindFiles::OpenFile(string_view const SearchFileName, int OpenKey, const FindListItem* FindItem, Dialog* Dlg) const
{
	if (!os::fs::exists(SearchFileName))
		return;

	auto openMode = FILETYPE_VIEW;
	auto shouldForceInternal = false;
	const auto isKnownKey = GetFiletypeOpenMode(OpenKey, openMode, shouldForceInternal);

	assert(isKnownKey); // ensure all possible keys are handled

	if (!isKnownKey)
		return;

	const auto strOldTitle = console.GetTitle();
	const auto FileNameOnly = ExtractFileName(SearchFileName);
	const auto PathOnly = SearchFileName.substr(0, SearchFileName.size() - FileNameOnly.size());

	if (shouldForceInternal || !ProcessLocalFileTypes(SearchFileName, FileNameOnly, openMode, PluginMode, PathOnly))
	{
		if (openMode == FILETYPE_ALTVIEW && Global->Opt->strExternalViewer.empty())
			openMode = FILETYPE_VIEW;

		if (openMode == FILETYPE_ALTEDIT && Global->Opt->strExternalEditor.empty())
			openMode = FILETYPE_EDIT;

		if (openMode == FILETYPE_VIEW)
		{
			NamesList ViewList;

			// Возьмем все файлы, которые имеют реальные имена...
			for (const auto& i: m_FindList)
			{
				if ((i.Arc && !(i.Arc->Flags & OPIF_REALNAMES)) || i.FindData.FileName.empty() || i.FindData.Attributes & FILE_ATTRIBUTE_DIRECTORY)
					continue;

				ViewList.AddName(i.FindData.FileName);
			}

			ViewList.SetCurName(FindItem->FindData.FileName);

			const auto ShellViewer = FileViewer::create(
				SearchFileName,
				false,
				false,
				false,
				-1,
				{},
				ViewList.size() > 1? &ViewList : nullptr);

			ShellViewer->SetEnableF6(TRUE);

			if (FindItem->Arc && !(FindItem->Arc->Flags & OPIF_REALNAMES))
				ShellViewer->SetSaveToSaveAs(true);

			if (ShellViewer->GetExitCode()) Global->WindowManager->ExecuteModal(ShellViewer);
			// заставляем рефрешится экран
			Global->WindowManager->ResizeAllWindows();
		}

		if (openMode == FILETYPE_EDIT)
		{
			const auto ShellEditor = FileEditor::create(SearchFileName, CP_DEFAULT, 0);
			ShellEditor->SetEnableF6(true);

			if (FindItem->Arc && !(FindItem->Arc->Flags & OPIF_REALNAMES))
				ShellEditor->SetSaveToSaveAs(true);

			if (any_of(ShellEditor->GetExitCode(), -1, XC_OPEN_NEWINSTANCE))
			{
				Global->WindowManager->ExecuteModal(ShellEditor);
				// заставляем рефрешится экран
				Global->WindowManager->ResizeAllWindows();
			}
		}

		if (openMode == FILETYPE_ALTEDIT || openMode == FILETYPE_ALTVIEW)
		{
			const auto& externalCommand = openMode == FILETYPE_ALTEDIT? Global->Opt->strExternalEditor : Global->Opt->strExternalViewer;
			ProcessExternal(externalCommand, SearchFileName, FileNameOnly, PluginMode, PathOnly);
		}
	}

	console.SetTitle(strOldTitle);
}

void FindFiles::AddMenuRecord(Dialog* const Dlg, string_view const FullName, const os::fs::find_data& FindData, UserDataItem const& UserData, ArcListItem* const Arc)
{
	if (!Dlg)
		return;

	auto& ListBox = Dlg->GetAllItem()[FD_LISTBOX].ListPtr;

	if(ListBox->empty())
	{
		Dlg->SendMessage( DM_ENABLE, FD_BUTTON_GOTO, ToPtr(TRUE));
		Dlg->SendMessage( DM_ENABLE, FD_BUTTON_VIEW, ToPtr(TRUE));
		if(AnySetFindList)
		{
			Dlg->SendMessage( DM_ENABLE, FD_BUTTON_PANEL, ToPtr(TRUE));
		}
		Dlg->SendMessage( DM_ENABLE, FD_LISTBOX, ToPtr(TRUE));
	}

	auto MenuText = L" "s;

	for (auto& i: Global->Opt->FindOpt.OutColumns)
	{
		int Width = i.width;
		if (!Width)
		{
			Width = GetDefaultWidth(i);
		}

		switch (i.type)
		{
			case column_type::description:
			case column_type::owner:
			{
				// пропускаем, не реализовано
				break;
			}
			case column_type::name:
			{
				// даже если указали, пропускаем, т.к. поле имени обязательное и идет в конце.
				break;
			}

			case column_type::attributes:
			{
				append(MenuText, FormatStr_Attribute(FindData.Attributes, Width), BoxSymbols[BS_V1]);
				break;
			}
			case column_type::streams_number:
			case column_type::streams_size:
			case column_type::size:
			case column_type::size_compressed:
			case column_type::links_number:
			{
				unsigned long long Size = 0;
				size_t Count = 0;

				if (Arc)
				{
					if (i.type == column_type::streams_number || i.type == column_type::streams_size)
						EnumStreams(FindData.FileName, Size, Count);
					else if (i.type == column_type::links_number)
					{
						const auto Hardlinks = GetNumberOfLinks(FindData.FileName);
						Count = Hardlinks? *Hardlinks : 1;
					}
				}

				const auto SizeToDisplay =
					i.type == column_type::size?
						FindData.FileSize :
						i.type == column_type::size_compressed?
							FindData.AllocationSize :
							i.type == column_type::streams_size?
								Size :
								Count; // ???

				append(MenuText,
					FormatStr_Size(
						SizeToDisplay,
						FindData.FileName,
						FindData.Attributes,
						0,
						FindData.ReparseTag,
						any_of(i.type, column_type::streams_number, column_type::links_number)?
							column_type::streams_size :
							i.type,
						i.type_flags,
						Width),
					BoxSymbols[BS_V1]);
				break;
			}

			case column_type::date:
			case column_type::time:
			case column_type::date_write:
			case column_type::date_access:
			case column_type::date_creation:
			case column_type::date_change:
			{
				os::chrono::time_point const os::fs::find_data::* FileTime;

				switch (i.type)
				{
				case column_type::date_creation:
					FileTime = &os::fs::find_data::CreationTime;
					break;

				case column_type::date_access:
					FileTime = &os::fs::find_data::LastAccessTime;
					break;

				case column_type::date_change:
					FileTime = &os::fs::find_data::ChangeTime;
					break;

				default:
					FileTime = &os::fs::find_data::LastWriteTime;
					break;
				}

				append(MenuText, FormatStr_DateTime(std::invoke(FileTime, FindData), i.type, i.type_flags, Width), BoxSymbols[BS_V1]);
				break;
			}

		default:
			break;
		}
	}


	// В плагинах принудительно поставим указатель в имени на имя
	// для корректного его отображения в списке, отбросив путь,
	// т.к. некоторые плагины возвращают имя вместе с полным путём,
	// к примеру временная панель.

	append(MenuText, Arc? PointToName(FindData.FileName) : FindData.FileName);

	string strPathName(FullName);
	{
		const auto pos = FindLastSlash(strPathName);
		if (pos != string::npos)
		{
			strPathName.resize(pos);
		}
		else
		{
			strPathName.clear();
	}
	}
	AddEndSlash(strPathName);

	if (!equal_icase(strPathName, m_LastDirName))
	{
		if (!ListBox->empty())
		{
			MenuItemEx ListItem;
			ListItem.Flags|=LIF_SEPARATOR;
			ListBox->AddItem(std::move(ListItem));
		}

		m_LastDirName = strPathName;

		if (Arc)
		{
			if(!(Arc->Flags & OPIF_REALNAMES) && !Arc->strArcName.empty())
			{
				auto strArcPathName = Arc->strArcName + L':';

				if (!path::is_separator(strPathName.front()))
					AddEndSlash(strArcPathName);

				strArcPathName += strPathName == L".\\"sv? L"\\"s : strPathName;
				strPathName = std::move(strArcPathName);
			}
		}

		auto& FindItem = AddFindListItem({}, {});
		// Используем LastDirName, т.к. PathName уже может быть искажена
		FindItem.FindData.FileName = m_LastDirName;
		// Used=0 - Имя не попадёт во временную панель.
		FindItem.Used=0;
		// Поставим атрибут у каталога, чтобы он не был файлом :)
		FindItem.FindData.Attributes = FILE_ATTRIBUTE_DIRECTORY;
		FindItem.Arc = Arc;

		const auto Ptr = &FindItem;
		MenuItemEx ListItem(strPathName);
		ListItem.ComplexUserData = Ptr;
		ListBox->AddItem(std::move(ListItem));
	}

	auto& FindItem = AddFindListItem(FindData, UserData);
	FindItem.FindData.FileName = FullName;
	FindItem.Used=1;
	FindItem.Arc = Arc;

	int ListPos;
	{
		MenuItemEx ListItem(MenuText);
		ListItem.ComplexUserData = &FindItem;
		ListPos = ListBox->AddItem(std::move(ListItem));
	}

	// Выделим как положено - в списке.
	if (!m_FileCount && !m_DirCount)
	{
		ListBox->SetSelectPos(ListPos, -1);
	}

	if (FindData.Attributes & FILE_ATTRIBUTE_DIRECTORY)
	{
		++m_DirCount;
	}
	else
	{
		++m_FileCount;
	}

	++m_LastFoundNumber;
}

void background_searcher::AddMenuRecord(string_view const FullName, PluginPanelItem& FindData) const
{
	os::fs::find_data fdata;
	PluginPanelItemToFindDataEx(FindData, fdata);
	m_Owner->m_Messages.emplace(messages::menu_data{ FullName, fdata, FindData.UserData, m_FindFileArcItem });
	FindData.UserData.FreeData = nullptr; //передано в FINDLIST
}

void background_searcher::ArchiveSearch(string_view const ArcName)
{
	std::unique_ptr<plugin_panel> hArc;

	{
		const auto SavePluginsOutput = std::exchange(Global->DisablePluginsOutput, true);

		const string strArcName(ArcName);
		{
			SCOPED_ACTION(auto)(m_Owner->ScopedLock());
			hArc = Global->CtrlObject->Plugins->OpenFilePlugin(&strArcName, OPM_FIND, OFP_SEARCH);
		}
		Global->DisablePluginsOutput = SavePluginsOutput;
	}

	if (!hArc)
		return;

	const auto SaveSearchMode = SearchMode;
	const auto SaveArcItem = m_FindFileArcItem;
	{
		const auto SavePluginsOutput = std::exchange(Global->DisablePluginsOutput, true);

		// BUGBUG
		const_cast<FINDAREA&>(SearchMode) = FINDAREA_FROM_CURRENT;
		OpenPanelInfo Info;
		{
			SCOPED_ACTION(auto)(m_Owner->ScopedLock());
			Global->CtrlObject->Plugins->GetOpenPanelInfo(hArc.get(), &Info);
		}
		m_FindFileArcItem = &m_Owner->itd->AddArcListItem(ArcName, hArc.get(), Info.Flags, NullToEmpty(Info.CurDir));
		// Запомним каталог перед поиском в архиве. И если ничего не нашли - не рисуем его снова.
		{
			// Запомним пути поиска в плагине, они могут измениться.
			const auto strSaveSearchPath = strPluginSearchPath;
			m_Owner->m_Messages.emplace(messages::push{});
			DoPreparePluginListImpl();
			strPluginSearchPath = strSaveSearchPath;
			{
				SCOPED_ACTION(auto)(m_Owner->ScopedLock());
				Global->CtrlObject->Plugins->ClosePanel(std::move(hArc));
				m_FindFileArcItem->hPlugin = {};
			}

			m_Owner->m_Messages.emplace(messages::pop{});
		}

		Global->DisablePluginsOutput=SavePluginsOutput;
	}
	m_FindFileArcItem = SaveArcItem;
	// BUGBUG
	const_cast<FINDAREA&>(SearchMode) = SaveSearchMode;
}

void background_searcher::DoScanTree(string_view const strRoot)
{
	ScanTree ScTree(
		false,
		!(SearchMode==FINDAREA_CURRENT_ONLY||SearchMode==FINDAREA_INPATH),
		Global->Opt->FindOpt.FindSymLinks,
		true
	);

	if (SearchMode==FINDAREA_SELECTED)
		Global->CtrlObject->Cp()->ActivePanel()->GetSelName(nullptr);

	os::fs::find_data FindData;

	while (!Stopped())
	{
		string strCurRoot;

		if (SearchMode==FINDAREA_SELECTED)
		{
			string strSelName;
			if (!Global->CtrlObject->Cp()->ActivePanel()->GetSelName(&strSelName, nullptr, &FindData))
				break;

			if (!(FindData.Attributes & FILE_ATTRIBUTE_DIRECTORY) || IsParentDirectory(strSelName))
				continue;

			strCurRoot = path::join(strRoot, strSelName);
		}
		else
		{
			strCurRoot = strRoot;
		}

		ScTree.SetFindPath(strCurRoot, L"*"sv);
		m_Owner->m_Messages.emplace(messages::status{ strCurRoot });

		string strFullName;

		while (!Stopped() && ScTree.GetNextName(FindData, strFullName))
		{
			std::this_thread::yield();
			PauseEvent.wait();

			const auto ProcessStream = [&](string_view const FullStreamName)
			{
				if (UseFilter)
				{
					const auto FilterResult = m_Owner->GetFilter()->FileInFilterEx(FindData, FullStreamName);
					switch (FilterResult.Action)
					{
					case filter_action::include:
						break;

					case filter_action::exclude:
						if (FindData.Attributes & FILE_ATTRIBUTE_DIRECTORY)
							ScTree.SkipDir();
						return !Stopped();

					case filter_action::ignore:
						if (FilterResult.State & filter_state::has_include)
							return !Stopped();
						break;
					}
				}

				if (FindData.Attributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					m_Owner->m_Messages.emplace(messages::status{ string(FullStreamName) });
				}

				if (IsFileIncluded(nullptr, FullStreamName, FindData.Attributes, strFullName))
				{
					m_Owner->m_Messages.emplace(messages::menu_data{ FullStreamName, FindData, {}, {} });
				}

				if (m_Options.SearchInArchives && !(FindData.Attributes & FILE_ATTRIBUTE_DIRECTORY))
					ArchiveSearch(FullStreamName);

				return !Stopped();
			};

			// default stream first:
			if (!ProcessStream(strFullName))
				break;

			// now the rest:
			if (Global->Opt->FindOpt.FindAlternateStreams)
			{
				const auto FindDataFileName = FindData.FileName;

				for(const auto& StreamData: os::fs::enum_streams(strFullName))
				{
					const string_view StreamFullName(StreamData.cStreamName + 1);
					const auto [StreamName, StreamType] = split(StreamFullName, L':');

					if (StreamName.empty()) // default stream has already been processed
						continue;

					const auto FullStreamName = concat(strFullName, L':', StreamName);
					FindData.FileName = concat(FindDataFileName, L':', StreamName);
					FindData.FileSize = StreamData.StreamSize.QuadPart;
					FindData.Attributes &= ~FILE_ATTRIBUTE_DIRECTORY;

					if (!ProcessStream(FullStreamName))
						break;
				}
			}
		}

		if (SearchMode!=FINDAREA_SELECTED)
			break;
	}
}

void background_searcher::ScanPluginTree(plugin_panel* hPlugin, unsigned long long Flags, int& RecurseLevel)
{
	std::span<PluginPanelItem> PanelData;
	bool GetFindDataResult=false;

	if(!Stopped())
	{
		SCOPED_ACTION(auto)(m_Owner->ScopedLock());
		GetFindDataResult = Global->CtrlObject->Plugins->GetFindData(hPlugin, PanelData, OPM_FIND) != FALSE;
	}

	if (!GetFindDataResult)
	{
		return;
	}

	RecurseLevel++;

	if (SearchMode!=FINDAREA_SELECTED || RecurseLevel!=1)
	{
		for (auto& CurPanelItem: PanelData)
		{
			if (Stopped())
				break;

			std::this_thread::yield();
			PauseEvent.wait();

			string_view CurName = NullToEmpty(CurPanelItem.FileName);
			if (CurName.empty() || IsParentDirectory(CurPanelItem))
				continue;

			if (UseFilter && !m_Owner->GetFilter()->FileInFilter(CurPanelItem))
				continue;

			const auto strFullName = concat(strPluginSearchPath, CurName);

			if (CurPanelItem.FileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				m_Owner->m_Messages.emplace(messages::status{ strFullName });
			}

			if (IsFileIncluded(&CurPanelItem, CurName, CurPanelItem.FileAttributes, strFullName))
				AddMenuRecord(strFullName, CurPanelItem);

			if (m_Options.SearchInArchives && !(CurPanelItem.FileAttributes & FILE_ATTRIBUTE_DIRECTORY) && (Flags & OPIF_REALNAMES))
				ArchiveSearch(strFullName);
		}
	}

	if (SearchMode!=FINDAREA_CURRENT_ONLY)
	{
		OpenPanelInfo PanelInfo{};
		{
			SCOPED_ACTION(auto)(m_Owner->ScopedLock());
			Global->CtrlObject->Plugins->GetOpenPanelInfo(hPlugin, &PanelInfo);
		}

		auto ParentPointSeen = (PanelInfo.Flags & OPIF_ADDDOTS) != 0;

		for (const auto& CurPanelItem: PanelData)
		{
			if (Stopped())
				break;

			if (!(CurPanelItem.FileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				continue;

			if (!ParentPointSeen && IsParentDirectory(CurPanelItem))
			{
				ParentPointSeen = true;
				continue;
			}

			if (UseFilter && !m_Owner->GetFilter()->FileInFilter(CurPanelItem))
				continue;

			if ((SearchMode == FINDAREA_SELECTED && RecurseLevel == 1 && !Global->CtrlObject->Cp()->ActivePanel()->IsSelected(CurPanelItem.FileName)))
				continue;

			int SetDirectoryResult;
			{
				SCOPED_ACTION(auto)(m_Owner->ScopedLock());
				SetDirectoryResult = Global->CtrlObject->Plugins->SetDirectory(hPlugin, CurPanelItem.FileName, OPM_FIND, &CurPanelItem.UserData);
			}
			if (!SetDirectoryResult)
				continue;

			if (!*CurPanelItem.FileName)
				continue;

			append(strPluginSearchPath, CurPanelItem.FileName, path::separator);
			ScanPluginTree(hPlugin, Flags, RecurseLevel);

			size_t pos = strPluginSearchPath.rfind(path::separator);
			if (pos != string::npos)
				strPluginSearchPath.resize(pos);

			if ((pos = strPluginSearchPath.rfind(path::separator)) != string::npos)
				strPluginSearchPath.resize(pos+1);
			else
				strPluginSearchPath.clear();

			{
				SCOPED_ACTION(auto)(m_Owner->ScopedLock());
				SetDirectoryResult = Global->CtrlObject->Plugins->SetDirectory(hPlugin, L".."s, OPM_FIND);
			}
			if (!SetDirectoryResult)
			{
				// BUGBUG, better way to stop searcher?
				Stop();
			}
		}
	}

	{
		SCOPED_ACTION(auto)(m_Owner->ScopedLock());
		Global->CtrlObject->Plugins->FreeFindData(hPlugin, PanelData, true);
	}
	RecurseLevel--;
}

void background_searcher::DoPrepareFileList()
{
	std::vector<string> Locations;

	if (SearchMode==FINDAREA_INPATH)
	{
		for (const auto& i: enum_tokens_with_quotes(os::env::get(L"PATH"sv), L";"sv))
		{
			if (i.empty())
				continue;

			Locations.emplace_back(i);
		}
	}
	else if (SearchMode==FINDAREA_ROOT)
	{
		Locations = {GetPathRoot(Global->CtrlObject->CmdLine()->GetCurDir())};
	}
	else if (SearchMode==FINDAREA_ALL || SearchMode==FINDAREA_ALL_BUTNETWORK)
	{
		const auto Drives = os::fs::get_logical_drives() & allowed_drives_mask();

		const auto is_acceptable_drive = [&](string_view const RootDirectory)
		{
			const auto Type = os::fs::drive::get_type(RootDirectory);
			return Type == DRIVE_FIXED || Type == DRIVE_RAMDISK || (SearchMode != FINDAREA_ALL_BUTNETWORK && Type == DRIVE_REMOTE);
		};

		for (const auto& i: os::fs::enum_drives(Drives))
		{
			auto RootDir = os::fs::drive::get_root_directory(i);
			if (!is_acceptable_drive(RootDir))
				continue;

			Locations.emplace_back(std::move(RootDir));
		}

		for (auto& VolumeName: os::fs::enum_volumes())
		{
			if (!is_acceptable_drive(VolumeName))
				continue;

			if (const auto DriveLetter = get_volume_drive(VolumeName); DriveLetter && Drives[os::fs::drive::get_number(*DriveLetter)])
				continue;

			Locations.emplace_back(std::move(VolumeName));
		}
	}
	else
	{
		Locations = {Global->CtrlObject->CmdLine()->GetCurDir()};
	}

	for (const auto& i: Locations)
	{
		DoScanTree(i);
	}
}

void background_searcher::DoPreparePluginListImpl()
{
	const auto ArcItem = m_FindFileArcItem;
	OpenPanelInfo Info;
	string strSaveDir;
	{
		SCOPED_ACTION(auto)(m_Owner->ScopedLock());
		Global->CtrlObject->Plugins->GetOpenPanelInfo(ArcItem->hPlugin,&Info);
		strSaveDir = NullToEmpty(Info.CurDir);
		if (SearchMode==FINDAREA_ROOT || SearchMode==FINDAREA_ALL || SearchMode==FINDAREA_ALL_BUTNETWORK || SearchMode==FINDAREA_INPATH)
		{
			Global->CtrlObject->Plugins->SetDirectory(ArcItem->hPlugin, L"\\"s, OPM_FIND);
			Global->CtrlObject->Plugins->GetOpenPanelInfo(ArcItem->hPlugin,&Info);
		}
	}

	strPluginSearchPath = NullToEmpty(Info.CurDir);

	if (!strPluginSearchPath.empty())
		AddEndSlash(strPluginSearchPath);

	int RecurseLevel=0;
	ScanPluginTree(ArcItem->hPlugin, ArcItem->Flags, RecurseLevel);

	if (SearchMode==FINDAREA_ROOT || SearchMode==FINDAREA_ALL || SearchMode==FINDAREA_ALL_BUTNETWORK || SearchMode==FINDAREA_INPATH)
	{
		SCOPED_ACTION(auto)(m_Owner->ScopedLock());
		Global->CtrlObject->Plugins->SetDirectory(ArcItem->hPlugin,strSaveDir,OPM_FIND,&Info.UserData);
	}
}

void background_searcher::DoPreparePluginList()
{
	DoPreparePluginListImpl();
}

void background_searcher::Search()
{
	os::debug::set_thread_name(L"Find file");

	seh_try_thread(m_SehException, [this]
	{
		cpp_try(
		[&]
		{
			SCOPED_ACTION(wakeful);
			InitInFileSearch();
			m_PluginMode? DoPreparePluginList() : DoPrepareFileList();
			ReleaseInFileSearch();
		},
		save_exception_to(m_ExceptionPtr)
		);
	});

	m_Owner->m_Messages.emplace(messages::percent{});
	m_TaskbarProgress.reset();
	m_Finished = true;
}

bool FindFiles::FindFilesProcess()
{
	// Если используется фильтр операций, то во время поиска сообщаем об этом
	string strTitle=msg(lng::MFindFileTitle);

	itd->Init();

	m_FileCount = 0;
	m_DirCount = 0;
	m_LastFoundNumber = 0;
	m_LastDirName.clear();

	if (!m_FindMask.empty())
	{
		append(strTitle, L": "sv, m_FindMask);

		if (UseFilter)
		{
			append(strTitle, L" ("sv, msg(lng::MFindFileUsingFilter), L')');
		}
	}
	else
	{
		if (UseFilter)
		{
			append(strTitle, L" ("sv, msg(lng::MFindFileUsingFilter), L')');
		}
	}

	int DlgWidth = ScrX + 1 - 2;
	int DlgHeight = ScrY + 1 - 2;

	auto FindDlg = MakeDialogItems<FD_COUNT>(
	{
		{ DI_DOUBLEBOX, {{3,                    1}, {DlgWidth-4, DlgHeight-2}}, DIF_SHOWAMPERSAND, strTitle, },
		{ DI_LISTBOX,   {{4,                    2}, {DlgWidth-5, DlgHeight-7}}, DIF_LISTNOBOX|DIF_DISABLE|DIF_NOFOCUS, },
		{ DI_TEXT,      {{-1,         DlgHeight-6}, {0,          DlgHeight-6}}, DIF_SEPARATOR2, },
		{ DI_TEXT,      {{5,          DlgHeight-5}, {DlgWidth-(m_SearchDlgParams.IsSearchPatternEmpty() ? 6 : 12), DlgHeight - 5}}, DIF_SHOWAMPERSAND, L"…"sv},
		{ DI_TEXT,      {{DlgWidth-9, DlgHeight-5}, {DlgWidth-6, DlgHeight-5}}, (m_SearchDlgParams.IsSearchPatternEmpty() ? DIF_HIDDEN : DIF_NONE), },
		{ DI_TEXT,      {{-1,         DlgHeight-4}, {0,          DlgHeight-4}}, DIF_SEPARATOR, },
		{ DI_BUTTON,    {{0,          DlgHeight-3}, {0,          DlgHeight-3}}, DIF_CENTERGROUP | DIF_FOCUS | DIF_DEFAULTBUTTON, msg(lng::MFindFileNewSearch), },
		{ DI_BUTTON,    {{0,          DlgHeight-3}, {0,          DlgHeight-3}}, DIF_CENTERGROUP | DIF_DISABLE, msg(lng::MFindFileGoTo), },
		{ DI_BUTTON,    {{0,          DlgHeight-3}, {0,          DlgHeight-3}}, DIF_CENTERGROUP | DIF_DISABLE, msg(lng::MFindFileView), },
		{ DI_BUTTON,    {{0,          DlgHeight-3}, {0,          DlgHeight-3}}, DIF_CENTERGROUP | DIF_DISABLE, msg(lng::MFindFilePanel), },
		{ DI_BUTTON,    {{0,          DlgHeight-3}, {0,          DlgHeight-3}}, DIF_CENTERGROUP, msg(lng::MFindFileStop), },
	});

	ArcListItem* FindFileArcItem{};
	if (PluginMode)
	{
		const auto hPlugin = Global->CtrlObject->Cp()->ActivePanel()->GetPluginHandle();
		OpenPanelInfo Info;
		// no lock - background thread hasn't been started yet
		Global->CtrlObject->Plugins->GetOpenPanelInfo(hPlugin,&Info);
		FindFileArcItem = &itd->AddArcListItem(NullToEmpty(Info.HostFile), hPlugin, Info.Flags, NullToEmpty(Info.CurDir));

		if (!(Info.Flags & OPIF_REALNAMES))
		{
			FindDlg[FD_BUTTON_PANEL].Type=DI_TEXT;
			FindDlg[FD_BUTTON_PANEL].strData.clear();
		}
	}

	AnySetFindList = std::ranges::any_of(*Global->CtrlObject->Plugins, [](Plugin const* const i)
	{
		return i->has(iSetFindList);
	});

	if (!AnySetFindList)
	{
		FindDlg[FD_BUTTON_PANEL].Flags|=DIF_DISABLE;
	}

	const auto Dlg = Dialog::create(FindDlg, std::bind_front(&FindFiles::FindDlgProc, this));
	Dlg->SetHelp(L"FindFileResult"sv);
	Dlg->SetPosition({ -1, -1, DlgWidth, DlgHeight });
	Dlg->SetId(FindFileResultId);
	Dlg->SetFlags(FSCROBJ_SPECIAL);

	m_ResultsDialogPtr = Dlg.get();

		{
			background_searcher BC
			{
				this,
				SearchMode,
				FindFileArcItem,
				CodePage,
				ConvertFileSizeString(Global->Opt->FindOpt.strSearchInFirstSize),
				UseFilter,
				PluginMode
			};

			// BUGBUG
			m_Searcher = &BC;

			// Надо бы показать диалог, а то инициализация элементов запаздывает
			// иногда при поиске и первые элементы не добавляются
			Dlg->InitDialog();
			Dlg->Show();

			os::thread FindThread(&background_searcher::Search, &BC);

			// In case of an exception in the main thread
			SCOPE_EXIT
			{
				stop_and_discard(Dlg.get());
				Dlg->CloseDialog();
				m_Searcher = nullptr;
			};

			listener const Listener(listener::scope{L"FindFile"sv}, [&]
			{
				if (m_TimeCheck)
					Dlg->SendMessage(DM_REFRESH, 0, {});
			});

			m_UpdateTimer = os::concurrency::timer(till_next_second(), GetRedrawTimeout(), [&]
			{
				message_manager::instance().notify(Listener.GetEventName());
			});

			SCOPE_EXIT{ m_UpdateTimer = {}; };

			Dlg->Process();

			if (!m_ExceptionPtr)
			{
				m_ExceptionPtr = BC.ExceptionPtr();
			}

			rethrow_if(m_ExceptionPtr);

			if (auto& SehException = BC.SehException(); SehException.is_signaled())
			{
				SehException.raise();
			}

		}

		switch (Dlg->GetExitCode())
		{
			case FD_BUTTON_NEW:
			{
				return true;
			}

			case FD_BUTTON_PANEL:
			// Отработаем переброску на временную панель
			{
				plugin_item_list PanelItems;
				PanelItems.reserve(m_FindList.size());

				for (auto& i: m_FindList)
				{
					if (!i.FindData.FileName.empty() && i.Used)
					{
					// Добавляем всегда, если имя задано
						// Для плагинов с виртуальными именами заменим имя файла на имя архива.
						// панель сама уберет лишние дубли.
						const auto IsArchive = i.Arc && !(i.Arc->Flags&OPIF_REALNAMES);
						// Добавляем только файлы или имена архивов или папки когда просили
						if (IsArchive || (Global->Opt->FindOpt.FindFolders && !m_SearchDlgParams.Hex.value()) ||
								!(i.FindData.Attributes&FILE_ATTRIBUTE_DIRECTORY))
						{
							if (IsArchive)
							{
								i.FindData.FileName = i.Arc->strArcName;
							}
							PluginPanelItemHolderHeapNonOwning pi;
							FindDataExToPluginPanelItemHolder(i.FindData, pi);

							if (IsArchive)
								pi.Item.FileAttributes = 0;

							if (pi.Item.FileAttributes & FILE_ATTRIBUTE_DIRECTORY)
							{
								legacy::DeleteEndSlash(const_cast<wchar_t*>(pi.Item.FileName));
							}
							PanelItems.emplace_back(pi.Item);
						}
					}
				}

				{
					SCOPED_ACTION(std::scoped_lock)(PluginCS);
					if (auto hNewPlugin = Global->CtrlObject->Plugins->OpenFindListPlugin(PanelItems))
					{
						const auto NewPanel = Global->CtrlObject->Cp()->ChangePanel(Global->CtrlObject->Cp()->ActivePanel(), panel_type::FILE_PANEL, TRUE, TRUE);
						NewPanel->SetPluginMode(std::move(hNewPlugin), {}, true);
						NewPanel->SetVisible(true);
						NewPanel->Update(0);
						//if (FindExitItem)
						//NewPanel->GoToFile(FindExitItem->FindData.cFileName);
						NewPanel->Show();
					}
				}

				break;
			}
			case FD_BUTTON_GOTO:
			case FD_LISTBOX:
			{
				string strFileName=FindExitItem->FindData.FileName;
				auto FindPanel = Global->CtrlObject->Cp()->ActivePanel();

				if (FindExitItem->Arc)
				{
					if (!FindExitItem->Arc->hPlugin)
					{
						string strArcName = FindExitItem->Arc->strArcName;

						if (FindPanel->GetType() != panel_type::FILE_PANEL)
						{
							FindPanel = Global->CtrlObject->Cp()->ChangePanel(FindPanel, panel_type::FILE_PANEL, TRUE, TRUE);
						}

						string_view ArcPath=strArcName;
						CutToSlash(ArcPath);
						FindPanel->SetCurDir(ArcPath, true);
						FindExitItem->Arc->hPlugin = std::static_pointer_cast<FileList>(FindPanel)->OpenFilePlugin(strArcName, FALSE, OFP_SEARCH);
					}

					if (FindExitItem->Arc->hPlugin)
					{
						OpenPanelInfo Info;
						{
							SCOPED_ACTION(std::scoped_lock)(PluginCS);
							Global->CtrlObject->Plugins->GetOpenPanelInfo(FindExitItem->Arc->hPlugin, &Info);

							if (SearchMode == FINDAREA_ROOT ||
								SearchMode == FINDAREA_ALL ||
								SearchMode == FINDAREA_ALL_BUTNETWORK ||
								SearchMode == FINDAREA_INPATH)
								Global->CtrlObject->Plugins->SetDirectory(FindExitItem->Arc->hPlugin, L"\\"s, OPM_NONE);
						}
						SetPluginDirectory(strFileName, FindExitItem->Arc->hPlugin, true, &FindExitItem->UserData);
					}
				}
				else
				{
					if (strFileName.empty())
						break;

					const auto remove_trailing_slash_if_not_root = [](string& Path)
					{
						if (Path.size() > 1 && path::is_separator(Path.back()) && Path[Path.size() - 2] != L':')
							Path.pop_back();
					};

					remove_trailing_slash_if_not_root(strFileName);

					if (!os::fs::exists(strFileName) && (GetLastError() != ERROR_ACCESS_DENIED))
						break;

					const auto NamePtr = PointToName(strFileName);
					string strSetName(NamePtr);

					if (Global->Opt->FindOpt.FindAlternateStreams)
					{
						size_t Pos = strSetName.find(L':');

						if (Pos != string::npos)
							strSetName.resize(Pos);
					}

					strFileName.resize(strFileName.size() - NamePtr.size());

					remove_trailing_slash_if_not_root(strFileName);

					if (strFileName.empty())
						break;

					if (FindPanel->GetType() != panel_type::FILE_PANEL && Global->CtrlObject->Cp()->GetAnotherPanel(FindPanel)->GetType() == panel_type::FILE_PANEL)
						FindPanel=Global->CtrlObject->Cp()->GetAnotherPanel(FindPanel);

					if ((FindPanel->GetType() != panel_type::FILE_PANEL) || (FindPanel->GetMode() != panel_mode::NORMAL_PANEL))
					// Сменим панель на обычную файловую...
					{
						FindPanel = Global->CtrlObject->Cp()->ChangePanel(FindPanel, panel_type::FILE_PANEL, TRUE, TRUE);
						FindPanel->SetVisible(true);
						FindPanel->Update(0);
					}

					// ! Не меняем каталог, если мы уже в нем находимся.
					// Тем самым добиваемся того, что выделение с элементов панели не сбрасывается.
					string strDirTmp = FindPanel->GetCurDir();

					remove_trailing_slash_if_not_root(strDirTmp);

					if (!equal_icase(strFileName, strDirTmp))
						FindPanel->SetCurDir(strFileName,true);

					if (!strSetName.empty())
						FindPanel->GoToFile(strSetName);

					FindPanel->Show();
					FindPanel->Parent()->SetActivePanel(FindPanel);
				}
				break;
			}
		}
	return false;
}

void FindFiles::ProcessMessage(message& Message)
{
	std::visit(overload
	{
		[&](messages::menu_data const& Data)
		{
			AddMenuRecord(m_ResultsDialogPtr, Data.m_FullName, Data.m_FindData, Data.m_UserData, Data.m_Arc);
			m_EmptyArc = false;
		},
		[&](messages::push const&)
		{
			m_LastDir.push(m_LastDirName);
			m_LastDirName.clear();
			m_EmptyArc = true;
		},
		[&](messages::pop const&)
		{
			assert(!m_LastDir.empty());
			if (m_EmptyArc)
				m_LastDirName = m_LastDir.top();
			m_LastDir.pop();
		},
		[&](messages::status& Status)
		{
			m_Status = std::move(Status.Value);
		},
		[&](messages::percent const& Percent)
		{
			m_Percent = Percent.Value;
		}
	}, Message);
}

static FindFilesOptions& LastFindFileOptions()
{
	static FindFilesOptions LastFindFileOptions;
	return LastFindFileOptions;
}

static string& LastFindMask()
{
	static string LastFindMask{ AllFilesMask };
	return LastFindMask;
}

FindFiles::FindFiles():
	itd(std::make_unique<InterThreadData>()),
	m_Options{ LastFindFileOptions() },
	m_SearchDlgParams
	{
		.SearchStr = SearchReplaceDlgParams::GetShared(SearchReplaceDlgParams::SharedGroup::find_file).SearchStr,
		.SearchBytes = SearchReplaceDlgParams::GetShared(SearchReplaceDlgParams::SharedGroup::find_file).SearchBytes.value_or(bytes{}),
		.Hex = SearchReplaceDlgParams::GetShared(SearchReplaceDlgParams::SharedGroup::find_file).Hex.value_or(false),
		.CaseSensitive = SearchReplaceDlgParams::GetShared(SearchReplaceDlgParams::SharedGroup::find_file).CaseSensitive.value_or(false),
		.WholeWords = SearchReplaceDlgParams::GetShared(SearchReplaceDlgParams::SharedGroup::find_file).WholeWords.value_or(false),
		.Fuzzy = SearchReplaceDlgParams::GetShared(SearchReplaceDlgParams::SharedGroup::find_file).Fuzzy.value_or(false)
	},
	m_FindMask{ LastFindMask() },
	FileMaskForFindFile(std::make_unique<filemasks>()),
	Filter(std::make_unique<multifilter>(Global->CtrlObject->Cp()->ActivePanel().get(), FFT_FINDFILE))
{
	string strSearchFromRoot{ msg(lng::MFindFileSearchFromRootFolder) };

	SearchMode = static_cast<FINDAREA>(Global->Opt->FindOpt.FileSearchMode.Get());
	UseFilter = Global->Opt->FindOpt.UseFilter.Get();

	do
	{
		FindExitItem = nullptr;
		FindFoldersChanged=false;
		SearchFromChanged=false;
		FindPositionChanged=false;
		Finalized=false;
		ClearFindList();
		itd->ClearAllLists();
		const auto ActivePanel = Global->CtrlObject->Cp()->ActivePanel();
		PluginMode = ActivePanel->GetMode() == panel_mode::PLUGIN_PANEL && ActivePanel->IsVisible();
		PrepareDriveNameStr(strSearchFromRoot);

		constexpr auto DlgWidth{ 80 };
		constexpr auto HorizontalRadioGap{ 2 };
		const auto& Containing{ msg(lng::MFindFileContaining) };
		const auto& ContainingText{ msg(lng::MFindFileText) };
		const auto& ContainingHex{ msg(lng::MFindFileHex) };
		const auto ContainingW{ static_cast<int>(HiStrlen(Containing)) };
		const auto ContainingTextW{ static_cast<int>(HiStrlen(ContainingText) + 4) };
		const auto ContainingHexW{ static_cast<int>(HiStrlen(ContainingHex) + 4) };

		const auto ContainingX1{ 4 + 1 };                                           const auto ContainingX2{ ContainingX1 + ContainingW };

		const auto ContainingTextX1_{ ContainingX2 + HorizontalRadioGap };          const auto ContainingTextX2_{ ContainingTextX1_ + ContainingTextW };
		const auto ContainingHexX1_{ ContainingTextX2_ + HorizontalRadioGap };      const auto ContainingHexX2_{ ContainingHexX1_ + ContainingHexW };
		const auto ContainingHexOverage{ std::max(ContainingHexX2_ - (DlgWidth - 4 - 1), 0) };

		const auto ContainingTextX1{ ContainingTextX1_ - ContainingHexOverage };    const auto ContainingTextX2{ ContainingTextX2_ - ContainingHexOverage };
		const auto ContainingHexX1{ ContainingHexX1_ - ContainingHexOverage };      const auto ContainingHexX2{ ContainingHexX2_ - ContainingHexOverage };

		auto FindAskDlg = MakeDialogItems<FAD_COUNT>(
		{
			{ DI_DOUBLEBOX,   {{3,                1 }, {DlgWidth-4,       19}}, DIF_NONE, msg(lng::MFindFileTitle), },
			{ DI_TEXT,        {{5,                2 }, {0,                2 }}, DIF_NONE, msg(lng::MFindFileMasks), },
			{ DI_EDIT,        {{5,                3 }, {DlgWidth-4-2,     3 }}, DIF_FOCUS | DIF_HISTORY | DIF_USELASTHISTORY, },
			{ DI_TEXT,        {{-1,               4 }, {0,                4 }}, DIF_SEPARATOR, },
			{ DI_TEXT,        {{ContainingX1,     5 }, {0,                5 }}, DIF_NONE, Containing, },
			{ DI_EDIT,        {{5,                6 }, {DlgWidth-4-2,     6 }}, DIF_HISTORY, },
			{ DI_FIXEDIT,     {{5,                6 }, {DlgWidth-4-2,     6 }}, DIF_MASKEDIT, },
			{ DI_RADIOBUTTON, {{ContainingTextX1, 5 }, {ContainingTextX2, 5 }}, DIF_GROUP, ContainingText, },
			{ DI_RADIOBUTTON, {{ContainingHexX1,  5 }, {ContainingHexX2,  5 }}, DIF_NONE, ContainingHex, },
			{ DI_TEXT,        {{5,                7 }, {0,                7 }}, DIF_NONE, },
			{ DI_COMBOBOX,    {{5,                8 }, {DlgWidth-4-2,     8 }}, DIF_DROPDOWNLIST, },
			{ DI_TEXT,        {{-1,               9 }, {0,                9 }}, DIF_SEPARATOR, },
			{ DI_CHECKBOX,    {{5,                10}, {0,                10}}, DIF_NONE, msg(lng::MFindFileCase), },
			{ DI_CHECKBOX,    {{5,                11}, {0,                11}}, DIF_NONE, msg(lng::MFindFileWholeWords), },
			{ DI_CHECKBOX,    {{5,                12}, {0,                12}}, DIF_NONE, msg(lng::MFindFileFuzzy), },
			{ DI_CHECKBOX,    {{5,                13}, {0,                13}}, DIF_NONE, msg(lng::MFindFileNotContaining), },
			{ DI_CHECKBOX,    {{41,               10}, {0,                10}}, DIF_NONE, msg(lng::MFindFileArchives), },
			{ DI_CHECKBOX,    {{41,               11}, {0,                11}}, DIF_NONE, msg(lng::MFindFileFolders), },
			{ DI_CHECKBOX,    {{41,               12}, {0,                12}}, DIF_NONE, msg(lng::MFindFileSymLinks), },
			{ DI_CHECKBOX,    {{41,               13}, {0,                13}}, DIF_NONE, msg(lng::MFindFileAlternateStreams), },
			{ DI_TEXT,        {{-1,               14}, {0,                14}}, DIF_SEPARATOR, },
			{ DI_VTEXT,       {{39,               9 }, {39,               14}}, DIF_SEPARATORUSER, },
			{ DI_TEXT,        {{5,                15}, {0,                15}}, DIF_NONE, msg(lng::MFindFileSearchArea), },
			{ DI_COMBOBOX,    {{5,                16}, {36,               16}}, DIF_DROPDOWNLIST | DIF_LISTNOAMPERSAND, },
			{ DI_CHECKBOX,    {{41,               16}, {0,                16}}, DIF_AUTOMATION, msg(lng::MFindFileUseFilter), },
			{ DI_TEXT,        {{-1,               17}, {0,                17}}, DIF_SEPARATOR, },
			{ DI_BUTTON,      {{0,                18}, {0,                18}}, DIF_CENTERGROUP | DIF_DEFAULTBUTTON, msg(lng::MFindFileFind), },
			{ DI_BUTTON,      {{0,                18}, {0,                18}}, DIF_CENTERGROUP, msg(lng::MFindFileDrive), },
			{ DI_BUTTON,      {{0,                18}, {0,                18}}, DIF_CENTERGROUP | DIF_AUTOMATION | (UseFilter ? 0 : DIF_DISABLE), msg(lng::MFindFileSetFilter), },
			{ DI_BUTTON,      {{0,                18}, {0,                18}}, DIF_CENTERGROUP, msg(lng::MFindFileAdvanced), },
			{ DI_BUTTON,      {{0,                18}, {0,                18}}, DIF_CENTERGROUP, msg(lng::MCancel), },
		});

		FindAskDlg[FAD_EDIT_MASK].strHistory = L"Masks"sv;
		FindAskDlg[FAD_EDIT_TEXT].strHistory = L"SearchText"sv;
		FindAskDlg[FAD_EDIT_HEX].strMask = HexMask(23); // Fits without scrolling into the width of the edit control.
		FindAskDlg[FAD_CHECKBOX_NOTCONTAINING].Selected = m_Options.NotContaining;
		FindAskDlg[FAD_SEPARATOR_3].strMask = { BoxSymbols[BS_T_H1V1], BoxSymbols[BS_V1], BoxSymbols[BS_B_H1V1] };
		FindAskDlg[FAD_CHECKBOX_FILTER].Selected = UseFilter;
		FindAskDlg[FAD_CHECKBOX_DIRS].Selected = Global->Opt->FindOpt.FindFolders;

		FarListItem li[]=
		{
			{ 0, msg(lng::MFindFileSearchAllDisks).c_str() },
			{ 0, msg(lng::MFindFileSearchAllButNetwork).c_str() },
			{ 0, msg(lng::MFindFileSearchInPATH).c_str() },
			{ 0, strSearchFromRoot.c_str() },
			{ 0, msg(lng::MFindFileSearchFromCurrent).c_str() },
			{ 0, msg(lng::MFindFileSearchInCurrent).c_str() },
			{ 0, msg(lng::MFindFileSearchInSelected).c_str() },
		};

		static_assert(std::size(li) == FINDAREA_COUNT);

		li[FINDAREA_ALL + SearchMode].Flags|=LIF_SELECTED;
		FarList l{ sizeof(l), std::size(li), li };
		FindAskDlg[FAD_COMBOBOX_WHERE].ListItems=&l;

		if (PluginMode)
		{
			OpenPanelInfo Info;
			// no lock - background thread hasn't been started yet
			Global->CtrlObject->Plugins->GetOpenPanelInfo(ActivePanel->GetPluginHandle(),&Info);

			if (!(Info.Flags & OPIF_REALNAMES))
				FindAskDlg[FAD_CHECKBOX_ARC].Flags |= DIF_DISABLE;

			if (SearchMode == FINDAREA_ALL || SearchMode == FINDAREA_ALL_BUTNETWORK)
			{
				li[FINDAREA_ALL].Flags=0;
				li[FINDAREA_ALL_BUTNETWORK].Flags=0;
				li[FINDAREA_ROOT].Flags|=LIF_SELECTED;
			}

			li[FINDAREA_ALL].Flags|=LIF_GRAYED;
			li[FINDAREA_ALL_BUTNETWORK].Flags|=LIF_GRAYED;
			FindAskDlg[FAD_CHECKBOX_LINKS].Selected=0;
			FindAskDlg[FAD_CHECKBOX_LINKS].Flags|=DIF_DISABLE;
			FindAskDlg[FAD_CHECKBOX_STREAMS].Selected = 0;
			FindAskDlg[FAD_CHECKBOX_STREAMS].Flags |= DIF_DISABLE;
		}
		else
		{
			FindAskDlg[FAD_CHECKBOX_LINKS].Selected = Global->Opt->FindOpt.FindSymLinks;
			FindAskDlg[FAD_CHECKBOX_STREAMS].Selected = Global->Opt->FindOpt.FindAlternateStreams;
		}
		if (!(FindAskDlg[FAD_CHECKBOX_ARC].Flags & DIF_DISABLE))
			FindAskDlg[FAD_CHECKBOX_ARC].Selected = m_Options.SearchInArchives;

		FindAskDlg[FAD_EDIT_MASK].strData = m_FindMask;

		if (m_SearchDlgParams.Hex.value())
			FindAskDlg[FAD_EDIT_HEX].strData = BlobToHexString(m_SearchDlgParams.SearchBytes.value(), 0);
		else
			FindAskDlg[FAD_EDIT_TEXT].strData = m_SearchDlgParams.SearchStr;

		FindAskDlg[FAD_CHECKBOX_CASE].Selected = m_SearchDlgParams.CaseSensitive.value();
		FindAskDlg[FAD_CHECKBOX_WHOLEWORDS].Selected = m_SearchDlgParams.WholeWords.value();
		FindAskDlg[FAD_CHECKBOX_FUZZY].Selected = m_SearchDlgParams.Fuzzy.value();
		FindAskDlg[FAD_RADIO_TEXT].Selected = !m_SearchDlgParams.Hex.value();
		FindAskDlg[FAD_RADIO_HEX].Selected = m_SearchDlgParams.Hex.value();
		m_IsHexActive = m_SearchDlgParams.Hex.value();
		const auto Dlg = Dialog::create(FindAskDlg, std::bind_front(&FindFiles::MainDlgProc, this));
		Dlg->SetAutomation(FAD_CHECKBOX_FILTER,FAD_BUTTON_FILTER,DIF_DISABLE,DIF_NONE,DIF_NONE,DIF_DISABLE);
		Dlg->SetHelp(L"FindFile"sv);
		Dlg->SetId(FindFileId);
		Dlg->SetPosition({ -1, -1, 80, 21 });
		Dlg->Process();
		const auto ExitCode = Dlg->GetExitCode();
		//Рефреш текущему времени для фильтра сразу после выхода из диалога
		Filter->UpdateCurrentTime();

		if (ExitCode!=FAD_BUTTON_FIND)
		{
			return;
		}

		Global->Opt->FindCodePage = CodePage;
		m_Options.NotContaining = FindAskDlg[FAD_CHECKBOX_NOTCONTAINING].Selected == BSTATE_CHECKED;
		m_Options.SearchInArchives = FindAskDlg[FAD_CHECKBOX_ARC].Selected == BSTATE_CHECKED;
		Global->Opt->FindOpt.FindFolders = FindAskDlg[FAD_CHECKBOX_DIRS].Selected == BSTATE_CHECKED;

		if (!PluginMode)
		{
			Global->Opt->FindOpt.FindSymLinks=(FindAskDlg[FAD_CHECKBOX_LINKS].Selected==BSTATE_CHECKED);
			Global->Opt->FindOpt.FindAlternateStreams = (FindAskDlg[FAD_CHECKBOX_STREAMS].Selected == BSTATE_CHECKED);
		}

		UseFilter=(FindAskDlg[FAD_CHECKBOX_FILTER].Selected==BSTATE_CHECKED);
		Global->Opt->FindOpt.UseFilter=UseFilter;
		m_FindMask = !FindAskDlg[FAD_EDIT_MASK].strData.empty()? FindAskDlg[FAD_EDIT_MASK].strData : AllFilesMask;

		m_SearchDlgParams.Hex = m_IsHexActive;
		m_SearchDlgParams.SetSearchPattern(FindAskDlg[FAD_EDIT_TEXT].strData, FindAskDlg[FAD_EDIT_HEX].strData, CodePage);
		m_SearchDlgParams.CaseSensitive = FindAskDlg[FAD_CHECKBOX_CASE].Selected == BSTATE_CHECKED;
		m_SearchDlgParams.WholeWords = FindAskDlg[FAD_CHECKBOX_WHOLEWORDS].Selected == BSTATE_CHECKED;
		m_SearchDlgParams.Fuzzy = FindAskDlg[FAD_CHECKBOX_FUZZY].Selected == BSTATE_CHECKED;

		m_SearchDlgParams.SaveToShared(SearchReplaceDlgParams::SharedGroup::find_file);

		// gh-681: Empty search pattern means we do not use content search.
		// In this case, we should not touch viewer / editor dialog params.
		if (!m_SearchDlgParams.IsSearchPatternEmpty())
		{
			m_SearchDlgParams.SaveToShared(SearchReplaceDlgParams::SharedGroup::view_edit);
		}

		SearchMode = static_cast<FINDAREA>(FindAskDlg[FAD_COMBOBOX_WHERE].ListPos);

		if (SearchFromChanged)
		{
			Global->Opt->FindOpt.FileSearchMode=SearchMode;
		}

		LastFindFileOptions() = m_Options;
		LastFindMask() = m_FindMask;
	}
	while (FindFilesProcess());

	Global->CtrlObject->Cp()->ActivePanel()->RefreshTitle();
}

FindFiles::~FindFiles()
{
	ClearFindList();
}


background_searcher::background_searcher(
	FindFiles* Owner,
	FINDAREA SearchMode,
	ArcListItem* FindFileArcItem,
	uintptr_t CodePage,
	unsigned long long SearchInFirst,
	bool UseFilter,
	bool PluginMode):

	m_Owner(Owner),
	m_Autodetection(),
	SearchMode(SearchMode),
	m_FindFileArcItem(FindFileArcItem),
	CodePage(CodePage),
	SearchInFirst(SearchInFirst),
	m_Options(m_Owner->m_Options),
	m_SearchDlgParams(m_Owner->m_SearchDlgParams),
	UseFilter(UseFilter),
	m_PluginMode(PluginMode),
	PauseEvent(os::event::type::manual, os::event::state::signaled),
	StopEvent(os::event::type::manual, os::event::state::nonsignaled)
{
}

void find_files()
{
	FindFiles();
}
