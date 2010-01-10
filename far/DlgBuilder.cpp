/*
DlgBuilder.cpp

Динамическое построение диалогов
*/
/*
Copyright (c) 2009 Far Group
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

#include "lang.hpp"
#include "FarDlgBuilder.hpp"
#include "dialog.hpp"
#include "language.hpp"

const int DEFAULT_INDENT = 5;

struct EditFieldBinding: public DialogItemBinding<DialogItemEx>
{
	string *TextValue;

	EditFieldBinding(string *aTextValue)
		: TextValue(aTextValue)
	{
	}

	virtual void SaveValue(DialogItemEx *Item, int RadioGroupIndex)
	{
		*TextValue = Item->strData;
	}
};

struct EditFieldIntBinding: public DialogItemBinding<DialogItemEx>
{
	int *IntValue;

	EditFieldIntBinding(int *aIntValue)
		: IntValue(aIntValue)
	{
	}

	virtual void SaveValue(DialogItemEx *Item, int RadioGroupIndex)
	{
		wchar_t *endptr;
		*IntValue = wcstoul(Item->strData, &endptr, 10);
	}
};

/*
static bool IsEditField(DialogItemEx *Item)
{
	return Item->Type == DI_EDIT || Item->Type == DI_FIXEDIT || Item->Type == DI_PSWEDIT;
}
*/

DialogBuilder::DialogBuilder(int TitleMessageId, const wchar_t *HelpTopic)
{
	AddBorder(GetLangString(TitleMessageId));
	this->HelpTopic = HelpTopic;
}

DialogBuilder::~DialogBuilder()
{
}

void DialogBuilder::InitDialogItem(DialogItemEx *Item, const TCHAR *Text)
{
	Item->Clear();
	Item->ID = DialogItemsCount-1;
	Item->strData = Text;
}

int DialogBuilder::TextWidth(const DialogItemEx &Item)
{
	return static_cast<int>(Item.strData.GetLength());
}

const TCHAR *DialogBuilder::GetLangString(int MessageID)
{
	return MSG(MessageID);
}

DialogItemBinding<DialogItemEx> *DialogBuilder::CreateCheckBoxBinding(BOOL *Value)
{
	return new CheckBoxBinding<DialogItemEx>(Value);
}

void DialogBuilder::AddRadioButtons(int *Value, int OptionCount, int MessageIDs[])
{
	for(int i=0; i<OptionCount; i++)
	{
		DialogItemEx *Item = AddDialogItem(DI_RADIOBUTTON, MSG(MessageIDs[i]));
		SetNextY(Item);
		Item->X2 = Item->X1 + ItemWidth(*Item);
		if (i == 0)
			Item->Flags |= DIF_GROUP;
		if (*Value == i)
			Item->Selected = TRUE;
		SetLastItemBinding(new RadioButtonBinding<DialogItemEx>(Value));
	}
}

DialogItemEx *DialogBuilder::AddText(int LabelId)
{
	DialogItemEx *Item = AddDialogItem(DI_TEXT, MSG(LabelId));
	SetNextY(Item);
	return Item;
}

DialogItemEx *DialogBuilder::AddEditField(string *Value, int Width, const wchar_t *HistoryID)
{
	DialogItemEx *Item = AddDialogItem(DI_EDIT, *Value);
	SetNextY(Item);
	Item->X2 = Item->X1 + Width;
	if (HistoryID)
	{
		Item->History = HistoryID;
		Item->Flags |= DIF_HISTORY;
	}

	SetLastItemBinding(new EditFieldBinding(Value));
	return Item;
}

DialogItemEx *DialogBuilder::AddIntEditField(int *Value, int Width)
{
	DialogItemEx *Item = AddDialogItem(DI_FIXEDIT, L"");
	string ValueText;
	ValueText.Format(L"%u", *Value);
	Item->strData = ValueText;
	SetNextY(Item);
	Item->X2 = Item->X1 + Width;


	SetLastItemBinding(new EditFieldIntBinding(Value));
	return Item;
}

DialogItemEx *DialogBuilder::AddComboBox(int *Value, int Width,
										 DialogBuilderListItem *Items, int ItemCount,
										 DWORD Flags)
{
	DialogItemEx *Item = AddDialogItem(DI_COMBOBOX, L"");
	SetNextY(Item);
	Item->X2 = Item->X1 + Width;
	Item->Flags |= Flags;

	FarListItem *ListItems = new FarListItem[ItemCount];
	for(int i=0; i<ItemCount; i++)
	{
		ListItems [i].Text = MSG(Items [i].MessageId);
		ListItems [i].Flags = (*Value == Items [i].ItemValue) ? LIF_SELECTED : 0;
		ListItems [i].Reserved [0] = Items [i].ItemValue;
	}
	FarList *List = new FarList;
	List->Items = ListItems;
	List->ItemsNumber = ItemCount;
	Item->ListItems = List;

	SetLastItemBinding(new ComboBoxBinding<DialogItemEx>(Value, List));
	return Item;
}

DialogItemEx *DialogBuilder::AddTextBefore(DialogItemEx *RelativeTo, int LabelId)
{
	DialogItemEx *Item = AddDialogItem(DI_TEXT, MSG(LabelId));
	Item->Y1 = Item->Y2 = RelativeTo->Y1;
	Item->X1 = 5;
	Item->X2 = Item->X1 + ItemWidth(*Item) - 1;

	int RelativeToWidth = RelativeTo->X2 - RelativeTo->X1;
	RelativeTo->X1 = Item->X2 + 2;
	RelativeTo->X2 = RelativeTo->X1 + RelativeToWidth;

	DialogItemBinding<DialogItemEx> *Binding = FindBinding(RelativeTo);
	if (Binding)
		Binding->BeforeLabelID = Item->ID;

	return Item;
}

DialogItemEx *DialogBuilder::AddTextAfter(DialogItemEx *RelativeTo, int LabelId)
{
	DialogItemEx *Item = AddDialogItem(DI_TEXT, MSG(LabelId));
	Item->Y1 = Item->Y2 = RelativeTo->Y1;
	Item->X1 = RelativeTo->X2 + 2;

	DialogItemBinding<DialogItemEx> *Binding = FindBinding(RelativeTo);
	if (Binding)
		Binding->AfterLabelID = Item->ID;

	return Item;
}

void DialogBuilder::LinkFlags(DialogItemEx *Parent, DialogItemEx *Target, FarDialogItemFlags Flags, bool LinkLabels)
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

void DialogBuilder::LinkFlagsByID(DialogItemEx *Parent, int TargetID, FarDialogItemFlags Flags)
{
	if (TargetID >= 0)
	{
		Parent->AddAutomation(TargetID, Flags, DIF_NONE, DIF_NONE, Flags, DIF_NONE, DIF_NONE);
		if (!Parent->Selected)
			DialogItems [TargetID].Flags |= Flags;
	}
}

int DialogBuilder::DoShowDialog()
{
	Dialog Dlg(DialogItems, DialogItemsCount);
	Dlg.SetHelp(HelpTopic);
	Dlg.SetPosition(-1, -1, DialogItems [0].X2+4, DialogItems [0].Y2+2);
	Dlg.Process();
	return Dlg.GetExitCode();
}
