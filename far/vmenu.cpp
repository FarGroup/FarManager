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

#include "headers.hpp"
#pragma hdrstop

#include "vmenu.hpp"
#include "keyboard.hpp"
#include "keys.hpp"
#include "macroopcode.hpp"
#include "colors.hpp"
#include "chgprior.hpp"
#include "dialog.hpp"
#include "savescr.hpp"
#include "clipboard.hpp"
#include "ctrlobj.hpp"
#include "manager.hpp"
#include "constitle.hpp"
#include "syslog.hpp"
#include "interf.hpp"
#include "colormix.hpp"
#include "config.hpp"
#include "processname.hpp"
#include "pathmix.hpp"
#include "cmdline.hpp"
#include "FarGuid.hpp"
#include "xlat.hpp"
#include "language.hpp"
#include "vmenu2.hpp"

MenuItemEx FarList2MenuItem(const FarListItem& FItem)
{
	MenuItemEx Result;
	Result.Flags = FItem.Flags;
	Result.strName = NullToEmpty(FItem.Text);
	return Result;
}

VMenu::VMenu(const string& Title,       // заголовок меню
             MenuDataEx *Data, // пункты меню
             int ItemsCount,     // количество пунктов меню
             int MaxHeight,     // максимальная высота
             DWORD Flags,       // нужен ScrollBar?
             Dialog *ParentDialog
            ):  // родитель для ListBox
	strTitle(Title),
	SelectPos(-1),
	SelectPosResult(-1),
	TopPos(0),
	WasAutoHeight(false),
	MaxLength(0),
	BoxType(DOUBLE_BOX),
	ParentDialog(ParentDialog),
	OldTitle(nullptr),
	bFilterEnabled(false),
	bFilterLocked(false),
	ItemHiddenCount(0),
	ItemSubMenusCount(0),
	MenuId(FarGuid)
{
	SaveScr=nullptr;
	SetDynamicallyBorn(false);
	SetFlags(Flags|VMENU_MOUSEREACTION|VMENU_UPDATEREQUIRED);
	ClearFlags(VMENU_SHOWAMPERSAND|VMENU_MOUSEDOWN);
	CurrentFrame = Global->FrameManager->GetCurrentFrame();
	GetCursorType(PrevCursorVisible,PrevCursorSize);
	bRightBtnPressed = false;

	// инициализируем перед тем, как добавлять айтема
	UpdateMaxLengthFromTitles();

	FOR(const auto& i, make_range(Data, Data + ItemsCount))
	{
		MenuItemEx NewItem;

		if (!global::IsPtr(i.Name))
			NewItem.strName = MSG(static_cast<LNGID>(reinterpret_cast<intptr_t>(i.Name)));
		else
			NewItem.strName = i.Name;

		//NewItem.AmpPos = -1;
		NewItem.AccelKey = i.AccelKey;
		NewItem.Flags = i.Flags;
		AddItem(NewItem);
	}

	SetMaxHeight(MaxHeight);
	SetColors(nullptr); //Установим цвет по умолчанию

	if (!CheckFlags(VMENU_LISTBOX) && Global->CtrlObject)
	{
		PrevMacroMode = Global->CtrlObject->Macro.GetMode();

		if (!IsMenuArea(PrevMacroMode))
			Global->CtrlObject->Macro.SetMode(MACROAREA_MENU);
	}
}

VMenu::~VMenu()
{
	if (!CheckFlags(VMENU_LISTBOX) && Global->CtrlObject)
		Global->CtrlObject->Macro.SetMode(PrevMacroMode);

	Hide();
	DeleteItems();

	if (Global->FrameManager->GetCurrentFrame() == CurrentFrame)
		SetCursorType(PrevCursorVisible,PrevCursorSize);
}

void VMenu::ResetCursor()
{
	GetCursorType(PrevCursorVisible,PrevCursorSize);
}

//может иметь фокус
bool VMenu::ItemCanHaveFocus(UINT64 Flags) const
{
	return !(Flags&(LIF_DISABLE|LIF_HIDDEN|LIF_SEPARATOR));
}

//может быть выбран
bool VMenu::ItemCanBeEntered(UINT64 Flags) const
{
	return !(Flags&(LIF_DISABLE|LIF_HIDDEN|LIF_GRAYED|LIF_SEPARATOR));
}

//видимый
bool VMenu::ItemIsVisible(UINT64 Flags) const
{
	return !(Flags&(LIF_HIDDEN));
}

bool VMenu::UpdateRequired()
{
	SCOPED_ACTION(CriticalSectionLock)(CS);

	return CheckFlags(VMENU_UPDATEREQUIRED)!=0;
}

void VMenu::UpdateInternalCounters(UINT64 OldFlags, UINT64 NewFlags)
{
	if (OldFlags&MIF_SUBMENU)
		ItemSubMenusCount--;

	if (!ItemIsVisible(OldFlags))
		ItemHiddenCount--;

	if (NewFlags&MIF_SUBMENU)
		ItemSubMenusCount++;

	if (!ItemIsVisible(NewFlags))
		ItemHiddenCount++;
}

void VMenu::UpdateItemFlags(int Pos, UINT64 NewFlags)
{
	UpdateInternalCounters(Items[Pos].Flags, NewFlags);

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
}

// переместить курсор c учётом пунктов которые не могут получать фокус
int VMenu::SetSelectPos(int Pos, int Direct, bool stop_on_edge)
{
	SCOPED_ACTION(CriticalSectionLock)(CS);

	SelectPosResult=-1;

	if (Items.empty())
		return -1;

	std::for_each(RANGE(Items, i)
	{
		i.Flags&=~LIF_SELECTED;
	});

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

		if (ItemCanHaveFocus(Items[Pos].Flags))
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

	if (Pos != SelectPos && ParentDialog && !ParentDialog->CheckDialogMode(DMODE_ISMENU) && CheckFlags(VMENU_COMBOBOX|VMENU_LISTBOX) && ParentDialog->IsInited() && !ParentDialog->SendMessage(DN_LISTCHANGE, DialogItemID, ToPtr(Pos)))
	{
		UpdateItemFlags(SelectPos, Items[SelectPos].Flags|LIF_SELECTED);
		return -1;
	}

	if (Pos >= 0)
		UpdateItemFlags(Pos, Items[Pos].Flags|LIF_SELECTED);

	if (Pos != SelectPos)
		SetFlags(VMENU_UPDATEREQUIRED);

	SelectPosResult=Pos;
	return Pos;
}

// установить курсор и верхний итем
int VMenu::SetSelectPos(const FarListPos *ListPos, int Direct)
{
	SCOPED_ACTION(CriticalSectionLock)(CS);

	int pos = std::min(static_cast<intptr_t>(Items.size()-1), std::max((intptr_t)0, ListPos->SelectPos));
	int Ret = SetSelectPos(pos, Direct ? Direct : pos > SelectPos? 1 : -1);

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
	SCOPED_ACTION(CriticalSectionLock)(CS);

	if (Items.empty())
		return;

	// если selection стоит в некорректном месте - сбросим его
	if (SelectPos >= 0 && !ItemCanHaveFocus(Items[SelectPos].Flags))
		SelectPos = -1;

	for (size_t i=0; i<Items.size(); i++)
	{
		if (!ItemCanHaveFocus(Items[i].Flags))
		{
			Items[i].SetSelect(FALSE);
		}
		else
		{
			if (SelectPos == -1)
			{
				Items[i].SetSelect(TRUE);
				SelectPos = static_cast<int>(i);
			}
			else if (SelectPos != static_cast<int>(i))
			{
				Items[i].SetSelect(FALSE);
			}
			else
			{
				Items[i].SetSelect(TRUE);
			}
		}
	}
}

int VMenu::GetItemPosition(int Position)
{
	SCOPED_ACTION(CriticalSectionLock)(CS);

	int DataPos = (Position==-1) ? SelectPos : Position;

	if (DataPos>=static_cast<int>(Items.size()))
		DataPos = -1; //Items.size()-1;

	return DataPos;
}

// получить позицию курсора и верхнюю позицию итема
int VMenu::GetSelectPos(FarListPos *ListPos)
{
	SCOPED_ACTION(CriticalSectionLock)(CS);

	ListPos->SelectPos=SelectPos;
	ListPos->TopPos=TopPos;

	return ListPos->SelectPos;
}

int VMenu::InsertItem(const FarListInsert *NewItem)
{
	SCOPED_ACTION(CriticalSectionLock)(CS);

	if (NewItem)
	{
		auto MenuItem = FarList2MenuItem(NewItem->Item);
		if (AddItem(MenuItem, NewItem->Index) >= 0)
			return static_cast<int>(Items.size());
	}

	return -1;
}

int VMenu::AddItem(const FarList *List)
{
	SCOPED_ACTION(CriticalSectionLock)(CS);

	if (List && List->Items)
	{
		for (size_t i=0; i<List->ItemsNumber; i++)
		{
			auto MenuItem = FarList2MenuItem(List->Items[i]);
			AddItem(MenuItem);
		}
	}

	return static_cast<int>(Items.size());
}

int VMenu::AddItem(const wchar_t *NewStrItem)
{
	SCOPED_ACTION(CriticalSectionLock)(CS);

	FarListItem FarListItem0={};

	if (!NewStrItem || NewStrItem[0] == 0x1)
	{
		FarListItem0.Flags=LIF_SEPARATOR;
		FarListItem0.Text=NewStrItem+1;
	}
	else
	{
		FarListItem0.Text=NewStrItem;
	}

	FarList FarList0={sizeof(FarList),1,&FarListItem0};

	return AddItem(&FarList0)-1; //-1 потому что AddItem(FarList) возвращает количество элементов
}

