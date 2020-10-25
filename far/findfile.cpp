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
#include "syslog.hpp"
#include "encoding.hpp"
#include "taskbar.hpp"
#include "interf.hpp"
#include "message.hpp"
#include "delete.hpp"
#include "datetime.hpp"
#include "pathmix.hpp"
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

// Platform:
#include "platform.concurrency.hpp"
#include "platform.env.hpp"
#include "platform.fs.hpp"

// Common:
#include "common/bytes_view.hpp"
#include "common/enum_tokens.hpp"
#include "common/scope_exit.hpp"

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

// Список найденных файлов. Индекс из списка хранится в меню.
struct FindListItem
{
	os::fs::find_data FindData;
	FindFiles::ArcListItem* Arc;
	DWORD Used;
	UserDataItem UserData;
};

// TODO BUGBUG DELETE THIS
class InterThreadData
{
private:
	mutable os::critical_section DataCS;
	FindFiles::ArcListItem* FindFileArcItem;
	int Percent;

	std::list<FindListItem> FindList;
	std::list<FindFiles::ArcListItem> ArcList;
	string strFindMessage;

public:
	InterThreadData() {Init();}
	~InterThreadData() { ClearAllLists(); }

	void Init()
	{
		SCOPED_ACTION(std::lock_guard)(DataCS);
		FindFileArcItem = nullptr;
		Percent=0;
		FindList.clear();
		ArcList.clear();
		strFindMessage.clear();
	}


	FindFiles::ArcListItem* GetFindFileArcItem() const
	{
		SCOPED_ACTION(std::lock_guard)(DataCS);
		return FindFileArcItem;
	}

	void SetFindFileArcItem(FindFiles::ArcListItem* Value)
	{
		SCOPED_ACTION(std::lock_guard)(DataCS);
		FindFileArcItem = Value;
	}

	int GetPercent() const { return Percent; }

	void SetPercent(int Value)
	{
		SCOPED_ACTION(std::lock_guard)(DataCS);
		Percent = Value;
	}

	size_t GetFindListCount() const
	{
		SCOPED_ACTION(std::lock_guard)(DataCS);
		return FindList.size();
	}

	string GetFindMessage() const
	{
		SCOPED_ACTION(std::lock_guard)(DataCS);
		return strFindMessage;
	}

	void SetFindMessage(string_view const From)
	{
		SCOPED_ACTION(std::lock_guard)(DataCS);
		strFindMessage=From;
	}

	void ClearAllLists()
	{
		SCOPED_ACTION(std::lock_guard)(DataCS);
		FindFileArcItem = nullptr;

		if (!FindList.empty())
		{
			for (const auto& i: FindList)
			{
				FreePluginPanelItemUserData(i.Arc? i.Arc->hPlugin : nullptr, i.UserData);
			}

			FindList.clear();
		}

		ArcList.clear();
	}

	FindFiles::ArcListItem& AddArcListItem(string_view const ArcName, plugin_panel* const hPlugin, unsigned long long const Flags, string_view const RootPath)
	{
		SCOPED_ACTION(std::lock_guard)(DataCS);

		FindFiles::ArcListItem NewItem;
		NewItem.strArcName = ArcName;
		NewItem.hPlugin = hPlugin;
		NewItem.Flags = Flags;
		NewItem.strRootPath = RootPath;
		AddEndSlash(NewItem.strRootPath);
		ArcList.emplace_back(NewItem);
		return ArcList.back();
	}

	FindListItem& AddFindListItem(const os::fs::find_data& FindData, void* Data, FARPANELITEMFREECALLBACK FreeData)
	{
		SCOPED_ACTION(std::lock_guard)(DataCS);

		FindListItem NewItem;
		NewItem.FindData = FindData;
		NewItem.Arc = nullptr;
		NewItem.UserData = {Data, FreeData};
		FindList.emplace_back(NewItem);
		return FindList.back();
	}

	template <typename Visitor>
	void ForEachFindItem(const Visitor& visitor) const
	{
		SCOPED_ACTION(std::lock_guard)(DataCS);
		for (const auto& i: FindList)
			visitor(i);
	}

	template <typename Visitor>
	void ForEachFindItem(const Visitor& visitor)
	{
		SCOPED_ACTION(std::lock_guard)(DataCS);
		for (auto& i: FindList)
			visitor(i);
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
	FAD_TEXT_TEXTHEX,
	FAD_EDIT_TEXT,
	FAD_EDIT_HEX,
	FAD_TEXT_CP,
	FAD_COMBOBOX_CP,
	FAD_SEPARATOR1,
	FAD_CHECKBOX_CASE,
	FAD_CHECKBOX_WHOLEWORDS,
	FAD_CHECKBOX_HEX,
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
		string FindString,
		FINDAREA SearchMode,
		uintptr_t CodePage,
		unsigned long long SearchInFirst,
		bool CmpCase,
		bool WholeWords,
		bool SearchInArchives,
		bool SearchHex,
		bool NotContaining,
		bool UseFilter,
		bool PluginMode
	);

	void Search();
	void Pause() const { PauseEvent.reset(); }
	void Resume() const { PauseEvent.set(); }
	void Stop() const { PauseEvent.set(); StopEvent.set(); }
	bool Stopped() const { return StopEvent.is_signaled(); }
	bool Finished() const { return m_Finished; }

	auto ExceptionPtr() const { return m_ExceptionPtr; }
	auto IsRegularException() const { return m_IsRegularException; }

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
	bytes hexFindString;
	struct CodePageInfo;
	std::list<CodePageInfo> m_CodePages;
	string strPluginSearchPath;

	bool m_Autodetection;

	const string strFindStr;
	const FINDAREA SearchMode;
	const uintptr_t CodePage;
	const unsigned long long SearchInFirst;
	const bool CmpCase;
	const bool WholeWords;
	const bool SearchInArchives;
	const bool SearchHex;
	const bool NotContaining;
	const bool UseFilter;
	const bool m_PluginMode;

	os::event PauseEvent;
	os::event StopEvent;
	std::atomic_bool m_Finished{};

	std::exception_ptr m_ExceptionPtr;
	bool m_IsRegularException{};

	template<typename... args>
	using searcher = std::boyer_moore_horspool_searcher<args...>;

	using case_sensitive_searcher = searcher<string::const_iterator>;
	using case_insensitive_searcher = searcher<string::const_iterator, hash_icase_t, equal_icase_t>;
	using hex_searcher = searcher<bytes::const_iterator>;

	std::variant
	<
		bool, // Just to make it default-constructible
		case_sensitive_searcher,
		case_insensitive_searcher,
		hex_searcher
	>
	m_Searcher;

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

