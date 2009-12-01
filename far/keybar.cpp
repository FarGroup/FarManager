/*
keybar.cpp

Keybar
*/
/*
Copyright (c) 1996 Eugene Roshal
Copyright (c) 2000 Far Group
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
#include "registry.hpp"
#include "language.hpp"
#include "interf.hpp"
#include "config.hpp"

KeyBar::KeyBar()
{
	_OT(SysLog(L"[%p] KeyBar::KeyBar()", this));
	DisableMask=0;
	Owner=NULL;
	AltState=CtrlState=ShiftState=0;
	RegReaded=FALSE;
	memset(KeyTitles, 0, sizeof(KeyTitles));
	memset(KeyCounts, 0, sizeof(KeyCounts));
	memset(RegKeyTitles, 0, sizeof(RegKeyTitles));
}


void KeyBar::SetOwner(ScreenObject *Owner)
{
	KeyBar::Owner=Owner;
}


void KeyBar::DisplayObject()
{
	int I;
	GotoXY(X1,Y1);
	AltState=CtrlState=ShiftState=0;
	int KeyWidth=(X2-X1-1)/12;

	if (KeyWidth<8)
		KeyWidth=8;

	int LabelWidth=KeyWidth-2;

	for (I=0; I<KEY_COUNT; I++)
	{
		if (WhereX()+LabelWidth>=X2)
			break;

		SetColor(COL_KEYBARNUM);
		mprintf(L"%d",I+1);
		SetColor(COL_KEYBARTEXT);
		const wchar_t *Label=L"";

		if (ShiftPressed)
		{
			ShiftState=ShiftPressed;

			if (CtrlPressed)
			{
				CtrlState=CtrlPressed;

				if (!AltPressed) // Ctrl-Alt-Shift - это особый случай :-)
				{
					if (I<KeyCounts [KBL_CTRLSHIFT])
						Label=KeyTitles [KBL_CTRLSHIFT][I];
				}
				else if (!(Opt.CASRule&1) || !(Opt.CASRule&2))
				{
					if (I<KeyCounts [KBL_CTRLALTSHIFT])
						Label=KeyTitles [KBL_CTRLALTSHIFT][I];
				}
			}
			else if (AltPressed)
			{
				if (I<KeyCounts [KBL_ALTSHIFT])
					Label=KeyTitles [KBL_ALTSHIFT][I];

				AltState=AltPressed;
			}
			else
			{
				if (I<KeyCounts [KBL_SHIFT])
					Label=KeyTitles [KBL_SHIFT][I];
			}
		}
		else if (CtrlPressed)
		{
			CtrlState=CtrlPressed;

			if (AltPressed)
			{
				if (I<KeyCounts [KBL_CTRLALT])
					Label=KeyTitles [KBL_CTRLALT][I];

				AltState=AltPressed;
			}
			else
			{
				if (I<KeyCounts [KBL_CTRL])
					Label=KeyTitles [KBL_CTRL][I];
			}
		}
		else if (AltPressed)
		{
			AltState=AltPressed;

			if (I<KeyCounts [KBL_ALT])
				Label=KeyTitles [KBL_ALT][I];
		}
		else if (I<KeyCounts [KBL_MAIN] && (DisableMask & (1<<I))==0)
			Label=KeyTitles [KBL_MAIN][I];

		mprintf(L"%-*.*s",LabelWidth,LabelWidth,Label);

		if (I<KEY_COUNT-1)
		{
			SetColor(COL_KEYBARBACKGROUND);
			Text(L" ");
		}
	}

	int Width=X2-WhereX()+1;

	if (Width>0)
	{
		SetColor(COL_KEYBARTEXT);
		mprintf(L"%*s",Width,L"");
	}
}

void KeyBar::ReadRegGroup(const wchar_t *RegGroup, const wchar_t *Language)
{
	if (!RegReaded || StrCmpI(strLanguage,Language) || StrCmpI(strRegGroupName,RegGroup))
	{
		DWORD I;
		string strRegName;
		string strValue;
		string strValueName;
		memset(RegKeyTitles, 0, sizeof(RegKeyTitles));
		strLanguage=Language;
		strRegGroupName=RegGroup;
		strRegName=L"KeyBarLabels\\";
		strRegName+=strLanguage;
		strRegName+=L"\\";
		strRegName+=RegGroup;

		for (I=0;; I++)
		{
			strValueName.Clear();
			strValue.Clear();
			int Type=EnumRegValueEx(strRegName,I,strValueName,strValue);

			if (Type == REG_NONE)
				break;

			if (Type == REG_SZ)
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

					for (J=0; J < countof(Area); ++J)
						if (Area[J][1] == Ctrl)
							break;

					if (J <= countof(Area))
					{
						Key0 -= KEY_F1;
						int Group=Area[J][0];
						xwcsncpy(RegKeyTitles[Group][Key0], strValue, countof(KeyTitles[Group][Key0])-1);
					}
				}
			}
		}

		RegReaded=TRUE;
	}
}

void KeyBar::SetRegGroup(int Group)
{
	for (int I=0; I < KEY_COUNT; I++)
		if (*RegKeyTitles[Group][I])
			xwcsncpy(KeyTitles[Group][I], RegKeyTitles[Group][I], countof(KeyTitles[Group][I])-1);
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
			xwcsncpy(KeyTitles[Group][i], Key[i], countof(KeyTitles[Group][i])-1);

	KeyCounts [Group]=KeyCount;
}

void KeyBar::ClearGroup(int Group)
{
	memset(KeyTitles[Group], 0, sizeof(KeyTitles[Group]));
	KeyCounts [Group] = 0;
}

// Изменение любого Label
void KeyBar::Change(int Group,const wchar_t *NewStr,int Pos)
{
	if (NewStr)
		xwcsncpy(KeyTitles[Group][Pos], NewStr, countof(KeyTitles[Group][Pos])-1);
}


// Групповая установка идущих подряд строк LNG для указанной группы
void KeyBar::SetAllGroup(int Group, int StartIndex, int Count)
{
	if (Count > KEY_COUNT)
		Count = KEY_COUNT;

	for (int i=0, Index=StartIndex; i<Count; i++, Index++)
		xwcsncpy(KeyTitles[Group][i], MSG(Index), countof(KeyTitles[Group][i])-1);

	KeyCounts [Group] = Count;
}

int KeyBar::ProcessKey(int Key)
{
	switch (Key)
	{
		case KEY_KILLFOCUS:
		case KEY_GOTFOCUS:
			RedrawIfChanged();
			return(TRUE);
	} /* switch */

	return(FALSE);
}

