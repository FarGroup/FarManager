#pragma once

/*
DlgBuilder.hpp

Динамическое конструирование диалогов
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

struct DialogItemEx;
class string;

// Элемент выпадающего списка в диалоге.
struct DialogBuilderListItem
{
	// Строчка из LNG-файла, которая будет показана в диалоге.
	int MessageId;

	// Значение, которое будет записано в поле Value при выборе этой строчки.
	int ItemValue;
};

/* 
Класс для динамического построения диалогов. Автоматически вычисляет положение и размер
для добавляемых контролов, а также размер самого диалога. Автоматически записывает выбранные 
значения в указанное место после закрытия диалога по OK.

По умолчанию каждый контрол размещается в новой строке диалога. Ширина для текстовых строк,
checkbox и radio button вычисляется автоматически, для других элементов передаётся явно.
Есть также возможность добавить статический текст слева или справа от контрола, при помощи
методов AddTextBefore и AddTextAfter. Для того, чтобы сместить элемент относительно дефолтного
положения по горизонтали, можно использовать метод DialogItemEx::Indent().

Есть также поддержка automation (изменение флагов одного элемента в зависимости от состояния 
другого). Реализуется при помощи метода LinkFlags().
*/
class DialogBuilder
{
	private:
		const wchar_t *HelpTopic;
		DialogItemEx *DialogItems;
		int DialogItemsCount;
		int DialogItemsAllocated;
		int OKButtonID;
		int NextY;

		DialogItemEx *AddDialogItem(int Type, const string &strData);
		void ReallocDialogItems();
		void UpdateBorderSize();
		int MaxTextWidth();
		void SaveValue(DialogItemEx *Item, int RadioGroupIndex);
		void SetNextY(DialogItemEx *Item);
		void LinkFlagsByID(DialogItemEx *Parent, int TargetID, FarDialogItemFlags Flags);

	public:
		DialogBuilder(int TitleMessageId, const wchar_t *HelpTopic);
		~DialogBuilder();

		// Добавляет чекбокс.
		DialogItemEx *AddCheckbox(int TextMessageId, BOOL *Value);

		// Добавляет группу радиокнопок.
		void AddRadioButtons(int *Value, int OptionCount, int MessageIDs[]);

		// Добавляет статический текст, расположенный на отдельной строке в диалоге.
		DialogItemEx *AddText(int LabelId);

		// Добавляет поле типа DI_EDIT для редактирования указанного строкового значения.
		DialogItemEx *AddEditField(string *Value, int Width, const wchar_t *HistoryID = NULL);

		// Добавляет поле типа DI_FIXEDIT для редактирования указанного числового значения.
		DialogItemEx *AddIntEditField(int *Value, int Width);

		// Добавляет выпадающий список с указанными значениями.
		DialogItemEx *AddComboBox(int *Value, int Width, DialogBuilderListItem *Items, int ItemCount, DWORD Flags = DIF_NONE);

		// Добавляет указанную текстовую строку слева от элемента RelativeTo.
		DialogItemEx *AddTextBefore(DialogItemEx *RelativeTo, int LabelId);

		// Добавляет указанную текстовую строку справа от элемента RelativeTo.
		DialogItemEx *AddTextAfter(DialogItemEx *RelativeTo, int LabelId);

		// Связывает состояние элементов Parent и Target. Когда Parent->Selected равно
		// false, устанавливает флаги Flags у элемента Target; когда равно true -
		// сбрасывает флаги.
		// Если LinkLabels установлено в true, то текстовые элементы, добавленные к элементу Target
		// методами AddTextBefore и AddTextAfter, также связываются с элементом Parent.
		void LinkFlags(DialogItemEx *Parent, DialogItemEx *Target, FarDialogItemFlags Flags, bool LinkLabels=true);

		// Добавляет сепаратор.
		void AddSeparator();

		// Добавляет сепаратор, кнопки OK и Cancel.
		void AddOKCancel();
		bool ShowDialog();
};
