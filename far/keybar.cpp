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

KeyBar::KeyBar():
	Owner(nullptr),
	AltState(0),
	CtrlState(0),
	ShiftState(0),
	DisableMask(0),
	RegReaded(FALSE)
{
	_OT(SysLog(L"[%p] KeyBar::KeyBar()", this));
	ClearArray(KeyCounts);
	ClearArray(KeyTitles);
	ClearArray(KeyTitlesCustom);
}

KeyBar::~KeyBar()
{
	ClearKeyTitles(false);
	ClearKeyTitles(true);
}

void KeyBar::SetOwner(ScreenObject *Owner)
{
	KeyBar::Owner=Owner;
}


void KeyBar::DisplayObject()
{
	GotoXY(X1,Y1);
	AltState=CtrlState=ShiftState=0;
	int KeyWidth=(X2-X1-1)/12;

	if (KeyWidth<8)
		KeyWidth=8;

	int LabelWidth=KeyWidth-2;

	for (int i=0; i<KEY_COUNT; i++)
	{
		if (WhereX()+LabelWidth>=X2)
			break;

		SetColor(COL_KEYBARNUM);
		FS<<i+1;
		SetColor(COL_KEYBARTEXT);
		const wchar_t *Label=L"";

		if (IntKeyState.ShiftPressed)
		{
			ShiftState=IntKeyState.ShiftPressed;

			if (IntKeyState.CtrlPressed)
			{
				CtrlState=IntKeyState.CtrlPressed;

				if (!IntKeyState.AltPressed) // Ctrl-Alt-Shift - это особый случай :-)
				{
					if (i<KeyCounts [KBL_CTRLSHIFT])
						Label=KeyTitles [KBL_CTRLSHIFT][i];
				}
				else if (!(Opt.CASRule&1) || !(Opt.CASRule&2))
				{
					if (i<KeyCounts [KBL_CTRLALTSHIFT])
						Label=KeyTitles [KBL_CTRLALTSHIFT][i];
				}
			}
			else if (IntKeyState.AltPressed)
			{
				if (i<KeyCounts [KBL_ALTSHIFT])
					Label=KeyTitles [KBL_ALTSHIFT][i];

				AltState=IntKeyState.AltPressed;
			}
			else
			{
				if (i<KeyCounts [KBL_SHIFT])
					Label=KeyTitles [KBL_SHIFT][i];
			}
		}
		else if (IntKeyState.CtrlPressed)
		{
			CtrlState=IntKeyState.CtrlPressed;

			if (IntKeyState.AltPressed)
			{
				if (i<KeyCounts [KBL_CTRLALT])
					Label=KeyTitles [KBL_CTRLALT][i];

				AltState=IntKeyState.AltPressed;
			}
			else
			{
				if (i<KeyCounts [KBL_CTRL])
					Label=KeyTitles [KBL_CTRL][i];
			}
		}
		else if (IntKeyState.AltPressed)
		{
			AltState=IntKeyState.AltPressed;

			if (i<KeyCounts [KBL_ALT])
				Label=KeyTitles [KBL_ALT][i];
		}
		else if (i<KeyCounts [KBL_MAIN] && !(DisableMask & (1<<i)))
			Label=KeyTitles [KBL_MAIN][i];


		if (!Label)
			Label=L"";

		string strLabel=Label;

		if (strLabel.Contains(L'|'))
		{
			UserDefinedList LabelList(ULF_NOTRIM|ULF_NOUNQUOTE, L"|");
			if(LabelList.Set(Label) && !LabelList.IsEmpty())
			{
				string strLabelTest, strLabel2;
				strLabel=LabelList.GetNext();
				const wchar_t *Label2;
				while ((Label2=LabelList.GetNext()) != nullptr)
				{
					strLabelTest=strLabel;
					strLabelTest+=Label2;
					if (StrLength(strLabelTest) <= LabelWidth)
						if (StrLength(Label2) > StrLength(strLabel2))
							strLabel2=Label2;
				}

				strLabel+=strLabel2;
			}
		}

		FS<<fmt::LeftAlign()<<fmt::ExactWidth(LabelWidth)<<strLabel;

		if (i<KEY_COUNT-1)
		{
			SetColor(COL_KEYBARBACKGROUND);
			Text(L" ");
		}
	}

	int Width=X2-WhereX()+1;

	if (Width>0)
	{
		SetColor(COL_KEYBARTEXT);
		FS<<fmt::MinWidth(Width)<<L"";
	}
}

void KeyBar::ReadRegGroup(const wchar_t *RegGroup, const wchar_t *Language)
{
	if (!RegReaded || StrCmpI(strLanguage,Language) || StrCmpI(strRegGroupName,RegGroup))
	{
		DWORD Index=0;
		string strRegName;
		string strValue;
		string strValueName;

		strLanguage=Language;
		strRegGroupName=RegGroup;
		strRegName=L"KeyBarLabels.";
		strRegName+=strLanguage;
		strRegName+=L".";
		strRegName+=RegGroup;

		ClearKeyTitles(true);

		while (GeneralCfg->EnumValues(strRegName,Index++,strValueName,strValue))
		{
			DWORD Key=KeyNameToKey(strValueName);
			DWORD Key0=Key&(~KEY_CTRLMASK);
			DWORD Ctrl=Key&KEY_CTRLMASK;

			if (Key0 >= KEY_F1 && Key0 <= KEY_F24)
			{
				size_t J;
				static DWORD Area[][2]=
				{
					{ KBL_MAIN,         0 },
					{ KBL_SHIFT,        KEY_SHIFT },
					{ KBL_CTRL,         KEY_CTRL },
					{ KBL_ALT,          KEY_ALT },
					{ KBL_CTRLSHIFT,    KEY_CTRL|KEY_SHIFT },
					{ KBL_ALTSHIFT,     KEY_ALT|KEY_SHIFT },
					{ KBL_CTRLALT,      KEY_CTRL|KEY_ALT },
					{ KBL_CTRLALTSHIFT, KEY_CTRL|KEY_ALT|KEY_SHIFT },
				};

				for (J=0; J < ARRAYSIZE(Area); ++J)
					if (Area[J][1] == Ctrl)
						break;

				if (J <= ARRAYSIZE(Area))
				{
					KeyTitlesCustom[Area[J][0]][Key0-KEY_F1]=xf_wcsdup(strValue.CPtr());
				}
			}
		}

		RegReaded=TRUE;
	}
}

