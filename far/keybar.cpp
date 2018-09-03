﻿/*
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

#include "keybar.hpp"

#include "farcolor.hpp"
#include "keyboard.hpp"
#include "keys.hpp"
#include "manager.hpp"
#include "syslog.hpp"
#include "lang.hpp"
#include "interf.hpp"
#include "config.hpp"
#include "configdb.hpp"
#include "strmix.hpp"
#include "global.hpp"
#include "plugin.hpp"

#include "common/enum_tokens.hpp"
#include "common/range.hpp"

#include "format.hpp"

enum
{
	KEY_COUNT = 12
};

KeyBar::KeyBar(window_ptr Owner):
	SimpleScreenObject(std::move(Owner)),
	Items(KBL_GROUP_COUNT),
	CustomArea(),
	AltState(),
	CtrlState(),
	ShiftState(),
	CustomLabelsReaded(false)
{
	std::for_each(RANGE(Items, i)
	{
		i.resize(KEY_COUNT);
	});

	_OT(SysLog(L"[%p] KeyBar::KeyBar()", this));
}

void KeyBar::DisplayObject()
{
	GotoXY(m_X1,m_Y1);
	AltState = IntKeyState.AltPressed();
	CtrlState = IntKeyState.CtrlPressed();
	ShiftState = IntKeyState.ShiftPressed();

	int KeyWidth=(m_X2-m_X1-1)/12;

	if (KeyWidth<8)
		KeyWidth=8;

	int LabelWidth=KeyWidth-2;

	for (size_t i=0; i<KEY_COUNT; i++)
	{
		if (WhereX()+LabelWidth>=m_X2)
			break;

		SetColor(COL_KEYBARNUM);
		Text(str(i + 1));
		SetColor(COL_KEYBARTEXT);

		static const std::pair<bool(FarKeyboardState::*)() const, keybar_group> Mapping[] =
		{
			{ &FarKeyboardState::NonePressed, KBL_MAIN },
			{ &FarKeyboardState::OnlyAltPressed, KBL_ALT },
			{ &FarKeyboardState::OnlyCtrlPressed, KBL_CTRL },
			{ &FarKeyboardState::OnlyShiftPressed, KBL_SHIFT },
			{ &FarKeyboardState::OnlyCtrlAltPressed, KBL_CTRLALT },
			{ &FarKeyboardState::OnlyAltShiftPressed, KBL_ALTSHIFT },
			{ &FarKeyboardState::OnlyCtrlShiftPressed, KBL_CTRLSHIFT },
			{ &FarKeyboardState::OnlyCtrlAltShiftPressed, KBL_CTRLALTSHIFT },
		};

		static_assert(std::size(Mapping) == KBL_GROUP_COUNT);

		const auto State = std::find_if(ALL_CONST_RANGE(Mapping), [&](const auto& Item) { return std::invoke(Item.first, IntKeyState); });
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
			{
				Label = concat(Beginning, Ending);
			}
		}

		Text(fit_to_left(Label, LabelWidth));

		if (i<KEY_COUNT-1)
		{
			SetColor(COL_KEYBARBACKGROUND);
			Text(L' ');
		}
	}

	int Width=m_X2-WhereX()+1;

	if (Width>0)
	{
		SetColor(COL_KEYBARTEXT);
		Text(string(Width, L' '));
	}
}

void KeyBar::ClearKeyTitles(bool Custom)
{
	const auto ItemPtr = Custom? &keybar_item::second : &keybar_item::first;
	std::for_each(RANGE(Items, i)
	{
		std::for_each(RANGE(i, j)
		{
			std::invoke(ItemPtr, j).clear();
		});
	});
}

void KeyBar::SetLabels(lng StartIndex)
{
	bool no_tree = Global->Opt->Tree.TurnOffCompletely;

	std::for_each(RANGE(Items, Group)
	{
		std::for_each(RANGE(Group, i)
		{
			if (no_tree && (StartIndex == lng::MAltF10 || StartIndex == lng::MInfoAltF10 || StartIndex == lng::MQViewAltF10))
				i.first.clear();
			else
				i.first = msg(StartIndex);
			StartIndex++;
		});
	});
}

static int FnGroup(DWORD ControlState)
{
	static const struct
	{
		DWORD Group;
		DWORD ControlState;
	}
	Area[] =
	{
		{KBL_MAIN, 0},
		{KBL_SHIFT, KEY_SHIFT},
		{KBL_ALT, KEY_ALT},
		{KBL_CTRL, KEY_CTRL},
		{KBL_ALTSHIFT, KEY_ALTSHIFT},
		{KBL_CTRLSHIFT, KEY_CTRLSHIFT},
		{KBL_CTRLALT, KEY_CTRLALT},
		{KBL_CTRLALTSHIFT, KEY_CTRLALT|KEY_SHIFT}
	};
	static_assert(std::size(Area) == KBL_GROUP_COUNT);

	const auto ItemIterator = std::find_if(CONST_RANGE(Area, i)
	{
		return i.ControlState == ControlState;
	});

	return ItemIterator == std::cend(Area)? -1 : ItemIterator->Group;
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

		for (auto& i: ConfigProvider().GeneralCfg()->ValuesEnumerator<string>(concat(L"KeyBarLabels."sv, strLanguage, L'.', Names[Area])))
		{
			DWORD Key = KeyNameToKey(i.first);
			DWORD fnum = (Key & ~KEY_CTRLMASK) - KEY_F1;
			if (fnum < KEY_COUNT)
			{
				int fgroup = FnGroup(Key & KEY_CTRLMASK);
				if (fgroup >= 0)
					Items[fgroup][fnum].second = i.second;
			}
		}
		CustomLabelsReaded = true;
	}

	std::for_each(RANGE(Items, Group)
	{
		std::for_each(RANGE(Group, i)
		{
			if (!i.second.empty())
			{
				i.first = i.second;
			}
		});
	});
}

bool KeyBar::ProcessKey(const Manager::Key& Key)
{
	const auto LocalKey = Key();
	switch (LocalKey)
	{
		case KEY_KILLFOCUS:
		case KEY_GOTFOCUS:
			RedrawIfChanged();
			return true;
	}

	return false;
}

bool KeyBar::ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent)
{
	INPUT_RECORD rec;
	size_t Key;

	if (!IsVisible())
		return false;

	if (!(MouseEvent->dwButtonState & 3) || MouseEvent->dwEventFlags)
		return false;

	if (MouseEvent->dwMousePosition.X<m_X1 || MouseEvent->dwMousePosition.X>m_X2 ||
	        MouseEvent->dwMousePosition.Y!=m_Y1)
		return false;

	int KeyWidth=(m_X2-m_X1-1)/12;

	if (KeyWidth<8)
		KeyWidth=8;

	int X=MouseEvent->dwMousePosition.X-m_X1;

	if (X<KeyWidth*9)
		Key=X/KeyWidth;
	else
		Key=9+(X-KeyWidth*9)/(KeyWidth+1);

	for (;;)
	{
		GetInputRecord(&rec);

		if (rec.EventType==MOUSE_EVENT && !(rec.Event.MouseEvent.dwButtonState & 3))
			break;
	}

	if (rec.Event.MouseEvent.dwMousePosition.X<m_X1 ||
	        rec.Event.MouseEvent.dwMousePosition.X>m_X2 ||
	        rec.Event.MouseEvent.dwMousePosition.Y!=m_Y1)
		return false;

	const int NewX = MouseEvent->dwMousePosition.X - m_X1;
	const size_t NewKey = NewX < KeyWidth * 9? NewX / KeyWidth : 9 + (NewX - KeyWidth * 9) / (KeyWidth + 1);

	if (Key!=NewKey)
		return false;

	if (Key > F12)
		Key = F12;

	if (MouseEvent->dwControlKeyState & (RIGHT_ALT_PRESSED|LEFT_ALT_PRESSED) ||
	        (MouseEvent->dwButtonState & RIGHTMOST_BUTTON_PRESSED))
	{
		if (MouseEvent->dwControlKeyState & SHIFT_PRESSED)
			Key+=KEY_ALTSHIFTF1;
		else if (MouseEvent->dwControlKeyState & (RIGHT_CTRL_PRESSED|LEFT_CTRL_PRESSED))
			Key+=KEY_CTRLALTF1;
		else
			Key+=KEY_ALTF1;
	}
	else if (MouseEvent->dwControlKeyState & (RIGHT_CTRL_PRESSED|LEFT_CTRL_PRESSED))
	{
		if (MouseEvent->dwControlKeyState & SHIFT_PRESSED)
			Key+=KEY_CTRLSHIFTF1;
		else
			Key+=KEY_CTRLF1;
	}
	else if (MouseEvent->dwControlKeyState & SHIFT_PRESSED)
		Key+=KEY_SHIFTF1;
	else
		Key+=KEY_F1;

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
		//_SVS("KeyBar::RedrawIfChanged()");
		Redraw();
	}
}

size_t KeyBar::Change(const KeyBarTitles *Kbt)
{
	size_t Result = 0;
	if (Kbt)
	{
		for (const auto& i: make_range(Kbt->Labels, Kbt->CountLabels))
		{
			DWORD Pos = i.Key.VirtualKeyCode - VK_F1;
			if (Pos < KEY_COUNT)
			{
				DWORD Shift = 0, Flags = i.Key.ControlKeyState;
				if (Flags & (LEFT_CTRL_PRESSED|RIGHT_CTRL_PRESSED)) Shift |= KEY_CTRL;
				if (Flags & (LEFT_ALT_PRESSED|RIGHT_ALT_PRESSED)) Shift |= KEY_ALT;
				if (Flags & SHIFT_PRESSED) Shift |= KEY_SHIFT;

				int group = FnGroup(Shift);
				if (group >= 0)
				{
					Items[group][Pos].first = NullToEmpty(i.Text);
					++Result;
				}
			}
		}
	}
	return Result;
}
