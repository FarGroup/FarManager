/*
vmenu.cpp

Обычное вертикальное меню
  а так же:
    * список в DI_COMBOBOX
    * список в DI_LISTBOX
    * ...
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

// BUGBUG
#include "platform.headers.hpp"

// Self:
#include "vmenu.hpp"

// Internal:
#include "keyboard.hpp"
#include "keys.hpp"
#include "macroopcode.hpp"
#include "farcolor.hpp"
#include "dialog.hpp"
#include "savescr.hpp"
#include "clipboard.hpp"
#include "ctrlobj.hpp"
#include "manager.hpp"
#include "constitle.hpp"
#include "interf.hpp"
#include "colormix.hpp"
#include "config.hpp"
#include "processname.hpp"
#include "uuids.far.hpp"
#include "xlat.hpp"
#include "lang.hpp"
#include "vmenu2.hpp"
#include "strmix.hpp"
#include "string_sort.hpp"
#include "exception.hpp"
#include "global.hpp"

// Platform:

// Common:
#include "common.hpp"
#include "common/function_traits.hpp"
#include "common/scope_exit.hpp"
#include "common/uuid.hpp"
#include "common/view/enumerate.hpp"
#include "common/view/zip.hpp"

// External:

//----------------------------------------------------------------------------

static MenuItemEx FarList2MenuItem(const FarListItem& FItem)
{
	MenuItemEx Result;
	Result.Flags = FItem.Flags;
	Result.Name = NullToEmpty(FItem.Text);
	Result.SimpleUserData = FItem.UserData;
	return Result;
}

VMenu::VMenu(private_tag, string Title, int MaxHeight, dialog_ptr ParentDialog):
	strTitle(std::move(Title)),
	SelectPos(-1),
	SelectPosResult(-1),
	TopPos(0),
	MaxHeight(MaxHeight),
	WasAutoHeight(false),
	m_MaxLength(0),
	m_BoxType(DOUBLE_BOX),
	PrevCursorVisible(),
	PrevCursorSize(),
	ParentDialog(ParentDialog),
	DialogItemID(),
	bFilterEnabled(false),
	bFilterLocked(false),
	ItemHiddenCount(0),
	ItemSubMenusCount(0),
	Colors(),
	MaxLineWidth(),
	bRightBtnPressed(),
	MenuId(FarUuid)
{
}

vmenu_ptr VMenu::create(string Title, span<menu_item const> const Data, int MaxHeight, DWORD Flags, dialog_ptr ParentDialog)
{
	auto VmenuPtr = std::make_shared<VMenu>(private_tag(), std::move(Title), MaxHeight, ParentDialog);
	VmenuPtr->init(Data, Flags);
	return VmenuPtr;
}

void VMenu::init(span<menu_item const> const Data, DWORD Flags)
{
	SaveScr=nullptr;
	SetMenuFlags(Flags | VMENU_MOUSEREACTION | VMENU_UPDATEREQUIRED);
	ClearFlags(VMENU_MOUSEDOWN);
	CurrentWindow = Global->WindowManager->GetCurrentWindow();
	GetCursorType(PrevCursorVisible,PrevCursorSize);
	bRightBtnPressed = false;

	// инициализируем перед добавлением элемента
	UpdateMaxLengthFromTitles();

	for (const auto& i: Data)
	{
		MenuItemEx NewItem;
		static_cast<menu_item&>(NewItem) = i;
		AddItem(std::move(NewItem));
	}

	SetMaxHeight(MaxHeight);
	SetColors(nullptr); //Установим цвет по умолчанию
}

VMenu::~VMenu()
{
	VMenu::Hide();
	clear();

	if (Global->WindowManager->GetCurrentWindow() == CurrentWindow)
		SetCursorType(PrevCursorVisible,PrevCursorSize);
}

void VMenu::ResetCursor()
{
	GetCursorType(PrevCursorVisible,PrevCursorSize);
}

//может иметь фокус
static bool ItemCanHaveFocus(unsigned long long const Flags)
{
	return !(Flags & (LIF_DISABLE | LIF_HIDDEN | LIF_FILTERED | LIF_SEPARATOR));
}

static bool ItemCanHaveFocus(MenuItemEx const& Item)
{
	return ItemCanHaveFocus(Item.Flags);
}

//может быть выбран
static bool ItemCanBeEntered(MenuItemEx const& Item)
{
	return ItemCanHaveFocus(Item) && !(Item.Flags & LIF_GRAYED);
}

//видимый
static bool ItemIsVisible(MenuItemEx const& Item)
{
	return !(Item.Flags & (LIF_HIDDEN | LIF_FILTERED));
}

bool VMenu::UpdateRequired() const
{
	return CheckFlags(VMENU_UPDATEREQUIRED)!=0;
}

void VMenu::UpdateItemFlags(int Pos, unsigned long long NewFlags)
{
	if (Items[Pos].Flags & MIF_SUBMENU)
		--ItemSubMenusCount;

	if (!ItemIsVisible(Items[Pos]))
		--ItemHiddenCount;


	if (!ItemCanHaveFocus(NewFlags))
		NewFlags &= ~LIF_SELECTED;

	//remove selection
	if ((Items[Pos].Flags&LIF_SELECTED) && !(NewFlags&LIF_SELECTED))
	{
		SelectPos = -1;
	}
	//set new selection
	else if (!(Items[Pos].Flags&LIF_SELECTED) && (NewFlags&LIF_SELECTED))
	{
		if (SelectPos>=0)
			Items[SelectPos].Flags &= ~LIF_SELECTED;

		SelectPos = Pos;
	}

	Items[Pos].Flags = NewFlags;

	if (SelectPos < 0)
		SetSelectPos(0,1);

	if(LOWORD(Items[Pos].Flags))
	{
		Items[Pos].Flags|=LIF_CHECKED;
		if(LOWORD(Items[Pos].Flags)==1)
		{
			Items[Pos].Flags&=0xFFFF0000;
		}
	}

	if (NewFlags&MIF_SUBMENU)
		ItemSubMenusCount++;

	if (!ItemIsVisible(Items[Pos]))
		ItemHiddenCount++;
}

// переместить курсор c учётом пунктов которые не могут получать фокус
int VMenu::SetSelectPos(int Pos, int Direct, bool stop_on_edge)
{
	SelectPosResult=-1;

	if (Items.empty())
		return -1;

	for (auto& i: Items)
	{
		i.Flags &= ~LIF_SELECTED;
	}

	for (int Pass=0, I=0;;I++)
	{
		if (Pos<0)
		{
			if (CheckFlags(VMENU_WRAPMODE))
			{
				Pos = static_cast<int>(Items.size()-1);
				TopPos = Pos;
			}
			else
			{
				Pos = 0;
				TopPos = 0;
				Pass++;
			}
		}
		else if (Pos>=static_cast<int>(Items.size()))
		{
			if (CheckFlags(VMENU_WRAPMODE))
			{
				Pos = 0;
				TopPos = 0;
			}
			else
			{
				Pos = static_cast<int>(Items.size()-1);
				Pass++;
			}
		}

		if (ItemCanHaveFocus(Items[Pos]))
			break;

		if (Pass)
		{
			Pos = SelectPos;
			break;
		}

		Pos += Direct;

		if (I>=static_cast<int>(Items.size())) // круг пройден - ничего не найдено :-(
			Pass++;
	}

	if (stop_on_edge && CheckFlags(VMENU_WRAPMODE) && ((Direct > 0 && Pos < SelectPos) || (Direct<0 && Pos>SelectPos)))
		Pos = SelectPos;

	auto Parent = GetDialog();
	if (Pos != SelectPos && CheckFlags(VMENU_COMBOBOX|VMENU_LISTBOX) && Parent && Parent->IsInited() && !Parent->SendMessage(DN_LISTCHANGE, DialogItemID, ToPtr(Pos)))
	{
		UpdateItemFlags(SelectPos, Items[SelectPos].Flags|LIF_SELECTED);
		return -1;
	}

	if (Pos >= 0)
		UpdateItemFlags(Pos, Items[Pos].Flags|LIF_SELECTED);

	SetMenuFlags(VMENU_UPDATEREQUIRED);

	SelectPosResult=Pos;
	return Pos;
}

// установить курсор и верхний элемент
int VMenu::SetSelectPos(const FarListPos *ListPos, int Direct)
{
	const auto pos = std::clamp(ListPos->SelectPos, intptr_t(0), static_cast<intptr_t>(Items.size() - 1));
	const auto Ret = SetSelectPos(pos, Direct ? Direct : pos > SelectPos? 1 : -1);

	if (Ret >= 0)
	{
		TopPos = ListPos->TopPos;

		if (TopPos == -1)
		{
			if (GetShowItemCount() < MaxHeight)
			{
				TopPos = VisualPosToReal(0);
			}
			else
			{
				TopPos = GetVisualPos(TopPos);
				TopPos = (GetVisualPos(SelectPos)-TopPos+1) > MaxHeight ? TopPos+1 : TopPos;

				if (TopPos+MaxHeight > GetShowItemCount())
					TopPos = GetShowItemCount()-MaxHeight;

				TopPos = VisualPosToReal(TopPos);
			}
		}

		if (TopPos < 0)
			TopPos = 0;
	}

	return Ret;
}

//корректировка текущей позиции
void VMenu::UpdateSelectPos()
{
	if (Items.empty())
		return;

	// если selection стоит в некорректном месте - сбросим его
	if (SelectPos >= 0 && !ItemCanHaveFocus(Items[SelectPos]))
		SelectPos = -1;

	for (size_t i=0; i<Items.size(); i++)
	{
		if (!ItemCanHaveFocus(Items[i]))
		{
			Items[i].SetSelect(false);
		}
		else
		{
			if (SelectPos == -1)
			{
				Items[i].SetSelect(true);
				SelectPos = static_cast<int>(i);
			}
			else if (SelectPos != static_cast<int>(i))
			{
				Items[i].SetSelect(false);
			}
			else
			{
				Items[i].SetSelect(true);
			}
		}
	}
}

int VMenu::GetItemPosition(int Position) const
{
	int DataPos = (Position==-1) ? SelectPos : Position;

	if (DataPos>=static_cast<int>(Items.size()))
		DataPos = -1; //Items.size()-1;

	return DataPos;
}

// получить позицию курсора и верхнюю позицию элемента
int VMenu::GetSelectPos(FarListPos *ListPos) const
{
	ListPos->SelectPos=SelectPos;
	ListPos->TopPos=TopPos;

	return ListPos->SelectPos;
}

int VMenu::InsertItem(const FarListInsert *NewItem)
{
	if (NewItem)
	{
		if (AddItem(FarList2MenuItem(NewItem->Item), NewItem->Index) >= 0)
			return static_cast<int>(Items.size());
	}

	return -1;
}

int VMenu::AddItem(const FarList* List)
{
	if (List && List->Items)
	{
		for (const auto& Item: span(List->Items, List->ItemsNumber))
		{
			AddItem(FarList2MenuItem(Item));
		}
	}

	return static_cast<int>(Items.size());
}

int VMenu::AddItem(const wchar_t *NewStrItem)
{
	FarListItem FarListItem0={};

	if (!NewStrItem || NewStrItem[0] == 0x1)
	{
		FarListItem0.Flags=LIF_SEPARATOR;
		if (NewStrItem)
			FarListItem0.Text = NewStrItem + 1;
	}
	else
	{
		FarListItem0.Text=NewStrItem;
	}

	FarList FarList0={sizeof(FarList),1,&FarListItem0};

	return AddItem(&FarList0)-1; //-1 потому что AddItem(FarList) возвращает количество элементов
}

int VMenu::AddItem(MenuItemEx&& NewItem,int PosAdd)
{
	PosAdd = std::clamp(PosAdd, 0, static_cast<int>(Items.size()));

	Items.emplace(Items.begin() + PosAdd, std::move(NewItem));
	auto& NewMenuItem = Items[PosAdd];

	NewMenuItem.AutoHotkey = {};
	NewMenuItem.AutoHotkeyPos = 0;
	NewMenuItem.ShowPos = 0;

	if (PosAdd <= SelectPos)
		SelectPos++;

	if (CheckFlags(VMENU_SHOWAMPERSAND))
		UpdateMaxLength(NewMenuItem.Name.size());
	else
		UpdateMaxLength(HiStrlen(NewMenuItem.Name));

	const auto NewFlags = NewMenuItem.Flags;
	NewMenuItem.Flags = 0;
	UpdateItemFlags(PosAdd, NewFlags);

	SetMenuFlags(VMENU_UPDATEREQUIRED | (bFilterEnabled ? VMENU_REFILTERREQUIRED : VMENU_NONE));

	return static_cast<int>(Items.size()-1);
}

bool VMenu::UpdateItem(const FarListUpdate *NewItem)
{
	if (!NewItem || static_cast<size_t>(NewItem->Index) >= Items.size())
		return false;

	// Освободим память... от ранее занятого ;-)
	if (NewItem->Item.Flags&LIF_DELETEUSERDATA)
	{
		Items[NewItem->Index].ComplexUserData = {};
	}

	Items[NewItem->Index].Name = NullToEmpty(NewItem->Item.Text);
	UpdateItemFlags(NewItem->Index, NewItem->Item.Flags);
	Items[NewItem->Index].SimpleUserData = NewItem->Item.UserData;

	SetMenuFlags(VMENU_UPDATEREQUIRED | (bFilterEnabled ? VMENU_REFILTERREQUIRED : VMENU_NONE));

	return true;
}

//функция удаления N пунктов меню
int VMenu::DeleteItem(int ID, int Count)
{
	if (ID < 0 || ID >= static_cast<int>(Items.size()) || Count <= 0)
		return static_cast<int>(Items.size());

	if (ID+Count > static_cast<int>(Items.size()))
		Count=static_cast<int>(Items.size()-ID);

	if (Count <= 0)
		return static_cast<int>(Items.size());

	if (!ID && Count == static_cast<int>(Items.size()))
	{
		clear();
		return static_cast<int>(Items.size());
	}

	for (int I=0; I < Count; ++I)
	{
		if (Items[ID+I].Flags & MIF_SUBMENU)
			--ItemSubMenusCount;

		if (!ItemIsVisible(Items[ID+I]))
			--ItemHiddenCount;
	}

	// а вот теперь перемещения
	const auto FirstIter = Items.begin() + ID, LastIter = FirstIter + Count;
	if (Items.size() > 1)
		Items.erase(FirstIter, LastIter);

	// коррекция текущей позиции
	if (SelectPos >= ID && SelectPos < ID+Count)
	{
		if(SelectPos==static_cast<int>(Items.size()))
		{
			ID--;
		}
		SelectPos = -1;
		SetSelectPos(ID,1);
	}
	else if (SelectPos >= ID+Count)
	{
		SelectPos -= Count;

		if (TopPos >= ID+Count)
			TopPos -= Count;
	}

	SetMenuFlags(VMENU_UPDATEREQUIRED);

	return static_cast<int>(Items.size());
}

void VMenu::clear()
{
	Items.clear();
	ItemHiddenCount=0;
	ItemSubMenusCount=0;
	SelectPos=-1;
	TopPos=0;
	m_MaxLength=0;
	UpdateMaxLengthFromTitles();

	SetMenuFlags(VMENU_UPDATEREQUIRED);
}

int VMenu::GetCheck(int Position)
{
	const auto ItemPos = GetItemPosition(Position);

	if (ItemPos < 0)
		return 0;

	if (Items[ItemPos].Flags & LIF_SEPARATOR)
		return 0;

	if (!(Items[ItemPos].Flags & LIF_CHECKED))
		return 0;

	const auto Checked = Items[ItemPos].Flags & 0xFFFF;

	return Checked ? Checked : 1;
}

void VMenu::SetCheck(int Position)
{
	const auto ItemPos = GetItemPosition(Position);
	if (ItemPos < 0)
		return;

	Items[ItemPos].SetCheck();
}

void VMenu::SetCustomCheck(wchar_t Char, int Position)
{
	const auto ItemPos = GetItemPosition(Position);
	if (ItemPos < 0)
		return;

	Items[ItemPos].SetCustomCheck(Char);
}

void VMenu::ClearCheck(int Position)
{
	const auto ItemPos = GetItemPosition(Position);
	if (ItemPos < 0)
		return;

	Items[ItemPos].ClearCheck();
}

void VMenu::RestoreFilteredItems()
{
	for (auto& i: Items)
	{
		if (!(i.Flags & MIF_FILTERED))
			continue;

		i.Flags &= ~MIF_FILTERED;
		if (ItemIsVisible(i))
			--ItemHiddenCount;
	}

	FilterUpdateHeight();

	// Подровнять, а то в нижней части списка может оставаться куча пустых строк
	FarListPos pos={sizeof(FarListPos),SelectPos < 0 ? 0 : SelectPos,-1};
	SetSelectPos(&pos);
}

void VMenu::FilterStringUpdated()
{
	int PrevSeparator = -1, PrevGroup = -1;
	int UpperVisible = -1, LowerVisible = -2;
	bool bBottomMode = false;

	if (SelectPos > 0)
	{
		// Определить, в верхней или нижней части расположен курсор
		const auto TopVisible = GetVisualPos(TopPos);
		const auto SelectedVisible = GetVisualPos(SelectPos);
		const auto BottomVisible = (TopVisible+MaxHeight > GetShowItemCount()) ? (TopVisible+MaxHeight-1) : (GetShowItemCount()-1);
		if (SelectedVisible >= ((TopVisible+BottomVisible)>>1))
			bBottomMode = true;
	}

	ItemHiddenCount=0;

	for (const auto& [CurItem, index]: enumerate(Items))
	{
		CurItem.Flags &= ~LIF_FILTERED;

		if (!ItemIsVisible(CurItem))
		{
			++ItemHiddenCount;
			continue;
		}

		if (CurItem.Flags & LIF_SEPARATOR)
		{
			// В предыдущей группе все элементы скрыты, разделитель перед группой - не нужен
			if (PrevSeparator != -1)
			{
				Items[PrevSeparator].Flags |= LIF_FILTERED;
				ItemHiddenCount++;
			}

			if (CurItem.Name.empty() && PrevGroup == -1)
			{
				CurItem.Flags |= LIF_FILTERED;
				ItemHiddenCount++;
				PrevSeparator = -1;
			}
			else
			{
				PrevSeparator = static_cast<int>(index);
			}
		}
		else
		{
			auto Text = trim(CurItem.Name);
			RemoveHighlights(Text);
			if(!contains_icase(Text, strFilter))
			{
				CurItem.Flags |= LIF_FILTERED;
				ItemHiddenCount++;
				if (SelectPos == static_cast<int>(index))
				{
					CurItem.Flags &= ~LIF_SELECTED;
					SelectPos = -1;
					LowerVisible = -1;
				}
			}
			else
			{
				PrevGroup = static_cast<int>(index);
				if (LowerVisible == -2)
				{
					if (ItemCanHaveFocus(CurItem))
						UpperVisible = static_cast<int>(index);
				}
				else if (LowerVisible == -1)
				{
					if (ItemCanHaveFocus(CurItem))
						LowerVisible = static_cast<int>(index);
				}
				// Этот разделитель - оставить видимым
				if (PrevSeparator != -1)
					PrevSeparator = -1;
			}
		}
	}

	// В предыдущей группе все элементы скрыты, разделитель перед группой - не нужен
	if (PrevSeparator != -1)
	{
		Items[PrevSeparator].Flags |= LIF_FILTERED;
		ItemHiddenCount++;
	}

	FilterUpdateHeight();

	if (GetShowItemCount()>0)
	{
		// Подровнять, а то в нижней части списка может оставаться куча пустых строк
		FarListPos pos={sizeof(FarListPos),SelectPos,-1};
		if (SelectPos<0)
		{
			pos.SelectPos = bBottomMode ? ((LowerVisible>0) ? LowerVisible : UpperVisible) : UpperVisible;
			if (pos.SelectPos == -1)
				pos.SelectPos = bBottomMode ? VisualPosToReal(GetShowItemCount()-1) : 0;
		}
		SetSelectPos(&pos);
	}
}

void VMenu::FilterUpdateHeight(bool bShrink)
{
	auto Parent = std::dynamic_pointer_cast<VMenu2>(GetDialog());

	if (WasAutoHeight || Parent)
	{
		int NewBottom;
		if (MaxHeight && MaxHeight<GetShowItemCount())
			NewBottom = m_Where.top + MaxHeight + 1;
		else
			NewBottom = m_Where.top + GetShowItemCount() + 1;
		if (NewBottom > ScrY)
			NewBottom = ScrY;
		if (NewBottom > m_Where.bottom || (bShrink && NewBottom < m_Where.bottom))
		{
			if (Parent)
				Parent->Resize();
			else
			{
				auto NewPosition = m_Where;
				NewPosition.bottom = NewBottom;
				SetPosition(NewPosition);
			}
		}
	}
}

static bool IsFilterEditKey(int Key)
{
	return (Key >= static_cast<int>(KEY_SPACE) && Key < 0xffff) || Key == KEY_BS;
}

bool VMenu::ShouldSendKeyToFilter(unsigned const Key) const
{
	if (any_of(Key, KEY_CTRLALTF, KEY_RCTRLRALTF, KEY_CTRLRALTF, KEY_RCTRLALTF))
		return true;

	if (bFilterEnabled)
	{
		if (any_of(Key, KEY_CTRLALTL, KEY_RCTRLRALTL, KEY_CTRLRALTL, KEY_RCTRLALTL))
			return true;

		if (!bFilterLocked && IsFilterEditKey(Key))
			return true;
	}

	return false;
}

long long VMenu::VMProcess(int OpCode, void* vParam, long long iParam)
{
	switch (OpCode)
	{
		case MCODE_C_EMPTY:
			return GetShowItemCount()<=0;
		case MCODE_C_EOF:
			return GetVisualPos(SelectPos)==GetShowItemCount()-1;
		case MCODE_C_BOF:
			return GetVisualPos(SelectPos)<=0;
		case MCODE_C_SELECTED:
			return !Items.empty() && SelectPos >= 0;
		case MCODE_V_ITEMCOUNT:
			return GetShowItemCount();
		case MCODE_V_CURPOS:
			return GetVisualPos(SelectPos)+1;
		case MCODE_F_MENU_CHECKHOTKEY:
		{
			const auto str = static_cast<const wchar_t*>(vParam);
			return GetVisualPos(CheckHighlights(*str, VisualPosToReal(static_cast<int>(iParam)))) + 1;
		}
		case MCODE_F_MENU_SELECT:
		{
			const auto StrParam = static_cast<const wchar_t*>(vParam);
			if (!*StrParam)
				return 0;

			const string str = StrParam;

				int Direct=(iParam >> 8)&0xFF;
				/*
					Direct:
						0 - от начала в конец списка;
						1 - от текущей позиции в начало;
						2 - от текущей позиции в конец списка пунктов меню.
				*/
				iParam&=0xFF;
				const auto StartPos=Direct?SelectPos:0;
				int EndPos=static_cast<int>(Items.size()-1);

				if (Direct == 1)
				{
					EndPos=0;
					Direct=-1;
				}
				else
				{
					Direct=1;
				}

				for (int I=StartPos; ;I+=Direct)
				{
					if (Direct > 0)
					{
						if (I > EndPos)
							break;
					}
					else
					{
						if (I < EndPos)
							break;
					}

					const auto& Item = at(I);

					if (!ItemCanHaveFocus(Item))
						continue;

					int Res = 0;
					const auto strTemp = trim(HiText2Str(Item.Name));

					switch (iParam)
					{
						case 0: // full compare
							Res = equal_icase(strTemp, str);
							break;
						case 1: // begin compare
							Res = starts_with_icase(strTemp, str);
							break;
						case 2: // end compare
							Res = ends_with_icase(strTemp, str);
							break;
						case 3: // in str
							Res = contains_icase(strTemp, str);
							break;
					}

					if (Res)
					{
						SetSelectPos(I,1);

						ShowMenu(true);

						return GetVisualPos(SelectPos)+1;
					}
				}

			return 0;
		}

		case MCODE_F_MENU_GETHOTKEY:
		case MCODE_F_MENU_GETVALUE: // S=Menu.GetValue([N])
		{
			intptr_t Param = iParam;

			if (Param == -1)
				Param = SelectPos;
			else
				Param = VisualPosToReal(Param);

			if (Param>=0 && Param<static_cast<intptr_t>(Items.size()))
			{
				auto& menuEx = at(Param);
				if (OpCode == MCODE_F_MENU_GETVALUE)
				{
					*static_cast<string *>(vParam) = menuEx.Name;
					return 1;
				}
				else
				{
					return GetHighlights(&menuEx);
				}
			}

			return 0;
		}

		case MCODE_F_MENU_ITEMSTATUS: // N=Menu.ItemStatus([N])
		{
			if (iParam == -1LL)
				iParam = SelectPos;
			else if (iParam >= static_cast<int>(size()) || iParam <= -1LL)
				return -1;

			auto& menuEx = at(iParam);

			auto RetValue = menuEx.Flags;

			if (iParam == SelectPos)
				RetValue |= LIF_SELECTED;

			return MAKELONG(HIWORD(RetValue),LOWORD(RetValue));
		}

		case MCODE_V_MENU_VALUE: // Menu.Value
		{
			if (!HasVisible())
				return 0;
			*static_cast<string*>(vParam) = at(SelectPos).Name;
			return 1;
		}

		case MCODE_F_MENU_FILTER:
		{
			long long RetValue = 0;
			/*
			Action
			  0 - фильтр
			    Mode
			      -1 - (по умолчанию) вернуть 1 если фильтр уже включен, 0 - фильтр выключен
				   1 - включить фильтр, если фильтр уже включен - ничего не делает
				   0 - выключить фильтр
			  1 - фиксация текста фильтра
			    Mode
			      -1 - (по умолчанию) вернуть 1 если текст фильтра зафиксирован, 0 - фильтр можно менять с клавиатуры
				   1 - зафиксировать фильтр
				   0 - отменить фиксацию фильтра
			  2 - вернуть 1 если фильтр включен и строка фильтра не пуста
			  3 - вернуть количество отфильтрованных (невидимых) строк\
			  4 - (по умолчанию) подправить высоту списка под количество элементов
			*/
			const auto Parameter = reinterpret_cast<intptr_t>(vParam);
			switch (iParam)
			{
				case 0:
					switch (Parameter)
					{
						case 0:
						case 1:
							if (bFilterEnabled != (Parameter == 1))
							{
								EnableFilter(Parameter == 1);
								DisplayObject();
							}
							RetValue = 1;
							break;

						case -1:
							RetValue = bFilterEnabled;
							break;
					}
					break;

				case 1:
					switch (Parameter)
					{
						case 0:
						case 1:
							bFilterLocked = Parameter == 1;
							DisplayObject();
							RetValue = 1;
							break;

						case -1:
							RetValue = bFilterLocked;
							break;
					}
					break;

				case 2:
					RetValue = bFilterEnabled && !strFilter.empty();
					break;

				case 3:
					// Don't use ItemHiddenCount here - it also includes invisible (LIF_HIDDEN), but non-filtered items
					RetValue = std::count_if(ALL_CONST_RANGE(Items), [](const MenuItemEx& Item) { return (Item.Flags & LIF_FILTERED) != 0; });
					break;

				case 4:
					FilterUpdateHeight(true);
					DisplayObject();
					RetValue = 1;
					break;
			}
			return RetValue;
		}
		case MCODE_F_MENU_FILTERSTR:
		{
			/*
			Action
			  0 - (по умолчанию) вернуть текущую строку, если фильтр включен
			  1 - установить в фильтре строку S.
			      Если фильтр не был включен - включает его, режим фиксации не трогается, но игнорируется.
				  Возвращает предыдущее значение строки фильтра.
			*/
			switch (iParam)
			{
				case 0:
					if (bFilterEnabled)
					{
						*static_cast<string *>(vParam) = strFilter;
						return 1;
					}
					break;
				case 1:
					if (!bFilterEnabled)
						bFilterEnabled=true;
					const auto prevLocked = bFilterLocked;
					bFilterLocked = false;
					RestoreFilteredItems();
					auto oldFilter = std::move(strFilter);
					strFilter.clear();
					AddToFilter(*static_cast<const string*>(vParam));
					FilterStringUpdated();
					bFilterLocked = prevLocked;
					DisplayObject();
					*static_cast<string*>(vParam) = std::move(oldFilter);
					return 1;
			}

			return 0;
		}
		case MCODE_V_MENUINFOID:
		{
			static string strId;
			strId = uuid::str(MenuId);
			return reinterpret_cast<intptr_t>(UNSAFE_CSTR(strId));
		}

	}

	return 0;
}

