#pragma once
#ifndef __DLGBUILDER_HPP__
#define __DLGBUILDER_HPP__
#ifndef FAR_USE_INTERNALS
#define FAR_USE_INTERNALS
#endif // END FAR_USE_INTERNALS

#ifndef __cplusplus
#error C++ only
#endif

/*
  DlgBuilder.hpp

  Dynamic construction of dialogs for FAR Manager <%VERSION%>
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

// Элемент выпадающего списка в диалоге.
struct DialogBuilderListItem
{
	// Строчка из LNG-файла, которая будет показана в диалоге.
	int MessageId;

	// Значение, которое будет записано в поле Value при выборе этой строчки.
	int ItemValue;
};

template<class T>
struct DialogItemBinding
{
	int BeforeLabelID;
	int AfterLabelID;

	DialogItemBinding()
		: BeforeLabelID(-1), AfterLabelID(-1)
	{
	}

	virtual ~DialogItemBinding()
	{
	};

	virtual void SaveValue(T *Item, int RadioGroupIndex)
	{
	}
};

template<class T>
struct CheckBoxBinding: public DialogItemBinding<T>
{
	private:
		BOOL *Value;
		int Mask;

	public:
		CheckBoxBinding(int *aValue, int aMask) : Value(aValue), Mask(aMask) { }

		virtual void SaveValue(T *Item, int RadioGroupIndex)
		{
			if (!Mask)
			{
				*Value = Item->Selected;
			}
			else
			{
				if (Item->Selected)
					*Value |= Mask;
				else
					*Value &= ~Mask;
			}
		}
};

template<class T>
struct RadioButtonBinding: public DialogItemBinding<T>
{
	private:
		int *Value;

	public:
		RadioButtonBinding(int *aValue) : Value(aValue) { }

		virtual void SaveValue(T *Item, int RadioGroupIndex)
		{
			if (Item->Selected)
				*Value = RadioGroupIndex;
		}
};

/*
Класс для динамического построения диалогов. Автоматически вычисляет положение и размер
для добавляемых контролов, а также размер самого диалога. Автоматически записывает выбранные
значения в указанное место после закрытия диалога по OK.

По умолчанию каждый контрол размещается в новой строке диалога. Ширина для текстовых строк,
checkbox и radio button вычисляется автоматически, для других элементов передаётся явно.
Есть также возможность добавить статический текст слева или справа от контрола, при помощи
методов AddTextBefore и AddTextAfter.

Поддерживается также возможность расположения контролов в две колонки. Используется следующим
образом:
- StartColumns()
- добавляются контролы для первой колонки
- ColumnBreak()
- добавляются контролы для второй колонки
- EndColumns()

Поддерживается также возможность расположения контролов внутри бокса. Используется следующим
образом:
- StartSingleBox()
- добавляются контролы
- EndSingleBox()

Базовая версия класса используется как внутри кода Far, так и в плагинах.
*/

template<class T>
class DialogBuilderBase
{
	protected:
		T *m_DialogItems;
		DialogItemBinding<T> **m_Bindings;
		int m_DialogItemsCount;
		int m_DialogItemsAllocated;
		int m_NextY;
		int m_Indent;
		int m_SingleBoxIndex;
		int m_FirstButtonID;
		int m_CancelButtonID;
		int m_ColumnStartIndex;
		int m_ColumnBreakIndex;
		int m_ColumnStartY;
		int m_ColumnEndY;
		intptr_t m_ColumnMinWidth;

		static const int SECOND_COLUMN = -2;

		void ReallocDialogItems()
		{
			// реаллокация инвалидирует указатели на DialogItemEx, возвращённые из
			// AddDialogItem и аналогичных методов, поэтому размер массива подбираем такой,
			// чтобы все нормальные диалоги помещались без реаллокации
			// TODO хорошо бы, чтобы они вообще не инвалидировались
			m_DialogItemsAllocated += 64;
			if (!m_DialogItems)
			{
				m_DialogItems = new T[m_DialogItemsAllocated];
				m_Bindings = new DialogItemBinding<T> * [m_DialogItemsAllocated];
			}
			else
			{
				T *NewDialogItems = new T[m_DialogItemsAllocated];
				DialogItemBinding<T> **NewBindings = new DialogItemBinding<T> * [m_DialogItemsAllocated];
				for(int i=0; i<m_DialogItemsCount; i++)
				{
					NewDialogItems [i] = m_DialogItems [i];
					NewBindings [i] = m_Bindings [i];
				}
				delete [] m_DialogItems;
				delete [] m_Bindings;
				m_DialogItems = NewDialogItems;
				m_Bindings = NewBindings;
			}
		}

