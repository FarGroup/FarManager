/*
vmenu2.cpp

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

#include "headers.hpp"
#pragma hdrstop

#include "vmenu2.hpp"
#include "keyboard.hpp"
#include "keys.hpp"
#include "config.hpp"

intptr_t WINAPI VMenu2::VMenu2DlgProc(HANDLE  hDlg, intptr_t Msg, intptr_t Param1, void* Param2)
{
	VMenu2 *vm=(VMenu2*)hDlg;

	switch(Msg)
	{
	case DN_CTLCOLORDIALOG:
		{
			FarColor *color=(FarColor*)Param2;
			*color=ColorIndexToColor(COL_MENUBOX);
			return true;
		}

	case DN_CTLCOLORDLGLIST:
		{
			FarDialogItemColors *colors=(FarDialogItemColors*)Param2;

			PaletteColors MenuColors[]=
			{
				COL_MENUBOX,                               // подложка
				COL_MENUBOX,                               // рамка
				COL_MENUTITLE,                             // заголовок - верхний и нижний
				COL_MENUTEXT,                              // Текст пункта
				COL_MENUHIGHLIGHT,                         // HotKey
				COL_MENUBOX,                               // separator
				COL_MENUSELECTEDTEXT,                      // Выбранный
				COL_MENUSELECTEDHIGHLIGHT,                 // Выбранный - HotKey
				COL_MENUSCROLLBAR,                         // ScrollBar
				COL_MENUDISABLEDTEXT,                      // Disabled
				COL_MENUARROWS,                            // Arrow
				COL_MENUARROWSSELECTED,                    // Выбранный - Arrow
				COL_MENUARROWSDISABLED,                    // Arrow Disabled
				COL_MENUGRAYTEXT,                          // "серый"
				COL_MENUSELECTEDGRAYTEXT,                  // выбранный "серый"
			};
			for(size_t i=0; i<colors->ColorsCount && i<ARRAYSIZE(MenuColors); ++i)
				colors->Colors[i]=ColorIndexToColor(MenuColors[i]);

			return true;
		}

	case DN_CLOSE:
		if(vm->GetItemFlags() & (LIF_GRAYED|LIF_DISABLE))
			return false;
		if(vm->Call(Msg, (void*)(Param1<0 ? Param1 : vm->GetSelectPos())))
			return false;
		break;

	case DN_LISTHOTKEY:
		if (!vm->Call(Msg, Param2))
			SendDlgMessage(hDlg, DM_CLOSE, -1, nullptr);
		break;

	case DN_DRAWDIALOGDONE:
		if(vm->DefRec.EventType)
		{
			INPUT_RECORD rec=vm->DefRec;
			ClearStruct(vm->DefRec);
			if(!vm->Call(DN_INPUT, &rec))
				SendDlgMessage(hDlg, DM_KEY, 1, &rec);
		}
		break;

	case DN_CONTROLINPUT:
	case DN_INPUT:
		if(!vm->cancel)
		{
			if (Msg==DN_CONTROLINPUT)
			{
				INPUT_RECORD *ir=static_cast<INPUT_RECORD*>(Param2);
				int key=InputRecordToKey(ir);

				if(vm->ListBox().ProcessFilterKey(key))
					return true;
			}

			if(vm->Call(DN_INPUT, Param2))
				return Msg==DN_CONTROLINPUT ? true : false;
		}
		break;

	case DN_LISTCHANGE:
	case DN_ENTERIDLE:
		if(!vm->cancel)
		{
			if(vm->Call(Msg, Param2))
				return false;
		}
		break;

	case DN_RESIZECONSOLE:
		if(!vm->cancel)
		{
			INPUT_RECORD ReadRec={WINDOW_BUFFER_SIZE_EVENT};
			ReadRec.Event.WindowBufferSizeEvent.dwSize=*(COORD*)Param2;
			if(vm->Call(DN_INPUT, &ReadRec))
				return false;
			else
				vm->Resize();
		}
		break;

	default:
		if(Global->CloseFARMenu)
			SendDlgMessage(hDlg, DM_CLOSE, -1, nullptr);
		break;
	}
	return DefDlgProc(hDlg, Msg, Param1, Param2);
}

/*
   VMenu2:Call() (т.е. функция обработки меню)
   должна возвращать true если она обработала событие и дальше ничего делать не надо
   (вне зависимости что говорит енц. о кодах возврата различных DN_*).
*/
int VMenu2::Call(int Msg, void *param)
{
	if(!mfn)
		return 0;

	SendDlgMessage(this, DM_ENABLEREDRAW, 0, nullptr);
	int r=mfn(Msg, param);

	bool Visible;
	DWORD Size;
	SHORT X;
	SHORT Y;
	GetCursorType(Visible, Size);
	GetCursorPos(X, Y);

	if(SendDlgMessage(this, DM_ENABLEREDRAW, -1, nullptr)<0)
		SendDlgMessage(this, DM_ENABLEREDRAW, 1, nullptr);

	if(NeedResize)
		Resize();

	SetCursorType(Visible, Size);
	MoveCursor(X, Y);

	if(closing)
	{
		closing=false;
		if(Msg==DN_CLOSE)
			return false;
		SendDlgMessage(this, DM_CLOSE, -1, nullptr);
	}

	return r;
}


