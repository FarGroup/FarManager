/*
stddlg.cpp

Куча разных стандартных диалогов
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
#include "stddlg.hpp"

// Internal:
#include "dialog.hpp"
#include "strmix.hpp"
#include "imports.hpp"
#include "message.hpp"
#include "lang.hpp"
#include "uuids.far.dialogs.hpp"
#include "interf.hpp"
#include "dlgedit.hpp"
#include "cvtname.hpp"
#include "exception.hpp"
#include "RegExp.hpp"
#include "FarDlgBuilder.hpp"
#include "config.hpp"
#include "plist.hpp"
#include "notification.hpp"
#include "global.hpp"
#include "language.hpp"

// Platform:
#include "platform.fs.hpp"

// Common:
#include "common/from_string.hpp"
#include "common/function_ref.hpp"
#include "common/function_traits.hpp"
#include "common/scope_exit.hpp"

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

int GetSearchReplaceString(
	bool IsReplaceMode,
	string_view Title,
	string_view SubTitle,
	string& SearchStr,
	string& ReplaceStr,
	string_view TextHistoryName,
	string_view ReplaceHistoryName,
	bool* pCase,
	bool* pWholeWords,
	bool* pReverse,
	bool* pRegexp,
	bool* pPreserveStyle,
	string_view const HelpTopic,
	bool HideAll,
	const UUID* Id,
	function_ref<string(bool)> const Picker)
{
	int Result = 0;

	if (TextHistoryName.empty())
		TextHistoryName = L"SearchText"sv;

	if (ReplaceHistoryName.empty())
		ReplaceHistoryName = L"ReplaceText"sv;

	if (Title.empty())
		Title = msg(IsReplaceMode? lng::MEditReplaceTitle : lng::MEditSearchTitle);

	if (SubTitle.empty())
		SubTitle = msg(lng::MEditSearchFor);


	bool Case=pCase?*pCase:false;
	bool WholeWords=pWholeWords?*pWholeWords:false;
	bool Reverse=pReverse?*pReverse:false;
	bool Regexp=pRegexp?*pRegexp:false;
	bool PreserveStyle=pPreserveStyle?*pPreserveStyle:false;

	const auto DlgWidth = 76;
	const auto& WordLabel = msg(lng::MEditSearchPickWord);
	const auto& SelectionLabel = msg(lng::MEditSearchPickSelection);
	const auto WordButtonSize = HiStrlen(WordLabel) + 4;
	const auto SelectionButtonSize = HiStrlen(SelectionLabel) + 4;
	const auto SelectionButtonX2 = static_cast<int>(DlgWidth - 4 - 1);
	const auto SelectionButtonX1 = static_cast<int>(SelectionButtonX2 - SelectionButtonSize);
	const auto WordButtonX2 = static_cast<int>(SelectionButtonX1 - 1);
	const auto WordButtonX1 = static_cast<int>(WordButtonX2 - WordButtonSize);

	const auto YFix = IsReplaceMode? 0 : 2;

	enum item_id
	{
		dlg_border,
		dlg_button_word,
		dlg_button_selection,
		dlg_label_search,
		dlg_edit_search,
		dlg_label_replace,
		dlg_edit_replace,
		dlg_separator_1,
		dlg_checkbox_case,
		dlg_checkbox_words,
		dlg_checkbox_reverse,
		dlg_checkbox_regex,
		dlg_checkbox_style,
		dlg_separator_2,
		dlg_button_action,
		dlg_button_all,
		dlg_button_cancel,

		dlg_count
	};

	auto ReplaceDlg = MakeDialogItems<dlg_count>(
	{
		{ DI_DOUBLEBOX, {{3,                 1      }, {DlgWidth-4,        12-YFix}}, DIF_NONE, Title },
		{ DI_BUTTON,    {{WordButtonX1,      2      }, {WordButtonX2,      2      }}, DIF_BTNNOCLOSE, WordLabel },
		{ DI_BUTTON,    {{SelectionButtonX1, 2      }, {SelectionButtonX2, 2      }}, DIF_BTNNOCLOSE, SelectionLabel },
		{ DI_TEXT,      {{5,                 2      }, {0,                 2      }}, DIF_NONE, SubTitle },
		{ DI_EDIT,      {{5,                 3      }, {70,                3      }}, DIF_FOCUS | DIF_USELASTHISTORY | DIF_HISTORY, SearchStr, },
		{ DI_TEXT,      {{5,                 4      }, {0,                 4      }}, DIF_NONE, msg(lng::MEditReplaceWith), },
		{ DI_EDIT,      {{5,                 5      }, {70,                5      }}, DIF_USELASTHISTORY | DIF_HISTORY, ReplaceStr, },
		{ DI_TEXT,      {{-1,                6-YFix }, {0,                 6-YFix }}, DIF_SEPARATOR, },
		{ DI_CHECKBOX,  {{5,                 7-YFix }, {0,                 7-YFix }}, DIF_NONE, msg(lng::MEditSearchCase), },
		{ DI_CHECKBOX,  {{5,                 8-YFix }, {0,                 8-YFix }}, DIF_NONE, msg(lng::MEditSearchWholeWords), },
		{ DI_CHECKBOX,  {{5,                 9-YFix }, {0,                 9-YFix }}, DIF_NONE, msg(lng::MEditSearchReverse), },
		{ DI_CHECKBOX,  {{40,                7-YFix }, {0,                 7-YFix }}, DIF_NONE, msg(lng::MEditSearchRegexp), },
		{ DI_CHECKBOX,  {{40,                8-YFix }, {0,                 8-YFix }}, DIF_NONE, msg(lng::MEditSearchPreserveStyle), },
		{ DI_TEXT,      {{-1,                10-YFix}, {0,                 10-YFix}}, DIF_SEPARATOR, },
		{ DI_BUTTON,    {{0,                 11-YFix}, {0,                 11-YFix}}, DIF_CENTERGROUP | DIF_DEFAULTBUTTON, msg(IsReplaceMode? lng::MEditReplaceReplace : lng::MEditSearchSearch), },
		{ DI_BUTTON,    {{0,                 11-YFix}, {0,                 11-YFix}}, DIF_CENTERGROUP, msg(lng::MEditSearchAll), },
		{ DI_BUTTON,    {{0,                 11-YFix}, {0,                 11-YFix}}, DIF_CENTERGROUP, msg(lng::MEditSearchCancel), },
	});

	ReplaceDlg[dlg_edit_search].strHistory = TextHistoryName;
	ReplaceDlg[dlg_edit_replace].strHistory = ReplaceHistoryName;
	ReplaceDlg[dlg_checkbox_case].Selected = Case;
	ReplaceDlg[dlg_checkbox_words].Selected = WholeWords;
	ReplaceDlg[dlg_checkbox_reverse].Selected = Reverse;
	ReplaceDlg[dlg_checkbox_regex].Selected = Regexp;
	ReplaceDlg[dlg_checkbox_style].Selected = PreserveStyle;


	if (IsReplaceMode || HideAll)
	{
		ReplaceDlg[dlg_button_all].Flags |= DIF_HIDDEN;
	}

	if (!IsReplaceMode)
	{
		ReplaceDlg[dlg_label_replace].Flags |= DIF_HIDDEN;
		ReplaceDlg[dlg_edit_replace].Flags |= DIF_HIDDEN;
		ReplaceDlg[dlg_checkbox_style].Flags |= DIF_HIDDEN;
	}

	if (!Picker)
	{
		ReplaceDlg[dlg_button_word].Flags |= DIF_HIDDEN;
		ReplaceDlg[dlg_button_selection].Flags |= DIF_HIDDEN;
	}

	if (!pCase)
		ReplaceDlg[dlg_checkbox_case].Flags |= DIF_DISABLE; // DIF_HIDDEN ??
	if (!pWholeWords)
		ReplaceDlg[dlg_checkbox_words].Flags |= DIF_DISABLE; // DIF_HIDDEN ??
	if (!pReverse)
		ReplaceDlg[dlg_checkbox_reverse].Flags |= DIF_DISABLE; // DIF_HIDDEN ??
	if (!pRegexp)
		ReplaceDlg[dlg_checkbox_regex].Flags |= DIF_DISABLE; // DIF_HIDDEN ??
	if (!pPreserveStyle)
		ReplaceDlg[dlg_checkbox_style].Flags |= DIF_DISABLE; // DIF_HIDDEN ??

	const auto Handler = [&](Dialog* Dlg, intptr_t Msg, intptr_t Param1, void* Param2) -> intptr_t
	{
		if (Msg == DN_BTNCLICK && Picker && (Param1 == dlg_button_word || Param1 == dlg_button_selection))
		{
			// BUGBUG: #0003136: DM_INSERTTEXT or something like that
			static_cast<DlgEdit*>(Dlg->GetAllItem()[dlg_edit_search].ObjPtr)->InsertString(Picker(Param1 == dlg_button_selection));
			Dlg->SendMessage(DM_SETFOCUS, dlg_edit_search, nullptr);
			return TRUE;
		}
		return Dlg->DefProc(Msg, Param1, Param2);
	};

	const auto Dlg = Dialog::create(ReplaceDlg, Handler);
	Dlg->SetPosition({ -1, -1, DlgWidth, 14 - YFix });

	if (!HelpTopic.empty())
		Dlg->SetHelp(HelpTopic);

	if(Id)
		Dlg->SetId(*Id);

	Dlg->Process();

	const auto ExitCode = Dlg->GetExitCode();
	if(ExitCode == dlg_button_action || ExitCode == dlg_button_all)
	{
		Result = ExitCode == dlg_button_action ? 1 : 2;
		SearchStr = ReplaceDlg[dlg_edit_search].strData;
		ReplaceStr = ReplaceDlg[dlg_edit_replace].strData;
		Case=ReplaceDlg[dlg_checkbox_case].Selected == BSTATE_CHECKED;
		WholeWords=ReplaceDlg[dlg_checkbox_words].Selected == BSTATE_CHECKED;
		Reverse=ReplaceDlg[dlg_checkbox_reverse].Selected == BSTATE_CHECKED;
		Regexp=ReplaceDlg[dlg_checkbox_regex].Selected == BSTATE_CHECKED;
		PreserveStyle=ReplaceDlg[dlg_checkbox_style].Selected == BSTATE_CHECKED;
	}

	if (pCase)
		*pCase=Case;
	if (pWholeWords)
		*pWholeWords=WholeWords;
	if (pReverse)
		*pReverse=Reverse;
	if (pRegexp)
		*pRegexp=Regexp;
	if (pPreserveStyle)
		*pPreserveStyle=PreserveStyle;

	return Result;
}

bool GetString(
	const string_view Title,
	const string_view Prompt,
	const string_view HistoryName,
	const string_view SrcText,
	string& strDestText,
	const string_view HelpTopic,
	const DWORD Flags,
	int* const CheckBoxValue,
	const string_view CheckBoxText,
	Plugin* const PluginNumber,
	const UUID* const Id
)
{
	int Substract=5; // дополнительная величина :-)
	int ExitCode;
	const auto addCheckBox = Flags&FIB_CHECKBOX && CheckBoxValue && !CheckBoxText.empty();
	const auto offset = addCheckBox? 2 : 0;

	enum
	{
		gs_doublebox,
		gs_text,
		gs_edit,
		gs_separator_1,
		gs_checkbox,
		gs_separator_2,
		gs_button_1,
		gs_button_2,

		gs_count
	};

	auto StrDlg = MakeDialogItems<gs_count>(
	{
		{ DI_DOUBLEBOX, {{3,  1}, {72, 4}}, DIF_NONE,                      },
		{ DI_TEXT,      {{5,  2}, {0,  2}}, DIF_SHOWAMPERSAND,             },
		{ DI_EDIT,      {{5,  3}, {70, 3}}, DIF_FOCUS | DIF_DEFAULTBUTTON, },
		{ DI_TEXT,      {{-1, 4}, {0,  4}}, DIF_SEPARATOR,                 },
		{ DI_CHECKBOX,  {{5,  5}, {0,  5}}, DIF_NONE,                      },
		{ DI_TEXT,      {{-1, 6}, {0,  6}}, DIF_SEPARATOR,                 },
		{ DI_BUTTON,    {{0,  7}, {0,  7}}, DIF_CENTERGROUP,               },
		{ DI_BUTTON,    {{0,  7}, {0,  7}}, DIF_CENTERGROUP,               },
	});

	if (addCheckBox)
	{
		Substract-=2;
		StrDlg[gs_doublebox].Y2 += 2;
		StrDlg[gs_checkbox].Selected = *CheckBoxValue != 0;
		StrDlg[gs_checkbox].strData = CheckBoxText;
	}

	if (Flags&FIB_BUTTONS)
	{
		Substract-=3;
		StrDlg[gs_doublebox].Y2 += 2;
		StrDlg[gs_edit].Flags &= ~DIF_DEFAULTBUTTON;
		StrDlg[gs_separator_2 + offset].Y1 = StrDlg[gs_checkbox + offset].Y1 = 5 + offset;
		StrDlg[gs_checkbox + offset].Type = StrDlg[gs_separator_2 + offset].Type = DI_BUTTON;
		StrDlg[gs_checkbox + offset].Flags = StrDlg[gs_separator_2 + offset].Flags = DIF_CENTERGROUP;
		StrDlg[gs_checkbox + offset].Flags |= DIF_DEFAULTBUTTON;
		StrDlg[gs_checkbox + offset].strData = msg(lng::MOk);
		StrDlg[gs_separator_2 + offset].strData = msg(lng::MCancel);
	}

	if (Flags&FIB_EXPANDENV)
	{
		StrDlg[gs_edit].Flags |= DIF_EDITEXPAND;
	}

	if (Flags&FIB_EDITPATH)
	{
		StrDlg[gs_edit].Flags |= DIF_EDITPATH;
	}

	if (Flags&FIB_EDITPATHEXEC)
	{
		StrDlg[gs_edit].Flags |= DIF_EDITPATHEXEC;
	}

	if (!HistoryName.empty())
	{
		StrDlg[gs_edit].strHistory = HistoryName;
		StrDlg[gs_edit].Flags |= DIF_HISTORY | (Flags & FIB_NOUSELASTHISTORY ? 0 : DIF_USELASTHISTORY);
	}

	if (Flags&FIB_PASSWORD)
		StrDlg[gs_edit].Type = DI_PSWEDIT;

	if (!Title.empty())
		StrDlg[gs_doublebox].strData = Title;

	if (!Prompt.empty())
	{
		StrDlg[gs_text].strData = truncate_right(Prompt, 66);

		if (Flags&FIB_NOAMPERSAND)
			StrDlg[gs_text].Flags &= ~DIF_SHOWAMPERSAND;
	}

	if (!SrcText.empty())
		StrDlg[gs_edit].strData = SrcText;

	{
		const auto Dlg = Dialog::create(span(StrDlg.data(), StrDlg.size() - Substract));
		Dlg->SetPosition({ -1, -1, 76, offset + (Flags & FIB_BUTTONS? 8 : 6) });
		if(Id) Dlg->SetId(*Id);

		if (!HelpTopic.empty())
			Dlg->SetHelp(HelpTopic);

		Dlg->SetPluginOwner(PluginNumber);

		Dlg->Process();

		ExitCode=Dlg->GetExitCode();
	}

	if (ExitCode == gs_edit || ExitCode == gs_checkbox || (addCheckBox && ExitCode == gs_button_1))
	{
		if (!(Flags&FIB_ENABLEEMPTY) && StrDlg[gs_edit].strData.empty())
			return false;

		strDestText = StrDlg[gs_edit].strData;

		if (addCheckBox)
			*CheckBoxValue = StrDlg[gs_checkbox].Selected;

		return true;
	}

	return false;
}

/*
  Стандартный диалог ввода пароля.
  Умеет сам запоминать последнего юзвера и пароль.
*/
bool GetNameAndPassword(
	string_view const Title,
	string& strUserName,
	string& strPassword,
	string_view const HelpTopic,
	DWORD const Flags)
{
	static string strLastName, strLastPassword;
	int ExitCode;
	/*
	          1         2         3         4         5         6         7
	   3456789012345678901234567890123456789012345678901234567890123456789012
	 1 ╔══════════════════════════════ Title ═══════════════════════════════╗
	 2 ║ User name                                                          ║
	 3 ║ __________________________________________________________________↓║
	 4 ║ User password                                                      ║
	 5 ║ __________________________________________________________________ ║
	 6 ╟────────────────────────────────────────────────────────────────────╢
	 7 ║                         { OK } [ Cancel ]                          ║
	 8 ╚════════════════════════════════════════════════════════════════════╝
	*/

	enum
	{
		pd_doublebox,
		pd_text_user,
		pd_edit_user,
		pd_text_password,
		pd_edit_password,
		pd_separator,
		pd_button_ok,
		pd_button_cancel,

		pd_count
	};

	auto PassDlg = MakeDialogItems<pd_count>(
	{
		{ DI_DOUBLEBOX, {{3,  1}, {72, 8}}, DIF_NONE, Title, },
		{ DI_TEXT,      {{5,  2}, {0,  2}}, DIF_NONE, msg(lng::MNetUserName), },
		{ DI_EDIT,      {{5,  3}, {70, 3}}, DIF_FOCUS | DIF_USELASTHISTORY | DIF_HISTORY, (Flags & GNP_USELAST)? strLastName : strUserName, },
		{ DI_TEXT,      {{5,  4}, {0,  4}}, DIF_NONE, msg(lng::MNetUserPassword), },
		{ DI_PSWEDIT,   {{5,  5}, {70, 5}}, DIF_NONE, (Flags & GNP_USELAST)? strLastPassword : strPassword, },
		{ DI_TEXT,      {{-1, 6}, {0,  6}}, DIF_SEPARATOR, },
		{ DI_BUTTON,    {{0,  7}, {0,  7}}, DIF_CENTERGROUP | DIF_DEFAULTBUTTON, msg(lng::MOk), },
		{ DI_BUTTON,    {{0,  7}, {0,  7}}, DIF_CENTERGROUP, msg(lng::MCancel), },
	});

	PassDlg[pd_edit_user].strHistory = L"NetworkUser"sv;

	{
		const auto Dlg = Dialog::create(PassDlg);
		Dlg->SetPosition({ -1, -1, 76, 10 });
		Dlg->SetId(GetNameAndPasswordId);

		if (!HelpTopic.empty())
			Dlg->SetHelp(HelpTopic);

		Dlg->Process();
		ExitCode=Dlg->GetExitCode();
	}

	if (ExitCode != pd_button_ok)
		return false;

	// запоминаем всегда.
	strUserName = PassDlg[pd_edit_user].strData;
	strLastName = strUserName;
	strPassword = PassDlg[pd_edit_password].strData;
	strLastPassword = strPassword;
	return true;
}