		T *AddDialogItem(FARDIALOGITEMTYPES Type, const wchar_t *Text)
		{
			if (m_DialogItemsCount == m_DialogItemsAllocated)
			{
				ReallocDialogItems();
			}
			int Index = m_DialogItemsCount++;
			T *Item = &m_DialogItems [Index];
			InitDialogItem(Item, Text);
			Item->Type = Type;
			m_Bindings [Index] = nullptr;
			return Item;
		}

		void SetNextY(T *Item)
		{
			Item->X1 = 5 + m_Indent;
			Item->Y1 = Item->Y2 = m_NextY++;
		}

		intptr_t ItemWidth(const T &Item)
		{
			switch(Item.Type)
			{
			case DI_TEXT:
				return TextWidth(Item);

			case DI_CHECKBOX:
			case DI_RADIOBUTTON:
			case DI_BUTTON:
				return TextWidth(Item) + 4;

			case DI_EDIT:
			case DI_FIXEDIT:
			case DI_COMBOBOX:
			case DI_LISTBOX:
			case DI_PSWEDIT:
				{
					intptr_t Width = Item.X2 - Item.X1 + 1;
					// стрелка history занимает дополнительное место, но раньше она рисовалась поверх рамки???
					if (Item.Flags & DIF_HISTORY)
						Width++;
					return Width;
				}
				break;

			default:
				break;
			}
			return 0;
		}

		void AddBorder(const wchar_t *TitleText)
		{
			T *Title = AddDialogItem(DI_DOUBLEBOX, TitleText);
			Title->X1 = 3;
			Title->Y1 = 1;
		}

		void UpdateBorderSize()
		{
			T *Title = &m_DialogItems[0];
			intptr_t MaxWidth = MaxTextWidth();
			intptr_t MaxHeight = 0;
			Title->X2 = Title->X1 + MaxWidth + 3;

			for (int i=1; i<m_DialogItemsCount; i++)
			{
				if (m_DialogItems[i].Type == DI_SINGLEBOX)
				{
					m_Indent = 2;
					m_DialogItems[i].X2 = Title->X2;
				}
				else if (m_DialogItems[i].Type == DI_TEXT && (m_DialogItems[i].Flags & DIF_CENTERTEXT))
				{//BUGBUG: two columns items are not supported
					m_DialogItems[i].X2 = m_DialogItems[i].X1 + MaxWidth - 1;
				}
				
				if (m_DialogItems[i].Y2 > MaxHeight)
				{
					MaxHeight = m_DialogItems[i].Y2;
				}
			}

			Title->X2 += m_Indent;
			Title->Y2 = MaxHeight + 1;
			m_Indent = 0;
		}

		intptr_t MaxTextWidth()
		{
			intptr_t MaxWidth = 0;
			for(int i=1; i<m_DialogItemsCount; i++)
			{
				if (m_DialogItems [i].X1 == SECOND_COLUMN) continue;
				intptr_t Width = ItemWidth(m_DialogItems [i]);
				intptr_t Indent = m_DialogItems [i].X1 - 5;
				Width += Indent;

				if (MaxWidth < Width)
					MaxWidth = Width;
			}
			intptr_t ColumnsWidth = 2*m_ColumnMinWidth+1;
			if (MaxWidth < ColumnsWidth)
				return ColumnsWidth;
			return MaxWidth;
		}

		void UpdateSecondColumnPosition()
		{
			intptr_t SecondColumnX1 = 6 + (m_DialogItems [0].X2 - m_DialogItems [0].X1 - 1)/2;
			for(int i=0; i<m_DialogItemsCount; i++)
			{
				if (m_DialogItems [i].X1 == SECOND_COLUMN)
				{
					intptr_t Width = m_DialogItems [i].X2 - m_DialogItems [i].X1;
					m_DialogItems [i].X1 = SecondColumnX1;
					m_DialogItems [i].X2 = m_DialogItems [i].X1 + Width;
				}
			}
		}

		virtual void InitDialogItem(T *NewDialogItem, const wchar_t *Text)
		{
		}

		virtual int TextWidth(const T &Item)
		{
			return -1;
		}

		void SetLastItemBinding(DialogItemBinding<T> *Binding)
		{
			m_Bindings [m_DialogItemsCount-1] = Binding;
		}

		int GetItemID(T *Item) const
		{
			int Index = static_cast<int>(Item - m_DialogItems);
			if (Index >= 0 && Index < m_DialogItemsCount)
				return Index;
			return -1;
		}

