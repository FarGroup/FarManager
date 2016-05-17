#ifndef USERMENU_HPP_E234A9E0_BB8F_49CD_9C80_A1004107088D
#define USERMENU_HPP_E234A9E0_BB8F_49CD_9C80_A1004107088D
#pragma once

/*
usermenu.hpp

User menu и есть
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


class UserMenu: noncopyable
{
	struct UserMenuItem;

public:
	UserMenu(bool ChooseMenuType); //	true - выбор типа меню (основное или локальное), false - зависит от наличия FarMenu.Ini в текущем каталоге
	UserMenu(const string& MenuFileName);
	~UserMenu();

	typedef std::list<UserMenuItem> menu_container;

private:
	void ProcessUserMenu(bool ChooseMenuType, const string& MenuFileName);
	bool DeleteMenuRecord(menu_container& Menu, const menu_container::iterator& MenuItem);
	bool EditMenu(menu_container& Menu, menu_container::iterator* MenuItem, bool Create);
	int ProcessSingleMenu(menu_container& Menu, int MenuPos, menu_container& MenuRoot, const string& MenuFileName, const string& Title);
	void SaveMenu(const string& MenuFileName) const;
	intptr_t EditMenuDlgProc(Dialog* Dlg,intptr_t Msg,intptr_t Param1,void* Param2);

	enum class menu_mode: int;

	menu_mode m_MenuMode;
	bool m_MenuModified;
	bool m_ItemChanged;
	uintptr_t m_MenuCP;
	menu_container m_Menu;
};

#endif // USERMENU_HPP_E234A9E0_BB8F_49CD_9C80_A1004107088D