static os::com::ptr<IFileIsInUse> CreateIFileIsInUse(const string& File)
{
	os::com::ptr<IRunningObjectTable> RunningObjectTable;
	if (FAILED(GetRunningObjectTable(0, &ptr_setter(RunningObjectTable))))
		return nullptr;

	os::com::ptr<IMoniker> FileMoniker;
	if (FAILED(CreateFileMoniker(File.c_str(), &ptr_setter(FileMoniker))))
		return nullptr;

	os::com::ptr<IEnumMoniker> EnumMoniker;
	if (FAILED(RunningObjectTable->EnumRunning(&ptr_setter(EnumMoniker))))
		return nullptr;

	for(;;)
	{
		os::com::ptr<IMoniker> Moniker;
		if (EnumMoniker->Next(1, &ptr_setter(Moniker), nullptr) == S_FALSE)
			return nullptr;

		DWORD Type;
		if (FAILED(Moniker->IsSystemMoniker(&Type)) || Type != MKSYS_FILEMONIKER)
			continue;

		os::com::ptr<IMoniker> PrefixMoniker;
		if (FAILED(FileMoniker->CommonPrefixWith(Moniker.get(), &ptr_setter(PrefixMoniker))))
			continue;

		if (FileMoniker->IsEqual(PrefixMoniker.get()) == S_FALSE)
			continue;

		os::com::ptr<IUnknown> Unknown;
		if (RunningObjectTable->GetObject(Moniker.get(), &ptr_setter(Unknown)) == S_FALSE)
			continue;

		FN_RETURN_TYPE(CreateIFileIsInUse) FileIsInUse;
		if (SUCCEEDED(Unknown->QueryInterface(IID_IFileIsInUse, IID_PPV_ARGS_Helper(&ptr_setter(FileIsInUse)))))
			return FileIsInUse;
	}
}

