/*
hmenu.cpp

Горизонтальное меню
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

#include "hmenu.hpp"

#include "farcolor.hpp"
#include "keys.hpp"
#include "dialog.hpp"
#include "vmenu.hpp"
#include "vmenu2.hpp"
#include "ctrlobj.hpp"
#include "filepanels.hpp"
#include "macroopcode.hpp"
#include "savescr.hpp"
#include "lockscrn.hpp"
#include "interf.hpp"
#include "keyboard.hpp"
#include "colormix.hpp"
#include "manager.hpp"
#include "string_utils.hpp"
#include "global.hpp"

#include "common/scope_exit.hpp"

HMenu::HMenu(private_tag, HMenuData *Item,size_t ItemCount):
	Item(Item, Item + ItemCount),
	SelectPos(),
	m_VExitCode(-1),
	m_SubmenuOpened()
{
}

hmenu_ptr HMenu::create(HMenuData *Item, size_t ItemCount)
{
	auto HmenuPtr = std::make_shared<HMenu>(private_tag(), Item, ItemCount);
	HmenuPtr->init();
	return HmenuPtr;
}

void HMenu::init()
{
	SetMacroMode(MACROAREA_MAINMENU);
	SetRestoreScreenMode(true);
}

HMenu::~HMenu()
{
	Global->WindowManager->RefreshWindow();
}

void HMenu::DisplayObject()
{
	SetScreen(m_Where, L' ', colors::PaletteColorToFarColor(COL_HMENUTEXT));
	SetCursorType(false, 10);
	ShowMenu();
}


void HMenu::ShowMenu()
{
	GotoXY(m_Where.left + 2, m_Where.top);

	for (auto& i: Item)
	{
		i.XPos = WhereX();

		SetColor(i.Selected? COL_HMENUSELECTEDTEXT : COL_HMENUTEXT);

		HiText(concat(L"  "sv, i.Name, L"  "sv), colors::PaletteColorToFarColor(i.Selected? COL_HMENUSELECTEDHIGHLIGHT : COL_HMENUHIGHLIGHT));
	}
}

void HMenu::UpdateSelectPos()
{
	SelectPos = 0;

	const auto Selected = std::find_if(Item.begin(), Item.end(), [](const auto& i) { return i.Selected; });
	if (Selected != Item.end())
	{
		SelectPos = Selected - Item.begin();
	}
}

long long HMenu::VMProcess(int OpCode, void* vParam, long long iParam)
{
	UpdateSelectPos();

	switch (OpCode)
	{
		case MCODE_C_EMPTY:
			return Item.empty();
		case MCODE_C_EOF:
			return SelectPos == Item.size() - 1;
		case MCODE_C_BOF:
			return !SelectPos;
		case MCODE_C_SELECTED:
			return !Item.empty();
		case MCODE_V_ITEMCOUNT:
			return Item.size();
		case MCODE_V_CURPOS:
			return SelectPos+1;
		case MCODE_F_MENU_CHECKHOTKEY:
		{
			return CheckHighlights(*static_cast<const wchar_t *>(vParam), static_cast<int>(iParam))+1;
		}
		case MCODE_F_MENU_GETHOTKEY:
		case MCODE_F_MENU_GETVALUE: // S=Menu.GetValue([N])
		{
			if (iParam == -1)
				iParam=SelectPos;

			if (static_cast<size_t>(iParam) < Item.size())
			{
				if (OpCode == MCODE_F_MENU_GETVALUE)
				{
					*static_cast<string*>(vParam) = Item[static_cast<size_t>(iParam)].Name;
					return 1;
				}
				else
				{
					return GetHighlights(Item[static_cast<size_t>(iParam)]);
				}
			}

			return 0;
		}
		case MCODE_F_MENU_ITEMSTATUS: // N=Menu.ItemStatus([N])
		{
			long long RetValue = -1;

			if (iParam == -1)
				iParam=SelectPos;

			if (static_cast<size_t>(iParam) < Item.size())
			{
				RetValue=0;
				if (Item[static_cast<size_t>(iParam)].Selected)
					RetValue |= 1;
			}

			return RetValue;
		}
		case MCODE_V_MENU_VALUE: // Menu.Value
		{
			*static_cast<string*>(vParam) = Item[SelectPos].Name;
			return 1;
		}
	}

	return 0;
}


bool HMenu::ProcessPositioningKey(unsigned LocalKey)
{
	switch (LocalKey)
	{
	case KEY_TAB:
		Item[SelectPos].Selected = false;
		/* Кусок для "некрайних" меню - прыжок к меню пассивной панели */
		if (SelectPos && SelectPos != Item.size() - 1)
		{
			if (Global->CtrlObject->Cp()->IsRightActive())
				SelectPos = 0;
			else
				SelectPos = Item.size() - 1;
		}
		else
		{
			if (!SelectPos)
				SelectPos = Item.size() - 1;
			else
				SelectPos = 0;
		}

		Item[SelectPos].Selected = true;
		break;

	case KEY_HOME:
	case KEY_CTRLHOME:
	case KEY_RCTRLHOME:
	case KEY_CTRLPGUP:
	case KEY_RCTRLPGUP:
	case KEY_NUMPAD7:
	case KEY_CTRLNUMPAD7:
	case KEY_RCTRLNUMPAD7:
	case KEY_CTRLNUMPAD9:
	case KEY_RCTRLNUMPAD9:
		Item[SelectPos].Selected = false;
		Item[0].Selected = true;
		SelectPos = 0;
		break;

	case KEY_END:
	case KEY_CTRLEND:
	case KEY_RCTRLEND:
	case KEY_CTRLPGDN:
	case KEY_RCTRLPGDN:
	case KEY_NUMPAD1:
	case KEY_CTRLNUMPAD1:
	case KEY_RCTRLNUMPAD1:
	case KEY_CTRLNUMPAD3:
	case KEY_RCTRLNUMPAD3:
		Item[SelectPos].Selected = false;
		Item[Item.size() - 1].Selected = true;
		SelectPos = Item.size() - 1;
		break;

	case KEY_LEFT:
	case KEY_NUMPAD4:
	case KEY_MSWHEEL_LEFT:
		Item[SelectPos].Selected = false;
		if (!SelectPos)
			SelectPos = Item.size() - 1;
		else
			--SelectPos;
		Item[SelectPos].Selected = true;
		break;

	case KEY_RIGHT:
	case KEY_NUMPAD6:
	case KEY_MSWHEEL_RIGHT:
		Item[SelectPos].Selected = false;
		if (SelectPos == Item.size() - 1)
			SelectPos = 0;
		else
			++SelectPos;
		Item[SelectPos].Selected = true;
		break;

	default:
		return false;
	}

	ShowMenu();
	return true;
}

