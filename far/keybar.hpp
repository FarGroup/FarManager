#pragma once

/*
keybar.hpp

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

#include "scrobj.hpp"
#include "plugin.hpp"

//   Группы меток
enum
{
	KBL_MAIN=0,
	KBL_SHIFT,
	KBL_CTRL,
	KBL_ALT,
	KBL_CTRLSHIFT,
	KBL_ALTSHIFT,
	KBL_CTRLALT,
	KBL_CTRLALTSHIFT,

	KBL_GROUP_COUNT
};

const int KEY_COUNT = 12;

typedef wchar_t *KeyBarTitle;
typedef KeyBarTitle KeyBarTitleGroup [KEY_COUNT];

class KeyBar: public ScreenObject
{
	private:
		ScreenObject *Owner;

		KeyBarTitleGroup KeyTitles [KBL_GROUP_COUNT];
		int KeyCounts [KBL_GROUP_COUNT];

		int AltState,CtrlState,ShiftState;
		int DisableMask;

		KeyBarTitleGroup KeyTitlesCustom [KBL_GROUP_COUNT];
		bool RegReaded;

		string strLanguage;
		string strRegGroupName;

	private:
		virtual void DisplayObject();
		void ClearKeyTitles(bool Custom,int Group=-1);

	public:
		KeyBar();
		virtual  ~KeyBar();

	public:
		virtual int ProcessKey(int Key);
		virtual int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);

		void SetOwner(ScreenObject *Owner);

		void ReadRegGroup(const wchar_t *RegGroup, const wchar_t *Language);
		void SetRegGroup(int Group);
		void SetAllRegGroup();

		void SetGroup(int Group,const wchar_t * const *Key,int KeyCount);
		// Групповая установка идущих подряд строк LNG для указанной группы
		void SetAllGroup(int Group, LNGID StartIndex, int Count);

		void ClearGroup(int Group);

		void Set(const wchar_t * const *Key,int KeyCount)            { SetGroup(KBL_MAIN, Key, KeyCount); }
		void SetShift(const wchar_t * const *Key,int KeyCount)       { SetGroup(KBL_SHIFT, Key, KeyCount); }
		void SetAlt(const wchar_t * const *Key,int KeyCount)         { SetGroup(KBL_ALT, Key, KeyCount); }
		void SetCtrl(const wchar_t * const *Key,int KeyCount)        { SetGroup(KBL_CTRL, Key, KeyCount); }
		void SetCtrlShift(const wchar_t * const *Key,int KeyCount)   { SetGroup(KBL_CTRLSHIFT, Key, KeyCount); }
		void SetAltShift(const wchar_t * const *Key,int KeyCount)    { SetGroup(KBL_ALTSHIFT, Key, KeyCount); }
		void SetCtrlAlt(const wchar_t **Key,int KeyCount)            { SetGroup(KBL_CTRLALT, Key, KeyCount); }
		void SetCtrlAltShift(const wchar_t **Key,int KeyCount)       { SetGroup(KBL_CTRLALTSHIFT, Key, KeyCount); }

		void SetDisableMask(int Mask);
		void Change(const wchar_t *NewStr,int Pos)                   { Change(KBL_MAIN, NewStr, Pos); }

		// Изменение любого Label
		void Change(int Group,const wchar_t *NewStr,int Pos);

		size_t Change(const KeyBarTitles *);

		void RedrawIfChanged();
		virtual void ResizeConsole();
};
