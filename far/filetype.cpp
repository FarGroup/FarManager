/*
filetype.cpp

Работа с ассоциациями файлов
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
#include "filetype.hpp"

// Internal:
#include "keys.hpp"
#include "dialog.hpp"
#include "vmenu.hpp"
#include "vmenu2.hpp"
#include "preservelongname.hpp"
#include "ctrlobj.hpp"
#include "cmdline.hpp"
#include "history.hpp"
#include "filemasks.hpp"
#include "message.hpp"
#include "interf.hpp"
#include "config.hpp"
#include "execute.hpp"
#include "fnparce.hpp"
#include "configdb.hpp"
#include "lang.hpp"
#include "uuids.far.dialogs.hpp"
#include "global.hpp"
#include "keyboard.hpp"
#include "RegExp.hpp"

// Platform:
#include "platform.fs.hpp"

// Common:
#include "common.hpp"
#include "common/string_utils.hpp"

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

/* $ 14.01.2001 SVS
   Добавим интеллектуальности.
   Если встречается "IF" и оно выполняется, то команда
   помещается в список

   Вызывается для F3, F4 - ассоциации
   Enter в ком строке - ассоциации.
*/
/* $ 06.07.2001
   + Используем filemasks вместо GetCommaWord, этим самым добиваемся того, что
     можно использовать маски исключения
   - Убрал непонятный мне запрет на использование маски файлов типа "*.*"
     (был когда-то, вроде, такой баг-репорт)
*/
bool ProcessLocalFileTypes(string_view const Name, string_view const ShortName, FILETYPE_MODE Mode, bool AlwaysWaitFinish, string_view CurrentDirectory, bool AddToHistory, bool RunAs, function_ref<void(execute_info&)> const Launcher)
{
	std::optional<os::fs::current_directory_guard> Guard;
	// We have to set it - users can have associations like !.! which will work funny without this
	if (!CurrentDirectory.empty())
		Guard.emplace(CurrentDirectory);

	const subst_context Context(Name, ShortName);

	struct menu_data
	{
		string Command;
		std::vector<RegExpMatch> Matches;
		unordered_string_map<size_t> NamedMatches;
	};

	const auto AddMatches = [&](menu_data const& Data)
	{
		for (const auto& i: Data.Matches)
		{
			Context.Variables.emplace(
				far::format(L"RegexGroup{}"sv, &i - Data.Matches.data()),
				get_match(Context.Name, i)
			);
		}

		for (const auto& [GroupName, GroupNumber]: Data.NamedMatches)
		{
			const auto& Match = Data.Matches[GroupNumber];
			Context.Variables.emplace(
				far::format(L"RegexGroup{{{}}}"sv, GroupName),
				get_match(Context.Name, Match)
			);
		}
	};

	std::vector<menu_data> MenuData;

		int ActualCmdCount=0; // отображаемых ассоциаций в меню
		filemasks FMask; // для работы с масками файлов

		int CommandCount=0;

		std::vector<MenuItemEx> MenuItems;

		string strDescription;

		for(const auto& [Id, Mask]: ConfigProvider().AssocConfig()->TypedMasksEnumerator(Mode))
		{
			strDescription.clear();
			Context.Variables.clear();

			menu_data NewMenuData;
			filemasks::regex_matches const RegexMatches{ NewMenuData.Matches, NewMenuData.NamedMatches };

			if (FMask.assign(Mask, FMF_SILENT))
			{
				if (FMask.check(Context.Name, &RegexMatches))
				{
					ConfigProvider().AssocConfig()->GetCommand(Id, Mode, NewMenuData.Command);

					if (!NewMenuData.Command.empty())
					{
						ConfigProvider().AssocConfig()->GetDescription(Id, strDescription);
						CommandCount++;
					}
				}

				if (NewMenuData.Command.empty())
					continue;

				AddMatches(NewMenuData);
			}

			string strCommandText = NewMenuData.Command;
			if (
				!SubstFileName(strCommandText, Context, {}, true) ||
				// все "подставлено", теперь проверим условия "if exist"
				!ExtractIfExistCommand(strCommandText)
			)
				continue;

			ActualCmdCount++;

			if (!strDescription.empty())
				SubstFileName(strDescription, Context, {}, true, {}, true);
			else
				strDescription = std::move(strCommandText);

			MenuItemEx TypesMenuItem(strDescription);
			MenuData.emplace_back(std::move(NewMenuData));
			MenuItems.emplace_back(std::move(TypesMenuItem));
		}

		if (!CommandCount)
			return false;

		if (!ActualCmdCount)
			return true;

		int ExitCode=0;

		const auto TypesMenu = VMenu2::create(msg(lng::MSelectAssocTitle), {}, ScrY - 4);
		TypesMenu->SetHelp(L"FileAssoc"sv);
		TypesMenu->SetMenuFlags(VMENU_WRAPMODE);
		TypesMenu->SetId(SelectAssocMenuId);
		for (auto& Item : MenuItems)
		{
			TypesMenu->AddItem(std::move(Item));
		}

		if (ActualCmdCount>1)
		{
			ExitCode=TypesMenu->Run();

			if (ExitCode<0)
				return true;
		}

	auto& ItemData = MenuData[ExitCode];
	Context.Variables.clear();
	AddMatches(ItemData);

	bool PreserveLFN = false;
	if (SubstFileName(ItemData.Command, Context, &PreserveLFN) && !ItemData.Command.empty())
	{
		if (AddToHistory && !(Global->Opt->ExcludeCmdHistory & EXCLUDECMDHISTORY_NOTFARASS) && !AlwaysWaitFinish) //AN
		{
			const auto curDir = Global->CtrlObject->CmdLine()->GetCurDir();
			Global->CtrlObject->CmdHistory->AddToHistory(ItemData.Command, HR_DEFAULT, nullptr, {}, curDir);
		}

		SCOPED_ACTION(PreserveLongName)(Name, PreserveLFN);

		execute_info Info;
		Info.DisplayCommand = ItemData.Command;
		Info.Command = std::move(ItemData.Command);
		Info.Directory = CurrentDirectory;
		Info.WaitMode = AlwaysWaitFinish? execute_info::wait_mode::wait_finish : execute_info::wait_mode::if_needed;
		Info.RunAs = RunAs;
		// We've already processed them!
		Info.UseAssociations = false;

		Launcher? Launcher(Info) : Global->CtrlObject->CmdLine()->ExecString(Info);
	}

	return true;
}


