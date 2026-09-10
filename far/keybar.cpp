/*
keybar.cpp

Keybar
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
#include "keybar.hpp"

// Internal:
#include "farcolor.hpp"
#include "keyboard.hpp"
#include "keys.hpp"
#include "manager.hpp"
#include "lang.hpp"
#include "interf.hpp"
#include "config.hpp"
#include "configdb.hpp"
#include "strmix.hpp"
#include "global.hpp"
#include "plugin.hpp"

// Platform:

// Common:
#include "common.hpp"
#include "common/enum_tokens.hpp"

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

KeyBar::KeyBar(window_ptr Owner):
	SimpleScreenObject(std::move(Owner))
{
}

void KeyBar::DisplayObject()
{
	GotoXY(m_Where.left, m_Where.top);
	AltState = IntKeyState.AltPressed();
	CtrlState = IntKeyState.CtrlPressed();
	ShiftState = IntKeyState.ShiftPressed();

	static const std::array Mapping
	{
		std::pair{ &FarKeyboardState::NonePressed,             KBL_MAIN         },
		std::pair{ &FarKeyboardState::OnlyAltPressed,          KBL_ALT          },
		std::pair{ &FarKeyboardState::OnlyCtrlPressed,         KBL_CTRL         },
		std::pair{ &FarKeyboardState::OnlyShiftPressed,        KBL_SHIFT        },
		std::pair{ &FarKeyboardState::OnlyCtrlAltPressed,      KBL_CTRLALT      },
		std::pair{ &FarKeyboardState::OnlyAltShiftPressed,     KBL_ALTSHIFT     },
		std::pair{ &FarKeyboardState::OnlyCtrlShiftPressed,    KBL_CTRLSHIFT    },
		std::pair{ &FarKeyboardState::OnlyCtrlAltShiftPressed, KBL_CTRLALTSHIFT },
	};

	static_assert(std::size(Mapping) == KBL_GROUP_COUNT);

	// 1Help  _ ... 12Screen_
	// NLLLLLLB ... NNLLLLLLB
	// 76543210 ... 876543210
	const auto MinLabelWidth = 6;
	const auto BackgroundSize = 1;

	const size_t TotalAvailableWidth = m_Where.width();
	const auto MinFullKeybarSize = (1 + MinLabelWidth + BackgroundSize) * KEY_COUNT + (KEY_COUNT - 9) - BackgroundSize;
	auto ExtraSpaceToRedistribute = TotalAvailableWidth > MinFullKeybarSize? TotalAvailableWidth - MinFullKeybarSize : 0;

	const auto print_and_continue = [&](PaletteColors const Color, string_view const Str, size_t const CellsAvailable)
	{
		SetColor(Color);
		Text(Str, CellsAvailable);
		return WhereX() <= m_Where.right;
	};

	m_KeyBoundariesSize = 0;

	for (const auto i: std::views::iota(0uz, KEY_COUNT))
	{
		// extra space for 2-digit numbers 10, 11, 12
		const size_t ExtraDigit = i + 1 > 9;

		const auto MinKeyWidth = 1 + ExtraDigit + MinLabelWidth + BackgroundSize;

		if (!print_and_continue(COL_KEYBARNUM, str(i + 1), 1 + ExtraDigit))
			break;

		const auto PrevBoundary = i == 0? 0 : m_KeyBoundaries[i - 1];

		m_KeyBoundaries[i] = static_cast<unsigned short>(PrevBoundary + MinKeyWidth);
		++m_KeyBoundariesSize;

		if (ExtraSpaceToRedistribute)
		{
			if (const auto BoundaryUsingBresenhamStyle = (i + 1) * TotalAvailableWidth / KEY_COUNT; BoundaryUsingBresenhamStyle > m_KeyBoundaries[i])
			{
				const auto ExtraSpace = BoundaryUsingBresenhamStyle - m_KeyBoundaries[i];
				ExtraSpaceToRedistribute -= ExtraSpace;
				m_KeyBoundaries[i] += static_cast<unsigned short>(ExtraSpace);
			}
		}

		const auto KeyWidth = m_KeyBoundaries[i] - PrevBoundary;
		const auto LabelWidth = KeyWidth - BackgroundSize - 1 - ExtraDigit;

		const auto State = std::ranges::find_if(Mapping, [&](const auto& Item) { return std::invoke(Item.first, IntKeyState); });
		// State should always be valid so check is excessive, but style is style
		auto Label = Items[(State != std::cend(Mapping)? State : std::cbegin(Mapping))->second][i].first;

		{
			string_view Beginning, Ending;
			auto FirstEntry = true;
			for (const auto& Part: enum_tokens_with_quotes(Label, L"|"sv))
			{
				if (FirstEntry)
				{
					Beginning = Part;
					FirstEntry = false;
					continue;
				}

				if (Beginning.size() + Part.size() > static_cast<size_t>(LabelWidth))
					break;

				if (Part.size() > Ending.size())
					Ending = Part;
			}

			if (!Beginning.empty())
				Label = concat(Beginning, Ending);
		}

		if (!print_and_continue(COL_KEYBARTEXT, pad_right(Label, LabelWidth), LabelWidth))
			break;

		if (i<KEY_COUNT-1)
		{
			if (!print_and_continue(COL_KEYBARBACKGROUND, L" "sv, 1))
				break;
		}
	}

	const auto Width = m_Where.right - WhereX() + 1;

	if (Width>0)
	{
		SetColor(COL_KEYBARTEXT);
		Text(string(Width, L' '));
	}
}

void KeyBar::ClearKeyTitles(bool Custom)
{
	const auto ItemGetter = Custom? &keybar_item::second : &keybar_item::first;

	for (auto& i: Items)
	{
		for (auto& j: i)
		{
			std::invoke(ItemGetter, j).clear();
		}
	}
}

void KeyBar::SetLabels(lng StartIndex)
{
	for (auto& Group: Items)
	{
		for (auto& i: Group)
		{
			i.first = msg(StartIndex);
			StartIndex++;
		}
	}
}

static int FnGroup(unsigned ControlKey)
{
	switch (ControlKey)
	{
	case NO_KEY:            return KBL_MAIN;
	case KEY_ALT:           return KBL_ALT;
	case KEY_CTRL:          return KBL_CTRL;
	case KEY_SHIFT:         return KBL_SHIFT;
	case KEY_CTRLALT:       return KBL_CTRLALT;
	case KEY_ALTSHIFT:      return KBL_ALTSHIFT;
	case KEY_CTRLSHIFT:     return KBL_CTRLSHIFT;
	case KEY_CTRLALTSHIFT:  return KBL_CTRLALTSHIFT;
	default:                return -1;
	}
}

void KeyBar::SetCustomLabels(KEYBARAREA Area)
{
	static const string_view Names[]
	{
		L"Shell"sv,
		L"Info"sv,
		L"Tree"sv,
		L"QView"sv,
		L"FindFolder"sv,
		L"Editor"sv,
		L"Viewer"sv,
		L"Help"sv,
	};

	static_assert(std::size(Names) == KBA_COUNT);

	if (Area < KBA_COUNT && (!CustomLabelsReaded || !equal_icase(strLanguage, Global->Opt->strLanguage.Get()) || Area != CustomArea))
	{
		strLanguage = Global->Opt->strLanguage.Get();
		CustomArea = Area;
		ClearKeyTitles(true);

		const auto LabelsKey = concat(L"KeyBarLabels."sv, strLanguage, L'.', Names[Area]);

		for (const auto& [Name, Value]: ConfigProvider().GeneralCfg()->ValuesEnumerator<string>(LabelsKey))
		{
			const auto Key = KeyNameToKey(Name);
			if (!Key)
				continue;

			const auto fnum = (Key & ~KEY_CTRLMASK) - KEY_F1;
			if (fnum < KEY_COUNT)
			{
				const auto fgroup = FnGroup(Key & KEY_CTRLMASK);
				if (fgroup >= 0)
					Items[fgroup][fnum].second = Value;
			}
		}
		CustomLabelsReaded = true;
	}

	for (auto& Group: Items)
	{
		for (auto& [Title, CustomTitle]: Group)
		{
			if (!CustomTitle.empty())
			{
				Title = CustomTitle;
			}
		}
	}
}

bool KeyBar::ProcessKey(const Manager::Key& Key)
{
	switch (Key())
	{
		case KEY_KILLFOCUS:
		case KEY_GOTFOCUS:
			RedrawIfChanged();
			return true;
	}

	return false;
}

static unsigned control_state_to_key(DWORD ControlState)
{
	unsigned Result{};

	if (ControlState & (RIGHT_ALT_PRESSED | LEFT_ALT_PRESSED))
		Result |= KEY_ALT;

	if (ControlState & (RIGHT_CTRL_PRESSED | LEFT_CTRL_PRESSED))
		Result |= KEY_CTRL;

	if (ControlState & SHIFT_PRESSED)
		Result |= KEY_SHIFT;

	return Result;
}

bool KeyBar::ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent)
{
	if (!IsVisible())
		return false;

	if (!(MouseEvent->dwButtonState & 3) || !IsMouseButtonEvent(MouseEvent->dwEventFlags))
		return false;

	if (!m_Where.contains(MouseEvent->dwMousePosition))
		return false;

	if (!m_KeyBoundariesSize)
		return false;

	const auto PressedKeyIndex = [&](unsigned short const X)
	{
		const auto Boundaries = std::span(m_KeyBoundaries.data(), m_KeyBoundariesSize);
		return static_cast<size_t>(std::ranges::upper_bound(Boundaries, X) - Boundaries.begin());
	};

	const auto KeyIndex = PressedKeyIndex(MouseEvent->dwMousePosition.X - m_Where.left);

	for (;;)
	{
		INPUT_RECORD rec;
		GetInputRecord(&rec);

		if (rec.EventType == MOUSE_EVENT && !(rec.Event.MouseEvent.dwButtonState & 3)) // Release
		{
			const auto& NewEvent = rec.Event.MouseEvent;

			if (!m_Where.contains(NewEvent.dwMousePosition))
				return false;

			if (const auto ReleaseKeyIndex = PressedKeyIndex(NewEvent.dwMousePosition.X - m_Where.left); KeyIndex != ReleaseKeyIndex)
				return false;

			break;
		}
	}

	auto Key = (KEY_F1 | control_state_to_key(MouseEvent->dwControlKeyState)) + KeyIndex;

	if (MouseEvent->dwButtonState & RIGHTMOST_BUTTON_PRESSED)
		Key |= KEY_ALT;

	Global->WindowManager->ProcessKey(Manager::Key(static_cast<int>(Key)));
	return true;
}


void KeyBar::RedrawIfChanged()
{
	if (
		IntKeyState.ShiftPressed() != ShiftState ||
		IntKeyState.CtrlPressed() != CtrlState ||
		IntKeyState.AltPressed() != AltState)
	{
		Redraw();
	}
}

size_t KeyBar::Change(const KeyBarTitles *Kbt)
{
	if (!Kbt)
		return 0;

	size_t Result = 0;

	for (const auto& i: std::span(Kbt->Labels, Kbt->CountLabels))
	{
		if (i.Key.VirtualKeyCode < VK_F1 || i.Key.VirtualKeyCode >= VK_F1 + KEY_COUNT)
			continue;

		const auto Pos = i.Key.VirtualKeyCode - VK_F1;
		const auto Group = FnGroup(control_state_to_key(i.Key.ControlKeyState));
		if (Group < 0)
			continue;

		Items[Group][Pos].first = NullToEmpty(i.Text);
		++Result;
	}

	return Result;
}
