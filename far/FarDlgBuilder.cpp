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

// BUGBUG
#include "platform.headers.hpp"

// Self:
#include "FarDlgBuilder.hpp"

// Internal:
#include "lang.hpp"
#include "dialog.hpp"
#include "interf.hpp"
#include "strmix.hpp"
#include "config.hpp"
#include "log.hpp"

// Platform:

// Common:
#include "common/from_string.hpp"
#include "common/view/zip.hpp"

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

struct DialogItemBinding
{
	virtual ~DialogItemBinding() = default;
	virtual void SaveValue(DialogItemEx const& Item, int RadioGroupIndex) = 0;

	int BeforeLabelID{-1};
	int AfterLabelID{-1};
};

template<class T>
class EditFieldBinding final: public DialogItemBinding
{
public:
	explicit EditFieldBinding(T& TextValue):
		m_TextValue(TextValue)
	{
	}

	void SaveValue(DialogItemEx const& Item, int const RadioGroupIndex) override
	{
		m_TextValue = Item.strData;
	}

private:
	T& m_TextValue;
};

class EditFieldIntBinding final: public DialogItemBinding
{
public:
	EditFieldIntBinding(IntOption* IntValue, int Width):
		m_IntValue(IntValue)
	{
		m_Mask[0] = L'#';
		const auto MaskWidth = std::min(static_cast<int>(std::size(m_Mask) - 1), Width);
		std::fill(m_Mask + 1, m_Mask + MaskWidth, L'9');
		m_Mask[MaskWidth] = {};
	}

	void SaveValue(DialogItemEx const& Item, int const RadioGroupIndex) override
	{
		{
			long long Value;
			if (from_string(Item.strData, Value))
			{
				*m_IntValue = Value;
				return;
			}
		}

		{
			unsigned long long Value;
			if (from_string(Item.strData, Value))
			{
				*m_IntValue = Value;
				return;
			}
		}

		LOGWARNING(L"Invalid integer value {}"sv, Item.strData);
	}

	string_view GetMask() const
	{
		return { m_Mask, std::size(m_Mask) - 1 };
	}

private:
	IntOption* m_IntValue;
	wchar_t m_Mask[32];
};

class EditFieldHexBinding final: public DialogItemBinding
{
public:
	explicit EditFieldHexBinding(IntOption* IntValue):
		m_IntValue(IntValue)
	{
		m_Mask[0] = L'0';
		m_Mask[1] = L'x';
		std::fill(std::begin(m_Mask) + 2, std::end(m_Mask) - 1, L'H');
		*(std::end(m_Mask) - 1) = {};
	}

	void SaveValue(DialogItemEx const& Item, int const RadioGroupIndex) override
	{
		unsigned long long Value;
		if (from_string(Item.strData, Value, {}, 16))
		{
			*m_IntValue = Value;
			return;
		}

		LOGWARNING(L"Invalid integer value {}"sv, Item.strData);
	}

	string_view GetMask() const
	{
		return { m_Mask, std::size(m_Mask) - 1 };
	}

private:
	IntOption* m_IntValue;
	wchar_t m_Mask[2 + sizeof(long long) * 2 + 1];
};

class EditFieldBinaryBinding final: public DialogItemBinding
{
public:
	explicit EditFieldBinaryBinding(IntOption* IntValue):
		m_IntValue(IntValue)
	{
		std::fill(std::begin(m_Mask), std::end(m_Mask) - 1, L'\1');
		*(std::end(m_Mask) - 1) = {};
	}

	void SaveValue(DialogItemEx const& Item, int const RadioGroupIndex) override
	{
		// Must be converted to unsigned type first regardless of underlying type
		*m_IntValue = from_string<unsigned long long>(Item.strData, nullptr, 2);
	}

	string_view GetMask() const
	{
		return { m_Mask, std::size(m_Mask) - 1 };
	}

private:
	IntOption* m_IntValue;
	wchar_t m_Mask[sizeof(long long) * 8 + 1];
};

