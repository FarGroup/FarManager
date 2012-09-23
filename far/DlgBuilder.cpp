/*
DlgBuilder.cpp

Динамическое построение диалогов
*/
/*
Copyright © 2009 Far Group
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

#include "plugin.hpp"
#include "language.hpp"
#include "FarDlgBuilder.hpp"
#include "dialog.hpp"

const int DEFAULT_INDENT = 5;

template<class T>
struct EditFieldBinding: public DialogItemBinding<DialogItemEx>
{
	T& TextValue;

	EditFieldBinding(T& aTextValue)
		: TextValue(aTextValue)
	{
	}

	virtual void SaveValue(DialogItemEx *Item, int RadioGroupIndex)
	{
		TextValue = Item->strData;
	}
};

template<class T>
struct EditFieldIntBinding: public DialogItemBinding<DialogItemEx>
{
	T *IntValue;
	wchar_t Mask[32];

	EditFieldIntBinding(T *aIntValue, int Width)
		: IntValue(aIntValue)
	{
		int MaskWidth = Width < 31 ? Width : 31;
		for(int i=1; i<MaskWidth; i++)
			Mask[i] = L'9';
		Mask[0] = L'#';
		Mask[MaskWidth] = L'\0';
	}

	virtual void SaveValue(DialogItemEx *Item, int RadioGroupIndex)
	{
		wchar_t *endptr;
		*IntValue = wcstoul(Item->strData, &endptr, 10);
	}

	const wchar_t *GetMask()
	{
		return Mask;
	}
};

template<class T>
struct EditFieldHexBinding: public DialogItemBinding<DialogItemEx>
{
	T *IntValue;
	wchar_t Mask[1+16+1];

	EditFieldHexBinding(T *aIntValue, int Width) : IntValue(aIntValue)
	{
		int MaskWidth = Min(static_cast<int>(ARRAYSIZE(Mask)-1), Width);
		for(int i=0; i<MaskWidth; i++)
			Mask[i] = L'H';
		Mask[MaskWidth] = L'\0';
		Mask[0] = L'x';
	}

	virtual void SaveValue(DialogItemEx *Item, int RadioGroupIndex)
	{
		wchar_t *endptr;
		*IntValue = wcstoul(Item->strData.CPtr()+1, &endptr, 16);
	}

	const wchar_t *GetMask()
	{
		return Mask;
	}
};

template<class T>
struct FarCheckBoxIntBinding: public DialogItemBinding<DialogItemEx>
{
private:
	T& Value;
	int Mask;

public:
	FarCheckBoxIntBinding(T& aValue, int aMask=0) : Value(aValue), Mask(aMask) { }

	virtual void SaveValue(DialogItemEx *Item, int RadioGroupIndex)
	{
		if (!Mask)
		{
			Value = Item->Selected;
		}
		else
		{
			if (Item->Selected)
				Value |= Mask;
			else
				Value &= ~Mask;
		}
	}
};

template<class T>
struct FarCheckBoxBool3Binding: public DialogItemBinding<DialogItemEx>
{
private:
	T& Value;

public:
	FarCheckBoxBool3Binding(T& aValue) : Value(aValue) { }

	virtual void SaveValue(DialogItemEx *Item, int RadioGroupIndex)
	{
		Value = Item->Selected;
	}
};

template<class T>
struct FarCheckBoxBoolBinding: public DialogItemBinding<DialogItemEx>
{
private:
	T& Value;

public:
	FarCheckBoxBoolBinding(T& aValue) : Value(aValue) { }

	virtual void SaveValue(DialogItemEx *Item, int RadioGroupIndex)
	{
		Value = Item->Selected != BSTATE_UNCHECKED;
	}
};

template<class T>
struct FarComboBoxBinding: public DialogItemBinding<DialogItemEx>
{
private:
	T& Value;
	FarList *List;

public:
	FarComboBoxBinding(T& aValue, FarList *aList)
		: Value(aValue), List(aList)
	{
	}

	virtual ~FarComboBoxBinding()
	{
		delete [] List->Items;
		delete List;
	}

	virtual void SaveValue(DialogItemEx *Item, int RadioGroupIndex)
	{
		FarListItem &ListItem = List->Items[Item->ListPos];
		Value = ListItem.Reserved[0];
	}
};

template<class T>
struct FarRadioButtonBinding: public DialogItemBinding<DialogItemEx>
{
private:
	T& Value;

public:
	FarRadioButtonBinding(T& aValue) : Value(aValue) { }

	virtual void SaveValue(DialogItemEx *Item, int RadioGroupIndex)
	{
		if (Item->Selected)
			Value = RadioGroupIndex;
	}
};

/*
static bool IsEditField(DialogItemEx *Item)
{
	return Item->Type == DI_EDIT || Item->Type == DI_FIXEDIT || Item->Type == DI_PSWEDIT;
}
*/

