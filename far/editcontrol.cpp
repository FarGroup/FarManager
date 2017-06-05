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

#include "headers.hpp"
#pragma hdrstop

#include "editcontrol.hpp"
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
#include "string_utils.hpp"

EditControl::EditControl(window_ptr Owner, SimpleScreenObject* Parent, parent_processkey_t&& ParentProcessKey, Callback* aCallback, History* iHistory, FarList* iList, DWORD iFlags):
	Edit(Owner),
	pHistory(iHistory),
	pList(iList),
	m_ParentProcessKey(ParentProcessKey? std::move(ParentProcessKey) : [Parent](const Manager::Key& Key) {return Parent->ProcessKey(Key); }),
	MaxLength(-1),
	CursorSize(-1),
	CursorPos(0),
	PrevCurPos(0),
	MacroSelectionStart(-1),
	SelectionStart(-1),
	MacroAreaAC(MACROAREA_DIALOGAUTOCOMPLETION),
	ECFlags(iFlags),
	m_CallbackSuppressionsCount(),
	Selection(false),
	MenuUp(false),
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
	if (m_X2 - m_X1 + 1 > m_Str.size())
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
	int MaxHeight = std::min(Global->Opt->Dialogs.CBoxMaxHeight.Get(), static_cast<long long>(menu.size())) + 1;

	const auto NewX2 = std::max(std::min(ScrX - 2, static_cast<int>(m_X2)), m_X1 + 20);

	if((ScrY-m_Y1<MaxHeight && m_Y1>ScrY/2) || MenuUp)
	{
		MenuUp = true;
		menu.SetPosition(m_X1, std::max(0, m_Y1-1-MaxHeight), NewX2, m_Y1-1);
	}
	else
	{
		menu.SetPosition(m_X1, m_Y1+1, NewX2, std::min(static_cast<int>(ScrY), m_Y1+1+MaxHeight));
	}
}

static void AddSeparatorOrSetTitle(VMenu2& Menu, lng TitleId)
{
	bool Separator = false;
	for (size_t i = 0; i != Menu.size(); ++i)
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
			MenuItemEx Item(msg(TitleId));
			Item.Flags = LIF_SEPARATOR;
			Menu.AddItem(Item);
		}
		else
		{
			Menu.SetTitle(msg(TitleId));
		}
	}
}

using enumerator_type = void(VMenu2&, const string&, const std::function<void(const string&)>&);