template<typename int_type>
class CheckBoxIntBinding final: public DialogItemBinding
{
public:
	explicit CheckBoxIntBinding(int_type& Value, unsigned Mask):
		m_Value(Value),
		m_Mask(Mask)
	{
	}

	void SaveValue(DialogItemEx const& Item, int const RadioGroupIndex) override
	{
		if (!m_Mask)
		{
			m_Value = Item.Selected;
		}
		else
		{
			if (Item.Selected)
				m_Value |= m_Mask;
			else
				m_Value &= ~m_Mask;
		}
	}

private:
	int_type& m_Value;
	int m_Mask;
};

template<class T>
class CheckBoxBool3Binding final: public DialogItemBinding
{
public:
	explicit CheckBoxBool3Binding(T& Value):
		m_Value(Value)
	{
	}

	void SaveValue(DialogItemEx const& Item, int const RadioGroupIndex) override
	{
		m_Value = Item.Selected;
	}

private:
	T& m_Value;
};

template<class T>
class CheckBoxBoolBinding final: public DialogItemBinding
{
public:
	explicit CheckBoxBoolBinding(T& Value):
		m_Value(Value)
	{
	}

	void SaveValue(DialogItemEx const& Item, int const RadioGroupIndex) override
	{
		m_Value = Item.Selected != BSTATE_UNCHECKED;
	}

private:
	T& m_Value;
};

template<class T>
class RadioButtonBinding final: public DialogItemBinding
{
public:
	explicit RadioButtonBinding(T* Value):
		m_Value(Value)
	{
	}

	void SaveValue(DialogItemEx const& Item, int const RadioGroupIndex) override
	{
		if (Item.Selected)
			*m_Value = RadioGroupIndex;
	}

private:
	T* m_Value;
};

lng_string::lng_string(lng Str):
	m_Str(Str == lng{-1}? L"" : msg(Str).c_str())
{
}

lng_string::lng_string(const wchar_t* Str):
	m_Str(Str)
{
}

lng_string::lng_string(const string& Str):
	m_Str(Str.c_str())
{
}

const wchar_t* lng_string::c_str() const
{
	return m_Str;
}

const string& DialogBuilderListItem::str() const
{
	return std::visit(overload
	{
		[](lng Id) -> const string&
		{
			return msg(Id);
		},
		[](string const& Str) -> const string&
		{
			return Str;
		}
	},
	m_Str);
}

static int TextWidth(const DialogItemEx& Item)
{
	return static_cast<int>(Item.strData.size());
}

static intptr_t ItemWidth(const DialogItemEx& Item)
{
	switch (Item.Type)
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
		return Item.X2 - Item.X1 + 1;

	default:
		break;
	}
	return 0;
}

DialogBuilder::DialogBuilder(lng_string const Title, const string_view HelpTopic, Dialog::dialog_handler handler):
	m_HelpTopic(HelpTopic),
	m_handler(std::move(handler))
{
	constexpr size_t MinSize = 128;

	m_DialogItems.reserve(MinSize);
	m_Bindings.reserve(MinSize);

	AddBorder(Title.c_str());
}

DialogBuilder::~DialogBuilder() = default;

DialogItemEx& DialogBuilder::AddText(lng_string const Text)
{
	auto& Item = AddDialogItem(DI_TEXT, Text.c_str());
	SetNextY(Item);
	return Item;
}

DialogItemEx& DialogBuilder::AddCheckbox(lng_string const Text, int& Value, unsigned Mask, bool ThreeState)
{
	auto& Item = AddCheckboxImpl(Text, Value, Mask, ThreeState);
	SetLastItemBinding(std::make_unique<CheckBoxIntBinding<int>>(Value, Mask));
	return Item;
}

DialogItemEx& DialogBuilder::AddCheckbox(lng_string const Text, IntOption& Value, unsigned Mask, bool ThreeState)
{
	auto& Item = AddCheckboxImpl(Text, Value, Mask, ThreeState);
	SetLastItemBinding(std::make_unique<CheckBoxIntBinding<IntOption>>(Value, Mask));
	return Item;
}