DialogBuilder::DialogBuilder(LNGID TitleMessageId, const wchar_t *HelpTopic):
	HelpTopic(HelpTopic)
{
	AddBorder(GetLangString(TitleMessageId));
}

DialogBuilder::DialogBuilder():
HelpTopic(nullptr)
{
	AddBorder(nullptr);
}

DialogBuilder::~DialogBuilder()
{
}

void DialogBuilder::InitDialogItem(DialogItemEx *Item, const wchar_t *Text)
{
	Item->Clear();
	Item->ID = DialogItemsCount-1;
	Item->strData = Text;
}

int DialogBuilder::TextWidth(const DialogItemEx &Item)
{
	return static_cast<int>(Item.strData.GetLength());
}

const wchar_t *DialogBuilder::GetLangString(int MessageID)
{
	return MSG(static_cast<LNGID>(MessageID));
}

DialogItemBinding<DialogItemEx> *DialogBuilder::CreateCheckBoxBinding(BOOL* Value, int Mask)
{
	return new CheckBoxBinding<DialogItemEx>(Value, Mask);
}

DialogItemBinding<DialogItemEx> *DialogBuilder::CreateCheckBoxBinding(IntOption& Value, int Mask)
{
	return new FarCheckBoxIntBinding<IntOption>(Value, Mask);
}

DialogItemBinding<DialogItemEx> *DialogBuilder::CreateCheckBoxBinding(Bool3Option& Value)
{
	return new FarCheckBoxBool3Binding<Bool3Option>(Value);
}

DialogItemBinding<DialogItemEx> *DialogBuilder::CreateCheckBoxBinding(BoolOption& Value)
{
	return new FarCheckBoxBoolBinding<BoolOption>(Value);
}

DialogItemBinding<DialogItemEx> *DialogBuilder::CreateRadioButtonBinding(int *Value)
{
	return new RadioButtonBinding<DialogItemEx>(Value);
}

DialogItemBinding<DialogItemEx> *DialogBuilder::CreateRadioButtonBinding(IntOption& Value)
{
	return new FarRadioButtonBinding<IntOption>(Value);
}

DialogItemEx *DialogBuilder::AddEditField(string *Value, int Width, const wchar_t *HistoryID, FARDIALOGITEMFLAGS Flags)
{
	DialogItemEx *Item = AddDialogItem(DI_EDIT, *Value);
	SetNextY(Item);
	Item->X2 = Item->X1 + Width;
	if (HistoryID)
	{
		Item->strHistory = HistoryID;
		Item->Flags |= DIF_HISTORY;
	}
	Item->Flags |= Flags;

	SetLastItemBinding(new EditFieldBinding<decltype(*Value)>(*Value));
	return Item;
}

DialogItemEx *DialogBuilder::AddEditField(StringOption& Value, int Width, const wchar_t *HistoryID, FARDIALOGITEMFLAGS Flags)
{
	DialogItemEx *Item = AddDialogItem(DI_EDIT, Value);
	SetNextY(Item);
	Item->X2 = Item->X1 + Width;
	if (HistoryID)
	{
		Item->strHistory = HistoryID;
		Item->Flags |= DIF_HISTORY;
	}
	Item->Flags |= Flags;

	SetLastItemBinding(new EditFieldBinding<decltype(Value)>(Value));
	return Item;
}

DialogItemEx *DialogBuilder::AddFixEditField(string *Value, int Width, const wchar_t *Mask)
{
	DialogItemEx *Item = AddDialogItem(DI_FIXEDIT, *Value);
	SetNextY(Item);
	Item->X2 = Item->X1 + Width - 1;
	if (Mask)
	{
		Item->Mask = Mask;
		Item->Flags |= DIF_MASKEDIT;
	}

	SetLastItemBinding(new EditFieldBinding<decltype(*Value)>(*Value));
	return Item;
}