bool HMenu::ProcessKey(const Manager::Key& Key)
{
	auto LocalKey = Key();

	UpdateSelectPos();

	if (ProcessPositioningKey(LocalKey))
		return true;

	switch (LocalKey)
	{
	case KEY_ALTF9:
	case KEY_RALTF9:
		Global->WindowManager->ProcessKey(Manager::Key(KEY_ALTF9));
		break;

	case KEY_NONE:
	case KEY_IDLE:
		return false;

	case KEY_F1:
		ShowHelp();
		return true;

	case KEY_NUMENTER:
	case KEY_ENTER:
	case KEY_UP:
	case KEY_DOWN:
	case KEY_NUMPAD8:
	case KEY_NUMPAD2:
		return ProcessCurrentSubMenu();

	case KEY_ESC:
	case KEY_F10:
		Close(-1);
		return false;

	default:
		const auto FindHighlightedKey = [&](bool Translate)
		{
			return std::find_if(Item.begin(), Item.end(), [&](const auto& Element)
			{
				return IsKeyHighlighted(Element.Name, LocalKey, Translate);
			});
		};

		auto Iterator = FindHighlightedKey(false);
		if (Iterator == Item.end())
		{
			Iterator = FindHighlightedKey(true);
			if (Iterator == Item.end())
				return false;
		}

		Item[SelectPos].Selected = false;
		Iterator->Selected = true;
		SelectPos = Iterator - Item.begin();
		ShowMenu();
		ProcessCurrentSubMenu();
		return true;
	}

	return false;
}

bool HMenu::TestMouse(const MOUSE_EVENT_RECORD *MouseEvent) const
{
	int MsX=MouseEvent->dwMousePosition.X;
	int MsY=MouseEvent->dwMousePosition.Y;

	return MsY != m_Where.top || ((!SelectPos || MsX >= Item[SelectPos].XPos) && (SelectPos == Item.size() - 1 || MsX<Item[SelectPos + 1].XPos));
}