	void initialize()
	{
		if (IsUnicodeCodePage(CodePage))
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
	if (strFindStr.empty())
		return;

	// Инициализируем буферы чтения из файла
	const size_t readBufferSize = 32768;

	readBufferA.resize(readBufferSize);
	readBuffer.resize(readBufferSize);

	if (!SearchHex)
	{
		if (CmpCase)
			m_Searcher.emplace<case_sensitive_searcher>(ALL_CONST_RANGE(strFindStr));
		else
			m_Searcher.emplace<case_insensitive_searcher>(ALL_CONST_RANGE(strFindStr));

		// Формируем список кодовых страниц
		if (CodePage == CP_ALL)
		{
			// Проверяем наличие выбранных страниц символов
			const auto CpEnum = codepages::GetFavoritesEnumerator();
			const auto hasSelected = std::any_of(CONST_RANGE(CpEnum, i) { return i.second & CPST_FIND; });

			if (hasSelected)
			{
				m_CodePages.clear();
			}
			else
			{
				// Добавляем стандартные таблицы символов
				const uintptr_t Predefined[] = { encoding::codepage::oem(), encoding::codepage::ansi(), CP_UTF8, CP_UNICODE, CP_REVERSEBOM };
				m_CodePages.insert(m_CodePages.end(), ALL_CONST_RANGE(Predefined));
			}

			// Добавляем избранные таблицы символов
			for (const auto& [Name, Value]: CpEnum)
			{
				if (Value & (hasSelected? CPST_FIND : CPST_FAVORITE))
				{
					// Проверяем дубли
					// TODO: P1091R3
					if (hasSelected || !std::any_of(ALL_CONST_RANGE(m_CodePages), [&Name = Name](const CodePageInfo& cp) { return cp.CodePage == Name; }))
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
		}
	}
	else
	{
		// Формируем hex-строку для поиска
		hexFindString = HexStringToBlob(strFindStr, 0);

		// Инициализируем данные для аглоритма поиска
		m_Searcher.emplace<hex_searcher>(ALL_CONST_RANGE(hexFindString));
	}
}

void background_searcher::ReleaseInFileSearch()
{
	clear_and_shrink(readBufferA);
	clear_and_shrink(readBuffer);
	hexFindString = {};
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
		strSearchFromRoot = msg(lng::MSearchFromRootFolder);
	}
	else
	{
		strSearchFromRoot = concat(msg(lng::MSearchFromRootOfDrive), L' ', strCurDir);
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
				span<PluginPanelItem> PanelData;

				SCOPED_ACTION(std::lock_guard)(PluginCS);
				if (Global->CtrlObject->Plugins->GetFindData(hPlugin, PanelData, OPM_SILENT))
					Global->CtrlObject->Plugins->FreeFindData(hPlugin, PanelData, true);
			}

			SCOPED_ACTION(std::lock_guard)(PluginCS);
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
				const auto Data = reinterpret_cast<const wchar_t*>(Dlg->SendMessage(DM_GETCONSTTEXTPTR, AD_EDIT_SEARCHFIRST, nullptr));

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
		const auto Title = msg(std::any_of(CONST_RANGE(CpEnum, i) { return i.second & CPST_FIND; })? lng::MFindFileSelectedCodePages : lng::MFindFileAllCodePages);
		Dlg->GetAllItem()[FAD_COMBOBOX_CP].ListPtr->at(TitlePosition).Name = Title;
		FarListPos Position = { sizeof(FarListPos) };
		Dlg->SendMessage(DM_LISTGETCURPOS, FAD_COMBOBOX_CP, &Position);
		if (Position.SelectPos == TitlePosition)
			Dlg->SendMessage(DM_SETTEXTPTR, FAD_COMBOBOX_CP, UNSAFE_CSTR(Title));
	};

	switch (Msg)
	{
		case DN_INITDIALOG:
		{
			bool Hex = (Dlg->SendMessage(DM_GETCHECK, FAD_CHECKBOX_HEX, nullptr) == BSTATE_CHECKED);
			Dlg->SendMessage(DM_SHOWITEM,FAD_EDIT_TEXT,ToPtr(!Hex));
			Dlg->SendMessage(DM_SHOWITEM,FAD_EDIT_HEX,ToPtr(Hex));
			Dlg->SendMessage(DM_ENABLE,FAD_TEXT_CP,ToPtr(!Hex));
			Dlg->SendMessage(DM_ENABLE,FAD_COMBOBOX_CP,ToPtr(!Hex));
			Dlg->SendMessage(DM_ENABLE,FAD_CHECKBOX_CASE,ToPtr(!Hex));
			Dlg->SendMessage(DM_ENABLE,FAD_CHECKBOX_WHOLEWORDS,ToPtr(!Hex));
			Dlg->SendMessage(DM_ENABLE,FAD_CHECKBOX_DIRS,ToPtr(!Hex));
			Dlg->SendMessage(DM_EDITUNCHANGEDFLAG,FAD_EDIT_TEXT,ToPtr(1));
			Dlg->SendMessage(DM_EDITUNCHANGEDFLAG,FAD_EDIT_HEX,ToPtr(1));
			Dlg->SendMessage(DM_SETTEXTPTR,FAD_TEXT_TEXTHEX,const_cast<wchar_t*>(msg(Hex? lng::MFindFileHex : lng::MFindFileText).c_str()));
			Dlg->SendMessage(DM_SETTEXTPTR,FAD_TEXT_CP,const_cast<wchar_t*>(msg(lng::MFindFileCodePage).c_str()));
			Dlg->SendMessage(DM_SETCOMBOBOXEVENT,FAD_COMBOBOX_CP,ToPtr(CBET_KEY));
			FarListTitles Titles={sizeof(FarListTitles),0,nullptr,0,msg(lng::MFindFileCodePageBottom).c_str()};
			Dlg->SendMessage(DM_LISTSETTITLES,FAD_COMBOBOX_CP,&Titles);
			// Установка запомненных ранее параметров
			CodePage = Global->Opt->FindCodePage;
			favoriteCodePages = static_cast<int>(codepages::instance().FillCodePagesList(Dlg, FAD_COMBOBOX_CP, CodePage, true, true, false, true, false));
			SetAllCpTitle();

			// Текущее значение в списке выбора кодовых страниц в общем случае может не совпадать с CodePage,
			// так что получаем CodePage из списка выбора
			FarListPos Position={sizeof(FarListPos)};
			Dlg->SendMessage( DM_LISTGETCURPOS, FAD_COMBOBOX_CP, &Position);
			FarListGetItem Item = { sizeof(FarListGetItem), Position.SelectPos };
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
					string Mask(reinterpret_cast<const wchar_t*>(Dlg->SendMessage(DM_GETCONSTTEXTPTR, FAD_EDIT_MASK, nullptr)));

					if (Mask.empty())
						Mask = L"*"sv;

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
					FarListGetItem item{ sizeof(FarListGetItem), FINDAREA_ROOT };
					Dlg->SendMessage(DM_LISTGETITEM,FAD_COMBOBOX_WHERE,&item);
					item.Item.Text=strSearchFromRoot.c_str();
					Dlg->SendMessage(DM_LISTUPDATE,FAD_COMBOBOX_WHERE,&item);
					PluginMode = Global->CtrlObject->Cp()->ActivePanel()->GetMode() == panel_mode::PLUGIN_PANEL;
					Dlg->SendMessage(DM_ENABLE,FAD_CHECKBOX_DIRS,ToPtr(!PluginMode));
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
					Filter->FilterEdit();
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
				case FAD_CHECKBOX_DIRS:
					{
						FindFoldersChanged = true;
					}
					break;

				case FAD_CHECKBOX_HEX:
				{
					SCOPED_ACTION(Dialog::suppress_redraw)(Dlg);

					const auto Src = reinterpret_cast<const wchar_t*>(Dlg->SendMessage(DM_GETCONSTTEXTPTR, Param2 ? FAD_EDIT_TEXT : FAD_EDIT_HEX, nullptr));
					const auto strDataStr = ConvertHexString(Src, CodePage, !Param2);
					Dlg->SendMessage(DM_SETTEXTPTR,Param2?FAD_EDIT_HEX:FAD_EDIT_TEXT, UNSAFE_CSTR(strDataStr));
					const auto iParam = reinterpret_cast<intptr_t>(Param2);
					Dlg->SendMessage(DM_SHOWITEM,FAD_EDIT_TEXT,ToPtr(!iParam));
					Dlg->SendMessage(DM_SHOWITEM,FAD_EDIT_HEX,ToPtr(iParam));
					Dlg->SendMessage(DM_ENABLE,FAD_TEXT_CP,ToPtr(!iParam));
					Dlg->SendMessage(DM_ENABLE,FAD_COMBOBOX_CP,ToPtr(!iParam));
					Dlg->SendMessage(DM_ENABLE,FAD_CHECKBOX_CASE,ToPtr(!iParam));
					Dlg->SendMessage(DM_ENABLE,FAD_CHECKBOX_WHOLEWORDS,ToPtr(!iParam));
					Dlg->SendMessage(DM_ENABLE,FAD_CHECKBOX_DIRS,ToPtr(!iParam));
					Dlg->SendMessage(DM_SETTEXTPTR,FAD_TEXT_TEXTHEX, UNSAFE_CSTR(msg(Param2? lng::MFindFileHex : lng::MFindFileText)));

					if (!strDataStr.empty())
					{
						const auto UnchangeFlag = static_cast<int>(Dlg->SendMessage(DM_EDITUNCHANGEDFLAG, FAD_EDIT_TEXT, ToPtr(-1)));
						Dlg->SendMessage(DM_EDITUNCHANGEDFLAG,FAD_EDIT_HEX,ToPtr(UnchangeFlag));
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
							FarListPos Position={sizeof(FarListPos)};
							Dlg->SendMessage( DM_LISTGETCURPOS, FAD_COMBOBOX_CP, &Position);
							// Получаем номер выбранной таблицы символов
							FarListGetItem Item = { sizeof(FarListGetItem), Position.SelectPos };
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

								FarListPos Pos={sizeof(FarListPos),Position.SelectPos+1,Position.TopPos};
								Dlg->SendMessage( DM_LISTSETCURPOS, FAD_COMBOBOX_CP,&Pos);

								// Обрабатываем случай, когда таблица символов может присутствовать, как в стандартных, так и в избранных,
								// т.е. выбор/снятие флага автоматически происходит у обоих элементов
								bool bStandardCodePage = Position.SelectPos < FavoritesIndex;

								for (int Index = bStandardCodePage ? FavoritesIndex : 0; Index < (bStandardCodePage ? FavoritesIndex + favoriteCodePages : FavoritesIndex); Index++)
								{
									// Получаем элемент таблицы символов
									FarListGetItem CheckItem = { sizeof(FarListGetItem), Index };
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
			auto& Item=*static_cast<FarDialogItem*>(Param2);

			switch (Param1)
			{
				case FAD_EDIT_TEXT:
					// Строка "Содержащих текст"
					if (!FindFoldersChanged)
					{
						const auto Checked = Item.Data && *Item.Data? false : Global->Opt->FindOpt.FindFolders;
						Dlg->SendMessage( DM_SETCHECK, FAD_CHECKBOX_DIRS, ToPtr(Checked? BSTATE_CHECKED : BSTATE_UNCHECKED));
					}
					return TRUE;

				case FAD_COMBOBOX_CP:
					// Получаем выбранную в выпадающем списке таблицу символов
					CodePage = Dlg->GetListItemSimpleUserData(FAD_COMBOBOX_CP, Dlg->SendMessage(DM_LISTGETCURPOS, FAD_COMBOBOX_CP, nullptr));
					return TRUE;

				case FAD_COMBOBOX_WHERE:
					SearchFromChanged = true;
					return TRUE;
			}
		}
		[[fallthrough]];
		case DN_HOTKEY:
			if (Param1==FAD_TEXT_TEXTHEX)
			{
				Dlg->SendMessage(DM_SETFOCUS, FAD_EDIT_HEX, nullptr); // only one
				Dlg->SendMessage(DM_SETFOCUS, FAD_EDIT_TEXT, nullptr); // is active
				return FALSE;
			}
		[[fallthrough]];
		default:
			break;
	}

	return Dlg->DefProc(Msg,Param1,Param2);
}

bool FindFiles::GetPluginFile(ArcListItem const* const ArcItem, const os::fs::find_data& FindData, const string& DestPath, string &strResultName, const UserDataItem* const UserData)
{
	SCOPED_ACTION(std::lock_guard)(PluginCS);
	_ALGO(CleverSysLog clv(L"FindFiles::GetPluginFile()"));
	OpenPanelInfo Info;

	Global->CtrlObject->Plugins->GetOpenPanelInfo(ArcItem->hPlugin,&Info);
	string strSaveDir = NullToEmpty(Info.CurDir);
	AddEndSlash(strSaveDir);
	Global->CtrlObject->Plugins->SetDirectory(ArcItem->hPlugin, L"\\"s, OPM_SILENT);
	SetPluginDirectory(FindData.FileName,ArcItem->hPlugin,false,UserData);
	const auto FileNameToFind = PointToName(FindData.FileName);
	const auto FileNameToFindShort = FindData.HasAlternateFileName()? PointToName(FindData.AlternateFileName()) : string_view{};
	span<PluginPanelItem> Items;
	bool nResult=false;

	if (Global->CtrlObject->Plugins->GetFindData(ArcItem->hPlugin, Items, OPM_SILENT))
	{
		const auto It = std::find_if(ALL_CONST_RANGE(Items), [&](const auto& Item)
		{
			return FileNameToFind == NullToEmpty(Item.FileName) && FileNameToFindShort == NullToEmpty(Item.AlternateFileName);
		});

		if (It != Items.cend())
		{
			nResult = Global->CtrlObject->Plugins->GetFile(ArcItem->hPlugin, &*It, DestPath, strResultName, OPM_SILENT) != 0;
		}

		Global->CtrlObject->Plugins->FreeFindData(ArcItem->hPlugin, Items, true);
	}

	Global->CtrlObject->Plugins->SetDirectory(ArcItem->hPlugin, L"\\"s, OPM_SILENT);
	SetPluginDirectory(strSaveDir, ArcItem->hPlugin, false, nullptr);
	return nResult;
}

bool background_searcher::LookForString(string_view const FileName)
{
	// Длина строки поиска
	const auto findStringCount = strFindStr.size();

	// Открываем файл
	const os::fs::file File(FileName, FILE_READ_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN);
	if(!File)
	{
		return false;
	}

	if (m_Autodetection)
	{
		m_CodePages.front().CodePage = GetFileCodepage(File, encoding::codepage::ansi());
		m_CodePages.front().initialize();
	}

	// Количество считанных из файла байт
	size_t readBlockSize = 0;
	// Количество прочитанных из файла байт
	unsigned long long alreadyRead = 0;
	// Смещение на которое мы отступили при переходе между блоками
	size_t offset = 0;

	if (SearchHex)
		offset = hexFindString.size() - 1;

	unsigned long long FileSize = 0;
	// BUGBUG check result
	(void)File.GetSize(FileSize);

	if (SearchInFirst)
	{
		FileSize=std::min(SearchInFirst,FileSize);
	}

	unsigned LastPercents{};

	// Основной цикл чтения из файла
	while (!Stopped() && File.Read(readBufferA.data(), (!SearchInFirst || alreadyRead + readBufferA.size() <= SearchInFirst)? readBufferA.size() : SearchInFirst - alreadyRead, readBlockSize))
	{
		const auto Percents = ToPercent(alreadyRead, FileSize);

		if (Percents!=LastPercents)
		{
			m_Owner->itd->SetPercent(Percents);
			LastPercents=Percents;
		}

		// Увеличиваем счётчик прочитаннх байт
		alreadyRead += readBlockSize;

		// Для hex и обыкновенного поиска разные ветки
		if (SearchHex)
		{
			// Выходим, если прочитали мало
			if (readBlockSize < hexFindString.size())
				return false;

			// Ищем
			const auto Begin = readBufferA.data(), End = Begin + readBlockSize;
			if (std::search(Begin, End, std::get<hex_searcher>(m_Searcher)) != End)
				return true;
		}
		else
		{
			bool ErrorState = false;
			for (auto& i: m_CodePages)
			{
				ErrorState = false;
				// Пропускаем ошибочные кодовые страницы
				if (!i.MaxCharSize)
				{
					ErrorState = true;
					continue;
				}

				// Если начало файла очищаем информацию о поиске по словам
				if (WholeWords && alreadyRead==readBlockSize)
				{
					i.WordFound = false;
					i.LastSymbol = 0;
				}

				// Если ничего не прочитали
				if (!readBlockSize)
				{
					// Если поиск по словам и в конце предыдущего блока было что-то найдено,
					// то считаем, что нашли то, что нужно
					if(WholeWords && i.WordFound)
						return true;
					else
					{
						ErrorState = true;
						continue;
					}
					// Выходим, если прочитали меньше размера строки поиска и нет поиска по словам
				}

				if (readBlockSize < findStringCount && !(WholeWords && i.WordFound))
				{
					ErrorState = true;
					continue;
				}

				// Количество символов в выходном буфере
				size_t bufferCount;

				// Буфер для поиска
				wchar_t const *buffer;

				// Перегоняем буфер в UTF-16
				if (IsUnicodeCodePage(i.CodePage))
				{
					// Вычисляем размер буфера в UTF-16
					bufferCount = readBlockSize/sizeof(wchar_t);

					// Выходим, если размер буфера меньше длины строки поиска
					if (bufferCount < findStringCount)
					{
						ErrorState = true;
						continue;
					}

					// Копируем буфер чтения в буфер сравнения
					if (i.CodePage==CP_REVERSEBOM)
					{
						// Для UTF-16 (big endian) преобразуем буфер чтения в буфер сравнения
						swap_bytes(readBufferA.data(), readBuffer.data(), readBlockSize);
						// Устанавливаем буфер сравнения
						buffer = readBuffer.data();
					}
					else
					{
						// Если поиск в UTF-16 (little endian), то используем исходный буфер
						buffer = reinterpret_cast<wchar_t*>(readBufferA.data());
					}
				}
				else
				{
					// Конвертируем буфер чтения из кодировки поиска в UTF-16
					bufferCount = encoding::get_chars(i.CodePage, { readBufferA.data(), readBlockSize }, readBuffer);

					// Выходим, если нам не удалось сконвертировать строку
					if (!bufferCount)
					{
						ErrorState = true;
						continue;
					}

					// Если у нас поиск по словам и в конце предыдущего блока было вхождение
					if (WholeWords && i.WordFound)
					{
						// Если конец файла, то считаем, что есть разделитель в конце
						if (findStringCount-1>=bufferCount)
							return true;

						// Проверяем первый символ текущего блока с учётом обратного смещения, которое делается
						// при переходе между блоками
						i.LastSymbol = readBuffer[findStringCount-1];

						if (FindFiles::IsWordDiv(i.LastSymbol))
							return true;

						// Если размер буфера меньше размера слова, то выходим
						if (readBlockSize < findStringCount)
						{
							ErrorState = true;
							continue;
						}
					}

					// Устанавливаем буфер сравнения
					buffer = readBuffer.data();
				}

				i.WordFound = false;

				auto Iterator = buffer;
				const auto End = buffer + bufferCount;

				do
				{
					// Ищем подстроку в буфере и возвращаем индекс её начала в случае успеха
					const auto NewIterator = CmpCase?
						std::search(Iterator, End, std::get<case_sensitive_searcher>(m_Searcher)):
						std::search(Iterator, End, std::get<case_insensitive_searcher>(m_Searcher));

					// Если подстрока не найдена идём на следующий шаг
					if (NewIterator == End)
						break;

					// Если подстрока найдена и отключен поиск по словам, то считаем что всё хорошо
					if (!WholeWords)
						return true;
					// Устанавливаем позицию в исходном буфере
					Iterator = NewIterator;

					// Если идёт поиск по словам, то делаем соответствующие проверки
					bool firstWordDiv = false;

					// Если мы находимся вначале блока
					if (Iterator == buffer)
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
						i.LastSymbol = *std::prev(Iterator);

						if (FindFiles::IsWordDiv(i.LastSymbol))
							firstWordDiv = true;
					}

					// Проверяем разделитель в конце, только если найден разделитель вначале
					if (firstWordDiv)
					{
						// Если блок выбран не до конца
						if (Iterator + findStringCount != End)
						{
							// Проверяем является или нет последующий за найденным символ блока разделителем
							i.LastSymbol = *std::next(Iterator, findStringCount);

							if (FindFiles::IsWordDiv(i.LastSymbol))
								return true;
						}
						else
							i.WordFound = true;
					}
				}
				while (++Iterator != End - findStringCount);

				// Выходим, если мы вышли за пределы количества байт разрешённых для поиска
				if (SearchInFirst && alreadyRead >= SearchInFirst)
				{
					ErrorState = true;
					continue;
				}
				// Запоминаем последний символ блока
				i.LastSymbol = buffer[bufferCount-1];
			}

			if (ErrorState)
				return false;

			// Получаем смещение на которое мы отступили при переходе между блоками
			offset = (CodePage == CP_ALL? sizeof(wchar_t) : m_CodePages.begin()->MaxCharSize) * (findStringCount - 1);
		}

		// Если мы потенциально прочитали не весь файл
		if (readBlockSize == readBuffer.size())
		{
			// Отступаем назад на длину слова поиска минус 1
			if (!File.SetPointer(-1ll*offset, nullptr, FILE_CURRENT))
				return false;
			alreadyRead -= offset;
		}
	}

	return false;
}

bool background_searcher::IsFileIncluded(PluginPanelItem* FileItem, string_view const FullName, os::fs::attributes FileAttr, string_view const DisplayName)
{
	if (!m_Owner->GetFileMask()->check(PointToName(FullName)))
		return false;

	const auto ArcItem = m_Owner->itd->GetFindFileArcItem();
	const auto hPlugin = ArcItem? ArcItem->hPlugin : nullptr;

	if (FileAttr & FILE_ATTRIBUTE_DIRECTORY)
		return Global->Opt->FindOpt.FindFolders && strFindStr.empty();

	if (strFindStr.empty())
		return true;

	m_Owner->itd->SetFindMessage(DisplayName);

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
				(void)os::fs::remove_directory(strTempDir);
				return false;
			}

			RemoveTemp = true;
		}
	}