bool VMenu::AddToFilter(string_view const Str)
{
	if (!bFilterEnabled || bFilterLocked)
		return false;

	for (const auto Key: Str)
	{
		if (IsFilterEditKey(Key))
		{
			if (Key == KEY_BS && !strFilter.empty())
				strFilter.pop_back();
			else
				strFilter += Key;
		}
	}
	return true;
}

bool VMenu::ProcessFilterKey(int Key)
{
	if (!bFilterEnabled || bFilterLocked || !IsFilterEditKey(Key))
		return false;

	if (Key==KEY_BS)
	{
		if (!strFilter.empty())
		{
			strFilter.pop_back();

			if (strFilter.empty())
			{
				RestoreFilteredItems();
				DisplayObject();
				return true;
			}
		}
		else
		{
			return true;
		}
	}
	else
	{
		if (!GetShowItemCount())
			return true;

		strFilter += static_cast<wchar_t>(Key);
	}

	FilterStringUpdated();
	DisplayObject();

	return true;
}

bool VMenu::ProcessKey(const Manager::Key& Key)
{
	auto LocalKey = Key();
	auto Parent = GetDialog();
	if (IsComboBox() && !Parent->GetDropDownOpened())
	{
		Close(-1);
		return false;
	}

	if (IsComboBox() && CheckFlags(VMENU_COMBOBOXEVENTKEY))
	{
		auto Event = Key.Event();
		if (!Parent->DlgProc(DN_INPUT, 0, &Event))
			return true;
	}

	if (any_of(LocalKey, KEY_NONE, KEY_IDLE))
		return false;

	if (IsComboBox() && CheckFlags(VMENU_COMBOBOXEVENTKEY))
	{
		auto Event = Key.Event();
		if (Parent->DlgProc(DN_CONTROLINPUT, Parent->GetDlgFocusPos(), &Event))
			return true;
	}

	if (LocalKey == KEY_OP_PLAINTEXT)
	{
		const auto str = Global->CtrlObject->Macro.GetStringToPrint();

		if (str.empty())
			return false;

		if ( AddToFilter(str) ) // для фильтра: всю строку целиком в фильтр, а там разберемся.
		{
			if (strFilter.empty())
				RestoreFilteredItems();
			else
				FilterStringUpdated();

			DisplayObject();

			return true;
		}
		else // не для фильтра: по старинке, первый символ последовательности, остальное игнорируем (ибо некуда)
			LocalKey = str.front();
	}

	SetMenuFlags(VMENU_UPDATEREQUIRED);

	if (!GetShowItemCount())
	{
		if (none_of(LocalKey, KEY_F1, KEY_SHIFTF1, KEY_F10, KEY_ESC, KEY_ALTF9, KEY_RALTF9))
		{
			if (!bFilterEnabled || none_of(LocalKey, KEY_BS, KEY_CTRLALTF, KEY_RCTRLRALTF, KEY_CTRLRALTF, KEY_RCTRLALTF, KEY_RALT, KEY_OP_XLAT))
			{
				m_ExitCode = -1;
				return false;
			}
		}
	}

	if (!((LocalKey >= KEY_MACRO_BASE && LocalKey <= KEY_MACRO_ENDBASE) || (LocalKey >= KEY_OP_BASE && LocalKey <= KEY_OP_ENDBASE)))
	{
		DWORD S=LocalKey&(KEY_CTRL|KEY_ALT|KEY_SHIFT|KEY_RCTRL|KEY_RALT);
		DWORD K=LocalKey&(~(KEY_CTRL|KEY_ALT|KEY_SHIFT|KEY_RCTRL|KEY_RALT));

		if (K==KEY_MULTIPLY)
			LocalKey = L'*'|S;
		else if (K==KEY_ADD)
			LocalKey = L'+'|S;
		else if (K==KEY_SUBTRACT)
			LocalKey = L'-'|S;
		else if (K==KEY_DIVIDE)
			LocalKey = L'/'|S;
	}

	const auto ProcessEnter = [this]()
	{
		if (ItemCanBeEntered(Items[SelectPos]))
		{
			if (IsComboBox())
			{
				Close(SelectPos);
			}
			else
			{
				SetExitCode(SelectPos);
			}
		}
	};
	switch (LocalKey)
	{
		case KEY_ALTF9:
		case KEY_RALTF9:
			Global->WindowManager->ProcessKey(Manager::Key(KEY_ALTF9));
			break;
		case KEY_NUMENTER:
		case KEY_ENTER:
		{
			if (!Parent || CheckFlags(VMENU_COMBOBOX))
				ProcessEnter();
			break;
		}
		case KEY_ESC:
		case KEY_F10:
		{
			if (IsComboBox())
			{
				Close(-1);
			}
			else if(!Parent)
			{
				SetExitCode(-1);
			}
			break;
		}
		case KEY_HOME:         case KEY_NUMPAD7:
		case KEY_CTRLHOME:     case KEY_CTRLNUMPAD7:
		case KEY_RCTRLHOME:    case KEY_RCTRLNUMPAD7:
		case KEY_CTRLPGUP:     case KEY_CTRLNUMPAD9:
		case KEY_RCTRLPGUP:    case KEY_RCTRLNUMPAD9:
		{
			FarListPos pos={sizeof(FarListPos),0,-1};
			SetSelectPos(&pos, 1);
			ShowMenu(true);
			break;
		}
		case KEY_END:          case KEY_NUMPAD1:
		case KEY_CTRLEND:      case KEY_CTRLNUMPAD1:
		case KEY_RCTRLEND:     case KEY_RCTRLNUMPAD1:
		case KEY_CTRLPGDN:     case KEY_CTRLNUMPAD3:
		case KEY_RCTRLPGDN:    case KEY_RCTRLNUMPAD3:
		{
			int p = static_cast<int>(Items.size())-1;
			FarListPos pos={sizeof(FarListPos),p,std::max(0,p-MaxHeight+1)};
			SetSelectPos(&pos, -1);
			ShowMenu(true);
			break;
		}
		case KEY_PGUP:         case KEY_NUMPAD9:
		{
			const auto dy = m_Where.height() - (m_BoxType == NO_BOX? 1 : 2);

			int p = VisualPosToReal(GetVisualPos(SelectPos)-dy);

			if (p < 0)
				p = 0;

			FarListPos pos={sizeof(FarListPos),p,p};
			SetSelectPos(&pos, 1);
			ShowMenu(true);
			break;
		}
		case KEY_PGDN:         case KEY_NUMPAD3:
		{
			const auto dy = m_Where.height() - (m_BoxType == NO_BOX? 1 : 2);

			int pSel = VisualPosToReal(GetVisualPos(SelectPos)+dy);
			int pTop = VisualPosToReal(GetVisualPos(TopPos + 1));

			pSel = std::min(pSel, static_cast<int>(Items.size())-1);
			pTop = std::min(pTop, static_cast<int>(Items.size())-1);

			FarListPos pos={sizeof(FarListPos),pSel,pTop};
			SetSelectPos(&pos, -1);
			ShowMenu(true);
			break;
		}
		case KEY_ALTHOME:           case KEY_ALT|KEY_NUMPAD7:
		case KEY_RALTHOME:          case KEY_RALT|KEY_NUMPAD7:
		case KEY_ALTEND:            case KEY_ALT|KEY_NUMPAD1:
		case KEY_RALTEND:           case KEY_RALT|KEY_NUMPAD1:
		{
			if (any_of(LocalKey, KEY_ALTHOME, KEY_RALTHOME, KEY_ALT | KEY_NUMPAD7, KEY_RALT | KEY_NUMPAD7))
			{
				for (auto& i: Items)
				{
					i.ShowPos = 0;
				}
			}
			else
			{

				for (auto& i: Items)
				{
					const auto Len = CheckFlags(VMENU_SHOWAMPERSAND)? i.Name.size() : HiStrlen(i.Name);
					if (Len >= MaxLineWidth)
						i.ShowPos = Len - MaxLineWidth;
				}
			}

			ShowMenu(true);
			break;
		}
		case KEY_ALTLEFT:   case KEY_ALT|KEY_NUMPAD4:  case KEY_MSWHEEL_LEFT:
		case KEY_RALTLEFT:  case KEY_RALT|KEY_NUMPAD4:
		case KEY_ALTRIGHT:  case KEY_ALT|KEY_NUMPAD6:  case KEY_MSWHEEL_RIGHT:
		case KEY_RALTRIGHT: case KEY_RALT|KEY_NUMPAD6:
		{
			bool NeedRedraw=false;

			for (size_t I=0; I < Items.size(); ++I)
				if (ShiftItemShowPos(static_cast<int>(I), any_of(LocalKey, KEY_ALTLEFT, KEY_RALTLEFT, KEY_ALT | KEY_NUMPAD4, KEY_RALT | KEY_NUMPAD4, KEY_MSWHEEL_LEFT)? -1 : 1))
					NeedRedraw=true;

			if (NeedRedraw)
				ShowMenu(true);

			break;
		}
		case KEY_ALTSHIFTLEFT:      case KEY_ALT|KEY_SHIFT|KEY_NUMPAD4:
		case KEY_RALTSHIFTLEFT:     case KEY_RALT|KEY_SHIFT|KEY_NUMPAD4:
		case KEY_ALTSHIFTRIGHT:     case KEY_ALT|KEY_SHIFT|KEY_NUMPAD6:
		case KEY_RALTSHIFTRIGHT:    case KEY_RALT|KEY_SHIFT|KEY_NUMPAD6:
		{
			if (ShiftItemShowPos(SelectPos, any_of(LocalKey, KEY_ALTSHIFTLEFT, KEY_RALTSHIFTLEFT, KEY_ALT | KEY_SHIFT | KEY_NUMPAD4, KEY_RALT | KEY_SHIFT | KEY_NUMPAD4)? -1 : 1))
				ShowMenu(true);

			break;
		}
		case KEY_MSWHEEL_UP:
		{
			if(SelectPos)
			{
				FarListPos Pos = {sizeof(Pos), SelectPos-1, TopPos-1};
				SetSelectPos(&Pos);
				ShowMenu(true);
			}
			break;
		}
		case KEY_MSWHEEL_DOWN:
		{
			if(SelectPos < static_cast<int>(Items.size()-1))
			{
				FarListPos Pos = { sizeof(Pos), SelectPos + 1, TopPos };
				const auto ItemsSize = static_cast<int>(Items.size());
				const auto HeightSize = std::max(0, m_Where.height() - (m_BoxType == NO_BOX? 0 : 2));
				if (!(ItemsSize - TopPos <= HeightSize || ItemsSize <= HeightSize))
					Pos.TopPos++;
				SetSelectPos(&Pos);
				ShowMenu(true);
			}
			break;
		}

		case KEY_LEFT:         case KEY_NUMPAD4:
		case KEY_UP:           case KEY_NUMPAD8:
		{
			SetSelectPos(SelectPos-1,-1,IsRepeatedKey());
			ShowMenu(true);
			break;
		}

		case KEY_RIGHT:        case KEY_NUMPAD6:
		case KEY_DOWN:         case KEY_NUMPAD2:
		{
			SetSelectPos(SelectPos+1,1,IsRepeatedKey());
			ShowMenu(true);
			break;
		}

		case KEY_RALT:
		case KEY_CTRLALTF:
		case KEY_RCTRLRALTF:
		case KEY_CTRLRALTF:
		case KEY_RCTRLALTF:
		{
			EnableFilter(!bFilterEnabled);
			DisplayObject();
			break;
		}
		case KEY_CTRLV:
		case KEY_RCTRLV:
		case KEY_SHIFTINS:    case KEY_SHIFTNUMPAD0:
		{
			if (bFilterEnabled && !bFilterLocked)
			{
				string ClipText;
				if (!GetClipboardText(ClipText))
					return true;

				if (AddToFilter(ClipText))
				{
					if (strFilter.empty())
						RestoreFilteredItems();
					else
						FilterStringUpdated();

					DisplayObject();
				}
			}
			return true;
		}
		case KEY_CTRLALTL:
		case KEY_RCTRLRALTL:
		case KEY_CTRLRALTL:
		case KEY_RCTRLALTL:
			if (bFilterEnabled)
			{
				bFilterLocked=!bFilterLocked;
				DisplayObject();
			}
			break;

		case KEY_OP_XLAT:
		{
			if (bFilterEnabled && !bFilterLocked)
			{
				int start = static_cast<int>(strFilter.size());
				bool DoXlat = true;

				if (IsWordDiv(Global->Opt->XLat.strWordDivForXlat, strFilter[start]))
				{
					if (start) start--;
					DoXlat = !IsWordDiv(Global->Opt->XLat.strWordDivForXlat, strFilter[start]);
				}

				if (DoXlat)
				{
					while (start >= 0 && !IsWordDiv(Global->Opt->XLat.strWordDivForXlat, strFilter[start]))
						start--;

					start++;
					::Xlat(span(strFilter).subspan(start), Global->Opt->XLat.Flags);
					FilterStringUpdated();
					DisplayObject();
				}
			}
			break;
		}
		case KEY_TAB:
			if (IsComboBox())
			{
				ProcessEnter();
				break;
			}
			[[fallthrough]];
		default:
		{
			if (ProcessFilterKey(LocalKey))
				return true;

			int OldSelectPos=SelectPos;
			int NewPos=SelectPos;

			bool IsHotkey=true;
			if (!CheckKeyHiOrAcc(LocalKey, 0, false, !(Parent && !Parent->CheckDialogMode(DMODE_ISMENU) && CheckFlags(VMENU_COMBOBOX | VMENU_LISTBOX)), NewPos))
			{
				if (any_of(LocalKey, KEY_F1, KEY_SHIFTF1))
				{
					if (Parent)
						;//ParentDialog->ProcessKey(Key);
					else
						ShowHelp();

					break;
				}
				else
				{
					if (!CheckKeyHiOrAcc(LocalKey,1,FALSE,!(Parent && !Parent->CheckDialogMode(DMODE_ISMENU) && CheckFlags(VMENU_COMBOBOX|VMENU_LISTBOX)),NewPos))
						if (!CheckKeyHiOrAcc(LocalKey,1,TRUE,!(Parent && !Parent->CheckDialogMode(DMODE_ISMENU) && CheckFlags(VMENU_COMBOBOX|VMENU_LISTBOX)),NewPos))
							IsHotkey=false;
				}
			}

			if (IsHotkey && Parent)
			{
				if (Parent->SendMessage(DN_LISTHOTKEY,DialogItemID,ToPtr(NewPos)))
				{
					ShowMenu(true);
					ClearDone();
					break;
				}
				else
				{
					if (NewPos != OldSelectPos && SetSelectPos(NewPos, 1) < 0)
					{
						ClearDone();
					}
					else
					{
						CheckFlags(VMENU_COMBOBOX) ? Close(GetExitCode()) : ClearDone();
					}

					break;
				}
			}

			return false;
		}
	}

	return true;
}

