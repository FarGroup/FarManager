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
#include "language.hpp"
#include "pathmix.hpp"
#include "history.hpp"
#include "vmenu2.hpp"
#include "console.hpp"
#include "elevation.hpp"
#include "colormix.hpp"
#include "manager.hpp"
#include "interf.hpp"
#include "ctrlobj.hpp"
#include "strmix.hpp"

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
	int MaxHeight = std::min(Global->Opt->Dialogs.CBoxMaxHeight.Get(),(long long)menu.size()) + 1;

	int NewX2 = std::max(std::min(ScrX-2,(int)m_X2), m_X1 + 20);

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

static void AddSeparatorOrSetTitle(VMenu2& Menu, LNGID TitleId)
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
			MenuItemEx Item(MSG(TitleId));
			Item.Flags = LIF_SEPARATOR;
			Menu.AddItem(Item);
		}
		else
		{
			Menu.SetTitle(MSG(TitleId));
		}
	}
}

using enumerator_type = void(VMenu2&, const string&, const std::function<void(const string&)>&);

static bool EnumWithQuoutes(VMenu2& Menu, const string& Str, enumerator_type Enumerator)
{
	bool Result = false;
	if(!Str.empty())
	{
		size_t Pos = 0;
		if(std::count(ALL_CONST_RANGE(Str), L'"') & 1) // odd quotes count
		{
			Pos = Str.rfind(L'"');
		}
		else
		{
			auto WordDiv = GetSpaces() + Global->Opt->strWordDiv.Get();
			static const string NoQuote = L"\":\\/%.-";
			for (size_t i = 0; i != WordDiv.size(); ++i)
			{
				if (NoQuote.find(WordDiv[i]) != string::npos)
				{
					WordDiv.erase(i--, 1);
				}
			}

			for(Pos = Str.size()-1; Pos!=static_cast<size_t>(-1); Pos--)
			{
				if(Str[Pos]==L'"')
				{
					Pos--;
					while(Str[Pos]!=L'"' && Pos!=static_cast<size_t>(-1))
					{
						Pos--;
					}
				}
				else if (WordDiv.find(Str[Pos]) != string::npos)
				{
					Pos++;
					break;
				}
			}
		}
		if(Pos==static_cast<size_t>(-1))
		{
			Pos=0;
		}
		bool StartQuote=false;
		if(Pos < Str.size() && Str[Pos]==L'"')
		{
			Pos++;
			StartQuote=true;
		}

		const auto strStart = Str.substr(0, Pos);
		auto Token = Str.substr(Pos);
		Unquote(Token);
		if (!Token.empty())
		{
			std::set<string, string_i_less> ResultStrings;

			Enumerator(Menu, Token, [&](string strAdd)
			{
				if (!StartQuote)
				{
					QuoteSpace(strAdd);
				}

				string strTmp(strStart + strAdd);
				if (StartQuote)
				{
					strTmp += L'"';
				}

				ResultStrings.emplace(strTmp);
			});

			if (!ResultStrings.empty())
			{
				AddSeparatorOrSetTitle(Menu, MCompletionFilesTitle);

				std::for_each(CONST_RANGE(ResultStrings, i)
				{
					Menu.AddItem(i);
				});

				Result = true;
			}
		}
	}
	return Result;
}

static bool EnumFiles(VMenu2& Menu, const string& Str)
{
	SCOPED_ACTION(elevation::suppress);

	return EnumWithQuoutes(Menu, Str, [](VMenu2& Menu, const string& Token, const std::function<void(const string&)>& Inserter)
	{
		for (const auto& i: os::fs::enum_file(os::env::expand_strings(Token) + L"*"))
		{
			const auto FileName = PointToName(Token);
			const auto NameMatch = !StrCmpNI(FileName, i.strFileName.data(), StrLength(FileName));
			const auto AltNameMatch = !NameMatch && !StrCmpNI(FileName, i.strAlternateFileName.data(), StrLength(FileName));
			if (NameMatch || AltNameMatch)
			{
				Inserter(Token.substr(0, FileName - Token.data()) + (NameMatch? i.strFileName : i.strAlternateFileName));
			}
		}
	});
}