bool HMenu::ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent)
{
	UpdateSelectPos();

	if (m_Where.contains(MouseEvent->dwMousePosition))
	{
		const auto SubmenuIterator = std::find_if(REVERSE_RANGE(Item, i) { return MouseEvent->dwMousePosition.X >= i.XPos; });
		const size_t NewPos = std::distance(SubmenuIterator, Item.rend()) - 1;

		if (m_SubmenuOpened && SelectPos == NewPos)
			return false;

		Item[SelectPos].Selected = false;
		SubmenuIterator->Selected = true;
		SelectPos = NewPos;
		ShowMenu();
		ProcessCurrentSubMenu();
	}
	else if (!(MouseEvent->dwButtonState & 3) && !MouseEvent->dwEventFlags)
		Close(-1);

	return true;
}


void HMenu::GetExitCode(int &ExitCode,int &VExitCode) const
{
	ExitCode = m_ExitCode;
	VExitCode = m_VExitCode;
}

bool HMenu::ProcessCurrentSubMenu()
{
	for (;;)
	{
		UpdateSelectPos();

		if (Item[SelectPos].SubMenu.empty())
			return false;

		bool SendKey = false, SendMouse = false;
		int MenuKey;
		MOUSE_EVENT_RECORD MenuMouseRecord;

		{
			m_SubmenuOpened = true;
			SCOPE_EXIT{ m_SubmenuOpened = false; };

			const auto SubMenu = VMenu2::create({}, Item[SelectPos].SubMenu);
			SubMenu->SetBoxType(SHORT_DOUBLE_BOX);
			SubMenu->SetMenuFlags(VMENU_WRAPMODE);
			SubMenu->SetHelp(Item[SelectPos].SubMenuHelp);
			SubMenu->SetPosition({ Item[SelectPos].XPos, m_Where.top + 1, 0, 0 });
			SubMenu->SetMacroMode(MACROAREA_MAINMENU);

			m_VExitCode = SubMenu->RunEx([&](int Msg, void *param)
			{
				if (Msg != DN_INPUT)
					return 0;

				auto& rec = *static_cast<INPUT_RECORD*>(param);
				int Key = InputRecordToKey(&rec);

				if (Key == KEY_CONSOLE_BUFFER_RESIZE)
				{
					SCOPED_ACTION(LockScreen);
					ResizeConsole();
					Show();
					return 1;
				}
				else if (rec.EventType == MOUSE_EVENT)
				{
					if (!TestMouse(&rec.Event.MouseEvent))
					{
						MenuMouseRecord = rec.Event.MouseEvent;
						SendMouse = true;
						SubMenu->Close(-1);
						return 1;
					}
					if (rec.Event.MouseEvent.dwMousePosition.Y == m_Where.top)
						return 1;
				}
				else
				{
					if (Key == KEY_LEFT || Key == KEY_RIGHT || Key == KEY_TAB ||
						Key == KEY_NUMPAD4 || Key == KEY_NUMPAD6 ||
						Key == KEY_MSWHEEL_LEFT || Key == KEY_MSWHEEL_RIGHT)
					{
						SendKey = true;
						MenuKey = Key;
						SubMenu->Close(-1);
						return 1;
					}
				}
				return 0;
			});

			if (m_VExitCode != -1)
			{
				Close(static_cast<int>(SelectPos));
				return true;
			}
		}

		if (SendMouse)
			ProcessMouse(&MenuMouseRecord);

		if (!SendKey)
			break;

		ProcessPositioningKey(MenuKey);
	}

	return true;
}
void HMenu::ResizeConsole()
{
	SaveScr.reset();
	Hide();
	Modal::ResizeConsole();
	SetPosition({ 0, 0, ScrX, 0 });
}

wchar_t HMenu::GetHighlights(const HMenuData& MenuItem) const
{
	const auto Pos = MenuItem.Name.find(L'&');
	if (Pos == MenuItem.Name.npos)
		return 0;

	if (Pos + 1 == MenuItem.Name.size())
		return 0;

	return MenuItem.Name[Pos + 1];
}

size_t HMenu::CheckHighlights(WORD CheckSymbol, int StartPos) const
{
	if (StartPos < 0)
		StartPos=0;

	for (size_t I = StartPos; I < Item.size(); I++)
	{
		wchar_t Ch=GetHighlights(Item[I]);

		if (Ch)
		{
			if (upper(CheckSymbol) == upper(Ch))
				return I;
		}
		else if (!CheckSymbol)
			return I;
	}

	return -1;
}