DialogItemEx& DialogBuilder::AddCheckbox(lng_string const Text, Bool3Option& Value)
{
	auto& Item = AddCheckboxImpl(Text, Value, 0, true);
	SetLastItemBinding(std::make_unique<CheckBoxBool3Binding<Bool3Option>>(Value));
	return Item;
}

DialogItemEx& DialogBuilder::AddCheckbox(lng_string const Text, BoolOption& Value)
{
	auto& Item = AddCheckboxImpl(Text, Value, 0, false);
	SetLastItemBinding(std::make_unique<CheckBoxBoolBinding<BoolOption>>(Value));
	return Item;
}

DialogItemEx& DialogBuilder::AddTextBefore(DialogItemEx& RelativeTo, lng_string const Text)
{
	auto& Item = AddDialogItem(DI_TEXT, Text.c_str());
	Item.Y1 = Item.Y2 = RelativeTo.Y1;
	Item.X1 = 5 + m_Indent;
	Item.X2 = Item.X1 + ItemWidth(Item) - 1;

	const auto RelativeToWidth = RelativeTo.X2 - RelativeTo.X1;
	RelativeTo.X1 = Item.X2 + 2;
	RelativeTo.X2 = RelativeTo.X1 + RelativeToWidth;

	FindBinding(RelativeTo).BeforeLabelID = GetItemID(Item);

	return Item;
}

DialogItemEx& DialogBuilder::AddTextAfter(DialogItemEx const& RelativeTo, lng_string const Text, int skip)
{
	auto& Item = AddDialogItem(DI_TEXT, Text.c_str());
	Item.Y1 = Item.Y2 = RelativeTo.Y1;
	Item.X1 = RelativeTo.X1 + ItemWidth(RelativeTo) + skip;

	FindBinding(RelativeTo).AfterLabelID = GetItemID(Item);

	return Item;
}

DialogItemEx& DialogBuilder::AddButtonAfter(DialogItemEx const& RelativeTo, lng_string const Text)
{
	auto& Item = AddDialogItem(DI_BUTTON, Text.c_str());
	Item.Y1 = Item.Y2 = RelativeTo.Y1;
	Item.X1 = RelativeTo.X1 + ItemWidth(RelativeTo) - 1 + 2;

	FindBinding(RelativeTo).AfterLabelID = GetItemID(Item);

	return Item;
}

DialogItemEx& DialogBuilder::AddIntEditField(IntOption& Value, int Width)
{
	auto& Item = AddDialogItem(DI_FIXEDIT, L"");
	Item.strData = str(Value.Get());
	SetNextY(Item);
	Item.X2 = Item.X1 + Width - 1;

	auto Binding = std::make_unique<EditFieldIntBinding>(&Value, Width);
	Item.Flags |= DIF_MASKEDIT;
	Item.strMask = Binding->GetMask();
	SetLastItemBinding(std::move(Binding));
	return Item;
}

DialogItemEx& DialogBuilder::AddHexEditField(IntOption& Value, int Width)
{
	auto& Item = AddDialogItem(DI_FIXEDIT, L"");
	Item.strData = far::format(L"{:016X}"sv, as_unsigned(Value.Get()));
	SetNextY(Item);
	Item.X2 = Item.X1 + Width - 1;

	auto Binding = std::make_unique<EditFieldHexBinding>(&Value);
	Item.Flags |= DIF_MASKEDIT;
	Item.strMask = Binding->GetMask();
	SetLastItemBinding(std::move(Binding));
	return Item;
}

DialogItemEx& DialogBuilder::AddBinaryEditField(IntOption& Value, int Width)
{
	auto& Item = AddDialogItem(DI_FIXEDIT, L"");
	Item.strData = far::format(L"{0:064b}", as_unsigned(Value.Get()));
	SetNextY(Item);
	Item.X2 = Item.X1 + Width - 1;

	auto Binding = std::make_unique<EditFieldBinaryBinding>(&Value);
	Item.Flags |= DIF_MASKEDIT;
	Item.strMask = Binding->GetMask();
	SetLastItemBinding(std::move(Binding));
	return Item;
}

