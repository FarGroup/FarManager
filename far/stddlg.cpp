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

#include "headers.hpp"
#pragma hdrstop

#include "stddlg.hpp"
#include "dialog.hpp"
#include "strmix.hpp"
#include "imports.hpp"
#include "message.hpp"
#include "language.hpp"
#include "DlgGuid.hpp"
#include "datetime.hpp"
#include "interf.hpp"
#include "dlgedit.hpp"
#include "cvtname.hpp"

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
	const std::function<string(bool)>& Picker)
{
	int Result = 0;

	if (!TextHistoryName)
		TextHistoryName = L"SearchText";

	if (!ReplaceHistoryName)
		ReplaceHistoryName = L"ReplaceText";

	if (!Title)
		Title=MSG(IsReplaceMode? lng::MEditReplaceTitle : lng::MEditSearchTitle);

	if (!SubTitle)
		SubTitle=MSG(lng::MEditSearchFor);


	bool Case=pCase?*pCase:false;
	bool WholeWords=pWholeWords?*pWholeWords:false;
	bool Reverse=pReverse?*pReverse:false;
	bool Regexp=pRegexp?*pRegexp:false;
	bool PreserveStyle=pPreserveStyle?*pPreserveStyle:false;

	const auto DlgWidth = 76;
	const auto WordLabel = MSG(lng::MEditSearchPickWord);
	const auto SelectionLabel = MSG(lng::MEditSearchPickSelection);
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
		{ DI_EDIT, 5, 3, 70, 3, 0, TextHistoryName, nullptr, DIF_FOCUS | DIF_USELASTHISTORY | (*TextHistoryName? DIF_HISTORY : 0), SearchStr.data() },
		{ DI_TEXT, 5, 4, 0, 4, 0, nullptr, nullptr, 0, MSG(lng::MEditReplaceWith) },
		{ DI_EDIT, 5, 5, 70, 5, 0, ReplaceHistoryName, nullptr, DIF_USELASTHISTORY | (*ReplaceHistoryName? DIF_HISTORY : 0), ReplaceStr.data() },
		{ DI_TEXT, -1, 6 - YCorrection, 0, 6 - YCorrection, 0, nullptr, nullptr, DIF_SEPARATOR, L"" },
		{ DI_CHECKBOX, 5, 7 - YCorrection, 0, 7 - YCorrection, Case, nullptr, nullptr, 0, MSG(lng::MEditSearchCase) },
		{ DI_CHECKBOX, 5, 8 - YCorrection, 0, 8 - YCorrection, WholeWords, nullptr, nullptr, 0, MSG(lng::MEditSearchWholeWords) },
		{ DI_CHECKBOX, 5, 9 - YCorrection, 0, 9 - YCorrection, Reverse, nullptr, nullptr, 0, MSG(lng::MEditSearchReverse) },
		{ DI_CHECKBOX, 40, 7 - YCorrection, 0, 7 - YCorrection, Regexp, nullptr, nullptr, 0, MSG(lng::MEditSearchRegexp) },
		{ DI_CHECKBOX, 40, 8 - YCorrection, 0, 8 - YCorrection, PreserveStyle, nullptr, nullptr, 0, MSG(lng::MEditSearchPreserveStyle) },
		{ DI_TEXT, -1, 10 - YCorrection, 0, 10 - YCorrection, 0, nullptr, nullptr, DIF_SEPARATOR, L"" },
		{ DI_BUTTON, 0, 11 - YCorrection, 0, 11 - YCorrection, 0, nullptr, nullptr, DIF_DEFAULTBUTTON | DIF_CENTERGROUP, MSG(IsReplaceMode? lng::MEditReplaceReplace : lng::MEditSearchSearch) },
		{ DI_BUTTON, 0, 11 - YCorrection, 0, 11 - YCorrection, 0, nullptr, nullptr, DIF_CENTERGROUP, MSG(lng::MEditSearchAll) },
		{ DI_BUTTON, 0, 11 - YCorrection, 0, 11 - YCorrection, 0, nullptr, nullptr, DIF_CENTERGROUP, MSG(lng::MEditSearchCancel) },
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
	Dlg->SetPosition(-1, -1, DlgWidth, 14 - YCorrection);

	if (HelpTopic && *HelpTopic)
		Dlg->SetHelp(HelpTopic);

	if(Id) Dlg->SetId(*Id);

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

int GetString(
	const wchar_t *Title,
	const wchar_t *Prompt,
	const wchar_t *HistoryName,
	const wchar_t *SrcText,
	string &strDestText,
	const wchar_t *HelpTopic,
	DWORD Flags,
	int *CheckBoxValue,
	const wchar_t *CheckBoxText,
	Plugin* PluginNumber,
	const GUID* Id
)
{
	int Substract=5; // дополнительная величина :-)
	int ExitCode;
	bool addCheckBox=Flags&FIB_CHECKBOX && CheckBoxValue && CheckBoxText;
	int offset=addCheckBox?2:0;
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
		StrDlg[4+offset].strData = MSG(lng::MOk);
		StrDlg[5+offset].strData = MSG(lng::MCancel);
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

	if (HistoryName)
	{
		StrDlg[2].strHistory=HistoryName;
		StrDlg[2].Flags|=DIF_HISTORY|(Flags&FIB_NOUSELASTHISTORY?0:DIF_USELASTHISTORY);
	}

	if (Flags&FIB_PASSWORD)
		StrDlg[2].Type=DI_PSWEDIT;

	if (Title)
		StrDlg[0].strData = Title;

	if (Prompt)
	{
		StrDlg[1].strData = Prompt;
		TruncStrFromEnd(StrDlg[1].strData, 66);

		if (Flags&FIB_NOAMPERSAND)
			StrDlg[1].Flags&=~DIF_SHOWAMPERSAND;
	}

	if (SrcText)
		StrDlg[2].strData = SrcText;

	{
		const auto Dlg = Dialog::create(make_range(StrDlg.data(), StrDlg.size() - Substract));
		Dlg->SetPosition(-1,-1,76,offset+((Flags&FIB_BUTTONS)?8:6));
		if(Id) Dlg->SetId(*Id);

		if (HelpTopic)
			Dlg->SetHelp(HelpTopic);

		Dlg->SetPluginOwner(PluginNumber);

		Dlg->Process();

		ExitCode=Dlg->GetExitCode();
	}

	if (ExitCode == 2 || ExitCode == 4 || (addCheckBox && ExitCode == 6))
	{
		if (!(Flags&FIB_ENABLEEMPTY) && StrDlg[2].strData.empty())
			return FALSE;

		strDestText = StrDlg[2].strData;

		if (addCheckBox)
			*CheckBoxValue=StrDlg[4].Selected;

		return TRUE;
	}

	return FALSE;
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
int GetNameAndPassword(const string& Title, string &strUserName, string &strPassword,const wchar_t *HelpTopic,DWORD Flags)
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
		{DI_DOUBLEBOX,  3, 1,72, 8,0,nullptr,nullptr,0,NullToEmpty(Title.data())},
		{DI_TEXT,       5, 2, 0, 2,0,nullptr,nullptr,0,MSG(lng::MNetUserName)},
		{DI_EDIT,       5, 3,70, 3,0,L"NetworkUser",nullptr,DIF_FOCUS|DIF_USELASTHISTORY|DIF_HISTORY,(Flags&GNP_USELAST)?strLastName.data():strUserName.data()},
		{DI_TEXT,       5, 4, 0, 4,0,nullptr,nullptr,0,MSG(lng::MNetUserPassword)},
		{DI_PSWEDIT,    5, 5,70, 5,0,nullptr,nullptr,0,(Flags&GNP_USELAST)?strLastPassword.data():strPassword.data()},
		{DI_TEXT,      -1, 6, 0, 6,0,nullptr,nullptr,DIF_SEPARATOR,L""},
		{DI_BUTTON,     0, 7, 0, 7,0,nullptr,nullptr,DIF_DEFAULTBUTTON|DIF_CENTERGROUP,MSG(lng::MOk)},
		{DI_BUTTON,     0, 7, 0, 7,0,nullptr,nullptr,DIF_CENTERGROUP,MSG(lng::MCancel)},
	};
	auto PassDlg = MakeDialogItemsEx(PassDlgData);

	{
		const auto Dlg = Dialog::create(PassDlg);
		Dlg->SetPosition(-1,-1,76,10);
		Dlg->SetId(GetNameAndPasswordId);

		if (HelpTopic)
			Dlg->SetHelp(HelpTopic);

		Dlg->Process();
		ExitCode=Dlg->GetExitCode();
	}

	if (ExitCode!=6)
		return FALSE;

	// запоминаем всегда.
	strUserName = PassDlg[2].strData;
	strLastName = strUserName;
	strPassword = PassDlg[4].strData;
	strLastPassword = strPassword;
	return TRUE;
}

