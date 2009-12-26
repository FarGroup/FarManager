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

DialogBuilder::DialogBuilder(int TitleMessageId, const wchar_t *HelpTopic)
	: DialogItemsCount(0),
	  DialogItemsAllocated(0),
	  DialogItems(NULL)
{
	DialogItemEx *Title = AddDialogItem(DI_DOUBLEBOX, MSG(TitleMessageId));
	Title->X1 = 3;
	Title->Y1 = 1;
	this->HelpTopic = HelpTopic;
}

DialogBuilder::~DialogBuilder()
{
	if (DialogItems)
		delete [] DialogItems;
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

DialogItemEx *DialogBuilder::AddCheckbox(int TextMessageId, BOOL *Value)
{
	DialogItemEx *Item = AddDialogItem(DI_CHECKBOX, MSG(TextMessageId));
	Item->X1 = 5;
	Item->Y1 = Item->Y2 = DialogItemsCount;
	Item->Selected = *Value;
	Item->UserData = (DWORD_PTR) Value;
	return Item;
}

void DialogBuilder::AddOKCancel()
{
	DialogItemEx *Separator = AddDialogItem(DI_TEXT, L"");
	Separator->Flags = DIF_BOXCOLOR | DIF_SEPARATOR;
	Separator->X1 = 3;
	Separator->Y1 = Separator->Y2 = DialogItemsCount;

	DialogItemEx *OKButton = AddDialogItem(DI_BUTTON, MSG(MOk));
	OKButton->Flags = DIF_CENTERGROUP;
	OKButton->DefaultButton = 1;
	OKButton->Y1 = OKButton->Y2 = DialogItemsCount;
	OKButtonID = OKButton->ID;

	DialogItemEx *CancelButton = AddDialogItem(DI_BUTTON, MSG(MCancel));
	CancelButton->Flags = DIF_CENTERGROUP;
	CancelButton->Y1 = CancelButton->Y2 = OKButton->Y1;	
}

void DialogBuilder::UpdateBorderSize()
{
	DialogItemEx *Title = &DialogItems[0];
	Title->X2 = Title->X1 + MaxTextWidth() + 2;
	Title->Y2 = DialogItems [DialogItemsCount-1].Y2 + 1;
}

int DialogBuilder::MaxTextWidth()
{
	int MaxWidth = 0;
	for(int i=1; i<DialogItemsCount; i++)
	{
		int Width = 0;
		if (DialogItems [i].Type == DI_TEXT)
			Width = static_cast<int>(DialogItems [i].strData.GetLength());
		else if (DialogItems [i].Type == DI_CHECKBOX || DialogItems [i].Type == DI_RADIOBUTTON)
			Width = static_cast<int>(DialogItems [i].strData.GetLength() + 4);
		
		if (MaxWidth < Width)
			MaxWidth = Width;
	}
	return MaxWidth;
}

BOOL DialogBuilder::ShowDialog()
{
	UpdateBorderSize();

	Dialog Dlg(DialogItems, DialogItemsCount);
	Dlg.SetHelp(HelpTopic);
	Dlg.SetPosition(-1, -1, DialogItems [0].X2+4, DialogItems [0].Y2+2);
	Dlg.Process();

	if (Dlg.GetExitCode() != OKButtonID)
		return FALSE;

	for(int i=0; i<DialogItemsCount; i++)
		SaveValue(&DialogItems [i]);

	return TRUE;
}

void DialogBuilder::SaveValue(DialogItemEx *Item)
{
	if (Item->Type == DI_CHECKBOX)
	{
		BOOL *Value = (BOOL *)Item->UserData;
		*Value = Item->Selected;
	}
}