DialogItemEx *DialogBuilder::AddFixEditField(StringOption& Value, int Width, const wchar_t *Mask)
{
	DialogItemEx *Item = AddDialogItem(DI_FIXEDIT, Value);
	SetNextY(Item);
	Item->X2 = Item->X1 + Width - 1;
	if (Mask)
	{
		Item->Mask = Mask;
		Item->Flags |= DIF_MASKEDIT;
	}

	SetLastItemBinding(new EditFieldBinding<decltype(Value)>(Value));
	return Item;
}

DialogItemEx *DialogBuilder::AddConstEditField(const wchar_t* Value, int Width, FARDIALOGITEMFLAGS Flags)
{
	DialogItemEx *Item = AddDialogItem(DI_EDIT, Value);
	SetNextY(Item);
	Item->X2 = Item->X1 + Width;
	Item->Flags |= Flags|DIF_READONLY;
	return Item;
}

DialogItemEx *DialogBuilder::AddIntEditField(int *Value, int Width)
{
	DialogItemEx *Item = AddDialogItem(DI_FIXEDIT, L"");
	Item->strData = FormatString() << *Value;
	SetNextY(Item);
	Item->X2 = Item->X1 + Width - 1;

	auto *Binding = new EditFieldIntBinding<int>(Value, Width);
	SetLastItemBinding(Binding);
	Item->Flags |= DIF_MASKEDIT;
	Item->strMask = Binding->GetMask();
	return Item;
}

DialogItemEx *DialogBuilder::AddIntEditField(IntOption& Value, int Width)
{
	DialogItemEx *Item = AddDialogItem(DI_FIXEDIT, L"");
	Item->strData = FormatString() << Value.Get();
	SetNextY(Item);
	Item->X2 = Item->X1 + Width - 1;

	auto *Binding = new EditFieldIntBinding<IntOption>(&Value, Width);
	SetLastItemBinding(Binding);
	Item->Flags |= DIF_MASKEDIT;
	Item->strMask = Binding->GetMask();
	return Item;
}

DialogItemEx *DialogBuilder::AddHexEditField(IntOption& Value, int Width)
{
	DialogItemEx *Item = AddDialogItem(DI_FIXEDIT, L"");
	Item->strData = FormatString() << L'x' << fmt::Radix(16) << fmt::ExactWidth(8) << fmt::FillChar(L'0') << Value.Get();
	SetNextY(Item);
	Item->X2 = Item->X1 + Width - 1;

	auto *Binding = new EditFieldHexBinding<IntOption>(&Value, Width);
	SetLastItemBinding(Binding);
	Item->Flags |= DIF_MASKEDIT;
	Item->strMask = Binding->GetMask();
	return Item;
}

DialogItemEx *DialogBuilder::AddComboBox(int *Value, int Width,
										 DialogBuilderListItem *Items, size_t ItemCount,
										 FARDIALOGITEMFLAGS Flags)
{
	DialogItemEx *Item = AddDialogItem(DI_COMBOBOX, L"");
	SetNextY(Item);
	Item->X2 = Item->X1 + Width;
	Item->Flags |= Flags;

	FarListItem *ListItems = new FarListItem[ItemCount];
	for(size_t i=0; i<ItemCount; i++)
	{
		ListItems [i].Text = MSG(static_cast<LNGID>(Items[i].MessageId));
		ListItems [i].Flags = (*Value == Items [i].ItemValue) ? LIF_SELECTED : 0;
		ListItems [i].Reserved [0] = Items [i].ItemValue;
	}
	FarList *List = new FarList;
	List->StructSize = sizeof(FarList);
	List->Items = ListItems;
	List->ItemsNumber = ItemCount;
	Item->ListItems = List;

	SetLastItemBinding(new ComboBoxBinding<DialogItemEx>(Value, List));
	return Item;
}

DialogItemEx *DialogBuilder::AddComboBox(IntOption& Value, int Width,
	DialogBuilderListItem *Items, size_t ItemCount,
	FARDIALOGITEMFLAGS Flags)
{
	DialogItemEx *Item = AddDialogItem(DI_COMBOBOX, L"");
	SetNextY(Item);
	Item->X2 = Item->X1 + Width;
	Item->Flags |= Flags;

	FarListItem *ListItems = new FarListItem[ItemCount];
	for(size_t i=0; i<ItemCount; i++)
	{
		ListItems [i].Text = MSG(static_cast<LNGID>(Items[i].MessageId));
		ListItems [i].Flags = (Value == Items [i].ItemValue) ? LIF_SELECTED : 0;
		ListItems [i].Reserved [0] = Items [i].ItemValue;
	}
	FarList *List = new FarList;
	List->StructSize = sizeof(FarList);
	List->Items = ListItems;
	List->ItemsNumber = ItemCount;
	Item->ListItems = List;

	SetLastItemBinding(new FarComboBoxBinding<IntOption>(Value, List));
	return Item;
}