DialogItemEx& DialogBuilder::AddEditField(string& Value, int Width, string_view const HistoryID, FARDIALOGITEMFLAGS Flags)
{
	auto& Item = AddDialogItem(DI_EDIT, Value.c_str());
	SetNextY(Item);
	Item.X2 = Item.X1 + Width;
	if (!HistoryID.empty())
	{
		Item.strHistory = HistoryID;
		Item.Flags |= DIF_HISTORY;
	}
	Item.Flags |= Flags;

	SetLastItemBinding(std::make_unique<EditFieldBinding<string>>(Value));
	return Item;
}

DialogItemEx& DialogBuilder::AddEditField(StringOption& Value, int Width, string_view const HistoryID, FARDIALOGITEMFLAGS Flags)
{
	auto& Item = AddDialogItem(DI_EDIT, Value.c_str());
	SetNextY(Item);
	Item.X2 = Item.X1 + Width;
	if (!HistoryID.empty())
	{
		Item.strHistory = HistoryID;
		Item.Flags |= DIF_HISTORY;
	}
	Item.Flags |= Flags;

	SetLastItemBinding(std::make_unique<EditFieldBinding<StringOption>>(Value));
	return Item;
}

DialogItemEx& DialogBuilder::AddFixEditField(string& Value, int Width, const wchar_t* Mask)
{
	auto& Item = AddDialogItem(DI_FIXEDIT, Value.c_str());
	SetNextY(Item);
	Item.X2 = Item.X1 + Width - 1;
	if (Mask)
	{
		Item.Mask = Mask;
		Item.Flags |= DIF_MASKEDIT;
	}

	SetLastItemBinding(std::make_unique<EditFieldBinding<string>>(Value));
	return Item;
}

DialogItemEx& DialogBuilder::AddFixEditField(StringOption& Value, int Width, const wchar_t* Mask)
{
	auto& Item = AddDialogItem(DI_FIXEDIT, Value.c_str());
	SetNextY(Item);
	Item.X2 = Item.X1 + Width - 1;
	if (Mask)
	{
		Item.Mask = Mask;
		Item.Flags |= DIF_MASKEDIT;
	}

	SetLastItemBinding(std::make_unique<EditFieldBinding<StringOption>>(Value));
	return Item;
}

DialogItemEx& DialogBuilder::AddConstEditField(const string& Value, int Width, FARDIALOGITEMFLAGS Flags)
{
	auto& Item = AddDialogItem(DI_EDIT, Value.c_str());
	SetNextY(Item);
	Item.X2 = Item.X1 + Width;
	Item.Flags |= Flags | DIF_READONLY;
	return Item;
}

DialogItemEx& DialogBuilder::AddComboBox(int& Value, int Width, std::span<DialogBuilderListItem const> const Items, FARDIALOGITEMFLAGS Flags)
{
	return AddListControlImpl(DI_COMBOBOX, Value, Width, 0, Items, Flags | DIF_DROPDOWNLIST | DIF_LISTAUTOHIGHLIGHT);
}

DialogItemEx& DialogBuilder::AddComboBox(IntOption& Value, int Width, std::span<DialogBuilderListItem const> const Items, FARDIALOGITEMFLAGS Flags)
{
	return AddListControlImpl(DI_COMBOBOX, Value, Width, 0, Items, Flags | DIF_DROPDOWNLIST | DIF_LISTAUTOHIGHLIGHT);
}

DialogItemEx& DialogBuilder::AddListBox(int& Value, int Width, int Height, std::span<DialogBuilderListItem const> const Items, FARDIALOGITEMFLAGS Flags)
{
	return AddListControlImpl(DI_LISTBOX, Value, Width, Height, Items, Flags | DIF_LISTWRAPMODE | DIF_LISTAUTOHIGHLIGHT);
}