bool VMenu::ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent)
{
	auto Parent = GetDialog();
	if (IsComboBox() && !Parent->GetDropDownOpened())
	{
		Close(-1);
		return false;
	}

	SetMenuFlags(VMENU_UPDATEREQUIRED);

	if (IsComboBox() && CheckFlags(VMENU_COMBOBOXEVENTMOUSE))
	{
		INPUT_RECORD Event = {};
		Event.EventType = MOUSE_EVENT;
		Event.Event.MouseEvent = *MouseEvent;
		if (!Parent->DlgProc(DN_INPUT, 0, &Event))
			return true;
	}

	if (!GetShowItemCount())
	{
		if (MouseEvent->dwButtonState && !MouseEvent->dwEventFlags)
			SetExitCode(-1);
		return false;
	}

	int MsX=MouseEvent->dwMousePosition.X;
	int MsY=MouseEvent->dwMousePosition.Y;

	// необходимо знать, что RBtn был нажат ПОСЛЕ появления VMenu, а не до
	if (MouseEvent->dwButtonState&RIGHTMOST_BUTTON_PRESSED && MouseEvent->dwEventFlags==0)
		bRightBtnPressed=true;

	if (MouseEvent->dwButtonState&FROM_LEFT_2ND_BUTTON_PRESSED && MouseEvent->dwEventFlags!=MOUSE_MOVED)
	{
		if (
			m_BoxType == NO_BOX?
				MsX >= m_Where.left && MsX <= m_Where.right && MsY >= m_Where.top && MsY <= m_Where.bottom :
				MsX > m_Where.left && MsX < m_Where.right && MsY > m_Where.top && MsY < m_Where.bottom
			)
		{
			ProcessKey(Manager::Key(KEY_ENTER));
		}
		return true;
	}

	if ((MouseEvent->dwButtonState&FROM_LEFT_1ST_BUTTON_PRESSED) && (MsX == m_Where.left + 2 || MsX == m_Where.right - 1 - (CheckFlags(VMENU_COMBOBOX | VMENU_LISTBOX) ? 0 : 2)))
	{
		// Click [«] or [»]
		while_mouse_button_pressed([&](DWORD const Button)
		{
			if (Button != FROM_LEFT_1ST_BUTTON_PRESSED)
				return false;

			ProcessKey(Manager::Key(MsX == m_Where.left + 2? KEY_ALTLEFT : KEY_ALTRIGHT));
			return true;
		});
		return true;
	}

	const auto SbY1 = m_Where.top + (m_BoxType == NO_BOX? 0 : 1);
	const auto SbY2 = m_Where.bottom - (m_BoxType == NO_BOX? 0 : 1);
	bool bShowScrollBar = false;

	if (CheckFlags(VMENU_LISTBOX|VMENU_ALWAYSSCROLLBAR) || Global->Opt->ShowMenuScrollbar)
		bShowScrollBar = true;

	if (bShowScrollBar && MsX == m_Where.right && (m_Where.height() - (m_BoxType == NO_BOX? 0 : 2)) < static_cast<int>(Items.size()) && (MouseEvent->dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED))
	{
		const auto WrapState = CheckFlags(VMENU_WRAPMODE);
		if (WrapState)
			ClearFlags(VMENU_WRAPMODE);
		SCOPE_EXIT{ if (WrapState) SetMenuFlags(VMENU_WRAPMODE); };

		if (MsY==SbY1)
		{
			// Press and hold the [▲] button
			while_mouse_button_pressed([&](DWORD const Button)
			{
				if (Button != FROM_LEFT_1ST_BUTTON_PRESSED)
					return false;

				ProcessKey(Manager::Key(KEY_UP));
				ShowMenu(true);
				return true;
			});

			return true;
		}

		if (MsY==SbY2)
		{
			// Press and hold the [▼] button
			while_mouse_button_pressed([&](DWORD const Button)
			{
				if (Button != FROM_LEFT_1ST_BUTTON_PRESSED)
					return false;

				ProcessKey(Manager::Key(KEY_DOWN));
				ShowMenu(true);
				return true;
			});

			return true;
		}

		if (MsY>SbY1 && MsY<SbY2)
		{
			// Drag the thumb
			int Delta=0;

			while (IsMouseButtonPressed() == FROM_LEFT_1ST_BUTTON_PRESSED)
			{
				const auto SbHeight = m_Where.height() - 3;
				int MsPos = (GetShowItemCount() - 1) * (IntKeyState.MousePos.y - m_Where.top) / SbHeight;

				if (MsPos >= GetShowItemCount())
				{
					MsPos=GetShowItemCount()-1;
					Delta=-1;
				}

				if (MsPos < 0)
				{
					MsPos=0;
					Delta=1;
				}


				SetSelectPos(VisualPosToReal(MsPos),Delta);

				ShowMenu(true);
			}

			return true;
		}
	}

	// dwButtonState & 3 - Left & Right button
	if (m_BoxType != NO_BOX && (MouseEvent->dwButtonState & 3) && MsX > m_Where.left && MsX < m_Where.right)
	{
		const auto WrapState = CheckFlags(VMENU_WRAPMODE);
		if (WrapState)
			ClearFlags(VMENU_WRAPMODE);
		SCOPE_EXIT{ if (WrapState) SetMenuFlags(VMENU_WRAPMODE); };

		if (MsY == m_Where.top)
		{
			while_mouse_button_pressed([&](DWORD)
			{
				if (MsY != m_Where.top || !GetVisualPos(SelectPos))
					return false;

				ProcessKey(Manager::Key(KEY_UP));
				return true;
			});

			return true;
		}

		if (MsY == m_Where.bottom)
		{
			while_mouse_button_pressed([&](DWORD)
			{
				if (MsY != m_Where.bottom || GetVisualPos(SelectPos) == GetShowItemCount() - 1)
					return false;

				ProcessKey(Manager::Key(KEY_DOWN));
				return true;
			});

			return true;
		}
	}

	if (m_BoxType==NO_BOX?
		MsX >= m_Where.left && MsX <= m_Where.right && MsY >= m_Where.top && MsY <= m_Where.bottom :
		MsX > m_Where.left && MsX < m_Where.right && MsY > m_Where.top && MsY < m_Where.bottom
		)
	{
		const auto MsPos = VisualPosToReal(GetVisualPos(TopPos) + MsY - m_Where.top - (m_BoxType == NO_BOX? 0 : 1));

		if (MsPos>=0 && MsPos<static_cast<int>(Items.size()) && ItemCanHaveFocus(Items[MsPos]))
		{
			if (IntKeyState.MousePos.x != IntKeyState.MousePrevPos.x || IntKeyState.MousePos.y != IntKeyState.MousePrevPos.y || !MouseEvent->dwEventFlags)
			{
				/* TODO:

				   Это заготовка для управления поведением листов "не в стиле меню" - когда текущий
				   указатель списка (позиция) следит за мышой...

				        if(!CheckFlags(VMENU_LISTBOX|VMENU_COMBOBOX) && MouseEvent->dwEventFlags==MOUSE_MOVED ||
				            CheckFlags(VMENU_LISTBOX|VMENU_COMBOBOX) && MouseEvent->dwEventFlags!=MOUSE_MOVED)
				*/
				if ((CheckFlags(VMENU_MOUSEREACTION) && MouseEvent->dwEventFlags==MOUSE_MOVED)
			        ||
			        (!CheckFlags(VMENU_MOUSEREACTION) && MouseEvent->dwEventFlags!=MOUSE_MOVED)
			        ||
			        (MouseEvent->dwButtonState & (FROM_LEFT_1ST_BUTTON_PRESSED|RIGHTMOST_BUTTON_PRESSED))
				   )
				{
					SetSelectPos(MsPos,1);
				}

				ShowMenu(true);
			}

			/* $ 13.10.2001 VVM
			  + Запомнить нажатие клавиши мышки и только в этом случае реагировать при отпускании */
			if (!MouseEvent->dwEventFlags && (MouseEvent->dwButtonState & (FROM_LEFT_1ST_BUTTON_PRESSED|RIGHTMOST_BUTTON_PRESSED)))
				SetMenuFlags(VMENU_MOUSEDOWN);

			if (!MouseEvent->dwEventFlags && !(MouseEvent->dwButtonState & (FROM_LEFT_1ST_BUTTON_PRESSED|RIGHTMOST_BUTTON_PRESSED)) && CheckFlags(VMENU_MOUSEDOWN))
			{
				ClearFlags(VMENU_MOUSEDOWN);
				ProcessKey(Manager::Key(KEY_ENTER));
			}
		}

		return true;
	}
	else if (m_BoxType!=NO_BOX && (MouseEvent->dwButtonState & 3) && !MouseEvent->dwEventFlags)
	{
		const auto ClickOpt = (MouseEvent->dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) ? Global->Opt->VMenu.LBtnClick : Global->Opt->VMenu.RBtnClick;
		if (ClickOpt==VMENUCLICK_CANCEL)
			ProcessKey(Manager::Key(KEY_ESC));

		return true;
	}
	else if (m_BoxType!=NO_BOX && !(MouseEvent->dwButtonState&FROM_LEFT_1ST_BUTTON_PRESSED) && (IntKeyState.PrevMouseButtonState&FROM_LEFT_1ST_BUTTON_PRESSED) && !MouseEvent->dwEventFlags && (Global->Opt->VMenu.LBtnClick==VMENUCLICK_APPLY))
	{
		ProcessKey(Manager::Key(KEY_ENTER));

		return true;
	}
	else if (m_BoxType!=NO_BOX && !(MouseEvent->dwButtonState&FROM_LEFT_2ND_BUTTON_PRESSED) && (IntKeyState.PrevMouseButtonState&FROM_LEFT_2ND_BUTTON_PRESSED) && !MouseEvent->dwEventFlags && (Global->Opt->VMenu.MBtnClick==VMENUCLICK_APPLY))
	{
		ProcessKey(Manager::Key(KEY_ENTER));

		return true;
	}
	else if (m_BoxType!=NO_BOX && bRightBtnPressed && !(MouseEvent->dwButtonState&RIGHTMOST_BUTTON_PRESSED) && (IntKeyState.PrevMouseButtonState&RIGHTMOST_BUTTON_PRESSED) && !MouseEvent->dwEventFlags && (Global->Opt->VMenu.RBtnClick==VMENUCLICK_APPLY))
	{
		ProcessKey(Manager::Key(KEY_ENTER));

		return true;
	}

	return false;
}

