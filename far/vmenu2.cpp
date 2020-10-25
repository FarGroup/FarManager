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

// BUGBUG
#include "platform.headers.hpp"

// Self:
#include "vmenu2.hpp"

// Internal:
#include "vmenu.hpp"
#include "keyboard.hpp"
#include "keys.hpp"
#include "config.hpp"
#include "lang.hpp"
#include "colormix.hpp"
#include "interf.hpp"
#include "syslog.hpp"
#include "strmix.hpp"
#include "global.hpp"

// Platform:

// Common:
#include "common/view/enumerate.hpp"

// External:

//----------------------------------------------------------------------------

void VMenu2::Pack()
{
	ListBox().Pack();
}

MenuItemEx& VMenu2::at(size_t n)
{
	return ListBox().at(n);
}

MenuItemEx& VMenu2::current()
{
	return ListBox().current();
}

int VMenu2::GetShowItemCount()
{
	return ListBox().GetShowItemCount();
}

intptr_t VMenu2::VMenu2DlgProc(Dialog* Dlg, intptr_t Msg, intptr_t Param1, void* Param2)
{
	_DIALOG(CleverSysLog CL(L"VMenu2::VMenu2DlgProc()"));
	_DIALOG(SysLog(L"hDlg=%p, Msg=%s, Param1=%d (0x%08X), Param2=%d (0x%08X)",Dlg,_DLGMSG_ToName(Msg),Param1,Param1,Param2,Param2));
	switch(Msg)
	{
	case DN_CTLCOLORDIALOG:
		*static_cast<FarColor*>(Param2) = colors::PaletteColorToFarColor(COL_MENUBOX);
		return true;

	case DN_CTLCOLORDLGLIST:
		{
			static const PaletteColors MenuColors[]=
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

			const auto colors = static_cast<FarDialogItemColors*>(Param2);
			std::transform(MenuColors, MenuColors + std::min(colors->ColorsCount, std::size(MenuColors)), colors->Colors, &colors::PaletteColorToFarColor);
			return true;
		}

	case DN_CLOSE:
		if(!ForceClosing && !Param1 && GetItemFlags() & (LIF_GRAYED|LIF_DISABLE))
			return false;
		if(Call(Msg, reinterpret_cast<void*>(Param1 < 0? Param1 : GetSelectPos())))
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
			DefRec = {};
			if(!Call(DN_INPUT, &rec))
				Dlg->SendMessage( DM_KEY, 1, &rec);
		}
		break;

	case DN_INPUT:
		if (static_cast<INPUT_RECORD*>(Param2)->EventType==KEY_EVENT)
			break;
		[[fallthrough]];
	case DN_CONTROLINPUT:
		if(!cancel)
		{
			if (Msg==DN_CONTROLINPUT && ListBox().ProcessFilterKey(InputRecordToKey(static_cast<const INPUT_RECORD*>(Param2))))
			{
				return true;
			}

			if(Call(DN_INPUT, Param2))
				return Msg==DN_CONTROLINPUT;
		}
		break;

	case DN_LISTCHANGE:
		if (Dlg->CheckDialogMode(DMODE_ISMENU))
			break;
		[[fallthrough]];
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
			ReadRec.Event.WindowBufferSizeEvent.dwSize = *static_cast<COORD*>(Param2);
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

	++InsideCall;
	const auto r = mfn(Msg, param);

	bool Visible;
	size_t Size;

	GetCursorType(Visible, Size);
	const auto CursorPos = GetCursorPos();

	--InsideCall;

	if(NeedResize)
		Resize();


	SetCursorType(Visible, Size);
	MoveCursor(CursorPos);

	if(InsideCall==0 && ListBox().UpdateRequired())
		SendMessage(DM_REDRAW, 0, nullptr);

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
	if(!force && (!ProcessEvents() || InsideCall>0))
	{
		NeedResize=true;
		return;
	}
	NeedResize=false;

	FarListInfo info={sizeof(FarListInfo)};
	SendMessage(DM_LISTINFO, 0, &info);


	int X1 = m_X1;
	int Y1 = m_Y1;
	if(m_BoxType == box_type::full)
	{
		if(X1>1)
			X1-=2;
		if(Y1>0)
			Y1-=1;
	}


	int width = info.MaxLength + (m_BoxType == box_type::none? 0 : m_BoxType == box_type::thin? 2 : 6) + 3;
	if(m_X2>0)
		width=m_X2-X1+1;

	if(width>ScrX+1)
		width=ScrX+1;


	int height=GetShowItemCount();
	if(MaxHeight && height>MaxHeight)
		height=MaxHeight;

	height += m_BoxType == box_type::none? 0 : m_BoxType == box_type::thin? 2 : 4;
	if(m_Y2>0)
		height=m_Y2-Y1+1;

	int mh=Y1<0 ? ScrY : ScrY-Y1;

	mh += m_BoxType == box_type::none? 0 : m_BoxType == box_type::thin? 1 : 2;

	if(mh<0)
		mh=0;
	if(height>mh)
	{
		if(m_Y2<=0 && Y1>=ScrY/2)
		{
			Y1 += m_BoxType == box_type::none? 0 : m_BoxType == box_type::thin? 1 : 3;
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
	if (m_BoxType == box_type::full)
	{
		ipos.Left = 2;
		ipos.Top = 1;
		ipos.Right = width - 3;
		ipos.Bottom = height - 2;
	}
	else
	{
		ipos.Left = 0;
		ipos.Top = 0;
		ipos.Right = width - 1;
		ipos.Bottom = height - 1;
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
	wchar_t_ptr_n<256> title;
	FarListTitles titles={sizeof(FarListTitles)};
	if(!SendMessage(DM_LISTGETTITLES, 0, &titles))
		return {};
	size_t size;
	if(bottom)
	{
		size=titles.BottomSize;
		title.reset(size);
		titles.Bottom = title.data();
	}
	else
	{
		size=titles.TitleSize;
		title.reset(size);
		titles.Title = title.data();
	}
	SendMessage(DM_LISTGETTITLES, 0, &titles);
	return { title.data(), size - 1 };
}

static const FarDialogItem VMenu2DialogItems[]
{
	{ DI_LISTBOX, 2, 1, 10, 10, {}, {}, {}, DIF_LISTNOAMPERSAND/* | DIF_LISTNOCLOSE*/, {} },
};

VMenu2::VMenu2(private_tag, int MaxHeight):
	Dialog(Dialog::private_tag(), span(VMenu2DialogItems), [this](Dialog* Dlg, intptr_t Msg, intptr_t Param1, void* Param2) { return VMenu2DlgProc(Dlg, Msg, Param1, Param2); }, nullptr),
	MaxHeight(MaxHeight),
	cancel(0),
	m_X1(-1),
	m_Y1(-1),
	m_X2(0),
	m_Y2(0),
	DefRec(),
	InsideCall(0),
	NeedResize(false),
	closing(false),
	ForceClosing(false)
{
}

vmenu2_ptr VMenu2::create(const string& Title, span<const menu_item> const Data, int MaxHeight, DWORD Flags)
{
	auto VMenu2Ptr = std::make_shared<VMenu2>(private_tag(), MaxHeight);

	VMenu2Ptr->InitDialogObjects();
	VMenu2Ptr->SetMacroMode(MACROAREA_MENU);

	VMenu2Ptr->SetDialogMode(DMODE_ISMENU | (Flags & VMENU_CHANGECONSOLETITLE? DMODE_NONE : DMODE_KEEPCONSOLETITLE));

	VMenu2Ptr->SetTitle(Title);
	VMenu2Ptr->SendMessage(DM_SETINPUTNOTIFY, 1, nullptr);

	std::vector<FarListItem> fli;
	fli.reserve(Data.size());
	std::transform(ALL_CONST_RANGE(Data), std::back_inserter(fli), [](const auto& i) { return FarListItem{ i.Flags, i.Name.c_str() }; });

	FarList fl = { sizeof(FarList), fli.size(), fli.data() };

	VMenu2Ptr->SendMessage(DM_LISTSET, 0, &fl);

	for (const auto& [Item, Index]: enumerate(Data))
		VMenu2Ptr->at(Index).AccelKey = Item.AccelKey;

	// BUGBUG
	VMenu2Ptr->Dialog::SetPosition({ -1, -1, 20, 20 });
	VMenu2Ptr->SetMenuFlags(Flags | VMENU_MOUSEREACTION);
	VMenu2Ptr->Resize();
	return VMenu2Ptr;
}

void VMenu2::SetTitle(const string& Title)
{
	FarListTitles titles={sizeof(FarListTitles)};
	const auto t = GetMenuTitle(true);
	titles.Bottom=t.c_str();
	titles.Title=Title.c_str();
	SendMessage(DM_LISTSETTITLES, 0, &titles);
}

void VMenu2::SetBottomTitle(const string& Title)
{
	FarListTitles titles={sizeof(FarListTitles)};
	const auto t = GetMenuTitle();
	titles.Bottom=Title.c_str();
	titles.Title=t.c_str();
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
	if (Flags&VMENU_NOMERGEBORDER)
		fdi.Flags|=DIF_LISTNOMERGEBORDER;

	ListBox().SetMenuFlags(Flags & (VMENU_REVERSEHIGHLIGHT | VMENU_LISTSINGLEBOX));

	SendMessage(DM_SETDLGITEMSHORT, 0, &fdi);
}

void VMenu2::AssignHighlights(bool Reverse)
{
	SetMenuFlags(VMENU_AUTOHIGHLIGHT | (Reverse? VMENU_REVERSEHIGHLIGHT : VMENU_NONE));
}

void VMenu2::clear()
{
	SendMessage(DM_LISTDELETE, 0, nullptr);
	Resize();
}

int VMenu2::DeleteItem(int ID, int Count)
{
	FarListDelete fld={sizeof(FarListDelete), ID, Count};
	SendMessage(DM_LISTDELETE, 0, &fld);
	Resize();
	return static_cast<int>(size());
}

int VMenu2::AddItem(const MenuItemEx& NewItem, int PosAdd)
{
	// BUGBUG

	const auto n = static_cast<int>(size());
	if(PosAdd<0)
		PosAdd=0;
	if(PosAdd>n)
		PosAdd=n;


	FarListInsert fli{ sizeof(FarListInsert), PosAdd, { NewItem.Flags, NewItem.Name.c_str(), NewItem.SimpleUserData } };
	if(SendMessage(DM_LISTINSERT, 0, &fli)<0)
		return -1;

	ListBox().SetComplexUserData(NewItem.ComplexUserData, PosAdd);

	auto& Item = at(PosAdd);
	Item.AccelKey=NewItem.AccelKey;
	Item.Annotations = NewItem.Annotations;

	Resize();
	return n;
}
int VMenu2::AddItem(const FarList *NewItem)
{
	const auto r = SendMessage(DM_LISTADD, 0, const_cast<FarList*>(NewItem));
	Resize();
	return r;
}
int VMenu2::AddItem(const string& NewStrItem)
{
	const auto r = SendMessage(DM_LISTADDSTR, 0, UNSAFE_CSTR(NewStrItem));
	Resize();
	return r;
}

int VMenu2::FindItem(int StartIndex, const string& Pattern, unsigned long long Flags)
{
	FarListFind flf{sizeof(FarListFind), StartIndex, Pattern.c_str(), Flags};
	return SendMessage(DM_LISTFINDSTRING, 0, &flf);
}

size_t VMenu2::size()
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

wchar_t VMenu2::GetCheck(int Position)
{
	const auto Flags = GetItemFlags(Position);

	if ((Flags & LIF_SEPARATOR) || !(Flags & LIF_CHECKED))
		return 0;

	const auto Checked = Flags & std::numeric_limits<wchar_t>::max();

	return Checked ? Checked : 1;
}

void VMenu2::SetCheck(int Position)
{
	const auto Flags = GetItemFlags(Position) & ~std::numeric_limits<wchar_t>::max();
	UpdateItemFlags(Position, Flags | LIF_CHECKED);
}

void VMenu2::SetCustomCheck(wchar_t Char, int Position)
{
	const auto Flags = GetItemFlags(Position) & ~std::numeric_limits<wchar_t>::max();
	UpdateItemFlags(Position, Flags | LIF_CHECKED | Char);
}

void VMenu2::ClearCheck(int Position)
{
	const auto Flags = GetItemFlags(Position) & ~std::numeric_limits<wchar_t>::max();
	UpdateItemFlags(Position, Flags & ~LIF_CHECKED);
}

void VMenu2::UpdateItemFlags(int Pos, unsigned long long NewFlags)
{
	if(Pos<0)
		Pos=GetSelectPos();
	FarListGetItem flgi={sizeof(FarListGetItem), Pos};
	SendMessage(DM_LISTGETITEM, 0, &flgi);
	flgi.Item.Flags=NewFlags;

	FarListUpdate flu={sizeof(FarListUpdate), Pos, flgi.Item};
	SendMessage(DM_LISTUPDATE, 0, &flu);
}

void VMenu2::SetPosition(rectangle Where)
{
	if((Where.right > 0 && Where.right < Where.left) || (Where.bottom > 0 && Where.bottom < Where.top))
		return;

	m_X1 = Where.left;
	m_Y1 = Where.top;
	m_X2 = Where.right;
	m_Y2 = Where.bottom;
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

	return RunEx([&](int Msg, void *param)
	{
		const auto Key =
			Msg == DN_INPUT?
			Manager::Key(static_cast<const INPUT_RECORD*>(param)->EventType == WINDOW_BUFFER_SIZE_EVENT? KEY_CONSOLE_BUFFER_RESIZE : InputRecordToKey(static_cast<const INPUT_RECORD*>(param)), *static_cast<const INPUT_RECORD*>(param)) :
			Manager::Key(KEY_NONE);

		return fn(Key);
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

intptr_t VMenu2::GetSimpleUserData(int Position) const
{
	return ListBox().GetSimpleUserData(Position);
}

const std::any* VMenu2::GetComplexUserData(int Position) const
{
	return ListBox().GetComplexUserData(Position);
}

std::any* VMenu2::GetComplexUserData(int Position)
{
	return ListBox().GetComplexUserData(Position);
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

int VMenu2::GetSelectPos(FarListPos* ListPos)
{
	return ListBox().GetSelectPos(ListPos);
}

int VMenu2::SetSelectPos(const FarListPos* ListPos, int Direct)
{
	return ListBox().SetSelectPos(ListPos, Direct);
}

void VMenu2::SortItems(bool Reverse, int Offset)
{
	ListBox().SortItems(Reverse, Offset);
}

void VMenu2::SortItems(function_ref<bool(const MenuItemEx&, const MenuItemEx&, SortItemParam&)> const Pred, bool Reverse, int Offset)
{
	ListBox().SortItems(Pred, Reverse, Offset);
}

int VMenu2::GetTypeAndName(string &strType, string &strName)
{
	strType = msg(lng::MVMenuType);
	strName = GetMenuTitle();
	return windowtype_menu;
}

static auto ClickHandler(VMenu2* Menu, const IntOption& MenuClick)
{
	switch (MenuClick)
	{
	case  VMENUCLICK_APPLY:
		return Menu->ProcessKey(Manager::Key(KEY_ENTER));

	case VMENUCLICK_CANCEL:
		return Menu->ProcessKey(Manager::Key(KEY_ESC));

	case VMENUCLICK_IGNORE:
	default:
		return true;
	}
}

bool VMenu2::ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent)
{
	if (!IsMoving())
	{
		if (!m_Where.contains(MouseEvent->dwMousePosition))
		{
			if (MouseEvent->dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED)
				return ClickHandler(this, Global->Opt->VMenu.LBtnClick);
			else if (MouseEvent->dwButtonState & FROM_LEFT_2ND_BUTTON_PRESSED)
				return ClickHandler(this, Global->Opt->VMenu.MBtnClick);
			else if (MouseEvent->dwButtonState & RIGHTMOST_BUTTON_PRESSED)
				return ClickHandler(this, Global->Opt->VMenu.RBtnClick);
		}
	}
	return Dialog::ProcessMouse(MouseEvent);
}

void VMenu2::SetBoxType(int BoxType)
{
	if (BoxType == NO_BOX)
	{
		m_BoxType = box_type::none;
		SetMenuFlags(VMENU_SHOWNOBOX);
	}
	else if (BoxType == SHORT_SINGLE_BOX || BoxType == SHORT_DOUBLE_BOX)
	{
		m_BoxType = box_type::thin;
	}
	else
	{
		m_BoxType = box_type::full;
	}

	if (BoxType == SINGLE_BOX || BoxType == SHORT_SINGLE_BOX)
		SetMenuFlags(VMENU_LISTSINGLEBOX);

	Resize();
}

intptr_t VMenu2::SendMessage(intptr_t Msg,intptr_t Param1,void* Param2)
{
	switch (Msg)
	{
		case DM_RESIZEDIALOG:
		{
			const auto fixSize = [](short& Size, short const Min) { Size = std::max(Min, Size); };
			const auto MarginsX = (m_BoxType == box_type::none? 0 : m_BoxType == box_type::thin? 1 : 3) * 2;
			const auto MarginsY = (m_BoxType == box_type::none? 0 : m_BoxType == box_type::thin? 1 : 2) * 2;
			fixSize(static_cast<COORD*>(Param2)->X, MarginsX + 1);
			fixSize(static_cast<COORD*>(Param2)->Y, MarginsY + (GetShowItemCount()? 1 : 0));
			break;
		}
	}
	return Dialog::SendMessage(Msg,Param1,Param2);
}