		DialogItemBinding<T> *FindBinding(T *Item)
		{
			int Index = static_cast<int>(Item - m_DialogItems);
			if (Index >= 0 && Index < m_DialogItemsCount)
				return m_Bindings [Index];
			return nullptr;
		}

		void SaveValues()
		{
			int RadioGroupIndex = 0;
			for(int i=0; i<m_DialogItemsCount; i++)
			{
				if (m_DialogItems [i].Flags & DIF_GROUP)
					RadioGroupIndex = 0;
				else
					RadioGroupIndex++;

				if (m_Bindings [i])
					m_Bindings [i]->SaveValue(&m_DialogItems [i], RadioGroupIndex);
			}
		}

		virtual const wchar_t *GetLangString(int MessageID)
		{
			return nullptr;
		}

		virtual intptr_t DoShowDialog()
		{
			return -1;
		}

		virtual DialogItemBinding<T> *CreateCheckBoxBinding(int *Value, int Mask)
		{
			return nullptr;
		}

		virtual DialogItemBinding<T> *CreateRadioButtonBinding(int *Value)
		{
			return nullptr;
		}

		DialogBuilderBase()
			: m_DialogItems(nullptr), m_Bindings(nullptr), m_DialogItemsCount(0), m_DialogItemsAllocated(0), m_NextY(2), m_Indent(0), m_SingleBoxIndex(-1),
			  m_FirstButtonID(-1), m_CancelButtonID(-1),
			  m_ColumnStartIndex(-1), m_ColumnBreakIndex(-1), m_ColumnStartY(-1), m_ColumnEndY(-1), m_ColumnMinWidth(0)
		{
		}

		virtual ~DialogBuilderBase()
		{
			for(int i=0; i<m_DialogItemsCount; i++)
			{
				delete m_Bindings [i];
			}
			delete [] m_DialogItems;
			delete [] m_Bindings;
		}

	public:

		int GetLastID() const
		{
			return m_DialogItemsCount-1;
		}

		// Добавляет статический текст, расположенный на отдельной строке в диалоге.
		T *AddText(int LabelId)
		{
			T *Item = AddDialogItem(DI_TEXT, LabelId == -1 ? L"" : GetLangString(LabelId));
			SetNextY(Item);
			return Item;
		}

		// Добавляет статический текст, расположенный на отдельной строке в диалоге.
		T *AddText(const wchar_t *Label)
		{
			T *Item = AddDialogItem(DI_TEXT, Label);
			SetNextY(Item);
			return Item;
		}

		// Добавляет чекбокс.
		T *AddCheckbox(int TextMessageId, int *Value, int Mask=0, bool ThreeState=false)
		{
			T *Item = AddDialogItem(DI_CHECKBOX, GetLangString(TextMessageId));
			if (ThreeState && !Mask)
				Item->Flags |= DIF_3STATE;
			SetNextY(Item);
			Item->X2 = Item->X1 + ItemWidth(*Item);
			if (!Mask)
				Item->Selected = *Value;
			else
				Item->Selected = (*Value & Mask) != 0;
			SetLastItemBinding(CreateCheckBoxBinding(Value, Mask));
			return Item;
		}

		// Добавляет группу радиокнопок.
		void AddRadioButtons(int *Value, int OptionCount, const int MessageIDs[], bool FocusOnSelected=false)
		{
			for(int i=0; i<OptionCount; i++)
			{
				T *Item = AddDialogItem(DI_RADIOBUTTON, GetLangString(MessageIDs[i]));
				SetNextY(Item);
				Item->X2 = Item->X1 + ItemWidth(*Item);
				if (!i)
					Item->Flags |= DIF_GROUP;
				if (*Value == i)
				{
					Item->Selected = TRUE;
					if (FocusOnSelected)
						Item->Flags |= DIF_FOCUS;
				}
				SetLastItemBinding(CreateRadioButtonBinding(Value));
			}
		}

		// Добавляет поле типа DI_FIXEDIT для редактирования указанного числового значения.
		virtual T *AddIntEditField(int *Value, int Width)
		{
			return nullptr;
		}

		virtual T *AddUIntEditField(unsigned int *Value, int Width)
		{
			return nullptr;
		}