int VMenu::GetVisualPos(int Pos)
{
	if (!ItemHiddenCount)
		return Pos;

	if (Pos < 0)
		return -1;

	if (Pos >= static_cast<int>(Items.size()))
		return GetShowItemCount();

	return std::count_if(Items.cbegin(), Items.cbegin() + Pos, [](const auto& Item) { return ItemIsVisible(Item); });
}

int VMenu::VisualPosToReal(int VPos)
{
	if (!ItemHiddenCount)
		return VPos;

	if (VPos < 0)
		return -1;

	if (VPos >= GetShowItemCount())
		return static_cast<int>(Items.size());

	const auto ItemIterator = std::find_if(CONST_RANGE(Items, i) { return ItemIsVisible(i) && !VPos--; });
	return ItemIterator != Items.cend()? ItemIterator - Items.cbegin() : -1;
}

bool VMenu::ShiftItemShowPos(int Pos, int Direct)
{
	auto ItemShowPos = Items[Pos].ShowPos;

	const auto Len = VMFlags.Check(VMENU_SHOWAMPERSAND)? Items[Pos].Name.size() : HiStrlen(Items[Pos].Name);

	if (Len < MaxLineWidth || (Direct < 0 && !ItemShowPos) || (Direct > 0 && ItemShowPos > Len))
		return false;

	if (Direct < 0)
	{
		if (ItemShowPos)
			--ItemShowPos;
	}
	else
	{
		++ItemShowPos;
	}

	if (ItemShowPos + MaxLineWidth > Len)
		ItemShowPos = Len - MaxLineWidth;

	if (ItemShowPos != Items[Pos].ShowPos)
	{
		Items[Pos].ShowPos = ItemShowPos;
		VMFlags.Set(VMENU_UPDATEREQUIRED);
		return true;
	}

	return false;
}