void VMenu2::Resize(bool force)
{
	if(!force && (!ProcessEvents() || SendDlgMessage(this, DM_ENABLEREDRAW, -1, nullptr)<0))
	{
		NeedResize=true;
		return;
	}
	NeedResize=false;

	FarListInfo info={sizeof(FarListInfo)};
	SendDlgMessage(this, DM_LISTINFO, 0, &info);


	int X1=this->X1;
	int Y1=this->Y1;
	if(!ShortBox)
	{
		if(X1>1)
			X1-=2;
		if(Y1>0)
			Y1-=1;
	}


	int width=info.MaxLength+(ShortBox?2:6) + 3;
	if(X2>0)
		width=X2-X1+1;

	if(width>ScrX+1)
		width=ScrX+1;


	int height=GetShowItemCount();
	if(MaxHeight && height>MaxHeight)
		height=MaxHeight;

	height+=ShortBox?2:4;
	if(Y2>0)
		height=Y2-Y1+1;


	int mh=Y1<0 ? ScrY : ScrY-Y1;

	mh+=ShortBox ? 1 : 2;

	if(mh<0)
		mh=0;
	if(height>mh)
	{
	  if(Y2<=0 && Y1>=ScrY/2)
	  {
      Y1+=ShortBox?1:3;
	    if(height>Y1)
	      height=Y1;
	    Y1-=height;
	  }
	  else
		  height=mh;
	}

	int X=X1;
	if(X>0 && (X+width>ScrX))
		X-=X+width-ScrX;


	COORD size;
	size.X=width;
	size.Y=height;
	SendDlgMessage(this, DM_RESIZEDIALOG, true, &size);

	SMALL_RECT ipos;
	if(ShortBox)
	{
		ipos.Left=0;
		ipos.Top=0;
		ipos.Right=width-1;
		ipos.Bottom=height-1;
	}
	else
	{
		ipos.Left=2;
		ipos.Top=1;
		ipos.Right=width-3;
		ipos.Bottom=height-2;
	}
	SendDlgMessage(this, DM_SETITEMPOSITION, 0, &ipos);

	COORD pos;
	pos.X=X;
	pos.Y=Y1;
	SendDlgMessage(this, DM_MOVEDIALOG, true, &pos);
}

LISTITEMFLAGS VMenu2::GetItemFlags(int Position)
{
	if(Position<0)
		Position=GetSelectPos();

	FarListGetItem flgi={sizeof(FarListGetItem), Position};
	SendDlgMessage(this, DM_LISTGETITEM, 0, &flgi);
	return flgi.Item.Flags;
}