bool GetFiletypeOpenMode(int keyPressed, FILETYPE_MODE& mode, bool& shouldForceInternal)
{
	bool isModeFound = false;

	switch (keyPressed)
	{
	case KEY_F3:
	case KEY_NUMPAD5:
	case KEY_SHIFTNUMPAD5:
		mode = Global->Opt->ViOpt.UseExternalViewer? FILETYPE_ALTVIEW : FILETYPE_VIEW;
		shouldForceInternal = false;
		isModeFound = true;
		break;

	case KEY_CTRLSHIFTF3:
	case KEY_RCTRLSHIFTF3:
		mode = FILETYPE_VIEW;
		shouldForceInternal = true;
		isModeFound = true;
		break;

	case KEY_RALTF3:
	case KEY_ALTF3:
		mode = Global->Opt->ViOpt.UseExternalViewer? FILETYPE_VIEW : FILETYPE_ALTVIEW;
		shouldForceInternal = false;
		isModeFound = true;
		break;

	case KEY_F4:
		mode = Global->Opt->EdOpt.UseExternalEditor? FILETYPE_ALTEDIT: FILETYPE_EDIT;
		shouldForceInternal = false;
		isModeFound = true;
		break;

	case KEY_CTRLSHIFTF4:
	case KEY_RCTRLSHIFTF4:
		mode = FILETYPE_EDIT;
		shouldForceInternal = true;
		isModeFound = true;
		break;

	case KEY_ALTF4:
	case KEY_RALTF4:
		mode = Global->Opt->EdOpt.UseExternalEditor? FILETYPE_EDIT : FILETYPE_ALTEDIT;
		shouldForceInternal = false;
		isModeFound = true;
		break;

	case KEY_ENTER:
		mode = FILETYPE_EXEC;
		isModeFound = true;
		break;

	case KEY_CTRLPGDN:
	case KEY_RCTRLPGDN:
		mode = FILETYPE_ALTEXEC;
		isModeFound = true;
		break;

	default:
		// non-default key may be pressed, it's ok.
		break;
	}

	return isModeFound;
}