static os::com::ptr<IFileIsInUse> CreateIFileIsInUse(const string& File)
{
	os::com::ptr<IRunningObjectTable> rot;
	if (FAILED(GetRunningObjectTable(0, &ptr_setter(rot))))
		return nullptr;

	os::com::ptr<IMoniker> mkFile;
	if (FAILED(CreateFileMoniker(File.data(), &ptr_setter(mkFile))))
		return nullptr;

	os::com::ptr<IEnumMoniker> enumMk;
	if (FAILED(rot->EnumRunning(&ptr_setter(enumMk))))
		return nullptr;

	for(;;)
	{
		os::com::ptr<IMoniker> mk;
		ULONG celt;
		if (enumMk->Next(1, &ptr_setter(mk), &celt) != S_OK)
			return nullptr;

		DWORD dwType;
		if (FAILED(mk->IsSystemMoniker(&dwType)) || dwType != MKSYS_FILEMONIKER)
			continue;

		os::com::ptr<IMoniker> mkPrefix;
		if (FAILED(mkFile->CommonPrefixWith(mk.get(), &ptr_setter(mkPrefix))))
			continue;

		if (mkFile->IsEqual(mkPrefix.get()) != S_OK)
			continue;

		os::com::ptr<IUnknown> unk;
		if (rot->GetObject(mk.get(), &ptr_setter(unk)) != S_OK)
			continue;

		FN_RETURN_TYPE(CreateIFileIsInUse) fiu;
		if (SUCCEEDED(unk->QueryInterface(IID_IFileIsInUse, IID_PPV_ARGS_Helper(&ptr_setter(fiu)))))
			return fiu;
	}
}

