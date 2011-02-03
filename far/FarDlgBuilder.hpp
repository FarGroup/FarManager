#pragma once

/*
FarDlgBuilder.hpp

Динамическое конструирование диалогов - версия для внутреннего употребления в FAR
*/
/*
Copyright © 2010 Far Group
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

#include "DlgBuilder.hpp"

struct DialogItemEx;

/*
Класс для динамического построения диалогов, используемый внутри кода FAR.
Использует FAR'овский класс string для работы с текстовыми полями.

Для того, чтобы сместить элемент относительно дефолтного
положения по горизонтали, можно использовать метод DialogItemEx::Indent().

Поддерживает automation (изменение флагов одного элемента в зависимости от состояния
другого). Реализуется при помощи метода LinkFlags().
*/
class DialogBuilder: public DialogBuilderBase<DialogItemEx>
{
	private:
		const wchar_t *HelpTopic;

		void LinkFlagsByID(DialogItemEx *Parent, int TargetID, FarDialogItemFlags Flags);

	protected:
		virtual void InitDialogItem(DialogItemEx *Item, const TCHAR *Text);
		virtual int TextWidth(const DialogItemEx &Item);
		virtual const TCHAR *GetLangString(int MessageID);
		virtual int DoShowDialog();

		virtual DialogItemBinding<DialogItemEx> *CreateCheckBoxBinding(BOOL *Value, int Mask);
		virtual DialogItemBinding<DialogItemEx> *CreateRadioButtonBinding(int *Value);

	public:
		DialogBuilder(int TitleMessageId, const wchar_t *HelpTopic);
		~DialogBuilder();

		// Добавляет поле типа DI_EDIT для редактирования указанного строкового значения.
		DialogItemEx *AddEditField(string *Value, int Width, const wchar_t *HistoryID = nullptr, int Flags = 0);

		// Добавляет поле типа DI_FIXEDIT для редактирования указанного числового значения.
		virtual DialogItemEx *AddIntEditField(int *Value, int Width);

		// Добавляет выпадающий список с указанными значениями.
		DialogItemEx *AddComboBox(int *Value, int Width, DialogBuilderListItem *Items, int ItemCount, DWORD Flags = DIF_NONE);

		// Связывает состояние элементов Parent и Target. Когда Parent->Selected равно
		// false, устанавливает флаги Flags у элемента Target; когда равно true -
		// сбрасывает флаги.
		// Если LinkLabels установлено в true, то текстовые элементы, добавленные к элементу Target
		// методами AddTextBefore и AddTextAfter, также связываются с элементом Parent.
		void LinkFlags(DialogItemEx *Parent, DialogItemEx *Target, FarDialogItemFlags Flags, bool LinkLabels=true);

		void AddOKCancel()
		{
			DialogBuilderBase<DialogItemEx>::AddOKCancel(MOk, MCancel);
		}
};