		// Добавляет указанную текстовую строку слева от элемента RelativeTo.
		T *AddTextBefore(T *RelativeTo, int LabelId)
		{
			T *Item = AddDialogItem(DI_TEXT, GetLangString(LabelId));
			Item->Y1 = Item->Y2 = RelativeTo->Y1;
			Item->X1 = 5 + m_Indent;
			Item->X2 = Item->X1 + ItemWidth(*Item) - 1;

			intptr_t RelativeToWidth = RelativeTo->X2 - RelativeTo->X1;
			RelativeTo->X1 = Item->X2 + 2;
			RelativeTo->X2 = RelativeTo->X1 + RelativeToWidth;

			DialogItemBinding<T> *Binding = FindBinding(RelativeTo);
			if (Binding)
				Binding->BeforeLabelID = GetItemID(Item);

			return Item;
		}

		// Добавляет указанную текстовую строку справа от элемента RelativeTo.
		T *AddTextAfter(T *RelativeTo, const wchar_t* Label, int skip=1)
		{
			T *Item = AddDialogItem(DI_TEXT, Label);
			Item->Y1 = Item->Y2 = RelativeTo->Y1;
			Item->X1 = RelativeTo->X1 + ItemWidth(*RelativeTo) + skip;

			DialogItemBinding<T> *Binding = FindBinding(RelativeTo);
			if (Binding)
				Binding->AfterLabelID = GetItemID(Item);

			return Item;
		}

		T *AddTextAfter(T *RelativeTo, int LabelId, int skip=1)
		{
			return AddTextAfter(RelativeTo, GetLangString(LabelId), skip);
		}

		// Добавляет кнопку справа от элемента RelativeTo.
		T *AddButtonAfter(T *RelativeTo, int LabelId)
		{
			T *Item = AddDialogItem(DI_BUTTON, GetLangString(LabelId));
			Item->Y1 = Item->Y2 = RelativeTo->Y1;
			Item->X1 = RelativeTo->X1 + ItemWidth(*RelativeTo) - 1 + 2;

			DialogItemBinding<T> *Binding = FindBinding(RelativeTo);
			if (Binding)
				Binding->AfterLabelID = GetItemID(Item);

			return Item;
		}

		// Начинает располагать поля диалога в две колонки.
		void StartColumns()
		{
			m_ColumnStartIndex = m_DialogItemsCount;
			m_ColumnStartY = m_NextY;
		}

		// Завершает колонку полей в диалоге и переходит к следующей колонке.
		void ColumnBreak()
		{
			m_ColumnBreakIndex = m_DialogItemsCount;
			m_ColumnEndY = m_NextY;
			m_NextY = m_ColumnStartY;
		}

		// Завершает расположение полей диалога в две колонки.
		void EndColumns()
		{
			for(int i=m_ColumnStartIndex; i<m_DialogItemsCount; i++)
			{
				intptr_t Width = ItemWidth(m_DialogItems [i]);
				if (Width > m_ColumnMinWidth)
					m_ColumnMinWidth = Width;
				if (i >= m_ColumnBreakIndex)
				{
					m_DialogItems [i].X1 = SECOND_COLUMN;
					m_DialogItems [i].X2 = SECOND_COLUMN + Width;
				}
			}

			m_ColumnStartIndex = -1;
			m_ColumnBreakIndex = -1;
		}

		// Начинает располагать поля диалога внутри single box
		void StartSingleBox(int MessageId=-1, bool LeftAlign=false)
		{
			T *SingleBox = AddDialogItem(DI_SINGLEBOX, MessageId == -1 ? L"" : GetLangString(MessageId));
			SingleBox->Flags = LeftAlign ? DIF_LEFTTEXT : DIF_NONE;
			SingleBox->X1 = 5;
			SingleBox->Y1 = m_NextY++;
			m_Indent = 2;
			m_SingleBoxIndex = m_DialogItemsCount - 1;
		}

		// Завершает расположение полей диалога внутри single box
		void EndSingleBox()
		{
			if (m_SingleBoxIndex != -1)
			{
				m_DialogItems[m_SingleBoxIndex].Y2 = m_NextY++;
				m_Indent = 0;
				m_SingleBoxIndex = -1;
			}
		}

		// Добавляет пустую строку.
		void AddEmptyLine()
		{
			m_NextY++;
		}

		// Добавляет сепаратор.
		void AddSeparator(int MessageId=-1)
		{
			return AddSeparator(MessageId == -1 ? L"" : GetLangString(MessageId));
		}

		void AddSeparator(const wchar_t* Text)
		{
			T *Separator = AddDialogItem(DI_TEXT, Text);
			Separator->Flags = DIF_SEPARATOR;
			Separator->X1 = -1;
			Separator->Y1 = Separator->Y2 = m_NextY++;
		}