operation OperationFailed(const string& Object, lng Title, const string& Description, bool AllowSkip)
{
	std::vector<string> Msg;
	os::com::ptr<IFileIsInUse> fiu;
	lng Reason = lng::MObjectLockedReasonOpened;
	bool SwitchBtn = false, CloseBtn = false;
	const auto Error = Global->CaughtError().Win32Error;
	if(Error == ERROR_ACCESS_DENIED ||
		Error == ERROR_SHARING_VIOLATION ||
		Error == ERROR_LOCK_VIOLATION ||
		Error == ERROR_DRIVE_LOCKED)
	{
		const auto FullName = ConvertNameToFull(Object);
		fiu = CreateIFileIsInUse(FullName);
		if (fiu)
		{
			auto UsageType = FUT_GENERIC;
			fiu->GetUsage(&UsageType);
			switch(UsageType)
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
			DWORD Capabilities = 0;
			fiu->GetCapabilities(&Capabilities);
			
			SwitchBtn = (Capabilities & OF_CAP_CANSWITCHTO) != 0;
			CloseBtn = (Capabilities & OF_CAP_CANCLOSE) != 0;

			wchar_t* AppName = nullptr;
			if(SUCCEEDED(fiu->GetAppName(&AppName)))
			{
				Msg.emplace_back(AppName);
			}
		}
		else
		{
			DWORD dwSession;
			WCHAR szSessionKey[CCH_RM_SESSION_KEY+1] = {};
			if (Imports().RmStartSession(&dwSession, 0, szSessionKey) == ERROR_SUCCESS)
			{
				SCOPE_EXIT{ Imports().RmEndSession(dwSession); };
				auto pszFile = FullName.data();
				if (Imports().RmRegisterResources(dwSession, 1, &pszFile, 0, nullptr, 0, nullptr) == ERROR_SUCCESS)
				{
					DWORD dwReason;
					DWORD RmGetListResult;
					UINT nProcInfoNeeded;
					UINT nProcInfo = 1;
					std::vector<RM_PROCESS_INFO> rgpi(nProcInfo);
					while((RmGetListResult=Imports().RmGetList(dwSession, &nProcInfoNeeded, &nProcInfo, rgpi.data(), &dwReason)) == ERROR_MORE_DATA)
					{
						nProcInfo = nProcInfoNeeded;
						rgpi.resize(nProcInfo);
					}
					if(RmGetListResult ==ERROR_SUCCESS)
					{
						for (const auto& i: rgpi)
						{
							string tmp = *i.strAppName? i.strAppName : L"Unknown";

							if (*i.strServiceShortName)
							{
								append(tmp, L" ["s, i.strServiceShortName, L']');
							}
							append(tmp, L" (PID: "s, str(i.Process.dwProcessId));
							if (const auto Process = os::handle(OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, i.Process.dwProcessId)))
							{
								FILETIME ftCreate, ftExit, ftKernel, ftUser;
								if (GetProcessTimes(Process.native_handle(), &ftCreate, &ftExit, &ftKernel, &ftUser) && i.Process.ProcessStartTime == ftCreate)
								{
									string Name;
									if (os::GetModuleFileNameEx(Process.native_handle(), nullptr, Name))
									{
										append(tmp, L", "s, Name);
									}
								}
							}
							tmp += L')';
							Msg.emplace_back(tmp);
						}
					}
				}
			}
		}
	}

	std::vector<string> Msgs{Description, QuoteOuterSpace(string(Object))};
	if(!Msg.empty())
	{
		Msgs.emplace_back(format(lng::MObjectLockedReason, MSG(Reason)));
		Msgs.insert(Msgs.end(), ALL_CONST_RANGE(Msg));
	}

	std::vector<string> Buttons;
	Buttons.reserve(4);
	if(SwitchBtn)
	{
		Buttons.emplace_back(MSG(lng::MObjectLockedSwitchTo));
	}
	Buttons.emplace_back(MSG(CloseBtn? lng::MObjectLockedClose : lng::MDeleteRetry));
	if(AllowSkip)
	{
		Buttons.emplace_back(MSG(lng::MDeleteSkip));
		Buttons.emplace_back(MSG(lng::MDeleteFileSkipAll));
	}
	Buttons.emplace_back(MSG(lng::MDeleteCancel));

	int Result = -1;
	for(;;)
	{
		Result = Message(MSG_WARNING|MSG_ERRORTYPE, MSG(Title), Msgs, Buttons);

		if(SwitchBtn)
		{
			if(Result == Message::first_button)
			{
				HWND Wnd = nullptr;
				if (fiu && SUCCEEDED(fiu->GetSwitchToHWND(&Wnd)))
				{
					SetForegroundWindow(Wnd);
					if (IsIconic(Wnd))
						ShowWindow(Wnd, SW_RESTORE);
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
			if (fiu)
			{
				fiu->CloseFile();
			}
		}
		break;
	}

	if (Result < 0 || (!AllowSkip && Result == Message::second_button))
		return operation::cancel;

	return static_cast<operation>(Result);
}

static string GetReErrorString(int code)
{
	// TODO: localization
	switch (code)
	{
	case errNone:
		return L"No errors";
	case errNotCompiled:
		return L"RegExp wasn't even tried to compile";
	case errSyntax:
		return L"Expression contains a syntax error";
	case errBrackets:
		return L"Unbalanced brackets";
	case errMaxDepth:
		return L"Max recursive brackets level reached";
	case errOptions:
		return L"Invalid options combination";
	case errInvalidBackRef:
		return L"Reference to nonexistent bracket";
	case errInvalidEscape:
		return L"Invalid escape char";
	case errInvalidRange:
		return L"Invalid range value";
	case errInvalidQuantifiersCombination:
		return L"Quantifier applied to invalid object. f.e. lookahead assertion";
	case errNotEnoughMatches:
		return L"Size of match array isn't large enough";
	case errNoStorageForNB:
		return L"Attempt to match RegExp with Named Brackets but no storage class provided";
	case errReferenceToUndefinedNamedBracket:
		return L"Reference to undefined named bracket";
	case errVariableLengthLookBehind:
		return L"Only fixed length look behind assertions are supported";
	default:
		return L"Unknown error";
	}
};

void ReCompileErrorMessage(const RegExp& re, const string& str)
{
	Message(MSG_WARNING | MSG_LEFTALIGN, MSG(lng::MError),
		{ GetReErrorString(re.LastError()), str, string(re.ErrorPosition(), L' ') + L'^' },
		{ MSG(lng::MOk) });
}

void ReMatchErrorMessage(const RegExp& re)
{
	if (re.LastError() != errNone)
	{
		Message(MSG_WARNING | MSG_LEFTALIGN, MSG(lng::MError),
			{ GetReErrorString(re.LastError()) },
			{ MSG(lng::MOk) });
	}
}