void KeyBar::ClearKeyTitles(bool Custom,int Group)
{
	KeyBarTitleGroup *kb = Custom? KeyTitlesCustom : KeyTitles;

	size_t Begin=0, End=KBL_GROUP_COUNT-1;
	if (Group != -1)
		Begin=End=(size_t)Group;

	for (size_t I=Begin; I <= End; I++)
	{
		for (size_t J=0; J < (size_t)KEY_COUNT; ++J)
		{
			if (kb[I][J])
			{
				xf_free((void*)kb[I][J]);
				kb[I][J]=nullptr;
			}
		}
	}
	if (Group != -1)
		ClearArray(kb[Group]);
	else
		ClearArray(*kb);
}

void KeyBar::SetRegGroup(int Group)
{
	for (int I=0; I < KEY_COUNT; I++)
		if (KeyTitlesCustom[Group][I] && *KeyTitlesCustom[Group][I])
		{
			if (KeyTitles[Group][I]) xf_free(KeyTitles[Group][I]);
			KeyTitles[Group][I]=xf_wcsdup(KeyTitlesCustom[Group][I]);
		}
}

void KeyBar::SetAllRegGroup()
{
	for (int I=0; I < KBL_GROUP_COUNT; ++I)
		SetRegGroup(I);
}


void KeyBar::SetGroup(int Group,const wchar_t * const *Key,int KeyCount)
{
	if (!Key) return;

	for (int i=0; i<KeyCount && i<KEY_COUNT; i++)
		if (Key[i])
		{
			if (KeyTitles[Group][i]) xf_free(KeyTitles[Group][i]);
			KeyTitles[Group][i]=xf_wcsdup(Key[i]);
		}

	KeyCounts [Group]=KeyCount;
}

void KeyBar::ClearGroup(int Group)
{
	ClearKeyTitles(false,Group);
	KeyCounts [Group] = 0;
}

// Изменение любого Label
void KeyBar::Change(int Group,const wchar_t *NewStr,int Pos)
{
	if (NewStr)
	{
		if (KeyTitles[Group][Pos]) xf_free(KeyTitles[Group][Pos]);
		KeyTitles[Group][Pos]=xf_wcsdup(NewStr);
	}
}


// Групповая установка идущих подряд строк LNG для указанной группы
void KeyBar::SetAllGroup(int Group, LNGID StartIndex, int Count)
{
	if (Count > KEY_COUNT)
		Count = KEY_COUNT;

	for (int i=0; i<Count; i++)
	{
		if (KeyTitles[Group][i]) xf_free(KeyTitles[Group][i]);
		KeyTitles[Group][i]=xf_wcsdup(MSG(StartIndex+i));
	}

	KeyCounts [Group] = Count;
}

int KeyBar::ProcessKey(int Key)
{
	switch (Key)
	{
		case KEY_KILLFOCUS:
		case KEY_GOTFOCUS:
			RedrawIfChanged();
			return TRUE;
	}

	return FALSE;
}

int KeyBar::ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent)
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
	FrameManager->ProcessKey(Key);
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


void KeyBar::SetDisableMask(int Mask)
{
	DisableMask=Mask;
}

void KeyBar::ResizeConsole()
{
}

size_t KeyBar::Change(const KeyBarTitles *Kbt)
{
	size_t Result=0;

	if (!Kbt)
		return Result;

	static DWORD Groups[]=
	{
		0,KBL_MAIN,
		KEY_SHIFT,KBL_SHIFT,
		KEY_CTRL,KBL_CTRL,
		KEY_ALT,KBL_ALT,
		KEY_CTRL|KEY_SHIFT,KBL_CTRLSHIFT,
		KEY_ALT|KEY_SHIFT,KBL_ALTSHIFT,
		KEY_CTRL|KEY_ALT,KBL_CTRLALT,
		KEY_CTRL|KEY_ALT|KEY_SHIFT,KBL_CTRLALTSHIFT,
	};

	for (size_t I = 0; I < Kbt->CountLabels; ++I)
	{

		WORD Pos=Kbt->Labels[I].Key.VirtualKeyCode;
		DWORD Shift=0,Flags=Kbt->Labels[I].Key.ControlKeyState;
		if(Flags&(LEFT_CTRL_PRESSED|RIGHT_CTRL_PRESSED)) Shift|=KEY_CTRL;
		if(Flags&(LEFT_ALT_PRESSED|RIGHT_ALT_PRESSED)) Shift|=KEY_ALT;
		if(Flags&SHIFT_PRESSED) Shift|=KEY_SHIFT;

		if (Pos >= VK_F1 && Pos <= VK_F24)
		{
			for (unsigned J=0; J < ARRAYSIZE(Groups); J+=2)
			{
				if (Groups[J] == Shift)
				{
					Change(Groups[J+1],Kbt->Labels[I].Text,Pos-VK_F1);
					Result++;
					break;
				}
			}
		}
	}

	return Result;
}