void VMenu::Show()
{
	if (CheckFlags(VMENU_LISTBOX))
	{
		if (CheckFlags(VMENU_LISTSINGLEBOX))
			m_BoxType = SHORT_SINGLE_BOX;
		else if (CheckFlags(VMENU_SHOWNOBOX))
			m_BoxType = NO_BOX;
		else if (CheckFlags(VMENU_LISTHASFOCUS))
			m_BoxType = SHORT_DOUBLE_BOX;
		else
			m_BoxType = SHORT_SINGLE_BOX;
	}

	if (!CheckFlags(VMENU_LISTBOX))
	{
		bool AutoHeight = false;

		if (!CheckFlags(VMENU_COMBOBOX))
		{
			const auto HasSubMenus = ItemSubMenusCount > 0;
			bool AutoCenter = false;

			if (m_Where.left == -1)
			{
				m_Where.left = static_cast<short>(ScrX - m_MaxLength - 4 - (HasSubMenus? 1 : 0)) / 2;
				AutoCenter = true;
			}

			if (m_Where.left < 2)
				m_Where.left = 2;

			if (m_Where.right <= 0)
				m_Where.right = static_cast<short>(m_Where.left + m_MaxLength + 4 + (HasSubMenus? 1 : 0));

			if (!AutoCenter && m_Where.right > ScrX-4+2*(m_BoxType==SHORT_DOUBLE_BOX || m_BoxType==SHORT_SINGLE_BOX))
			{
				m_Where.left += ScrX - 4 - m_Where.right;
				m_Where.right = ScrX - 4;

				if (m_Where.left < 2)
				{
					m_Where.left = 2;
					m_Where.right = ScrX - 2;
				}
			}

			if (m_Where.right > ScrX - 2)
				m_Where.right = ScrX - 2;

			if (m_Where.top == -1)
			{
				if (MaxHeight && MaxHeight<GetShowItemCount())
					m_Where.top = (ScrY - MaxHeight - 2) / 2;
				else if ((m_Where.top = (ScrY - GetShowItemCount() - 2) / 2) < 0)
					m_Where.top = 0;

				AutoHeight=true;
			}
		}

		WasAutoHeight = false;
		if (m_Where.bottom <= 0)
		{
			WasAutoHeight = true;
			if (MaxHeight && MaxHeight<GetShowItemCount())
				m_Where.bottom = m_Where.top + MaxHeight + 1;
			else
				m_Where.bottom = m_Where.top + GetShowItemCount() + 1;
		}

		if (m_Where.bottom > ScrY)
			m_Where.bottom = ScrY;

		if (AutoHeight && m_Where.top < 3 && m_Where.bottom > ScrY - 3)
		{
			m_Where.top = 2;
			m_Where.bottom = ScrY - 2;
		}
	}

	if (m_Where.right > m_Where.left && m_Where.bottom + (CheckFlags(VMENU_SHOWNOBOX)? 1 : 0) > m_Where.top)
	{
		if (!CheckFlags(VMENU_LISTBOX))
		{
			ScreenObjectWithShadow::Show();
		}
		else
		{
			SetMenuFlags(VMENU_UPDATEREQUIRED);
			DisplayObject();
		}
	}
}

void VMenu::Hide()
{
	if (!CheckFlags(VMENU_LISTBOX) && SaveScr)
	{
		SaveScr.reset();
		ScreenObjectWithShadow::Hide();
	}

	SetMenuFlags(VMENU_UPDATEREQUIRED);
}