static size_t enumerate_rm_processes(const string& Filename, DWORD& Reasons, function_ref<bool(string&&)> const Handler)
{
	DWORD Session;
	wchar_t SessionKey[CCH_RM_SESSION_KEY + 1] = {};
	if (imports.RmStartSession(&Session, 0, SessionKey) != ERROR_SUCCESS)
		return 0;

	SCOPE_EXIT{ imports.RmEndSession(Session); };
	auto FilenamePtr = Filename.c_str();
	if (imports.RmRegisterResources(Session, 1, &FilenamePtr, 0, nullptr, 0, nullptr) != ERROR_SUCCESS)
		return 0;

	DWORD RmGetListResult;
	unsigned ProceccInfoSizeNeeded = 0, ProcessInfoSize = 1;
	std::vector<RM_PROCESS_INFO> ProcessInfos(ProcessInfoSize);
	while ((RmGetListResult = imports.RmGetList(Session, &ProceccInfoSizeNeeded, &ProcessInfoSize, ProcessInfos.data(), &Reasons)) == ERROR_MORE_DATA)
	{
		ProcessInfoSize = ProceccInfoSizeNeeded;
		ProcessInfos.resize(ProcessInfoSize);
	}

	if (RmGetListResult != ERROR_SUCCESS)
		return 0;

	for (const auto& Info : ProcessInfos)
	{
		auto Str = *Info.strAppName? Info.strAppName : L"Unknown"s;

		if (*Info.strServiceShortName)
			append(Str, L" ["sv, Info.strServiceShortName, L']');

		append(Str, L" (PID: "sv, str(Info.Process.dwProcessId));

		if (const auto Process = os::handle(OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, Info.Process.dwProcessId)))
		{
			os::chrono::time_point CreationTime;
			if (os::chrono::get_process_creation_time(Process.native_handle(), CreationTime) &&
				os::chrono::nt_clock::from_filetime(Info.Process.ProcessStartTime) == CreationTime)
			{
				string Name;
				if (os::fs::GetModuleFileName(Process.native_handle(), nullptr, Name))
				{
					append(Str, L", "sv, Name);
				}
			}
		}

		Str += L')';

		if (!Handler(std::move(Str)))
			break;
	}

	return ProcessInfos.size();
}

