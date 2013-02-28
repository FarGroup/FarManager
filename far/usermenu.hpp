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


struct UserMenuItem {
	string strHotKey;
	string strLabel;
	std::list<string> Commands;
	bool Submenu;
	std::list<UserMenuItem> *Menu;

	UserMenuItem() { Submenu=false; Menu=nullptr; }
	~UserMenuItem() { if (Menu) delete Menu; }
};

ENUM(MENUMODE);

class UserMenu
{
public:
	UserMenu(bool ChoiceMenuType); //	true - выбор типа меню (основное или локальное), false - зависит от наличия FarMenu.Ini в текущем каталоге
	UserMenu(const wchar_t *MenuFileName);
	~UserMenu();

private:
	void ProcessUserMenu(bool ChoiceMenuType,const wchar_t *MenuFileName=nullptr);
	bool DeleteMenuRecord(std::list<UserMenuItem>& Menu, std::list<UserMenuItem>::iterator& MenuItem);
	bool EditMenu(std::list<UserMenuItem>& Menu, std::list<UserMenuItem>::iterator* MenuItem, bool Create);
	int ProcessSingleMenu(std::list<UserMenuItem>& Menu, int MenuPos, std::list<UserMenuItem>& MenuRoot, const string& MenuFileName, const wchar_t *Title=nullptr);
	void SaveMenu(const string& MenuFileName);
	intptr_t EditMenuDlgProc(Dialog* Dlg,intptr_t Msg,intptr_t Param1,void* Param2);

	MENUMODE MenuMode;
	bool MenuModified;
	bool MenuNeedRefresh;
	bool ItemChanged;
	std::list<UserMenuItem> Menu;

};