DialogItemEx& DialogBuilder::AddListBox(IntOption& Value, int Width, int Height, std::span<DialogBuilderListItem const> const Items, FARDIALOGITEMFLAGS Flags)
{
	return AddListControlImpl(DI_LISTBOX, Value, Width, Height, Items, Flags | DIF_LISTWRAPMODE | DIF_LISTAUTOHIGHLIGHT);
}

void DialogBuilder::AddRadioButtons(size_t& Value, std::span<lng const> const Options, bool FocusOnSelected)
{
	for (const auto i: std::views::iota(0uz, Options.size()))
	{
		auto& Item = AddDialogItem(DI_RADIOBUTTON, msg(Options[i]).c_str());
		SetNextY(Item);
		Item.X2 = Item.X1 + ItemWidth(Item);
		if (!i)
		{
			Item.Flags |= DIF_GROUP;
		}

		if (Value == i)
		{
			Item.Selected = TRUE;
			if (FocusOnSelected)
				Item.Flags |= DIF_FOCUS;
		}
		SetLastItemBinding(std::make_unique<RadioButtonBinding<size_t>>(&Value));
	}
}

void DialogBuilder::AddRadioButtons(IntOption& Value, std::span<lng const> const Options, bool FocusOnSelected)
{
	for (const auto i: std::views::iota(0uz, Options.size()))
	{
		auto& Item = AddDialogItem(DI_RADIOBUTTON, msg(Options[i]).c_str());
		SetNextY(Item);
		Item.X2 = Item.X1 + ItemWidth(Item);
		if (!i)
			Item.Flags |= DIF_GROUP;
		if (static_cast<size_t>(Value) == i)
		{
			Item.Selected = TRUE;
			if (FocusOnSelected)
				Item.Flags |= DIF_FOCUS;
		}
		SetLastItemBinding(std::make_unique<RadioButtonBinding<IntOption>>(&Value));
	}
}

static void LinkFlagsByID(DialogItemEx& Parent, DialogItemEx& Target, FARDIALOGITEMFLAGS Flags)
{
	Parent.AddAutomation(Target, Flags, DIF_NONE, DIF_NONE, Flags, DIF_NONE, DIF_NONE);
	if (!Parent.Selected)
		Target.Flags |= Flags;
}

// Связывает состояние элементов Parent и Target. Когда Parent->Selected равно
// false, устанавливает флаги Flags у элемента Target; когда равно true -
// сбрасывает флаги.
// Если LinkLabels установлено в true, то текстовые элементы, добавленные к элементу Target
// методами AddTextBefore и AddTextAfter, также связываются с элементом Parent.
void DialogBuilder::LinkFlags(DialogItemEx& Parent, DialogItemEx& Target, FARDIALOGITEMFLAGS Flags, bool LinkLabels)
{
	Parent.Flags |= DIF_AUTOMATION;
	Parent.AddAutomation(Target, Flags, DIF_NONE, DIF_NONE, Flags, DIF_NONE, DIF_NONE);
	if (!Parent.Selected)
		Target.Flags |= Flags;

	if (LinkLabels)
	{
		const auto& Binding = FindBinding(Target);
		if (Binding.BeforeLabelID != -1)
			LinkFlagsByID(Parent, m_DialogItems[Binding.BeforeLabelID], Flags);
		if (Binding.AfterLabelID != -1)
			LinkFlagsByID(Parent, m_DialogItems[Binding.AfterLabelID], Flags);
	}
}

void DialogBuilder::AddOK()
{
	AddSeparator();
	AddButtons({{ lng::MOk }});
}

void DialogBuilder::AddOKCancel()
{
	AddOKCancel(lng::MOk, lng::MCancel);
}

void DialogBuilder::AddOKCancel(lng OKMessageId, lng CancelMessageId)
{
	AddSeparator();
	AddButtons({{ OKMessageId, CancelMessageId }});
}

