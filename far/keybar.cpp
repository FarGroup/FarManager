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

#include "headers.hpp"
#pragma hdrstop

#include "keybar.hpp"
#include "colors.hpp"
#include "keyboard.hpp"
#include "keys.hpp"
#include "manager.hpp"
#include "syslog.hpp"
#include "language.hpp"
#include "interf.hpp"
#include "config.hpp"
#include "configdb.hpp"

enum
{
	KEY_COUNT = 12
};

KeyBar::KeyBar():
	Items(KBL_GROUP_COUNT),
	CustomArea(),
	Owner(nullptr),
	AltState(0),
	CtrlState(0),
	ShiftState(0),
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
	GotoXY(X1,Y1);
	AltState=CtrlState=ShiftState=0;
	int KeyWidth=(X2-X1-1)/12;

	if (KeyWidth<8)
		KeyWidth=8;

	int LabelWidth=KeyWidth-2;

	for (size_t i=0; i<KEY_COUNT; i++)
	{
		if (WhereX()+LabelWidth>=X2)
			break;

		SetColor(COL_KEYBARNUM);
		Global->FS << i+1;
		SetColor(COL_KEYBARTEXT);

		string Label;


		if (IntKeyState.ShiftPressed)
		{
			ShiftState=IntKeyState.ShiftPressed;

			if (IntKeyState.CtrlPressed)
			{
				CtrlState=IntKeyState.CtrlPressed;

				if (!IntKeyState.AltPressed) // Ctrl-Alt-Shift - это особый случай :-)
				{
					Label = Items[KBL_CTRLSHIFT][i].first;
				}
				else if (!(Global->Opt->CASRule&1) || !(Global->Opt->CASRule&2))
				{
					Label = Items[KBL_CTRLALTSHIFT][i].first;
				}
			}
			else if (IntKeyState.AltPressed)
			{
				Label = Items[KBL_ALTSHIFT][i].first;

				AltState=IntKeyState.AltPressed;
			}
			else
			{
				Label = Items[KBL_SHIFT][i].first;
			}
		}
		else if (IntKeyState.CtrlPressed)
		{
			CtrlState=IntKeyState.CtrlPressed;

			if (IntKeyState.AltPressed)
			{
				Label = Items[KBL_CTRLALT][i].first;

				AltState=IntKeyState.AltPressed;
			}
			else
			{
				Label = Items[KBL_CTRL][i].first;
			}
		}
		else if (IntKeyState.AltPressed)
		{
			AltState=IntKeyState.AltPressed;

			Label = Items[KBL_ALT][i].first;
		}
		else
		{
			Label = Items[KBL_MAIN][i].first;
		}

		if (Label.find(L'|') != string::npos)
		{
			auto LabelList(split_to_list::get(Label, STLF_NOTRIM | STLF_NOUNQUOTE, L"|"));
			if(!LabelList.empty())
			{
				string strLabelTest, strLabel2;
				Label = LabelList.front();
				LabelList.pop_front();
				std::for_each(CONST_RANGE(LabelList, Label2)
				{
					strLabelTest=Label;
					strLabelTest += Label2;
					if (strLabelTest.size() <= static_cast<size_t>(LabelWidth))
						if (Label2.size() > strLabel2.size())
							strLabel2 = Label2;
				});

				Label+=strLabel2;
			}
		}

		Global->FS << fmt::LeftAlign()<<fmt::ExactWidth(LabelWidth)<<Label;

		if (i<KEY_COUNT-1)
		{
			SetColor(COL_KEYBARBACKGROUND);
			Text(L' ');
		}
	}

	int Width=X2-WhereX()+1;

	if (Width>0)
	{
		SetColor(COL_KEYBARTEXT);
		Global->FS << fmt::MinWidth(Width)<<L"";
	}
}

void KeyBar::ClearKeyTitles(bool Custom)
{
	std::for_each(RANGE(Items, i)
	{
		std::for_each(RANGE(i, j)
		{
			(Custom? j.second : j.first).clear();
		});
	});
}