operation OperationFailed(const error_state_ex& ErrorState, string_view const Object, lng Title, string Description, bool AllowSkip, bool AllowSkipAll)
{
	std::vector<string> Msg;
	os::com::ptr<IFileIsInUse> FileIsInUse;
	auto Reason = lng::MObjectLockedReasonOpened;
	bool SwitchBtn = false, CloseBtn = false;
	const auto Error = ErrorState.Win32Error;
	if(Error == ERROR_ACCESS_DENIED ||
		Error == ERROR_SHARING_VIOLATION ||
		Error == ERROR_LOCK_VIOLATION ||
		Error == ERROR_DRIVE_LOCKED)
	{
		const auto FullName = ConvertNameToFull(Object);
		FileIsInUse = CreateIFileIsInUse(FullName);
		if (FileIsInUse)
		{
			FILE_USAGE_TYPE UsageType;
			if (FAILED(FileIsInUse->GetUsage(&UsageType)))
				UsageType = FUT_GENERIC;

			switch (UsageType)
			{
			case FUT_PLAYING:
				Reason = lng::MObjectLockedReasonPlayed;
				break;
			case FUT_EDITING:
				Reason = lng::MObjectLockedReasonEdited;
				break;
			case FUT_GENERIC:
				Reason = lng::MObjectLockedReasonOpened;
				break;
			}

			DWORD Capabilities;
			if (SUCCEEDED(FileIsInUse->GetCapabilities(&Capabilities)))
			{
				SwitchBtn = (Capabilities & OF_CAP_CANSWITCHTO) != 0;
				CloseBtn = (Capabilities & OF_CAP_CANCLOSE) != 0;
			}

			wchar_t* AppName;
			if(SUCCEEDED(FileIsInUse->GetAppName(&AppName)))
			{
				Msg.emplace_back(AppName);
			}
		}
		else
		{
			const size_t MaxRmProcesses = 5;
			DWORD Reasons = RmRebootReasonNone;
			const auto ProcessCount = enumerate_rm_processes(FullName, Reasons, [&](string&& Str)
			{
				Msg.emplace_back(std::move(Str));
				return Msg.size() != MaxRmProcesses;
			});

			if (ProcessCount > MaxRmProcesses)
			{
				Msg.emplace_back(format(msg(lng::MObjectLockedAndMore), ProcessCount - MaxRmProcesses));
			}

			static const std::pair<DWORD, lng> Mappings[] =
			{
				// We don't handle RmRebootReasonPermissionDenied here as we don't try to close anything.
				{RmRebootReasonSessionMismatch, lng::MObjectLockedReasonSessionMismatch },
				{RmRebootReasonCriticalProcess, lng::MObjectLockedReasonCriticalProcess },
				{RmRebootReasonCriticalService, lng::MObjectLockedReasonCriticalService },
				{RmRebootReasonDetectedSelf,    lng::MObjectLockedReasonDetectedSelf },
			};

			bool SeparatorAdded = false;

			for (const auto& [Flag, Lng]: Mappings)
			{
				if (!(Reasons & Flag))
					continue;

				if (!SeparatorAdded)
				{
					Msg.emplace_back(L"\1"sv);
					SeparatorAdded = true;
				}

				Msg.emplace_back(msg(Lng));
			}
		}
	}

	std::vector Msgs{std::move(Description), QuoteOuterSpace(string(Object))};
	if(!Msg.empty())
	{
		Msgs.emplace_back(format(msg(lng::MObjectLockedReason), msg(Reason)));
		std::move(ALL_RANGE(Msg), std::back_inserter(Msgs));
		Msg.clear();
	}

	std::vector<lng> Buttons;
	Buttons.reserve(4);
	if(SwitchBtn)
	{
		Buttons.emplace_back(lng::MObjectLockedSwitchTo);
	}
	Buttons.emplace_back(CloseBtn? lng::MObjectLockedClose : lng::MDeleteRetry);
	if(AllowSkip)
	{
		Buttons.emplace_back(lng::MDeleteSkip);
		if (AllowSkipAll)
		{
			Buttons.emplace_back(lng::MDeleteFileSkipAll);
		}
	}
	Buttons.emplace_back(lng::MCancel);

	std::optional<listener> Listener;
	if (SwitchBtn)
	{
		Listener.emplace([](const std::any& Payload)
		{
			// Switch asynchronously after the message is reopened,
			// otherwise Far will lose the focus too early
			// and reopened message will cause window flashing.
			SwitchToWindow(std::any_cast<HWND>(Payload));
		});
	}

	int Result;
	for(;;)
	{
		Result = Message(MSG_WARNING, ErrorState,
			msg(Title),
			Msgs,
			Buttons);

		if(SwitchBtn)
		{
			if(Result == Message::first_button)
			{
				HWND Window = nullptr;
				if (FileIsInUse && SUCCEEDED(FileIsInUse->GetSwitchToHWND(&Window)))
				{
					message_manager::instance().notify(Listener->GetEventName(), Window);
				}
				continue;
			}
			else if(Result > 0)
			{
				--Result;
			}
		}

		if(CloseBtn && Result == Message::first_button)
		{
			// close & retry
			if (FileIsInUse)
			{
				FileIsInUse->CloseFile();
			}
		}
		break;
	}

	if (Result < 0 || static_cast<size_t>(Result) == Buttons.size() - 1)
		return operation::cancel;

	return static_cast<operation>(Result);
}