int VMenu::AddItem(MenuItemEx& NewItem,int PosAdd)
{
	SCOPED_ACTION(CriticalSectionLock)(CS);

	PosAdd = std::max(0, std::min(PosAdd, static_cast<int>(Items.size())));

	Items.emplace(Items.begin() + PosAdd, std::move(NewItem));
	auto& NewMenuItem = Items[PosAdd];

	// BUGBUG
	_SetUserData(&NewMenuItem, NewMenuItem.UserData, NewMenuItem.UserDataSize);

	NewMenuItem.AmpPos = -1;
	NewMenuItem.ShowPos = 0;

	if (PosAdd <= SelectPos)
		SelectPos++;

	if (CheckFlags(VMENU_SHOWAMPERSAND))
		UpdateMaxLength((int)NewMenuItem.strName.size());
	else
		UpdateMaxLength(HiStrlen(NewMenuItem.strName));

	auto NewFlags = NewMenuItem.Flags;
	NewMenuItem.Flags = 0;
	UpdateItemFlags(PosAdd, NewFlags);

	SetFlags(VMENU_UPDATEREQUIRED|(bFilterEnabled?VMENU_REFILTERREQUIRED:0));

	return static_cast<int>(Items.size()-1);
}

int VMenu::UpdateItem(const FarListUpdate *NewItem)
{
	SCOPED_ACTION(CriticalSectionLock)(CS);

	if (NewItem && (DWORD)NewItem->Index < (DWORD)Items.size())
	{
		// Освободим память... от ранее занятого ;-)
		if (NewItem->Item.Flags&LIF_DELETEUSERDATA)
		{
			xf_free(Items[NewItem->Index].UserData);
			Items[NewItem->Index].UserData = nullptr;
			Items[NewItem->Index].UserDataSize = 0;
		}

		MenuItemEx MItem = FarList2MenuItem(NewItem->Item);

		Items[NewItem->Index].strName = MItem.strName;

		UpdateItemFlags(NewItem->Index, MItem.Flags);

		SetFlags(VMENU_UPDATEREQUIRED|(bFilterEnabled?VMENU_REFILTERREQUIRED:0));

		return TRUE;
	}

	return FALSE;
}

//функция удаления N пунктов меню
int VMenu::DeleteItem(int ID, int Count)
{
	SCOPED_ACTION(CriticalSectionLock)(CS);

	if (ID < 0 || ID >= static_cast<int>(Items.size()) || Count <= 0)
		return static_cast<int>(Items.size());

	if (ID+Count > static_cast<int>(Items.size()))
		Count=static_cast<int>(Items.size()-ID);

	if (Count <= 0)
		return static_cast<int>(Items.size());

	if (!ID && Count == static_cast<int>(Items.size()))
	{
		DeleteItems();
		return static_cast<int>(Items.size());
	}

	// Надобно удалить данные, чтоб потери по памяти не были
	for (int I=0; I < Count; ++I)
	{
		xf_free(Items[ID+I].UserData);
		UpdateInternalCounters(Items[ID+I].Flags,0);
	}

	// а вот теперь перемещения
	auto FirstIter = Items.begin()+ID, LastIter = FirstIter+Count;
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

	SetFlags(VMENU_UPDATEREQUIRED);

	return static_cast<int>(Items.size());
}

void VMenu::DeleteItems()
{
	SCOPED_ACTION(CriticalSectionLock)(CS);

	std::for_each(CONST_RANGE(Items, i)
	{
		xf_free(i.UserData);
	});
	Items.clear();
	ItemHiddenCount=0;
	ItemSubMenusCount=0;
	SelectPos=-1;
	TopPos=0;
	MaxLength=0;
	UpdateMaxLengthFromTitles();

	SetFlags(VMENU_UPDATEREQUIRED);
}

int VMenu::GetCheck(int Position)
{
	SCOPED_ACTION(CriticalSectionLock)(CS);

	int ItemPos = GetItemPosition(Position);

	if (ItemPos < 0)
		return 0;

	if (Items[ItemPos].Flags & LIF_SEPARATOR)
		return 0;

	if (!(Items[ItemPos].Flags & LIF_CHECKED))
		return 0;

	int Checked = Items[ItemPos].Flags & 0xFFFF;

	return Checked ? Checked : 1;
}


void VMenu::SetCheck(int Check, int Position)
{
	SCOPED_ACTION(CriticalSectionLock)(CS);

	int ItemPos = GetItemPosition(Position);

	if (ItemPos < 0)
		return;

	Items[ItemPos].SetCheck(Check);
}

void VMenu::RestoreFilteredItems()
{
	std::for_each(RANGE(Items, i) { i.Flags &= ~LIF_HIDDEN; });

	ItemHiddenCount=0;

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
	string strName;

	if (SelectPos > 0)
	{
		// Определить, в верхней или нижней части расположен курсор
		int TopVisible = GetVisualPos(TopPos);
		int SelectedVisible = GetVisualPos(SelectPos);
		int BottomVisible = (TopVisible+MaxHeight > GetShowItemCount()) ? (TopVisible+MaxHeight-1) : (GetShowItemCount()-1);
		if (SelectedVisible >= ((TopVisible+BottomVisible)>>1))
			bBottomMode = true;
	}

	ItemHiddenCount=0;

	for_each_cnt(RANGE(Items, CurItem, size_t index)
	{
		CurItem.Flags &= ~LIF_HIDDEN;
		strName=CurItem.strName;
		if (CurItem.Flags & LIF_SEPARATOR)
		{
			// В предыдущей группе все элементы скрыты, разделитель перед группой - не нужен
			if (PrevSeparator != -1)
			{
				Items[PrevSeparator].Flags |= LIF_HIDDEN;
				ItemHiddenCount++;
			}

			if (strName.empty() && PrevGroup == -1)
			{
				CurItem.Flags |= LIF_HIDDEN;
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
			RemoveExternalSpaces(strName);
			RemoveChar(strName,L'&',true);
			if(!StrStrI(strName.data(), strFilter.data()))
			{
				CurItem.Flags |= LIF_HIDDEN;
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
					if (ItemCanHaveFocus(CurItem.Flags))
						UpperVisible = static_cast<int>(index);
				}
				else if (LowerVisible == -1)
				{
					if (ItemCanHaveFocus(CurItem.Flags))
						LowerVisible = static_cast<int>(index);
				}
				// Этот разделитель - оставить видимым
				if (PrevSeparator != -1)
					PrevSeparator = -1;
			}
		}
	});
	// В предыдущей группе все элементы скрыты, разделитель перед группой - не нужен
	if (PrevSeparator != -1)
	{
		Items[PrevSeparator].Flags |= LIF_HIDDEN;
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
	VMenu2 *Parent = nullptr;
	if (ParentDialog && ParentDialog->GetType() == MODALTYPE_VMENU)
		Parent = static_cast<VMenu2*>(ParentDialog);

	if (WasAutoHeight || Parent)
	{
		int NewY2;
		if (MaxHeight && MaxHeight<GetShowItemCount())
			NewY2 = Y1 + MaxHeight + 1;
		else
			NewY2 = Y1 + GetShowItemCount() + 1;
		if (NewY2 > ScrY)
			NewY2 = ScrY;
		if (NewY2 > Y2 || (bShrink && NewY2 < Y2))
		{
		  if (Parent)
				Parent->Resize();
		  else
				SetPosition(X1,Y1,X2,NewY2);
		}
	}
}

static bool IsFilterEditKey(int Key)
{
	return (Key>=(int)KEY_SPACE && Key<0xffff) || Key==KEY_BS;
}

bool VMenu::ShouldSendKeyToFilter(int Key) const
{
	if (Key==KEY_CTRLALTF || Key==KEY_RCTRLRALTF || Key==KEY_CTRLRALTF || Key==KEY_RCTRLALTF)
		return true;

	if (bFilterEnabled)
	{
		if (Key==KEY_CTRLALTL || Key==KEY_RCTRLRALTL || Key==KEY_CTRLRALTL || Key==KEY_RCTRLALTL)
			return true;

		if (!bFilterLocked && IsFilterEditKey(Key))
			return true;
	}

	return false;
}

int VMenu::ReadInput(INPUT_RECORD *GetReadRec)
{
	int ReadKey;

	for (;;)
	{
		ReadKey = Modal::ReadInput(GetReadRec);

		//фильтр должен обрабатывать нажатия раньше "пользователя" меню
		if (ShouldSendKeyToFilter(ReadKey))
		{
			ProcessInput();
			continue;
		}
		break;
	}

	return ReadKey;
}

__int64 VMenu::VMProcess(int OpCode,void *vParam,__int64 iParam)
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
			return Items.size() > 0 && SelectPos >= 0;
		case MCODE_V_ITEMCOUNT:
			return GetShowItemCount();
		case MCODE_V_CURPOS:
			return GetVisualPos(SelectPos)+1;
		case MCODE_F_MENU_CHECKHOTKEY:
		{
			const wchar_t *str = (const wchar_t *)vParam;
			return GetVisualPos(CheckHighlights(*str,(int)iParam))+1;
		}
		case MCODE_F_MENU_SELECT:
		{
			const wchar_t *str = (const wchar_t *)vParam;
			const size_t strLength = wcslen(str);

			if (*str)
			{
				string strTemp;
				int Res;
				int Direct=(iParam >> 8)&0xFF;
				/*
					Direct:
						0 - от начала в конец списка;
						1 - от текущей позиции в начало;
						2 - от текущей позиции в конец списка пунктов меню.
				*/
				iParam&=0xFF;
				int StartPos=Direct?SelectPos:0;
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

					MenuItemEx *_item = GetItemPtr(I);

					if (!ItemCanHaveFocus(_item->Flags))
						continue;

					Res = 0;
					RemoveExternalSpaces(HiText2Str(strTemp,_item->strName));
					const wchar_t *p;

					switch (iParam)
					{
						case 0: // full compare
							Res = !StrCmpI(strTemp.data(),str);
							break;
						case 1: // begin compare
							p = StrStrI(strTemp.data(),str);
							Res = p==strTemp.data();
							break;
						case 2: // end compare
							p = RevStrStrI(strTemp.data(),str);
							Res = p && !*(p + strLength);
							break;
						case 3: // in str
							Res = StrStrI(strTemp.data(),str)!=nullptr;
							break;
					}

					if (Res)
					{
						SetSelectPos(I,1);

						ShowMenu(true);

						return GetVisualPos(SelectPos)+1;
					}
				}
			}

			return 0;
		}

		case MCODE_F_MENU_GETHOTKEY:
		case MCODE_F_MENU_GETVALUE: // S=Menu.GetValue([N])
		{
			int Param = (int)iParam;

			if (Param == -1)
				Param = SelectPos;
			else
				Param = VisualPosToReal(Param);

			if (Param>=0 && Param<static_cast<int>(Items.size()))
			{
				MenuItemEx *menuEx = GetItemPtr(Param);
				if (menuEx)
				{
					if (OpCode == MCODE_F_MENU_GETVALUE)
					{
						*(string *)vParam = menuEx->strName;
						return 1;
					}
					else
					{
						return GetHighlights(menuEx);
					}
				}
			}

			return 0;
		}

		case MCODE_F_MENU_ITEMSTATUS: // N=Menu.ItemStatus([N])
		{
			__int64 RetValue=-1;
			int Param = (int)iParam;

			if (Param == -1)
				Param = SelectPos;

			MenuItemEx *menuEx = GetItemPtr(Param);

			if (menuEx)
			{
				RetValue=menuEx->Flags;

				if (Param == SelectPos)
					RetValue |= LIF_SELECTED;

				RetValue = MAKELONG(HIWORD(RetValue),LOWORD(RetValue));
			}

			return RetValue;
		}

		case MCODE_V_MENU_VALUE: // Menu.Value
		{
			MenuItemEx *menuEx = GetItemPtr(SelectPos);

			if (menuEx)
			{
				*(string *)vParam = menuEx->strName;
				return 1;
			}

			return 0;
		}

		case MCODE_F_MENU_FILTER:
		{
			__int64 RetValue = 0;
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
			switch (iParam)
			{
				case 0:
					switch ((intptr_t)vParam)
					{
						case 0:
						case 1:
							if (bFilterEnabled != ((intptr_t)vParam == 1))
							{
								bFilterEnabled=((intptr_t)vParam == 1);
								bFilterLocked=false;
								strFilter.clear();
								if (!vParam)
									RestoreFilteredItems();
								DisplayObject();
							}
							RetValue = 1;
							break;
						case -1:
							RetValue = bFilterEnabled ? 1 : 0;
							break;
					}
					break;

				case 1:
					switch ((intptr_t)vParam)
					{
						case 0:
						case 1:
							bFilterLocked=((intptr_t)vParam == 1);
							DisplayObject();
							RetValue = 1;
							break;
						case -1:
							RetValue = bFilterLocked ? 1 : 0;
							break;
					}
					break;

				case 2:
					RetValue = (bFilterEnabled && !strFilter.empty()) ? 1 : 0;
					break;

				case 3:
					RetValue = ItemHiddenCount;
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
						*(string *)vParam = strFilter;
						return 1;
					}
					break;
				case 1:
					if (!bFilterEnabled)
						bFilterEnabled=true;
					bool prevLocked = bFilterLocked;
					bFilterLocked = false;
					RestoreFilteredItems();
					string oldFilter = strFilter;
					strFilter.clear();
					AddToFilter(((string *)vParam)->data());
					FilterStringUpdated();
					bFilterLocked = prevLocked;
					DisplayObject();
					*(string *)vParam = oldFilter;
					return 1;
			}

			return 0;
		}
		case MCODE_V_MENUINFOID:
		{
			static string strId;
			strId = GuidToStr(MenuId);
			return reinterpret_cast<intptr_t>(UNSAFE_CSTR(strId));
		}

	}

	return 0;
}