void DialogBuilder::AddButtons(std::span<lng const> Buttons)
{
	AddButtons(Buttons, 0, Buttons.size() - 1);
}

void DialogBuilder::AddButtons(std::span<lng const> const Buttons, size_t const OkIndex, size_t const CancelIndex)
{
	const auto LineY = m_NextY++;
	DialogItemEx const* PrevButton = nullptr;

	for (const auto i: std::views::iota(0uz, Buttons.size()))
	{
		auto& NewButton = AddDialogItem(DI_BUTTON, msg(Buttons[i]).c_str());
		NewButton.Flags = DIF_CENTERGROUP;
		NewButton.Y1 = NewButton.Y2 = LineY;
		if (PrevButton)
		{
			NewButton.X1 = PrevButton->X2 + 1;
		}
		else
		{
			NewButton.X1 = 2 + m_Indent;
			m_FirstButtonID = m_DialogItems.size() - 1;
		}
		NewButton.X2 = NewButton.X1 + ItemWidth(NewButton);

		if (OkIndex == i)
		{
			NewButton.Flags |= DIF_DEFAULTBUTTON;
		}
		if (CancelIndex == i)
			m_CancelButtonID = m_DialogItems.size() - 1;

		PrevButton = &NewButton;
	}
}

void DialogBuilder::SetDialogMode(DWORD Flags)
{
	m_Mode = Flags;
}

void DialogBuilder::SetScrObjFlags(DWORD const Flags)
{
	m_ScrObjFlags = Flags;
}

int DialogBuilder::AddTextWrap(lng_string const Text, bool center, int width)
{
	int LineCount = 0;
	for (const auto& i: wrapped_text(string_view(Text.c_str()), width <= 0? ScrX - 1 - 10 : width))
	{
		auto& Item = AddText(null_terminated(i).c_str());
		Item.Flags = center? DIF_CENTERTEXT : 0;
		++LineCount;
	}

	return LineCount;
}

void DialogBuilder::SetId(const UUID& Id)
{
	m_Id = Id;
	m_IdExist = true;
}

const UUID& DialogBuilder::GetId() const
{
	return m_Id;
}

size_t DialogBuilder::GetLastID() const
{
	return m_DialogItems.size() - 1;
}

void DialogBuilder::StartColumns()
{
	m_ColumnStartIndex = m_DialogItems.size();
	m_ColumnStartY = m_NextY;
}

void DialogBuilder::ColumnBreak()
{
	m_ColumnBreakIndex = m_DialogItems.size();
	m_ColumnEndY = m_NextY;
	m_NextY = m_ColumnStartY;
}

void DialogBuilder::EndColumns()
{
	for (const auto i: std::views::iota(m_ColumnStartIndex, m_DialogItems.size()))
	{
		const intptr_t Width = ItemWidth(m_DialogItems[i]);
		if (Width > m_ColumnMinWidth)
			m_ColumnMinWidth = Width;
		if (i >= m_ColumnBreakIndex)
		{
			m_DialogItems[i].X1 = SECOND_COLUMN;
			m_DialogItems[i].X2 = SECOND_COLUMN + Width;
		}
	}

	m_ColumnStartIndex = -1;
	m_ColumnBreakIndex = -1;
	if (m_NextY < m_ColumnEndY)
	{
		m_NextY = m_ColumnEndY;
	}
}

void DialogBuilder::StartSingleBox(lng_string const Text, bool LeftAlign)
{
	auto& SingleBox = AddDialogItem(DI_SINGLEBOX, Text.c_str());
	SingleBox.Flags = LeftAlign? DIF_LEFTTEXT : DIF_NONE;
	SingleBox.X1 = 5;
	SingleBox.Y1 = m_NextY++;
	m_Indent = 2;
	m_SingleBoxIndex = m_DialogItems.size() - 1;
}