static string GetReErrorString(int code)
{
	// TODO: localization
	switch (code)
	{
	case errNone:
		return L"No errors"s;
	case errNotCompiled:
		return L"RegExp wasn't even tried to compile"s;
	case errSyntax:
		return L"Expression contains a syntax error"s;
	case errBrackets:
		return L"Unbalanced brackets"s;
	case errMaxDepth:
		return L"Max recursive brackets level reached"s;
	case errOptions:
		return L"Invalid options combination"s;
	case errInvalidBackRef:
		return L"Reference to nonexistent bracket"s;
	case errInvalidEscape:
		return L"Invalid escape char"s;
	case errInvalidRange:
		return L"Invalid range value"s;
	case errInvalidQuantifiersCombination:
		return L"Quantifier applied to invalid object. f.e. lookahead assertion"s;
	case errNotEnoughMatches:
		return L"Size of match array isn't large enough"s;
	case errNoStorageForNB:
		return L"Attempt to match RegExp with Named Brackets but no storage class provided"s;
	case errReferenceToUndefinedNamedBracket:
		return L"Reference to undefined named bracket"s;
	case errVariableLengthLookBehind:
		return L"Only fixed length look behind assertions are supported"s;
	default:
		return L"Unknown error"s;
	}
}

void ReCompileErrorMessage(const RegExp& re, string_view const str)
{
	Message(MSG_WARNING | MSG_LEFTALIGN,
		msg(lng::MError),
		{
			GetReErrorString(re.LastError()),
			string(str),
			string(re.ErrorPosition(), L' ') + L'↑'
		},
		{ lng::MOk });
}