int KeyBar::ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent)
{
	INPUT_RECORD rec;
	int Key;

	if (!IsVisible())
		return(FALSE);

	if ((MouseEvent->dwButtonState & 3)==0 || MouseEvent->dwEventFlags!=0)
		return(FALSE);

	if (MouseEvent->dwMousePosition.X<X1 || MouseEvent->dwMousePosition.X>X2 ||
	        MouseEvent->dwMousePosition.Y!=Y1)
		return(FALSE);

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

		if (rec.EventType==MOUSE_EVENT && (rec.Event.MouseEvent.dwButtonState & 3)==0)
			break;
	}

	if (rec.Event.MouseEvent.dwMousePosition.X<X1 ||
	        rec.Event.MouseEvent.dwMousePosition.X>X2 ||
	        rec.Event.MouseEvent.dwMousePosition.Y!=Y1)
		return(FALSE);

	int NewKey,NewX=MouseEvent->dwMousePosition.X-X1;

	if (NewX<KeyWidth*9)
		NewKey=NewX/KeyWidth;
	else
		NewKey=9+(NewX-KeyWidth*9)/(KeyWidth+1);

	if (Key!=NewKey)
		return(FALSE);

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
	return(TRUE);
}


void KeyBar::RedrawIfChanged()
{
	if (ShiftPressed!=ShiftState ||
	        CtrlPressed!=CtrlState ||
	        AltPressed!=AltState)
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