void VMenu::DisplayObject()
{
	const auto Parent = GetDialog();
	if (Parent && !Parent->IsRedrawEnabled()) return;
	ClearFlags(VMENU_UPDATEREQUIRED);

	if (CheckFlags(VMENU_REFILTERREQUIRED)!=0)
	{
		if (bFilterEnabled)
		{
			RestoreFilteredItems();
			if (!strFilter.empty())
				FilterStringUpdated();
		}
		ClearFlags(VMENU_REFILTERREQUIRED);
	}

	SetCursorType(false, 10);

	if (!CheckFlags(VMENU_LISTBOX) && !SaveScr)
	{
		if (!CheckFlags(VMENU_DISABLEDRAWBACKGROUND) && !(m_BoxType==SHORT_DOUBLE_BOX || m_BoxType==SHORT_SINGLE_BOX))
			SaveScr = std::make_unique<SaveScreen>(rectangle{ m_Where.left - 2, m_Where.top - 1, m_Where.right + 4, m_Where.bottom + 2 });
		else
			SaveScr = std::make_unique<SaveScreen>(rectangle{ m_Where.left, m_Where.top, m_Where.right + 2, m_Where.bottom + 1 });
	}

	if (!CheckFlags(VMENU_DISABLEDRAWBACKGROUND) && !CheckFlags(VMENU_LISTBOX))
	{
		if (m_BoxType==SHORT_DOUBLE_BOX || m_BoxType==SHORT_SINGLE_BOX)
		{
			SetScreen(m_Where, L' ', Colors[VMenuColorBody]);
			Box(m_Where, Colors[VMenuColorBox], m_BoxType);

			if (!CheckFlags(VMENU_LISTBOX|VMENU_ALWAYSSCROLLBAR))
			{
				MakeShadow({ m_Where.left + 2, m_Where.bottom + 1, m_Where.right + 1, m_Where.bottom + 1 });
				MakeShadow({ m_Where.right + 1, m_Where.top + 1, m_Where.right + 2, m_Where.bottom + 1 });
			}
		}
		else
		{
			if (m_BoxType!=NO_BOX)
				SetScreen({ m_Where.left - 2, m_Where.top - 1, m_Where.right + 2, m_Where.bottom + 1 }, L' ', Colors[VMenuColorBody]);
			else
				SetScreen(m_Where, L' ', Colors[VMenuColorBody]);

			if (!CheckFlags(VMENU_LISTBOX|VMENU_ALWAYSSCROLLBAR))
			{
				MakeShadow({ m_Where.left, m_Where.bottom + 2, m_Where.right + 3, m_Where.bottom + 2 });
				MakeShadow({ m_Where.right + 3, m_Where.top, m_Where.right + 4, m_Where.bottom + 2 });
			}

			if (m_BoxType!=NO_BOX)
				Box(m_Where, Colors[VMenuColorBox], m_BoxType);
		}

		//SetMenuFlags(VMENU_DISABLEDRAWBACKGROUND);
	}

	if (!CheckFlags(VMENU_LISTBOX))
		DrawTitles();

	ShowMenu(true);
}

void VMenu::DrawTitles() const
{
	if (CheckFlags(VMENU_SHOWNOBOX)) return;

	const auto MaxTitleLength = m_Where.width() - 3;
	int WidthTitle;

	if (!strTitle.empty() || bFilterEnabled)
	{
		string strDisplayTitle = strTitle;

		if (bFilterEnabled)
		{
			if (bFilterLocked)
				strDisplayTitle += L' ';
			else
				strDisplayTitle.clear();

			append(strDisplayTitle, bFilterLocked? L'<' : L'[', strFilter, bFilterLocked? L'>' : L']');
		}

		WidthTitle = static_cast<int>(strDisplayTitle.size());

		if (WidthTitle > MaxTitleLength)
			WidthTitle = MaxTitleLength - 1;

		GotoXY(m_Where.left + (m_Where.width() - 2 - WidthTitle) / 2, m_Where.top);
		SetColor(Colors[VMenuColorTitle]);

		Text(concat(L' ', string_view(strDisplayTitle).substr(0, WidthTitle), L' '));
	}

	if (!strBottomTitle.empty())
	{
		WidthTitle = static_cast<int>(strBottomTitle.size());

		if (WidthTitle > MaxTitleLength)
			WidthTitle = MaxTitleLength - 1;

		GotoXY(m_Where.left + (m_Where.width() - 2 - WidthTitle) / 2, m_Where.bottom);
		SetColor(Colors[VMenuColorTitle]);

		Text(concat(L' ', string_view(strBottomTitle).substr(0, WidthTitle), L' '));
	}
}

void VMenu::ShowMenu(bool IsParent)
{
	size_t MaxItemLength = 0;

	//BUGBUG, this must be optimized
	for (const auto& i: Items)
	{
		MaxItemLength = std::max(MaxItemLength, CheckFlags(VMENU_SHOWAMPERSAND)? i.Name.size() : HiStrlen(i.Name));
	}

	MaxLineWidth = m_Where.width();

	if (m_BoxType != NO_BOX)
		MaxLineWidth -= 2; // frame

	MaxLineWidth -= 2; // check mark + left horz. scroll

	if (/*!CheckFlags(VMENU_COMBOBOX|VMENU_LISTBOX) && */ ItemSubMenusCount > 0)
		MaxLineWidth -= 1; // sub menu arrow

	if ((CheckFlags(VMENU_LISTBOX | VMENU_ALWAYSSCROLLBAR) || Global->Opt->ShowMenuScrollbar) && m_BoxType == NO_BOX && ScrollBarRequired(m_Where.height(), GetShowItemCount()))
		MaxLineWidth -= 1; // scrollbar

	if (MaxItemLength > MaxLineWidth)
	{
		MaxLineWidth -= 1; // right horz. scroll
	}

	MaxLineWidth = std::max(MaxLineWidth, size_t(0));

	if (m_Where.right <= m_Where.left || m_Where.bottom <= m_Where.top)
	{
		if (!(CheckFlags(VMENU_SHOWNOBOX) && m_Where.bottom == m_Where.top))
			return;
	}

	if (CheckFlags(VMENU_LISTBOX))
	{
		if (CheckFlags(VMENU_LISTSINGLEBOX))
			m_BoxType = SHORT_SINGLE_BOX;
		else if (CheckFlags(VMENU_SHOWNOBOX))
			m_BoxType = NO_BOX;
		else if (CheckFlags(VMENU_LISTHASFOCUS))
			m_BoxType = SHORT_DOUBLE_BOX;
		else
			m_BoxType = SHORT_SINGLE_BOX;

		if (!IsParent || !GetShowItemCount())
		{
			if (GetShowItemCount())
				m_BoxType=CheckFlags(VMENU_SHOWNOBOX)?NO_BOX:SHORT_SINGLE_BOX;

			SetScreen(m_Where, L' ', Colors[VMenuColorBody]);
		}

		if (m_BoxType!=NO_BOX)
			Box(m_Where, Colors[VMenuColorBox], m_BoxType);

		DrawTitles();
	}

	wchar_t BoxChar[2]={};

	switch (m_BoxType)
	{
		case NO_BOX:
			*BoxChar=L' ';
			break;

		case SINGLE_BOX:
		case SHORT_SINGLE_BOX:
			*BoxChar=BoxSymbols[BS_V1];
			break;

		case DOUBLE_BOX:
		case SHORT_DOUBLE_BOX:
			*BoxChar=BoxSymbols[BS_V2];
			break;
	}

	if (GetShowItemCount() <= 0)
		return;

	if (CheckFlags(VMENU_AUTOHIGHLIGHT))
		AssignHighlights(CheckFlags(VMENU_REVERSEHIGHLIGHT));

	int VisualSelectPos = GetVisualPos(SelectPos);
	int VisualTopPos = GetVisualPos(TopPos);

	// коррекция Top`а
	if (VisualTopPos+GetShowItemCount() >= m_Where.height() - 1 && VisualSelectPos == GetShowItemCount()-1)
	{
		VisualTopPos--;

		if (VisualTopPos<0)
			VisualTopPos=0;
	}

	VisualTopPos = std::min(VisualTopPos, GetShowItemCount() - (m_Where.height() - 2 - (m_BoxType == NO_BOX ? 2 : 0)));

	if (VisualSelectPos > VisualTopPos + (m_Where.height() - 1 - (m_BoxType == NO_BOX? 0 : 2)))
	{
		VisualTopPos = VisualSelectPos - (m_Where.height() - 1 - (m_BoxType == NO_BOX? 0 : 2));
	}

	if (VisualSelectPos < VisualTopPos)
	{
		TopPos=SelectPos;
		VisualTopPos=VisualSelectPos;
	}
	else
	{
		TopPos=VisualPosToReal(VisualTopPos);
	}

	if (VisualTopPos<0)
		VisualTopPos=0;

	if (TopPos<0)
		TopPos=0;

	for (int Y = m_Where.top + (m_BoxType == NO_BOX? 0 : 1), I = TopPos; Y < m_Where.bottom + (m_BoxType == NO_BOX? 1 : 0); ++Y, ++I)
	{
		GotoXY(m_Where.left, Y);

		if (I < static_cast<int>(Items.size()))
		{
			if (!ItemIsVisible(Items[I]))
			{
				Y--;
				continue;
			}

			if (Items[I].Flags&LIF_SEPARATOR)
			{
				int SepWidth = m_Where.width();

				auto strTmpStr = MakeLine(SepWidth, m_BoxType == NO_BOX? line_type::h1_to_none : (m_BoxType == SINGLE_BOX || m_BoxType == SHORT_SINGLE_BOX? line_type::h1_to_v1 : line_type::h1_to_v2));

				if (!CheckFlags(VMENU_NOMERGEBORDER) && SepWidth > 3)
				{
					for (size_t J = 0; J < strTmpStr.size() - 3; ++J)
					{
						const auto AnyPrev = I > 0;
						const auto AnyNext = I < static_cast<int>(Items.size() - 1);

						const auto PCorrection = AnyPrev && !CheckFlags(VMENU_SHOWAMPERSAND)? HiFindRealPos(Items[I - 1].Name, J) - J : 0;
						const auto NCorrection = AnyNext && !CheckFlags(VMENU_SHOWAMPERSAND)? HiFindRealPos(Items[I + 1].Name, J) - J : 0;

						wchar_t PrevItem = (AnyPrev && Items[I - 1].Name.size() > J + PCorrection)? Items[I - 1].Name[J + PCorrection] : 0;
						wchar_t NextItem = (AnyNext && Items[I + 1].Name.size() > J + NCorrection)? Items[I + 1].Name[J + NCorrection] : 0;

						if (!PrevItem && !NextItem)
							break;

						if (PrevItem==BoxSymbols[BS_V1])
						{
							if (NextItem==BoxSymbols[BS_V1])
								strTmpStr[J+(m_BoxType==NO_BOX?1:2) + 1] = BoxSymbols[BS_C_H1V1];
							else
								strTmpStr[J+(m_BoxType==NO_BOX?1:2) + 1] = BoxSymbols[BS_B_H1V1];
						}
						else if (NextItem==BoxSymbols[BS_V1])
						{
							strTmpStr[J+(m_BoxType==NO_BOX?1:2) + 1] = BoxSymbols[BS_T_H1V1];
						}
					}
				}

				SetColor(Colors[VMenuColorSeparator]);
				BoxText(strTmpStr, false);

				if (!Items[I].Name.empty())
				{
					auto ItemWidth = static_cast<int>(Items[I].Name.size());

					if (ItemWidth + 2 > m_Where.width())
						ItemWidth = m_Where.width() - 2;

					GotoXY(m_Where.left + (m_Where.width() - 2 - ItemWidth) / 2, Y);
					Text(concat(L' ', fit_to_left(Items[I].Name, ItemWidth), L' '));
				}
			}
			else
			{
				if (m_BoxType!=NO_BOX)
				{
					SetColor(Colors[VMenuColorBox]);
					BoxText(BoxChar);
					GotoXY(m_Where.right, Y);
					BoxText(BoxChar);
				}

				GotoXY(m_Where.left + (m_BoxType == NO_BOX? 0 : 1), Y);

				FarColor CurColor;
				if ((Items[I].Flags&LIF_SELECTED))
					CurColor = VMenu::Colors[Items[I].Flags&LIF_GRAYED?VMenuColorSelGrayed:VMenuColorSelected];
				else
					CurColor = VMenu::Colors[Items[I].Flags&LIF_DISABLE?VMenuColorDisabled:(Items[I].Flags&LIF_GRAYED?VMenuColorGrayed:VMenuColorText)];

				SetColor(CurColor);

				string strMenuLine;
				wchar_t CheckMark = L' ';

				if (Items[I].Flags & LIF_CHECKED)
				{
					if (!(Items[I].Flags & 0x0000FFFF))
						CheckMark = 0x221A;
					else
						CheckMark = static_cast<wchar_t>(Items[I].Flags & 0x0000FFFF);
				}

				strMenuLine.push_back(CheckMark);
				strMenuLine.push_back(L' '); // left scroller (<<) placeholder
				const auto PrefixSize = strMenuLine.size();

				size_t HotkeyVisualPos = string::npos;
				auto MenuItemForDisplay = CheckFlags(VMENU_SHOWAMPERSAND)? Items[I].Name : HiText2Str(Items[I].Name, &HotkeyVisualPos);

				MenuItemForDisplay.erase(0, Items[I].ShowPos);

				if (HotkeyVisualPos != string::npos)
				{
					if (HotkeyVisualPos < Items[I].ShowPos)
						HotkeyVisualPos = string::npos;
					else
						HotkeyVisualPos -= Items[I].ShowPos;
				}

				// fit menu string into available space
				bool ShowRightScroller = false;
				if (MenuItemForDisplay.size() > MaxLineWidth)
				{
					MenuItemForDisplay.resize(MaxLineWidth);
					ShowRightScroller = true;
				}

				strMenuLine += MenuItemForDisplay;

				// табуляции меняем только при показе!!!
				// для сохранение оригинальной строки!!!
				std::replace(ALL_RANGE(strMenuLine), L'\t', L' ');

				FarColor Col;

				if (!(Items[I].Flags & LIF_DISABLE))
				{
					static const vmenu_colors ItemColors[][2] =
					{
						{ VMenuColorHighlight, VMenuColorHSelect },
						{ VMenuColorGrayed, VMenuColorSelGrayed },
					};

					const auto Index = ItemColors[Items[I].Flags & LIF_GRAYED ? 1 : 0][Items[I].Flags & LIF_SELECTED ? 1 : 0];
					Col = Colors[Index];
				}
				else
				{
					Col = Colors[VMenuColorDisabled];
				}

				if (!Items[I].Annotations.empty())
				{
						int StartOffset = 1; // 1 is '<<' placeholder size
						size_t Pos = 0;
						FarColor InvColor = CurColor;
						using std::swap;
						swap(InvColor.ForegroundColor, InvColor.BackgroundColor);
						for (const auto& [AnnPos, AnnSize]: Items[I].Annotations)
						{
							const size_t pre_len = AnnPos - Items[I].ShowPos + StartOffset - Pos + 1;
							if (Pos < strMenuLine.size())
							{
								Text(string_view(strMenuLine).substr(Pos, pre_len));
								Pos += pre_len;
								if (Pos < strMenuLine.size())
								{
									SetColor(Col);
									Text(string_view(strMenuLine).substr(Pos, AnnSize));
									Pos += AnnSize;
									SetColor(CurColor);
								}
							}
						}
						if (Pos < strMenuLine.size())
							Text(string_view(strMenuLine).substr(Pos));
				}
				else
				{
					if (HotkeyVisualPos != string::npos || Items[I].AutoHotkey)
					{
						const auto HotkeyPos = (HotkeyVisualPos != string::npos? HotkeyVisualPos : Items[I].AutoHotkeyPos) + PrefixSize;

						Text(string_view(strMenuLine).substr(0, HotkeyPos));

						if (HotkeyPos < strMenuLine.size())
						{
							const auto SaveColor = CurColor;
							SetColor(Col);
							Text(strMenuLine[HotkeyPos]);
							SetColor(SaveColor);
							Text(string_view(strMenuLine).substr(HotkeyPos + 1));
						}
					}
					else
					{
						Text(strMenuLine);
					}
				}

				// сделаем добавочку для NO_BOX
				{
					int Width = m_Where.right - WhereX() + (m_BoxType == NO_BOX? 1 : 0);
					if (Width > 0)
						Text(string(Width, L' '));
				}

				if (Items[I].Flags & MIF_SUBMENU)
				{
					GotoXY(static_cast<int>(m_Where.right - 1), Y);
					BoxText(L'\x25BA'); // sub menu arrow
				}

				SetColor(Colors[(Items[I].Flags&LIF_DISABLE)?VMenuColorArrowsDisabled:(Items[I].Flags&LIF_SELECTED?VMenuColorArrowsSelect:VMenuColorArrows)]);

				if (Items[I].ShowPos)
				{
					GotoXY(m_Where.left + (m_BoxType == NO_BOX? 0 : 1) + 1, Y);
					BoxText(L'\xab'); // '<<'
				}

				if (ShowRightScroller)
				{
					GotoXY(static_cast<int>(m_Where.left + (m_BoxType == NO_BOX? 0 : 1) + 2 + MaxLineWidth), Y);
					BoxText(L'\xbb'); // '>>'
				}
			}
		}
		else
		{
			if (m_BoxType!=NO_BOX)
			{
				SetColor(Colors[VMenuColorBox]);
				BoxText(BoxChar);
				GotoXY(m_Where.right, Y);
				BoxText(BoxChar);
				GotoXY(m_Where.left + 1, Y);
			}
			else
			{
				GotoXY(m_Where.left, Y);
			}

			SetColor(Colors[VMenuColorText]);
			// сделаем добавочку для NO_BOX
			const auto Size = m_Where.width() - (m_BoxType == NO_BOX? 0 : 2);
			Text(string(Size, L' '));
		}
	}

	if (CheckFlags(VMENU_LISTBOX|VMENU_ALWAYSSCROLLBAR) || Global->Opt->ShowMenuScrollbar)
	{
		SetColor(Colors[VMenuColorScrollBar]);

		if (m_BoxType!=NO_BOX)
			ScrollBar(m_Where.right, m_Where.top + 1, m_Where.height() - 2, VisualTopPos, GetShowItemCount());
		else
			ScrollBar(m_Where.right, m_Where.top, m_Where.height(), VisualTopPos, GetShowItemCount());
	}
}