		// Добавляет сепаратор, кнопки OK и Cancel.
		void AddOKCancel(int OKMessageId, int CancelMessageId, int ExtraMessageId = -1, bool Separator=true)
		{
			if (Separator)
				AddSeparator();

			int MsgIDs[] = { OKMessageId, CancelMessageId, ExtraMessageId };
			int NumButtons = (ExtraMessageId != -1) ? 3 : (CancelMessageId != -1? 2 : 1);

			AddButtons(NumButtons, MsgIDs, 0, 1);
		}

		// Добавляет линейку кнопок.
		void AddButtons(int ButtonCount, const int* MessageIDs, int defaultButtonIndex = 0, int cancelButtonIndex = -1)
		{
			int LineY = m_NextY++;
			T *PrevButton = nullptr;

			for (int i = 0; i < ButtonCount; i++)
			{
				T *NewButton = AddDialogItem(DI_BUTTON, GetLangString(MessageIDs[i]));
				NewButton->Flags = DIF_CENTERGROUP;
				NewButton->Y1 = NewButton->Y2 = LineY;
				if (PrevButton)
				{
					NewButton->X1 = PrevButton->X2 + 1;
				}
				else
				{
					NewButton->X1 = 2 + m_Indent;
					m_FirstButtonID = m_DialogItemsCount - 1;
				}
				NewButton->X2 = NewButton->X1 + ItemWidth(*NewButton);

				if (defaultButtonIndex == i)
				{
					NewButton->Flags |= DIF_DEFAULTBUTTON;
				}
				if (cancelButtonIndex == i)
					m_CancelButtonID = m_DialogItemsCount - 1;

				PrevButton = NewButton;
			}
		}

		intptr_t ShowDialogEx()
		{
			UpdateBorderSize();
			UpdateSecondColumnPosition();
			intptr_t Result = DoShowDialog();
			if (Result != m_CancelButtonID)
			{
				SaveValues();
			}

			if (m_FirstButtonID >= 0 && Result >= m_FirstButtonID)
			{
				Result -= m_FirstButtonID;
			}
			return Result;
		}

		bool ShowDialog()
		{
			return ShowDialogEx() == 0;
		}

};

class PluginDialogBuilder;

class DialogAPIBinding: public DialogItemBinding<FarDialogItem>
{
protected:
	const PluginStartupInfo &Info;
	HANDLE *DialogHandle;
	int ID;

	DialogAPIBinding(const PluginStartupInfo &aInfo, HANDLE *aHandle, int aID)
		: Info(aInfo), DialogHandle(aHandle), ID(aID)
	{
	}
};

class PluginCheckBoxBinding: public DialogAPIBinding
{
	int *Value;
	int Mask;

public:
	PluginCheckBoxBinding(const PluginStartupInfo &aInfo, HANDLE *aHandle, int aID, int *aValue, int aMask)
		: DialogAPIBinding(aInfo, aHandle, aID),
		  Value(aValue), Mask(aMask)
	{
	}

	virtual void SaveValue(FarDialogItem *Item, int RadioGroupIndex)
	{
		int Selected = static_cast<int>(Info.SendDlgMessage(*DialogHandle, DM_GETCHECK, ID, 0));
		if (!Mask)
		{
			*Value = Selected;
		}
		else
		{
			if (Selected)
				*Value |= Mask;
			else
				*Value &= ~Mask;
		}
	}
};

class PluginRadioButtonBinding: public DialogAPIBinding
{
	private:
		int *Value;

	public:
		PluginRadioButtonBinding(const PluginStartupInfo &aInfo, HANDLE *aHandle, int aID, int *aValue)
			: DialogAPIBinding(aInfo, aHandle, aID),
			  Value(aValue)
		{
		}

		virtual void SaveValue(FarDialogItem *Item, int RadioGroupIndex)
		{
			if (Info.SendDlgMessage(*DialogHandle, DM_GETCHECK, ID, 0))
				*Value = RadioGroupIndex;
		}
};

class PluginEditFieldBinding: public DialogAPIBinding
{
private:
	wchar_t *Value;
	int MaxSize;

public:
	PluginEditFieldBinding(const PluginStartupInfo &aInfo, HANDLE *aHandle, int aID, wchar_t *aValue, int aMaxSize)
		: DialogAPIBinding(aInfo, aHandle, aID), Value(aValue), MaxSize(aMaxSize)
	{
	}

	virtual void SaveValue(FarDialogItem *Item, int RadioGroupIndex)
	{
		const wchar_t *DataPtr = (const wchar_t *) Info.SendDlgMessage(*DialogHandle, DM_GETCONSTTEXTPTR, ID, 0);
		lstrcpynW(Value, DataPtr, MaxSize);
	}
};