string VMenu2::GetTitles(int bottom)
{
	string title; size_t size=1;
	FarListTitles titles={sizeof(FarListTitles)};
	if(!SendDlgMessage(this, DM_LISTGETTITLES, 0, &titles))
		return title;
	if(bottom)
	{
		size=titles.BottomSize;
		titles.Bottom=title.GetBuffer(size);
	}
	else
	{
		size=titles.TitleSize;
		titles.Title=title.GetBuffer(size);
	}
	SendDlgMessage(this, DM_LISTGETTITLES, 0, &titles);
	title.ReleaseBuffer(size-1);
	return title;
}

FarDialogItem VMenu2DialogItems[]=
{
	{DI_LISTBOX, 2, 1, 10, 10, 0, nullptr, nullptr, DIF_LISTNOAMPERSAND/*|DIF_LISTNOCLOSE*/, nullptr},
};

VMenu2::VMenu2(const wchar_t *Title, MenuDataEx *Data, size_t ItemCount, int MaxHeight, DWORD Flags) : Dialog(VMenu2DialogItems, 1, VMenu2DlgProc, nullptr)
{
	InitDialogObjects();

	this->MaxHeight=MaxHeight;
	ShortBox=false;
	ClearStruct(DefRec);
	X1=-1;
	Y1=-1;
	X2=0;
	Y2=0;

	MacroMode=MACRO_MENU;

	SetDialogMode(DMODE_KEEPCONSOLETITLE|DMODE_ISMENU);


	SetTitle(Title);
	SendDlgMessage(this, DM_SETMOUSEEVENTNOTIFY, 1, nullptr);

	FarListItem *fli=new FarListItem[ItemCount];
	for(size_t i=0; i<ItemCount; ++i)
	{
		fli[i].Flags=Data[i].Flags;
		fli[i].Text=Data[i].Name;
	}
	FarList fl={sizeof(FarList), ItemCount, fli};

	SendDlgMessage(this, DM_LISTSET, 0, &fl);
	delete[] fli;

	for(size_t i=0; i<ItemCount; ++i)
		GetItemPtr(static_cast<int>(i))->AccelKey=Data[i].AccelKey;

	Dialog::SetPosition(-1, -1, 20, 20);
	SetFlags(Flags|VMENU_MOUSEREACTION);
	Resize();
}

void VMenu2::SetTitle(const wchar_t *Title)
{
	FarListTitles titles={sizeof(FarListTitles)};
	string t=GetTitles(1);
	titles.Bottom=t;
	titles.Title=Title;
	SendDlgMessage(this, DM_LISTSETTITLES, 0, &titles);
}
void VMenu2::SetBottomTitle(const wchar_t *Title)
{
	FarListTitles titles={sizeof(FarListTitles)};
	string t=GetTitles();
	titles.Bottom=Title;
	titles.Title=t;
	SendDlgMessage(this, DM_LISTSETTITLES, 0, &titles);
}

void VMenu2::SetFlags(DWORD Flags)
{
	FarDialogItem fdi;
	SendDlgMessage(this, DM_GETDLGITEMSHORT, 0, &fdi);

	if(Flags&VMENU_SHOWAMPERSAND)
		fdi.Flags&=~DIF_LISTNOAMPERSAND;
	if(Flags&VMENU_WRAPMODE)
		fdi.Flags|=DIF_LISTWRAPMODE;
	if(Flags&VMENU_MOUSEREACTION)
		fdi.Flags|=DIF_LISTTRACKMOUSE;
	if(Flags&VMENU_AUTOHIGHLIGHT)
		fdi.Flags|=DIF_LISTAUTOHIGHLIGHT;
	if(Flags&VMENU_SHOWNOBOX)
		fdi.Flags|=DIF_LISTNOBOX;

	ListBox().SetFlags(Flags&(VMENU_REVERSEHIGHLIGHT|VMENU_CHANGECONSOLETITLE|VMENU_LISTSINGLEBOX));

	SendDlgMessage(this, DM_SETDLGITEMSHORT, 0, &fdi);
}

void VMenu2::DeleteItems()
{
	SendDlgMessage(this, DM_LISTDELETE, 0, nullptr);
	Resize();
}