/*
  Используется для запуска внешнего редактора и вьювера
*/
void ProcessExternal(string_view const Command, string_view const Name, string_view const ShortName, bool const AlwaysWaitFinish, string_view const CurrentDirectory)
{
	std::optional<os::fs::current_directory_guard> Guard;
	// We have to set it - users can have associations like !.! which will work funny without this
	if (!CurrentDirectory.empty())
		Guard.emplace(CurrentDirectory);

	string strExecStr(Command);
	bool PreserveLFN = false;
	if (!SubstFileName(strExecStr, { Name, ShortName }, &PreserveLFN) || strExecStr.empty())
		return;

	// If you want your history to be usable - use full paths yourself. We cannot reliably substitute them.
	Global->CtrlObject->ViewHistory->AddToHistory(strExecStr, AlwaysWaitFinish? HR_EXTERNAL_WAIT : HR_EXTERNAL);

	SCOPED_ACTION(PreserveLongName)(Name, PreserveLFN);

	execute_info Info;
	Info.DisplayCommand = strExecStr;
	Info.Command = std::move(strExecStr);
	Info.Directory = CurrentDirectory;
	Info.WaitMode = AlwaysWaitFinish? execute_info::wait_mode::wait_finish : execute_info::wait_mode::if_needed;

	Global->CtrlObject->CmdLine()->ExecString(Info);
}

static auto FillFileTypesMenu(VMenu2* TypesMenu, int MenuPos)
{
	struct data_item
	{
		unsigned long long Id;
		string Mask;
		string Description;
	};

	std::vector<data_item> Data;

	for(auto& [Id, Mask]: ConfigProvider().AssocConfig()->MasksEnumerator())
	{
		data_item Item{ Id, std::move(Mask) };
		ConfigProvider().AssocConfig()->GetDescription(Item.Id, Item.Description);
		Data.emplace_back(std::move(Item));
	}

	const auto MaxElementSize = std::ranges::fold_left(Data, 0uz, [](size_t const Value, data_item const & i){ return std::max(Value, i.Description.size()); });

	TypesMenu->clear();

	for (const auto& i: Data)
	{
		const auto AddLen = i.Description.size() - HiStrlen(i.Description);
		MenuItemEx TypesMenuItem(concat(fit_to_left(i.Description, MaxElementSize + AddLen), L' ', BoxSymbols[BS_V1], L' ', i.Mask));
		TypesMenuItem.ComplexUserData = i.Id;
		TypesMenu->AddItem(TypesMenuItem);
	}

	TypesMenu->SetSelectPos(MenuPos);

	return static_cast<int>(Data.size());
}

enum EDITTYPERECORD
{
	ETR_DOUBLEBOX,
	ETR_TEXT_MASKS,
	ETR_EDIT_MASKS,
	ETR_TEXT_DESCR,
	ETR_EDIT_DESCR,
	ETR_SEPARATOR1,
	ETR_CHECK_EXEC,
	ETR_EDIT_EXEC,
	ETR_CHECK_ALTEXEC,
	ETR_EDIT_ALTEXEC,
	ETR_CHECK_VIEW,
	ETR_EDIT_VIEW,
	ETR_CHECK_ALTVIEW,
	ETR_EDIT_ALTVIEW,
	ETR_CHECK_EDIT,
	ETR_EDIT_EDIT,
	ETR_CHECK_ALTEDIT,
	ETR_EDIT_ALTEDIT,
	ETR_SEPARATOR2,
	ETR_BUTTON_OK,
	ETR_BUTTON_CANCEL,

