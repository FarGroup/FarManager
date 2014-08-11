#pragma once

/*
vmenu2.hpp

¬ертикальное меню
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

#include "dialog.hpp"

class VMenu2 : public Dialog
{
public:
	VMenu2(const string& Title, const MenuDataEx *Data, size_t ItemCount, int MaxHeight=0, DWORD Flags=0);

	virtual const wchar_t *GetTypeName() override { return L"[VMenu]"; }
	virtual int GetTypeAndName(string &strType, string &strName) override;
	virtual int GetType() const override { return MODALTYPE_VMENU; }
	virtual int ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent) override;
	virtual void SetPosition(int X1, int Y1, int X2, int Y2) override;

	void Resize(bool force=false);
	void SetTitle(const string& Title);
	void SetBottomTitle(const string& Title);
	void SetBoxType(int BoxType);
	void SetFlags(DWORD Flags);
	void AssignHighlights(int Reverse) { SetFlags(Reverse ? VMENU_REVERSEHIGHLIGHT|VMENU_AUTOHIGHLIGHT : VMENU_AUTOHIGHLIGHT); }
	void DeleteItems();
	int DeleteItem(int ID,int Count=1);
	int AddItem(const MenuItemEx& NewItem,int PosAdd=0x7FFFFFFF);
	int AddItem(const FarList *NewItem);
	int AddItem(const string& NewStrItem);
	int FindItem(int StartIndex,const string& Pattern,UINT64 Flags=0);
	intptr_t GetItemCount();
	int GetSelectPos();
	int SetSelectPos(int Pos, int Direct=0/*, bool stop_on_edge=false*/);
	int GetCheck(int Position=-1);
	void SetCheck(int Check, int Position=-1);
	void UpdateItemFlags(int Pos, UINT64 NewFlags);
	void SetMaxHeight(int NewMaxHeight){MaxHeight=NewMaxHeight; Resize();}
	/*
		функци€ обработки меню должна возвращать true если она обработала событие и дальше ничего делать не надо
		(вне зависимости что говорит енц. о кодах возврата различных DN_*).
	*/
	intptr_t Run(const std::function<int(int Key)>& fn=nullptr);
	intptr_t RunEx(const std::function<int(int Msg, void *param)>& fn);
	intptr_t GetExitCode();
	void Close(int ExitCode=-2, bool Force = false);
	void *GetUserData(void *Data, size_t Size, intptr_t Position=-1);
	size_t SetUserData(LPCVOID Data, size_t Size=0, intptr_t Position=-1);
	void Key(int key);
	int GetSelectPos(FarListPos *ListPos) { return ListBox().GetSelectPos(ListPos); }
	int SetSelectPos(const FarListPos *ListPos, int Direct=0) { return ListBox().SetSelectPos(ListPos, Direct); }
	void SortItems(bool Reverse = false, int Offset = 0) { ListBox().SortItems(Reverse, Offset); }
	template<class predicate>
	void SortItems(predicate Pred, bool Reverse = false, int Offset = 0) { ListBox().SortItems(Pred, Reverse, Offset); }
	void Pack() { ListBox().Pack(); }
	MenuItemEx *GetItemPtr(int Position=-1) { return ListBox().GetItemPtr(Position); }
	int GetShowItemCount() { return ListBox().GetShowItemCount(); }

private:
	intptr_t VMenu2DlgProc(Dialog* Dlg, intptr_t Msg, intptr_t Param1, void* Param2);
	int Call(int Msg, void *param);
	LISTITEMFLAGS GetItemFlags(int Position = -1);
	string GetMenuTitle(bool bottom = false);
	VMenu &ListBox() { return *GetAllItem()[0].ListPtr; }

	int MaxHeight;
	int cancel;
	int X1;
	int Y1;
	int X2;
	int Y2;
	bool ShortBox;
	INPUT_RECORD DefRec;
	bool NeedResize;
	bool closing;
	bool ForceClosing;
	std::function<int(int Msg, void *param)> mfn;
};
