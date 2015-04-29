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
#include "language.hpp"
#include "colormix.hpp"
#include "interf.hpp"
#include "syslog.hpp"
#include "constitle.hpp"
#include "strmix.hpp"

intptr_t VMenu2::VMenu2DlgProc(Dialog* Dlg, intptr_t Msg, intptr_t Param1, void* Param2)
{
	_DIALOG(CleverSysLog CL(L"VMenu2::VMenu2DlgProc()"));
	_DIALOG(SysLog(L"hDlg=%p, Msg=%s, Param1=%d (0x%08X), Param2=%d (0x%08X)",Dlg,_DLGMSG_ToName(Msg),Param1,Param1,Param2,Param2));
	switch(Msg)
	{
	case DN_CTLCOLORDIALOG:
		{
			FarColor *color=(FarColor*)Param2;
			*color=colors::PaletteColorToFarColor(COL_MENUBOX);
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
				colors->Colors[i]=colors::PaletteColorToFarColor(MenuColors[i]);

			return true;
		}

	case DN_CLOSE:
		if(!ForceClosing && !Param1 && GetItemFlags() & (LIF_GRAYED|LIF_DISABLE))
			return false;
		if(Call(Msg, (void*)(Param1<0 ? Param1 : GetSelectPos())))
			return false;
		break;

	case DN_LISTHOTKEY:
		if (!Call(Msg, Param2))
			Dlg->SendMessage( DM_CLOSE, -1, nullptr);
		break;

	case DN_DRAWDLGITEMDONE: //???
	case DN_DRAWDIALOGDONE:
		if(DefRec.EventType)
		{
			INPUT_RECORD rec=DefRec;
			ClearStruct(DefRec);
			if(!Call(DN_INPUT, &rec))
				Dlg->SendMessage( DM_KEY, 1, &rec);
		}
		break;

	case DN_INPUT:
		if (static_cast<INPUT_RECORD*>(Param2)->EventType==KEY_EVENT) break;
	case DN_CONTROLINPUT:
		if(!cancel)
		{
			if (Msg==DN_CONTROLINPUT)
			{
				auto ir = static_cast<INPUT_RECORD*>(Param2);
				int key=InputRecordToKey(ir);

				if(ListBox().ProcessFilterKey(key))
					return true;
			}

			if(Call(DN_INPUT, Param2))
				return Msg==DN_CONTROLINPUT;
		}
		break;

	case DN_LISTCHANGE:
	case DN_ENTERIDLE:
		if(!cancel)
		{
			if(Call(Msg, Param2))
				return false;
		}
		break;

	case DN_RESIZECONSOLE:
		if(!cancel)
		{
			INPUT_RECORD ReadRec={WINDOW_BUFFER_SIZE_EVENT};
			ReadRec.Event.WindowBufferSizeEvent.dwSize=*(COORD*)Param2;
			if(Call(DN_INPUT, &ReadRec))
				return false;
			else
				Resize();
		}
		break;

	default:
		if(Global->CloseFARMenu)
			ProcessKey(Manager::Key(KEY_ESC));
		break;
	}
	return Dlg->DefProc(Msg, Param1, Param2);
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

	SendMessage(DM_ENABLEREDRAW, 0, nullptr);
	int r=mfn(Msg, param);

	bool Visible;
	DWORD Size;
	SHORT X, Y;
	GetCursorType(Visible, Size);
	GetCursorPos(X, Y);

	if(SendMessage(DM_ENABLEREDRAW, -1, nullptr)<0)
		SendMessage(DM_ENABLEREDRAW, 1, nullptr);

	if(NeedResize)
		Resize();

	SetCursorType(Visible, Size);
	MoveCursor(X, Y);

	if(closing)
	{
		closing=false;
		if(Msg==DN_CLOSE)
			return false;
		SendMessage(DM_CLOSE, -1, nullptr);
	}

	return r;
}