	ETR_COUNT
};

static intptr_t EditTypeRecordDlgProc(Dialog* Dlg,intptr_t Msg,intptr_t Param1,void* Param2)
{
	switch (Msg)
	{
		case DN_BTNCLICK:

			switch (Param1)
			{
				case ETR_CHECK_EXEC:
				case ETR_CHECK_ALTEXEC:
				case ETR_CHECK_VIEW:
				case ETR_CHECK_ALTVIEW:
				case ETR_CHECK_EDIT:
				case ETR_CHECK_ALTEDIT:
					Dlg->SendMessage(DM_ENABLE,Param1+1,ToPtr(std::bit_cast<intptr_t>(Param2) == BSTATE_CHECKED));
					break;
				default:
					break;
			}

			break;
		case DN_CLOSE:

			if (Param1==ETR_BUTTON_OK)
			{
				return filemasks().assign(std::bit_cast<const wchar_t*>(Dlg->SendMessage(DM_GETCONSTTEXTPTR, ETR_EDIT_MASKS, nullptr)));
			}
			break;

		default:
			break;
	}

	return Dlg->DefProc(Msg,Param1,Param2);
}

static bool EditTypeRecord(unsigned long long EditPos,bool NewRec)
{
	const int DlgX=76,DlgY=23;

	auto EditDlg = MakeDialogItems<ETR_COUNT>(
	{
		{ DI_DOUBLEBOX, {{3,   1,   }, {DlgX-4, DlgY-2}}, DIF_NONE, msg(lng::MFileAssocTitle), },
		{ DI_TEXT,      {{5,   2,   }, {0,      2     }}, DIF_NONE, msg(lng::MFileAssocMasks), },
		{ DI_EDIT,      {{5,   3,   }, {DlgX-6, 3     }}, DIF_FOCUS | DIF_HISTORY, },
		{ DI_TEXT,      {{5,   4,   }, {0,      4     }}, DIF_NONE, msg(lng::MFileAssocDescr), },
		{ DI_EDIT,      {{5,   5,   }, {DlgX-6, 5     }}, DIF_NONE, },
		{ DI_TEXT,      {{-1,  6,   }, {0,      6     }}, DIF_SEPARATOR, },
		{ DI_CHECKBOX,  {{5,   7,   }, {0,      7     }}, DIF_NONE, msg(lng::MFileAssocExec), },
		{ DI_EDIT,      {{9,   8,   }, {DlgX-6, 8     }}, DIF_EDITPATH | DIF_EDITPATHEXEC, },
		{ DI_CHECKBOX,  {{5,   9,   }, {0,      9     }}, DIF_NONE, msg(lng::MFileAssocAltExec), },
		{ DI_EDIT,      {{9,  10,   }, {DlgX-6, 10    }}, DIF_EDITPATH | DIF_EDITPATHEXEC, },
		{ DI_CHECKBOX,  {{5,  11,   }, {0,      11    }}, DIF_NONE, msg(lng::MFileAssocView), },
		{ DI_EDIT,      {{9,  12,   }, {DlgX-6, 12    }}, DIF_EDITPATH | DIF_EDITPATHEXEC, },
		{ DI_CHECKBOX,  {{5,  13,   }, {0,      13    }}, DIF_NONE, msg(lng::MFileAssocAltView), },
		{ DI_EDIT,      {{9,  14,   }, {DlgX-6, 14    }}, DIF_EDITPATH | DIF_EDITPATHEXEC, },
		{ DI_CHECKBOX,  {{5,  15,   }, {0,      15    }}, DIF_NONE, msg(lng::MFileAssocEdit), },
		{ DI_EDIT,      {{9,  16,   }, {DlgX-6, 16    }}, DIF_EDITPATH | DIF_EDITPATHEXEC, },
		{ DI_CHECKBOX,  {{5,  17,   }, {0,      17    }}, DIF_NONE, msg(lng::MFileAssocAltEdit), },
		{ DI_EDIT,      {{9,  18,   }, {DlgX-6, 18    }}, DIF_EDITPATH | DIF_EDITPATHEXEC, },
		{ DI_TEXT,      {{-1, DlgY-4}, {0,      DlgY-4}}, DIF_SEPARATOR, },
		{ DI_BUTTON,    {{0,  DlgY-3}, {0,      DlgY-3}}, DIF_CENTERGROUP | DIF_DEFAULTBUTTON, msg(lng::MOk), },
		{ DI_BUTTON,    {{0,  DlgY-3}, {0,      DlgY-3}}, DIF_CENTERGROUP, msg(lng::MCancel), },
	});

	EditDlg[ETR_EDIT_MASKS].strHistory = L"Masks"sv;

	EditDlg[ETR_CHECK_EXEC].Selected = BSTATE_CHECKED;
	EditDlg[ETR_CHECK_EXEC].Selected = BSTATE_CHECKED;
	EditDlg[ETR_CHECK_ALTEXEC].Selected = BSTATE_CHECKED;
	EditDlg[ETR_CHECK_VIEW].Selected = BSTATE_CHECKED;
	EditDlg[ETR_CHECK_ALTVIEW].Selected = BSTATE_CHECKED;
	EditDlg[ETR_CHECK_EDIT].Selected = BSTATE_CHECKED;
	EditDlg[ETR_CHECK_ALTEDIT].Selected = BSTATE_CHECKED;

	if (!NewRec)
	{
		ConfigProvider().AssocConfig()->GetMask(EditPos,EditDlg[ETR_EDIT_MASKS].strData);
		ConfigProvider().AssocConfig()->GetDescription(EditPos,EditDlg[ETR_EDIT_DESCR].strData);
		for (int i=FILETYPE_EXEC,Item=ETR_EDIT_EXEC; i<=FILETYPE_ALTEDIT; i++,Item+=2)
		{
			bool on=false;
			if (!ConfigProvider().AssocConfig()->GetCommand(EditPos,i,EditDlg[Item].strData,&on) || !on)
			{
				EditDlg[Item-1].Selected = BSTATE_UNCHECKED;
				EditDlg[Item].Flags |= DIF_DISABLE;
			}
			else
			{
				EditDlg[Item-1].Selected = BSTATE_CHECKED;
			}
		}
	}

	const auto Dlg = Dialog::create(EditDlg, EditTypeRecordDlgProc);
	Dlg->SetHelp(L"FileAssocModify"sv);
	Dlg->SetId(FileAssocModifyId);
	Dlg->SetPosition({ -1, -1, DlgX, DlgY });
	Dlg->Process();

	if (Dlg->GetExitCode()==ETR_BUTTON_OK)
	{
		auto& Cfg = *ConfigProvider().AssocConfig();

		if (NewRec)
		{
			EditPos = Cfg.AddType(EditPos, EditDlg[ETR_EDIT_MASKS].strData, EditDlg[ETR_EDIT_DESCR].strData);
		}
		else
		{
			Cfg.UpdateType(EditPos, EditDlg[ETR_EDIT_MASKS].strData, EditDlg[ETR_EDIT_DESCR].strData);
		}

		for (int i=FILETYPE_EXEC,Item=ETR_EDIT_EXEC; i<=FILETYPE_ALTEDIT; i++,Item+=2)
		{
			Cfg.SetCommand(EditPos, i, EditDlg[Item].strData, EditDlg[Item - 1].Selected == BSTATE_CHECKED);
		}

		return true;
	}

	return false;
}