void DialogBuilder::EndSingleBox()
{
	if (m_SingleBoxIndex != npos)
	{
		m_DialogItems[m_SingleBoxIndex].Y2 = m_NextY++;
		m_Indent = 0;
		m_SingleBoxIndex = npos;
	}
}

void DialogBuilder::AddEmptyLine()
{
	m_NextY++;
}

void DialogBuilder::AddSeparator(lng_string const Text)
{
	auto& Separator = AddDialogItem(DI_TEXT, Text.c_str());
	Separator.Flags = DIF_SEPARATOR;
	Separator.X1 = -1;
	Separator.Y1 = Separator.Y2 = m_NextY++;
}

intptr_t DialogBuilder::ShowDialogEx()
{
	UpdateBorderSize();
	UpdateSecondColumnPosition();
	intptr_t Result = DoShowDialog();
	if (Result >= 0 && static_cast<size_t>(Result) != m_CancelButtonID)
	{
		SaveValues();
	}

	if (m_FirstButtonID != npos && static_cast<size_t>(Result) >= m_FirstButtonID)
	{
		Result -= m_FirstButtonID;
	}
	return Result;
}

bool DialogBuilder::ShowDialog()
{
	const intptr_t Result = ShowDialogEx();
	return Result >= 0 && (m_CancelButtonID == npos || static_cast<size_t>(Result) + m_FirstButtonID != m_CancelButtonID);
}

DialogItemEx& DialogBuilder::AddDialogItem(FARDIALOGITEMTYPES Type, const wchar_t* Text)
{
	auto& Item = m_DialogItems.emplace_back();
	Item.Type = Type;
	Item.strData = Text;
	m_Bindings.emplace_back();
	return Item;
}

void DialogBuilder::SetNextY(DialogItemEx& Item)
{
	Item.X1 = 5 + m_Indent;
	Item.Y1 = Item.Y2 = m_NextY++;
}

void DialogBuilder::AddBorder(const wchar_t* TitleText)
{
	auto& Title = AddDialogItem(DI_DOUBLEBOX, TitleText);
	Title.X1 = 3;
	Title.Y1 = 1;
}