class PluginIntEditFieldBinding: public DialogAPIBinding
{
private:
	int *Value;
	wchar_t Buffer[32];
	wchar_t Mask[32];

public:
	PluginIntEditFieldBinding(const PluginStartupInfo &aInfo, HANDLE *aHandle, int aID, int *aValue, int Width)
		: DialogAPIBinding(aInfo, aHandle, aID),
		  Value(aValue)
	{
		memset(Buffer, 0, sizeof(Buffer));
		aInfo.FSF->sprintf(Buffer, L"%u", *aValue);
		int MaskWidth = Width < 31 ? Width : 31;
		for(int i=1; i<MaskWidth; i++)
			Mask[i] = L'9';
		Mask[0] = L'#';
		Mask[MaskWidth] = L'\0';
	}

	virtual void SaveValue(FarDialogItem *Item, int RadioGroupIndex)
	{
		const wchar_t *DataPtr = (const wchar_t *) Info.SendDlgMessage(*DialogHandle, DM_GETCONSTTEXTPTR, ID, 0);
		*Value = Info.FSF->atoi(DataPtr);
	}

	wchar_t *GetBuffer()
	{
		return Buffer;
	}

	const wchar_t *GetMask() const
	{
		return Mask;
	}
};

class PluginUIntEditFieldBinding: public DialogAPIBinding
{
private:
	unsigned int *Value;
	wchar_t Buffer[32];
	wchar_t Mask[32];

public:
	PluginUIntEditFieldBinding(const PluginStartupInfo &aInfo, HANDLE *aHandle, int aID, unsigned int *aValue, int Width)
		: DialogAPIBinding(aInfo, aHandle, aID),
		  Value(aValue)
	{
		memset(Buffer, 0, sizeof(Buffer));
		aInfo.FSF->sprintf(Buffer, L"%u", *aValue);
		int MaskWidth = Width < 31 ? Width : 31;
		for(int i=1; i<MaskWidth; i++)
			Mask[i] = L'9';
		Mask[0] = L'#';
		Mask[MaskWidth] = L'\0';
	}

	virtual void SaveValue(FarDialogItem *Item, int RadioGroupIndex)
	{
		const wchar_t *DataPtr = (const wchar_t *) Info.SendDlgMessage(*DialogHandle, DM_GETCONSTTEXTPTR, ID, 0);
		*Value = (unsigned int)Info.FSF->atoi64(DataPtr);
	}

	wchar_t *GetBuffer()
	{
		return Buffer;
	}

	const wchar_t *GetMask() const
	{
		return Mask;
	}
};

class PluginListControlBinding : public DialogAPIBinding
{
private:
	int *SelectedIndex;
	wchar_t *TextBuf;
	FarList *List;
	
public:
	PluginListControlBinding(const PluginStartupInfo &aInfo, HANDLE *aHandle, int aID, int *aValue, wchar_t *aText, FarList *aList)
		: DialogAPIBinding(aInfo, aHandle, aID), SelectedIndex(aValue), TextBuf(aText), List(aList)
	{
	}

	PluginListControlBinding(const PluginStartupInfo &aInfo, HANDLE *aHandle, int aID, int *aValue, FarList *aList)
		: DialogAPIBinding(aInfo, aHandle, aID), SelectedIndex(aValue), TextBuf(nullptr), List(aList)
	{
	}

	~PluginListControlBinding()
	{
		if (List)
		{
			delete[] List->Items;
		}
		delete List;
	}

	virtual void SaveValue(FarDialogItem *Item, int RadioGroupIndex)
	{
		if (SelectedIndex)
		{
			*SelectedIndex = static_cast<int>(Info.SendDlgMessage(*DialogHandle, DM_LISTGETCURPOS, ID, 0));
		}
		if (TextBuf)
		{
			FarDialogItemData fdid = {sizeof(FarDialogItemData), 0, TextBuf};
			Info.SendDlgMessage(*DialogHandle, DM_GETTEXT, ID, &fdid);
		}
	}
};

/*
Версия класса для динамического построения диалогов, используемая в плагинах к Far.
*/
class PluginDialogBuilder: public DialogBuilderBase<FarDialogItem>
{
	protected:
		const PluginStartupInfo &Info;
		HANDLE DialogHandle;
		const wchar_t *HelpTopic;
		GUID PluginId;
		GUID Id;
		FARWINDOWPROC DlgProc;
		void* UserParam;
		FARDIALOGFLAGS Flags;

		virtual void InitDialogItem(FarDialogItem *Item, const wchar_t *Text)
		{
			memset(Item, 0, sizeof(FarDialogItem));
			Item->Data = Text;
		}