int VMenu2::DeleteItem(int ID, int Count)
{
	FarListDelete fld={sizeof(FarListDelete), ID, Count};
	SendDlgMessage(this, DM_LISTDELETE, 0, &fld);
	Resize();
	return GetItemCount();
}

int VMenu2::AddItem(const MenuItemEx *NewItem, int PosAdd)
{
	int n=GetItemCount();
	if(PosAdd<0)
		PosAdd=0;
	if(PosAdd>n)
		PosAdd=n;


	FarListItem fi={NewItem->Flags, NewItem->strName};
	FarListInsert fli={sizeof(FarListInsert), PosAdd, fi};
	if(SendDlgMessage(this, DM_LISTINSERT, 0, &fli)<0)
		return -1;

	FarListItemData flid={sizeof(FarListItemData), PosAdd, NewItem->UserDataSize, NewItem->UserData};
	SendDlgMessage(this, DM_LISTSETDATA, 0, &flid);

	GetItemPtr(PosAdd)->AccelKey=NewItem->AccelKey;

	Resize();
	return n;
}
int VMenu2::AddItem(const FarList *NewItem)
{
	int r=SendDlgMessage(this, DM_LISTADD, 0, (void*)NewItem);
	Resize();
	return r;
}
int VMenu2::AddItem(const wchar_t *NewStrItem)
{
	int r=SendDlgMessage(this, DM_LISTADDSTR, 0, (void*)NewStrItem);
	Resize();
	return r;
}

int VMenu2::FindItem(int StartIndex, const wchar_t *Pattern, UINT64 Flags)
{
	FarListFind flf={sizeof(FarListFind), StartIndex, Pattern, Flags};
	return SendDlgMessage(this, DM_LISTFINDSTRING, 0, &flf);
}

intptr_t VMenu2::GetItemCount()
{
	FarListInfo info={sizeof(FarListInfo)};
	SendDlgMessage(this, DM_LISTINFO, 0, &info);
	return info.ItemsNumber;
}

int VMenu2::GetSelectPos()
{
	FarListPos flp={sizeof(FarListPos)};
	SendDlgMessage(this, DM_LISTGETCURPOS, 0, &flp);
	return flp.SelectPos;
}

int VMenu2::SetSelectPos(int Pos, int Direct)
{
/*
	FarListPos flp={sizeof(FarListPos), Pos, -1};
	return SendDlgMessage(this, DM_LISTSETCURPOS, 0, &flp); //!! DM_LISTSETCURPOS не работает для скрытого диалога
*/
	return ListBox().SetSelectPos(Pos, Direct);
}

int VMenu2::GetCheck(int Position)
{
	LISTITEMFLAGS Flags=GetItemFlags(Position);

	if ((Flags & LIF_SEPARATOR) || !(Flags & LIF_CHECKED))
		return 0;

	int Checked = Flags & 0xFFFF;

	return Checked ? Checked : 1;
}

void VMenu2::SetCheck(int Check, int Position)
{
	LISTITEMFLAGS Flags=GetItemFlags(Position);
	if (Check)
	{
		Flags &= ~0xFFFF;
		Flags|=((Check&0xFFFF)|LIF_CHECKED);
	}
	else
		Flags&=~(0xFFFF|LIF_CHECKED);

	UpdateItemFlags(Position, Flags);
}

void VMenu2::UpdateItemFlags(int Pos, UINT64 NewFlags)
{
	if(Pos<0)
		Pos=GetSelectPos();
	FarListGetItem flgi={sizeof(FarListGetItem), Pos};
	SendDlgMessage(this, DM_LISTGETITEM, 0, &flgi);
	flgi.Item.Flags=NewFlags;

	FarListUpdate flu={sizeof(FarListUpdate), Pos, flgi.Item};
	SendDlgMessage(this, DM_LISTUPDATE, 0, &flu);
}

void VMenu2::SetPosition(int X1,int Y1,int X2,int Y2)
{
	if((X2>0 && X2<X1) || (Y2>0 && Y2<Y1))
		return;

	this->X1=X1;
	this->Y1=Y1;
	this->X2=X2;
	this->Y2=Y2;
	Resize();
}