void DialogBuilder::UpdateBorderSize()
{
	const auto Title = &m_DialogItems[0];
	const auto MaxWidth = MaxTextWidth();
	intptr_t MaxHeight = 0;
	Title->X2 = Title->X1 + MaxWidth + 3;

	for (const auto i: std::views::iota(1uz, m_DialogItems.size()))
	{
		if (m_DialogItems[i].Type == DI_SINGLEBOX)
		{
			m_Indent = 2;
			m_DialogItems[i].X2 = Title->X2;
		}
		else if (m_DialogItems[i].Type == DI_TEXT && (m_DialogItems[i].Flags & DIF_CENTERTEXT))
		{
			//BUGBUG: two columns items are not supported
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

intptr_t DialogBuilder::MaxTextWidth() const
{
	intptr_t MaxWidth = 0;
	for (const auto i: std::views::iota(1uz, m_DialogItems.size()))
	{
		if (m_DialogItems[i].X1 == SECOND_COLUMN)
			continue;
		intptr_t Width = ItemWidth(m_DialogItems[i]);
		const intptr_t Indent = m_DialogItems[i].X1 - 5;
		Width += Indent;

		if (MaxWidth < Width)
			MaxWidth = Width;
	}
	const intptr_t ColumnsWidth = 2 * m_ColumnMinWidth + 1;
	if (MaxWidth < ColumnsWidth)
		return ColumnsWidth;
	return MaxWidth;
}

void DialogBuilder::UpdateSecondColumnPosition()
{
	const auto SecondColumnX1 = 6 + (m_DialogItems[0].X2 - m_DialogItems[0].X1 - 1) / 2;
	for (auto& i: m_DialogItems)
	{
		if (i.X1 == SECOND_COLUMN)
		{
			const auto Width = i.X2 - i.X1;
			i.X1 = SecondColumnX1;
			i.X2 = i.X1 + Width;
		}
	}
}

void DialogBuilder::SetLastItemBinding(std::unique_ptr<DialogItemBinding>&& Binding)
{
	m_Bindings.back() = std::move(Binding);
}

int DialogBuilder::GetItemID(DialogItemEx const& Item) const
{
	return static_cast<int>(&Item - m_DialogItems.data());
}

DialogItemBinding& DialogBuilder::FindBinding(DialogItemEx const& Item) const
{
	return *m_Bindings[GetItemID(Item)];
}

void DialogBuilder::SaveValues()
{
	int RadioGroupIndex = 0;
	for (const auto& [Item, Binding]: zip(m_DialogItems, m_Bindings))
	{
		if (Item.Flags & DIF_GROUP)
			RadioGroupIndex = 0;
		else
			RadioGroupIndex++;

		if (Binding)
			Binding->SaveValue(Item, RadioGroupIndex);
	}
}

intptr_t DialogBuilder::DoShowDialog()
{
	const auto Dlg = Dialog::create(m_DialogItems, m_handler, nullptr);
	Dlg->SetHelp(m_HelpTopic);
	Dlg->SetPosition({ -1, -1, static_cast<int>(m_DialogItems[0].X2 + 4), static_cast<int>(m_DialogItems[0].Y2 + 2) });
	if (m_Mode)
		Dlg->SetDialogMode(m_Mode);

	if (m_ScrObjFlags)
		Dlg->SetFlags(m_ScrObjFlags);

	if (m_IdExist)
		Dlg->SetId(m_Id);
	Dlg->Process();
	return Dlg->GetExitCode();
}

template<typename value_type>
DialogItemEx& DialogBuilder::AddCheckboxImpl(lng_string const Text, value_type& Value, unsigned Mask, bool ThreeState)
{
	auto& Item = AddDialogItem(DI_CHECKBOX, Text.c_str());
	if (ThreeState && !Mask)
		Item.Flags |= DIF_3STATE;
	SetNextY(Item);
	Item.X2 = Item.X1 + ItemWidth(Item);

	if constexpr (is_one_of_v<value_type, BoolOption, Bool3Option>)
	{
		Item.Selected = Value;
	}
	else
	{
		if (!Mask)
			Item.Selected = Value;
		else
			Item.Selected = (Value & Mask) != 0;
	}

	return Item;
}

template<typename value_type>
DialogItemEx& DialogBuilder::AddListControlImpl(FARDIALOGITEMTYPES Type, value_type& Value, int Width, int Height, std::span<DialogBuilderListItem const> const Items, FARDIALOGITEMFLAGS Flags)
{
	auto& Item = AddDialogItem(Type, L"");
	SetNextY(Item);
	Item.X2 = Item.X1 + Width;
	Item.Y2 = Item.Y1 + Height;
	Item.Flags |= Flags;

	m_NextY += Height;

	std::vector<FarListItem> ListItems;
	ListItems.reserve(Items.size());

	std::ranges::transform(Items, std::back_inserter(ListItems), [&Value](const DialogBuilderListItem& i)
	{
		FarListItem NewItem{};
		NewItem.Text = i.str().c_str();
		NewItem.Flags = (Value == i.value()? LIF_SELECTED : 0) | i.flags();
		NewItem.UserData = i.value();
		return NewItem;
	});


	class ListControlBinding: public DialogItemBinding
	{
	public:
		NONCOPYABLE(ListControlBinding);

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

		void SaveValue(DialogItemEx const& Item, int const RadioGroupIndex) override
		{
			if (!m_ListItems.empty())
			{
				m_Value = m_ListItems[Item.ListPos].UserData;
			}
		}

	private:
		value_type& m_Value;
		std::vector<FarListItem> m_ListItems;
		FarList m_List{ sizeof(m_List) };
	};

	auto Binding = std::make_unique<ListControlBinding>(Value, std::move(ListItems));
	Item.ListItems = Binding->list();
	SetLastItemBinding(std::move(Binding));
	return Item;
}