void ReMatchErrorMessage(const RegExp& re)
{
	if (re.LastError() != errNone)
	{
		Message(MSG_WARNING | MSG_LEFTALIGN,
			msg(lng::MError),
			{
				GetReErrorString(re.LastError())
			},
			{ lng::MOk });
	}
}

static void GetRowCol(const string& Str, bool Hex, goto_coord& Row, goto_coord& Col)
{
	const auto Parse = [Hex](string Part, goto_coord& Dest)
	{
		inplace::erase_all(Part, L' ');

		if (Part.empty())
			return;

		// юзер хочет относительности
		switch (Part.front())
		{
		case L'-':
			Part.erase(0, 1);
			Dest.relative = -1;
			break;

		case L'+':
			Part.erase(0, 1);
			Dest.relative = +1;
			break;

		default:
			break;
		}

		if (Part.empty())
			return;

		// он хочет процентов
		if (Part.back() == L'%')
		{
			Part.pop_back();
			Dest.percent = true;
		}

		if (Part.empty())
			return;

		auto Radix = 0;

		// он умный - hex код ввел!
		if (starts_with(Part, L"0x"sv))
		{
			Part.erase(0, 2);
			Radix = 16;
		}
		else if (starts_with(Part, L"$"sv))
		{
			Part.erase(0, 1);
			Radix = 16;
		}
		else if (ends_with(Part, L"h"sv))
		{
			Part.pop_back();
			Radix = 16;
		}
		else if (ends_with(Part, L"m"sv))
		{
			Part.pop_back();
			Radix = 10;
		}

		if (Part.empty())
			return;

		if (!Radix)
			Radix = Hex? 16 : 10;

		Dest.value = from_string<unsigned long long>(Part, nullptr, Radix);
		Dest.exist = true;
	};

	const auto SeparatorPos = Str.find_first_of(L".,;:"sv);

	if (SeparatorPos == Str.npos)
	{
		Parse(Str, Row);
	}
	else
	{
		Parse(Str.substr(0, SeparatorPos), Row);
		Parse(Str.substr(SeparatorPos + 1), Col);
	}
}