int VMenu::CheckHighlights(wchar_t CheckSymbol, int StartPos)
{
	if (CheckSymbol)
		CheckSymbol=upper(CheckSymbol);

	for (size_t I=StartPos; I < Items.size(); I++)
	{
		if (!ItemIsVisible(Items[I]))
			continue;

		const auto Ch = GetHighlights(&Items[I]);

		if (Ch)
		{
			if (CheckSymbol == upper(Ch) || CheckSymbol == upper(KeyToKeyLayout(Ch)))
				return static_cast<int>(I);
		}
		else if (!CheckSymbol)
		{
			return static_cast<int>(I);
		}
	}

	return -1;
}

wchar_t VMenu::GetHighlights(const MenuItemEx* const Item) const
{
	if (!Item)
		return 0;

	if (Item->AutoHotkey)
		return Item->AutoHotkey;

	if (CheckFlags(VMENU_SHOWAMPERSAND))
		return 0;

	wchar_t Ch;
	return HiTextHotkey(Item->Name, Ch)? Ch : 0;
}

void VMenu::AssignHighlights(bool Reverse)
{
	static_assert(sizeof(wchar_t) == 2, "512 MB for a bitset is too much, rewrite it.");
	std::bitset<std::numeric_limits<wchar_t>::max() + 1> Used;

	const auto Delta = Reverse? -1 : 1;

	const auto RegisterHotkey = [&Used](wchar_t Hotkey)
	{
		const auto OtherHotkey = KeyToKeyLayout(Hotkey);
		Used[upper(OtherHotkey)] = true;
		Used[lower(OtherHotkey)] = true;
		Used[upper(Hotkey)] = true;
		Used[lower(Hotkey)] = true;
	};

	const auto ShowAmpersand = CheckFlags(VMENU_SHOWAMPERSAND);
	// проверка заданных хоткеев
	for (int I = Reverse? static_cast<int>(Items.size() - 1) : 0; I >= 0 && I < static_cast<int>(Items.size()); I += Delta)
	{
		wchar_t Hotkey{};
		size_t HotkeyVisualPos{};
		// TODO: проверка на LIF_HIDDEN
		if (!ShowAmpersand && HiTextHotkey(Items[I].Name, Hotkey, &HotkeyVisualPos) && !Used[upper(Hotkey)] && !Used[lower(Hotkey)])
		{
			RegisterHotkey(Hotkey);
			Items[I].AutoHotkey = Hotkey;
			Items[I].AutoHotkeyPos = HotkeyVisualPos;
		}
	}

	// TODO:  ЭТОТ цикл нужно уточнить - возможно вылезут артефакты (хотя не уверен)
	for (int I = Reverse? static_cast<int>(Items.size() - 1) : 0; I >= 0 && I < static_cast<int>(Items.size()); I += Delta)
	{
		size_t HotkeyVisualPos;
		auto MenuItemForDisplay = HiText2Str(Items[I].Name, &HotkeyVisualPos);
		if (!ShowAmpersand && HotkeyVisualPos != string::npos)
			continue;

		MenuItemForDisplay.erase(0, Items[I].ShowPos);

		// TODO: проверка на LIF_HIDDEN
		for (const auto& Ch: MenuItemForDisplay)
		{
			if ((Ch == L'&' || is_alpha(Ch) || std::iswdigit(Ch)) && !Used[upper(Ch)] && !Used[lower(Ch)])
			{
				RegisterHotkey(Ch);
				Items[I].AutoHotkey = Ch;
				Items[I].AutoHotkeyPos = &Ch - MenuItemForDisplay.data();
				break;
			}
		}
	}

	SetMenuFlags(VMENU_AUTOHIGHLIGHT | (Reverse? VMENU_REVERSEHIGHLIGHT : VMENU_NONE));
}

bool VMenu::CheckKeyHiOrAcc(DWORD Key, int Type, bool Translate, bool ChangePos, int& NewPos)
{
	//не забудем сбросить EndLoop для листбокса, иначе не будут работать хоткеи в активном списке
	if (CheckFlags(VMENU_LISTBOX))
		ClearDone();

	FOR_CONST_RANGE(Items, Iterator)
	{
		auto& CurItem = *Iterator;
		if (ItemCanHaveFocus(CurItem) && ((!Type && CurItem.AccelKey && Key == CurItem.AccelKey) || (Type && (CurItem.AutoHotkey || !CheckFlags(VMENU_SHOWAMPERSAND)) && IsKeyHighlighted(CurItem.Name, Key, Translate, CurItem.AutoHotkey))))
		{
			NewPos=static_cast<int>(Iterator - Items.cbegin());
			if (ChangePos)
			{
				SetSelectPos(NewPos, 1);
				ShowMenu(true);
			}

			if ((!GetDialog() || CheckFlags(VMENU_COMBOBOX|VMENU_LISTBOX)) && ItemCanBeEntered(Items[SelectPos]))
			{
				SetExitCode(NewPos);
			}

			return true;
		}
	}

	return Done();
}

void VMenu::UpdateMaxLengthFromTitles()
{
	//тайтл + 2 пробела вокруг
	UpdateMaxLength(std::max(strTitle.size(), strBottomTitle.size()) + 2);
}

void VMenu::UpdateMaxLength(size_t Length)
{
	if (Length > m_MaxLength)
		m_MaxLength = Length;

	if (m_MaxLength + 8 > static_cast<size_t>(ScrX))
		m_MaxLength = std::max(0, ScrX - 8);
}

void VMenu::SetMaxHeight(int NewMaxHeight)
{
	MaxHeight = NewMaxHeight;

	if (MaxHeight > ScrY-6)
		MaxHeight = ScrY-6;
}

string VMenu::GetTitle() const
{
	return strTitle;
}

string &VMenu::GetBottomTitle(string &strDest) const
{
	strDest = strBottomTitle;
	return strDest;
}

void VMenu::SetBottomTitle(string_view const BottomTitle)
{
	SetMenuFlags(VMENU_UPDATEREQUIRED);

	strBottomTitle = BottomTitle;

	UpdateMaxLength(strBottomTitle.size() + 2);
}

void VMenu::SetTitle(string_view const Title)
{
	SetMenuFlags(VMENU_UPDATEREQUIRED);

	strTitle = Title;

	UpdateMaxLength(strTitle.size() + 2);
}

void VMenu::ResizeConsole()
{
	if (SaveScr)
	{
		SaveScr->Discard();
		SaveScr.reset();
	}
}

void VMenu::SetDeleting()
{
}

void VMenu::ShowConsoleTitle()
{
	if (CheckFlags(VMENU_CHANGECONSOLETITLE))
	{
		ConsoleTitle::SetFarTitle(strTitle);
	}
}

void VMenu::OnClose()
{
	EnableFilter(false);
}

void VMenu::SetBoxType(int BoxType)
{
	m_BoxType=BoxType;
}