static bool EnumModules(VMenu2& Menu, const string& Module)
{
	SCOPED_ACTION(elevation::suppress);

	if (FindSlash(Module) != string::npos)
	{
		return false;
	}

	return EnumWithQuoutes(Menu, Module, [](VMenu2& Menu, const string& Token, const std::function<void(const string&)>& Inserter)
	{
		for (const auto& i: split<std::vector<string>>(os::env::expand_strings(Global->Opt->Exec.strExcludeCmds)))
		{
			if (!StrCmpNI(Token.data(), i.data(), Token.size()))
			{
				Inserter(i);
			}
		}

		{
			const auto strPathEnv(os::env::get_variable(L"PATH"));
			if (!strPathEnv.empty())
			{
				const auto PathExtList = split<std::vector<string>>(os::env::get_pathext());

				for (const auto& Path: split<std::vector<string>>(strPathEnv))
				{
					string str = Path;
					AddEndSlash(str);
					str.append(Token).append(L"*");
					for (const auto& FindData: os::fs::enum_file(str))
					{
						for (const auto& Ext: PathExtList)
						{
							if (!StrCmpI(Ext.data(), PointToExt(FindData.strFileName)))
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
							if (!StrCmpNI(Token.data(), SubkeyName.data(), Token.size()))
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

static bool EnumEnvironment(VMenu2& Menu, const string& Str)
{
	bool Result=false;

	SCOPED_ACTION(elevation::suppress);

	auto Token = Str.data();
	auto TokenSize = Str.size();
	string Head;
	{
		auto WordDiv = GetSpaces() + Global->Opt->strWordDiv.Get();
		std::replace(ALL_RANGE(WordDiv), L'%', L' ');
		const auto WordStart = Str.find_last_of(WordDiv);
		if (WordStart != string::npos)
		{
			Token += WordStart + 1;
			TokenSize -= WordStart + 1;
			Head = Str.substr(0, Str.size() - TokenSize);
		}
	}

	if(*Token)
	{
		std::set<string, string_i_less> ResultStrings;

		{
			const os::env::provider::strings EnvStrings;
			const auto EnvStringsPtr = EnvStrings.data();
			for (const auto& i: enum_substrings(EnvStringsPtr))
			{
				const auto VarName = L"%" + os::env::split(i.data()).first + L"%";

				if (!StrCmpNI(Token, VarName.data(), TokenSize))
				{
					ResultStrings.emplace(Head + VarName);
				}
			}
		}

		if (!ResultStrings.empty())
		{
			AddSeparatorOrSetTitle(Menu, MCompletionEnvironmentTitle);

			std::for_each(CONST_RANGE(ResultStrings, i)
			{
				Menu.AddItem(i);
			});

			Result = true;
		}
	}
	return Result;
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
		string strTemp = m_Str;

		ComplMenu->SetMacroMode(Area);

		const auto CompletionEnabled = [&Manual](int State)
		{
			return (Manual && State) || (!Manual && State == 1);
		};

		if(pHistory && ECFlags.Check(EC_COMPLETE_HISTORY) && CompletionEnabled(Global->Opt->AutoComplete.UseHistory))
		{
			if(pHistory->GetAllSimilar(*ComplMenu,strTemp))
			{
				ComplMenu->SetTitle(MSG(MCompletionHistoryTitle));
			}
		}
		else if(pList)
		{
			for(size_t i=0;i<pList->ItemsNumber;i++)
			{
				if (!StrCmpNI(pList->Items[i].Text, strTemp.data(), strTemp.size()) && pList->Items[i].Text != strTemp.data())
				{
					ComplMenu->AddItem(pList->Items[i].Text);
				}
			}
		}

		const auto Complete = [&](VMenu2& Menu, const string& Str)
		{
			if(ECFlags.Check(EC_COMPLETE_FILESYSTEM) && CompletionEnabled(Global->Opt->AutoComplete.UseFilesystem))
			{
				EnumFiles(Menu,Str);
			}
			if(ECFlags.Check(EC_COMPLETE_ENVIRONMENT) && CompletionEnabled(Global->Opt->AutoComplete.UseEnvironment))
			{
				EnumEnvironment(Menu, Str);
			}
			if(ECFlags.Check(EC_COMPLETE_PATH) && CompletionEnabled(Global->Opt->AutoComplete.UsePath))
			{
				EnumModules(Menu, Str);
			}
		};

		Complete(*ComplMenu, strTemp);

		const auto AppendCmd = [&]
		{
			int SelStart = GetLength();

			const auto& FirstItem = ComplMenu->at(0).strName;

			// magic
			if (SelStart > 1 && IsSlash(m_Str[SelStart - 1]) && m_Str[SelStart - 2] == L'"' && IsSlash(FirstItem[SelStart - 2]))
			{
				m_Str.erase(SelStart - 2, 1);
				SelStart--;
				m_CurPos--;
			}

			{
				SCOPED_ACTION(auto)(CallbackSuppressor());
				if (StrCmpI(FirstItem, m_Str + FirstItem.substr(SelStart)))
				{
					// New string contains opening quote, but not the original one
					++SelStart;
				}
				SetString(ComplMenu->at(0).strName);
				if (m_X2 - m_X1 > GetLength())
					SetLeftPos(0);
				Select(SelStart, GetLength());
			}
		};

		if(ComplMenu->size() > 1 || (ComplMenu->size() == 1 && StrCmpI(strTemp, ComplMenu->at(0).strName)))
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
							SetString(CurPos? ComplMenu->at(CurPos).strName : strTemp);
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
							strTemp = GetString();
							if(strPrev != strTemp)
							{
								ComplMenu->clear();
								PrevPos=0;
								if(!strTemp.empty())
								{
									if(pHistory && ECFlags.Check(EC_COMPLETE_HISTORY) && CompletionEnabled(Global->Opt->AutoComplete.UseHistory))
									{
										if(pHistory->GetAllSimilar(*ComplMenu,strTemp))
										{
											ComplMenu->SetTitle(MSG(MCompletionHistoryTitle));
										}
									}
									else if(pList)
									{
										for(size_t i=0;i<pList->ItemsNumber;i++)
										{
											if (!StrCmpNI(pList->Items[i].Text, strTemp.data(), strTemp.size()) && pList->Items[i].Text != strTemp.data())
											{
												ComplMenu->AddItem(pList->Items[i].Text);
											}
										}
									}
								}

								Complete(*ComplMenu, strTemp);

								if (ComplMenu->size() > 1 || (ComplMenu->size() == 1 && StrCmpI(strTemp, ComplMenu->at(0).strName)))
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
										const auto CurrentRecordPtr = ComplMenu->GetUserDataPtr<unsigned __int64>();
										if(CurrentRecordPtr && pHistory->DeleteIfUnlocked(*CurrentRecordPtr))
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
		// BUGBUG, hack
		int Wait=Global->WaitInMainLoop;
		Global->WaitInMainLoop=1;
		struct FAR_INPUT_RECORD irec={(DWORD)Key(), Key.Event()};
		if(!Global->CtrlObject->Macro.ProcessEvent(&irec))
			m_ParentProcessKey(Manager::Key(Key));
		Global->WaitInMainLoop=Wait;
		int CurWindowType = Global->WindowManager->GetCurrentWindow()->GetType();
		if (CurWindowType == windowtype_dialog || CurWindowType == windowtype_panels)
		{
			Show();
		}
	}
}

int EditControl::ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent)
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
		return TRUE;
	}
	return FALSE;
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
