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
#include "interf.hpp"
#include "ctrlobj.hpp"

#include <functional>
using std::function;

class VMenu2 : public Dialog
{
	private:
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
		MACROMODEAREA MacroMode;

		function<int(int Msg, void *param)> mfn;

		static intptr_t WINAPI VMenu2DlgProc(HANDLE  hDlg, intptr_t Msg, intptr_t Param1, void* Param2);


		int Call(int Msg, void *param);
		void Redraw();
		void Resize(bool force=false);
		LISTITEMFLAGS GetItemFlags(int Position=-1);
		string GetTitles(int bottom=0);

		VMenu &ListBox()
		{
			return *GetAllItem()[0]->ListPtr;
		}

	public:
		VMenu2(const wchar_t *Title, MenuDataEx *Data, size_t ItemCount, int MaxHeight=0, DWORD Flags=0);

		void SetTitle(const wchar_t *Title);
		void SetBottomTitle(const wchar_t *Title);


		void SetBoxType(int BoxType)
		{
			ShortBox=(BoxType==SHORT_SINGLE_BOX || BoxType==SHORT_DOUBLE_BOX || NO_BOX);
			if(BoxType==NO_BOX)
				SetFlags(VMENU_SHOWNOBOX);
			if(BoxType==SINGLE_BOX || BoxType==SHORT_SINGLE_BOX)
				SetFlags(VMENU_LISTSINGLEBOX);
			Resize();
		}

		void SetFlags(DWORD Flags);


		void AssignHighlights(int Reverse)
		{
			SetFlags(Reverse ? VMENU_REVERSEHIGHLIGHT|VMENU_AUTOHIGHLIGHT : VMENU_AUTOHIGHLIGHT);
		}

		void DeleteItems();
		int DeleteItem(int ID,int Count=1);
		int AddItem(const MenuItemEx *NewItem,int PosAdd=0x7FFFFFFF);
		int AddItem(const FarList *NewItem);
		int AddItem(const wchar_t *NewStrItem);


		int FindItem(int StartIndex,const wchar_t *Pattern,UINT64 Flags=0);
		intptr_t GetItemCount();
		int GetSelectPos();

		int SetSelectPos(int Pos, int Direct=0/*, bool stop_on_edge=false*/);
		int GetCheck(int Position=-1);
		void SetCheck(int Check, int Position=-1);
		void UpdateItemFlags(int Pos, UINT64 NewFlags);


		void SetPosition(int X1,int Y1,int X2,int Y2);
		void SetMaxHeight(int NewMaxHeight){MaxHeight=NewMaxHeight;}

		/*
			функци€ обработки меню должна возвращать true если она обработала событие и дальше ничего делать не надо
			(вне зависимости что говорит енц. о кодах возврата различных DN_*).
		*/
		intptr_t Run(function<int(int Key)> fn=nullptr);
		intptr_t RunEx(function<int(int Msg, void *param)> fn);
		intptr_t GetExitCode();
		void Close(int ExitCode=-2);


		void *GetUserData(void *Data, size_t Size, intptr_t Position=-1);
		size_t SetUserData(LPCVOID Data, size_t Size=0, intptr_t Position=-1);
		void Key(int key);

		virtual MACROMODEAREA GetMacroMode()
		{
			return MacroMode;
		}
		void SetMacroMode(MACROMODEAREA mode)
		{
			MacroMode=mode;
		}




		int GetSelectPos(struct FarListPos *ListPos)
		{
			return ListBox().GetSelectPos(ListPos);
		}

		int SetSelectPos(struct FarListPos *ListPos, int Direct=0)
		{
			return ListBox().SetSelectPos(ListPos, Direct);
		}

		void SortItems(int Direction=0, int Offset=0)
		{
			ListBox().SortItems(Direction, Offset);
		}
		void SortItems(TMENUITEMEXCMPFUNC user_cmp_func,int Direction=0,int Offset=0)
		{
			ListBox().SortItems(user_cmp_func, Direction, Offset);
		}
		void Pack()
		{
			ListBox().Pack();
		}

		MenuItemEx *GetItemPtr(int Position=-1)
		{
			return ListBox().GetItemPtr(Position);
		}

		int GetShowItemCount()
		{
			return ListBox().GetShowItemCount();
		}

		virtual const wchar_t *GetTypeName() {return L"[VMenu]";};
		virtual int GetTypeAndName(string &strType, string &strName);
		virtual int GetType() { return MODALTYPE_VMENU; }
};
