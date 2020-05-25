#ifndef FARDLGBUILDER_HPP_4AD5C50D_B9AC_49DE_B34B_BAD22219BCBD
#define FARDLGBUILDER_HPP_4AD5C50D_B9AC_49DE_B34B_BAD22219BCBD
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

// Internal:
#include "DlgBuilder.hpp"
#include "dialog.hpp"

// Platform:

// Common:
#include "common/range.hpp"

// External:

//----------------------------------------------------------------------------

enum class lng: int;
struct DialogItemEx;
class BoolOption;
class Bool3Option;
class IntOption;
class StringOption;

// Элемент выпадающего списка в диалоге.
class FarDialogBuilderListItem
{
public:
	FarDialogBuilderListItem(lng const MessageId, int const Value, LISTITEMFLAGS Flags = LIF_NONE):
		m_Str(MessageId),
		m_Value(Value),
		m_Flags(Flags)
	{
	}

	FarDialogBuilderListItem(string_view Str, int const Value, LISTITEMFLAGS Flags = LIF_NONE):
		m_Str(string(Str)),
		m_Value(Value),
		m_Flags(Flags)
	{
	}

	std::variant<string, lng> m_Str;
	int m_Value;
	LISTITEMFLAGS m_Flags;

	const string& str() const;
	auto value() const { return m_Value; }
	auto flags() const { return m_Flags; }
};

template<class T, typename value_type>
struct ListControlBinding: public DialogItemBinding<T>
{
	NONCOPYABLE(ListControlBinding);

	value_type& m_Value;
	std::vector<FarListItem> m_ListItems;
	FarList m_List{sizeof(m_List)};

	ListControlBinding(value_type& Value, std::vector<FarListItem>&& ListItems):
		m_Value(Value),
		m_ListItems(std::move(ListItems))
	{
		m_List.Items = m_ListItems.data();
		m_List.ItemsNumber = m_ListItems.size();
	}

	auto list()
	{
		return &m_List;
	}

	void SaveValue(T *Item, int RadioGroupIndex) override
	{
		if (!m_ListItems.empty())
		{
			m_Value = m_ListItems[Item->ListPos].UserData;
		}
	}
};

/*
Класс для динамического построения диалогов, используемый внутри кода FAR.
Использует FAR'овский класс string для работы с текстовыми полями.

Для того, чтобы сместить элемент относительно дефолтного
положения по горизонтали, можно использовать метод DialogItemEx::Indent().

Поддерживает automation (изменение флагов одного элемента в зависимости от состояния
другого). Реализуется при помощи метода LinkFlags().
*/
class DialogBuilder: noncopyable, public base<DialogBuilderBase<DialogItemEx>>
{
public:
	explicit DialogBuilder(lng TitleMessageId, string_view HelpTopic = {}, Dialog::dialog_handler handler = nullptr);
	DialogBuilder();
	~DialogBuilder() override;

	// Добавляет поле типа DI_FIXEDIT для редактирования указанного числового значения.
	DialogItemEx *AddIntEditField(int *Value, int Width) override;
	virtual DialogItemEx *AddIntEditField(IntOption& Value, int Width);
	virtual DialogItemEx *AddHexEditField(IntOption& Value, int Width);

	// Добавляет поле типа DI_EDIT для редактирования указанного строкового значения.
	DialogItemEx *AddEditField(string& Value, int Width, string_view HistoryID = {}, FARDIALOGITEMFLAGS Flags = 0);
	DialogItemEx *AddEditField(StringOption& Value, int Width, string_view HistoryID = {}, FARDIALOGITEMFLAGS Flags = 0);

	// Добавляет поле типа DI_FIXEDIT для редактирования указанного строкового значения.
	DialogItemEx *AddFixEditField(string& Value, int Width, const wchar_t *Mask = nullptr);
	DialogItemEx *AddFixEditField(StringOption& Value, int Width, const wchar_t *Mask = nullptr);

	// Добавляет неизменяемое поле типа DI_EDIT для посмотра указанного строкового значения.
	DialogItemEx *AddConstEditField(const string& Value, int Width, FARDIALOGITEMFLAGS Flags = 0);

	// Добавляет выпадающий список с указанными значениями.
	DialogItemEx *AddComboBox(int& Value, int Width, span<FarDialogBuilderListItem const> Items, FARDIALOGITEMFLAGS Flags = DIF_NONE);
	DialogItemEx *AddComboBox(IntOption& Value, int Width, span<FarDialogBuilderListItem const> Items, FARDIALOGITEMFLAGS Flags = DIF_NONE);

	DialogItemEx *AddListBox(int& Value, int Width, int Height, span<FarDialogBuilderListItem const> Items, FARDIALOGITEMFLAGS Flags = DIF_NONE);
	DialogItemEx *AddListBox(IntOption& Value, int Width, int Height, span<FarDialogBuilderListItem const> Items, FARDIALOGITEMFLAGS Flags = DIF_NONE);