static bool DeleteTypeRecord(unsigned long long DeletePos)
{
	string strMask;
	ConfigProvider().AssocConfig()->GetMask(DeletePos,strMask);
	inplace::quote_unconditional(strMask);

	if (Message(MSG_WARNING,
		msg(lng::MAssocTitle),
		{
			msg(lng::MAskDelAssoc),
			strMask
		},
		{ lng::MDelete, lng::MCancel }) == message_result::first_button)
	{
		ConfigProvider().AssocConfig()->DelType(DeletePos);
		return true;
	}

	return false;
}

void EditFileTypes()
{
	int MenuPos=0;
	const auto TypesMenu = VMenu2::create(msg(lng::MAssocTitle), {}, ScrY - 4);
	TypesMenu->SetHelp(L"FileAssoc"sv);
	TypesMenu->SetMenuFlags(VMENU_WRAPMODE);
	TypesMenu->SetBottomTitle(KeysToLocalizedText(KEY_INS, KEY_DEL, KEY_F4, KEY_CTRLUP, KEY_CTRLDOWN));
	TypesMenu->SetId(FileAssocMenuId);

	bool Changed = false;
	for (;;)
	{
		int NumLine = FillFileTypesMenu(TypesMenu.get(), MenuPos);
		const auto ExitCode = TypesMenu->Run([&](const Manager::Key& RawKey)
		{
			const auto Key=RawKey();
			MenuPos=TypesMenu->GetSelectPos();

			int KeyProcessed = 1;

			switch (Key)
			{
				case KEY_NUMDEL:
				case KEY_DEL:
					if (MenuPos<NumLine)
					{
						if (const auto IdPtr = TypesMenu->GetComplexUserDataPtr<unsigned long long>(MenuPos))
						{
							Changed = DeleteTypeRecord(*IdPtr);
						}
					}
					break;

				case KEY_NUMPAD0:
				case KEY_INS:
					if (MenuPos - 1 >= 0)
					{
						if (const auto IdPtr = TypesMenu->GetComplexUserDataPtr<unsigned long long>(MenuPos - 1))
						{
							Changed = EditTypeRecord(*IdPtr, true);
						}
					}
					else
					{
						Changed = EditTypeRecord(0, true);
					}
					break;

				case KEY_NUMENTER:
				case KEY_ENTER:
				case KEY_F4:
					if (MenuPos<NumLine)
					{
						if (const auto IdPtr = TypesMenu->GetComplexUserDataPtr<unsigned long long>(MenuPos))
						{
							Changed = EditTypeRecord(*IdPtr, false);
						}
					}
					break;

				case KEY_CTRLUP:
				case KEY_RCTRLUP:
				case KEY_CTRLDOWN:
				case KEY_RCTRLDOWN:
				{
					if (!(any_of(Key, KEY_CTRLUP, KEY_RCTRLUP) && !MenuPos) &&
						!(any_of(Key, KEY_CTRLDOWN, KEY_RCTRLDOWN) && MenuPos == static_cast<int>(TypesMenu->size() - 1)))
					{
						const auto NewMenuPos = MenuPos + (any_of(Key, KEY_CTRLUP, KEY_RCTRLUP) ? -1 : 1);
						if (const auto IdPtr = TypesMenu->GetComplexUserDataPtr<unsigned long long>(MenuPos))
						{
							if (const auto IdPtr2 = TypesMenu->GetComplexUserDataPtr<unsigned long long>(NewMenuPos))
							{
								if (ConfigProvider().AssocConfig()->SwapPositions(*IdPtr, *IdPtr2))
								{
									MenuPos = NewMenuPos;
									Changed = true;
								}
							}
						}
					}
				}
				break;

				default:
					KeyProcessed = 0;
			}
			if (Changed)
			{
				Changed = false;
				NumLine = FillFileTypesMenu(TypesMenu.get(), MenuPos);
			}
			return KeyProcessed;
		});

		if (ExitCode!=-1)
		{
			MenuPos=ExitCode;
			TypesMenu->Key(KEY_F4);
			TypesMenu->ClearDone();
			continue;
		}

		break;
	}
}
