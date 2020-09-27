#ifndef HMENU_HPP_4850D545_465D_434F_B439_23DF6445EEFC
#define HMENU_HPP_4850D545_465D_434F_B439_23DF6445EEFC
#pragma once

/*
hmenu.hpp

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

// Internal:
#include "modal.hpp"

// Platform:

// Common:
#include "common/range.hpp"

// External:

//----------------------------------------------------------------------------

struct menu_item;

struct HMenuData
{
	string_view Name;
	string_view SubMenuHelp;
	span<menu_item> SubMenu;
	bool Selected;
	int XPos;
};

class VMenu2;

class HMenu: public SimpleModal
{
	struct private_tag {};

public:
	static hmenu_ptr create(HMenuData* Item, size_t ItemCount);
	HMenu(private_tag, HMenuData* Item, size_t ItemCount);
	~HMenu() override;

	bool ProcessKey(const Manager::Key& Key) override;
	bool ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent) override;
	long long VMProcess(int OpCode, void* vParam = nullptr, long long iParam = 0) override;
	void ResizeConsole() override;
	int GetType() const override { return windowtype_hmenu; }
	int GetTypeAndName(string &, string &) override { return windowtype_hmenu; }

	void GetExitCode(int &ExitCode, int &VExitCode) const;

private:
	void init();

	void DisplayObject() override;
	string GetTitle() const override { return {}; }

	void ShowMenu();
	bool ProcessCurrentSubMenu();
	bool ProcessPositioningKey(unsigned LocalKey);
	static wchar_t GetHighlights(const HMenuData& Item);
	size_t CheckHighlights(WORD CheckSymbol, int StartPos = 0) const;
	bool TestMouse(const MOUSE_EVENT_RECORD *MouseEvent) const;
	void UpdateSelectPos();

	span<HMenuData> m_Item;
	size_t m_SelectPos{};
	int m_VExitCode{-1};
	bool m_SubmenuOpened{};
};

#endif // HMENU_HPP_4850D545_465D_434F_B439_23DF6445EEFC