DialogItemEx *DialogBuilder::AddCheckbox(int TextMessageId, Bool3Option& Value)
{
	DialogItemEx *Item = AddDialogItem(DI_CHECKBOX, GetLangString(TextMessageId));
	Item->Flags |= DIF_3STATE;
	SetNextY(Item);
	Item->X2 = Item->X1 + ItemWidth(*Item);
	Item->Selected = Value;
	SetLastItemBinding(CreateCheckBoxBinding(Value));
	return Item;
}

DialogItemEx *DialogBuilder::AddCheckbox(int TextMessageId, IntOption& Value, int Mask, bool ThreeState)
{
	DialogItemEx *Item = AddDialogItem(DI_CHECKBOX, GetLangString(TextMessageId));
	if (ThreeState && !Mask)
		Item->Flags |= DIF_3STATE;
	SetNextY(Item);
	Item->X2 = Item->X1 + ItemWidth(*Item);
	if (!Mask)
		Item->Selected = Value;
	else
		Item->Selected = (Value & Mask) ? TRUE : FALSE ;
	SetLastItemBinding(CreateCheckBoxBinding(Value, Mask));
	return Item;
}

DialogItemEx *DialogBuilder::AddCheckbox(const wchar_t* Caption, BoolOption& Value)
{
	DialogItemEx *Item = AddDialogItem(DI_CHECKBOX, Caption);
	SetNextY(Item);
	Item->X2 = Item->X1 + ItemWidth(*Item);
	Item->Selected = Value;
	SetLastItemBinding(CreateCheckBoxBinding(Value));
	return Item;
}

DialogItemEx *DialogBuilder::AddCheckbox(int TextMessageId, BoolOption& Value)
{
	return AddCheckbox(GetLangString(TextMessageId), Value);
}

void DialogBuilder::AddRadioButtons(IntOption& Value, int OptionCount, const int MessageIDs[], bool FocusOnSelected)
{
	for(int i=0; i<OptionCount; i++)
	{
		DialogItemEx *Item = AddDialogItem(DI_RADIOBUTTON, GetLangString(MessageIDs[i]));
		SetNextY(Item);
		Item->X2 = Item->X1 + ItemWidth(*Item);
		if (!i)
			Item->Flags |= DIF_GROUP;
		if (Value == i)
		{
			Item->Selected = TRUE;
			if (FocusOnSelected)
				Item->Flags |= DIF_FOCUS;
		}
		SetLastItemBinding(CreateRadioButtonBinding(Value));
	}
}

void DialogBuilder::LinkFlags(DialogItemEx *Parent, DialogItemEx *Target, FARDIALOGITEMFLAGS Flags, bool LinkLabels)
{
	Parent->Flags |= DIF_AUTOMATION;
	Parent->AddAutomation(Target->ID, Flags, DIF_NONE, DIF_NONE, Flags, DIF_NONE, DIF_NONE);
	if (!Parent->Selected)
		Target->Flags |= Flags;

	if (LinkLabels)
	{
		DialogItemBinding<DialogItemEx> *Binding = FindBinding(Target);
		if (Binding)
		{
			LinkFlagsByID(Parent, Binding->BeforeLabelID, Flags);
			LinkFlagsByID(Parent, Binding->AfterLabelID, Flags);
		}
	}
}

void DialogBuilder::LinkFlagsByID(DialogItemEx *Parent, int TargetID, FARDIALOGITEMFLAGS Flags)
{
	if (TargetID >= 0)
	{
		Parent->AddAutomation(TargetID, Flags, DIF_NONE, DIF_NONE, Flags, DIF_NONE, DIF_NONE);
		if (!Parent->Selected)
			DialogItems [TargetID].Flags |= Flags;
	}
}

intptr_t DialogBuilder::DoShowDialog()
{
	Dialog Dlg(DialogItems, DialogItemsCount);
	Dlg.SetHelp(HelpTopic);
	Dlg.SetPosition(-1, -1, DialogItems [0].X2+4, DialogItems [0].Y2+2);
	Dlg.Process();
	return Dlg.GetExitCode();
}
