/*
FarDlgBuilder.cpp

Динамическое построение диалогов - версия для внутреннего употребления в FAR
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

#include "language.hpp"
#include "FarDlgBuilder.hpp"
#include "dialog.hpp"
#include "interf.hpp"
#include "strmix.hpp"

template<class T>
struct EditFieldBinding: public DialogItemBinding<DialogItemEx>
{
	T& TextValue;

	EditFieldBinding(T& aTextValue)
		: TextValue(aTextValue)
	{
	}

	virtual void SaveValue(DialogItemEx *Item, int RadioGroupIndex) override
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
		Mask[0] = L'#';
		const auto MaskWidth = std::min(static_cast<int>(ARRAYSIZE(Mask) - 1), Width);
		std::fill(Mask + 1, Mask + MaskWidth, L'9');
		Mask[MaskWidth] = L'\0';
	}

	virtual void SaveValue(DialogItemEx *Item, int RadioGroupIndex) override
	{
		try
		{
			*IntValue = std::stoull(Item->strData);
		}
		catch (const std::exception&)
		{
			// don't changed
			// TODO: diagnostics
		}
	}

	const wchar_t *GetMask() const
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
		Mask[0] = L'x';
		const auto MaskWidth = std::min(static_cast<int>(ARRAYSIZE(Mask) - 1), Width);
		std::fill(Mask + 1, Mask + MaskWidth, L'H');
		Mask[MaskWidth] = L'\0';
	}

	virtual void SaveValue(DialogItemEx *Item, int RadioGroupIndex) override
	{
		*IntValue = std::wcstoll(Item->strData.data() + 1, nullptr, 16);
	}

	const wchar_t *GetMask() const
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

	virtual void SaveValue(DialogItemEx *Item, int RadioGroupIndex) override
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

	virtual void SaveValue(DialogItemEx *Item, int RadioGroupIndex) override
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

	virtual void SaveValue(DialogItemEx *Item, int RadioGroupIndex) override
	{
		Value = Item->Selected != BSTATE_UNCHECKED;
	}
};

template<class T>
struct FarListControlBinding: public DialogItemBinding<DialogItemEx>
{
private:
	T& Value;
	string *Text;
	FarList *List;

public:
	FarListControlBinding(T& aValue, string *aText, FarList *aList)
		: Value(aValue), Text(aText), List(aList)
	{
	}

	virtual ~FarListControlBinding()
	{
		if (List)
		{
			delete [] List->Items;
		}
		delete List;
	}

	virtual void SaveValue(DialogItemEx *Item, int RadioGroupIndex) override
	{
		if (List)
		{
			FarListItem &ListItem = List->Items[Item->ListPos];
			Value = ListItem.Reserved[0];
		}
		if (Text)
		{
			*Text = Item->strData;
		}
	}
};

template<class T>
struct FarRadioButtonBinding: public DialogItemBinding<DialogItemEx>
{
private:
	T& Value;

public:
	FarRadioButtonBinding(T& aValue) : Value(aValue) { }

	virtual void SaveValue(DialogItemEx *Item, int RadioGroupIndex) override
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

DialogBuilder::DialogBuilder(LNGID TitleMessageId, const wchar_t *HelpTopic, Dialog::dialog_handler handler):
	m_HelpTopic(NullToEmpty(HelpTopic)), m_Mode(0), m_IdExist(false), m_handler(handler)
{
	AddBorder(GetLangString(TitleMessageId));
}

DialogBuilder::DialogBuilder():
	m_HelpTopic(L""),  m_Mode(0), m_IdExist(false)
{
	AddBorder(L"");
}

DialogBuilder::~DialogBuilder()
{
}

void DialogBuilder::InitDialogItem(DialogItemEx *Item, const wchar_t* Text)
{
	Item->strData = Text;
}

int DialogBuilder::TextWidth(const DialogItemEx &Item)
{
	return static_cast<int>(Item.strData.size());
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

DialogItemEx *DialogBuilder::AddEditField(string& Value, int Width, const wchar_t *HistoryID, FARDIALOGITEMFLAGS Flags)
{
	const auto Item = AddDialogItem(DI_EDIT, Value.data());
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

DialogItemEx *DialogBuilder::AddEditField(StringOption& Value, int Width, const wchar_t *HistoryID, FARDIALOGITEMFLAGS Flags)
{
	const auto Item = AddDialogItem(DI_EDIT, Value.data());
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

DialogItemEx *DialogBuilder::AddFixEditField(string& Value, int Width, const wchar_t *Mask)
{
	const auto Item = AddDialogItem(DI_FIXEDIT, Value.data());
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

DialogItemEx *DialogBuilder::AddFixEditField(StringOption& Value, int Width, const wchar_t *Mask)
{
	const auto Item = AddDialogItem(DI_FIXEDIT, Value.data());
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

DialogItemEx *DialogBuilder::AddConstEditField(const string& Value, int Width, FARDIALOGITEMFLAGS Flags)
{
	const auto Item = AddDialogItem(DI_EDIT, Value.data());
	SetNextY(Item);
	Item->X2 = Item->X1 + Width;
	Item->Flags |= Flags|DIF_READONLY;
	return Item;
}

DialogItemEx *DialogBuilder::AddIntEditField(int *Value, int Width)
{
	const auto Item = AddDialogItem(DI_FIXEDIT, L"");
	Item->strData = std::to_wstring(*Value);
	SetNextY(Item);
	Item->X2 = Item->X1 + Width - 1;

	const auto Binding = new EditFieldIntBinding<int>(Value, Width);
	SetLastItemBinding(Binding);
	Item->Flags |= DIF_MASKEDIT;
	Item->strMask = Binding->GetMask();
	return Item;
}

DialogItemEx *DialogBuilder::AddIntEditField(IntOption& Value, int Width)
{
	const auto Item = AddDialogItem(DI_FIXEDIT, L"");
	Item->strData = std::to_wstring(Value.Get());
	SetNextY(Item);
	Item->X2 = Item->X1 + Width - 1;

	const auto Binding = new EditFieldIntBinding<IntOption>(&Value, Width);
	SetLastItemBinding(Binding);
	Item->Flags |= DIF_MASKEDIT;
	Item->strMask = Binding->GetMask();
	return Item;
}

DialogItemEx *DialogBuilder::AddHexEditField(IntOption& Value, int Width)
{
	const auto Item = AddDialogItem(DI_FIXEDIT, L"");
	std::wostringstream oss;
	oss << L'x' << std::hex << std::setw(8) << std::setfill(L'0') << Value.Get();
	Item->strData = oss.str();
	SetNextY(Item);
	Item->X2 = Item->X1 + Width - 1;

	const auto Binding = new EditFieldHexBinding<IntOption>(&Value, Width);
	SetLastItemBinding(Binding);
	Item->Flags |= DIF_MASKEDIT;
	Item->strMask = Binding->GetMask();
	return Item;
}

DialogItemEx *DialogBuilder::AddListControl(FARDIALOGITEMTYPES Type, int& Value, string *Text, int Width, int Height, const DialogBuilderListItem *Items, size_t ItemCount, FARDIALOGITEMFLAGS Flags)
{
	const auto Item = AddDialogItem(Type, Text ? Text->data() : L"");
	SetNextY(Item);
	Item->X2 = Item->X1 + Width;
	Item->Y2 = Item->Y1 + Height;
	Item->Flags |= Flags;

	m_NextY += Height;

	const auto ListItems = Items? new FarListItem[ItemCount] : nullptr;
	if (Items)
	{
		std::transform(Items, Items + ItemCount, ListItems, [&Value](const DialogBuilderListItem& Item) -> FarListItem
		{
			FarListItem NewItem = {};
			NewItem.Text = MSG(static_cast<LNGID>(Item.MessageId));
			NewItem.Flags = (Value == Item.ItemValue)? LIF_SELECTED : 0;
			NewItem.Reserved[0] = Item.ItemValue;
			return NewItem;
		});
	}
	const auto List = new FarList;
	List->StructSize = sizeof(FarList);
	List->Items = ListItems;
	List->ItemsNumber = ItemCount;
	Item->ListItems = List;

	SetLastItemBinding(new ListControlBinding<DialogItemEx>(Value, Text, List));
	return Item;
}

DialogItemEx *DialogBuilder::AddListControl(FARDIALOGITEMTYPES Type, IntOption& Value, string *Text, int Width, int Height, const DialogBuilderListItem *Items, size_t ItemCount, FARDIALOGITEMFLAGS Flags)
{
	const auto Item = AddDialogItem(Type, Text ? Text->data() : L"");
	SetNextY(Item);
	Item->X2 = Item->X1 + Width;
	Item->Y2 = Item->Y1 + Height;
	Item->Flags |= Flags;

	m_NextY += Height;

	const auto ListItems = Items? new FarListItem[ItemCount] : nullptr;
	if (Items)
	{
		std::transform(Items, Items + ItemCount, ListItems, [&Value](const DialogBuilderListItem& Item) -> FarListItem
		{
			FarListItem NewItem = {};
			NewItem.Text = MSG(static_cast<LNGID>(Item.MessageId));
			NewItem.Flags = (Value == Item.ItemValue)? LIF_SELECTED : 0;
			NewItem.Reserved[0] = Item.ItemValue;
			return NewItem;
		});
	}
	const auto List = new FarList;
	List->StructSize = sizeof(FarList);
	List->Items = ListItems;
	List->ItemsNumber = ItemCount;
	Item->ListItems = List;

	SetLastItemBinding(new FarListControlBinding<IntOption>(Value, Text, List));
	return Item;
}

DialogItemEx *DialogBuilder::AddListControl(FARDIALOGITEMTYPES Type, int& Value, string *Text, int Width, int Height, const std::vector<DialogBuilderListItem2> &Items, FARDIALOGITEMFLAGS Flags)
{
	const auto Item = AddDialogItem(Type, Text ? Text->data() : L"");
	SetNextY(Item);
	Item->X2 = Item->X1 + Width;
	Item->Y2 = Item->Y1 + Height;
	Item->Flags |= DIF_DROPDOWNLIST|Flags;

	m_NextY += Height;

	const auto ListItems = new FarListItem[Items.size()];
	std::transform(ALL_CONST_RANGE(Items), ListItems, [&Value](const DialogBuilderListItem2& Item) -> FarListItem
	{
		FarListItem NewItem = {};
		NewItem.Text = Item.Text.data();
		NewItem.Flags = Item.Flags | ((Value == Item.ItemValue)? LIF_SELECTED : 0);
		NewItem.Reserved[0] = Item.ItemValue;
		return NewItem;
	});
	const auto List = new FarList;
	List->StructSize = sizeof(FarList);
	List->Items = ListItems;
	List->ItemsNumber = Items.size();
	Item->ListItems = List;

	SetLastItemBinding(new ListControlBinding<DialogItemEx>(Value, Text, List));
	return Item;
}

DialogItemEx *DialogBuilder::AddListControl(FARDIALOGITEMTYPES Type, IntOption& Value, string *Text, int Width, int Height, const std::vector<DialogBuilderListItem2> &Items, FARDIALOGITEMFLAGS Flags)
{
	const auto Item = AddDialogItem(Type, Text ? Text->data() : L"");
	SetNextY(Item);
	Item->X2 = Item->X1 + Width;
	Item->Y2 = Item->Y1 + Height;
	Item->Flags |= DIF_DROPDOWNLIST|Flags;

	m_NextY += Height;

	const auto ListItems = new FarListItem[Items.size()];
	std::transform(ALL_CONST_RANGE(Items), ListItems, [&Value](const DialogBuilderListItem2& Item) -> FarListItem
	{
		FarListItem NewItem = {};
		NewItem.Text = Item.Text.data();
		NewItem.Flags = Item.Flags | ((Value == Item.ItemValue)? LIF_SELECTED : 0);
		NewItem.Reserved[0] = Item.ItemValue;
		return NewItem;
	});
	const auto List = new FarList;
	List->StructSize = sizeof(FarList);
	List->Items = ListItems;
	List->ItemsNumber = Items.size();
	Item->ListItems = List;

	SetLastItemBinding(new FarListControlBinding<IntOption>(Value, Text, List));
	return Item;
}

DialogItemEx *DialogBuilder::AddComboBox(int& Value, string *Text, int Width, const DialogBuilderListItem *Items, size_t ItemCount, FARDIALOGITEMFLAGS Flags)
{
	return AddListControl(DI_COMBOBOX, Value, Text, Width, 0, Items, ItemCount, Flags);
}

DialogItemEx *DialogBuilder::AddComboBox(IntOption& Value, string *Text, int Width, const DialogBuilderListItem *Items, size_t ItemCount, FARDIALOGITEMFLAGS Flags)
{
	return AddListControl(DI_COMBOBOX, Value, Text, Width, 0, Items, ItemCount, Flags);
}

DialogItemEx *DialogBuilder::AddComboBox(int& Value, string *Text, int Width, const std::vector<DialogBuilderListItem2> &Items, FARDIALOGITEMFLAGS Flags)
{
	return AddListControl(DI_COMBOBOX, Value, Text, Width, 0, Items, Flags);
}

DialogItemEx *DialogBuilder::AddComboBox(IntOption& Value, string *Text, int Width, const std::vector<DialogBuilderListItem2> &Items, FARDIALOGITEMFLAGS Flags)
{
	return AddListControl(DI_COMBOBOX, Value, Text, Width, 0, Items, Flags);
}

DialogItemEx *DialogBuilder::AddListBox(int& Value, int Width, int Height, const DialogBuilderListItem *Items, size_t ItemCount, FARDIALOGITEMFLAGS Flags)
{
	return AddListControl(DI_LISTBOX, Value, nullptr, Width, Height, Items, ItemCount, Flags);
}

DialogItemEx *DialogBuilder::AddListBox(IntOption& Value, int Width, int Height, const DialogBuilderListItem *Items, size_t ItemCount, FARDIALOGITEMFLAGS Flags)
{
	return AddListControl(DI_LISTBOX, Value, nullptr, Width, Height, Items, ItemCount, Flags);
}

DialogItemEx *DialogBuilder::AddListBox(int& Value, int Width, int Height, const std::vector<DialogBuilderListItem2> &Items, FARDIALOGITEMFLAGS Flags)
{
	return AddListControl(DI_LISTBOX, Value, nullptr, Width, Height, Items, Flags);
}

DialogItemEx *DialogBuilder::AddListBox(IntOption& Value, int Width, int Height, const std::vector<DialogBuilderListItem2> &Items, FARDIALOGITEMFLAGS Flags)
{
	return AddListControl(DI_LISTBOX, Value, nullptr, Width, Height, Items, Flags);
}

DialogItemEx *DialogBuilder::AddCheckbox(int TextMessageId, Bool3Option& Value)
{
	const auto Item = AddDialogItem(DI_CHECKBOX, GetLangString(TextMessageId));
	Item->Flags |= DIF_3STATE;
	SetNextY(Item);
	Item->X2 = Item->X1 + ItemWidth(*Item);
	Item->Selected = Value;
	SetLastItemBinding(CreateCheckBoxBinding(Value));
	return Item;
}

DialogItemEx *DialogBuilder::AddCheckbox(int TextMessageId, IntOption& Value, int Mask, bool ThreeState)
{
	const auto Item = AddDialogItem(DI_CHECKBOX, GetLangString(TextMessageId));
	if (ThreeState && !Mask)
		Item->Flags |= DIF_3STATE;
	SetNextY(Item);
	Item->X2 = Item->X1 + ItemWidth(*Item);
	if (!Mask)
		Item->Selected = Value;
	else
		Item->Selected = (Value & Mask) != 0;
	SetLastItemBinding(CreateCheckBoxBinding(Value, Mask));
	return Item;
}

DialogItemEx *DialogBuilder::AddCheckbox(const wchar_t* Caption, BoolOption& Value)
{
	const auto Item = AddDialogItem(DI_CHECKBOX, Caption);
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
		const auto Item = AddDialogItem(DI_RADIOBUTTON, GetLangString(MessageIDs[i]));
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
	Parent->AddAutomation(Target, Flags, DIF_NONE, DIF_NONE, Flags, DIF_NONE, DIF_NONE);
	if (!Parent->Selected)
		Target->Flags |= Flags;

	if (LinkLabels)
	{
		if (const auto Binding = FindBinding(Target))
		{
			if (Binding->BeforeLabelID != -1)
				LinkFlagsByID(Parent, &m_DialogItems[Binding->BeforeLabelID], Flags);
			if (Binding->AfterLabelID != -1)
				LinkFlagsByID(Parent, &m_DialogItems[Binding->AfterLabelID], Flags);
		}
	}
}

void DialogBuilder::LinkFlagsByID(DialogItemEx *Parent, DialogItemEx* Target, FARDIALOGITEMFLAGS Flags)
{
	Parent->AddAutomation(Target, Flags, DIF_NONE, DIF_NONE, Flags, DIF_NONE, DIF_NONE);
	if (!Parent->Selected)
		Target->Flags |= Flags;
}

intptr_t DialogBuilder::DoShowDialog()
{
	const auto Dlg = Dialog::create(make_range(m_DialogItems, m_DialogItems + m_DialogItemsCount), m_handler, nullptr);
	Dlg->SetHelp(m_HelpTopic);
	Dlg->SetPosition(-1, -1, m_DialogItems [0].X2+4, m_DialogItems [0].Y2+2);
	if (m_Mode)
		Dlg->SetDialogMode(m_Mode);
	if (m_IdExist)
		Dlg->SetId(m_Id);
	Dlg->Process();
	return Dlg->GetExitCode();
}

void DialogBuilder::SetDialogMode(DWORD Flags)
{
	m_Mode = Flags;
}

void DialogBuilder::AddOKCancel()
{
	DialogBuilderBase<DialogItemEx>::AddOKCancel(MOk, MCancel);
}

void DialogBuilder::AddOKCancel(int OKMessageId, int CancelMessageId)
{
	DialogBuilderBase<DialogItemEx>::AddOKCancel(OKMessageId, CancelMessageId);
}

void DialogBuilder::AddOK()
{
	DialogBuilderBase<DialogItemEx>::AddOKCancel(MOk, -1);
}

int DialogBuilder::AddTextWrap(const wchar_t *text, bool center, int width)
{
	string str(text);
	FarFormatText(str, width <= 0 ? ScrX-1-10 : width, str, L"\n", 0);
	int LineCount = 1 + std::count(ALL_CONST_RANGE(str), L'\n');
	std::replace(ALL_RANGE(str), L'\n', L'\0');
	const wchar_t *ps = str.data();
	for (int i = 0; i < LineCount; ++i, ps += wcslen(ps) + 1)
	{
		const auto Text = AddText(ps);
		Text->Flags = (center ? DIF_CENTERTEXT : 0);
	}
	return LineCount;
}

void DialogBuilder::SetId(const GUID& Id)
{
	m_Id=Id;
	m_IdExist=true;
}
