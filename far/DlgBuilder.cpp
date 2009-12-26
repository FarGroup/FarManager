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

#include "DlgBuilder.hpp"
#include "dialog.hpp"
#include "language.hpp"
#include "lang.hpp"

struct EditFieldUserData
{
	string *TextValue;
	int *IntValue;
	int PrefixLabelID;
	int SuffixLabelID;

	EditFieldUserData(string *aTextValue)
		: TextValue(aTextValue), IntValue(NULL), PrefixLabelID(-1), SuffixLabelID(-1)
	{
	}

	EditFieldUserData(int *aIntValue)
		: TextValue(NULL), IntValue(aIntValue), PrefixLabelID(-1), SuffixLabelID(-1)
	{
	}
};

static bool IsEditField(DialogItemEx *Item)
{
	return Item->Type == DI_EDIT || Item->Type == DI_FIXEDIT || Item->Type == DI_PSWEDIT;
}

static int TextWidth(const DialogItemEx &Item)
{
	switch(Item.Type)
	{
	case DI_TEXT:
		return static_cast<int>(Item.strData.GetLength());

	case DI_CHECKBOX:
	case DI_RADIOBUTTON:
		return static_cast<int>(Item.strData.GetLength() + 4);

	case DI_EDIT:
	case DI_FIXEDIT:
		return Item.X2 - Item.X1;
		break;
	}
	return 0;
}

DialogBuilder::DialogBuilder(int TitleMessageId, const wchar_t *HelpTopic)
	: DialogItemsCount(0),
	  DialogItemsAllocated(0),
	  DialogItems(NULL)
{
	DialogItemEx *Title = AddDialogItem(DI_DOUBLEBOX, MSG(TitleMessageId));
	Title->X1 = 3;
	Title->Y1 = 1;
	NextY = 2;
	this->HelpTopic = HelpTopic;
}

DialogBuilder::~DialogBuilder()
{
	if (DialogItems)
	{
		for(int i=0; i<DialogItemsCount; i++)
		{
			if (IsEditField(&DialogItems [i]))
				delete (EditFieldUserData *) DialogItems [i].UserData;
		}
		delete [] DialogItems;
	}
}

void DialogBuilder::ReallocDialogItems()
{
	// реаллокация инвалидирует указатели на DialogItemEx, возвращённые из
	// AddDialogItem и аналогичных методов, поэтому размер массива подбираем такой,
	// чтобы все нормальные диалоги помещались без реаллокации
	// TODO хорошо бы, чтобы они вообще не инвалидировались
	DialogItemsAllocated += 32;
	if (DialogItems == NULL) 
	{
		DialogItems = new DialogItemEx[DialogItemsAllocated];
	}
	else
	{
		DialogItemEx *NewDialogItems = new DialogItemEx[DialogItemsAllocated];
		for(int i=0; i<DialogItemsCount; i++)
			NewDialogItems [i] = DialogItems [i];
		delete [] DialogItems;
		DialogItems = NewDialogItems;
	}
}

DialogItemEx *DialogBuilder::AddDialogItem(int Type, const string &strData)
{
	if (DialogItemsCount == DialogItemsAllocated)
	{
		ReallocDialogItems();
	}
	DialogItemEx *Item = &DialogItems [DialogItemsCount++];
	Item->Clear();
	Item->ID = DialogItemsCount-1;
	Item->Type = Type;
	Item->strData = strData;
	return Item;
}

void DialogBuilder::SetNextY(DialogItemEx *Item)
{
	Item->X1 = 5;
	Item->Y1 = Item->Y2 = NextY++;
}

DialogItemEx *DialogBuilder::AddCheckbox(int TextMessageId, BOOL *Value)
{
	DialogItemEx *Item = AddDialogItem(DI_CHECKBOX, MSG(TextMessageId));
	SetNextY(Item);
	Item->X2 = Item->X1 + TextWidth(*Item);
	Item->Selected = *Value;
	Item->UserData = (DWORD_PTR) Value;
	return Item;
}

DialogItemEx *DialogBuilder::AddText(int LabelId)
{
	DialogItemEx *Item = AddDialogItem(DI_TEXT, MSG(LabelId));
	SetNextY(Item);
	return Item;
}