intptr_t VMenu2::RunEx(function<int(int Msg, void *param)> fn)
{
	cancel=false;
	closing=false;
	mfn=fn;
	Resize(true);


	MACROMODEAREA PrevMacroMode=Global->CtrlObject->Macro.GetMode();
	Global->CtrlObject->Macro.SetMode(MacroMode);

	Process();

	Global->CtrlObject->Macro.SetMode(PrevMacroMode);

	return GetExitCode();
}

intptr_t VMenu2::Run(function<int(int Key)> fn)
{
	if(!fn)
		return RunEx(nullptr);

	return RunEx([&](int Msg, void *param)->int
	{
		int key=KEY_NONE;
		if(Msg==DN_INPUT)
		{
			INPUT_RECORD *ir=static_cast<INPUT_RECORD*>(param);
			key=ir->EventType==WINDOW_BUFFER_SIZE_EVENT ? KEY_CONSOLE_BUFFER_RESIZE : InputRecordToKey(ir);
		}
		return fn(key);
	});
}

intptr_t VMenu2::GetExitCode()
{
	if(cancel || Dialog::GetExitCode()<0)
		return -1;
	return GetSelectPos();
}

void VMenu2::Close(int ExitCode)
{
	if(ExitCode>=0)
		SetSelectPos(ExitCode);
	cancel=ExitCode==-1;
	closing=true;
}

void *VMenu2::GetUserData(void *Data, size_t Size, intptr_t Position)
{
	if(Position<0)
		Position=GetSelectPos();
	void *d=(void*)SendDlgMessage(this, DM_LISTGETDATA, 0, (void*)Position);
	size_t s=SendDlgMessage(this, DM_LISTGETDATASIZE, 0, (void*)Position);

	if (d && Size && Data && Size>=s)
		memcpy(Data, d, s);

	return d;
}

size_t VMenu2::SetUserData(LPCVOID Data, size_t Size, intptr_t Position)
{
	if(Position<0)
		Position=GetSelectPos();

	FarListItemData flid={sizeof(FarListItemData), Position, Size, (void*)Data};
	return SendDlgMessage(this, DM_LISTSETDATA, 0, &flid);
}

void VMenu2::Key(int key)
{
	INPUT_RECORD rec;
	KeyToInputRecord(key, &rec);
	if(ProcessEvents())
		SendDlgMessage(this, DM_KEY, 1, &rec);
	else
		DefRec=rec;
}

int VMenu2::GetTypeAndName(string &strType, string &strName)
{
	strType = MSG(MVMenuType);
	strName = GetTitles();
	return MODALTYPE_VMENU;
}

static int ClickHandler(VMenu2* Menu, const IntOption& MenuClick)
{
	switch (MenuClick)
	{
	case  VMENUCLICK_APPLY:
		Menu->ProcessKey(KEY_ENTER);
		break;
	case VMENUCLICK_CANCEL:
		Menu->ProcessKey(KEY_ESC);
		break;
	case VMENUCLICK_IGNORE:
		break;
	}
	return TRUE;
}

int VMenu2::ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent)
{
	if (MouseEvent->dwMousePosition.X < Dialog::X1 || MouseEvent->dwMousePosition.Y < Dialog::Y1 ||
		MouseEvent->dwMousePosition.X > Dialog::X2 || MouseEvent->dwMousePosition.Y > Dialog::Y2)
	{
		if (MouseEvent->dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED)
			return ClickHandler(this, Global->Opt->VMenu.LBtnClick);
		else if (MouseEvent->dwButtonState & FROM_LEFT_2ND_BUTTON_PRESSED)
			return ClickHandler(this, Global->Opt->VMenu.MBtnClick);
		else if (MouseEvent->dwButtonState & RIGHTMOST_BUTTON_PRESSED)
			return ClickHandler(this, Global->Opt->VMenu.RBtnClick);
	}

	return Dialog::ProcessMouse(MouseEvent);
}
