#ifndef KEYBAR_HPP_8575C258_EBCC_4620_8657_6C56564AD9DE
#define KEYBAR_HPP_8575C258_EBCC_4620_8657_6C56564AD9DE
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

// Internal:
#include "scrobj.hpp"

// Platform:

// Common:

// External:

//----------------------------------------------------------------------------

struct KeyBarTitles;
enum class lng : int;

//   Группы меток
enum keybar_group: size_t
{
	// порядок соответствует .lng файлу
	KBL_MAIN,
	KBL_SHIFT,
	KBL_ALT,
	KBL_CTRL,
	KBL_ALTSHIFT,
	KBL_CTRLSHIFT,
	KBL_CTRLALT,
	KBL_CTRLALTSHIFT,

	KBL_GROUP_COUNT
};

enum fkeys: size_t
{
	F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12
};

enum KEYBARAREA
{
	KBA_SHELL,
	KBA_INFO,
	KBA_TREE,
	KBA_QUICKVIEW,
	KBA_FOLDERTREE,
	KBA_EDITOR,
	KBA_VIEWER,
	KBA_HELP,

	KBA_COUNT
};

class KeyBar final: public SimpleScreenObject
{
public:
	explicit KeyBar(window_ptr Owner);

	bool ProcessKey(const Manager::Key& Key) override;
	bool ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent) override;

	void SetLabels(lng StartIndex);
	void SetCustomLabels(KEYBARAREA Area);

	using keybar_item = std::pair<string, string>;

	class keybar_area
	{
	public:
		explicit keybar_area(std::vector<keybar_item>* Items): m_Items(Items) {}
		string& operator[](fkeys Key) const { return (*m_Items)[Key].first; }

	private:
		std::vector<keybar_item>* m_Items;
	};

	auto operator[](keybar_group Group) { return keybar_area(&Items[Group]); }
	size_t Change(const KeyBarTitles* Kbt);

	void RedrawIfChanged();

private:
	void DisplayObject() override;

	void ClearKeyTitles(bool Custom);

	// title, custom title
	std::vector<std::vector<keybar_item>> Items;
	string strLanguage;
	KEYBARAREA CustomArea;
	bool AltState, CtrlState, ShiftState;
	bool CustomLabelsReaded;
};

#endif // KEYBAR_HPP_8575C258_EBCC_4620_8657_6C56564AD9DE