DialogItemEx *DialogBuilder::AddEditField(string *Value, int Width)
{
	DialogItemEx *Item = AddDialogItem(DI_EDIT, *Value);
	SetNextY(Item);
	Item->X2 = Item->X1 + Width;

	Item->UserData = (DWORD_PTR) new EditFieldUserData(Value);
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

	Item->UserData = (DWORD_PTR) new EditFieldUserData(Value);
	return Item;
}

void DialogBuilder::AddSuffixText(DialogItemEx *RelativeTo, int LabelId)
{
	DialogItemEx *Item = AddDialogItem(DI_TEXT, MSG(LabelId));
	Item->Y1 = Item->Y2 = RelativeTo->Y1;
	Item->X1 = RelativeTo->X2 + 2;

	if (IsEditField(RelativeTo))
	{
		EditFieldUserData *UserData = (EditFieldUserData *) RelativeTo->UserData;
		UserData->SuffixLabelID = Item->ID;
	}
}

void DialogBuilder::AddOKCancel()
{
	DialogItemEx *Separator = AddDialogItem(DI_TEXT, L"");
	Separator->Flags = DIF_BOXCOLOR | DIF_SEPARATOR;
	Separator->X1 = 3;
	Separator->Y1 = Separator->Y2 = NextY++;

	DialogItemEx *OKButton = AddDialogItem(DI_BUTTON, MSG(MOk));
	OKButton->Flags = DIF_CENTERGROUP;
	OKButton->DefaultButton = 1;
	OKButton->Y1 = OKButton->Y2 = NextY++;
	OKButtonID = OKButton->ID;

	DialogItemEx *CancelButton = AddDialogItem(DI_BUTTON, MSG(MCancel));
	CancelButton->Flags = DIF_CENTERGROUP;
	CancelButton->Y1 = CancelButton->Y2 = OKButton->Y1;	
}

void DialogBuilder::LinkFlags(DialogItemEx *Parent, DialogItemEx *Target, FarDialogItemFlags Flags)
{
	Parent->Flags |= DIF_AUTOMATION;
	Parent->AddAutomation(Target->ID, Flags, DIF_NONE, DIF_NONE, Flags, DIF_NONE, DIF_NONE);
	if (!Parent->Selected)
		Target->Flags |= Flags;

	if (IsEditField(Target))
	{
		EditFieldUserData *UserData = (EditFieldUserData *) Target->UserData;
		if (UserData->SuffixLabelID >= 0)
		{
			Parent->AddAutomation(UserData->SuffixLabelID, Flags, DIF_NONE, DIF_NONE, Flags, DIF_NONE, DIF_NONE);
			if (!Parent->Selected)
				DialogItems [UserData->SuffixLabelID].Flags |= Flags;
		}
	}
}

void DialogBuilder::UpdateBorderSize()
{
	DialogItemEx *Title = &DialogItems[0];
	Title->X2 = Title->X1 + MaxTextWidth() + 3;
	Title->Y2 = DialogItems [DialogItemsCount-1].Y2 + 1;
}

int DialogBuilder::MaxTextWidth()
{
	int MaxWidth = 0;
	for(int i=1; i<DialogItemsCount; i++)
	{
		int Width = TextWidth(DialogItems [i]);
		int Indent = DialogItems [i].X1 - 5;
		Width += Indent;
		
		if (MaxWidth < Width)
			MaxWidth = Width;
	}
	return MaxWidth;
}

bool DialogBuilder::ShowDialog()
{
	UpdateBorderSize();

	Dialog Dlg(DialogItems, DialogItemsCount);
	Dlg.SetHelp(HelpTopic);
	Dlg.SetPosition(-1, -1, DialogItems [0].X2+4, DialogItems [0].Y2+2);
	Dlg.Process();

	if (Dlg.GetExitCode() != OKButtonID)
		return false;

	for(int i=0; i<DialogItemsCount; i++)
		SaveValue(&DialogItems [i]);

	return true;
}

void DialogBuilder::SaveValue(DialogItemEx *Item)
{
	if (Item->Type == DI_CHECKBOX)
	{
		BOOL *Value = (BOOL *)Item->UserData;
		*Value = Item->Selected;
	}
	else if (IsEditField(Item))
	{
		EditFieldUserData *UserData = (EditFieldUserData *)Item->UserData;
		if (UserData->TextValue)
			*UserData->TextValue = Item->strData;
		else if (UserData->IntValue)
		{
			wchar_t *endptr;
			*UserData->IntValue = wcstoul(Item->strData, &endptr, 10);
		}
	}
}