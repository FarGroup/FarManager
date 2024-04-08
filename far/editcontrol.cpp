/*
editcontrol.cpp

Надстройка над Edit.
Одиночная строка ввода для диалогов и комстроки (не для редактора)

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
#include "editcontrol.hpp"

// Internal:
#include "config.hpp"
#include "keys.hpp"
#include "keyboard.hpp"
#include "lang.hpp"
#include "pathmix.hpp"
#include "history.hpp"
#include "vmenu.hpp"
#include "vmenu2.hpp"
#include "console.hpp"
#include "elevation.hpp"
#include "colormix.hpp"
#include "manager.hpp"
#include "interf.hpp"
#include "ctrlobj.hpp"
#include "strmix.hpp"
#include "string_sort.hpp"
#include "global.hpp"

// Platform:
#include "platform.env.hpp"
#include "platform.reg.hpp"
#include "platform.fs.hpp"

// Common:
#include "common/algorithm.hpp"
#include "common/enum_tokens.hpp"
#include "common/enum_substrings.hpp"
#include "common/view/zip.hpp"

// External:

//----------------------------------------------------------------------------

EditControl::EditControl(window_ptr Owner, SimpleScreenObject* Parent, parent_processkey_t&& ParentProcessKey, Callback const* aCallback, History* iHistory, VMenu* iList, DWORD iFlags):
	Edit(std::move(Owner)),
	pHistory(iHistory),
	pList(iList),
	m_ParentProcessKey(ParentProcessKey? std::move(ParentProcessKey) : [Parent](const Manager::Key& Key) {return Parent->ProcessKey(Key); }),
	MacroAreaAC(MACROAREA_DIALOGAUTOCOMPLETION),
	ECFlags(iFlags),
	ACState(ECFlags.Check(EC_ENABLEAUTOCOMPLETE))
{
	SetObjectColor();

	if (aCallback)
	{
		m_Callback=*aCallback;
	}
	else
	{
		m_Callback.Active=true;
		m_Callback.m_Callback=nullptr;
		m_Callback.m_Param=nullptr;
	}
}

void EditControl::Show()
{
	if (m_Where.width() > RealPosToVisual(m_Str.size()))
	{
		SetLeftPos(0);
	}
	if (GetOwner()->IsVisible())
	{
		Edit::Show();
	}
}

void EditControl::Changed(bool DelBlock)
{
	m_Flags.Clear(FEDITLINE_CLEARFLAG);

	if(m_Callback.Active && !m_CallbackSuppressionsCount)
	{
		if(m_Callback.m_Callback)
		{
			m_Callback.m_Callback(m_Callback.m_Param);
		}
		AutoComplete(false, DelBlock);
	}
}

void EditControl::SetMenuPos(VMenu2& menu)
{
	const int MaxHeight = std::min(Global->Opt->Dialogs.CBoxMaxHeight.Get(), static_cast<long long>(menu.size())) + 1;

	const auto NewX2 = std::max(std::min(ScrX - 2, static_cast<int>(m_Where.right)), m_Where.left + 20);

	if((ScrY - m_Where.top < MaxHeight && m_Where.top > ScrY/2) || MenuUp)
	{
		MenuUp = true;
		menu.SetPosition({ m_Where.left, std::max(0, m_Where.top - 1 - MaxHeight), NewX2, m_Where.top - 1 });
	}
	else
	{
		menu.SetPosition({ m_Where.left, m_Where.top + 1, NewX2, std::min(ScrY, m_Where.top + 1 + MaxHeight) });
	}
}

static void AddSeparatorOrSetTitle(VMenu2& Menu, lng TitleId)
{
	bool Separator = false;
	for (const auto i: std::views::iota(0uz, Menu.size()))
	{
		if (Menu.at(i).Flags & LIF_SEPARATOR)
		{
			Separator = true;
			break;
		}
	}
	if (!Separator)
	{
		if (Menu.size())
		{
			Menu.AddItem(MenuItemEx(msg(TitleId), LIF_SEPARATOR));
		}
		else
		{
			Menu.SetTitle(msg(TitleId));
		}
	}
}

static bool ParseStringWithQuotes(string_view const Str, string& Start, string& Token, bool& StartQuote)
{
	size_t Pos;
	if (std::ranges::count(Str, L'"') & 1) // odd quotes count
	{
		Pos = Str.rfind(L'"');
	}
	else
	{
		auto WordDiv = GetBlanks() + Global->Opt->strWordDiv.Get();
		static const auto NoQuote = L"\":\\/%.?-"sv;
		std::erase_if(WordDiv, [&](wchar_t i){ return contains(NoQuote, i); });

		for (Pos = Str.size() - 1; Pos != static_cast<size_t>(-1); Pos--)
		{
			if (Str[Pos] == L'"')
			{
				Pos--;
				while (Str[Pos] != L'"' && Pos != static_cast<size_t>(-1))
				{
					Pos--;
				}
			}
			else if (contains(WordDiv, Str[Pos]))
			{
				Pos++;
				break;
			}
		}
	}
	if (Pos == static_cast<size_t>(-1))
	{
		Pos = 0;
	}

	if (Pos < Str.size() && Str[Pos] == L'"')
	{
		Pos++;
		StartQuote = true;
	}
	else
	{
		StartQuote = false;
	}

	Start.assign(Str, 0, Pos);
	Token = unquote(Str.substr(Pos));
	return !Token.empty();
}

struct cmp_user_data
{
	string OriginalCaseStr;
	unsigned long long HistoryRecordId;
};

static bool EnumWithQuoutes(VMenu2& Menu, const string_view strStart, const string_view Token, const bool StartQuote, const lng Title, const std::set<string, string_sort::less_t>& ResultStrings)
{
	if (ResultStrings.empty())
		return false;

	AddSeparatorOrSetTitle(Menu, Title);

	for (const auto& i: ResultStrings)
	{
		const auto BuildQuotedString = [&](string Str)
		{
			if (!StartQuote)
				inplace::QuoteSpace(Str);

			auto Result = concat(strStart, Str);
			if (StartQuote)
				Result += L'"';

			return Result;
		};

		MenuItemEx Item(BuildQuotedString(i));

		// Preserve the case of the already entered part
		if (Global->Opt->AutoComplete.AppendCompletion)
		{
			if (starts_with_icase(i, Token))
			{
				Item.ComplexUserData = cmp_user_data{ BuildQuotedString(Token + i.substr(Token.size())) };
			}
		}

		Menu.AddItem(Item);
	}

	return true;
}

static bool EnumFiles(VMenu2& Menu, const string_view strStart, const string_view Token, const bool StartQuote)
{
	SCOPED_ACTION(elevation::suppress);

	std::set<string, string_sort::less_t> ResultStrings;

	const auto FileName = PointToName(Token);

	for (const auto& i: os::fs::enum_files(os::env::expand(Token) + L'*'))
	{
		const auto NameMatch = starts_with_icase(i.FileName, FileName);
		const auto AltNameMatch = !NameMatch && i.HasAlternateFileName() && starts_with_icase(i.AlternateFileName(), FileName);
		if (NameMatch || AltNameMatch)
		{
			ResultStrings.emplace(Token.substr(0, Token.size() - FileName.size()) + (NameMatch? i.FileName : i.AlternateFileName()));
		}
	}

	if (HasPathPrefix(Token) && Token.substr(L"\\\\?\\"sv.size()).find_first_of(path::separators) == Token.npos)
	{
		for (const auto& Namespace: { L"\\??"sv, L"\\GLOBAL??"sv })
		{
			for (const auto& i: os::fs::enum_devices(Namespace))
			{
				if (starts_with_icase(i, FileName))
					ResultStrings.emplace(Token.substr(0, Token.size() - FileName.size()) + i);
			}
		}
	}

	return EnumWithQuoutes(Menu, strStart, Token, StartQuote, lng::MCompletionFilesTitle, ResultStrings);
}

static bool EnumModules(VMenu2& Menu, const string_view strStart, const string_view Token, const bool StartQuote)
{
	SCOPED_ACTION(elevation::suppress);

	std::set<string, string_sort::less_t> ResultStrings;

	if (const auto Range = std::ranges::equal_range(Global->Opt->Exec.ExcludeCmds, Token, string_sort::less_icase, std::views::take(Token.size())); !Range.empty())
	{
		// TODO: insert_range
		ResultStrings.insert(ALL_CONST_RANGE(Range));
	}

	if (const auto strPathEnv = os::env::get(L"PATH"sv); !strPathEnv.empty())
	{
		const auto PathExtList = enum_tokens(os::env::get_pathext(), L";"sv);
		const auto Pattern = Token + L"*"sv;

		for (const auto& Path: enum_tokens_with_quotes(strPathEnv, L";"sv))
		{
			if (Path.empty())
				continue;

			for (const auto& FindData: os::fs::enum_files(path::join(Path, Pattern)))
			{
				if (!os::fs::is_file(FindData))
					continue;

				const auto FindExt = name_ext(FindData.FileName).second;

				for (const auto& Ext: PathExtList)
				{
					if (starts_with_icase(Ext, FindExt))
					{
						ResultStrings.emplace(FindData.FileName);
					}
				}
			}
		}
	}

	const auto enum_app_paths = [&](os::reg::key const& RootKey, DWORD const Flags = 0)
	{
		const auto SamDesired = KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE | Flags;

		const auto AppPathsKey = RootKey.open(L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\"sv, SamDesired);
		if (!AppPathsKey)
			return;

		for (const auto& SubkeyName: AppPathsKey.enum_keys())
		{
			if (const auto SubKey = AppPathsKey.open(SubkeyName, SamDesired); SubKey.exits({}) && starts_with_icase(SubkeyName, Token))
			{
				ResultStrings.emplace(SubkeyName);
			}
		}
	};

	enum_app_paths(os::reg::key::current_user);
	enum_app_paths(os::reg::key::local_machine);

	if (const auto RedirectionFlag = os::GetAppPathsRedirectionFlag())
		enum_app_paths(os::reg::key::local_machine, RedirectionFlag);

	return EnumWithQuoutes(Menu, strStart, Token, StartQuote, lng::MCompletionFilesTitle, ResultStrings);
}

static bool EnumEnvironment(VMenu2& Menu, const string_view strStart, const string_view Token, const bool StartQuote)
{
	SCOPED_ACTION(elevation::suppress);

	std::set<string, string_sort::less_t> ResultStrings;

	const os::env::provider::strings EnvStrings;
	for (const auto& i: enum_substrings(EnvStrings.data()))
	{
		const auto Name = split(i).first;
		if (Name.empty()) // =C: etc.
			continue;

		const auto VarName = concat(L'%', Name, L'%');
		if (starts_with_icase(VarName, Token))
		{
			ResultStrings.emplace(VarName);
		}
	}

	return EnumWithQuoutes(Menu, strStart, Token, StartQuote, lng::MCompletionEnvironmentTitle, ResultStrings);
}

static bool is_input_queue_empty()
{
	size_t EventsCount = 0;
	if (!console.GetNumberOfInputEvents(EventsCount))
		return true; // Let's hope for the best

	if (!EventsCount)
		return true; // Grand!

	INPUT_RECORD Record;
	if (EventsCount == 1 && console.PeekOneInput(Record) && Record.EventType == KEY_EVENT && !Record.Event.KeyEvent.bKeyDown)
	{
		// The corresponding Up event. It should not happen under normal circumstances.
		// If it happens - either the user is Flash / Sonic or the host is Windows Terminal.
		// https://github.com/microsoft/terminal/issues/3910
		// https://github.com/FarGroup/FarManager/issues/262
		return true;
	}

	// Emulated input
	return false;
}

int EditControl::AutoCompleteProc(bool Manual,bool DelBlock,Manager::Key& BackKey, FARMACROAREA Area)
{
	int Result=0;
	static int Reenter=0;
	if(
		ECFlags.Check(EC_ENABLEAUTOCOMPLETE) &&
		!m_Str.empty() &&
		!Reenter &&
		is_input_queue_empty() &&
		(
			Manual ||
			Global->CtrlObject->Macro.GetState() == MACROSTATE_NOMACRO ||
			(Area == MACROAREA_DIALOGAUTOCOMPLETION && KeyMacro::IsMacroDialog(GetOwner()))
		)
	)
	{
		Reenter++;
		const auto ComplMenu = VMenu2::create({}, {});
		m_ComplMenu = ComplMenu;

		ComplMenu->SetDialogMode(DMODE_NODRAWSHADOW);
		ComplMenu->SetModeMoving(false);
		string CurrentInput = m_Str;

		ComplMenu->SetMacroMode(Area);

		const auto CompletionEnabled = [&Manual](int State)
		{
			return (Manual && State) || (!Manual && State == 1);
		};

		const auto Complete = [&](VMenu2& Menu, const string_view Str)
		{
			if (Str.empty())
				return;

			// These two guys use the whole string, not the extracted token:
			if (pHistory && ECFlags.Check(EC_COMPLETE_HISTORY) && CompletionEnabled(Global->Opt->AutoComplete.UseHistory))
			{
				bool AnyAdded = false;
				pHistory->GetAllSimilar(Str, [&](string_view const Name, unsigned long long const Id, bool const IsLocked)
				{
					MenuItemEx Item;
					// Preserve the case of the already entered part
					Item.ComplexUserData = cmp_user_data{ Global->Opt->AutoComplete.AppendCompletion? Str + Name.substr(Str.size()) : L""s, Id };
					Item.Name = Name;
					Item.Flags |= IsLocked? LIF_CHECKED : LIF_NONE;
					ComplMenu->AddItem(std::move(Item));

					AnyAdded = true;
				});

				if (AnyAdded)
					ComplMenu->SetTitle(msg(lng::MCompletionHistoryTitle));
			}
			else if (pList)
			{
				for (const auto i: std::views::iota(0uz, pList->size()))
				{
					string_view const Text = pList->at(i).Name;

					if (!starts_with_icase(Text, Str))
						continue;

					MenuItemEx Item(Text);
					// Preserve the case of the already entered part
					if (Global->Opt->AutoComplete.AppendCompletion)
					{
						Item.ComplexUserData = cmp_user_data{ Str + Text.substr(Str.size()) };
					}
					ComplMenu->AddItem(Item);
				}
			}

			string Prefix;
			string Token;
			bool StartQuote;
			if (!ParseStringWithQuotes(Str, Prefix, Token, StartQuote))
				return;

			if(ECFlags.Check(EC_COMPLETE_FILESYSTEM) && CompletionEnabled(Global->Opt->AutoComplete.UseFilesystem))
			{
				EnumFiles(Menu, Prefix, Token, StartQuote);
			}
			if(ECFlags.Check(EC_COMPLETE_ENVIRONMENT) && CompletionEnabled(Global->Opt->AutoComplete.UseEnvironment))
			{
				EnumEnvironment(Menu, Prefix, Token, StartQuote);
			}
			if(ECFlags.Check(EC_COMPLETE_PATH) && CompletionEnabled(Global->Opt->AutoComplete.UsePath))
			{
				if (FindSlash(Str) == string::npos)
					EnumModules(Menu, Prefix, Token, StartQuote);
			}
		};

		Complete(*ComplMenu, CurrentInput);

		const auto AppendCmd = [&]
		{
			int SelStart = GetLength();

			const auto& FirstItem = ComplMenu->at(0).Name;
			const auto Data = ComplMenu->GetComplexUserDataPtr<cmp_user_data>(0);

			// magic
			if (SelStart > 1 && path::is_separator(m_Str[SelStart - 1]) && m_Str[SelStart - 2] == L'"' && path::is_separator(FirstItem[SelStart - 2]))
			{
				m_Str.erase(SelStart - 2, 1);
				SelStart--;
				m_CurPos--;
			}

			{
				SCOPED_ACTION(auto)(CallbackSuppressor());
				if (!equal_icase(FirstItem, m_Str + FirstItem.substr(SelStart)))
				{
					// New string contains opening quote, but not the original one
					if (SelStart <= m_CurPos) // e. g. entering "\" in "C:\abc_" - where "_" is a cursor position
						++SelStart;
					else
						--SelStart;
				}

				SetString((Data && !Data->OriginalCaseStr.empty())? Data->OriginalCaseStr : FirstItem);

				if (m_Where.width() - 1 > GetLength())
					SetLeftPos(0);
				Select(SelStart, GetLength());
			}
		};

		if(ComplMenu->size() > 1 || (ComplMenu->size() == 1 && !equal_icase(CurrentInput, ComplMenu->at(0).Name)))
		{
			ComplMenu->SetMenuFlags(VMENU_WRAPMODE | VMENU_SHOWAMPERSAND);
			if(!DelBlock && Global->Opt->AutoComplete.AppendCompletion && (!m_Flags.Check(FEDITLINE_PERSISTENTBLOCKS) || Global->Opt->AutoComplete.ShowList))
			{
				AppendCmd();
				Show();
			}
			if(Global->Opt->AutoComplete.ShowList)
			{
				ComplMenu->AddItem(MenuItemEx(), 0);
				SetMenuPos(*ComplMenu);
				ComplMenu->SetSelectPos(0,0);
				ComplMenu->SetBoxType(SHORT_SINGLE_BOX);
				Show();
				int PrevPos=0;

				bool Visible;
				size_t Size;
				::GetCursorType(Visible, Size);
				bool IsChanged = false;
				const auto ExitCode = ComplMenu->RunEx([&](int Msg, void* Param)
				{
					if (Msg != DN_INPUT)
					{
						if (Global->Opt->AutoComplete.ModalList)
							return 0;

						if (!IsChanged && (Msg == DN_DRAWDIALOGDONE || Msg == DN_DRAWDLGITEMDONE))
						{
							::SetCursorType(Visible, Size);
							return 0;
						}

						const auto CurPos = Msg == DN_LISTCHANGE? static_cast<int>(std::bit_cast<intptr_t>(Param)) : ComplMenu->GetSelectPos();
						if(CurPos>=0 && (PrevPos!=CurPos || IsChanged))
						{
							PrevPos=CurPos;
							IsChanged = false;
							SetString(CurPos? ComplMenu->at(CurPos).Name : CurrentInput);
							Show();
						}

						return 0;
					}

					const auto& ReadRec = *static_cast<INPUT_RECORD const*>(Param);

					auto MenuKey = [&]() -> unsigned
					{
						// ugh
						if (ReadRec.EventType == MOUSE_EVENT)
						{
							auto Position = ComplMenu->GetPosition();

							++Position.left;
							++Position.top;
							--Position.right;
							--Position.bottom;

							if (!Position.contains(ReadRec.Event.MouseEvent.dwMousePosition))
								return KEY_NONE;
						}

						return InputRecordToKey(&ReadRec);
					}();

					::SetCursorType(Visible, Size);

						if (MenuKey == KEY_NONE)
							return 0;

						// ввод
						if(in_closed_range(L' ', MenuKey, std::numeric_limits<wchar_t>::max()) || any_of(MenuKey, KEY_BS, KEY_DEL, KEY_NUMDEL))
						{
							DeleteBlock();
							const auto strPrev = GetString();
							ProcessKey(Manager::Key(MenuKey));
							CurrentInput = GetString();
							if(strPrev != CurrentInput)
							{
								SCOPED_ACTION(Dialog::suppress_redraw)(ComplMenu.get());

								ComplMenu->clear();
								PrevPos=0;

								Complete(*ComplMenu, CurrentInput);

								if (ComplMenu->size() > 1 || (ComplMenu->size() == 1 && !equal_icase(CurrentInput, ComplMenu->at(0).Name)))
								{
									if(none_of(MenuKey, KEY_BS, KEY_DEL, KEY_NUMDEL) && Global->Opt->AutoComplete.AppendCompletion)
									{
										AppendCmd();
									}
									ComplMenu->AddItem(MenuItemEx(), 0);
									SetMenuPos(*ComplMenu);
									ComplMenu->SetSelectPos(0,0);
								}
								else
								{
									ComplMenu->Close(-1);
								}
							}
							Show();
							return 1;
						}
						else
						{
							switch(MenuKey)
							{
							// "классический" перебор
							case KEY_CTRLEND:
							case KEY_RCTRLEND:

							case KEY_CTRLSPACE:
							case KEY_RCTRLSPACE:
								{
									ComplMenu->Key(KEY_DOWN);
									return 1;
								}

							case KEY_SHIFTDEL:
							case KEY_SHIFTNUMDEL:
								{
									if(ComplMenu->size() > 1)
									{
										const auto Data = ComplMenu->GetComplexUserDataPtr<cmp_user_data>();
										if(Data && Data->HistoryRecordId && pHistory->DeleteIfUnlocked(Data->HistoryRecordId))
										{
											ComplMenu->DeleteItem(ComplMenu->GetSelectPos());
											if(ComplMenu->size() > 1)
											{
												IsChanged = true;
												SetMenuPos(*ComplMenu);
												Show();
											}
											else
											{
												ComplMenu->Close(-1);
											}
										}
									}
								}
								break;

							// навигация по строке ввода
							case KEY_LEFT:
							case KEY_NUMPAD4:
							case KEY_CTRLS:     case KEY_RCTRLS:
							case KEY_RIGHT:
							case KEY_NUMPAD6:
							case KEY_CTRLD:     case KEY_RCTRLD:
							case KEY_CTRLLEFT:  case KEY_RCTRLLEFT:
							case KEY_CTRLRIGHT: case KEY_RCTRLRIGHT:
							case KEY_CTRLHOME:  case KEY_RCTRLHOME:
								{
									if(any_of(MenuKey, KEY_LEFT, KEY_NUMPAD4))
									{
										MenuKey = KEY_CTRLS;
									}
									else if(any_of(MenuKey, KEY_RIGHT, KEY_NUMPAD6))
									{
										MenuKey = KEY_CTRLD;
									}
									m_ParentProcessKey(Manager::Key(MenuKey));
									Show();
									ComplMenu->Show();
									return 1;
								}

							// навигация по списку
							case KEY_SHIFT:
							case KEY_ALT:
							case KEY_RALT:
							case KEY_CTRL:
							case KEY_RCTRL:
							case KEY_HOME:
							case KEY_NUMPAD7:
							case KEY_END:
							case KEY_NUMPAD1:
							case KEY_NONE:
							case KEY_ESC:
							case KEY_F10:
							case KEY_ALTF9:
							case KEY_RALTF9:
							case KEY_UP:
							case KEY_NUMPAD8:
							case KEY_DOWN:
							case KEY_NUMPAD2:
							case KEY_PGUP:
							case KEY_NUMPAD9:
							case KEY_PGDN:
							case KEY_NUMPAD3:
							case KEY_ALTLEFT:
							case KEY_ALTRIGHT:
							case KEY_ALTHOME:
							case KEY_ALTEND:
							case KEY_RALTLEFT:
							case KEY_RALTRIGHT:
							case KEY_RALTHOME:
							case KEY_RALTEND:
							case KEY_MSWHEEL_UP:
							case KEY_MSWHEEL_DOWN:
							case KEY_MSWHEEL_LEFT:
							case KEY_MSWHEEL_RIGHT:
								{
									break;
								}

							case KEY_MSLCLICK:
								MenuKey = KEY_ENTER;
								[[fallthrough]];
							case KEY_ENTER:
							case KEY_NUMENTER:
								{
									if (!Global->Opt->AutoComplete.ModalList)
									{
										ComplMenu->Close(-1);
										BackKey = Manager::Key(MenuKey);
										Result = 1;
									}
									break;
								}

							// всё остальное закрывает список и идёт владельцу
							default:
								{
									ComplMenu->Close(-1);
									BackKey= Manager::Key(MenuKey, ReadRec);
									Result=1;
								}
							}
						}

					return 0;
				});
				// mouse click
				if(ExitCode>0)
				{
					if(Global->Opt->AutoComplete.ModalList)
					{
						SetString(ComplMenu->at(ExitCode).Name);
						Show();
					}
					else
					{
						BackKey = Manager::Key(KEY_ENTER);
						Result=1;
					}
				}
			}
		}

		Reenter--;
	}
	return Result;
}

void EditControl::ResizeConsole()
{
	if (const auto ComplMenu = m_ComplMenu.lock())
		SetMenuPos(*ComplMenu);
}

void EditControl::AutoComplete(bool Manual,bool DelBlock)
{
	Manager::Key Key;
	if(AutoCompleteProc(Manual,DelBlock,Key,MacroAreaAC))
	{
		const FAR_INPUT_RECORD irec{ static_cast<DWORD>(Key()), Key.Event() };
		if(!Global->CtrlObject->Macro.ProcessEvent(&irec))
			m_ParentProcessKey(Key);
		const auto CurWindowType = Global->WindowManager->GetCurrentWindow()->GetType();
		if (CurWindowType == windowtype_dialog || CurWindowType == windowtype_panels)
		{
			Show();
		}
	}
}

bool EditControl::ProcessKey(const Manager::Key& Key)
{
	const unsigned int NonClearKeys[]=
	{
		KEY_CTRLC,
		KEY_RCTRLC,
		KEY_CTRLINS,
		KEY_CTRLNUMPAD0,
		KEY_RCTRLINS,
		KEY_RCTRLNUMPAD0
	};
	const auto Result = Edit::ProcessKey(Key);
	if (Result && m_Flags.Check(FEDITLINE_CLEARFLAG) && !contains(NonClearKeys, Key()))
	{
		m_Flags.Clear(FEDITLINE_CLEARFLAG);
		Show();
	}
	return Result;
}

bool EditControl::ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent)
{
	if (!Edit::ProcessMouse(MouseEvent))
		return false;

	const auto Scrolling = []
	{
		return IsMouseButtonPressed() == FROM_LEFT_1ST_BUTTON_PRESSED;
	};

	const auto ToLeft = [&]
	{
		return IntKeyState.MousePos.x < m_Where.left;
	};

	const auto ToRight = [&]
	{
		return IntKeyState.MousePos.x > m_Where.right;
	};

	keyboard_repeat_emulation const Emulation;

	while(Scrolling())
	{
		m_Flags.Clear(FEDITLINE_CLEARFLAG);

		auto NewPos = GetTabCurPos();
		const auto CurLeftPos = GetLeftPos();

		if (ToLeft())
		{
			if (NewPos && --NewPos < CurLeftPos)
			{
				if (CurLeftPos)
					SetLeftPos(CurLeftPos - 1);

				while (!Emulation.signaled() && Scrolling() && ToLeft())
					std::this_thread::yield();
			}
		}
		else if (ToRight())
		{
			if (++NewPos >= CurLeftPos + m_Where.width())
			{
				SetLeftPos(CurLeftPos + 1);

				while (!Emulation.signaled() && Scrolling() && ToRight())
					std::this_thread::yield();
			}
		}
		else
		{
			NewPos = CurLeftPos + IntKeyState.MousePos.x - m_Where.left;
			Emulation.reset();
		}

		SetTabCurPos(NewPos);

		if (!(IntKeyState.MouseEventFlags & MOUSE_MOVED))
			continue;

		if(!Selection)
		{
			Selection=true;
			SelectionStart=-1;
			Select(SelectionStart,0);
		}
		else
		{
			if(SelectionStart==-1)
			{
				SelectionStart=m_CurPos;
			}
			Select(std::min(SelectionStart, m_CurPos), std::min(m_Str.size(), std::max(SelectionStart, m_CurPos)));
			Show();
		}
	}

	Selection = false;
	return true;
}

void EditControl::SetObjectColor(PaletteColors Color,PaletteColors SelColor,PaletteColors ColorUnChanged)
{
	m_Color=colors::PaletteColorToFarColor(Color);
	m_SelectedColor=colors::PaletteColorToFarColor(SelColor);
	m_UnchangedColor=colors::PaletteColorToFarColor(ColorUnChanged);
}

void EditControl::SetObjectColor(const FarColor& Color,const FarColor& SelColor, const FarColor& ColorUnChanged)
{
	m_Color=Color;
	m_SelectedColor=SelColor;
	m_UnchangedColor=ColorUnChanged;
}

void EditControl::GetObjectColor(FarColor& Color, FarColor& SelColor, FarColor& ColorUnChanged) const
{
	Color = m_Color;
	SelColor = m_SelectedColor;
	ColorUnChanged = m_UnchangedColor;
}

const FarColor& EditControl::GetNormalColor() const
{
	return m_Color;
}

const FarColor& EditControl::GetSelectedColor() const
{
	return m_SelectedColor;
}

const FarColor& EditControl::GetUnchangedColor() const
{
	return m_UnchangedColor;
}

size_t EditControl::GetTabSize() const
{
	return Global->Opt->EdOpt.TabSize;
}

EXPAND_TABS EditControl::GetTabExpandMode() const
{
	return EXPAND_NOTABS;
}

void EditControl::SetInputMask(string_view const InputMask)
{
	m_Mask = InputMask;
	if (!m_Mask.empty())
	{
		RefreshStrByMask(TRUE);
	}
}

// Функция обновления состояния строки ввода по содержимому Mask
void EditControl::RefreshStrByMask(int InitMode)
{
	const auto Mask = GetInputMask();
	if (Mask.empty())
		return;

	m_Str.resize(Mask.size(), L' ');
	MaxLength = m_Str.size();

	for (const auto& [Str, Msk]: zip(m_Str, Mask))
	{
		if (InitMode)
			Str = MaskDefaultChar(Msk);

		if (!CheckCharMask(Msk))
			Str = Msk;
	}
}

const string& EditControl::WordDiv() const
{
	return Global->Opt->strWordDiv;
}