		virtual int TextWidth(const FarDialogItem &Item)
		{
			return lstrlenW(Item.Data);
		}

		virtual const wchar_t *GetLangString(int MessageID)
		{
			return Info.GetMsg(&PluginId, MessageID);
		}

		virtual intptr_t DoShowDialog()
		{
			intptr_t Width = m_DialogItems[0].X2+4;
			intptr_t Height = m_DialogItems[0].Y2+2;
			DialogHandle = Info.DialogInit(&PluginId, &Id, -1, -1, Width, Height, HelpTopic, m_DialogItems, m_DialogItemsCount, 0, Flags, DlgProc, UserParam);
			return Info.DialogRun(DialogHandle);
		}

		virtual DialogItemBinding<FarDialogItem> *CreateCheckBoxBinding(int *Value, int Mask)
		{
			return new PluginCheckBoxBinding(Info, &DialogHandle, m_DialogItemsCount-1, Value, Mask);
		}

		virtual DialogItemBinding<FarDialogItem> *CreateRadioButtonBinding(BOOL *Value)
		{
			return new PluginRadioButtonBinding(Info, &DialogHandle, m_DialogItemsCount-1, Value);
		}

		FarDialogItem *AddListControl(FARDIALOGITEMTYPES Type, int *SelectedItem, wchar_t *Text, int Width, int Height, const wchar_t* ItemsText [], size_t ItemCount, FARDIALOGITEMFLAGS ItemFlags)
		{
			FarDialogItem *Item = AddDialogItem(Type, Text ? Text : L"");
			SetNextY(Item);
			Item->X2 = Item->X1 + Width;
			Item->Y2 = Item->Y2 + Height;
			Item->Flags |= ItemFlags;

			m_NextY += Height;

			FarListItem *ListItems = nullptr;
			if (ItemsText)
			{
				ListItems = new FarListItem[ItemCount];
				for(size_t i=0; i<ItemCount; i++)
				{
					ListItems[i].Text = ItemsText[i];
					ListItems[i].Flags = SelectedItem && (*SelectedItem == static_cast<int>(i)) ? LIF_SELECTED : 0;
				}
			}
			FarList *List = new FarList;
			List->StructSize = sizeof(FarList);
			List->Items = ListItems;
			List->ItemsNumber = ListItems ? ItemCount : 0;
			Item->ListItems = List;

			SetLastItemBinding(new PluginListControlBinding(Info, &DialogHandle, m_DialogItemsCount - 1, SelectedItem, Text, List));
			return Item;
		}

		FarDialogItem *AddListControl(FARDIALOGITEMTYPES Type, int *SelectedItem, wchar_t *Text, int Width, int Height, const int MessageIDs [], size_t ItemCount, FARDIALOGITEMFLAGS ItemFlags)
		{
			const wchar_t** ItemsText = nullptr;
			if (MessageIDs)
			{
				ItemsText = new const wchar_t*[ItemCount];
				for (size_t i = 0; i < ItemCount; i++)
				{
					ItemsText[i] = GetLangString(MessageIDs[i]);
				}
			}

			FarDialogItem* result = AddListControl(Type, SelectedItem, Text, Width, Height, ItemsText, ItemCount, ItemFlags);

			delete [] ItemsText;

			return result;
		}

public:
		PluginDialogBuilder(const PluginStartupInfo &aInfo, const GUID &aPluginId, const GUID &aId, int TitleMessageID, const wchar_t *aHelpTopic, FARWINDOWPROC aDlgProc=nullptr, void* aUserParam=nullptr, FARDIALOGFLAGS aFlags = FDLG_NONE)
			: Info(aInfo), DialogHandle(0), HelpTopic(aHelpTopic), PluginId(aPluginId), Id(aId), DlgProc(aDlgProc), UserParam(aUserParam), Flags(aFlags)
		{
			AddBorder(GetLangString(TitleMessageID));
		}

		PluginDialogBuilder(const PluginStartupInfo &aInfo, const GUID &aPluginId, const GUID &aId, const wchar_t *TitleMessage, const wchar_t *aHelpTopic, FARWINDOWPROC aDlgProc=nullptr, void* aUserParam=nullptr, FARDIALOGFLAGS aFlags = FDLG_NONE)
			: Info(aInfo), DialogHandle(0), HelpTopic(aHelpTopic), PluginId(aPluginId), Id(aId), DlgProc(aDlgProc), UserParam(aUserParam), Flags(aFlags)
		{
			AddBorder(TitleMessage);
		}

		~PluginDialogBuilder()
		{
			Info.DialogFree(DialogHandle);
		}

