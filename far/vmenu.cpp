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
#include "palette.hpp"
#include "config.hpp"
#include "processname.hpp"
#include "pathmix.hpp"
#include "cmdline.hpp"
#include "FarGuid.hpp"

VMenu::VMenu(const wchar_t *Title,       // заголовок меню
             MenuDataEx *Data, // пункты меню
             int ItemCount,     // количество пунктов меню
             int MaxHeight,     // максимальная высота
             DWORD Flags,       // нужен ScrollBar?
             FARWINDOWPROC Proc,    // обработчик
             Dialog *ParentDialog
            ):  // родитель для ListBox
	strTitle(Title),
	SelectPos(-1),
	TopPos(0),
	WasAutoHeight(false),
	MaxLength(0),
	BoxType(DOUBLE_BOX),
	ParentDialog(ParentDialog),
	VMenuProc(Proc?Proc:(FARWINDOWPROC)VMenu::DefMenuProc),
	OldTitle(nullptr),
	Used(new bool[WCHAR_MAX]),
	bFilterEnabled(false),
	bFilterLocked(false),
	Item(nullptr),
	ItemCount(0),
	ItemHiddenCount(0),
	ItemSubMenusCount(0),
	MenuId(FarGuid)
{
	SaveScr=nullptr;
	SetDynamicallyBorn(false);
	SetFlags(Flags|VMENU_MOUSEREACTION|VMENU_UPDATEREQUIRED);
	ClearFlags(VMENU_SHOWAMPERSAND|VMENU_MOUSEDOWN);
	GetCursorType(PrevCursorVisible,PrevCursorSize);
	bRightBtnPressed = false;

	// инициализируем перед тем, как добавлять айтема
	UpdateMaxLengthFromTitles();

	MenuItemEx NewItem;

	for (int I=0; I < ItemCount; I++)
	{
		NewItem.Clear();

		if (!IsPtr(Data[I].Name))
			NewItem.strName = MSG(static_cast<LNGID>(reinterpret_cast<intptr_t>(Data[I].Name)));
		else
			NewItem.strName = Data[I].Name;

		//NewItem.AmpPos = -1;
		NewItem.AccelKey = Data[I].AccelKey;
		NewItem.Flags = Data[I].Flags;
		AddItem(&NewItem);
	}

	SetMaxHeight(MaxHeight);
	SetColors(nullptr); //Установим цвет по умолчанию

	if (!CheckFlags(VMENU_LISTBOX) && CtrlObject)
	{
		PrevMacroMode = CtrlObject->Macro.GetMode();

		if (!IsMenuArea(PrevMacroMode))
			CtrlObject->Macro.SetMode(MACRO_MENU);
	}

	if (!CheckFlags(VMENU_LISTBOX))
		FrameManager->ModalizeFrame(this);
}

VMenu::~VMenu()
{
	if (!CheckFlags(VMENU_LISTBOX) && CtrlObject)
		CtrlObject->Macro.SetMode(PrevMacroMode);

	bool WasVisible=Flags.Check(FSCROBJ_VISIBLE)!=0;
	Hide();
	DeleteItems();
	delete[] Used;
	SetCursorType(PrevCursorVisible,PrevCursorSize);

	if (!CheckFlags(VMENU_LISTBOX))
	{
		FrameManager->UnmodalizeFrame(this);
		if(WasVisible)
		{
			FrameManager->RefreshFrame();
		}
	}
}

void VMenu::ResetCursor()
{
	GetCursorType(PrevCursorVisible,PrevCursorSize);
}

//может иметь фокус
bool VMenu::ItemCanHaveFocus(UINT64 Flags)
{
	return !(Flags&(LIF_DISABLE|LIF_HIDDEN|LIF_SEPARATOR));
}

//может быть выбран
bool VMenu::ItemCanBeEntered(UINT64 Flags)
{
	return !(Flags&(LIF_DISABLE|LIF_HIDDEN|LIF_GRAYED|LIF_SEPARATOR));
}

//видимый
bool VMenu::ItemIsVisible(UINT64 Flags)
{
	return !(Flags&(LIF_HIDDEN));
}

bool VMenu::UpdateRequired()
{
	CriticalSectionLock Lock(CS);

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
	UpdateInternalCounters(Item[Pos]->Flags, NewFlags);

	if (!ItemCanHaveFocus(NewFlags))
		NewFlags &= ~LIF_SELECTED;

	//remove selection
	if ((Item[Pos]->Flags&LIF_SELECTED) && !(NewFlags&LIF_SELECTED))
	{
		SelectPos = -1;
	}
	//set new selection
	else if (!(Item[Pos]->Flags&LIF_SELECTED) && (NewFlags&LIF_SELECTED))
	{
		if (SelectPos>=0)
			Item[SelectPos]->Flags &= ~LIF_SELECTED;

		SelectPos = Pos;
	}

	Item[Pos]->Flags = NewFlags;

	if (SelectPos < 0)
		SetSelectPos(0,1);

	if(LOWORD(Item[Pos]->Flags))
	{
		Item[Pos]->Flags|=LIF_CHECKED;
		if(LOWORD(Item[Pos]->Flags)==1)
		{
			Item[Pos]->Flags&=0xFFFF0000;
		}
	}
}

// переместить курсор c учётом пунктов которые не могу получать фокус
int VMenu::SetSelectPos(int Pos, int Direct, bool stop_on_edge)
{
	CriticalSectionLock Lock(CS);

	if (!Item || !ItemCount)
		return -1;

	for (int Pass=0, I=0;;I++)
	{
		if (Pos<0)
		{
			//if (CheckFlags(VMENU_WRAPMODE))
			{
				Pos = ItemCount-1;
			}
			//else
			//{
			//	Pos = 0;
			//	TopPos = 0;
			//	Pass++;
			//}
		}

		if (Pos>=ItemCount)
		{
			//if (CheckFlags(VMENU_WRAPMODE))
			{
				Pos = 0;
				TopPos = 0;
			}
			//else
			//{
			//	Pos = ItemCount-1;
			//	Pass++;
			//}
		}

		if (ItemCanHaveFocus(Item[Pos]->Flags))
			break;

		if (Pass)
			return SelectPos;

		Pos += Direct;

		if (I>=ItemCount) // круг пройден - ничего не найдено :-(
			Pass++;
	}

	// wrap-stop for all lists (not only VMENU_WRAPMODE)
	if (stop_on_edge && /*CheckFlags(VMENU_WRAPMODE) &&*/ ((Direct>0 && Pos<SelectPos) || (Direct<0 && Pos>SelectPos)))
		return SelectPos;

	UpdateItemFlags(Pos, Item[Pos]->Flags|LIF_SELECTED);

	SetFlags(VMENU_UPDATEREQUIRED);

	return Pos;
}

// установить курсор и верхний итем
int VMenu::SetSelectPos(FarListPos *ListPos, int Direct)
{
	CriticalSectionLock Lock(CS);

	int Ret = SetSelectPos(ListPos->SelectPos,Direct ? Direct : ListPos->SelectPos > SelectPos? 1 : -1);

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
	CriticalSectionLock Lock(CS);

	if (!Item || !ItemCount)
		return;

	// если selection стоит в некорректном месте - сбросим его
	if (SelectPos >= 0 && !ItemCanHaveFocus(Item[SelectPos]->Flags))
		SelectPos = -1;

	for (int i=0; i<ItemCount; i++)
	{
		if (!ItemCanHaveFocus(Item[i]->Flags))
		{
			Item[i]->SetSelect(FALSE);
		}
		else
		{
			if (SelectPos == -1)
			{
				Item[i]->SetSelect(TRUE);
				SelectPos = i;
			}
			else if (SelectPos != i)
			{
				Item[i]->SetSelect(FALSE);
			}
			else
			{
				Item[i]->SetSelect(TRUE);
			}
		}
	}
}

int VMenu::GetItemPosition(int Position)
{
	CriticalSectionLock Lock(CS);

	int DataPos = (Position==-1) ? SelectPos : Position;

	if (DataPos>=ItemCount)
		DataPos = -1; //ItemCount-1;

	return DataPos;
}

