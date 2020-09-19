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
#include "plugin.hpp"
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
struct DialogItemBinding;

class lng_string
{
public:
	lng_string(lng Str);
	lng_string(const wchar_t* Str);
	lng_string(const string& Str);

	const wchar_t* c_str() const;

private:
	const wchar_t* m_Str;
};

class DialogBuilderListItem
{
public:
	DialogBuilderListItem(lng const MessageId, int const Value, LISTITEMFLAGS Flags = LIF_NONE):
		m_Str(MessageId),
		m_Value(Value),
		m_Flags(Flags)
	{
	}

	DialogBuilderListItem(string_view Str, int const Value, LISTITEMFLAGS Flags = LIF_NONE):
		m_Str(string(Str)),
		m_Value(Value),
		m_Flags(Flags)
	{
	}

	const string& str() const;
	auto value() const { return m_Value; }
	auto flags() const { return m_Flags; }

//BUGBUG
//private:
	std::variant<string, lng> m_Str;
	int m_Value;
	LISTITEMFLAGS m_Flags;
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

Для того, чтобы сместить элемент относительно дефолтного
положения по горизонтали, можно использовать метод DialogItemEx::Indent().

Поддерживает automation (изменение флагов одного элемента в зависимости от состояния
другого). Реализуется при помощи метода LinkFlags().
*/

class DialogBuilder
{
public:
	NONCOPYABLE(DialogBuilder);

	explicit DialogBuilder(lng_string Title = L"", string_view HelpTopic = {}, Dialog::dialog_handler handler = nullptr);
	~DialogBuilder();

	DialogItemEx* AddText(lng_string Text);
	DialogItemEx* AddCheckbox(lng_string Text, int& Value, int Mask = 0, bool ThreeState = false);
	DialogItemEx* AddCheckbox(lng_string Text, IntOption& Value, int Mask = 0, bool ThreeState = false);
	DialogItemEx* AddCheckbox(lng_string Text, Bool3Option& Value);
	DialogItemEx* AddCheckbox(lng_string Text, BoolOption& Value);
	DialogItemEx* AddTextBefore(DialogItemEx* RelativeTo, lng_string Text);
	DialogItemEx* AddTextAfter(DialogItemEx* RelativeTo, lng_string Text, int skip = 1);
	DialogItemEx* AddButtonAfter(DialogItemEx* RelativeTo, lng_string Text);
	DialogItemEx* AddIntEditField(IntOption& Value, int Width);
	DialogItemEx* AddHexEditField(IntOption& Value, int Width);
	DialogItemEx* AddEditField(string& Value, int Width, string_view HistoryID = {}, FARDIALOGITEMFLAGS Flags = 0);
	DialogItemEx* AddEditField(StringOption& Value, int Width, string_view HistoryID = {}, FARDIALOGITEMFLAGS Flags = 0);
	DialogItemEx* AddFixEditField(string& Value, int Width, const wchar_t* Mask = nullptr);
	DialogItemEx* AddFixEditField(StringOption& Value, int Width, const wchar_t* Mask = nullptr);
	DialogItemEx* AddConstEditField(const string& Value, int Width, FARDIALOGITEMFLAGS Flags = 0);
	DialogItemEx* AddComboBox(int& Value, int Width, span<DialogBuilderListItem const> Items, FARDIALOGITEMFLAGS Flags = DIF_NONE);
	DialogItemEx* AddComboBox(IntOption& Value, int Width, span<DialogBuilderListItem const> Items, FARDIALOGITEMFLAGS Flags = DIF_NONE);
	DialogItemEx* AddListBox(int& Value, int Width, int Height, span<DialogBuilderListItem const> Items, FARDIALOGITEMFLAGS Flags = DIF_NONE);
	DialogItemEx* AddListBox(IntOption& Value, int Width, int Height, span<DialogBuilderListItem const> Items, FARDIALOGITEMFLAGS Flags = DIF_NONE);
	void AddRadioButtons(size_t& Value, span<lng const> Options, bool FocusOnSelected = false);
	void AddRadioButtons(IntOption& Value, span<lng const> Options, bool FocusOnSelected = false);
	void LinkFlags(DialogItemEx* Parent, DialogItemEx* Target, FARDIALOGITEMFLAGS Flags, bool LinkLabels = true);
	void AddOK();
	void AddOKCancel();
	void AddOKCancel(lng OKMessageId, lng CancelMessageId);
	void AddButtons(span<lng const> Buttons, size_t OkIndex, size_t CancelIndex);
	void SetDialogMode(DWORD Flags);
	int AddTextWrap(lng_string Text, bool center = false, int width = 0);
	void SetId(const UUID& Id);
	const UUID& GetId() const;
	size_t GetLastID() const;
	void StartColumns();
	void ColumnBreak();
	void EndColumns();
	void StartSingleBox(lng_string Text, bool LeftAlign = false);
	void EndSingleBox();
	void AddEmptyLine();
	void AddSeparator(lng_string Text = L"");
	intptr_t ShowDialogEx();
	bool ShowDialog();

private:
	DialogItemEx* AddDialogItem(FARDIALOGITEMTYPES Type, const wchar_t* Text);
	void SetNextY(DialogItemEx* Item);
	void AddBorder(const wchar_t* TitleText);
	void UpdateBorderSize();
	intptr_t MaxTextWidth();
	void UpdateSecondColumnPosition();
	void SetLastItemBinding(std::unique_ptr<DialogItemBinding>&& Binding);
	int GetItemID(DialogItemEx* Item) const;
	DialogItemBinding* FindBinding(DialogItemEx* Item);
	void SaveValues();
	intptr_t DoShowDialog();

	template<typename value_type>
	DialogItemEx* AddCheckboxImpl(lng_string Text, value_type& Value, int Mask, bool ThreeState);

	template<typename value_type>
	DialogItemEx* AddListControlImpl(FARDIALOGITEMTYPES Type, value_type& Value, int Width, int Height, span<DialogBuilderListItem const> Items, FARDIALOGITEMFLAGS Flags);

	static const int SECOND_COLUMN = -2;
	static constexpr size_t npos = -1;

	std::vector<DialogItemEx> m_DialogItems;
	std::vector<std::unique_ptr<DialogItemBinding>> m_Bindings;
	int m_NextY{2};
	int m_Indent{};
	size_t m_SingleBoxIndex{ npos };
	size_t m_FirstButtonID{ npos };
	size_t m_CancelButtonID{ npos };
	size_t m_ColumnStartIndex{ npos };
	size_t m_ColumnBreakIndex{ npos };
	int m_ColumnStartY{-1};
	int m_ColumnEndY{-1};
	intptr_t m_ColumnMinWidth{};
	string m_HelpTopic;
	DWORD m_Mode{};
	UUID m_Id{};
	bool m_IdExist{};
	Dialog::dialog_handler m_handler;
};

#endif // FARDLGBUILDER_HPP_4AD5C50D_B9AC_49DE_B34B_BAD22219BCBD