	return LookForString(strSearchFileName) ^ NotContaining;
}

static void clear_queue(os::synced_queue<FindFiles::AddMenuData>& Messages)
{
	SCOPED_ACTION(auto)(Messages.scoped_lock());

	FindFiles::AddMenuData Data;
	while (Messages.try_pop(Data))
	{
		FreePluginPanelItemUserData(Data.m_Arc? Data.m_Arc->hPlugin : nullptr, { Data.m_Data, Data.m_FreeData });
	}
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

	static int Recurse = 0;
	static int Drawing = 0;
	switch (Msg)
	{
	case DN_INITDIALOG:
		Drawing = 0;
		break;
	case DN_DRAWDIALOG:
		++Drawing;
		break;
	case DN_DRAWDIALOGDONE:
		--Drawing;
		break;
	default:
		if (!Finalized && !Recurse && !Drawing)
		{
			++Recurse;
			SCOPE_EXIT{ --Recurse; };
			{
				size_t EventsCount = 0;
				const time_check TimeCheck;
				while (!m_Messages.empty() && 0 == EventsCount)
				{
					if (m_Searcher->Stopped())
					{
						// The request to stop might arrive in the middle of something and searcher can still pump some messages
						while (!m_Searcher->Finished())
						{
							clear_queue(m_Messages);
						}
					}

					if (TimeCheck)
					{
						Global->WindowManager->CallbackWindow([]()
						{
							const auto f = Global->WindowManager->GetCurrentWindow();
							if (windowtype_dialog == f->GetType())
								std::static_pointer_cast<Dialog>(f)->SendMessage(DN_ENTERIDLE, 0, nullptr);
						});
						break;
					}
					AddMenuData Data;
					if (m_Messages.try_pop(Data))
					{
						ProcessMessage(Data);
					}
					console.GetNumberOfInputEvents(EventsCount);
				}
			}
		}
	}

	if(m_TimeCheck && !Finalized && !Recurse)
	{
		++Recurse;
		SCOPE_EXIT{ --Recurse; };

		if (!m_Searcher->Finished())
		{
			const auto strDataStr = format(msg(lng::MFindFound), m_FileCount, m_DirCount);
			Dlg->SendMessage(DM_SETTEXTPTR,FD_SEPARATOR1, UNSAFE_CSTR(strDataStr));

			string strSearchStr;

			if (!strFindStr.empty())
			{
				strSearchStr = format(msg(lng::MFindSearchingIn), quote_unconditional(truncate_right(strFindStr, 10)));
			}

			auto strFM = itd->GetFindMessage();
			SMALL_RECT Rect;
			Dlg->SendMessage(DM_GETITEMPOSITION, FD_TEXT_STATUS, &Rect);

			if (!strSearchStr.empty())
			{
				strSearchStr += L' ';
			}

			inplace::truncate_center(strFM, Rect.Right - Rect.Left + 1 - strSearchStr.size());
			Dlg->SendMessage(DM_SETTEXTPTR, FD_TEXT_STATUS, UNSAFE_CSTR(strSearchStr + strFM));
			if (!strFindStr.empty())
			{
				Dlg->SendMessage(DM_SETTEXTPTR, FD_TEXT_STATUS_PERCENTS, UNSAFE_CSTR(format(FSTR(L"{0:3}%"), itd->GetPercent())));
			}

			if (m_LastFoundNumber)
			{
				m_LastFoundNumber = 0;

				if (ListBox->UpdateRequired())
					Dlg->SendMessage(DM_SHOWITEM,FD_LISTBOX,ToPtr(1));
			}
		}
	}

	if(!Recurse && !Finalized && m_Searcher->Finished() && m_Messages.empty())
	{
		Finalized = true;

		SCOPED_ACTION(Dialog::suppress_redraw)(Dlg);
		const auto strMessage = format(msg(lng::MFindDone), m_FileCount, m_DirCount);
		Dlg->SendMessage( DM_SETTEXTPTR, FD_SEPARATOR1, nullptr);
		Dlg->SendMessage( DM_SETTEXTPTR, FD_TEXT_STATUS, UNSAFE_CSTR(strMessage));
		Dlg->SendMessage( DM_SETTEXTPTR, FD_TEXT_STATUS_PERCENTS, nullptr);
		Dlg->SendMessage( DM_SETTEXTPTR, FD_BUTTON_STOP, const_cast<wchar_t*>(msg(lng::MFindCancel).c_str()));
		ConsoleTitle::SetFarTitle(strMessage);
	}

	switch (Msg)
	{
	case DN_INITDIALOG:
		{
			Dlg->GetAllItem()[FD_LISTBOX].ListPtr->SetMenuFlags(VMENU_NOMERGEBORDER);
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
			if (record->EventType!=KEY_EVENT) break;
			int key = InputRecordToKey(record);
			switch (key)
			{
			case KEY_ESC:
			case KEY_F10:
				{
					if (!m_Searcher->Finished())
					{
						m_Searcher->Pause();

						ConfirmAbortOp()?
							m_Searcher->Stop() :
							m_Searcher->Resume();

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
									SCOPED_ACTION(std::lock_guard)(PluginCS);
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
								SCOPED_ACTION(std::lock_guard)(PluginCS);
								Global->CtrlObject->Plugins->ClosePanel(std::move(PluginPanelPtr));
								FindItem->Arc->hPlugin = nullptr;
							}

							if (!bGet)
							{
								// BUGBUG check result
								(void)os::fs::remove_directory(strTempDir);
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
				m_Searcher->Stop();
				return FALSE;

			case FD_BUTTON_STOP:
				// As Stop
				if (!m_Searcher->Finished())
				{
					m_Searcher->Stop();
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
				m_Searcher->Stop();
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

			for (int i = 0; i < FD_SEPARATOR1; i++)
			{
				SMALL_RECT rect;
				Dlg->SendMessage( DM_GETITEMPOSITION, i, &rect);
				rect.Right += IncX;
				rect.Bottom += IncY;
				Dlg->SendMessage( DM_SETITEMPOSITION, i, &rect);
			}

			for (int i = FD_SEPARATOR1; i <= FD_BUTTON_STOP; i++)
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

void FindFiles::OpenFile(const string& strSearchFileName, int OpenKey, const FindListItem* FindItem, Dialog* Dlg) const
{
	if (!os::fs::exists(strSearchFileName))
		return;

	auto openMode = FILETYPE_VIEW;
	auto shouldForceInternal = false;
	const auto isKnownKey = GetFiletypeOpenMode(OpenKey, openMode, shouldForceInternal);

	assert(isKnownKey); // ensure all possible keys are handled

	if (!isKnownKey)
		return;

	const auto strOldTitle = console.GetTitle();
	const auto shortFileName = ExtractFileName(strSearchFileName);

	if (shouldForceInternal || !ProcessLocalFileTypes(strSearchFileName, shortFileName, openMode, PluginMode))
	{
		if (openMode == FILETYPE_ALTVIEW && Global->Opt->strExternalViewer.empty())
			openMode = FILETYPE_VIEW;

		if (openMode == FILETYPE_ALTEDIT && Global->Opt->strExternalEditor.empty())
			openMode = FILETYPE_EDIT;

		if (openMode == FILETYPE_VIEW)
		{
			NamesList ViewList;

			// Возьмем все файлы, которые имеют реальные имена...
			itd->ForEachFindItem([&ViewList](const FindListItem& i)
			{
				if ((i.Arc && !(i.Arc->Flags & OPIF_REALNAMES)) || i.FindData.FileName.empty() || i.FindData.Attributes & FILE_ATTRIBUTE_DIRECTORY)
					return;

				ViewList.AddName(i.FindData.FileName);
			});

			ViewList.SetCurName(FindItem->FindData.FileName);

			const auto ShellViewer = FileViewer::create(
				strSearchFileName,
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
			const auto ShellEditor = FileEditor::create(strSearchFileName, CP_DEFAULT, 0);
			ShellEditor->SetEnableF6(true);

			if (FindItem->Arc && !(FindItem->Arc->Flags & OPIF_REALNAMES))
				ShellEditor->SetSaveToSaveAs(true);

			if (-1 == ShellEditor->GetExitCode())
			{
				Global->WindowManager->ExecuteModal(ShellEditor);
				// заставляем рефрешится экран
				Global->WindowManager->ResizeAllWindows();
			}
		}

		if (openMode == FILETYPE_ALTEDIT || openMode == FILETYPE_ALTVIEW)
		{
			const auto& externalCommand = openMode == FILETYPE_ALTEDIT? Global->Opt->strExternalEditor : Global->Opt->strExternalViewer;
			ProcessExternal(externalCommand, strSearchFileName, shortFileName, PluginMode);
		}
	}

	console.SetTitle(strOldTitle);
}

void FindFiles::AddMenuRecord(Dialog* const Dlg, string_view const FullName, const os::fs::find_data& FindData, void* const Data, FARPANELITEMFREECALLBACK const FreeData, ArcListItem* const Arc)
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

	auto DisplayName = FindData.FileName.c_str();

	string MenuText(1, L' ');

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

				const auto SizeToDisplay = (i.type == column_type::size)
					? FindData.FileSize
					: (i.type == column_type::size_compressed)
					? FindData.AllocationSize
					: (i.type == column_type::streams_size)
					? Size
					: Count; // ???

				append(MenuText, FormatStr_Size(
								SizeToDisplay,
								DisplayName,
								FindData.Attributes,
								0,
								FindData.ReparseTag,
								(i.type == column_type::streams_number || i.type == column_type::links_number)? column_type::streams_size : i.type,
								i.type_flags,
								Width), BoxSymbols[BS_V1]);
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

	const auto DisplayName0 = Arc? PointToName(DisplayName) : DisplayName;
	append(MenuText, DisplayName0);

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

		if (const auto ArcItem = Arc)
		{
			if(!(ArcItem->Flags & OPIF_REALNAMES) && !ArcItem->strArcName.empty())
			{
				auto strArcPathName = ArcItem->strArcName + L':';

				if (!IsSlash(strPathName.front()))
					AddEndSlash(strArcPathName);

				strArcPathName += strPathName == L".\\"sv? L"\\"s : strPathName;
				strPathName = strArcPathName;
			}
		}
		FindListItem& FindItem = itd->AddFindListItem({}, {}, {});
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

	FindListItem& FindItem = itd->AddFindListItem(FindData,Data,FreeData);
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
	m_Owner->m_Messages.emplace(FullName, fdata, FindData.UserData.Data, FindData.UserData.FreeData, m_Owner->itd->GetFindFileArcItem());
	FindData.UserData.FreeData = nullptr; //передано в FINDLIST
}

void background_searcher::ArchiveSearch(string_view const ArcName)
{
	_ALGO(CleverSysLog clv(L"FindFiles::ArchiveSearch()"));

	std::unique_ptr<plugin_panel> hArc;

	{
		const auto SavePluginsOutput = std::exchange(Global->DisablePluginsOutput, true);

		string strArcName(ArcName);
		{
			SCOPED_ACTION(auto)(m_Owner->ScopedLock());
			hArc = Global->CtrlObject->Plugins->OpenFilePlugin(&strArcName, OPM_FIND, OFP_SEARCH);
		}
		Global->DisablePluginsOutput = SavePluginsOutput;
	}

	if (!hArc)
		return;

	const auto SaveSearchMode = SearchMode;
	const auto SaveArcItem = m_Owner->itd->GetFindFileArcItem();
	{
		const auto SavePluginsOutput = std::exchange(Global->DisablePluginsOutput, true);

		// BUGBUG
		const_cast<FINDAREA&>(SearchMode) = FINDAREA_FROM_CURRENT;
		OpenPanelInfo Info;
		{
			SCOPED_ACTION(auto)(m_Owner->ScopedLock());
			Global->CtrlObject->Plugins->GetOpenPanelInfo(hArc.get(), &Info);
		}
		m_Owner->itd->SetFindFileArcItem(&m_Owner->itd->AddArcListItem(ArcName, hArc.get(), Info.Flags, NullToEmpty(Info.CurDir)));
		// Запомним каталог перед поиском в архиве. И если ничего не нашли - не рисуем его снова.
		{
			// Запомним пути поиска в плагине, они могут измениться.
			const auto strSaveSearchPath = strPluginSearchPath;
			m_Owner->m_Messages.emplace(FindFiles::push);
			DoPreparePluginListImpl();
			strPluginSearchPath = strSaveSearchPath;
			{
				SCOPED_ACTION(auto)(m_Owner->ScopedLock());
				Global->CtrlObject->Plugins->ClosePanel(std::move(hArc));

				const auto ArcItem = m_Owner->itd->GetFindFileArcItem();
				ArcItem->hPlugin = nullptr;
			}

			m_Owner->m_Messages.emplace(FindFiles::pop);
		}

		Global->DisablePluginsOutput=SavePluginsOutput;
	}
	m_Owner->itd->SetFindFileArcItem(SaveArcItem);
	// BUGBUG
	const_cast<FINDAREA&>(SearchMode) = SaveSearchMode;
}

void background_searcher::DoScanTree(string_view const strRoot)
{
	ScanTree ScTree(
		false,
		!(SearchMode==FINDAREA_CURRENT_ONLY||SearchMode==FINDAREA_INPATH),
		Global->Opt->FindOpt.FindSymLinks
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
		m_Owner->itd->SetFindMessage(strCurRoot);

		string strFullName;

		while (!Stopped() && ScTree.GetNextName(FindData, strFullName))
		{
			std::this_thread::yield();
			PauseEvent.wait();

			const auto ProcessStream = [&](string_view const FullStreamName)
			{
				filter_status FilterStatus;
				if (UseFilter && !m_Owner->GetFilter()->FileInFilter(FindData, &FilterStatus, FullStreamName))
				{
					// сюда заходим, если не попали в фильтр или попали в Exclude-фильтр
					if (FindData.Attributes & FILE_ATTRIBUTE_DIRECTORY && FilterStatus == filter_status::in_exclude)
						ScTree.SkipDir(); // скипаем только по Exclude-фильтру, т.к. глубже тоже нужно просмотреть
					return !Stopped();
				}

				if (FindData.Attributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					m_Owner->itd->SetFindMessage(FullStreamName);
				}

				if (IsFileIncluded(nullptr, FullStreamName, FindData.Attributes, strFullName))
				{
					m_Owner->m_Messages.emplace(FullStreamName, FindData, nullptr, nullptr, nullptr);
				}

				if (SearchInArchives)
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
					string_view StreamName(StreamData.cStreamName + 1);
					const auto NameEnd = StreamName.rfind(L':');
					if (NameEnd != StreamName.npos)
						StreamName = StreamName.substr(0, NameEnd);

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
	span<PluginPanelItem> PanelData;
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
				m_Owner->itd->SetFindMessage(strFullName);
			}

			if (IsFileIncluded(&CurPanelItem, CurName, CurPanelItem.FileAttributes, strFullName))
				AddMenuRecord(strFullName, CurPanelItem);

			if (SearchInArchives && (Flags & OPIF_REALNAMES))
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

			strPluginSearchPath += CurPanelItem.FileName;
			strPluginSearchPath += L'\\';
			ScanPluginTree(hPlugin, Flags, RecurseLevel);

			size_t pos = strPluginSearchPath.rfind(L'\\');
			if (pos != string::npos)
				strPluginSearchPath.resize(pos);

			if ((pos = strPluginSearchPath.rfind(L'\\')) != string::npos)
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
	const auto ArcItem = m_Owner->itd->GetFindFileArcItem();
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

struct THREADPARAM
{
	bool PluginMode;
};

void background_searcher::Search()
{
	seh_try_thread(m_ExceptionPtr, [this]
	{
		cpp_try(
		[&]
		{
			SCOPED_ACTION(wakeful);
			InitInFileSearch();
			m_PluginMode? DoPreparePluginList() : DoPrepareFileList();
			ReleaseInFileSearch();
		},
		[&]
		{
			SAVE_EXCEPTION_TO(m_ExceptionPtr);
			m_IsRegularException = true;
		});
	});

	m_Owner->itd->SetPercent(0);
	m_TaskbarProgress.reset();
	m_Finished = true;
}

bool FindFiles::FindFilesProcess()
{
	_ALGO(CleverSysLog clv(L"FindFiles::FindFilesProcess()"));
	// Если используется фильтр операций, то во время поиска сообщаем об этом
	string strTitle=msg(lng::MFindFileTitle);

	itd->Init();

	m_FileCount = 0;
	m_DirCount = 0;
	m_LastFoundNumber = 0;
	m_LastDirName.clear();

	if (!strFindMask.empty())
	{
		append(strTitle, L": "sv, strFindMask);

		if (UseFilter)
		{
			append(strTitle, L" ("sv, msg(lng::MFindUsingFilter), L')');
		}
	}
	else
	{
		if (UseFilter)
		{
			append(strTitle, L" ("sv, msg(lng::MFindUsingFilter), L')');
		}
	}

	int DlgWidth = ScrX + 1 - 2;
	int DlgHeight = ScrY + 1 - 2;

	auto FindDlg = MakeDialogItems<FD_COUNT>(
	{
		{ DI_DOUBLEBOX, {{3,                    1}, {DlgWidth-4, DlgHeight-2}}, DIF_SHOWAMPERSAND, strTitle, },
		{ DI_LISTBOX,   {{4,                    2}, {DlgWidth-5, DlgHeight-7}}, DIF_LISTNOBOX|DIF_DISABLE, },
		{ DI_TEXT,      {{-1,         DlgHeight-6}, {0,          DlgHeight-6}}, DIF_SEPARATOR2, },
		{ DI_TEXT,      {{5,          DlgHeight-5}, {DlgWidth-(strFindStr.empty()? 6 : 12), DlgHeight-5}}, DIF_SHOWAMPERSAND, L"…"sv },
		{ DI_TEXT,      {{DlgWidth-9, DlgHeight-5}, {DlgWidth-6, DlgHeight-5}}, (strFindStr.empty() ? DIF_HIDDEN : DIF_NONE), },
		{ DI_TEXT,      {{-1,         DlgHeight-4}, {0,          DlgHeight-4}}, DIF_SEPARATOR, },
		{ DI_BUTTON,    {{0,          DlgHeight-3}, {0,          DlgHeight-3}}, DIF_CENTERGROUP | DIF_FOCUS | DIF_DEFAULTBUTTON, msg(lng::MFindNewSearch), },
		{ DI_BUTTON,    {{0,          DlgHeight-3}, {0,          DlgHeight-3}}, DIF_CENTERGROUP | DIF_DISABLE, msg(lng::MFindGoTo), },
		{ DI_BUTTON,    {{0,          DlgHeight-3}, {0,          DlgHeight-3}}, DIF_CENTERGROUP | DIF_DISABLE, msg(lng::MFindView), },
		{ DI_BUTTON,    {{0,          DlgHeight-3}, {0,          DlgHeight-3}}, DIF_CENTERGROUP | DIF_DISABLE, msg(lng::MFindPanel), },
		{ DI_BUTTON,    {{0,          DlgHeight-3}, {0,          DlgHeight-3}}, DIF_CENTERGROUP, msg(lng::MFindStop), },
	});

	if (PluginMode)
	{
		const auto hPlugin = Global->CtrlObject->Cp()->ActivePanel()->GetPluginHandle();
		OpenPanelInfo Info;
		// no lock - background thread hasn't been started yet
		Global->CtrlObject->Plugins->GetOpenPanelInfo(hPlugin,&Info);
		itd->SetFindFileArcItem(&itd->AddArcListItem(NullToEmpty(Info.HostFile), hPlugin, Info.Flags, NullToEmpty(Info.CurDir)));

		if (!(Info.Flags & OPIF_REALNAMES))
		{
			FindDlg[FD_BUTTON_PANEL].Type=DI_TEXT;
			FindDlg[FD_BUTTON_PANEL].strData.clear();
		}
	}

	AnySetFindList = std::any_of(CONST_RANGE(*Global->CtrlObject->Plugins, i)
	{
		return i->has(iSetFindList);
	});

	if (!AnySetFindList)
	{
		FindDlg[FD_BUTTON_PANEL].Flags|=DIF_DISABLE;
	}

	const auto Dlg = Dialog::create(FindDlg, &FindFiles::FindDlgProc, this);
	Dlg->SetHelp(L"FindFileResult"sv);
	Dlg->SetPosition({ -1, -1, DlgWidth, DlgHeight });
	Dlg->SetId(FindFileResultId);
	Dlg->SetFlags(FSCROBJ_SPECIAL);

	m_ResultsDialogPtr = Dlg.get();

	clear_queue(m_Messages);

		{
			background_searcher BC(this, strFindStr, SearchMode, CodePage, ConvertFileSizeString(Global->Opt->FindOpt.strSearchInFirstSize), CmpCase, WholeWords, SearchInArchives, SearchHex, NotContaining, UseFilter, PluginMode);

			// BUGBUG
			m_Searcher = &BC;

			m_TimeCheck.reset();

			// Надо бы показать диалог, а то инициализация элементов запаздывает
			// иногда при поиске и первые элементы не добавляются
			Dlg->InitDialog();
			Dlg->Show();

			os::thread FindThread(os::thread::mode::join, &background_searcher::Search, &BC);

			// In case of an exception in the main thread
			SCOPE_EXIT
			{
				Dlg->CloseDialog();
				m_Searcher->Stop();
				m_Searcher = nullptr;
			};

			Dlg->Process();

			if (!m_ExceptionPtr)
			{
				m_ExceptionPtr = BC.ExceptionPtr();
			}

			if (m_ExceptionPtr && !BC.IsRegularException())
			{
				// You're someone else's problem
				FindThread.detach();
			}

			rethrow_if(m_ExceptionPtr);
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
				PanelItems.reserve(itd->GetFindListCount());

				itd->ForEachFindItem([&PanelItems, this](FindListItem& i)
				{
					if (!i.FindData.FileName.empty() && i.Used)
					{
					// Добавляем всегда, если имя задано
						// Для плагинов с виртуальными именами заменим имя файла на имя архива.
						// панель сама уберет лишние дубли.
						const auto IsArchive = i.Arc && !(i.Arc->Flags&OPIF_REALNAMES);
						// Добавляем только файлы или имена архивов или папки когда просили
						if (IsArchive || (Global->Opt->FindOpt.FindFolders && !SearchHex) ||
							    !(i.FindData.Attributes&FILE_ATTRIBUTE_DIRECTORY))
						{
							if (IsArchive)
							{
								i.FindData.FileName = i.Arc->strArcName;
							}
							PluginPanelItemHolderNonOwning pi;
							FindDataExToPluginPanelItemHolder(i.FindData, pi);

							if (IsArchive)
								pi.Item.FileAttributes = 0;

							if (pi.Item.FileAttributes & FILE_ATTRIBUTE_DIRECTORY)
							{
								DeleteEndSlash(const_cast<wchar_t*>(pi.Item.FileName));
							}
							PanelItems.emplace_back(pi.Item);
						}
					}
				});

				{
					SCOPED_ACTION(std::lock_guard)(PluginCS);
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
							SCOPED_ACTION(std::lock_guard)(PluginCS);
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
					size_t Length=strFileName.size();

					if (!Length)
						break;

					if (Length>1 && IsSlash(strFileName[Length-1]) && strFileName[Length-2] != L':')
						strFileName.pop_back();

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
					Length=strFileName.size();

					if (Length>1 && IsSlash(strFileName[Length-1]) && strFileName[Length-2] != L':')
						strFileName.pop_back();

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
					Length=strDirTmp.size();

					if (Length>1 && IsSlash(strDirTmp[Length-1]) && strDirTmp[Length-2] != L':')
						strDirTmp.pop_back();

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

void FindFiles::ProcessMessage(const AddMenuData& Data)
{
	switch(Data.m_Type)
	{
	case data:
		AddMenuRecord(m_ResultsDialogPtr, Data.m_FullName, Data.m_FindData, Data.m_Data, Data.m_FreeData, Data.m_Arc);
		m_EmptyArc = false;
		break;

	case push:
		m_LastDir.push(m_LastDirName);
		m_LastDirName.clear();
		m_EmptyArc = true;
		break;

	case pop:
		assert(!m_LastDir.empty());
		if (m_EmptyArc) m_LastDirName = m_LastDir.top();
		m_LastDir.pop();
		break;

	default:
		throw MAKE_FAR_FATAL_EXCEPTION(L"Unknown message type"sv);
	}
}


FindFiles::FindFiles():
	itd(std::make_unique<InterThreadData>()),
	FileMaskForFindFile(std::make_unique<filemasks>()),
	Filter(std::make_unique<FileFilter>(Global->CtrlObject->Cp()->ActivePanel().get(), FFT_FINDFILE)),
	m_TimeCheck(time_check::mode::immediate, GetRedrawTimeout()),
	m_MessageEvent(os::event::type::manual, os::event::state::signaled)
{
	_ALGO(CleverSysLog clv(L"FindFiles::FindFiles()"));

	static string strLastFindMask = L"*.*"s, strLastFindStr;

	static string strSearchFromRoot;
	strSearchFromRoot = msg(lng::MSearchFromRootFolder);

	static bool LastCmpCase = false, LastWholeWords = false, LastSearchInArchives = false, LastSearchHex = false, LastNotContaining = false;

	CmpCase=LastCmpCase;
	WholeWords=LastWholeWords;
	SearchInArchives=LastSearchInArchives;
	SearchHex=LastSearchHex;
	NotContaining = LastNotContaining;
	SearchMode = static_cast<FINDAREA>(Global->Opt->FindOpt.FileSearchMode.Get());
	UseFilter=Global->Opt->FindOpt.UseFilter.Get();
	strFindMask = strLastFindMask;
	strFindStr = strLastFindStr;

	do
	{
		FindExitItem = nullptr;
		FindFoldersChanged=false;
		SearchFromChanged=false;
		FindPositionChanged=false;
		Finalized=false;
		itd->ClearAllLists();
		const auto ActivePanel = Global->CtrlObject->Cp()->ActivePanel();
		PluginMode = ActivePanel->GetMode() == panel_mode::PLUGIN_PANEL && ActivePanel->IsVisible();
		PrepareDriveNameStr(strSearchFromRoot);
		const wchar_t VSeparator[] = { BoxSymbols[BS_T_H1V1], BoxSymbols[BS_V1], BoxSymbols[BS_V1], BoxSymbols[BS_V1], BoxSymbols[BS_V1], BoxSymbols[BS_B_H1V1], 0 };

		auto FindAskDlg = MakeDialogItems<FAD_COUNT>(
		{
			{ DI_DOUBLEBOX,   {{3,  1 }, {76, 19}}, DIF_NONE, msg(lng::MFindFileTitle), },
			{ DI_TEXT,        {{5,  2 }, {0,  2 }}, DIF_NONE, msg(lng::MFindFileMasks), },
			{ DI_EDIT,        {{5,  3 }, {74, 3 }}, DIF_FOCUS | DIF_HISTORY | DIF_USELASTHISTORY, },
			{ DI_TEXT,        {{-1, 4 }, {0,  4 }}, DIF_SEPARATOR, },
			{ DI_TEXT,        {{5,  5 }, {0,  5 }}, DIF_NONE, },
			{ DI_EDIT,        {{5,  6 }, {74, 6 }}, DIF_HISTORY, },
			{ DI_FIXEDIT,     {{5,  6 }, {74, 6 }}, DIF_MASKEDIT, },
			{ DI_TEXT,        {{5,  7 }, {0,  7 }}, DIF_NONE, },
			{ DI_COMBOBOX,    {{5,  8 }, {74, 8 }}, DIF_DROPDOWNLIST, },
			{ DI_TEXT,        {{-1, 9 }, {0,  9 }}, DIF_SEPARATOR, },
			{ DI_CHECKBOX,    {{5,  10}, {0,  10}}, DIF_NONE, msg(lng::MFindFileCase), },
			{ DI_CHECKBOX,    {{5,  11}, {0,  11}}, DIF_NONE, msg(lng::MFindFileWholeWords), },
			{ DI_CHECKBOX,    {{5,  12}, {0,  12}}, DIF_NONE, msg(lng::MSearchForHex), },
			{ DI_CHECKBOX,    {{5,  13}, {0,  13}}, DIF_NONE, msg(lng::MSearchNotContaining), },
			{ DI_CHECKBOX,    {{41, 10}, {0,  10}}, DIF_NONE, msg(lng::MFindArchives), },
			{ DI_CHECKBOX,    {{41, 11}, {0,  11}}, DIF_NONE, msg(lng::MFindFolders), },
			{ DI_CHECKBOX,    {{41, 12}, {0,  12}}, DIF_NONE, msg(lng::MFindSymLinks), },
			{ DI_CHECKBOX,    {{41, 13}, {0,  13}}, DIF_NONE, msg(lng::MFindAlternateStreams), },
			{ DI_TEXT,        {{-1, 14}, {0,  14}}, DIF_SEPARATOR, },
			{ DI_VTEXT,       {{39, 9 }, {0,  9 }}, DIF_BOXCOLOR, VSeparator },
			{ DI_TEXT,        {{5,  15}, {0,  15}}, DIF_NONE, msg(lng::MSearchWhere), },
			{ DI_COMBOBOX,    {{5,  16}, {36, 16}}, DIF_DROPDOWNLIST | DIF_LISTNOAMPERSAND, },
			{ DI_CHECKBOX,    {{41, 16}, {0,  16}}, DIF_AUTOMATION, msg(lng::MFindUseFilter), },
			{ DI_TEXT,        {{-1, 17}, {0,  17}}, DIF_SEPARATOR, },
			{ DI_BUTTON,      {{0,  18}, {0,  18}}, DIF_CENTERGROUP | DIF_DEFAULTBUTTON, msg(lng::MFindFileFind), },
			{ DI_BUTTON,      {{0,  18}, {0,  18}}, DIF_CENTERGROUP, msg(lng::MFindFileDrive), },
			{ DI_BUTTON,      {{0,  18}, {0,  18}}, DIF_CENTERGROUP | DIF_AUTOMATION | (UseFilter ? 0 : DIF_DISABLE), msg(lng::MFindFileSetFilter), },
			{ DI_BUTTON,      {{0,  18}, {0,  18}}, DIF_CENTERGROUP, msg(lng::MFindFileAdvanced), },
			{ DI_BUTTON,      {{0,  18}, {0,  18}}, DIF_CENTERGROUP, msg(lng::MCancel), },
		});

		FindAskDlg[FAD_EDIT_MASK].strHistory = L"Masks"sv;
		FindAskDlg[FAD_EDIT_TEXT].strHistory = L"SearchText"sv;
		FindAskDlg[FAD_EDIT_HEX].strMask = L"HH HH HH HH HH HH HH HH HH HH HH HH HH HH HH HH HH HH HH HH HH HH HH"sv;
		FindAskDlg[FAD_CHECKBOX_NOTCONTAINING].Selected = NotContaining;
		FindAskDlg[FAD_CHECKBOX_FILTER].Selected = UseFilter;

		if (strFindStr.empty())
			FindAskDlg[FAD_CHECKBOX_DIRS].Selected=Global->Opt->FindOpt.FindFolders;

		FarListItem li[]=
		{
			{ 0, msg(lng::MSearchAllDisks).c_str() },
			{ 0, msg(lng::MSearchAllButNetwork).c_str() },
			{ 0, msg(lng::MSearchInPATH).c_str() },
			{ 0, strSearchFromRoot.c_str() },
			{ 0, msg(lng::MSearchFromCurrent).c_str() },
			{ 0, msg(lng::MSearchInCurrent).c_str() },
			{ 0, msg(lng::MSearchInSelected).c_str() },
		};

		static_assert(std::size(li) == FINDAREA_COUNT);

		li[FINDAREA_ALL + SearchMode].Flags|=LIF_SELECTED;
		FarList l={sizeof(FarList),std::size(li),li};
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
			FindAskDlg[FAD_CHECKBOX_ARC].Selected=SearchInArchives;

		FindAskDlg[FAD_EDIT_MASK].strData = strFindMask;

		if (SearchHex)
			FindAskDlg[FAD_EDIT_HEX].strData = strFindStr;
		else
			FindAskDlg[FAD_EDIT_TEXT].strData = strFindStr;

		FindAskDlg[FAD_CHECKBOX_CASE].Selected=CmpCase;
		FindAskDlg[FAD_CHECKBOX_WHOLEWORDS].Selected=WholeWords;
		FindAskDlg[FAD_CHECKBOX_HEX].Selected=SearchHex;
		const auto Dlg = Dialog::create(FindAskDlg, &FindFiles::MainDlgProc, this);
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
		CmpCase=FindAskDlg[FAD_CHECKBOX_CASE].Selected == BSTATE_CHECKED;
		WholeWords=FindAskDlg[FAD_CHECKBOX_WHOLEWORDS].Selected == BSTATE_CHECKED;
		SearchHex=FindAskDlg[FAD_CHECKBOX_HEX].Selected == BSTATE_CHECKED;
		SearchInArchives=FindAskDlg[FAD_CHECKBOX_ARC].Selected == BSTATE_CHECKED;
		NotContaining = FindAskDlg[FAD_CHECKBOX_NOTCONTAINING].Selected == BSTATE_CHECKED;

		if (FindFoldersChanged)
		{
			Global->Opt->FindOpt.FindFolders=(FindAskDlg[FAD_CHECKBOX_DIRS].Selected==BSTATE_CHECKED);
		}

		if (!PluginMode)
		{
			Global->Opt->FindOpt.FindSymLinks=(FindAskDlg[FAD_CHECKBOX_LINKS].Selected==BSTATE_CHECKED);
			Global->Opt->FindOpt.FindAlternateStreams = (FindAskDlg[FAD_CHECKBOX_STREAMS].Selected == BSTATE_CHECKED);
		}

		UseFilter=(FindAskDlg[FAD_CHECKBOX_FILTER].Selected==BSTATE_CHECKED);
		Global->Opt->FindOpt.UseFilter=UseFilter;
		strFindMask = !FindAskDlg[FAD_EDIT_MASK].strData.empty()? FindAskDlg[FAD_EDIT_MASK].strData : L"*"sv;

		if (SearchHex)
		{
			strFindStr = ExtractHexString(FindAskDlg[FAD_EDIT_HEX].strData);
		}
		else
			strFindStr = FindAskDlg[FAD_EDIT_TEXT].strData;

		if (!strFindStr.empty())
		{
			Global->StoreSearchString(strFindStr, SearchHex);
			Global->GlobalSearchCase=CmpCase;
			Global->GlobalSearchWholeWords=WholeWords;
		}

		SearchMode = static_cast<FINDAREA>(FindAskDlg[FAD_COMBOBOX_WHERE].ListPos);

		if (SearchFromChanged)
		{
			Global->Opt->FindOpt.FileSearchMode=SearchMode;
		}

		LastCmpCase=CmpCase;
		LastWholeWords=WholeWords;
		LastSearchHex=SearchHex;
		LastSearchInArchives=SearchInArchives;
		LastNotContaining = NotContaining;
		strLastFindMask = strFindMask;
		strLastFindStr = strFindStr;

		if (!strFindStr.empty())
			Editor::SetReplaceMode(false);
	}
	while (FindFilesProcess());

	Global->CtrlObject->Cp()->ActivePanel()->RefreshTitle();
}

FindFiles::~FindFiles() = default;


background_searcher::background_searcher(
	FindFiles* Owner,
	string FindString,
	FINDAREA SearchMode,
	uintptr_t CodePage,
	unsigned long long SearchInFirst,
	bool CmpCase,
	bool WholeWords,
	bool SearchInArchives,
	bool SearchHex,
	bool NotContaining,
	bool UseFilter,
	bool PluginMode):

	m_Owner(Owner),
	m_Autodetection(),
	strFindStr(std::move(FindString)),
	SearchMode(SearchMode),
	CodePage(CodePage),
	SearchInFirst(SearchInFirst),
	CmpCase(CmpCase),
	WholeWords(WholeWords),
	SearchInArchives(SearchInArchives),
	SearchHex(SearchHex),
	NotContaining(NotContaining),
	UseFilter(UseFilter),
	m_PluginMode(PluginMode),
	PauseEvent(os::event::type::manual, os::event::state::signaled),
	StopEvent(os::event::type::manual, os::event::state::nonsignaled)
{
}