bool VMenu::AddToFilter(const wchar_t *str)
{
	if (bFilterEnabled && !bFilterLocked)
	{
		int Key;
		while ((Key=*str))
		{
			if( IsFilterEditKey(Key) )
			{
				if ( Key==KEY_BS && !strFilter.empty() )
					strFilter.pop_back();
				else
					strFilter += Key;
			}
			++str;
		}
		return true;
	}

	return false;
}

void VMenu::SetFilterString(const wchar_t *str)
{
	strFilter=str;
}

int VMenu::ProcessFilterKey(int Key)
{
	if (!bFilterEnabled || bFilterLocked || !IsFilterEditKey(Key))
		return FALSE;

	if (Key==KEY_BS)
	{
		if (!strFilter.empty())
		{
			strFilter.pop_back();

			if (strFilter.empty())
			{
				RestoreFilteredItems();
				DisplayObject();
				return TRUE;
			}
		}
		else
		{
			return TRUE;
		}
	}
	else
	{
		if (!GetShowItemCount())
			return TRUE;

		strFilter += (wchar_t)Key;
	}

	FilterStringUpdated();
	DisplayObject();

	return TRUE;
}

int VMenu::ProcessKey(const Manager::Key& Key)
{
	int LocalKey=Key.FarKey;
	SCOPED_ACTION(CriticalSectionLock)(CS);

	if (LocalKey==KEY_NONE || LocalKey==KEY_IDLE)
		return FALSE;

	if (LocalKey == KEY_OP_PLAINTEXT)
	{
		const wchar_t *str = Global->CtrlObject->Macro.GetStringToPrint();

		if (!*str)
			return FALSE;

		if ( AddToFilter(str) ) // для фильтра: всю строку целиком в фильтр, а там разберемся.
		{
			if (strFilter.empty())
				RestoreFilteredItems();
			else
				FilterStringUpdated();

			DisplayObject();

			return TRUE;
		}
		else // не для фильтра: по старинке, первый символ последовательности, остальное игнорируем (ибо некуда)
			LocalKey=*str;
	}

	SetFlags(VMENU_UPDATEREQUIRED);

	if (!GetShowItemCount())
	{
		if ((LocalKey!=KEY_F1 && LocalKey!=KEY_SHIFTF1 && LocalKey!=KEY_F10 && LocalKey!=KEY_ESC && LocalKey!=KEY_ALTF9 && LocalKey!=KEY_RALTF9))
		{
			if (!bFilterEnabled || (bFilterEnabled && LocalKey!=KEY_BS && LocalKey!=KEY_CTRLALTF && LocalKey!=KEY_RCTRLRALTF && LocalKey!=KEY_CTRLRALTF && LocalKey!=KEY_RCTRLALTF && LocalKey!=KEY_RALT && LocalKey!=KEY_OP_XLAT))
			{
				ExitCode = -1;
				return FALSE;
			}
		}
	}

	if (!(((unsigned int)LocalKey >= KEY_MACRO_BASE && (unsigned int)LocalKey <= KEY_MACRO_ENDBASE) || ((unsigned int)LocalKey >= KEY_OP_BASE && (unsigned int)LocalKey <= KEY_OP_ENDBASE)))
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

	switch (LocalKey)
	{
		case KEY_ALTF9:
		case KEY_RALTF9:
			Global->FrameManager->ProcessKey(Manager::Key(KEY_ALTF9));
			break;
		case KEY_NUMENTER:
		case KEY_ENTER:
		{
			if (!ParentDialog || CheckFlags(VMENU_COMBOBOX))
			{
				if (ItemCanBeEntered(Items[SelectPos].Flags))
				{
					SetExitCode(SelectPos);
				}
			}

			break;
		}
		case KEY_ESC:
		case KEY_F10:
		{
			if (!ParentDialog || CheckFlags(VMENU_COMBOBOX))
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
			int dy = ((BoxType!=NO_BOX)?Y2-Y1-1:Y2-Y1);

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
			int dy = ((BoxType!=NO_BOX)?Y2-Y1-1:Y2-Y1);

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
			if (LocalKey == KEY_ALTHOME || LocalKey == KEY_RALTHOME || LocalKey == (KEY_ALT|KEY_NUMPAD7) || LocalKey == (KEY_RALT|KEY_NUMPAD7))
			{
				std::for_each(RANGE(Items, i) { i.ShowPos=0; });
			}
			else
			{
				int _len;

				std::for_each(RANGE(Items, i)
				{
					if (CheckFlags(VMENU_SHOWAMPERSAND))
						_len=static_cast<int>(i.strName.size());
					else
						_len=HiStrlen(i.strName);

					if (_len >= MaxLineWidth)
						i.ShowPos = _len - MaxLineWidth;
				});
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
				if (ShiftItemShowPos(static_cast<int>(I),(LocalKey == KEY_ALTLEFT || LocalKey == KEY_RALTLEFT || LocalKey == (KEY_ALT|KEY_NUMPAD4) || LocalKey == (KEY_RALT|KEY_NUMPAD4) || LocalKey == KEY_MSWHEEL_LEFT)?-1:1))
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
			if (ShiftItemShowPos(SelectPos,(LocalKey == KEY_ALTSHIFTLEFT || LocalKey == KEY_RALTSHIFTLEFT || LocalKey == (KEY_ALT|KEY_SHIFT|KEY_NUMPAD4) || LocalKey == (KEY_RALT|KEY_SHIFT|KEY_NUMPAD4))?-1:1))
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
				int Items_size=static_cast<int>(Items.size());
				int Height_size = ((BoxType != NO_BOX) ? Y2 - Y1 - 1 : Y2 - Y1 + 1);
				if (!(Items_size - TopPos <= Height_size || Items_size <= Height_size) )
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
			bFilterEnabled=!bFilterEnabled;
			bFilterLocked=false;
			strFilter.clear();

			if (!bFilterEnabled)
				RestoreFilteredItems();

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
				if (!GetClipboard(ClipText))
					return TRUE;

				if (AddToFilter(ClipText.data()))
				{
					if (strFilter.empty())
						RestoreFilteredItems();
					else
						FilterStringUpdated();

					DisplayObject();
				}
			}
			return TRUE;
		}
		case KEY_CTRLALTL:
		case KEY_RCTRLRALTL:
		case KEY_CTRLRALTL:
		case KEY_RCTRLALTL:
		{
			if (bFilterEnabled)
			{
				bFilterLocked=!bFilterLocked;
				DisplayObject();
				break;
			}
		}
		case KEY_OP_XLAT:
		{
			if (bFilterEnabled && !bFilterLocked)
			{
				const wchar_t *FilterString=strFilter.data();
				int start=StrLength(FilterString);
				bool DoXlat = true;

				if (IsWordDiv(Global->Opt->XLat.strWordDivForXlat,FilterString[start]))
				{
					if (start) start--;
					DoXlat=(!IsWordDiv(Global->Opt->XLat.strWordDivForXlat,FilterString[start]));
				}

				if (DoXlat)
				{
					while (start>=0 && !IsWordDiv(Global->Opt->XLat.strWordDivForXlat,FilterString[start]))
						start--;

					start++;
					::Xlat(const_cast<wchar_t*>(FilterString), start, StrLength(FilterString), Global->Opt->XLat.Flags);
					SetFilterString(FilterString);
					FilterStringUpdated();
					DisplayObject();
				}
			}
			break;
		}
		case KEY_TAB:
		case KEY_SHIFTTAB:
		default:
		{
			if (ProcessFilterKey(LocalKey))
				return TRUE;

			int OldSelectPos=SelectPos;
			int NewPos=SelectPos;

			bool IsHotkey=true;
			if (!CheckKeyHiOrAcc(LocalKey, 0, 0, !(ParentDialog && !ParentDialog->CheckDialogMode(DMODE_ISMENU) && CheckFlags(VMENU_COMBOBOX | VMENU_LISTBOX)), NewPos))
			{
				if (LocalKey == KEY_SHIFTF1 || LocalKey == KEY_F1)
				{
					if (ParentDialog)
						;//ParentDialog->ProcessKey(Key);
					else
						ShowHelp();

					break;
				}
				else
				{
					if (!CheckKeyHiOrAcc(LocalKey,1,FALSE,!(ParentDialog && !ParentDialog->CheckDialogMode(DMODE_ISMENU) && CheckFlags(VMENU_COMBOBOX|VMENU_LISTBOX)),NewPos))
						if (!CheckKeyHiOrAcc(LocalKey,1,TRUE,!(ParentDialog && !ParentDialog->CheckDialogMode(DMODE_ISMENU) && CheckFlags(VMENU_COMBOBOX|VMENU_LISTBOX)),NewPos))
							IsHotkey=false;
				}
			}

			if (IsHotkey && ParentDialog)
			{
				if (ParentDialog->SendMessage(DN_LISTHOTKEY,DialogItemID,ToPtr(NewPos)))
				{
					UpdateItemFlags(OldSelectPos,Items[OldSelectPos].Flags|LIF_SELECTED);
					ShowMenu(true);
					ClearDone();
					break;
				}
				else
				{
					if (NewPos != OldSelectPos)
					{
						if (SetSelectPos(NewPos, 1) < 0)
						{
							ClearDone();
						}
						else
						{
							CheckFlags(VMENU_COMBOBOX) ? SetDone() : ClearDone();
						}

						break;
					}
				}
			}

			return FALSE;
		}
	}

	return TRUE;
}

