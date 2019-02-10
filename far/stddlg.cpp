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

#include "stddlg.hpp"

#include "dialog.hpp"
#include "strmix.hpp"
#include "imports.hpp"
#include "message.hpp"
#include "lang.hpp"
#include "DlgGuid.hpp"
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

#include "platform.fs.hpp"

#include "common/from_string.hpp"
#include "common/function_ref.hpp"
#include "common/function_traits.hpp"
#include "common/scope_exit.hpp"

#include "format.hpp"

int GetSearchReplaceString(
	bool IsReplaceMode,
	const wchar_t *Title,
	const wchar_t *SubTitle,
	string& SearchStr,
	string& ReplaceStr,
	const wchar_t *TextHistoryName,
	const wchar_t *ReplaceHistoryName,
	bool* pCase,
	bool* pWholeWords,
	bool* pReverse,
	bool* pRegexp,
	bool* pPreserveStyle,
	const wchar_t *HelpTopic,
	bool HideAll,
	const GUID* Id,
	function_ref<string(bool)> const Picker)
{
	int Result = 0;

	if (!TextHistoryName)
		TextHistoryName = L"SearchText";

	if (!ReplaceHistoryName)
		ReplaceHistoryName = L"ReplaceText";

	if (!Title)
		Title = msg(IsReplaceMode? lng::MEditReplaceTitle : lng::MEditSearchTitle).c_str();

	if (!SubTitle)
		SubTitle = msg(lng::MEditSearchFor).c_str();


	bool Case=pCase?*pCase:false;
	bool WholeWords=pWholeWords?*pWholeWords:false;
	bool Reverse=pReverse?*pReverse:false;
	bool Regexp=pRegexp?*pRegexp:false;
	bool PreserveStyle=pPreserveStyle?*pPreserveStyle:false;

	const auto DlgWidth = 76;
	const auto WordLabel = msg(lng::MEditSearchPickWord).c_str();
	const auto SelectionLabel = msg(lng::MEditSearchPickSelection).c_str();
	const auto WordButtonSize = HiStrlen(WordLabel) + 4;
	const auto SelectionButtonSize = HiStrlen(SelectionLabel) + 4;
	const auto SelectionButtonX2 = static_cast<intptr_t>(DlgWidth - 4 - 1);
	const auto SelectionButtonX1 = static_cast<intptr_t>(SelectionButtonX2 - SelectionButtonSize);
	const auto WordButtonX2 = static_cast<intptr_t>(SelectionButtonX1 - 1);
	const auto WordButtonX1 = static_cast<intptr_t>(WordButtonX2 - WordButtonSize);

	const auto YCorrection = IsReplaceMode? 0 : 2;

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
	};

	FarDialogItem ReplaceDlgData[]=
	{
		{ DI_DOUBLEBOX, 3, 1, DlgWidth - 4, 12 - YCorrection, 0, nullptr, nullptr, 0, Title },
		{ DI_BUTTON, WordButtonX1, 2, WordButtonX2, 2, 0, nullptr, nullptr, DIF_BTNNOCLOSE, WordLabel },
		{ DI_BUTTON, SelectionButtonX1, 2, SelectionButtonX2, 2, 0, nullptr, nullptr, DIF_BTNNOCLOSE, SelectionLabel },
		{ DI_TEXT, 5, 2, 0, 2, 0, nullptr, nullptr, 0, SubTitle },
		{ DI_EDIT, 5, 3, 70, 3, 0, TextHistoryName, nullptr, DIF_FOCUS | DIF_USELASTHISTORY | (*TextHistoryName? DIF_HISTORY : 0), SearchStr.c_str() },
		{ DI_TEXT, 5, 4, 0, 4, 0, nullptr, nullptr, 0, msg(lng::MEditReplaceWith).c_str() },
		{ DI_EDIT, 5, 5, 70, 5, 0, ReplaceHistoryName, nullptr, DIF_USELASTHISTORY | (*ReplaceHistoryName? DIF_HISTORY : 0), ReplaceStr.c_str() },
		{ DI_TEXT, -1, 6 - YCorrection, 0, 6 - YCorrection, 0, nullptr, nullptr, DIF_SEPARATOR, L"" },
		{ DI_CHECKBOX, 5, 7 - YCorrection, 0, 7 - YCorrection, Case, nullptr, nullptr, 0, msg(lng::MEditSearchCase).c_str() },
		{ DI_CHECKBOX, 5, 8 - YCorrection, 0, 8 - YCorrection, WholeWords, nullptr, nullptr, 0, msg(lng::MEditSearchWholeWords).c_str() },
		{ DI_CHECKBOX, 5, 9 - YCorrection, 0, 9 - YCorrection, Reverse, nullptr, nullptr, 0, msg(lng::MEditSearchReverse).c_str() },
		{ DI_CHECKBOX, 40, 7 - YCorrection, 0, 7 - YCorrection, Regexp, nullptr, nullptr, 0, msg(lng::MEditSearchRegexp).c_str() },
		{ DI_CHECKBOX, 40, 8 - YCorrection, 0, 8 - YCorrection, PreserveStyle, nullptr, nullptr, 0, msg(lng::MEditSearchPreserveStyle).c_str() },
		{ DI_TEXT, -1, 10 - YCorrection, 0, 10 - YCorrection, 0, nullptr, nullptr, DIF_SEPARATOR, L"" },
		{ DI_BUTTON, 0, 11 - YCorrection, 0, 11 - YCorrection, 0, nullptr, nullptr, DIF_DEFAULTBUTTON | DIF_CENTERGROUP, msg(IsReplaceMode? lng::MEditReplaceReplace : lng::MEditSearchSearch).c_str() },
		{ DI_BUTTON, 0, 11 - YCorrection, 0, 11 - YCorrection, 0, nullptr, nullptr, DIF_CENTERGROUP, msg(lng::MEditSearchAll).c_str() },
		{ DI_BUTTON, 0, 11 - YCorrection, 0, 11 - YCorrection, 0, nullptr, nullptr, DIF_CENTERGROUP, msg(lng::MEditSearchCancel).c_str() },
	};
	auto ReplaceDlg = MakeDialogItemsEx(ReplaceDlgData);

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

	const auto& Handler = [&](Dialog* Dlg, intptr_t Msg, intptr_t Param1, void* Param2) -> intptr_t
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
	Dlg->SetPosition({ -1, -1, DlgWidth, 14 - YCorrection });

	if (HelpTopic && *HelpTopic)
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
	const GUID* const Id
)
{
	int Substract=5; // дополнительная величина :-)
	int ExitCode;
	const auto addCheckBox = Flags&FIB_CHECKBOX && CheckBoxValue && !CheckBoxText.empty();
	const auto offset = addCheckBox? 2 : 0;
	FarDialogItem StrDlgData[]=
	{
		{DI_DOUBLEBOX, 3, 1, 72, 4, 0, nullptr, nullptr, 0,                                L""},
		{DI_TEXT,      5, 2,  0, 2, 0, nullptr, nullptr, DIF_SHOWAMPERSAND,                L""},
		{DI_EDIT,      5, 3, 70, 3, 0, nullptr, nullptr, DIF_FOCUS|DIF_DEFAULTBUTTON,      L""},
		{DI_TEXT,     -1, 4,  0, 4, 0, nullptr, nullptr, DIF_SEPARATOR,                    L""},
		{DI_CHECKBOX,  5, 5,  0, 5, 0, nullptr, nullptr, 0,                                L""},
		{DI_TEXT,     -1, 6,  0, 6, 0, nullptr, nullptr, DIF_SEPARATOR,                    L""},
		{DI_BUTTON,    0, 7,  0, 7, 0, nullptr, nullptr, DIF_CENTERGROUP,                  L""},
		{DI_BUTTON,    0, 7,  0, 7, 0, nullptr, nullptr, DIF_CENTERGROUP,                  L""},
	};
	auto StrDlg = MakeDialogItemsEx(StrDlgData);

	if (addCheckBox)
	{
		Substract-=2;
		StrDlg[0].Y2+=2;
		StrDlg[4].Selected = *CheckBoxValue != 0;
		StrDlg[4].strData = CheckBoxText;
	}

	if (Flags&FIB_BUTTONS)
	{
		Substract-=3;
		StrDlg[0].Y2+=2;
		StrDlg[2].Flags&=~DIF_DEFAULTBUTTON;
		StrDlg[5+offset].Y1=StrDlg[4+offset].Y1=5+offset;
		StrDlg[4+offset].Type=StrDlg[5+offset].Type=DI_BUTTON;
		StrDlg[4+offset].Flags=StrDlg[5+offset].Flags=DIF_CENTERGROUP;
		StrDlg[4+offset].Flags|=DIF_DEFAULTBUTTON;
		StrDlg[4+offset].strData = msg(lng::MOk);
		StrDlg[5+offset].strData = msg(lng::MCancel);
	}

	if (Flags&FIB_EXPANDENV)
	{
		StrDlg[2].Flags|=DIF_EDITEXPAND;
	}

	if (Flags&FIB_EDITPATH)
	{
		StrDlg[2].Flags|=DIF_EDITPATH;
	}

	if (Flags&FIB_EDITPATHEXEC)
	{
		StrDlg[2].Flags|=DIF_EDITPATHEXEC;
	}

	if (!HistoryName.empty())
	{
		StrDlg[2].strHistory = HistoryName;
		StrDlg[2].Flags|=DIF_HISTORY|(Flags&FIB_NOUSELASTHISTORY?0:DIF_USELASTHISTORY);
	}

	if (Flags&FIB_PASSWORD)
		StrDlg[2].Type=DI_PSWEDIT;

	if (!Title.empty())
		StrDlg[0].strData = Title;

	if (!Prompt.empty())
	{
		StrDlg[1].strData = Prompt;
		TruncStrFromEnd(StrDlg[1].strData, 66);

		if (Flags&FIB_NOAMPERSAND)
			StrDlg[1].Flags&=~DIF_SHOWAMPERSAND;
	}

	if (!SrcText.empty())
		StrDlg[2].strData = SrcText;

	{
		const auto Dlg = Dialog::create(make_span(StrDlg.data(), StrDlg.size() - Substract));
		Dlg->SetPosition({ -1, -1, 76, offset + (Flags & FIB_BUTTONS? 8 : 6) });
		if(Id) Dlg->SetId(*Id);

		if (!HelpTopic.empty())
			Dlg->SetHelp(HelpTopic);

		Dlg->SetPluginOwner(PluginNumber);

		Dlg->Process();

		ExitCode=Dlg->GetExitCode();
	}

	if (ExitCode == 2 || ExitCode == 4 || (addCheckBox && ExitCode == 6))
	{
		if (!(Flags&FIB_ENABLEEMPTY) && StrDlg[2].strData.empty())
			return false;

		strDestText = StrDlg[2].strData;

		if (addCheckBox)
			*CheckBoxValue=StrDlg[4].Selected;

		return true;
	}
	
	return false;
}