	decltype(auto) AddCheckbox(lng TextMessageId, int *Value, int Mask = 0, bool ThreeState = false)
	{
		return base_type::AddCheckbox(static_cast<int>(TextMessageId), Value, Mask, ThreeState);
	}
	DialogItemEx *AddCheckbox(lng TextMessageId, IntOption& Value, int Mask=0, bool ThreeState=false);
	DialogItemEx *AddCheckbox(lng TextMessageId, Bool3Option& Value);
	DialogItemEx *AddCheckbox(lng TextMessageId, BoolOption& Value);
	DialogItemEx *AddCheckbox(const wchar_t* Caption, BoolOption& Value);

	void AddRadioButtons(IntOption& Value, int OptionCount, const lng MessageIDs[], bool FocusOnSelected=false);

	using base_type::AddText;

	decltype(auto) AddText(lng LabelId)
	{
		return base_type::AddText(static_cast<int>(LabelId));
	}

	// BUGBUG
	decltype(auto) AddText(const string& Label)
	{
		return base_type::AddText(Label.c_str());
	}

	decltype(auto) AddTextBefore(DialogItemEx* RelativeTo, lng LabelId)
	{
		return base_type::AddTextBefore(RelativeTo, static_cast<int>(LabelId));
	}

	using base_type::AddTextAfter;

	decltype(auto) AddTextAfter(DialogItemEx* RelativeTo, lng LabelId, int skip = 1)
	{
		return base_type::AddTextAfter(RelativeTo, static_cast<int>(LabelId), skip);
	}

	using base_type::AddSeparator;

	decltype(auto) AddSeparator(lng LabelId)
	{
		return base_type::AddSeparator(static_cast<int>(LabelId));
	}

	// Связывает состояние элементов Parent и Target. Когда Parent->Selected равно
	// false, устанавливает флаги Flags у элемента Target; когда равно true -
	// сбрасывает флаги.
	// Если LinkLabels установлено в true, то текстовые элементы, добавленные к элементу Target
	// методами AddTextBefore и AddTextAfter, также связываются с элементом Parent.
	void LinkFlags(DialogItemEx *Parent, DialogItemEx *Target, FARDIALOGITEMFLAGS Flags, bool LinkLabels=true);

	void AddOKCancel();
	void AddOKCancel(lng OKMessageId, lng CancelMessageId);
	void AddButtons(span<const lng> Buttons, size_t OkIndex, size_t CancelIndex);
	void AddOK();

	void SetDialogMode(DWORD Flags);

	int AddTextWrap(const wchar_t *text, bool center=false, int width=0);

	void SetId(const GUID& Id);
	const GUID& GetId() const {return m_Id;}

protected:
	void InitDialogItem(DialogItemEx *Item, const wchar_t* Text) override;
	int TextWidth(const DialogItemEx &Item) override;
	intptr_t DoShowDialog() override;
	DialogItemBinding<DialogItemEx> *CreateCheckBoxBinding(BOOL* Value, int Mask) override;
	DialogItemBinding<DialogItemEx> *CreateRadioButtonBinding(int *Value) override;

	DialogItemBinding<DialogItemEx> *CreateCheckBoxBinding(IntOption &Value, int Mask);
	DialogItemBinding<DialogItemEx> *CreateCheckBoxBinding(Bool3Option& Value);
	DialogItemBinding<DialogItemEx> *CreateCheckBoxBinding(BoolOption& Value);
	DialogItemBinding<DialogItemEx> *CreateRadioButtonBinding(IntOption& Value);

	DialogItemEx *AddListControl(FARDIALOGITEMTYPES Type, int& Value, int Width, int Height, span<FarDialogBuilderListItem const> Items, FARDIALOGITEMFLAGS Flags = DIF_NONE);
	DialogItemEx *AddListControl(FARDIALOGITEMTYPES Type, IntOption& Value, int Width, int Height, span<FarDialogBuilderListItem const> Items, FARDIALOGITEMFLAGS Flags = DIF_NONE);

	const wchar_t* GetLangString(lng MessageID);

private:
	const wchar_t* GetLangString(int MessageID) override;
	static void LinkFlagsByID(DialogItemEx *Parent, DialogItemEx* Target, FARDIALOGITEMFLAGS Flags);

	template<typename value_type>
	DialogItemEx* AddListControlImpl(FARDIALOGITEMTYPES Type, value_type& Value, int Width, int Height, span<FarDialogBuilderListItem const> Items, FARDIALOGITEMFLAGS Flags);

	string m_HelpTopic;
	DWORD m_Mode{};
	GUID m_Id{ GUID_NULL };
	bool m_IdExist{};
	Dialog::dialog_handler m_handler;
};

#endif // FARDLGBUILDER_HPP_4AD5C50D_B9AC_49DE_B34B_BAD22219BCBD