void VMenu2::Resize(bool force)
{
	if(!force && (!ProcessEvents() || SendMessage(DM_ENABLEREDRAW, -1, nullptr)<0))
	{
		NeedResize=true;
		return;
	}
	NeedResize=false;

	FarListInfo info={sizeof(FarListInfo)};
	SendMessage(DM_LISTINFO, 0, &info);


	int X1 = m_X1;
	int Y1 = m_Y1;
	if(!ShortBox)
	{
		if(X1>1)
			X1-=2;
		if(Y1>0)
			Y1-=1;
	}


	int width=info.MaxLength+(ShortBox?2:6) + 3;
	if(m_X2>0)
		width=m_X2-X1+1;

	if(width>ScrX+1)
		width=ScrX+1;


	int height=GetShowItemCount();
	if(MaxHeight && height>MaxHeight)
		height=MaxHeight;

	height+=ShortBox?2:4;
	if(m_Y2>0)
		height=m_Y2-Y1+1;


	int mh=Y1<0 ? ScrY : ScrY-Y1;

	mh+=ShortBox ? 1 : 2;

	if(mh<0)
		mh=0;
	if(height>mh)
	{
		if(m_Y2<=0 && Y1>=ScrY/2)
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
	SendMessage(DM_RESIZEDIALOG, true, &size);

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
	SendMessage(DM_SETITEMPOSITION, 0, &ipos);

	COORD pos;
	pos.X=X;
	pos.Y=Y1;
	SendMessage(DM_MOVEDIALOG, true, &pos);
}

LISTITEMFLAGS VMenu2::GetItemFlags(int Position)
{
	if(Position<0)
		Position=GetSelectPos();

	FarListGetItem flgi={sizeof(FarListGetItem), Position};
	SendMessage(DM_LISTGETITEM, 0, &flgi);
	return flgi.Item.Flags;
}

string VMenu2::GetMenuTitle(bool bottom)
{
	wchar_t_ptr title;
	FarListTitles titles={sizeof(FarListTitles)};
	if(!SendMessage(DM_LISTGETTITLES, 0, &titles))
		return string();
	size_t size;
	if(bottom)
	{
		size=titles.BottomSize;
		title.reset(size);
		titles.Bottom = title.get();
	}
	else
	{
		size=titles.TitleSize;
		title.reset(size);
		titles.Title = title.get();
	}
	SendMessage(DM_LISTGETTITLES, 0, &titles);
	return string(title.get(), size - 1);
}

const std::array<FarDialogItem, 1> VMenu2DialogItems =
{{
	{DI_LISTBOX, 2, 1, 10, 10, 0, nullptr, nullptr, DIF_LISTNOAMPERSAND/*|DIF_LISTNOCLOSE*/, nullptr},
}};

VMenu2::VMenu2(int MaxHeight):
	Dialog(VMenu2DialogItems, this, &VMenu2::VMenu2DlgProc, nullptr),
	MaxHeight(MaxHeight),
	cancel(0),
	m_X1(-1),
	m_Y1(-1),
	m_X2(0),
	m_Y2(0),
	ShortBox(false),
	DefRec(),
	NeedResize(false),
	closing(false),
	ForceClosing(false)
{
}

vmenu2_ptr VMenu2::create(const string& Title, const MenuDataEx *Data, size_t ItemCount, int MaxHeight, DWORD Flags)
{
	vmenu2_ptr VMenu2Ptr(new VMenu2(MaxHeight));

	VMenu2Ptr->InitDialogObjects();
	VMenu2Ptr->SetMacroMode(MACROAREA_MENU);

	VMenu2Ptr->SetDialogMode(DMODE_KEEPCONSOLETITLE | DMODE_ISMENU);

	VMenu2Ptr->SetTitle(Title);
	VMenu2Ptr->SendMessage(DM_SETINPUTNOTIFY, 1, nullptr);

	std::vector<FarListItem> fli(ItemCount);
	std::transform(Data, Data + ItemCount, fli.begin(), [](const MenuDataEx& i)->FarListItem
	{
		FarListItem Item = {i.Flags, i.Name};
		return Item;
	});

	FarList fl={sizeof(FarList), ItemCount, fli.data()};

	VMenu2Ptr->SendMessage(DM_LISTSET, 0, &fl);

	for(size_t i=0; i<ItemCount; ++i)
		VMenu2Ptr->GetItemPtr(static_cast<int>(i))->AccelKey = Data[i].AccelKey;

	// BUGBUG
	VMenu2Ptr->Dialog::SetPosition(-1, -1, 20, 20);
	VMenu2Ptr->SetMenuFlags(Flags | VMENU_MOUSEREACTION);
	VMenu2Ptr->Resize();
	return VMenu2Ptr;
}

void VMenu2::SetTitle(const string& Title)
{
	FarListTitles titles={sizeof(FarListTitles)};
	string t=GetMenuTitle(true);
	titles.Bottom=t.data();
	titles.Title=Title.data();
	SendMessage(DM_LISTSETTITLES, 0, &titles);
}

void VMenu2::SetBottomTitle(const string& Title)
{
	FarListTitles titles={sizeof(FarListTitles)};
	string t=GetMenuTitle();
	titles.Bottom=Title.data();
	titles.Title=t.data();
	SendMessage(DM_LISTSETTITLES, 0, &titles);
}

void VMenu2::SetMenuFlags(DWORD Flags)
{
	FarDialogItem fdi;
	SendMessage(DM_GETDLGITEMSHORT, 0, &fdi);

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

	ListBox().SetMenuFlags(Flags&(VMENU_REVERSEHIGHLIGHT | VMENU_CHANGECONSOLETITLE | VMENU_LISTSINGLEBOX));

	SendMessage(DM_SETDLGITEMSHORT, 0, &fdi);
}

void VMenu2::DeleteItems()
{
	SendMessage(DM_LISTDELETE, 0, nullptr);
	Resize();
}

int VMenu2::DeleteItem(int ID, int Count)
{
	FarListDelete fld={sizeof(FarListDelete), ID, Count};
	SendMessage(DM_LISTDELETE, 0, &fld);
	Resize();
	return GetItemCount();
}

int VMenu2::AddItem(const MenuItemEx& NewItem, int PosAdd)
{
	// BUGBUG

	int n=GetItemCount();
	if(PosAdd<0)
		PosAdd=0;
	if(PosAdd>n)
		PosAdd=n;


	FarListItem fi={NewItem.Flags, NewItem.strName.data()};
	FarListInsert fli={sizeof(FarListInsert), PosAdd, fi};
	if(SendMessage(DM_LISTINSERT, 0, &fli)<0)
		return -1;

	FarListItemData flid={sizeof(FarListItemData), PosAdd, NewItem.UserDataSize, NewItem.UserData};
	SendMessage(DM_LISTSETDATA, 0, &flid);

	GetItemPtr(PosAdd)->AccelKey=NewItem.AccelKey;
	GetItemPtr(PosAdd)->Annotations=NewItem.Annotations;

	Resize();
	return n;
}
int VMenu2::AddItem(const FarList *NewItem)
{
	int r=SendMessage(DM_LISTADD, 0, const_cast<FarList*>(NewItem));
	Resize();
	return r;
}
int VMenu2::AddItem(const string& NewStrItem)
{
	int r=SendMessage(DM_LISTADDSTR, 0, UNSAFE_CSTR(NewStrItem));
	Resize();
	return r;
}

int VMenu2::FindItem(int StartIndex, const string& Pattern, UINT64 Flags)
{
	FarListFind flf={sizeof(FarListFind), StartIndex, Pattern.data(), Flags};
	return SendMessage(DM_LISTFINDSTRING, 0, &flf);
}

intptr_t VMenu2::GetItemCount()
{
	FarListInfo info={sizeof(FarListInfo)};
	SendMessage(DM_LISTINFO, 0, &info);
	return info.ItemsNumber;
}

int VMenu2::GetSelectPos()
{
	FarListPos flp={sizeof(FarListPos)};
	SendMessage(DM_LISTGETCURPOS, 0, &flp);
	return flp.SelectPos;
}

int VMenu2::SetSelectPos(int Pos, int Direct)
{
/*
	FarListPos flp={sizeof(FarListPos), Pos, -1};
	return SendMessage(DM_LISTSETCURPOS, 0, &flp); //!! DM_LISTSETCURPOS не работает для скрытого диалога
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
	SendMessage(DM_LISTGETITEM, 0, &flgi);
	flgi.Item.Flags=NewFlags;

	FarListUpdate flu={sizeof(FarListUpdate), Pos, flgi.Item};
	SendMessage(DM_LISTUPDATE, 0, &flu);
}

void VMenu2::SetPosition(int X1,int Y1,int X2,int Y2)
{
	if((X2>0 && X2<X1) || (Y2>0 && Y2<Y1))
		return;

	m_X1=X1;
	m_Y1=Y1;
	m_X2=X2;
	m_Y2=Y2;
	Resize();
}

intptr_t VMenu2::RunEx(const std::function<int(int Msg, void *param)>& fn)
{
	cancel=false;
	closing=false;
	mfn=fn;
	Resize(true);

	Process();

	return GetExitCode();
}

intptr_t VMenu2::Run(const std::function<int(const Manager::Key& RawKey)>& fn)
{
	if(!fn)
		return RunEx(nullptr);

	return RunEx([&](int Msg, void *param)->int
	{
		if(Msg==DN_INPUT)
		{
			auto ir = static_cast<INPUT_RECORD*>(param);
			return fn(Manager::Key(ir->EventType==WINDOW_BUFFER_SIZE_EVENT ? KEY_CONSOLE_BUFFER_RESIZE : InputRecordToKey(ir), *ir));
		}
		return fn(Manager::Key(KEY_NONE));
	});
}

intptr_t VMenu2::GetExitCode()
{
	if(cancel || Dialog::GetExitCode()<0)
		return -1;
	return GetSelectPos();
}

void VMenu2::Close(int ExitCode, bool Force)
{
	if(ExitCode>=0)
		SetSelectPos(ExitCode);
	cancel=ExitCode==-1;
	closing=true;
	ForceClosing = Force;
}

void *VMenu2::GetUserData(void *Data, size_t Size, intptr_t Position)
{
	if(Position<0)
		Position=GetSelectPos();
	void *d=(void*)SendMessage(DM_LISTGETDATA, 0, (void*)Position);
	size_t s=SendMessage(DM_LISTGETDATASIZE, 0, (void*)Position);

	if (d && Size && Data && Size>=s)
		memcpy(Data, d, s);

	return d;
}

size_t VMenu2::SetUserData(LPCVOID Data, size_t Size, intptr_t Position)
{
	if(Position<0)
		Position=GetSelectPos();

	FarListItemData flid={sizeof(FarListItemData), Position, Size, const_cast<void*>(Data)};
	return SendMessage(DM_LISTSETDATA, 0, &flid);
}

void VMenu2::Key(int key)
{
	INPUT_RECORD rec;
	KeyToInputRecord(key, &rec);
	if(ProcessEvents())
		SendMessage(DM_KEY, 1, &rec);
	else
		DefRec=rec;
}

int VMenu2::GetTypeAndName(string &strType, string &strName)
{
	strType = MSG(MVMenuType);
	strName = GetMenuTitle();
	return windowtype_menu;
}

static int ClickHandler(VMenu2* Menu, const IntOption& MenuClick)
{
	switch (MenuClick)
	{
	case  VMENUCLICK_APPLY:
		Menu->ProcessKey(Manager::Key(KEY_ENTER));
		break;
	case VMENUCLICK_CANCEL:
		Menu->ProcessKey(Manager::Key(KEY_ESC));
		break;
	case VMENUCLICK_IGNORE:
		break;
	}
	return TRUE;
}

int VMenu2::ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent)
{
	// BUGBUG
	// m_X1, m_X2, m_Y1, m_Y2 hides the same members from base class, fix it ASAP
	if (MouseEvent->dwMousePosition.X < Dialog::m_X1 || MouseEvent->dwMousePosition.Y < Dialog::m_Y1 ||
		MouseEvent->dwMousePosition.X > Dialog::m_X2 || MouseEvent->dwMousePosition.Y > Dialog::m_Y2)
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

void VMenu2::SetBoxType(int BoxType)
{
	ShortBox=(BoxType==SHORT_SINGLE_BOX || BoxType==SHORT_DOUBLE_BOX || BoxType==NO_BOX);
	if(BoxType==NO_BOX)
		SetMenuFlags(VMENU_SHOWNOBOX);
	if(BoxType==SINGLE_BOX || BoxType==SHORT_SINGLE_BOX)
		SetMenuFlags(VMENU_LISTSINGLEBOX);
	Resize();
}