/*
  Стандартный диалог ввода пароля.
  Умеет сам запоминать последнего юзвера и пароль.

  Name      - сюда будет помещен юзвер (max 256 символов!!!)
  Password  - сюда будет помещен пароль (max 256 символов!!!)
  Title     - заголовок диалога (может быть nullptr)
  HelpTopic - тема помощи (может быть nullptr)
  Flags     - флаги (GNP_*)
*/
bool GetNameAndPassword(const string& Title, string &strUserName, string &strPassword,const wchar_t *HelpTopic,DWORD Flags)
{
	static string strLastName, strLastPassword;
	int ExitCode;
	/*
	  0         1         2         3         4         5         6         7
	  0123456789012345678901234567890123456789012345678901234567890123456789012345
	|0                                                                             |
	|1   +------------------------------- Title -------------------------------+   |
	|2   | User name                                                           |   |
	|3   | *******************************************************************|   |
	|4   | User password                                                       |   |
	|5   | ******************************************************************* |   |
	|6   +---------------------------------------------------------------------+   |
	|7   |                         [ Ok ]   [ Cancel ]                         |   |
	|8   +---------------------------------------------------------------------+   |
	|9                                                                             |
	*/
	FarDialogItem PassDlgData[]=
	{
		{DI_DOUBLEBOX,  3, 1,72, 8,0,nullptr,nullptr,0,NullToEmpty(Title.c_str())},
		{DI_TEXT,       5, 2, 0, 2,0,nullptr,nullptr,0,msg(lng::MNetUserName).c_str()},
		{DI_EDIT,       5, 3,70, 3,0,L"NetworkUser",nullptr,DIF_FOCUS|DIF_USELASTHISTORY|DIF_HISTORY,(Flags & GNP_USELAST)? strLastName.c_str() : strUserName.c_str()},
		{DI_TEXT,       5, 4, 0, 4,0,nullptr,nullptr,0,msg(lng::MNetUserPassword).c_str()},
		{DI_PSWEDIT,    5, 5,70, 5,0,nullptr,nullptr,0,(Flags & GNP_USELAST)? strLastPassword.c_str() : strPassword.c_str()},
		{DI_TEXT,      -1, 6, 0, 6,0,nullptr,nullptr,DIF_SEPARATOR,L""},
		{DI_BUTTON,     0, 7, 0, 7,0,nullptr,nullptr,DIF_DEFAULTBUTTON|DIF_CENTERGROUP,msg(lng::MOk).c_str()},
		{DI_BUTTON,     0, 7, 0, 7,0,nullptr,nullptr,DIF_CENTERGROUP,msg(lng::MCancel).c_str()},
	};
	auto PassDlg = MakeDialogItemsEx(PassDlgData);

	{
		const auto Dlg = Dialog::create(PassDlg);
		Dlg->SetPosition({ -1, -1, 76, 10 });
		Dlg->SetId(GetNameAndPasswordId);

		if (HelpTopic)
			Dlg->SetHelp(HelpTopic);

		Dlg->Process();
		ExitCode=Dlg->GetExitCode();
	}

	if (ExitCode!=6)
		return false;

	// запоминаем всегда.
	strUserName = PassDlg[2].strData;
	strLastName = strUserName;
	strPassword = PassDlg[4].strData;
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
	WCHAR SessionKey[CCH_RM_SESSION_KEY + 1] = {};
	if (imports.RmStartSession(&Session, 0, SessionKey) != ERROR_SUCCESS)
		return 0;

	SCOPE_EXIT{ imports.RmEndSession(Session); };
	auto FilenamePtr = Filename.c_str();
	if (imports.RmRegisterResources(Session, 1, &FilenamePtr, 0, nullptr, 0, nullptr) != ERROR_SUCCESS)
		return 0;

	DWORD RmGetListResult;
	UINT ProceccInfoSizeNeeded;
	UINT ProcessInfoSize = 1;
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

operation OperationFailed(const error_state_ex& ErrorState, string Object, lng Title, string Description, bool AllowSkip, bool AllowSkipAll)
{
	std::vector<string> Msg;
	os::com::ptr<IFileIsInUse> FileIsInUse;
	lng Reason = lng::MObjectLockedReasonOpened;
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

			for (const auto& i: Mappings)
			{
				if (Reasons & i.first)
				{
					if (!SeparatorAdded)
					{
						Msg.emplace_back(L"\1"sv);
						SeparatorAdded = true;
					}

					Msg.emplace_back(msg(i.second));
				}
			}
		}
	}

	std::vector<string> Msgs{std::move(Description), QuoteOuterSpace(std::move(Object))};
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

	std::unique_ptr<listener> Listener;
	if (SwitchBtn)
	{
		Listener = std::make_unique<listener>([](const std::any& Payload)
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
};

void ReCompileErrorMessage(const RegExp& re, const string& str)
{
	Message(MSG_WARNING | MSG_LEFTALIGN,
		msg(lng::MError),
		{
			GetReErrorString(re.LastError()),
			str,
			string(re.ErrorPosition(), L' ') + L'^'
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
	const auto& Parse = [Hex](string Part, goto_coord& Dest)
	{
		Part.resize(std::remove(ALL_RANGE(Part), L' ') - Part.begin());

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

	const auto SeparatorPos = Str.find_first_of(L".,;:");

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

int RetryAbort(std::vector<string>&& Messages)
{
	if (Global->WindowManager && !Global->WindowManager->ManagerIsDown())
	{
		return Message(FMSG_WARNING,
			msg(lng::MError),
			std::move(Messages),
			{ lng::MRetry, lng::MAbort }) == Message::first_button;
	}

	std::wcerr << L"\nError:\n\n"sv;

	for (const auto& i: Messages)
		std::wcerr << i << L'\n';

	return ConsoleYesNo(L"Retry"sv);
}