// получить позицию курсора и верхнюю позицию итема
int VMenu::GetSelectPos(FarListPos *ListPos)
{
	CriticalSectionLock Lock(CS);

	ListPos->SelectPos=SelectPos;
	ListPos->TopPos=TopPos;

	return ListPos->SelectPos;
}

int VMenu::InsertItem(const FarListInsert *NewItem)
{
	CriticalSectionLock Lock(CS);

	if (NewItem)
	{
		MenuItemEx MItem;

		if (AddItem(FarList2MenuItem(&NewItem->Item,&MItem),NewItem->Index) >= 0)
			return ItemCount;
	}

	return -1;
}

int VMenu::AddItem(const FarList *List)
{
	CriticalSectionLock Lock(CS);

	if (List && List->Items)
	{
		MenuItemEx MItem;

		for (size_t i=0; i<List->ItemsNumber; i++)
		{
			AddItem(FarList2MenuItem(&List->Items[i], &MItem));
		}
	}

	return ItemCount;
}

int VMenu::AddItem(const wchar_t *NewStrItem)
{
	CriticalSectionLock Lock(CS);

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

	FarList FarList0={1,&FarListItem0};

	return AddItem(&FarList0)-1; //-1 потому что AddItem(FarList) возвращает количество элементов
}

int VMenu::AddItem(const MenuItemEx *NewItem,int PosAdd)
{
	CriticalSectionLock Lock(CS);

	if (!NewItem)
		return -1;

	if (PosAdd > ItemCount)
		PosAdd = ItemCount;

	// Если < 0 - однозначно ставим в нулевую позицию, т.е добавка сверху
	if (PosAdd < 0)
		PosAdd = 0;

	SetFlags(VMENU_UPDATEREQUIRED|(bFilterEnabled?VMENU_REFILTERREQUIRED:0));

	if (!(ItemCount & 255))
	{
		MenuItemEx **NewPtr;
		if (!(NewPtr=(MenuItemEx **)xf_realloc(Item, sizeof(*Item)*(ItemCount+256+1))))
			return -1;

		Item=NewPtr;
	}

	if (PosAdd < ItemCount)
		memmove(Item+PosAdd+1,Item+PosAdd,sizeof(*Item)*(ItemCount-PosAdd)); //??

	if (PosAdd <= SelectPos)
		SelectPos++;

	ItemCount++;

	Item[PosAdd] = new MenuItemEx;
	Item[PosAdd]->Clear();
	Item[PosAdd]->Flags = 0;
	Item[PosAdd]->strName = NewItem->strName;
	Item[PosAdd]->AccelKey = NewItem->AccelKey;
	_SetUserData(Item[PosAdd], NewItem->UserData, NewItem->UserDataSize);
	//Item[PosAdd]->AmpPos = NewItem->AmpPos;
	Item[PosAdd]->AmpPos = -1;
	Item[PosAdd]->Len[0] = NewItem->Len[0];
	Item[PosAdd]->Len[1] = NewItem->Len[1];
	Item[PosAdd]->Idx2 = NewItem->Idx2;
	//Item[PosAdd]->ShowPos = NewItem->ShowPos;
	Item[PosAdd]->ShowPos = 0;

	if (CheckFlags(VMENU_SHOWAMPERSAND))
		UpdateMaxLength((int)Item[PosAdd]->strName.GetLength());
	else
		UpdateMaxLength(HiStrlen(Item[PosAdd]->strName));

	UpdateItemFlags(PosAdd, NewItem->Flags);

	return ItemCount-1;
}

int VMenu::UpdateItem(const FarListUpdate *NewItem)
{
	CriticalSectionLock Lock(CS);

	if (NewItem && (DWORD)NewItem->Index < (DWORD)ItemCount)
	{
		// Освободим память... от ранее занятого ;-)
		MenuItemEx *PItem = Item[NewItem->Index];

		if (NewItem->Item.Flags&LIF_DELETEUSERDATA)
		{
			xf_free(PItem->UserData);
			PItem->UserData = nullptr;
			PItem->UserDataSize = 0;
		}

		MenuItemEx MItem;
		FarList2MenuItem(&NewItem->Item, &MItem);

		PItem->strName = MItem.strName;

		UpdateItemFlags(NewItem->Index, MItem.Flags);

		SetFlags(VMENU_UPDATEREQUIRED|(bFilterEnabled?VMENU_REFILTERREQUIRED:0));

		return TRUE;
	}

	return FALSE;
}