bool GoToRowCol(goto_coord& Row, goto_coord& Col, bool& Hex, string_view const HelpTopic)
{
	BoolOption HexOption;
	HexOption.Set(Hex);

	DialogBuilder Builder(lng::MGoTo, HelpTopic);
	string strData;
	Builder.AddEditField(strData, 28, L"LineNumber"sv, DIF_FOCUS | DIF_HISTORY | DIF_USELASTHISTORY | DIF_NOAUTOCOMPLETE);
	Builder.AddSeparator();
	Builder.AddCheckbox(lng::MGoToHex, HexOption);
	Builder.AddOKCancel();

	if (!Builder.ShowDialog())
		return false;

	Hex = HexOption;

	try
	{
		GetRowCol(strData, Hex, Row, Col);
		return true;
	}
	catch (const std::exception&)
	{
		// TODO: log
		// maybe we need to display a message in case of an incorrect input
		return false;
	}
}

bool RetryAbort(std::vector<string>&& Messages)
{
	if (Global->WindowManager && !Global->WindowManager->ManagerIsDown() && far_language::instance().is_loaded())
	{
		return Message(FMSG_WARNING,
			msg(lng::MError),
			std::move(Messages),
			{ lng::MRetry, lng::MAbort }) == Message::first_button;
	}

	std::wcerr << L"\nError:\n\n"sv;

	for (const auto& i: Messages)
		std::wcerr << i << L'\n';

	return ConsoleYesNo(L"Retry"sv, false);
}