void KeyBar::SetLabels(LNGID StartIndex)
{
	std::for_each(RANGE(Items, Group)
	{
		std::for_each(RANGE(Group, i)
		{
			i.first = MSG(StartIndex++);
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
	static_assert(ARRAYSIZE(Area) == KBL_GROUP_COUNT, "Not all areas handled");

	auto ItemIterator = std::find_if(CONST_RANGE(Area, i)
	{
		return i.ControlState == ControlState;
	});

	return ItemIterator == std::cend(Area)? -1 : ItemIterator->Group;
}

void KeyBar::SetCustomLabels(KEYBARAREA Area)
{
	static const wchar_t* const Names[] =
	{
		L"Shell",
		L"Info",
		L"Tree",
		L"QView",
		L"FindFolder",
		L"Editor",
		L"Viewer",
		L"Help",
	};

	static_assert(ARRAYSIZE(Names) == KBA_COUNT, "Names not filled properly");

	if (Area < KBA_COUNT && (!CustomLabelsReaded || StrCmpI(strLanguage, Global->Opt->strLanguage) || Area != CustomArea))
	{
		strLanguage = Global->Opt->strLanguage.Get();
		CustomArea = Area;
		string strRegName = L"KeyBarLabels." + strLanguage + L"." + Names[Area];

		ClearKeyTitles(true);

		DWORD Index=0;
		string strValue, strValueName;
		while (Global->Db->GeneralCfg()->EnumValues(strRegName,Index++,strValueName,strValue))
		{
			DWORD Key = KeyNameToKey(strValueName);
			DWORD fnum = (Key & ~KEY_CTRLMASK) - KEY_F1;
			if (fnum < KEY_COUNT)
			{
				int fgroup = FnGroup(Key & KEY_CTRLMASK);
				if (fgroup >= 0)
					Items[fgroup][fnum].second = strValue;
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

// Изменение любого Label
void KeyBar::Change(int Group,const wchar_t *NewStr,int Pos)
{
	if (NewStr)
	{
		Items[Group][Pos].first = NewStr;
	}
}


int KeyBar::ProcessKey(const Manager::Key& Key)
{
	int LocalKey=Key.FarKey;
	switch (LocalKey)
	{
		case KEY_KILLFOCUS:
		case KEY_GOTFOCUS:
			RedrawIfChanged();
			return TRUE;
	}

	return FALSE;
}

int KeyBar::ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent)
{
	INPUT_RECORD rec;
	int Key;

	if (!IsVisible())
		return FALSE;

	if (!(MouseEvent->dwButtonState & 3) || MouseEvent->dwEventFlags)
		return FALSE;

	if (MouseEvent->dwMousePosition.X<X1 || MouseEvent->dwMousePosition.X>X2 ||
	        MouseEvent->dwMousePosition.Y!=Y1)
		return FALSE;

	int KeyWidth=(X2-X1-1)/12;

	if (KeyWidth<8)
		KeyWidth=8;

	int X=MouseEvent->dwMousePosition.X-X1;

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

	if (rec.Event.MouseEvent.dwMousePosition.X<X1 ||
	        rec.Event.MouseEvent.dwMousePosition.X>X2 ||
	        rec.Event.MouseEvent.dwMousePosition.Y!=Y1)
		return FALSE;

	int NewKey,NewX=MouseEvent->dwMousePosition.X-X1;

	if (NewX<KeyWidth*9)
		NewKey=NewX/KeyWidth;
	else
		NewKey=9+(NewX-KeyWidth*9)/(KeyWidth+1);

	if (Key!=NewKey)
		return FALSE;

	if (Key>11)
		Key=11;

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

	//if (Owner)
	//Owner->ProcessKey(Key);
	Global->FrameManager->ProcessKey(Manager::Key(Key));
	return TRUE;
}


void KeyBar::RedrawIfChanged()
{
	if (IntKeyState.ShiftPressed!=ShiftState ||
	        IntKeyState.CtrlPressed!=CtrlState ||
	        IntKeyState.AltPressed!=AltState)
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
		for (size_t I = 0; I < Kbt->CountLabels; ++I)
		{
			DWORD Pos = Kbt->Labels[I].Key.VirtualKeyCode - VK_F1;
			if (Pos < KEY_COUNT)
			{
				DWORD Shift = 0, Flags = Kbt->Labels[I].Key.ControlKeyState;
				if (Flags & (LEFT_CTRL_PRESSED|RIGHT_CTRL_PRESSED)) Shift |= KEY_CTRL;
				if (Flags & (LEFT_ALT_PRESSED|RIGHT_ALT_PRESSED)) Shift |= KEY_ALT;
				if (Flags & SHIFT_PRESSED) Shift |= KEY_SHIFT;

				int group = FnGroup(Shift);
				if (group >= 0)
				{
					Change(group, Kbt->Labels[I].Text, static_cast<int>(Pos));
					++Result;
				}
			}
		}
	}
	return Result;
}