		virtual FarDialogItem *AddIntEditField(int *Value, int Width)
		{
			FarDialogItem *Item = AddDialogItem(DI_FIXEDIT, L"");
			Item->Flags |= DIF_MASKEDIT;
			PluginIntEditFieldBinding *Binding;
			Binding = new PluginIntEditFieldBinding(Info, &DialogHandle, m_DialogItemsCount-1, Value, Width);
			Item->Data = Binding->GetBuffer();
			Item->Mask = Binding->GetMask();
			SetNextY(Item);
			Item->X2 = Item->X1 + Width - 1;
			SetLastItemBinding(Binding);
			return Item;
		}

		virtual FarDialogItem *AddUIntEditField(unsigned int *Value, int Width)
		{
			FarDialogItem *Item = AddDialogItem(DI_FIXEDIT, L"");
			Item->Flags |= DIF_MASKEDIT;
			PluginUIntEditFieldBinding *Binding;
			Binding = new PluginUIntEditFieldBinding(Info, &DialogHandle, m_DialogItemsCount-1, Value, Width);
			Item->Data = Binding->GetBuffer();
			Item->Mask = Binding->GetMask();
			SetNextY(Item);
			Item->X2 = Item->X1 + Width - 1;
			SetLastItemBinding(Binding);
			return Item;
		}

		FarDialogItem *AddEditField(wchar_t *Value, int MaxSize, int Width, const wchar_t *HistoryID = nullptr, bool UseLastHistory = false)
		{
			FarDialogItem *Item = AddDialogItem(DI_EDIT, Value);
			SetNextY(Item);
			Item->X2 = Item->X1 + Width - 1;
			if (HistoryID)
			{
				Item->History = HistoryID;
				Item->Flags |= DIF_HISTORY;
				if (UseLastHistory)
					Item->Flags |= DIF_USELASTHISTORY;
			}

			SetLastItemBinding(new PluginEditFieldBinding(Info, &DialogHandle, m_DialogItemsCount-1, Value, MaxSize));
			return Item;
		}

		FarDialogItem *AddPasswordField(wchar_t *Value, int MaxSize, int Width)
		{
			FarDialogItem *Item = AddDialogItem(DI_PSWEDIT, Value);
			SetNextY(Item);
			Item->X2 = Item->X1 + Width - 1;

			SetLastItemBinding(new PluginEditFieldBinding(Info, &DialogHandle, m_DialogItemsCount-1, Value, MaxSize));
			return Item;
		}

		FarDialogItem *AddFixEditField(wchar_t *Value, int MaxSize, int Width, const wchar_t *Mask = nullptr)
		{
			FarDialogItem *Item = AddDialogItem(DI_FIXEDIT, Value);
			SetNextY(Item);
			Item->X2 = Item->X1 + Width - 1;
			if (Mask)
			{
				Item->Mask = Mask;
				Item->Flags |= DIF_MASKEDIT;
			}

			SetLastItemBinding(new PluginEditFieldBinding(Info, &DialogHandle, m_DialogItemsCount-1, Value, MaxSize));
			return Item;
		}

		FarDialogItem *AddComboBox(int *SelectedItem, wchar_t *Text, int Width, const wchar_t* ItemsText[], size_t ItemCount, FARDIALOGITEMFLAGS ItemFlags)
		{
			return AddListControl(DI_COMBOBOX, SelectedItem, Text, Width, 0, ItemsText, ItemCount, ItemFlags);
		}

		FarDialogItem *AddComboBox(int *SelectedItem, wchar_t *Text,  int Width, const int MessageIDs[], size_t ItemCount, FARDIALOGITEMFLAGS ItemFlags)
		{
			return AddListControl(DI_COMBOBOX, SelectedItem, Text, Width, 0, MessageIDs, ItemCount, ItemFlags);
		}

		FarDialogItem *AddListBox(int *SelectedItem, int Width, int Height, const wchar_t* ItemsText[], size_t ItemCount, FARDIALOGITEMFLAGS ItemFlags)
		{
			return AddListControl(DI_LISTBOX, SelectedItem, nullptr, Width, Height, ItemsText, ItemCount, ItemFlags);
		}

		FarDialogItem *AddListBox(int *SelectedItem, int Width, int Height, const int MessageIDs[], size_t ItemCount, FARDIALOGITEMFLAGS ItemFlags)
		{
			return AddListControl(DI_LISTBOX, SelectedItem, nullptr, Width, Height, MessageIDs, ItemCount, ItemFlags);
		}
};
#endif /* __DLGBUILDER_HPP__ */