int VMenu::ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent)
{
	SCOPED_ACTION(CriticalSectionLock)(CS);

	SetFlags(VMENU_UPDATEREQUIRED);

	if (!GetShowItemCount())
	{
		if (MouseEvent->dwButtonState && !MouseEvent->dwEventFlags)
			SetExitCode(-1);
		return FALSE;
	}

	int MsX=MouseEvent->dwMousePosition.X;
	int MsY=MouseEvent->dwMousePosition.Y;

	// необходимо знать, что RBtn был нажат ПОСЛЕ появления VMenu, а не до
	if (MouseEvent->dwButtonState&RIGHTMOST_BUTTON_PRESSED && MouseEvent->dwEventFlags==0)
		bRightBtnPressed=true;

	if (MouseEvent->dwButtonState&FROM_LEFT_2ND_BUTTON_PRESSED && MouseEvent->dwEventFlags!=MOUSE_MOVED)
	{
		if (((BoxType!=NO_BOX)?
				(MsX>X1 && MsX<X2 && MsY>Y1 && MsY<Y2):
				(MsX>=X1 && MsX<=X2 && MsY>=Y1 && MsY<=Y2)))
		{
			ProcessKey(Manager::Key(KEY_ENTER));
		}
		return TRUE;
	}

	if ( (MouseEvent->dwButtonState&FROM_LEFT_1ST_BUTTON_PRESSED) && ( MsX==X1+2||MsX==X2-1-(CheckFlags(VMENU_COMBOBOX|VMENU_LISTBOX)?0:2) ) )
	{
		while (IsMouseButtonPressed())
			ProcessKey(Manager::Key(MsX==X1+2?KEY_ALTLEFT:KEY_ALTRIGHT));

		return TRUE;
	}

	int SbY1 = ((BoxType!=NO_BOX)?Y1+1:Y1), SbY2=((BoxType!=NO_BOX)?Y2-1:Y2);
	bool bShowScrollBar = false;

	if (CheckFlags(VMENU_LISTBOX|VMENU_ALWAYSSCROLLBAR) || Global->Opt->ShowMenuScrollbar)
		bShowScrollBar = true;

	if (bShowScrollBar && MsX==X2 && ((BoxType!=NO_BOX)?Y2-Y1-1:Y2-Y1+1)<static_cast<int>(Items.size()) && (MouseEvent->dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED))
	{
		if (MsY==SbY1)
		{
			while (IsMouseButtonPressed())
			{
				//прокрутка мышью не должна врапить меню
				if (SelectPos>=0 && GetVisualPos(SelectPos))
					ProcessKey(Manager::Key(KEY_UP));

				ShowMenu(true);
			}

			return TRUE;
		}

		if (MsY==SbY2)
		{
			while (IsMouseButtonPressed())
			{
				//прокрутка мышью не должна врапить меню
				if (SelectPos>=0 && GetVisualPos(SelectPos)!=GetShowItemCount()-1)
					ProcessKey(Manager::Key(KEY_DOWN));

				ShowMenu(true);
			}

			return TRUE;
		}

		if (MsY>SbY1 && MsY<SbY2)
		{
			int Delta=0;

			while (IsMouseButtonPressed())
			{
				int SbHeight=Y2-Y1-2;
				int MsPos=(GetShowItemCount()-1)*(IntKeyState.MouseY-Y1)/(SbHeight);

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

			return TRUE;
		}
	}

	// dwButtonState & 3 - Left & Right button
	if (BoxType!=NO_BOX && (MouseEvent->dwButtonState & 3) && MsX>X1 && MsX<X2)
	{
		if (MsY==Y1)
		{
			while (MsY==Y1 && GetVisualPos(SelectPos)>0 && IsMouseButtonPressed())
				ProcessKey(Manager::Key(KEY_UP));

			return TRUE;
		}

		if (MsY==Y2)
		{
			while (MsY==Y2 && GetVisualPos(SelectPos)<GetShowItemCount()-1 && IsMouseButtonPressed())
				ProcessKey(Manager::Key(KEY_DOWN));

			return TRUE;
		}
	}

	if ((BoxType!=NO_BOX)?
	        (MsX>X1 && MsX<X2 && MsY>Y1 && MsY<Y2):
	        (MsX>=X1 && MsX<=X2 && MsY>=Y1 && MsY<=Y2))
	{
		int MsPos=VisualPosToReal(GetVisualPos(TopPos)+((BoxType!=NO_BOX)?MsY-Y1-1:MsY-Y1));

		if (MsPos>=0 && MsPos<static_cast<int>(Items.size()) && ItemCanHaveFocus(Items[MsPos].Flags))
		{
			if (IntKeyState.MouseX!=IntKeyState.PrevMouseX || IntKeyState.MouseY!=IntKeyState.PrevMouseY || !MouseEvent->dwEventFlags)
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
				SetFlags(VMENU_MOUSEDOWN);

			if (!MouseEvent->dwEventFlags && !(MouseEvent->dwButtonState & (FROM_LEFT_1ST_BUTTON_PRESSED|RIGHTMOST_BUTTON_PRESSED)) && CheckFlags(VMENU_MOUSEDOWN))
			{
				ClearFlags(VMENU_MOUSEDOWN);
				ProcessKey(Manager::Key(KEY_ENTER));
			}
		}

		return TRUE;
	}
	else if (BoxType!=NO_BOX && (MouseEvent->dwButtonState & 3) && !MouseEvent->dwEventFlags)
	{
		int ClickOpt = (MouseEvent->dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) ? Global->Opt->VMenu.LBtnClick : Global->Opt->VMenu.RBtnClick;
		if (ClickOpt==VMENUCLICK_CANCEL)
			ProcessKey(Manager::Key(KEY_ESC));

		return TRUE;
	}
	else if (BoxType!=NO_BOX && !(MouseEvent->dwButtonState&FROM_LEFT_1ST_BUTTON_PRESSED) && (IntKeyState.PrevMouseButtonState&FROM_LEFT_1ST_BUTTON_PRESSED) && !MouseEvent->dwEventFlags && (Global->Opt->VMenu.LBtnClick==VMENUCLICK_APPLY))
	{
		ProcessKey(Manager::Key(KEY_ENTER));

		return TRUE;
	}
	else if (BoxType!=NO_BOX && !(MouseEvent->dwButtonState&FROM_LEFT_2ND_BUTTON_PRESSED) && (IntKeyState.PrevMouseButtonState&FROM_LEFT_2ND_BUTTON_PRESSED) && !MouseEvent->dwEventFlags && (Global->Opt->VMenu.MBtnClick==VMENUCLICK_APPLY))
	{
		ProcessKey(Manager::Key(KEY_ENTER));

		return TRUE;
	}
	else if (BoxType!=NO_BOX && bRightBtnPressed && !(MouseEvent->dwButtonState&RIGHTMOST_BUTTON_PRESSED) && (IntKeyState.PrevMouseButtonState&RIGHTMOST_BUTTON_PRESSED) && !MouseEvent->dwEventFlags && (Global->Opt->VMenu.RBtnClick==VMENUCLICK_APPLY))
	{
		ProcessKey(Manager::Key(KEY_ENTER));

		return TRUE;
	}

	return FALSE;
}

int VMenu::GetVisualPos(int Pos)
{
	if (!ItemHiddenCount)
		return Pos;

	if (Pos < 0)
		return -1;

	if (Pos >= static_cast<int>(Items.size()))
		return GetShowItemCount();

	int v=0;

	for (int i=0; i < Pos; i++)
	{
		if (ItemIsVisible(Items[i].Flags))
			v++;
	}

	return v;
}

int VMenu::VisualPosToReal(int VPos)
{
	if (!ItemHiddenCount)
		return VPos;

	if (VPos < 0)
		return -1;

	if (VPos >= GetShowItemCount())
		return static_cast<int>(Items.size());

	auto ItemIterator = std::find_if(CONST_RANGE(Items, i) { return ItemIsVisible(i.Flags) && !VPos--; });
	return ItemIterator != Items.cend()? ItemIterator - Items.cbegin() : -1;
}

bool VMenu::ShiftItemShowPos(int Pos, int Direct)
{
	int _len;
	int ItemShowPos = Items[Pos].ShowPos;

	if (VMFlags.Check(VMENU_SHOWAMPERSAND))
		_len = (int)Items[Pos].strName.size();
	else
		_len = HiStrlen(Items[Pos].strName);

	if (_len < MaxLineWidth || (Direct < 0 && !ItemShowPos) || (Direct > 0 && ItemShowPos > _len))
		return false;

	if (VMFlags.Check(VMENU_SHOWAMPERSAND))
	{
		if (Direct < 0)
			ItemShowPos--;
		else
			ItemShowPos++;
	}
	else
	{
		ItemShowPos = HiFindNextVisualPos(Items[Pos].strName,ItemShowPos,Direct);
	}

	if (ItemShowPos < 0)
		ItemShowPos = 0;

	if (ItemShowPos + MaxLineWidth > _len)
		ItemShowPos = _len - MaxLineWidth;

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
	SCOPED_ACTION(CriticalSectionLock)(CS);

	if (CheckFlags(VMENU_LISTBOX))
	{
		if (CheckFlags(VMENU_LISTSINGLEBOX))
			BoxType = SHORT_SINGLE_BOX;
		else if (CheckFlags(VMENU_SHOWNOBOX))
			BoxType = NO_BOX;
		else if (CheckFlags(VMENU_LISTHASFOCUS))
			BoxType = SHORT_DOUBLE_BOX;
		else
			BoxType = SHORT_SINGLE_BOX;
	}

	if (!CheckFlags(VMENU_LISTBOX))
	{
		bool AutoHeight = false;

		if (!CheckFlags(VMENU_COMBOBOX))
		{
			bool HasSubMenus = ItemSubMenusCount > 0;
			bool AutoCenter = false;

			if (X1 == -1)
			{
				X1 = (ScrX - MaxLength - 4 - (HasSubMenus ? 2 : 0)) / 2;
				AutoCenter = true;
			}

			if (X1 < 2)
				X1 = 2;

			if (X2 <= 0)
				X2 = X1 + MaxLength + 4 + (HasSubMenus ? 2 : 0);

			if (!AutoCenter && X2 > ScrX-4+2*(BoxType==SHORT_DOUBLE_BOX || BoxType==SHORT_SINGLE_BOX))
			{
				X1 += ScrX - 4 - X2;
				X2 = ScrX - 4;

				if (X1 < 2)
				{
					X1 = 2;
					X2 = ScrX - 2;
				}
			}

			if (X2 > ScrX-2)
				X2 = ScrX - 2;

			if (Y1 == -1)
			{
				if (MaxHeight && MaxHeight<GetShowItemCount())
					Y1 = (ScrY-MaxHeight-2)/2;
				else if ((Y1=(ScrY-GetShowItemCount()-2)/2) < 0)
					Y1 = 0;

				AutoHeight=true;
			}
		}

		WasAutoHeight = false;
		if (Y2 <= 0)
		{
			WasAutoHeight = true;
			if (MaxHeight && MaxHeight<GetShowItemCount())
				Y2 = Y1 + MaxHeight + 1;
			else
				Y2 = Y1 + GetShowItemCount() + 1;
		}

		if (Y2 > ScrY)
			Y2 = ScrY;

		if (AutoHeight && Y1 < 3 && Y2 > ScrY-3)
		{
			Y1 = 2;
			Y2 = ScrY - 2;
		}
	}

	if (X2>X1 && Y2+(CheckFlags(VMENU_SHOWNOBOX)?1:0)>Y1)
	{
		if (!CheckFlags(VMENU_LISTBOX))
		{
			ScreenObjectWithShadow::Show();
		}
		else
		{
			SetFlags(VMENU_UPDATEREQUIRED);
			DisplayObject();
		}
	}
}

void VMenu::Hide()
{
	SCOPED_ACTION(CriticalSectionLock)(CS);
	SCOPED_ACTION(ChangePriority)(THREAD_PRIORITY_NORMAL);

	if (!CheckFlags(VMENU_LISTBOX) && SaveScr)
	{
		SaveScr.reset();
		ScreenObjectWithShadow::Hide();
	}

	SetFlags(VMENU_UPDATEREQUIRED);

	OldTitle.reset();
}

void VMenu::DisplayObject()
{
	SCOPED_ACTION(CriticalSectionLock)(CS);
	SCOPED_ACTION(ChangePriority)(THREAD_PRIORITY_NORMAL);

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

	SetCursorType(0,10);

	if (!CheckFlags(VMENU_LISTBOX) && !SaveScr)
	{
		if (!CheckFlags(VMENU_DISABLEDRAWBACKGROUND) && !(BoxType==SHORT_DOUBLE_BOX || BoxType==SHORT_SINGLE_BOX))
			SaveScr = std::make_unique<SaveScreen>(X1-2,Y1-1,X2+4,Y2+2);
		else
			SaveScr = std::make_unique<SaveScreen>(X1, Y1, X2 + 2, Y2 + 1);
	}

	if (!CheckFlags(VMENU_DISABLEDRAWBACKGROUND) && !CheckFlags(VMENU_LISTBOX))
	{
		if (BoxType==SHORT_DOUBLE_BOX || BoxType==SHORT_SINGLE_BOX)
		{
			SetScreen(X1,Y1,X2,Y2,L' ',Colors[VMenuColorBody]);
			Box(X1,Y1,X2,Y2,Colors[VMenuColorBox],BoxType);

			if (!CheckFlags(VMENU_LISTBOX|VMENU_ALWAYSSCROLLBAR))
			{
				MakeShadow(X1+2,Y2+1,X2+1,Y2+1);
				MakeShadow(X2+1,Y1+1,X2+2,Y2+1);
			}
		}
		else
		{
			if (BoxType!=NO_BOX)
				SetScreen(X1-2,Y1-1,X2+2,Y2+1,L' ',Colors[VMenuColorBody]);
			else
				SetScreen(X1,Y1,X2,Y2,L' ',Colors[VMenuColorBody]);

			if (!CheckFlags(VMENU_LISTBOX|VMENU_ALWAYSSCROLLBAR))
			{
				MakeShadow(X1,Y2+2,X2+3,Y2+2);
				MakeShadow(X2+3,Y1,X2+4,Y2+2);
			}

			if (BoxType!=NO_BOX)
				Box(X1,Y1,X2,Y2,Colors[VMenuColorBox],BoxType);
		}

		//SetFlags(VMENU_DISABLEDRAWBACKGROUND);
	}

	if (!CheckFlags(VMENU_LISTBOX))
		DrawTitles();

	ShowMenu(true);
}

void VMenu::DrawTitles()
{
	SCOPED_ACTION(CriticalSectionLock)(CS);

	int MaxTitleLength = X2-X1-2;
	int WidthTitle;

	if (!strTitle.empty() || bFilterEnabled)
	{
		string strDisplayTitle = strTitle;

		if (bFilterEnabled)
		{
			if (bFilterLocked)
				strDisplayTitle += L" ";
			else
				strDisplayTitle.clear();

			strDisplayTitle += bFilterLocked?L"<":L"[";
			strDisplayTitle += strFilter;
			strDisplayTitle += bFilterLocked?L">":L"]";
		}

		WidthTitle=(int)strDisplayTitle.size();

		if (WidthTitle > MaxTitleLength)
			WidthTitle = MaxTitleLength - 1;

		GotoXY(X1+(X2-X1-1-WidthTitle)/2,Y1);
		SetColor(Colors[VMenuColorTitle]);

		Global->FS << L" " << fmt::ExactWidth(WidthTitle) << strDisplayTitle << L" ";
	}

	if (!strBottomTitle.empty())
	{
		WidthTitle=(int)strBottomTitle.size();

		if (WidthTitle > MaxTitleLength)
			WidthTitle = MaxTitleLength - 1;

		GotoXY(X1+(X2-X1-1-WidthTitle)/2,Y2);
		SetColor(Colors[VMenuColorTitle]);

		Global->FS << L" " << fmt::ExactWidth(WidthTitle) << strBottomTitle << L" ";
	}
}

void VMenu::ShowMenu(bool IsParent)
{
	SCOPED_ACTION(CriticalSectionLock)(CS);
	SCOPED_ACTION(ChangePriority)(THREAD_PRIORITY_NORMAL);

	int MaxItemLength = 0;
	bool HasRightScroll = false;
	bool HasSubMenus = ItemSubMenusCount > 0;

	//BUGBUG, this must be optimized
	std::for_each(CONST_RANGE(Items, i)
	{
		int ItemLen;

		if (CheckFlags(VMENU_SHOWAMPERSAND))
			ItemLen = static_cast<int>(i.strName.size());
		else
			ItemLen = HiStrlen(i.strName);

		if (ItemLen > MaxItemLength)
			MaxItemLength = ItemLen;
	});

	MaxLineWidth = X2 - X1 + 1;

	if (BoxType != NO_BOX)
		MaxLineWidth -= 2; // frame

	MaxLineWidth -= 2; // check mark + left horz. scroll

	if (/*!CheckFlags(VMENU_COMBOBOX|VMENU_LISTBOX) && */HasSubMenus)
		MaxLineWidth -= 2; // sub menu arrow

	if ((CheckFlags(VMENU_LISTBOX|VMENU_ALWAYSSCROLLBAR) || Global->Opt->ShowMenuScrollbar) && BoxType==NO_BOX && ScrollBarRequired(Y2-Y1+1, GetShowItemCount()))
		MaxLineWidth -= 1; // scrollbar

	if (MaxItemLength > MaxLineWidth)
	{
		HasRightScroll = true;
		MaxLineWidth -= 1; // right horz. scroll
	}

	MaxLineWidth = std::max(MaxLineWidth, 0);

	if (X2<=X1 || Y2<=Y1)
	{
		if (!(CheckFlags(VMENU_SHOWNOBOX) && Y2==Y1))
			return;
	}

	if (CheckFlags(VMENU_LISTBOX))
	{
		if (CheckFlags(VMENU_LISTSINGLEBOX))
			BoxType = SHORT_SINGLE_BOX;
		else if (CheckFlags(VMENU_SHOWNOBOX))
			BoxType = NO_BOX;
		else if (CheckFlags(VMENU_LISTHASFOCUS))
			BoxType = SHORT_DOUBLE_BOX;
		else
			BoxType = SHORT_SINGLE_BOX;

		if (!IsParent || !GetShowItemCount())
		{
			if (GetShowItemCount())
				BoxType=CheckFlags(VMENU_SHOWNOBOX)?NO_BOX:SHORT_SINGLE_BOX;

			SetScreen(X1,Y1,X2,Y2,L' ',Colors[VMenuColorBody]);
		}

		if (BoxType!=NO_BOX)
			Box(X1,Y1,X2,Y2,Colors[VMenuColorBox],BoxType);

		DrawTitles();
	}

	wchar_t BoxChar[2]={};

	switch (BoxType)
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

	if (CheckFlags(VMENU_AUTOHIGHLIGHT|VMENU_REVERSEHIGHLIGHT))
		AssignHighlights(CheckFlags(VMENU_REVERSEHIGHLIGHT));

	int VisualSelectPos = GetVisualPos(SelectPos);
	int VisualTopPos = GetVisualPos(TopPos);

	// коррекция Top`а
	if (VisualTopPos+GetShowItemCount() >= Y2-Y1 && VisualSelectPos == GetShowItemCount()-1)
	{
		VisualTopPos--;

		if (VisualTopPos<0)
			VisualTopPos=0;
	}

	VisualTopPos = std::min(VisualTopPos, GetShowItemCount() - (Y2-Y1-1-((BoxType==NO_BOX)?2:0)));

	if (VisualSelectPos > VisualTopPos+((BoxType!=NO_BOX)?Y2-Y1-2:Y2-Y1))
	{
		VisualTopPos=VisualSelectPos-((BoxType!=NO_BOX)?Y2-Y1-2:Y2-Y1);
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

	for (int Y=Y1+((BoxType!=NO_BOX)?1:0), I=TopPos; Y<((BoxType!=NO_BOX)?Y2:Y2+1); Y++, I++)
	{
		GotoXY(X1,Y);

		if (I < static_cast<int>(Items.size()))
		{
			if (!ItemIsVisible(Items[I].Flags))
			{
				Y--;
				continue;
			}

			if (Items[I].Flags&LIF_SEPARATOR)
			{
				int SepWidth = X2-X1+1;

				string strTmpStr = MakeSeparator(SepWidth, BoxType==NO_BOX?0:(BoxType==SINGLE_BOX||BoxType==SHORT_SINGLE_BOX?2:1));

				if (!CheckFlags(VMENU_NOMERGEBORDER) && I>0 && I<static_cast<int>(Items.size()-1) && SepWidth>3)
				{
					for (size_t J = 0; J < strTmpStr.size() - 3; ++J)
					{
						int PCorrection = !CheckFlags(VMENU_SHOWAMPERSAND) && Items[I - 1].strName.find(L'&') < J;
						int NCorrection = !CheckFlags(VMENU_SHOWAMPERSAND) && Items[I + 1].strName.find(L'&') < J;

						wchar_t PrevItem = (Items[I-1].strName.size() > J + PCorrection) ? Items[I-1].strName[J+PCorrection] : 0;
						wchar_t NextItem = (Items[I+1].strName.size() > J + NCorrection) ? Items[I+1].strName[J+NCorrection] : 0;

						if (!PrevItem && !NextItem)
							break;

						if (PrevItem==BoxSymbols[BS_V1])
						{
							if (NextItem==BoxSymbols[BS_V1])
								strTmpStr[J+(BoxType==NO_BOX?1:2) + 1] = BoxSymbols[BS_C_H1V1];
							else
								strTmpStr[J+(BoxType==NO_BOX?1:2) + 1] = BoxSymbols[BS_B_H1V1];
						}
						else if (NextItem==BoxSymbols[BS_V1])
						{
							strTmpStr[J+(BoxType==NO_BOX?1:2) + 1] = BoxSymbols[BS_T_H1V1];
						}
					}
				}

				SetColor(Colors[VMenuColorSeparator]);
				BoxText(strTmpStr,FALSE);

				if (!Items[I].strName.empty())
				{
					int ItemWidth = (int)Items[I].strName.size();

					if (ItemWidth > X2-X1-3)
						ItemWidth = X2-X1-3;

					GotoXY(X1+(X2-X1-1-ItemWidth)/2,Y);
					Global->FS << L" " << fmt::LeftAlign() << fmt::ExactWidth(ItemWidth) << Items[I].strName << L" ";
				}
			}
			else
			{
				if (BoxType!=NO_BOX)
				{
					SetColor(Colors[VMenuColorBox]);
					BoxText(BoxChar);
					GotoXY(X2,Y);
					BoxText(BoxChar);
				}

				if (BoxType!=NO_BOX)
					GotoXY(X1+1,Y);
				else
					GotoXY(X1,Y);

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
				int ShowPos = HiFindRealPos(Items[I].strName, Items[I].ShowPos, CheckFlags(VMENU_SHOWAMPERSAND));
				string strMItemPtr(Items[I].strName.data() + ShowPos);
				int strMItemPtrLen;

				if (CheckFlags(VMENU_SHOWAMPERSAND))
					strMItemPtrLen = static_cast<int>(strMItemPtr.size());
				else
					strMItemPtrLen = HiStrlen(strMItemPtr);

				// fit menu string into available space
				if (strMItemPtrLen > MaxLineWidth)
					strMItemPtr.resize(HiFindRealPos(strMItemPtr, MaxLineWidth, CheckFlags(VMENU_SHOWAMPERSAND)));

				// set highlight
				if (!VMFlags.Check(VMENU_SHOWAMPERSAND))
				{
					int AmpPos = Items[I].AmpPos - ShowPos;

					if ((AmpPos >= 0) && (static_cast<size_t>(AmpPos) < strMItemPtr.size()) && (strMItemPtr[AmpPos] != L'&'))
					{
						string strEnd = strMItemPtr.data() + AmpPos;
						strMItemPtr.resize(AmpPos);
						strMItemPtr += L"&";
						strMItemPtr += strEnd;
					}
				}

				strMenuLine.append(strMItemPtr);

				// табуляции меняем только при показе!!!
				// для сохранение оригинальной строки!!!
				std::replace(ALL_RANGE(strMenuLine), L'\t', L' ');

				FarColor Col;

				if (!(Items[I].Flags & LIF_DISABLE))
				{
					if (Items[I].Flags & LIF_SELECTED)
						Col = Colors[Items[I].Flags & LIF_GRAYED ? VMenuColorSelGrayed : VMenuColorHSelect];
					else
						Col = Colors[Items[I].Flags & LIF_GRAYED ? VMenuColorGrayed : VMenuColorHilite];
				}
				else
				{
					Col = Colors[VMenuColorDisabled];
				}

				if (CheckFlags(VMENU_SHOWAMPERSAND))
				{
					if (Items[I].Annotations.empty())
					{
						Text(strMenuLine);
					}
					else
					{
						int StartOffset = 1; // 1 is '<<' placeholder size
						size_t Pos = 0;
						FarColor InvColor = CurColor;
						std::swap(InvColor.ForegroundColor, InvColor.BackgroundColor);
						std::for_each(CONST_RANGE(Items[I].Annotations, i)
						{
							size_t pre_len = i.first - Items[I].ShowPos + StartOffset - Pos + 1;
							if (Pos < strMenuLine.size())
							{
								Text(strMenuLine.substr(Pos, pre_len));
								Pos += pre_len;
								if (Pos < strMenuLine.size())
								{
									SetColor(Col);
									Text(strMenuLine.substr(Pos, i.second));
									Pos += i.second;
									SetColor(CurColor);
								}
							}
						});
						if (Pos < strMenuLine.size())
							Text(strMenuLine.data() + Pos);
					}
				}
				else
					HiText(strMenuLine, Col);

				// сделаем добавочку для NO_BOX
				{
					int Width = X2-WhereX()+(BoxType==NO_BOX?1:0);
					if (Width > 0)
						Global->FS << fmt::MinWidth(Width) << L"";
				}

				if (Items[I].Flags & MIF_SUBMENU)
				{
					GotoXY(X1+(BoxType!=NO_BOX?1:0)+2+MaxLineWidth+(HasRightScroll?1:0)+1,Y);
					BoxText(L'\x25BA'); // sub menu arrow
				}

				SetColor(Colors[(Items[I].Flags&LIF_DISABLE)?VMenuColorArrowsDisabled:(Items[I].Flags&LIF_SELECTED?VMenuColorArrowsSelect:VMenuColorArrows)]);

				if (/*BoxType!=NO_BOX && */Items[I].ShowPos > 0)
				{
					GotoXY(X1+(BoxType!=NO_BOX?1:0)+1,Y);
					BoxText(L'\xab'); // '<<'
				}

				if (strMItemPtrLen > MaxLineWidth)
				{
					GotoXY(X1+(BoxType!=NO_BOX?1:0)+2+MaxLineWidth,Y);
					BoxText(L'\xbb'); // '>>'
				}
			}
		}
		else
		{
			if (BoxType!=NO_BOX)
			{
				SetColor(Colors[VMenuColorBox]);
				BoxText(BoxChar);
				GotoXY(X2,Y);
				BoxText(BoxChar);
				GotoXY(X1+1,Y);
			}
			else
			{
				GotoXY(X1,Y);
			}

			SetColor(Colors[VMenuColorText]);
			// сделаем добавочку для NO_BOX
			Global->FS << fmt::MinWidth(((BoxType!=NO_BOX)?X2-X1-1:X2-X1)+((BoxType==NO_BOX)?1:0)) << L"";
		}
	}

	if (CheckFlags(VMENU_LISTBOX|VMENU_ALWAYSSCROLLBAR) || Global->Opt->ShowMenuScrollbar)
	{
		SetColor(Colors[VMenuColorScrollBar]);

		if (BoxType!=NO_BOX)
			ScrollBarEx(X2,Y1+1,Y2-Y1-1,VisualTopPos,GetShowItemCount());
		else
			ScrollBarEx(X2,Y1,Y2-Y1+1,VisualTopPos,GetShowItemCount());
	}
}

int VMenu::CheckHighlights(wchar_t CheckSymbol, int StartPos)
{
	SCOPED_ACTION(CriticalSectionLock)(CS);

	if (CheckSymbol)
		CheckSymbol=Upper(CheckSymbol);

	for (size_t I=StartPos; I < Items.size(); I++)
	{
		if (!ItemIsVisible(Items[I].Flags))
			continue;

		wchar_t Ch = GetHighlights(&Items[I]);

		if (Ch)
		{
			if (CheckSymbol == Upper(Ch) || CheckSymbol == Upper(KeyToKeyLayout(Ch)))
				return static_cast<int>(I);
		}
		else if (!CheckSymbol)
		{
			return static_cast<int>(I);
		}
	}

	return -1;
}

wchar_t VMenu::GetHighlights(const MenuItemEx *_item)
{
	SCOPED_ACTION(CriticalSectionLock)(CS);

	wchar_t Ch = 0;

	if (_item)
	{
		if (_item->AmpPos != -1)
		{
			Ch = _item->strName[_item->AmpPos];
		}
		else if (!CheckFlags(VMENU_SHOWAMPERSAND))
		{
			auto AmpPos = _item->strName.find(L'&');
			if (AmpPos != string::npos && AmpPos + 1 != _item->strName.size())
			Ch = _item->strName[AmpPos + 1];
		}
	}

	return Ch;
}

void VMenu::AssignHighlights(int Reverse)
{
	SCOPED_ACTION(CriticalSectionLock)(CS);

	std::bitset<65536> Used;

	/* $ 02.12.2001 KM
	   + Поелику VMENU_SHOWAMPERSAND сбрасывается для корректной
	     работы ShowMenu сделаем сохранение энтого флага, в противном
	     случае если в диалоге использовался DI_LISTBOX без флага
	     DIF_LISTNOAMPERSAND, то амперсанды отображались в списке
	     только один раз до следующего ShowMenu.
	*/
	if (CheckFlags(VMENU_SHOWAMPERSAND))
		VMOldFlags.Set(VMENU_SHOWAMPERSAND);

	if (VMOldFlags.Check(VMENU_SHOWAMPERSAND))
		SetFlags(VMENU_SHOWAMPERSAND);

	int I, Delta = Reverse ? -1 : 1;

	// проверка заданных хоткеев
	for (I = Reverse ? static_cast<int>(Items.size()-1) : 0; I>=0 && I<static_cast<int>(Items.size()); I+=Delta)
	{
		wchar_t Ch = 0;
		int ShowPos = HiFindRealPos(Items[I].strName, Items[I].ShowPos, CheckFlags(VMENU_SHOWAMPERSAND));
		Items[I].AmpPos = -1;
		// TODO: проверка на LIF_HIDDEN
		size_t AmpPos = -1;
		if (!CheckFlags(VMENU_SHOWAMPERSAND))
		{
			AmpPos = Items[I].strName.find(L'&', ShowPos);
			if (AmpPos != string::npos && AmpPos + 1 != Items[I].strName.size())
				Ch = Items[I].strName[AmpPos + 1];
		}

		if (Ch && !Used[Upper(Ch)] && !Used[Lower(Ch)])
		{
			wchar_t ChKey=KeyToKeyLayout(Ch);
			Used[Upper(ChKey)] = true;
			Used[Lower(ChKey)] = true;
			Used[Upper(Ch)] = true;
			Used[Lower(Ch)] = true;
			Items[I].AmpPos = static_cast<short>(AmpPos + ShowPos);
		}
	}

	// TODO:  ЭТОТ цикл нужно уточнить - возможно вылезут артефакты (хотя не уверен)
	for (I = Reverse ? static_cast<int>(Items.size()-1) : 0; I>=0 && I<static_cast<int>(Items.size()); I+=Delta)
	{
		int ShowPos = HiFindRealPos(Items[I].strName, Items[I].ShowPos, CheckFlags(VMENU_SHOWAMPERSAND));
		const wchar_t *Name = Items[I].strName.data() + ShowPos;
		const wchar_t *ChPtr = wcschr(Name, L'&');

		if (!ChPtr || CheckFlags(VMENU_SHOWAMPERSAND))
		{
			// TODO: проверка на LIF_HIDDEN
			for (int J=0; Name[J]; J++)
			{
				wchar_t Ch = Name[J];

				if ((Ch == L'&' || IsAlpha(Ch) || (Ch >= L'0' && Ch <=L'9')) && !Used[Upper(Ch)] && !Used[Lower(Ch)])
				{
					wchar_t ChKey=KeyToKeyLayout(Ch);
					Used[Upper(ChKey)] = true;
					Used[Lower(ChKey)] = true;
					Used[Upper(Ch)] = true;
					Used[Lower(Ch)] = true;
					Items[I].AmpPos = J + ShowPos;
					break;
				}
			}
		}
	}

	SetFlags(VMENU_AUTOHIGHLIGHT|(Reverse?VMENU_REVERSEHIGHLIGHT:0));
	ClearFlags(VMENU_SHOWAMPERSAND);
}

bool VMenu::CheckKeyHiOrAcc(DWORD Key, int Type, int Translate,bool ChangePos, int& NewPos)
{
	SCOPED_ACTION(CriticalSectionLock)(CS);

	//не забудем сбросить EndLoop для листбокса, иначе не будут работать хоткеи в активном списке
	if (CheckFlags(VMENU_LISTBOX))
		ClearDone();

	FOR_CONST_RANGE(Items, Iterator)
	{
		auto& CurItem = *Iterator;
		if (ItemCanHaveFocus(CurItem.Flags) && ((!Type && CurItem.AccelKey && Key == CurItem.AccelKey) || (Type && (CurItem.AmpPos>=0 || !CheckFlags(VMENU_SHOWAMPERSAND)) && IsKeyHighlighted(CurItem.strName,Key,Translate,CurItem.AmpPos))))
		{
			NewPos=static_cast<int>(Iterator - Items.cbegin());
			if (ChangePos)
			{
				SetSelectPos(NewPos, 1);
				ShowMenu(true);
			}

			if ((!ParentDialog  || CheckFlags(VMENU_COMBOBOX|VMENU_LISTBOX)) && ItemCanBeEntered(Items[SelectPos].Flags))
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
	UpdateMaxLength((int)std::max(strTitle.size(),strBottomTitle.size())+2);
}

void VMenu::UpdateMaxLength(int Length)
{
	if (Length > MaxLength)
		MaxLength = Length;

	if (MaxLength > ScrX-8)
		MaxLength = ScrX-8;
}

void VMenu::SetMaxHeight(int NewMaxHeight)
{
	SCOPED_ACTION(CriticalSectionLock)(CS);

	MaxHeight = NewMaxHeight;

	if (MaxHeight > ScrY-6)
		MaxHeight = ScrY-6;
}

const string& VMenu::GetTitle(string &strDest)
{
	SCOPED_ACTION(CriticalSectionLock)(CS);

	strDest = strTitle;
	return strDest;
}

string &VMenu::GetBottomTitle(string &strDest)
{
	SCOPED_ACTION(CriticalSectionLock)(CS);

	strDest = strBottomTitle;
	return strDest;
}

void VMenu::SetBottomTitle(const wchar_t *BottomTitle)
{
	SCOPED_ACTION(CriticalSectionLock)(CS);

	SetFlags(VMENU_UPDATEREQUIRED);

	if (BottomTitle)
		strBottomTitle = BottomTitle;
	else
		strBottomTitle.clear();

	UpdateMaxLength((int)strBottomTitle.size() + 2);
}

void VMenu::SetTitle(const string& Title)
{
	SCOPED_ACTION(CriticalSectionLock)(CS);

	SetFlags(VMENU_UPDATEREQUIRED);

	strTitle = Title;

	UpdateMaxLength((int)strTitle.size() + 2);

	if (CheckFlags(VMENU_CHANGECONSOLETITLE))
	{
		if (!strTitle.empty())
		{
			if (!OldTitle)
				OldTitle = std::make_unique<ConsoleTitle>();

			ConsoleTitle::SetFarTitle(strTitle);
		}
		else
		{
			OldTitle.reset();
		}
	}
}

void VMenu::ResizeConsole()
{
	SCOPED_ACTION(CriticalSectionLock)(CS);

	if (SaveScr)
	{
		SaveScr->Discard();
		SaveScr.reset();
	}
}

void VMenu::SetBoxType(int BoxType)
{
	SCOPED_ACTION(CriticalSectionLock)(CS);

	VMenu::BoxType=BoxType;
}

void VMenu::SetColors(const FarDialogItemColors *ColorsIn)
{
	SCOPED_ACTION(CriticalSectionLock)(CS);

	if (ColorsIn)
	{
		std::copy(ColorsIn->Colors, ColorsIn->Colors + std::min(ARRAYSIZE(Colors), ColorsIn->ColorsCount), Colors);
	}
	else
	{
		int TypeMenu  = CheckFlags(VMENU_LISTBOX) ? 0 : (CheckFlags(VMENU_COMBOBOX) ? 1 : 2);
		int StyleMenu = CheckFlags(VMENU_WARNDIALOG) ? 1 : 0;

		if (CheckFlags(VMENU_DISABLED))
		{
			Colors[0] = ColorIndexToColor(StyleMenu?COL_WARNDIALOGDISABLED:COL_DIALOGDISABLED);

			for (int I=1; I < VMENU_COLOR_COUNT; ++I)
				Colors[I] = Colors[0];
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

			for (int I=0; I < VMENU_COLOR_COUNT; ++I)
				Colors[I] = ColorIndexToColor(StdColor[StyleMenu][TypeMenu][I]);
		}
	}
}

void VMenu::GetColors(FarDialogItemColors *ColorsOut)
{
	SCOPED_ACTION(CriticalSectionLock)(CS);

	std::copy(Colors, Colors + std::min(ARRAYSIZE(Colors), ColorsOut->ColorsCount), ColorsOut->Colors);
}

void VMenu::SetOneColor(int Index, PaletteColors Color)
{
	SCOPED_ACTION(CriticalSectionLock)(CS);

	if (Index < (int)ARRAYSIZE(Colors))
		Colors[Index] = ColorIndexToColor(Color);
}

BOOL VMenu::GetVMenuInfo(FarListInfo* Info)
{
	SCOPED_ACTION(CriticalSectionLock)(CS);

	if (Info)
	{
		Info->Flags = GetFlags() & (LINFO_SHOWNOBOX|LINFO_AUTOHIGHLIGHT|LINFO_REVERSEHIGHLIGHT|LINFO_WRAPMODE|LINFO_SHOWAMPERSAND);
		Info->ItemsNumber = Items.size();
		Info->SelectPos = SelectPos;
		Info->TopPos = TopPos;
		Info->MaxHeight = MaxHeight;
		Info->MaxLength = MaxLength + (ItemSubMenusCount > 0 ? 2 : 0);
		return TRUE;
	}

	return FALSE;
}

// Функция GetItemPtr - получить указатель на нужный Items.
MenuItemEx *VMenu::GetItemPtr(int Position)
{
	SCOPED_ACTION(CriticalSectionLock)(CS);

	int ItemPos = GetItemPosition(Position);

	if (ItemPos < 0)
		return nullptr;

	return &Items[ItemPos];
}

void *VMenu::_GetUserData(MenuItemEx *PItem, void *Data, size_t Size)
{
	if (Size && Data)
	{
		if (PItem->UserData && Size >= PItem->UserDataSize)
		{
			memcpy(Data, PItem->UserData, PItem->UserDataSize);
		}
	}

	return PItem->UserData;
}

size_t VMenu::GetUserDataSize(int Position)
{
	SCOPED_ACTION(CriticalSectionLock)(CS);

	int ItemPos = GetItemPosition(Position);

	if (ItemPos < 0)
		return 0;

	return Items[ItemPos].UserDataSize;
}

size_t VMenu::_SetUserData(MenuItemEx *PItem,
                        const void *Data,   // Данные
                        size_t Size)           // Размер, если =0 то предполагается, что в Data-строка
{
	xf_free(PItem->UserData);
	PItem->UserDataSize=0;
	PItem->UserData=nullptr;

	if (Data)
	{
		PItem->UserDataSize=Size;

		// Если Size==0, то подразумевается, что в Data находится zero-terminated wide string
		if (!PItem->UserDataSize)
		{
			PItem->UserDataSize = (wcslen(static_cast<const wchar_t *>(Data)) + 1)*sizeof(wchar_t);
		}

		PItem->UserData = xf_malloc(PItem->UserDataSize);
		memcpy(PItem->UserData, Data, PItem->UserDataSize);
	}

	return PItem->UserDataSize;
}

// Присовокупить к итему данные.
size_t VMenu::SetUserData(LPCVOID Data,   // Данные
                       size_t Size,     // Размер, если =0 то предполагается, что в Data-строка
                       int Position) // номер итема
{
	SCOPED_ACTION(CriticalSectionLock)(CS);

	int ItemPos = GetItemPosition(Position);

	if (ItemPos < 0)
		return 0;

	return _SetUserData(&Items[ItemPos], Data, Size);
}

// Получить данные
void* VMenu::GetUserData(void *Data,size_t Size,int Position)
{
	SCOPED_ACTION(CriticalSectionLock)(CS);

	int ItemPos = GetItemPosition(Position);

	if (ItemPos < 0)
		return nullptr;

	return _GetUserData(&Items[ItemPos], Data, Size);
}

FarListItem *VMenu::MenuItem2FarList(const MenuItemEx *MItem, FarListItem *FItem)
{
	if (FItem && MItem)
	{
		ClearStruct(*FItem);
		FItem->Flags = MItem->Flags;
		FItem->Text = MItem->strName.data();
		return FItem;
	}

	return nullptr;
}

int VMenu::GetTypeAndName(string &strType, string &strName)
{
	SCOPED_ACTION(CriticalSectionLock)(CS);

	strType = MSG(MVMenuType);
	strName = strTitle;
	return CheckFlags(VMENU_COMBOBOX) ? MODALTYPE_COMBOBOX : MODALTYPE_VMENU;
}

// return Pos || -1
int VMenu::FindItem(const FarListFind *FItem)
{
	return FindItem(FItem->StartIndex,FItem->Pattern,FItem->Flags);
}

int VMenu::FindItem(int StartIndex,const string& Pattern,UINT64 Flags)
{
	SCOPED_ACTION(CriticalSectionLock)(CS);

	if ((DWORD)StartIndex < (DWORD)Items.size())
	{
		size_t LenPattern = Pattern.size();

		for (size_t I=StartIndex; I < Items.size(); I++)
		{
			string strTmpBuf(Items[I].strName);
			size_t LenNamePtr = strTmpBuf.size();
			RemoveChar(strTmpBuf, L'&');

			if (Flags&LIFIND_EXACTMATCH)
			{
				if (!StrCmpNI(strTmpBuf.data(),Pattern.data(),static_cast<int>(std::max(LenPattern, LenNamePtr))))
					return static_cast<int>(I);
			}
			else
			{
				if (CmpName(Pattern.data(),strTmpBuf.data(),true))
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
	SortItems([](const MenuItemEx& a, const MenuItemEx& b, const SortItemParam& Param)->bool
	{
		string strName1(a.strName);
		string strName2(b.strName);
		RemoveChar(strName1, L'&', true);
		RemoveChar(strName2, L'&', true);
		bool Less = StrCmpI(strName1.data()+Param.Offset, strName2.data() + Param.Offset) < 0;
		return Param.Reverse? !Less : Less;
	}, Reverse, Offset);
}

bool VMenu::Pack()
{
	auto OldItemCount=Items.size();
	size_t FirstIndex=0;

	while (FirstIndex<Items.size())
	{
		size_t LastIndex=Items.size()-1;
		while (LastIndex>FirstIndex)
		{
			if (!(Items[FirstIndex].Flags & LIF_SEPARATOR) && !(Items[LastIndex].Flags & LIF_SEPARATOR))
			{
				if (Items[FirstIndex].strName == Items[LastIndex].strName)
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

void VMenu::SetId(const GUID& Id)
{
	MenuId=Id;
}

const GUID& VMenu::Id() const
{
	return MenuId;
}

void VMenu::AddHotkeys(std::vector<string>& Strings, MenuDataEx* Menu, size_t MenuSize)
{
	size_t MaxLength = 0;
	std::for_each(Menu, Menu + MenuSize, [&](const MenuDataEx& i) { MaxLength = std::max(MaxLength, wcslen(i.Name)); });
	for (size_t i = 0; i < MenuSize; ++i)
	{
		if (!(Menu[i].Flags & LIF_SEPARATOR) && Menu[i].AccelKey)
		{
			string Key;
			KeyToLocalizedText(Menu[i].AccelKey, Key);
			bool Hl = HiStrlen(Menu[i].Name) != static_cast<int>(wcslen(Menu[i].Name));
			Strings[i] = FormatString() << fmt::ExactWidth(MaxLength + (Hl? 2 : 1)) << fmt::LeftAlign() << Menu[i].Name << Key;
			Menu[i].Name = Strings[i].data();
		}
	}
}