//функция удаления N пунктов меню
int VMenu::DeleteItem(int ID, int Count)
{
	CriticalSectionLock Lock(CS);

	if (ID < 0 || ID >= ItemCount || Count <= 0)
		return ItemCount;

	if (ID+Count > ItemCount)
		Count=ItemCount-ID;

	if (Count <= 0)
		return ItemCount;

	if (!ID && Count == ItemCount)
	{
		DeleteItems();
		return ItemCount;
	}

	// Надобно удалить данные, чтоб потери по памяти не были
	for (int I=0; I < Count; ++I)
	{
		MenuItemEx *PtrItem = Item[ID+I];

		xf_free(PtrItem->UserData);

		UpdateInternalCounters(PtrItem->Flags,0);
	}

	// а вот теперь перемещения
	if (ItemCount > 1)
		memmove(Item+ID,Item+ID+Count,sizeof(*Item)*(ItemCount-(ID+Count))); //BUGBUG

	ItemCount -= Count;

	// коррекция текущей позиции
	if (SelectPos >= ID && SelectPos < ID+Count)
	{
		if(SelectPos==ItemCount)
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

	return ItemCount;
}

void VMenu::DeleteItems()
{
	CriticalSectionLock Lock(CS);

	if (Item)
	{
		for (int I=0; I < ItemCount; ++I)
		{
			xf_free(Item[I]->UserData);

			delete Item[I];
		}

		xf_free(Item);
	}

	Item=nullptr;
	ItemCount=0;
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
	CriticalSectionLock Lock(CS);

	int ItemPos = GetItemPosition(Position);

	if (ItemPos < 0)
		return 0;

	if (Item[ItemPos]->Flags & LIF_SEPARATOR)
		return 0;

	if (!(Item[ItemPos]->Flags & LIF_CHECKED))
		return 0;

	int Checked = Item[ItemPos]->Flags & 0xFFFF;

	return Checked ? Checked : 1;
}


void VMenu::SetCheck(int Check, int Position)
{
	CriticalSectionLock Lock(CS);

	int ItemPos = GetItemPosition(Position);

	if (ItemPos < 0)
		return;

	Item[ItemPos]->SetCheck(Check);
}

void VMenu::RestoreFilteredItems()
{
	for (int i=0; i < ItemCount; i++)
	{
		Item[i]->Flags &= ~LIF_HIDDEN;
	}

	ItemHiddenCount=0;

	FilterUpdateHeight();

	// Подровнять, а то в нижней части списка может оставаться куча пустых строк
	FarListPos pos={sizeof(FarListPos),SelectPos < 0 ? 0 : SelectPos,-1};
	SetSelectPos(&pos);
}

void VMenu::FilterStringUpdated(bool bLonger)
{
	int PrevSeparator = -1, PrevGroup = -1;
	int UpperVisible = -1, LowerVisible = -2;
	bool bBottomMode = false;

	if (SelectPos > 0)
	{
		// Определить, в верхней или нижней части расположен курсор
		int TopVisible = GetVisualPos(TopPos);
		int SelectedVisible = GetVisualPos(SelectPos);
		int BottomVisible = (TopVisible+MaxHeight > GetShowItemCount()) ? (TopVisible+MaxHeight-1) : (GetShowItemCount()-1);
		if (SelectedVisible >= ((TopVisible+BottomVisible)>>1))
			bBottomMode = true;
	}

	if (bLonger)
	{
		//строка фильтра увеличилась
		for (int i=0; i < ItemCount; i++)
		{
			// Нет смысла накладывать фильтр на разделители
			if (ItemIsVisible(Item[i]->Flags))
			{
				if (Item[i]->Flags & LIF_SEPARATOR)
				{
					// В предыдущей группе все элементы скрыты, разделитель перед группой - не нужен
					if (PrevSeparator != -1)
					{
						Item[PrevSeparator]->Flags |= LIF_HIDDEN;
						ItemHiddenCount++;
					}
					if (Item[i]->strName.IsEmpty() && PrevGroup == -1)
					{
						Item[i]->Flags |= LIF_HIDDEN;
						ItemHiddenCount++;
						PrevSeparator = -1;
					}
					else
					{
						PrevSeparator = i;
					}
				}
				else if (!StrStrI(Item[i]->strName, strFilter))
				{
					Item[i]->Flags |= LIF_HIDDEN;
					ItemHiddenCount++;
					if (SelectPos == i)
					{
						Item[i]->Flags &= ~LIF_SELECTED;
						SelectPos = -1;
						LowerVisible = -1;
					}
				}
				else
				{
					PrevGroup = i;
					if (LowerVisible == -2)
					{
						if (ItemCanHaveFocus(Item[i]->Flags))
							UpperVisible = i;
					}
					else if (LowerVisible == -1)
					{
						if (ItemCanHaveFocus(Item[i]->Flags))
							LowerVisible = i;
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
			Item[PrevSeparator]->Flags |= LIF_HIDDEN;
			ItemHiddenCount++;
		}
	}
	else
	{
		//строка фильтра сократилась
		for (int i=0; i < ItemCount; i++)
		{
			if (!ItemIsVisible(Item[i]->Flags))
			{
				if (Item[i]->Flags & LIF_SEPARATOR)
				{
					PrevSeparator = i;
				}
				else if (StrStrI(Item[i]->strName, strFilter))
				{
					Item[i]->Flags &= ~LIF_HIDDEN;
					ItemHiddenCount--;
				}
			}

			if (ItemIsVisible(Item[i]->Flags))
			{
				if (Item[i]->Flags & LIF_SEPARATOR)
				{
					PrevGroup = -1;
					if ((PrevSeparator != -1))
						PrevSeparator = -1;
				}
				else
				{
					if (PrevSeparator != -1)
					{
						if (!Item[PrevSeparator]->strName.IsEmpty() || PrevGroup != -1)
						{
							// Разделитель перед ранее скрытой группой был скрыт
							Item[PrevSeparator]->Flags &= ~LIF_HIDDEN;
							ItemHiddenCount--;
						}
						PrevSeparator = -1;
					}
					PrevGroup = i;
				}
			}
		}

		FilterUpdateHeight();
	}

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
	if (WasAutoHeight)
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
			SetPosition(X1,Y1,X2,NewY2);
		}
	}
}

bool VMenu::IsFilterEditKey(int Key)
{
	return (Key>=(int)KEY_SPACE && Key<0xffff) || Key==KEY_BS;
}

bool VMenu::ShouldSendKeyToFilter(int Key)
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

#ifdef FAR_LUA_TEMP
#else
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
			return ItemCount > 0 && SelectPos >= 0;
		case MCODE_V_ITEMCOUNT:
			return GetShowItemCount();
		case MCODE_V_CURPOS:
			return GetVisualPos(SelectPos)+1;
		case MCODE_F_MENU_CHECKHOTKEY:
		{
			const wchar_t *str = (const wchar_t *)vParam;
			return (__int64)(GetVisualPos(CheckHighlights(*str,(int)iParam))+1);
		}
		case MCODE_F_MENU_SELECT:
		{
			const wchar_t *str = (const wchar_t *)vParam;

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
				int EndPos=ItemCount-1;

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

					if (!ItemIsVisible(_item->Flags))
						continue;

					Res = 0;
					RemoveExternalSpaces(HiText2Str(strTemp,_item->strName));
					const wchar_t *p;

					switch (iParam)
					{
						case 0: // full compare
							Res = !StrCmpI(strTemp,str);
							break;
						case 1: // begin compare
							p = StrStrI(strTemp,str);
							Res = p==strTemp;
							break;
						case 2: // end compare
							p = RevStrStrI(strTemp,str);
							Res = p && !*(p+StrLength(str));
							break;
						case 3: // in str
							Res = StrStrI(strTemp,str)!=nullptr;
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

			if (Param>=0 && Param<ItemCount)
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
								strFilter.Clear();
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
					RetValue = (bFilterEnabled && !strFilter.IsEmpty()) ? 1 : 0;
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
					strFilter.Clear();
					if (vParam!=nullptr)
						AddToFilter(((string *)vParam)->CPtr());
					FilterStringUpdated(true);
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
			return reinterpret_cast<intptr_t>(strId.CPtr());
		}

	}

	return 0;
}
#endif

bool VMenu::AddToFilter(const wchar_t *str)
{
	int Key;

	if (bFilterEnabled && !bFilterLocked)
	{
		while ((Key=*str) )
		{
			if( IsFilterEditKey(Key) )
			{
				if ( Key==KEY_BS && !strFilter.IsEmpty() )
					strFilter.SetLength(strFilter.GetLength()-1);
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

int VMenu::ProcessKey(int Key)
{
	CriticalSectionLock Lock(CS);

	if (Key==KEY_NONE || Key==KEY_IDLE)
		return FALSE;

	if (Key == KEY_OP_PLAINTEXT)
	{
		const wchar_t *str = eStackAsString();

		if (!*str)
			return FALSE;

		if ( AddToFilter(str) ) // для фильтра: всю строку целиком в фильтр, а там разберемся.
		{
			if (strFilter.IsEmpty())
				RestoreFilteredItems();
			else
				FilterStringUpdated(true);

			DisplayObject();

			return TRUE;
		}
		else // не для фильтра: по старинке, первый символ последовательности, остальное игнорируем (ибо некуда)
			Key=*str;
	}

	SetFlags(VMENU_UPDATEREQUIRED);

	if (!GetShowItemCount())
	{
		if ((Key!=KEY_F1 && Key!=KEY_SHIFTF1 && Key!=KEY_F10 && Key!=KEY_ESC && Key!=KEY_ALTF9 && Key!=KEY_RALTF9))
		{
			if (!bFilterEnabled || (bFilterEnabled && Key!=KEY_BS && Key!=KEY_CTRLALTF && Key!=KEY_RCTRLRALTF && Key!=KEY_CTRLRALTF && Key!=KEY_RCTRLALTF))
			{
				Modal::ExitCode = -1;
				return FALSE;
			}
		}
	}

	if (!(((unsigned int)Key >= KEY_MACRO_BASE && (unsigned int)Key <= KEY_MACRO_ENDBASE) || ((unsigned int)Key >= KEY_OP_BASE && (unsigned int)Key <= KEY_OP_ENDBASE)))
	{
		DWORD S=Key&(KEY_CTRL|KEY_ALT|KEY_SHIFT|KEY_RCTRL|KEY_RALT);
		DWORD K=Key&(~(KEY_CTRL|KEY_ALT|KEY_SHIFT|KEY_RCTRL|KEY_RALT));

		if (K==KEY_MULTIPLY)
			Key = L'*'|S;
		else if (K==KEY_ADD)
			Key = L'+'|S;
		else if (K==KEY_SUBTRACT)
			Key = L'-'|S;
		else if (K==KEY_DIVIDE)
			Key = L'/'|S;
	}

	switch (Key)
	{
		case KEY_ALTF9:
		case KEY_RALTF9:
			FrameManager->ProcessKey(KEY_ALTF9);
			break;
		case KEY_NUMENTER:
		case KEY_ENTER:
		{
			if (!ParentDialog || CheckFlags(VMENU_COMBOBOX))
			{
				if (ItemCanBeEntered(Item[SelectPos]->Flags))
				{
					EndLoop = TRUE;
					Modal::ExitCode = SelectPos;
				}
			}

			break;
		}
		case KEY_ESC:
		case KEY_F10:
		{
			if (!ParentDialog || CheckFlags(VMENU_COMBOBOX))
			{
				EndLoop = TRUE;
				Modal::ExitCode = -1;
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
			SetSelectPos(ItemCount-1,-1);
			ShowMenu(true);
			break;
		}
		case KEY_PGUP:         case KEY_NUMPAD9:
		{
			int dy = ((BoxType!=NO_BOX)?Y2-Y1-1:Y2-Y1);

			int p = VisualPosToReal(GetVisualPos(SelectPos)-dy);

			if (p < 0)
				p = 0;

			SetSelectPos(p,1);
			ShowMenu(true);
			break;
		}
		case KEY_PGDN:         case KEY_NUMPAD3:
		{
			int dy = ((BoxType!=NO_BOX)?Y2-Y1-1:Y2-Y1);

			int p = VisualPosToReal(GetVisualPos(SelectPos)+dy);;

			if (p >= ItemCount)
				p = ItemCount-1;

			SetSelectPos(p,-1);
			ShowMenu(true);
			break;
		}
		case KEY_ALTHOME:           case KEY_ALT|KEY_NUMPAD7:
		case KEY_RALTHOME:          case KEY_RALT|KEY_NUMPAD7:
		case KEY_ALTEND:            case KEY_ALT|KEY_NUMPAD1:
		case KEY_RALTEND:           case KEY_RALT|KEY_NUMPAD1:
		{
			if (Key == KEY_ALTHOME || Key == KEY_RALTHOME || Key == (KEY_ALT|KEY_NUMPAD7) || Key == (KEY_RALT|KEY_NUMPAD7))
			{
				for (int I=0; I < ItemCount; ++I)
					Item[I]->ShowPos=0;
			}
			else
			{
				int _len;

				for (int I=0; I < ItemCount; ++I)
				{
					if (CheckFlags(VMENU_SHOWAMPERSAND))
						_len=static_cast<int>(Item[I]->strName.GetLength());
					else
						_len=HiStrlen(Item[I]->strName);

					if (_len >= MaxLineWidth)
						Item[I]->ShowPos = _len - MaxLineWidth;
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

			for (int I=0; I < ItemCount; ++I)
				if (ShiftItemShowPos(I,(Key == KEY_ALTLEFT || Key == KEY_RALTLEFT || Key == (KEY_ALT|KEY_NUMPAD4) || Key == (KEY_RALT|KEY_NUMPAD4) || Key == KEY_MSWHEEL_LEFT)?-1:1))
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
			if (ShiftItemShowPos(SelectPos,(Key == KEY_ALTSHIFTLEFT || Key == KEY_RALTSHIFTLEFT || Key == (KEY_ALT|KEY_SHIFT|KEY_NUMPAD4) || Key == (KEY_RALT|KEY_SHIFT|KEY_NUMPAD4))?-1:1))
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
			if(SelectPos < ItemCount-1)
			{
				FarListPos Pos = {sizeof(Pos), SelectPos+1, TopPos+1};
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

		case KEY_CTRLALTF:
		case KEY_RCTRLRALTF:
		case KEY_CTRLRALTF:
		case KEY_RCTRLALTF:
		{
			bFilterEnabled=!bFilterEnabled;
			bFilterLocked=false;
			strFilter.Clear();

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
				wchar_t *ClipText=PasteFromClipboard();

				if (!ClipText)
					return TRUE;

				if ( AddToFilter(ClipText) )
				{
					if (strFilter.IsEmpty())
						RestoreFilteredItems();
					else
						FilterStringUpdated(true);

					DisplayObject();
				}

				xf_free(ClipText);
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
		case KEY_TAB:
		case KEY_SHIFTTAB:
		default:
		{
			if (bFilterEnabled && !bFilterLocked && IsFilterEditKey(Key))
			{
				if (Key==KEY_BS)
				{
					if (!strFilter.IsEmpty())
					{
						strFilter.SetLength(strFilter.GetLength()-1);

						if (strFilter.IsEmpty())
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

				FilterStringUpdated(Key!=KEY_BS);
				DisplayObject();

				return TRUE;
			}

			int OldSelectPos=SelectPos;

			if (!CheckKeyHiOrAcc(Key,0,0))
			{
				if (Key == KEY_SHIFTF1 || Key == KEY_F1)
				{
					if (ParentDialog)
						;//ParentDialog->ProcessKey(Key);
					else
						ShowHelp();

					break;
				}
				else
				{
					if (!CheckKeyHiOrAcc(Key,1,FALSE))
						CheckKeyHiOrAcc(Key,1,TRUE);
				}
			}

			if (ParentDialog && SendDlgMessage((HANDLE)ParentDialog,DN_LISTHOTKEY,DialogItemID,ToPtr(SelectPos)))
			{
				UpdateItemFlags(OldSelectPos,Item[OldSelectPos]->Flags|LIF_SELECTED);
				ShowMenu(true);
				EndLoop = FALSE;
				break;
			}

			return FALSE;
		}
	}

	return TRUE;
}

int VMenu::ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent)
{
	CriticalSectionLock Lock(CS);

	SetFlags(VMENU_UPDATEREQUIRED);

	if (!GetShowItemCount())
	{
		if (MouseEvent->dwButtonState && !MouseEvent->dwEventFlags)
			EndLoop=TRUE;

		Modal::ExitCode=-1;
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
			ProcessKey(KEY_ENTER);
		}
		return TRUE;
	}

	if (MouseEvent->dwButtonState&FROM_LEFT_1ST_BUTTON_PRESSED&&(MsX==X1+2||MsX==X2-1-((CheckFlags(VMENU_COMBOBOX)||CheckFlags(VMENU_LISTBOX))?0:2)))
	{
		while (IsMouseButtonPressed())
			ProcessKey(MsX==X1+2?KEY_ALTLEFT:KEY_ALTRIGHT);

		return TRUE;
	}

	int SbY1 = ((BoxType!=NO_BOX)?Y1+1:Y1), SbY2=((BoxType!=NO_BOX)?Y2-1:Y2);
	bool bShowScrollBar = false;

	if (CheckFlags(VMENU_LISTBOX|VMENU_ALWAYSSCROLLBAR) || Opt.ShowMenuScrollbar)
		bShowScrollBar = true;

	if (bShowScrollBar && MsX==X2 && ((BoxType!=NO_BOX)?Y2-Y1-1:Y2-Y1+1)<ItemCount && (MouseEvent->dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED))
	{
		if (MsY==SbY1)
		{
			while (IsMouseButtonPressed())
			{
				//прокрутка мышью не должна врапить меню
				if (SelectPos>=0 && GetVisualPos(SelectPos))
					ProcessKey(KEY_UP);

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
					ProcessKey(KEY_DOWN);

				ShowMenu(true);
			}

			return TRUE;
		}

		if (MsY>SbY1 && MsY<SbY2)
		{
			int SbHeight;
			int Delta=0;

			while (IsMouseButtonPressed())
			{
				SbHeight=Y2-Y1-2;
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
				ProcessKey(KEY_UP);

			return TRUE;
		}

		if (MsY==Y2)
		{
			while (MsY==Y2 && GetVisualPos(SelectPos)<GetShowItemCount()-1 && IsMouseButtonPressed())
				ProcessKey(KEY_DOWN);

			return TRUE;
		}
	}

	if ((BoxType!=NO_BOX)?
	        (MsX>X1 && MsX<X2 && MsY>Y1 && MsY<Y2):
	        (MsX>=X1 && MsX<=X2 && MsY>=Y1 && MsY<=Y2))
	{
		int MsPos=GetVisualPos(TopPos)+((BoxType!=NO_BOX)?MsY-Y1-1:MsY-Y1);

		MsPos = VisualPosToReal(MsPos);

		if (MsPos>=0 && MsPos<ItemCount && ItemCanHaveFocus(Item[MsPos]->Flags))
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
				ProcessKey(KEY_ENTER);
			}
		}

		return TRUE;
	}
	else if (BoxType!=NO_BOX && (MouseEvent->dwButtonState & 3) && !MouseEvent->dwEventFlags)
	{
		int ClickOpt = (MouseEvent->dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) ? Opt.VMenu.LBtnClick : Opt.VMenu.RBtnClick;
		if (ClickOpt==VMENUCLICK_CANCEL)
			ProcessKey(KEY_ESC);

		return TRUE;
	}
	else if (BoxType!=NO_BOX && !(MouseEvent->dwButtonState&FROM_LEFT_1ST_BUTTON_PRESSED) && (IntKeyState.PrevMouseButtonState&FROM_LEFT_1ST_BUTTON_PRESSED) && !MouseEvent->dwEventFlags && (Opt.VMenu.LBtnClick==VMENUCLICK_APPLY))
	{
		ProcessKey(KEY_ENTER);

		return TRUE;
	}
	else if (BoxType!=NO_BOX && !(MouseEvent->dwButtonState&FROM_LEFT_2ND_BUTTON_PRESSED) && (IntKeyState.PrevMouseButtonState&FROM_LEFT_2ND_BUTTON_PRESSED) && !MouseEvent->dwEventFlags && (Opt.VMenu.MBtnClick==VMENUCLICK_APPLY))
	{
		ProcessKey(KEY_ENTER);

		return TRUE;
	}
	else if (BoxType!=NO_BOX && bRightBtnPressed && !(MouseEvent->dwButtonState&RIGHTMOST_BUTTON_PRESSED) && (IntKeyState.PrevMouseButtonState&RIGHTMOST_BUTTON_PRESSED) && !MouseEvent->dwEventFlags && (Opt.VMenu.RBtnClick==VMENUCLICK_APPLY))
	{
		ProcessKey(KEY_ENTER);

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

	if (Pos >= ItemCount)
		return GetShowItemCount();

	int v=0;

	for (int i=0; i < Pos; i++)
	{
		if (ItemIsVisible(Item[i]->Flags))
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
		return ItemCount;

	for (int i=0; i < ItemCount; i++)
	{
		if (ItemIsVisible(Item[i]->Flags))
		{
			if (!VPos--)
				return i;
		}
	}

	return -1;
}

bool VMenu::ShiftItemShowPos(int Pos, int Direct)
{
	int _len;
	int ItemShowPos = Item[Pos]->ShowPos;

	if (VMFlags.Check(VMENU_SHOWAMPERSAND))
		_len = (int)Item[Pos]->strName.GetLength();
	else
		_len = HiStrlen(Item[Pos]->strName);

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
		ItemShowPos = HiFindNextVisualPos(Item[Pos]->strName,ItemShowPos,Direct);
	}

	if (ItemShowPos < 0)
		ItemShowPos = 0;

	if (ItemShowPos + MaxLineWidth > _len)
		ItemShowPos = _len - MaxLineWidth;

	if (ItemShowPos != Item[Pos]->ShowPos)
	{
		Item[Pos]->ShowPos = ItemShowPos;
		VMFlags.Set(VMENU_UPDATEREQUIRED);
		return true;
	}

	return false;
}

void VMenu::Show()
{
	CriticalSectionLock Lock(CS);

	if (CheckFlags(VMENU_LISTBOX))
	{
		if (CheckFlags(VMENU_SHOWNOBOX))
			BoxType = NO_BOX;
		else if (CheckFlags(VMENU_LISTHASFOCUS))
			BoxType = SHORT_DOUBLE_BOX;
		else
			BoxType = SHORT_SINGLE_BOX;
	}

	if (!CheckFlags(VMENU_LISTBOX))
	{
		bool AutoCenter = false;
		bool AutoHeight = false;

		if (!CheckFlags(VMENU_COMBOBOX))
		{
			bool HasSubMenus = ItemSubMenusCount > 0;

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
			ScreenObject::Show();
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
	CriticalSectionLock Lock(CS);
	ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);

	if (!CheckFlags(VMENU_LISTBOX) && SaveScr)
	{
		delete SaveScr;
		SaveScr = nullptr;
		ScreenObject::Hide();
	}

	SetFlags(VMENU_UPDATEREQUIRED);

	if (OldTitle)
	{
		delete OldTitle;
		OldTitle = nullptr;
	}
}

void VMenu::DisplayObject()
{
	CriticalSectionLock Lock(CS);
	ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);

	ClearFlags(VMENU_UPDATEREQUIRED);
	//Modal::ExitCode = -1; // Mantis#0002041 (build 2520)

	if (CheckFlags(VMENU_REFILTERREQUIRED)!=0)
	{
		if (bFilterEnabled)
		{
			RestoreFilteredItems();
			if (!strFilter.IsEmpty())
				FilterStringUpdated(true);
		}
		ClearFlags(VMENU_REFILTERREQUIRED);
	}

	SetCursorType(0,10);

	if (!CheckFlags(VMENU_LISTBOX) && !SaveScr)
	{
		if (!CheckFlags(VMENU_DISABLEDRAWBACKGROUND) && !(BoxType==SHORT_DOUBLE_BOX || BoxType==SHORT_SINGLE_BOX))
			SaveScr = new SaveScreen(X1-2,Y1-1,X2+4,Y2+2);
		else
			SaveScr = new SaveScreen(X1,Y1,X2+2,Y2+1);
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
	CriticalSectionLock Lock(CS);

	int MaxTitleLength = X2-X1-2;
	int WidthTitle;

	if (!strTitle.IsEmpty() || bFilterEnabled)
	{
		string strDisplayTitle = strTitle;

		if (bFilterEnabled)
		{
			if (bFilterLocked)
				strDisplayTitle += L" ";
			else
				strDisplayTitle.Clear();

			strDisplayTitle += bFilterLocked?L"<":L"[";
			strDisplayTitle += strFilter;
			strDisplayTitle += bFilterLocked?L">":L"]";
		}

		WidthTitle=(int)strDisplayTitle.GetLength();

		if (WidthTitle > MaxTitleLength)
			WidthTitle = MaxTitleLength - 1;

		GotoXY(X1+(X2-X1-1-WidthTitle)/2,Y1);
		SetColor(Colors[VMenuColorTitle]);

		FS << L" " << fmt::ExactWidth(WidthTitle) << strDisplayTitle << L" ";
	}

	if (!strBottomTitle.IsEmpty())
	{
		WidthTitle=(int)strBottomTitle.GetLength();

		if (WidthTitle > MaxTitleLength)
			WidthTitle = MaxTitleLength - 1;

		GotoXY(X1+(X2-X1-1-WidthTitle)/2,Y2);
		SetColor(Colors[VMenuColorTitle]);

		FS << L" " << fmt::ExactWidth(WidthTitle) << strBottomTitle << L" ";
	}
}

void VMenu::ShowMenu(bool IsParent)
{
	CriticalSectionLock Lock(CS);
	ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);

	int MaxItemLength = 0;
	bool HasRightScroll = false;
	bool HasSubMenus = ItemSubMenusCount > 0;

	//BUGBUG, this must be optimized
	for (int i = 0; i < ItemCount; i++)
	{
		int ItemLen;

		if (CheckFlags(VMENU_SHOWAMPERSAND))
			ItemLen = static_cast<int>(Item[i]->strName.GetLength());
		else
			ItemLen = HiStrlen(Item[i]->strName);

		if (ItemLen > MaxItemLength)
			MaxItemLength = ItemLen;
	}

	MaxLineWidth = X2 - X1 + 1;

	if (BoxType != NO_BOX)
		MaxLineWidth -= 2; // frame

	MaxLineWidth -= 2; // check mark + left horz. scroll

	if (!CheckFlags(VMENU_COMBOBOX|VMENU_LISTBOX) && HasSubMenus)
		MaxLineWidth -= 2; // sub menu arrow

	if ((CheckFlags(VMENU_LISTBOX|VMENU_ALWAYSSCROLLBAR) || Opt.ShowMenuScrollbar) && BoxType==NO_BOX && ScrollBarRequired(Y2-Y1+1, GetShowItemCount()))
		MaxLineWidth -= 1; // scrollbar

	if (MaxItemLength > MaxLineWidth)
	{
		HasRightScroll = true;
		MaxLineWidth -= 1; // right horz. scroll
	}

	if (X2<=X1 || Y2<=Y1)
	{
		if (!(CheckFlags(VMENU_SHOWNOBOX) && Y2==Y1))
			return;
	}

	if (CheckFlags(VMENU_LISTBOX))
	{
		if (CheckFlags(VMENU_SHOWNOBOX))
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

	VisualTopPos = Min(VisualTopPos, GetShowItemCount() - (Y2-Y1-1-((BoxType==NO_BOX)?2:0)));

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

	string strTmpStr;

	for (int Y=Y1+((BoxType!=NO_BOX)?1:0), I=TopPos; Y<((BoxType!=NO_BOX)?Y2:Y2+1); Y++, I++)
	{
		GotoXY(X1,Y);

		if (I < ItemCount)
		{
			if (!ItemIsVisible(Item[I]->Flags))
			{
				Y--;
				continue;
			}

			if (Item[I]->Flags&LIF_SEPARATOR)
			{
				int SepWidth = X2-X1+1;
				wchar_t *TmpStr = strTmpStr.GetBuffer(SepWidth+1);
				wchar_t *Ptr = TmpStr+1;

				MakeSeparator(SepWidth,TmpStr,BoxType==NO_BOX?0:(BoxType==SINGLE_BOX||BoxType==SHORT_SINGLE_BOX?2:1));

				if (!CheckFlags(VMENU_NOMERGEBORDER) && I>0 && I<ItemCount-1 && SepWidth>3)
				{
					for (unsigned int J=0; Ptr[J+3]; J++)
					{
						wchar_t PrevItem = (Item[I-1]->strName.GetLength()>=J) ? Item[I-1]->strName.At(J) : 0;
						wchar_t NextItem = (Item[I+1]->strName.GetLength()>=J) ? Item[I+1]->strName.At(J) : 0;

						if (!PrevItem && !NextItem)
							break;

						if (PrevItem==BoxSymbols[BS_V1])
						{
							int Correction = 0;

							if (!CheckFlags(VMENU_SHOWAMPERSAND) && wmemchr(Item[I-1]->strName,L'&',J))
								Correction = 1;

							if (NextItem==BoxSymbols[BS_V1])
								Ptr[J-Correction+(BoxType==NO_BOX?1:2)] = BoxSymbols[BS_C_H1V1];
							else
								Ptr[J-Correction+(BoxType==NO_BOX?1:2)] = BoxSymbols[BS_B_H1V1];
						}
						else if (NextItem==BoxSymbols[BS_V1])
						{
							int Correction = 0;

							if (!CheckFlags(VMENU_SHOWAMPERSAND) && wmemchr(Item[I+1]->strName,L'&',J))
								Correction = 1;

							Ptr[J-Correction+(BoxType==NO_BOX?1:2)] = BoxSymbols[BS_T_H1V1];
						}
					}
				}

				SetColor(Colors[VMenuColorSeparator]);
				BoxText(TmpStr,FALSE);

				if (!Item[I]->strName.IsEmpty())
				{
					int ItemWidth = (int)Item[I]->strName.GetLength();

					if (ItemWidth > X2-X1-3)
						ItemWidth = X2-X1-3;

					GotoXY(X1+(X2-X1-1-ItemWidth)/2,Y);
					FS << L" " << fmt::LeftAlign() << fmt::ExactWidth(ItemWidth) << Item[I]->strName << L" ";
				}

				strTmpStr.ReleaseBuffer();
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

				if ((Item[I]->Flags&LIF_SELECTED))
					SetColor(VMenu::Colors[Item[I]->Flags&LIF_GRAYED?VMenuColorSelGrayed:VMenuColorSelected]);
				else
					SetColor(VMenu::Colors[Item[I]->Flags&LIF_DISABLE?VMenuColorDisabled:(Item[I]->Flags&LIF_GRAYED?VMenuColorGrayed:VMenuColorText)]);

				string strMenuLine;
				wchar_t CheckMark = L' ';

				if (Item[I]->Flags & LIF_CHECKED)
				{
					if (!(Item[I]->Flags & 0x0000FFFF))
						CheckMark = 0x221A;
					else
						CheckMark = static_cast<wchar_t>(Item[I]->Flags & 0x0000FFFF);
				}

				strMenuLine.Append(CheckMark);
				strMenuLine.Append(L' '); // left scroller (<<) placeholder
				int ShowPos = HiFindRealPos(Item[I]->strName, Item[I]->ShowPos, CheckFlags(VMENU_SHOWAMPERSAND));
				string strMItemPtr(Item[I]->strName.CPtr() + ShowPos);
				int strMItemPtrLen;

				if (CheckFlags(VMENU_SHOWAMPERSAND))
					strMItemPtrLen = static_cast<int>(strMItemPtr.GetLength());
				else
					strMItemPtrLen = HiStrlen(strMItemPtr);

				// fit menu string into available space
				if (strMItemPtrLen > MaxLineWidth)
					strMItemPtr.SetLength(HiFindRealPos(strMItemPtr, MaxLineWidth, CheckFlags(VMENU_SHOWAMPERSAND)));

				// set highlight
				if (!VMFlags.Check(VMENU_SHOWAMPERSAND))
				{
					int AmpPos = Item[I]->AmpPos - ShowPos;

					if ((AmpPos >= 0) && (static_cast<size_t>(AmpPos) < strMItemPtr.GetLength()) && (strMItemPtr.At(AmpPos) != L'&'))
					{
						string strEnd = strMItemPtr.CPtr() + AmpPos;
						strMItemPtr.SetLength(AmpPos);
						strMItemPtr += L"&";
						strMItemPtr += strEnd;
					}
				}

				strMenuLine.Append(strMItemPtr);

				{
					// табуляции меняем только при показе!!!
					// для сохранение оригинальной строки!!!
					wchar_t *TabPtr, *TmpStr = strMenuLine.GetBuffer();

					while ((TabPtr = wcschr(TmpStr, L'\t')))
						*TabPtr = L' ';

					strMenuLine.ReleaseBuffer(strMenuLine.GetLength());
				}

				FarColor Col;

				if (!(Item[I]->Flags & LIF_DISABLE))
				{
					if (Item[I]->Flags & LIF_SELECTED)
						Col = Colors[Item[I]->Flags & LIF_GRAYED ? VMenuColorSelGrayed : VMenuColorHSelect];
					else
						Col = Colors[Item[I]->Flags & LIF_GRAYED ? VMenuColorGrayed : VMenuColorHilite];
				}
				else
				{
					Col = Colors[VMenuColorDisabled];
				}

				if (CheckFlags(VMENU_SHOWAMPERSAND))
					Text(strMenuLine);
				else
					HiText(strMenuLine, Col);

				// сделаем добавочку для NO_BOX
				{
					int Width = X2-WhereX()+(BoxType==NO_BOX?1:0);
					if (Width > 0)
						FS << fmt::MinWidth(Width) << L"";
				}

				if (Item[I]->Flags & MIF_SUBMENU)
				{
					GotoXY(X1+(BoxType!=NO_BOX?1:0)+2+MaxLineWidth+(HasRightScroll?1:0)+1,Y);
					BoxText(L'\x25BA'); // sub menu arrow
				}

				SetColor(Colors[(Item[I]->Flags&LIF_DISABLE)?VMenuColorArrowsDisabled:(Item[I]->Flags&LIF_SELECTED?VMenuColorArrowsSelect:VMenuColorArrows)]);

				if (/*BoxType!=NO_BOX && */Item[I]->ShowPos > 0)
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
			FS << fmt::MinWidth(((BoxType!=NO_BOX)?X2-X1-1:X2-X1)+((BoxType==NO_BOX)?1:0)) << L"";
		}
	}

	if (CheckFlags(VMENU_LISTBOX|VMENU_ALWAYSSCROLLBAR) || Opt.ShowMenuScrollbar)
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
	CriticalSectionLock Lock(CS);

	if (CheckSymbol)
		CheckSymbol=Upper(CheckSymbol);

	for (int I=StartPos; I < ItemCount; I++)
	{
		if (!ItemIsVisible(Item[I]->Flags))
			continue;

		wchar_t Ch = GetHighlights(Item[I]);

		if (Ch)
		{
			if (CheckSymbol == Upper(Ch) || CheckSymbol == Upper(KeyToKeyLayout(Ch)))
				return I;
		}
		else if (!CheckSymbol)
		{
			return I;
		}
	}

	return -1;
}

wchar_t VMenu::GetHighlights(const MenuItemEx *_item)
{
	CriticalSectionLock Lock(CS);

	wchar_t Ch = 0;

	if (_item)
	{
		const wchar_t *Name = _item->strName;
		const wchar_t *ChPtr = wcschr(Name,L'&');

		if (ChPtr || _item->AmpPos > -1)
		{
			if (!ChPtr && _item->AmpPos > -1)
			{
				ChPtr = Name + _item->AmpPos;
				Ch = *ChPtr;
			}
			else
			{
				Ch = ChPtr[1];
			}

			if (CheckFlags(VMENU_SHOWAMPERSAND))
			{
				ChPtr = wcschr(ChPtr+1,L'&');

				if (ChPtr)
					Ch = ChPtr[1];
			}
		}
	}

	return Ch;
}

void VMenu::AssignHighlights(int Reverse)
{
	CriticalSectionLock Lock(CS);

	memset(Used,0,WCHAR_MAX);

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
	for (I = Reverse ? ItemCount-1 : 0; I>=0 && I<ItemCount; I+=Delta)
	{
		wchar_t Ch = 0;
		int ShowPos = HiFindRealPos(Item[I]->strName, Item[I]->ShowPos, CheckFlags(VMENU_SHOWAMPERSAND));
		const wchar_t *Name = Item[I]->strName.CPtr() + ShowPos;
		Item[I]->AmpPos = -1;
		// TODO: проверка на LIF_HIDDEN
		const wchar_t *ChPtr = wcschr(Name, L'&');

		if (ChPtr)
		{
			Ch = ChPtr[1];

			if (Ch && VMFlags.Check(VMENU_SHOWAMPERSAND))
			{
				ChPtr = wcschr(ChPtr+1, L'&');

				if (ChPtr)
					Ch=ChPtr[1];
			}
		}

		if (Ch && !Used[Upper(Ch)] && !Used[Lower(Ch)])
		{
			wchar_t ChKey=KeyToKeyLayout(Ch);
			Used[Upper(ChKey)] = true;
			Used[Lower(ChKey)] = true;
			Used[Upper(Ch)] = true;
			Used[Lower(Ch)] = true;
			Item[I]->AmpPos = static_cast<short>(ChPtr-Name)+static_cast<short>(ShowPos);
		}
	}

	// TODO:  ЭТОТ цикл нужно уточнить - возможно вылезут артефакты (хотя не уверен)
	for (I = Reverse ? ItemCount-1 : 0; I>=0 && I<ItemCount; I+=Delta)
	{
		int ShowPos = HiFindRealPos(Item[I]->strName, Item[I]->ShowPos, CheckFlags(VMENU_SHOWAMPERSAND));
		const wchar_t *Name = Item[I]->strName.CPtr() + ShowPos;
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
					Item[I]->AmpPos = J + ShowPos;
					break;
				}
			}
		}
	}

	SetFlags(VMENU_AUTOHIGHLIGHT|(Reverse?VMENU_REVERSEHIGHLIGHT:0));
	ClearFlags(VMENU_SHOWAMPERSAND);
}

bool VMenu::CheckKeyHiOrAcc(DWORD Key, int Type, int Translate)
{
	CriticalSectionLock Lock(CS);

	//не забудем сбросить EndLoop для листбокса, иначе не будут работать хоткеи в активном списке
	if (CheckFlags(VMENU_LISTBOX))
		EndLoop = FALSE;

	for (int I=0; I < ItemCount; I++)
	{
		MenuItemEx *CurItem = Item[I];

		if (ItemCanHaveFocus(CurItem->Flags) && ((!Type && CurItem->AccelKey && Key == CurItem->AccelKey) || (Type && IsKeyHighlighted(CurItem->strName,Key,Translate,CurItem->AmpPos))))
		{
			SetSelectPos(I,1);
			ShowMenu(true);

			if ((!ParentDialog  || CheckFlags(VMENU_COMBOBOX)) && ItemCanBeEntered(Item[SelectPos]->Flags))
			{
				Modal::ExitCode = I;
				EndLoop = TRUE;
			}

			break;
		}
	}

	return EndLoop==TRUE;
}

void VMenu::UpdateMaxLengthFromTitles()
{
	//тайтл + 2 пробела вокруг
	UpdateMaxLength((int)Max(strTitle.GetLength(),strBottomTitle.GetLength())+2);
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
	CriticalSectionLock Lock(CS);

	MaxHeight = NewMaxHeight;

	if (MaxHeight > ScrY-6)
		MaxHeight = ScrY-6;
}

string &VMenu::GetTitle(string &strDest,int,int)
{
	CriticalSectionLock Lock(CS);

	strDest = strTitle;
	return strDest;
}

string &VMenu::GetBottomTitle(string &strDest)
{
	CriticalSectionLock Lock(CS);

	strDest = strBottomTitle;
	return strDest;
}

void VMenu::SetBottomTitle(const wchar_t *BottomTitle)
{
	CriticalSectionLock Lock(CS);

	SetFlags(VMENU_UPDATEREQUIRED);

	if (BottomTitle)
		strBottomTitle = BottomTitle;
	else
		strBottomTitle.Clear();

	UpdateMaxLength((int)strBottomTitle.GetLength() + 2);
}

void VMenu::SetTitle(const wchar_t *Title)
{
	CriticalSectionLock Lock(CS);

	SetFlags(VMENU_UPDATEREQUIRED);

	if (Title)
		strTitle = Title;
	else
		strTitle.Clear();

	UpdateMaxLength((int)strTitle.GetLength() + 2);

	if (CheckFlags(VMENU_CHANGECONSOLETITLE))
	{
		if (!strTitle.IsEmpty())
		{
			if (!OldTitle)
				OldTitle = new ConsoleTitle;

			ConsoleTitle::SetFarTitle(strTitle);
		}
		else
		{
			if (OldTitle)
			{
				delete OldTitle;
				OldTitle = nullptr;
			}
		}
	}
}

void VMenu::ResizeConsole()
{
	CriticalSectionLock Lock(CS);

	if (SaveScr)
	{
		SaveScr->Discard();
		delete SaveScr;
		SaveScr = nullptr;
	}

	if (CheckFlags(VMENU_NOTCHANGE))
	{
		return;
	}

	ObjWidth = ObjHeight = 0;

	if (!CheckFlags(VMENU_NOTCENTER))
	{
		Y2 = X2 = Y1 = X1 = -1;
	}
	else
	{
		X1 = 5;

		if (!CheckFlags(VMENU_LEFTMOST) && ScrX>40)
		{
			X1 = (ScrX+1)/2+5;
		}

		Y1 = (ScrY+1-(GetShowItemCount()+5))/2;

		if (Y1 < 1)
			Y1 = 1;

		X2 = Y2 = 0;
	}
}

void VMenu::SetBoxType(int BoxType)
{
	CriticalSectionLock Lock(CS);

	VMenu::BoxType=BoxType;
}

void VMenu::SetColors(FarDialogItemColors *ColorsIn)
{
	CriticalSectionLock Lock(CS);

	if (ColorsIn)
	{
		memmove(Colors,ColorsIn->Colors,sizeof(Colors));
	}
	else
	{
		static PaletteColors StdColor[2][3][VMENU_COLOR_COUNT]=
		{
			// Not VMENU_WARNDIALOG
			{
				{ // VMENU_LISTBOX
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
				{ // VMENU_COMBOBOX
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
				{ // VMenu
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

			// == VMENU_WARNDIALOG
			{
				{ // VMENU_LISTBOX
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
				{ // VMENU_COMBOBOX
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
				{ // VMenu
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
			for (int I=0; I < VMENU_COLOR_COUNT; ++I)
				Colors[I] = ColorIndexToColor(StdColor[StyleMenu][TypeMenu][I]);
		}
	}
}

void VMenu::GetColors(FarDialogItemColors *ColorsOut)
{
	CriticalSectionLock Lock(CS);

	memmove(ColorsOut->Colors, Colors, Min(sizeof(Colors), ColorsOut->ColorsCount*sizeof(Colors[0])));
}

void VMenu::SetOneColor(int Index, PaletteColors Color)
{
	CriticalSectionLock Lock(CS);

	if (Index < (int)ARRAYSIZE(Colors))
		Colors[Index] = ColorIndexToColor(Color);
}

BOOL VMenu::GetVMenuInfo(FarListInfo* Info)
{
	CriticalSectionLock Lock(CS);

	if (Info)
	{
		Info->Flags = GetFlags() & (LINFO_SHOWNOBOX|LINFO_AUTOHIGHLIGHT|LINFO_REVERSEHIGHLIGHT|LINFO_WRAPMODE|LINFO_SHOWAMPERSAND);
		Info->ItemsNumber = ItemCount;
		Info->SelectPos = SelectPos;
		Info->TopPos = TopPos;
		Info->MaxHeight = MaxHeight;
		Info->MaxLength = MaxLength;
		ClearArray(Info->Reserved);
		return TRUE;
	}

	return FALSE;
}

// функция обработки меню (по умолчанию)
intptr_t WINAPI VMenu::DefMenuProc(HANDLE hVMenu, int Msg, int Param1, void* Param2)
{
	return 0;
}

// функция посылки сообщений меню
intptr_t WINAPI VMenu::SendMenuMessage(HANDLE hVMenu, int Msg, int Param1, void* Param2)
{
	CriticalSectionLock Lock(((VMenu*)hVMenu)->CS);

	if (hVMenu)
		return ((VMenu*)hVMenu)->VMenuProc(hVMenu,Msg,Param1,Param2);

	return 0;
}

// Функция GetItemPtr - получить указатель на нужный Item.
MenuItemEx *VMenu::GetItemPtr(int Position)
{
	CriticalSectionLock Lock(CS);

	int ItemPos = GetItemPosition(Position);

	if (ItemPos < 0)
		return nullptr;

	return Item[ItemPos];
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
	CriticalSectionLock Lock(CS);

	int ItemPos = GetItemPosition(Position);

	if (ItemPos < 0)
		return 0;

	return Item[ItemPos]->UserDataSize;
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
			PItem->UserDataSize = (StrLength(static_cast<const wchar_t *>(Data))+1)*sizeof(wchar_t);
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
	CriticalSectionLock Lock(CS);

	int ItemPos = GetItemPosition(Position);

	if (ItemPos < 0)
		return 0;

	return _SetUserData(Item[ItemPos], Data, Size);
}

// Получить данные
void* VMenu::GetUserData(void *Data,size_t Size,int Position)
{
	CriticalSectionLock Lock(CS);

	int ItemPos = GetItemPosition(Position);

	if (ItemPos < 0)
		return nullptr;

	return _GetUserData(Item[ItemPos], Data, Size);
}

FarListItem *VMenu::MenuItem2FarList(const MenuItemEx *MItem, FarListItem *FItem)
{
	if (FItem && MItem)
	{
		ClearStruct(*FItem);
		FItem->Flags = MItem->Flags;
		FItem->Text = MItem->strName;
		return FItem;
	}

	return nullptr;
}

MenuItemEx *VMenu::FarList2MenuItem(const FarListItem *FItem, MenuItemEx *MItem)
{
	if (FItem && MItem)
	{
		MItem->Clear();
		MItem->Flags = FItem->Flags;
		MItem->strName = FItem->Text;
		return MItem;
	}

	return nullptr;
}

int VMenu::GetTypeAndName(string &strType, string &strName)
{
	CriticalSectionLock Lock(CS);

	strType = MSG(MVMenuType);
	strName = strTitle;
	return CheckFlags(VMENU_COMBOBOX) ? MODALTYPE_COMBOBOX : MODALTYPE_VMENU;
}

// return Pos || -1
int VMenu::FindItem(const FarListFind *FItem)
{
	return FindItem(FItem->StartIndex,FItem->Pattern,FItem->Flags);
}

int VMenu::FindItem(int StartIndex,const wchar_t *Pattern,UINT64 Flags)
{
	CriticalSectionLock Lock(CS);

	if ((DWORD)StartIndex < (DWORD)ItemCount)
	{
		int LenPattern=StrLength(Pattern);

		for (int I=StartIndex; I < ItemCount; I++)
		{
			string strTmpBuf(Item[I]->strName);
			int LenNamePtr = (int)strTmpBuf.GetLength();
			RemoveChar(strTmpBuf, L'&');

			if (Flags&LIFIND_EXACTMATCH)
			{
				if (!StrCmpNI(strTmpBuf,Pattern,Max(LenPattern,LenNamePtr)))
					return I;
			}
			else
			{
				if (CmpName(Pattern,strTmpBuf,true))
					return I;
			}
		}
	}

	return -1;
}

static int WINAPI SortItem(const MenuItemEx **el1, const MenuItemEx **el2, const SortItemParam *Param)
{
	string strName1((*el1)->strName);
	string strName2((*el2)->strName);
	RemoveChar(strName1,L'&',true);
	RemoveChar(strName2,L'&',true);
	int Res = StrCmpI(strName1.CPtr()+Param->Offset,strName2.CPtr()+Param->Offset);
	return (Param->Direction?(Res<0?1:(Res>0?-1:0)):Res);
}

static int WINAPI SortItemDataDWORD(const MenuItemEx **el1, const MenuItemEx **el2, const SortItemParam *Param)
{
	int Res;
	DWORD Dw1=(DWORD)(intptr_t)((*el1)->UserData);
	DWORD Dw2=(DWORD)(intptr_t)((*el2)->UserData);

	if (Dw1 == Dw2)
		Res=0;
	else if (Dw1 > Dw2)
		Res=1;
	else
		Res=-1;

	return (Param->Direction?(Res<0?1:(Res>0?-1:0)):Res);
}

// Сортировка элементов списка
// Offset - начало сравнения! по умолчанию =0
void VMenu::SortItems(int Direction, int Offset, BOOL SortForDataDWORD)
{
	CriticalSectionLock Lock(CS);

	typedef int (WINAPI *qsortex_fn)(const void*,const void*,void*);

	SortItemParam Param;
	Param.Direction=Direction;
	Param.Offset=Offset;

	if (!SortForDataDWORD) // обычная сортировка
	{
		qsortex((char *)Item,
				ItemCount,
				sizeof(*Item),
				(qsortex_fn)SortItem,
				&Param);
	}
	else
	{
		qsortex((char *)Item,
				ItemCount,
				sizeof(*Item),
				(qsortex_fn)SortItemDataDWORD,
				&Param);
	}

	// скорректируем SelectPos
	UpdateSelectPos();

	SetFlags(VMENU_UPDATEREQUIRED);
}

void VMenu::SortItems(TMENUITEMEXCMPFUNC user_cmp_func,int Direction,int Offset)
{
	CriticalSectionLock Lock(CS);

	typedef int (WINAPI *qsortex_fn)(const void*,const void*,void*);

	SortItemParam Param;
	Param.Direction=Direction;
	Param.Offset=Offset;

	qsortex((char *)Item,
			ItemCount,
			sizeof(*Item),
			(qsortex_fn)user_cmp_func,
			&Param);

	// скорректируем SelectPos
	UpdateSelectPos();

	SetFlags(VMENU_UPDATEREQUIRED);
}

bool VMenu::Pack()
{
	int OldItemCount=ItemCount;
	int FirstIndex=0;

	while (FirstIndex<ItemCount)
	{
		int LastIndex=ItemCount-1;
		while (LastIndex>FirstIndex)
		{
			if (!(Item[FirstIndex]->Flags & LIF_SEPARATOR) && !(Item[LastIndex]->Flags & LIF_SEPARATOR))
			{
				if (StrCmp(Item[FirstIndex]->strName,Item[LastIndex]->strName)==0)
				{
					DeleteItem(LastIndex);
				}
			}
			LastIndex--;
		}
		FirstIndex++;
	}
	return (OldItemCount!=ItemCount);
}

void VMenu::SetId(const GUID& Id)
{
	MenuId=Id;
}

const GUID& VMenu::Id(void)
{
	return MenuId;
}
