#ifndef VMENU2_HPP_FA09A29A_2D5A_4F42_B16E_753DE520AB92
#define VMENU2_HPP_FA09A29A_2D5A_4F42_B16E_753DE520AB92
#pragma once

/*
vmenu2.hpp

Вертикальное меню
*/
/*
Copyright © 2012 Far Group
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
#include "dialog.hpp"

// Platform:

// Common:
#include "common/function_ref.hpp"
#include "common/range.hpp"

// External:

//----------------------------------------------------------------------------

struct menu_item;
struct MenuItemEx;
struct SortItemParam;

class VMenu2 : public Dialog
{
	struct private_tag {};

public:
	static vmenu2_ptr create(const string& Title, span<const menu_item> Data, int MaxHeight=0, DWORD Flags=0);

	VMenu2(private_tag, int MaxHeight);

	int GetTypeAndName(string &strType, string &strName) override;
	int GetType() const override { return windowtype_menu; }
	bool ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent) override;
	void SetPosition(rectangle Where) override;
	intptr_t SendMessage(intptr_t Msg,intptr_t Param1,void* Param2) override;

	void Resize(bool force=false);
	void SetTitle(const string& Title);
	void SetBottomTitle(const string& Title);
	void SetBoxType(int BoxType);
	void SetMenuFlags(DWORD Flags);
	void AssignHighlights(bool Reverse = false);
	void clear();
	int DeleteItem(int ID,int Count=1);
	int AddItem(const MenuItemEx& NewItem,int PosAdd = std::numeric_limits<int>::max());
	int AddItem(const FarList *NewItem);
	int AddItem(const string& NewStrItem);
	int FindItem(int StartIndex, const string& Pattern, unsigned long long Flags = 0);
	size_t size();
	bool empty() { return !size(); }
	int GetSelectPos();
	int SetSelectPos(int Pos, int Direct=0/*, bool stop_on_edge=false*/);
	wchar_t GetCheck(int Position=-1);
	void SetCheck(int Position=-1);
	void SetCustomCheck(wchar_t Char, int Position = -1);
	void ClearCheck(int Position = -1);
	void UpdateItemFlags(int Pos, unsigned long long NewFlags);
	void SetMaxHeight(int NewMaxHeight){MaxHeight=NewMaxHeight; Resize();}
	/*
		функция обработки меню должна возвращать true если она обработала событие и дальше ничего делать не надо
		(вне зависимости что говорит енц. о кодах возврата различных DN_*).
	*/
	intptr_t Run(const std::function<int(const Manager::Key& RawKey)>& fn=nullptr);
	intptr_t RunEx(const std::function<int(int Msg, void *param)>& fn);
	intptr_t GetExitCode();
	void Close(int ExitCode=-2, bool Force = false);

	intptr_t GetSimpleUserData(int Position = -1) const;

	const std::any* GetComplexUserData(int Position = -1) const;
	std::any* GetComplexUserData(int Position = -1);

	template<class T>
	const T* GetComplexUserDataPtr(intptr_t Position = -1) const
	{
		return std::any_cast<T>(GetComplexUserData(Position));
	}

	template<class T>
	T* GetComplexUserDataPtr(intptr_t Position = -1)
	{
		return std::any_cast<T>(GetComplexUserData(Position));
	}

	void Key(int key);
	int GetSelectPos(FarListPos* ListPos);
	int SetSelectPos(const FarListPos* ListPos, int Direct = 0);

	void SortItems(bool Reverse, int Offset);
	void SortItems(function_ref<bool(const MenuItemEx&, const MenuItemEx&, SortItemParam&)> Pred, bool Reverse = false, int Offset = 0);

	void Pack();
	MenuItemEx& at(size_t n);
	MenuItemEx& current();
	int GetShowItemCount();

private:
	intptr_t VMenu2DlgProc(Dialog* Dlg, intptr_t Msg, intptr_t Param1, void* Param2);
	int Call(int Msg, void *param);
	LISTITEMFLAGS GetItemFlags(int Position = -1);
	string GetMenuTitle(bool bottom = false);
	VMenu& ListBox() const { return *GetAllItem()[0].ListPtr; }

	enum class box_type
	{
		full,
		thin,
		none
	};

	box_type m_BoxType{ box_type::full };
	int MaxHeight;
	int cancel;
	int m_X1;
	int m_Y1;
	int m_X2;
	int m_Y2;
	INPUT_RECORD DefRec;
	int InsideCall;
	bool NeedResize;
	bool closing;
	bool ForceClosing;
	std::function<int(int Msg, void *param)> mfn;
};

#endif // VMENU2_HPP_FA09A29A_2D5A_4F42_B16E_753DE520AB92