void VMenu::SetColors(const FarDialogItemColors *ColorsIn)
{
	if (ColorsIn)
	{
		std::copy_n(ColorsIn->Colors, std::min(std::size(Colors), ColorsIn->ColorsCount), Colors);
	}
	else
	{
		const auto TypeMenu  = CheckFlags(VMENU_LISTBOX)? 0 : (CheckFlags(VMENU_COMBOBOX)? 1 : 2);
		const auto StyleMenu = CheckFlags(VMENU_WARNDIALOG)? 1 : 0;

		if (CheckFlags(VMENU_DISABLED))
		{
			std::fill_n(Colors, size_t(VMENU_COLOR_COUNT), colors::PaletteColorToFarColor(StyleMenu? COL_WARNDIALOGDISABLED : COL_DIALOGDISABLED));
		}
		else
		{
			static const PaletteColors StdColor[2][3][VMENU_COLOR_COUNT]=
			{
				// Not VMENU_WARNDIALOG
				{
					// VMENU_LISTBOX
					{
						COL_DIALOGLISTTEXT,                        // подложка
						COL_DIALOGLISTBOX,                         // рамка
						COL_DIALOGLISTTITLE,                       // заголовок - верхний и нижний
						COL_DIALOGLISTTEXT,                        // Текст пункта
						COL_DIALOGLISTHIGHLIGHT,                   // HotKey
						COL_DIALOGLISTBOX,                         // separator
						COL_DIALOGLISTSELECTEDTEXT,                // Выбранный
						COL_DIALOGLISTSELECTEDHIGHLIGHT,           // Выбранный - HotKey
						COL_DIALOGLISTSCROLLBAR,                   // ScrollBar
						COL_DIALOGLISTDISABLED,                    // Disabled
						COL_DIALOGLISTARROWS,                      // Arrow
						COL_DIALOGLISTARROWSSELECTED,              // Выбранный - Arrow
						COL_DIALOGLISTARROWSDISABLED,              // Arrow Disabled
						COL_DIALOGLISTGRAY,                        // "серый"
						COL_DIALOGLISTSELECTEDGRAYTEXT,            // выбранный "серый"
					},
					// VMENU_COMBOBOX
					{
						COL_DIALOGCOMBOTEXT,                       // подложка
						COL_DIALOGCOMBOBOX,                        // рамка
						COL_DIALOGCOMBOTITLE,                      // заголовок - верхний и нижний
						COL_DIALOGCOMBOTEXT,                       // Текст пункта
						COL_DIALOGCOMBOHIGHLIGHT,                  // HotKey
						COL_DIALOGCOMBOBOX,                        // separator
						COL_DIALOGCOMBOSELECTEDTEXT,               // Выбранный
						COL_DIALOGCOMBOSELECTEDHIGHLIGHT,          // Выбранный - HotKey
						COL_DIALOGCOMBOSCROLLBAR,                  // ScrollBar
						COL_DIALOGCOMBODISABLED,                   // Disabled
						COL_DIALOGCOMBOARROWS,                     // Arrow
						COL_DIALOGCOMBOARROWSSELECTED,             // Выбранный - Arrow
						COL_DIALOGCOMBOARROWSDISABLED,             // Arrow Disabled
						COL_DIALOGCOMBOGRAY,                       // "серый"
						COL_DIALOGCOMBOSELECTEDGRAYTEXT,           // выбранный "серый"
					},
					// VMenu
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
					}
				},

				// VMENU_WARNDIALOG
				{
					// VMENU_LISTBOX
					{
						COL_WARNDIALOGLISTTEXT,                    // подложка
						COL_WARNDIALOGLISTBOX,                     // рамка
						COL_WARNDIALOGLISTTITLE,                   // заголовок - верхний и нижний
						COL_WARNDIALOGLISTTEXT,                    // Текст пункта
						COL_WARNDIALOGLISTHIGHLIGHT,               // HotKey
						COL_WARNDIALOGLISTBOX,                     // separator
						COL_WARNDIALOGLISTSELECTEDTEXT,            // Выбранный
						COL_WARNDIALOGLISTSELECTEDHIGHLIGHT,       // Выбранный - HotKey
						COL_WARNDIALOGLISTSCROLLBAR,               // ScrollBar
						COL_WARNDIALOGLISTDISABLED,                // Disabled
						COL_WARNDIALOGLISTARROWS,                  // Arrow
						COL_WARNDIALOGLISTARROWSSELECTED,          // Выбранный - Arrow
						COL_WARNDIALOGLISTARROWSDISABLED,          // Arrow Disabled
						COL_WARNDIALOGLISTGRAY,                    // "серый"
						COL_WARNDIALOGLISTSELECTEDGRAYTEXT,        // выбранный "серый"
					},
					// VMENU_COMBOBOX
					{
						COL_WARNDIALOGCOMBOTEXT,                   // подложка
						COL_WARNDIALOGCOMBOBOX,                    // рамка
						COL_WARNDIALOGCOMBOTITLE,                  // заголовок - верхний и нижний
						COL_WARNDIALOGCOMBOTEXT,                   // Текст пункта
						COL_WARNDIALOGCOMBOHIGHLIGHT,              // HotKey
						COL_WARNDIALOGCOMBOBOX,                    // separator
						COL_WARNDIALOGCOMBOSELECTEDTEXT,           // Выбранный
						COL_WARNDIALOGCOMBOSELECTEDHIGHLIGHT,      // Выбранный - HotKey
						COL_WARNDIALOGCOMBOSCROLLBAR,              // ScrollBar
						COL_WARNDIALOGCOMBODISABLED,               // Disabled
						COL_WARNDIALOGCOMBOARROWS,                 // Arrow
						COL_WARNDIALOGCOMBOARROWSSELECTED,         // Выбранный - Arrow
						COL_WARNDIALOGCOMBOARROWSDISABLED,         // Arrow Disabled
						COL_WARNDIALOGCOMBOGRAY,                   // "серый"
						COL_WARNDIALOGCOMBOSELECTEDGRAYTEXT,       // выбранный "серый"
					},
					// VMenu
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
					}
				}
			};

			std::transform(StdColor[StyleMenu][TypeMenu], StdColor[StyleMenu][TypeMenu] + VMENU_COLOR_COUNT, Colors, colors::PaletteColorToFarColor);
		}
	}
}

void VMenu::GetColors(FarDialogItemColors *ColorsOut)
{
	std::copy_n(Colors, std::min(std::size(Colors), ColorsOut->ColorsCount), ColorsOut->Colors);
}

void VMenu::SetOneColor(int Index, PaletteColors Color)
{
	if (Index < static_cast<int>(std::size(Colors)))
		Colors[Index] = colors::PaletteColorToFarColor(Color);
}

bool VMenu::GetVMenuInfo(FarListInfo* Info) const
{
	if (Info)
	{
		Info->Flags = GetFlags() & (LINFO_SHOWNOBOX|LINFO_AUTOHIGHLIGHT|LINFO_REVERSEHIGHLIGHT|LINFO_WRAPMODE|LINFO_SHOWAMPERSAND);
		Info->ItemsNumber = Items.size();
		Info->SelectPos = SelectPos;
		Info->TopPos = TopPos;
		Info->MaxHeight = MaxHeight;
		Info->MaxLength = m_MaxLength + (ItemSubMenusCount > 0? 1 : 0);
		return true;
	}

	return false;
}

// Функция GetItemPtr - получить указатель на нужный Items.
MenuItemEx& VMenu::at(size_t n)
{
	const auto ItemPos = GetItemPosition(static_cast<int>(n));

	if (ItemPos < 0)
		throw MAKE_FAR_FATAL_EXCEPTION(L"menu index out of range"sv);

	return Items[ItemPos];
}

intptr_t VMenu::GetSimpleUserData(int Position) const
{
	const auto ItemPos = GetItemPosition(Position);
	if (ItemPos < 0 || static_cast<size_t>(ItemPos) >= Items.size())
		return 0;

	return Items[ItemPos].SimpleUserData;
}

// Присовокупить к элементу данные.
void VMenu::SetComplexUserData(const std::any& Data, int Position)
{
	const auto ItemPos = GetItemPosition(Position);
	if (ItemPos < 0 || static_cast<size_t>(ItemPos) >= Items.size())
		return;

	Items[ItemPos].ComplexUserData = Data;
}

// Получить данные
std::any* VMenu::GetComplexUserData(int Position)
{
	const auto ItemPos = GetItemPosition(Position);
	if (ItemPos < 0 || static_cast<size_t>(ItemPos) >= Items.size())
		return nullptr;

	return &Items[ItemPos].ComplexUserData;
}

FarListItem *VMenu::MenuItem2FarList(const MenuItemEx *MItem, FarListItem *FItem)
{
	if (FItem && MItem)
	{
		*FItem = {};
		FItem->Flags = MItem->Flags;
		FItem->Text = MItem->Name.c_str();
		FItem->UserData = MItem->SimpleUserData;
		return FItem;
	}

	return nullptr;
}

FARMACROAREA VMenu::GetMacroArea() const
{
	if (IsComboBox())
		return MACROAREA_DIALOG;
	return SimpleModal::GetMacroArea();
}

int VMenu::GetTypeAndName(string &strType, string &strName)
{
	strType = msg(lng::MVMenuType);
	strName = strTitle;
	return CheckFlags(VMENU_COMBOBOX) ? windowtype_combobox : windowtype_menu;
}

// return Pos || -1
int VMenu::FindItem(const FarListFind *FItem)
{
	return FindItem(FItem->StartIndex,FItem->Pattern,FItem->Flags);
}

int VMenu::FindItem(int StartIndex, string_view const Pattern, unsigned long long Flags)
{
	if (static_cast<size_t>(StartIndex) < Items.size())
	{
		for (size_t I=StartIndex; I < Items.size(); I++)
		{
			string strTmpBuf(Items[I].Name);
			RemoveHighlights(strTmpBuf);

			if (Flags&LIFIND_EXACTMATCH)
			{
				if (starts_with_icase(strTmpBuf, Pattern))
					return static_cast<int>(I);
			}
			else
			{
				if (CmpName(Pattern, strTmpBuf, true))
					return static_cast<int>(I);
			}
		}
	}

	return -1;
}

// Сортировка элементов списка
// Offset - начало сравнения! по умолчанию =0
void VMenu::SortItems(bool Reverse, int Offset)
{
	SortItems([](const MenuItemEx& a, const MenuItemEx& b, const SortItemParam& Param)
	{
		string strName1(a.Name);
		string strName2(b.Name);
		RemoveHighlights(strName1);
		RemoveHighlights(strName2);
		const auto Less = string_sort::less(string_view(strName1).substr(Param.Offset), string_view(strName2).substr(Param.Offset));
		return Param.Reverse? !Less : Less;
	}, Reverse, Offset);
}

bool VMenu::Pack()
{
	const auto OldItemCount = Items.size();
	size_t FirstIndex=0;

	while (FirstIndex<Items.size())
	{
		size_t LastIndex=Items.size()-1;
		while (LastIndex>FirstIndex)
		{
			if (!(Items[FirstIndex].Flags & LIF_SEPARATOR) && !(Items[LastIndex].Flags & LIF_SEPARATOR))
			{
				if (Items[FirstIndex].Name == Items[LastIndex].Name)
				{
					DeleteItem(static_cast<int>(LastIndex));
				}
			}
			LastIndex--;
		}
		FirstIndex++;
	}
	return OldItemCount != Items.size();
}

void VMenu::SetId(const UUID& Id)
{
	MenuId=Id;
}

const UUID& VMenu::Id() const
{
	return MenuId;
}

std::vector<string> VMenu::AddHotkeys(span<menu_item> const MenuItems)
{
	FN_RETURN_TYPE(VMenu::AddHotkeys) Result(MenuItems.size());

	const size_t MaxLength = std::accumulate(ALL_CONST_RANGE(MenuItems), size_t(0), [](size_t Value, const auto& i)
	{
		return std::max(Value, i.Name.size());
	});

	for (const auto& [Item, Str]: zip(MenuItems, Result))
	{
		if (Item.Flags & LIF_SEPARATOR || !Item.AccelKey)
			continue;

		const auto Key = KeyToLocalizedText(Item.AccelKey);
		const auto Hl = HiStrlen(Item.Name) != Item.Name.size();
		Str = fit_to_left(Item.Name, MaxLength + (Hl? 2 : 1)) + Key;
		Item.Name = Str;
	}

	return Result;
}

void VMenu::EnableFilter(bool const Enable)
{
	bFilterEnabled = Enable;
	bFilterLocked = false;
	strFilter.clear();

	if (!Enable)
		RestoreFilteredItems();
}