static bool ParseStringWithQuotes(const string& Str, string& Start, string& Token, bool& StartQuote)
{
	size_t Pos;
	if (std::count(ALL_CONST_RANGE(Str), L'"') & 1) // odd quotes count
	{
		Pos = Str.rfind(L'"');
	}
	else
	{
		auto WordDiv = GetSpaces() + Global->Opt->strWordDiv.Get();
		static const string NoQuote = L"\":\\/%.-";
		for (size_t i = 0; i != WordDiv.size(); ++i)
		{
			if (contains(NoQuote, WordDiv[i]))
			{
				WordDiv.erase(i--, 1);
			}
		}

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

	Start = Str.substr(0, Pos);
	Token = Str.substr(Pos);
	Unquote(Token);
	return !Token.empty();
}

struct cmp_user_data
{
	string OriginalCaseStr;
	unsigned long long HistoryRecordId;
};

static bool EnumWithQuoutes(VMenu2& Menu, const string& strStart, const string& Token, bool StartQuote, lng Title, enumerator_type Enumerator)
{
	std::set<string, string_i_less> ResultStrings;

	Enumerator(Menu, Token, [&](const string& strAdd)
	{
		ResultStrings.emplace(strAdd);
	});

	if (ResultStrings.empty())
		return false;

	AddSeparatorOrSetTitle(Menu, Title);

	for (const auto& i: ResultStrings)
	{
		const auto& BuildQuotedString = [&](string Str)
		{
			if (!StartQuote)
				QuoteSpace(Str);

			auto Result = strStart + Str;
			if (StartQuote)
				Result += L'"';

			return Result;
		};

		MenuItemEx Item;
		Item.strName = BuildQuotedString(i);

		// Preserve the case of the already entered part
		if (Global->Opt->AutoComplete.AppendCompletion)
		{
			if (starts_with_icase(i, Token))
			{
				Item.UserData = cmp_user_data{ BuildQuotedString(Token + i.substr(Token.size())) };
			}
		}

		Menu.AddItem(Item);
	}

	return true;
}

static bool EnumFiles(VMenu2& Menu, const string& strStart, const string& Token, bool StartQuote)
{
	SCOPED_ACTION(elevation::suppress);

	return EnumWithQuoutes(Menu, strStart, Token, StartQuote, lng::MCompletionFilesTitle, [](VMenu2& Menu, const string& Token, const std::function<void(const string&)>& Inserter)
	{
		const auto Pattern = os::env::expand_strings(Token) + L'*';
		const auto FileName = PointToName(Token);

		for (const auto& i: os::fs::enum_files(Pattern))
		{
			const auto NameMatch = starts_with_icase(i.strFileName, FileName);
			const auto AltNameMatch = !NameMatch && starts_with_icase(i.strAlternateFileName, FileName);
			if (NameMatch || AltNameMatch)
			{
				Inserter(Token.substr(0, FileName - Token.data()) + (NameMatch? i.strFileName : i.strAlternateFileName));
			}
		}
	});
}

static bool EnumModules(VMenu2& Menu, const string& strStart, const string& Token, bool StartQuote)
{
	SCOPED_ACTION(elevation::suppress);

	return EnumWithQuoutes(Menu, strStart, Token, StartQuote, lng::MCompletionFilesTitle, [](VMenu2& Menu, const string& Token, const std::function<void(const string&)>& Inserter)
	{
		for (const auto& i: split<std::vector<string>>(os::env::expand_strings(Global->Opt->Exec.strExcludeCmds)))
		{
			if (starts_with_icase(i, Token))
			{
				Inserter(i);
			}
		}

		{
			const auto strPathEnv(os::env::get_variable(L"PATH"));
			if (!strPathEnv.empty())
			{
				const auto PathExt = os::env::get_pathext();
				const auto PathExtList = enum_tokens(PathExt, L";");

				string str;
				for (const auto& Path: split<std::vector<string>>(strPathEnv))
				{
					str = Path;
					AddEndSlash(str);
					append(str, Token, L'*');
					for (const auto& FindData: os::fs::enum_files(str))
					{
						const auto FindExt = PointToExt(FindData.strFileName);
						for (const auto& Ext: PathExtList)
						{
							if (starts_with_icase(Ext, FindExt))
							{
								Inserter(FindData.strFileName);
							}
						}
					}
				}
			}
		}

		static const wchar_t RegPath[] = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\";
		static const HKEY RootFindKey[]={HKEY_CURRENT_USER,HKEY_LOCAL_MACHINE,HKEY_LOCAL_MACHINE};

		DWORD samDesired = KEY_ENUMERATE_SUB_KEYS|KEY_QUERY_VALUE;

		for (size_t i=0; i<std::size(RootFindKey); i++)
		{
			if (i==std::size(RootFindKey)-1)
			{
				if (const auto RedirectionFlag = os::GetAppPathsRedirectionFlag())
				{
					samDesired|=RedirectionFlag;
				}
				else
				{
					break;
				}
			}

			if (const auto Key = os::reg::open_key(RootFindKey[i], RegPath, samDesired))
			{
				for (const auto& SubkeyName: os::reg::enum_key(Key))
				{
					if (const auto SubKey = os::reg::open_key(Key.get(), SubkeyName.data(), samDesired))
					{
						if(os::reg::GetValue(SubKey, L""))
						{
							if (starts_with_icase(SubkeyName, Token))
							{
								Inserter(SubkeyName);
							}
						}
					}
				}
			}
		}
	});
}

static bool EnumEnvironment(VMenu2& Menu, const string& strStart, const string& Token, bool StartQuote)
{
	SCOPED_ACTION(elevation::suppress);

	return EnumWithQuoutes(Menu, strStart, Token, StartQuote, lng::MCompletionEnvironmentTitle, [](VMenu2& Menu, const string& Token, const std::function<void(const string&)>& Inserter)
	{
		const os::env::provider::strings EnvStrings;
		for (const auto& i : enum_substrings(EnvStrings.data()))
		{
			auto Name = split_name_value(i).first;
			if (Name.empty()) // =C: etc.
				continue;

			const auto VarName = concat(L'%', Name, L'%');
			if (starts_with_icase(VarName, Token))
			{
				Inserter(VarName);
			}
		}
	});
}

int EditControl::AutoCompleteProc(bool Manual,bool DelBlock,Manager::Key& BackKey, FARMACROAREA Area)
{
	int Result=0;
	static int Reenter=0;
	size_t EventsCount = 0;
	Console().GetNumberOfInputEvents(EventsCount);
	if(ECFlags.Check(EC_ENABLEAUTOCOMPLETE) && !m_Str.empty() && !Reenter && !EventsCount && (Global->CtrlObject->Macro.GetState() == MACROSTATE_NOMACRO || Manual))
	{
		Reenter++;
		const auto ComplMenu = VMenu2::create({}, nullptr, 0, 0);
		ComplMenu->SetDialogMode(DMODE_NODRAWSHADOW);
		ComplMenu->SetModeMoving(false);
		string CurrentInput = m_Str;

		ComplMenu->SetMacroMode(Area);

		const auto& CompletionEnabled = [&Manual](int State)
		{
			return (Manual && State) || (!Manual && State == 1);
		};

		const auto& Complete = [&](VMenu2& Menu, const string& Str)
		{
			if (Str.empty())
				return;

			// These two guys use the whole string, not the extracted token:
			if (pHistory && ECFlags.Check(EC_COMPLETE_HISTORY) && CompletionEnabled(Global->Opt->AutoComplete.UseHistory))
			{
				auto Items = pHistory->GetAllSimilar(Str);
				if (!Items.empty())
				{
					for (auto& i : Items)
					{
						MenuItemEx Item;
						// Preserve the case of the already entered part
						Item.UserData = cmp_user_data{ Global->Opt->AutoComplete.AppendCompletion? Str + std::get<0>(i).substr(Str.size()) : L""s, std::get<1>(i) };
						Item.strName = std::move(std::get<0>(i));
						Item.Flags |= std::get<2>(i)? LIF_CHECKED : LIF_NONE;
						ComplMenu->AddItem(std::move(Item));
					}
					ComplMenu->SetTitle(msg(lng::MCompletionHistoryTitle));
				}
			}
			else if (pList)
			{
				for (size_t i = 0; i < pList->ItemsNumber; i++)
				{
					if (starts_with_icase(pList->Items[i].Text, Str) && pList->Items[i].Text != Str.data())
					{
						MenuItemEx Item;
						// Preserve the case of the already entered part
						if (Global->Opt->AutoComplete.AppendCompletion)
						{
							Item.UserData = cmp_user_data{ Str + (pList->Items[i].Text + Str.size()) };
						}
						ComplMenu->AddItem(pList->Items[i].Text);
					}
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

		const auto& AppendCmd = [&]
		{
			int SelStart = GetLength();

			const auto& FirstItem = ComplMenu->at(0).strName;
			const auto Data = ComplMenu->GetUserDataPtr<cmp_user_data>(0);

			// magic
			if (SelStart > 1 && IsSlash(m_Str[SelStart - 1]) && m_Str[SelStart - 2] == L'"' && IsSlash(FirstItem[SelStart - 2]))
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

				if (m_X2 - m_X1 > GetLength())
					SetLeftPos(0);
				Select(SelStart, GetLength());
			}
		};

		if(ComplMenu->size() > 1 || (ComplMenu->size() == 1 && !equal_icase(CurrentInput, ComplMenu->at(0).strName)))
		{
			ComplMenu->SetMenuFlags(VMENU_WRAPMODE | VMENU_SHOWAMPERSAND);
			if(!DelBlock && Global->Opt->AutoComplete.AppendCompletion && (!m_Flags.Check(FEDITLINE_PERSISTENTBLOCKS) || Global->Opt->AutoComplete.ShowList))
			{
				AppendCmd();
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
				DWORD Size;
				::GetCursorType(Visible, Size);
				ComplMenu->Key(KEY_NONE);
				bool IsChanged = false;
				int ExitCode=ComplMenu->Run([&](const Manager::Key& RawKey)
				{
					auto MenuKey = RawKey();
					::SetCursorType(Visible, Size);

					if(!Global->Opt->AutoComplete.ModalList)
					{
						int CurPos=ComplMenu->GetSelectPos();
						if(CurPos>=0 && (PrevPos!=CurPos || IsChanged))
						{
							PrevPos=CurPos;
							IsChanged = false;
							SetString(CurPos? ComplMenu->at(CurPos).strName : CurrentInput);
							Show();
						}
					}
					if(MenuKey==KEY_CONSOLE_BUFFER_RESIZE)
						SetMenuPos(*ComplMenu);
					else if(MenuKey!=KEY_NONE)
					{
						// ввод
						if((MenuKey>=L' ' && MenuKey<=static_cast<int>(WCHAR_MAX)) || MenuKey==KEY_BS || MenuKey==KEY_DEL || MenuKey==KEY_NUMDEL)
						{
							DeleteBlock();
							const auto strPrev = GetString();
							ProcessKey(Manager::Key(MenuKey));
							CurrentInput = GetString();
							if(strPrev != CurrentInput)
							{
								ComplMenu->clear();
								PrevPos=0;

								Complete(*ComplMenu, CurrentInput);

								if (ComplMenu->size() > 1 || (ComplMenu->size() == 1 && !equal_icase(CurrentInput, ComplMenu->at(0).strName)))
								{
									if(MenuKey!=KEY_BS && MenuKey!=KEY_DEL && MenuKey!=KEY_NUMDEL && Global->Opt->AutoComplete.AppendCompletion)
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
								Show();
							}
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
										const auto Data = ComplMenu->GetUserDataPtr<cmp_user_data>();
										if(Data && pHistory->DeleteIfUnlocked(Data->HistoryRecordId))
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
									if(MenuKey == KEY_LEFT || MenuKey == KEY_NUMPAD4)
									{
										MenuKey = KEY_CTRLS;
									}
									else if(MenuKey == KEY_RIGHT || MenuKey == KEY_NUMPAD6)
									{
										MenuKey = KEY_CTRLD;
									}
									m_ParentProcessKey(Manager::Key(MenuKey));
									Show();
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
							case KEY_IDLE:
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
								// fallthrough
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
									BackKey=RawKey;
									Result=1;
								}
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
						SetString(ComplMenu->at(ExitCode).strName);
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

void EditControl::AutoComplete(bool Manual,bool DelBlock)
{
	Manager::Key Key;
	if(AutoCompleteProc(Manual,DelBlock,Key,MacroAreaAC))
	{
		struct FAR_INPUT_RECORD irec = { static_cast<DWORD>(Key()), Key.Event() };
		if(!Global->CtrlObject->Macro.ProcessEvent(&irec))
			m_ParentProcessKey(Manager::Key(Key));
		int CurWindowType = Global->WindowManager->GetCurrentWindow()->GetType();
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
	if(Edit::ProcessMouse(MouseEvent))
	{
		while(IsMouseButtonPressed()==FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			m_Flags.Clear(FEDITLINE_CLEARFLAG);
			SetTabCurPos(IntKeyState.MouseX - m_X1 + GetLeftPos());
			if(IntKeyState.MouseEventFlags&MOUSE_MOVED)
			{
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
		}
		Selection=false;
		return true;
	}
	return false;
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

void EditControl::SetInputMask(const string& InputMask)
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
	if (!Mask.empty())
	{
		const auto MaskLen = Mask.size();
		m_Str.resize(MaskLen, L' ');

		for (size_t i = 0; i != MaskLen; ++i)
		{
			if (InitMode)
				m_Str[i]=L' ';

			if (!CheckCharMask(Mask[i]))
				m_Str[i]=Mask[i];
		}
	}
}

const string& EditControl::WordDiv() const
{
	return Global->Opt->strWordDiv;
}
