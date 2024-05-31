/*
vmenu.cpp

Обычное вертикальное меню
  а так же:
    * список в DI_COMBOBOX
    * список в DI_LISTBOX
    * ...
*/
/*
Copyright © 1996 Eugene Roshal
Copyright © 2000 Far Group
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
#include "vmenu.hpp"

// Internal:
#include "keyboard.hpp"
#include "keys.hpp"
#include "macroopcode.hpp"
#include "farcolor.hpp"
#include "dialog.hpp"
#include "savescr.hpp"
#include "clipboard.hpp"
#include "ctrlobj.hpp"
#include "manager.hpp"
#include "constitle.hpp"
#include "interf.hpp"
#include "colormix.hpp"
#include "config.hpp"
#include "processname.hpp"
#include "uuids.far.hpp"
#include "xlat.hpp"
#include "lang.hpp"
#include "vmenu2.hpp"
#include "strmix.hpp"
#include "string_sort.hpp"
#include "exception.hpp"
#include "global.hpp"

// Platform:

// Common:
#include "common.hpp"
#include "common/scope_exit.hpp"
#include "common/uuid.hpp"
#include "common/view/enumerate.hpp"
#include "common/view/zip.hpp"

// External:

//----------------------------------------------------------------------------

// Must be in the TU scope because it is befriended by VMenu
struct menu_layout
{
	short BoxType{};
	small_rectangle ClientRect{};
	std::optional<short> LeftBox;
	std::optional<short> CheckMark;
	std::optional<short> LeftHScroll;
	std::optional<std::pair<short, short>> TextArea; // Begin, Width
	std::optional<short> RightHScroll;
	std::optional<short> SubMenu;
	std::optional<short> Scrollbar;
	std::optional<short> RightBox;

	explicit menu_layout(const VMenu& Menu)
		: BoxType{ get_box_type(Menu) }
		, ClientRect{ get_client_rect(Menu, BoxType) }
	{
		auto Left{ Menu.m_Where.left };
		if (need_box(BoxType))       LeftBox = Left++;
		if (need_check_mark())       CheckMark = Left++;
		if (need_left_hscroll())     LeftHScroll = Left++;

		auto Right{ Menu.m_Where.right };
		if (need_box(BoxType))       RightBox = Right;
		if (need_scrollbar(Menu, BoxType)) Scrollbar = Right;
		if (RightBox || Scrollbar)   Right--;
		if (need_submenu(Menu))      SubMenu = Right--;
		if (need_right_hscroll())    RightHScroll = Right--;

		if (Left <= Right)
			TextArea = { Left, Right + 1 - Left };
	}

	[[nodiscard]] static bool need_box(const VMenu& Menu) noexcept
	{
		return !(Menu.CheckFlags(VMENU_LISTBOX) && Menu.CheckFlags(VMENU_SHOWNOBOX));
	}

	[[nodiscard]] static short get_box_type(const VMenu& Menu) noexcept
	{
		if (Menu.CheckFlags(VMENU_LISTBOX))
		{
			if (Menu.CheckFlags(VMENU_LISTSINGLEBOX))
				return SHORT_SINGLE_BOX;
			else if (Menu.CheckFlags(VMENU_SHOWNOBOX))
				return NO_BOX;
			else if (Menu.CheckFlags(VMENU_LISTHASFOCUS))
				return SHORT_DOUBLE_BOX;
			else
				return SHORT_SINGLE_BOX;
		}
		else if (Menu.CheckFlags(VMENU_COMBOBOX))
			return SHORT_SINGLE_BOX;
		else
			return DOUBLE_BOX;
	}

	[[nodiscard]] static int get_service_area_size(const VMenu& Menu)
	{
		return get_service_area_size(Menu, need_box(Menu));
	}

	[[nodiscard]] static int get_service_area_size(const VMenu& Menu, const short BoxType)
	{
		const auto NeedBox = need_box(BoxType);

		return NeedBox
			+ need_check_mark()
			+ need_left_hscroll()
			+ need_right_hscroll()
			+ need_submenu(Menu)
			+ (NeedBox || need_scrollbar(Menu, BoxType));
	}

private:
	[[nodiscard]] static rectangle get_client_rect(const VMenu& Menu, short const BoxType) noexcept
	{
		if (!need_box(BoxType))
			return Menu.m_Where;

		return { Menu.m_Where.left + 1, Menu.m_Where.top + 1, Menu.m_Where.right - 1, Menu.m_Where.bottom - 1 };
	}

	[[nodiscard]] static bool need_box(short BoxType) noexcept { return BoxType != NO_BOX; }
	[[nodiscard]] static bool need_check_mark() noexcept { return true; }
	[[nodiscard]] static bool need_left_hscroll() noexcept { return true; }
	[[nodiscard]] static bool need_right_hscroll() noexcept { return true; }
	[[nodiscard]] static bool need_submenu(const VMenu& Menu) noexcept { return Menu.ItemSubMenusCount > 0; }
	[[nodiscard]] static bool need_scrollbar(const VMenu& Menu, short const BoxType)
	{
		if (!Menu.CheckFlags(VMENU_LISTBOX | VMENU_ALWAYSSCROLLBAR) && !Global->Opt->ShowMenuScrollbar)
			return false;

		// Check separately because passing an empty menu to get_client_rect will trigger an assertion
		const auto ItemsCount = Menu.GetShowItemCount();
		if (!ItemsCount)
			return false;

		return ScrollBarRequired(get_client_rect(Menu, BoxType).height(), ItemsCount);
	}
};

namespace
{
	MenuItemEx far_list_to_menu_item(const FarListItem& FItem)
	{
		MenuItemEx Result;
		Result.Flags = FItem.Flags;
		Result.Name = NullToEmpty(FItem.Text);
		Result.SimpleUserData = FItem.UserData;
		return Result;
	}

	int find_nearest(std::ranges::contiguous_range auto const& Range, const int Pos, const auto Pred, const bool GoBackward, const bool DoWrap)
	{
		using namespace std::views;

		assert(0 <= Pos && Pos < static_cast<int>(Range.size()));

		const auto FindPos =
			[&](const auto First, const auto Second)
			{
				const auto FindPosPart =
					[&](const auto Part)
					{
						if (auto Filtered = Range | Part | filter(Pred))
							return static_cast<int>(&Filtered.front() - Range.data());

						return -1;
					};

				if (const auto Found{ FindPosPart(First) }; Found != -1) return Found;
				if (const auto Found{ FindPosPart(Second) }; Found != -1) return Found;
				return -1;
			};

		return GoBackward
			? (DoWrap
				? FindPos(take(Pos + 1) | reverse, drop(Pos + 1) | reverse)
				: FindPos(take(Pos + 1) | reverse, drop(Pos + 1)))
			: (DoWrap
				? FindPos(drop(Pos), take(Pos))
				: FindPos(drop(Pos), take(Pos) | reverse));
	}

	std::pair<int, int> intersect(std::pair<int, int> A, std::pair<int, int> B)
	{
		assert(A.first <= A.second);
		assert(B.first <= B.second);

		if (A.first == A.second || B.first == B.second)
			return {};

		if (B.first < A.first)
			std::ranges::swap(A, B);

		if (A.second <= B.first)
			return {};

		return { B.first, std::min(A.second, B.second) };
	}

	void markup_slice_boundaries(std::pair<int, int> Segment, std::ranges::input_range auto const& Slices, std::vector<int>& Markup)
	{
		assert(Segment.first <= Segment.second);

		for (const auto& Slice : Slices)
		{
			if (Slice.first >= Slice.second)
				continue;

			const auto Intersection{ intersect(Segment, Slice) };

			if (Intersection.first == Intersection.second)
				continue;

			Markup.emplace_back(Intersection.first);
			Markup.emplace_back(Intersection.second);
			Segment.first = Intersection.second;

			if (Segment.first == Segment.second)
				return;
		}

		Markup.emplace_back(Segment.second);
	}

	bool item_flags_allow_focus(unsigned long long const Flags)
	{
		return !(Flags & (LIF_DISABLE | LIF_HIDDEN | LIF_FILTERED | LIF_SEPARATOR));
	}

	bool item_can_have_focus(MenuItemEx const& Item)
	{
		return item_flags_allow_focus(Item.Flags);
	}

	bool item_can_be_entered(MenuItemEx const& Item)
	{
		return item_can_have_focus(Item) && !(Item.Flags & LIF_GRAYED);
	}

	bool item_is_visible(MenuItemEx const& Item)
	{
		return !(Item.Flags & (LIF_HIDDEN | LIF_FILTERED));
	}

	int get_item_visual_length(const bool ShowAmpersand, const string_view ItemName)
	{
		return static_cast<int>(ShowAmpersand ? visual_string_length(ItemName) : HiStrlen(ItemName));
	}

	enum class item_hscroll_policy
	{
		unbound,            // The item can move freely beyond window edges.
		cling_to_edge,      // The item can move beyond window edges, but at least one character is always visible.
		bound,              // The item can move only within the window boundaries.
		bound_stick_to_left // Like bound, but if the item shorter than TextAreaWidth, it is always attached to the left window edge.
	};

	std::pair<int, int> item_hpos_limits(const int ItemLength, const int TextAreaWidth, const item_hscroll_policy Policy) noexcept
	{
		using enum item_hscroll_policy;

		assert(ItemLength > 0);
		assert(TextAreaWidth > 0);

		switch (Policy)
		{
		case unbound:
			return{ std::numeric_limits<int>::min(), std::numeric_limits<int>::max()};

		case cling_to_edge:
			return{ 1 - ItemLength, TextAreaWidth - 1 };

		case bound:
			return{ std::min(0, TextAreaWidth - ItemLength), std::max(0, TextAreaWidth - ItemLength) };

		case bound_stick_to_left:
			return{ std::min(0, TextAreaWidth - ItemLength), 0 };

		default:
			std::unreachable();
		}
	}

	int get_item_absolute_hpos(const int NewHPos, const int ItemLength, const int TextAreaWidth, const item_hscroll_policy Policy)
	{
		const auto [HPosMin, HPosMax]{ item_hpos_limits(ItemLength, TextAreaWidth, Policy) };
		return std::clamp(NewHPos, HPosMin, HPosMax);
	}

	int get_item_smart_hpos(const int NewHPos, const int ItemLength, const int TextAreaWidth, const item_hscroll_policy Policy)
	{
		return get_item_absolute_hpos(NewHPos >= 0 ? NewHPos : TextAreaWidth - ItemLength + NewHPos + 1, ItemLength, TextAreaWidth, Policy);
	}

	int adjust_hpos_shift(const int Shift, const int Left, const int Right, const int TextAreaWidth)
	{
		assert(Left < Right);

		if (Shift == 0) return 0;

		// Shift left.
		if (Shift > 0)
		{
			const auto ShiftLimit{ std::max(TextAreaWidth - Left - 1, 0) };
			const auto GapLeftOfTextArea{ std::max(-Right, 0) };
			return std::min(Shift + GapLeftOfTextArea, ShiftLimit);
		}

		// Shift right. It's just shift left seen from behind the screen.
		return -adjust_hpos_shift(-Shift, TextAreaWidth - Right, TextAreaWidth - Left, TextAreaWidth);
	}

	// Indices in the color array
	enum class color_indices
	{
		Body                = 0,     // background
		Box                 = 1,     // border
		Title               = 2,     // title - top and bottom
		Text                = 3,     // item text
		Highlight           = 4,     // hot key
		Separator           = 5,     // separator
		Selected            = 6,     // selected
		HSelect             = 7,     // selected - HotKey
		ScrollBar           = 8,     // scrollBar
		Disabled            = 9,     // disabled
		Arrows              =10,     // '«' & '»' normal
		ArrowsSelect        =11,     // '«' & '»' selected
		ArrowsDisabled      =12,     // '«' & '»' disabled
		Grayed              =13,     // grayed
		SelGrayed           =14,     // selected grayed

		COUNT                        // always the last - array dimension
	};

	static_assert(std::tuple_size_v<vmenu_colors_t> == std::to_underlying(color_indices::COUNT));

	[[nodiscard]] const FarColor& get_color(const vmenu_colors_t& VMenuColors, color_indices ColorIndex) noexcept
	{
		return VMenuColors[std::to_underlying(ColorIndex)];
	}

	void set_color(const vmenu_colors_t& VMenuColors, color_indices ColorIndex)
	{
		SetColor(get_color(VMenuColors, ColorIndex));
	}

	struct item_color_indicies
	{
		color_indices Normal, Highlighted, HScroller;

		item_color_indicies(const MenuItemEx& CurItem)
		{
			const auto Selected{ !!(CurItem.Flags & LIF_SELECTED) };
			const auto Grayed{ !!(CurItem.Flags & LIF_GRAYED) };
			const auto Disabled{ !!(CurItem.Flags & LIF_DISABLE) };

			if (Disabled)
			{
				Normal = color_indices::Disabled;
				Highlighted = color_indices::Disabled;
				HScroller = color_indices::ArrowsDisabled;
				return;
			}

			if (Selected)
			{
				Normal = Grayed ? color_indices::SelGrayed : color_indices::Selected;
				Highlighted = Grayed ? color_indices::SelGrayed : color_indices::HSelect;
				HScroller = color_indices::ArrowsSelect;
				return;
			}

			Normal = Grayed ? color_indices::Grayed : color_indices::Text;
			Highlighted = Grayed ? color_indices::Grayed : color_indices::Highlight;
			HScroller = color_indices::Arrows;
		}
	};

	std::tuple<color_indices, wchar_t> get_item_check_mark(const MenuItemEx& CurItem, item_color_indicies ColorIndices) noexcept
	{
		return
		{
			ColorIndices.Normal,
			!(CurItem.Flags & LIF_CHECKED)
				? L' '
				: !(CurItem.Flags & 0x0000FFFF) ? L'√' : static_cast<wchar_t>(CurItem.Flags & 0x0000FFFF)
		};
	}

	std::tuple<color_indices, wchar_t> get_item_submenu(const MenuItemEx& CurItem, item_color_indicies ColorIndices) noexcept
	{
		return
		{
			ColorIndices.Normal,
			(CurItem.Flags & MIF_SUBMENU) ? L'►' : L' '
		};
	}

	std::tuple<color_indices, wchar_t> get_item_left_hscroll(const bool NeedLeftHScroll, item_color_indicies ColorIndices) noexcept
	{
		return
		{
			NeedLeftHScroll ? ColorIndices.HScroller : ColorIndices.Normal,
			NeedLeftHScroll ? L'«' : L' '
		};
	}

	std::tuple<color_indices, wchar_t> get_item_right_hscroll(const bool NeedRightHScroll, item_color_indicies ColorIndices) noexcept
	{
		return
		{
			NeedRightHScroll ? ColorIndices.HScroller : ColorIndices.Normal,
			NeedRightHScroll ? L'»' : L' '
		};
	}
}

VMenu::VMenu(private_tag, string Title, int MaxHeight, dialog_ptr ParentDialog):
	strTitle(std::move(Title)),
	MaxHeight(MaxHeight),
	ParentDialog(ParentDialog),
	MenuId(FarUuid)
{
}

vmenu_ptr VMenu::create(string Title, std::span<menu_item const> const Data, int MaxHeight, DWORD Flags, dialog_ptr ParentDialog)
{
	auto VmenuPtr = std::make_shared<VMenu>(private_tag(), std::move(Title), MaxHeight, ParentDialog);
	VmenuPtr->init(Data, Flags);
	return VmenuPtr;
}

void VMenu::init(std::span<menu_item const> const Data, DWORD Flags)
{
	SaveScr=nullptr;
	SetMenuFlags(Flags | VMENU_MOUSEREACTION | VMENU_UPDATEREQUIRED);
	ClearFlags(VMENU_MOUSEDOWN);
	CurrentWindow = Global->WindowManager->GetCurrentWindow();
	GetCursorType(PrevCursorVisible,PrevCursorSize);
	bRightBtnPressed = false;

	// инициализируем перед добавлением элемента
	UpdateMaxLengthFromTitles();

	for (const auto& i: Data)
	{
		MenuItemEx NewItem;
		static_cast<menu_item&>(NewItem) = i;
		AddItem(std::move(NewItem));
	}

	SetMaxHeight(MaxHeight);
	SetColors(nullptr); //Установим цвет по умолчанию
}

VMenu::~VMenu()
{
	VMenu::Hide();
	clear();

	if (Global->WindowManager->GetCurrentWindow() == CurrentWindow)
		SetCursorType(PrevCursorVisible,PrevCursorSize);
}

void VMenu::ResetCursor()
{
	GetCursorType(PrevCursorVisible,PrevCursorSize);
}

bool VMenu::UpdateRequired() const
{
	return CheckFlags(VMENU_UPDATEREQUIRED)!=0;
}

void VMenu::UpdateItemFlags(int Pos, unsigned long long NewFlags)
{
	if (Items[Pos].Flags & MIF_SUBMENU)
		--ItemSubMenusCount;

	if (!item_is_visible(Items[Pos]))
		--ItemHiddenCount;


	if (!item_flags_allow_focus(NewFlags))
		NewFlags &= ~LIF_SELECTED;

	//remove selection
	if ((Items[Pos].Flags&LIF_SELECTED) && !(NewFlags&LIF_SELECTED))
	{
		SelectPos = -1;
	}
	//set new selection
	else if (!(Items[Pos].Flags&LIF_SELECTED) && (NewFlags&LIF_SELECTED))
	{
		if (SelectPos>=0)
			Items[SelectPos].Flags &= ~LIF_SELECTED;

		SelectPos = Pos;
	}

	Items[Pos].Flags = NewFlags;

	if (SelectPos < 0)
		SetSelectPos(0,1);

	if(const auto Value = extract_integer<WORD, 0>(Items[Pos].Flags))
	{
		Items[Pos].Flags|=LIF_CHECKED;
		if (Value == 1)
		{
			Items[Pos].Flags&=0xFFFF0000;
		}
	}

	if (NewFlags&MIF_SUBMENU)
		ItemSubMenusCount++;

	if (!item_is_visible(Items[Pos]))
		ItemHiddenCount++;
}

// переместить курсор c учётом пунктов которые не могут получать фокус
int VMenu::SetSelectPos(int Pos, int Direct, bool stop_on_edge)
{
	SelectPosResult=-1;

	if (Items.empty())
		return -1;

	for (auto& i: Items)
	{
		i.Flags &= ~LIF_SELECTED;
	}

	const auto DoWrap{ CheckFlags(VMENU_WRAPMODE) && Direct != 0 && !stop_on_edge };
	const auto GoBackward{ Direct < 0 };
	const auto ItemsSize{ static_cast<int>(Items.size()) };

	if (Pos < 0)
	{
		Pos = DoWrap ? ItemsSize - 1 : 0;
	}
	else if (Pos >= ItemsSize)
	{
		Pos = DoWrap ? 0 : ItemsSize - 1;
	}

	Pos = find_nearest(Items, Pos, item_can_have_focus, GoBackward, DoWrap);

	if (Pos != SelectPos && CheckFlags(VMENU_COMBOBOX | VMENU_LISTBOX))
	{
		if (const auto Parent = GetDialog(); Parent && Parent->IsInited() && !Parent->SendMessage(DN_LISTCHANGE, DialogItemID, ToPtr(Pos)))
		{
			UpdateItemFlags(SelectPos, Items[SelectPos].Flags | LIF_SELECTED);
			return -1;
		}
	}

	if (Pos >= 0)
		UpdateItemFlags(Pos, Items[Pos].Flags | LIF_SELECTED);

	SetMenuFlags(VMENU_UPDATEREQUIRED);

	SelectPosResult = Pos;
	return Pos;
}

// установить курсор и верхний элемент
int VMenu::SetSelectPos(const FarListPos *ListPos, int Direct)
{
	if (Items.empty())
		return -1;

	const auto pos = std::clamp(ListPos->SelectPos, intptr_t{}, static_cast<intptr_t>(Items.size() - 1));
	const auto Ret = SetSelectPos(pos, Direct ? Direct : pos > SelectPos? 1 : -1);

	if (Ret >= 0)
	{
		TopPos = ListPos->TopPos;

		if (TopPos == -1)
		{
			if (GetShowItemCount() < MaxHeight)
			{
				TopPos = VisualPosToReal(0);
			}
			else
			{
				TopPos = GetVisualPos(TopPos);
				TopPos = (GetVisualPos(SelectPos)-TopPos+1) > MaxHeight ? TopPos+1 : TopPos;

				if (TopPos+MaxHeight > GetShowItemCount())
					TopPos = GetShowItemCount()-MaxHeight;

				TopPos = VisualPosToReal(TopPos);
			}
		}

		if (TopPos < 0)
			TopPos = 0;
	}

	return Ret;
}

//корректировка текущей позиции
void VMenu::UpdateSelectPos()
{
	if (Items.empty())
		return;

	// если selection стоит в некорректном месте - сбросим его
	if (SelectPos >= 0 && !item_can_have_focus(Items[SelectPos]))
		SelectPos = -1;

	for (const auto& [Item, Index]: enumerate(Items))
	{
		if (!item_can_have_focus(Item))
		{
			Item.SetSelect(false);
		}
		else
		{
			if (SelectPos == -1)
			{
				Item.SetSelect(true);
				SelectPos = static_cast<int>(Index);
			}
			else if (SelectPos != static_cast<int>(Index))
			{
				Item.SetSelect(false);
			}
			else
			{
				Item.SetSelect(true);
			}
		}
	}
}

int VMenu::GetItemPosition(int Position) const
{
	int DataPos = (Position==-1) ? SelectPos : Position;

	if (DataPos>=static_cast<int>(Items.size()))
		DataPos = -1; //Items.size()-1;

	return DataPos;
}

// получить позицию курсора и верхнюю позицию элемента
int VMenu::GetSelectPos(FarListPos *ListPos) const
{
	ListPos->SelectPos=SelectPos;
	ListPos->TopPos=TopPos;

	return ListPos->SelectPos;
}

int VMenu::InsertItem(const FarListInsert *NewItem)
{
	if (NewItem)
	{
		if (AddItem(far_list_to_menu_item(NewItem->Item), NewItem->Index) >= 0)
			return static_cast<int>(Items.size());
	}

	return -1;
}

int VMenu::AddItem(const FarList* List)
{
	if (List && List->Items)
	{
		for (const auto& Item: std::span(List->Items, List->ItemsNumber))
		{
			AddItem(far_list_to_menu_item(Item));
		}
	}

	return static_cast<int>(Items.size());
}

int VMenu::AddItem(const wchar_t *NewStrItem)
{
	FarListItem FarListItem0{};

	if (!NewStrItem || NewStrItem[0] == 0x1)
	{
		FarListItem0.Flags=LIF_SEPARATOR;
		if (NewStrItem)
			FarListItem0.Text = NewStrItem + 1;
	}
	else
	{
		FarListItem0.Text=NewStrItem;
	}

	const FarList List{ sizeof(List), 1, &FarListItem0 };

	return AddItem(&List) - 1; //-1 потому что AddItem(FarList) возвращает количество элементов
}

int VMenu::AddItem(MenuItemEx&& NewItem,int PosAdd)
{
	PosAdd = std::clamp(PosAdd, 0, static_cast<int>(Items.size()));

	Items.emplace(Items.begin() + PosAdd, std::move(NewItem));
	auto& NewMenuItem = Items[PosAdd];

	NewMenuItem.AutoHotkey = {};
	NewMenuItem.AutoHotkeyPos = 0;
	NewMenuItem.HorizontalPosition = 0;

	if (PosAdd <= SelectPos)
		SelectPos++;

	const auto ItemLength{ get_item_visual_length(CheckFlags(VMENU_SHOWAMPERSAND), NewMenuItem.Name) };
	UpdateMaxLength(ItemLength);
	UpdateAllItemsBoundaries(NewMenuItem.HorizontalPosition, ItemLength);

	const auto NewFlags = NewMenuItem.Flags;
	NewMenuItem.Flags = 0;
	UpdateItemFlags(PosAdd, NewFlags);

	SetMenuFlags(VMENU_UPDATEREQUIRED | (bFilterEnabled ? VMENU_REFILTERREQUIRED : VMENU_NONE));

	return static_cast<int>(Items.size()-1);
}

bool VMenu::UpdateItem(const FarListUpdate *NewItem)
{
	if (!NewItem || static_cast<size_t>(NewItem->Index) >= Items.size())
		return false;

	auto& Item = Items[NewItem->Index];

	// Освободим память... от ранее занятого ;-)
	if (NewItem->Item.Flags&LIF_DELETEUSERDATA)
	{
		Item.ComplexUserData = {};
	}

	Item.Name = NullToEmpty(NewItem->Item.Text);
	UpdateItemFlags(NewItem->Index, NewItem->Item.Flags);
	Item.SimpleUserData = NewItem->Item.UserData;

	const auto ItemLength{ get_item_visual_length(CheckFlags(VMENU_SHOWAMPERSAND), Item.Name) };
	UpdateMaxLength(ItemLength);
	UpdateAllItemsBoundaries(Item.HorizontalPosition, ItemLength);

	SetMenuFlags(VMENU_UPDATEREQUIRED | (bFilterEnabled ? VMENU_REFILTERREQUIRED : VMENU_NONE));

	return true;
}

//функция удаления N пунктов меню
int VMenu::DeleteItem(int ID, int Count)
{
	if (ID < 0 || ID >= static_cast<int>(Items.size()) || Count <= 0)
		return static_cast<int>(Items.size());

	if (ID+Count > static_cast<int>(Items.size()))
		Count=static_cast<int>(Items.size()-ID);

	if (Count <= 0)
		return static_cast<int>(Items.size());

	if (!ID && Count == static_cast<int>(Items.size()))
	{
		clear();
		return static_cast<int>(Items.size());
	}

	for (const auto I: std::views::iota(0, Count))
	{
		if (Items[ID+I].Flags & MIF_SUBMENU)
			--ItemSubMenusCount;

		if (!item_is_visible(Items[ID+I]))
			--ItemHiddenCount;
	}

	// а вот теперь перемещения
	const auto FirstIter = Items.begin() + ID, LastIter = FirstIter + Count;
	if (Items.size() > 1)
		Items.erase(FirstIter, LastIter);

	// коррекция текущей позиции
	if (SelectPos >= ID && SelectPos < ID+Count)
	{
		if(SelectPos==static_cast<int>(Items.size()))
		{
			ID--;
		}
		SelectPos = -1;
		SetSelectPos(ID, 0, true);
	}
	else if (SelectPos >= ID+Count)
	{
		SelectPos -= Count;

		if (TopPos >= ID+Count)
			TopPos -= Count;
	}

	SetMenuFlags(VMENU_UPDATEREQUIRED);

	return static_cast<int>(Items.size());
}

void VMenu::clear()
{
	Items.clear();
	ItemHiddenCount=0;
	ItemSubMenusCount=0;
	SelectPos=-1;
	TopPos=0;
	m_MaxItemLength = 0;
	UpdateMaxLengthFromTitles();
	ResetAllItemsBoundaries();

	SetMenuFlags(VMENU_UPDATEREQUIRED);
}

int VMenu::GetCheck(int Position)
{
	const auto ItemPos = GetItemPosition(Position);

	if (ItemPos < 0)
		return 0;

	if (Items[ItemPos].Flags & LIF_SEPARATOR)
		return 0;

	if (!(Items[ItemPos].Flags & LIF_CHECKED))
		return 0;

	const auto Checked = Items[ItemPos].Flags & 0xFFFF;

	return Checked ? Checked : 1;
}

void VMenu::SetCheck(int Position)
{
	const auto ItemPos = GetItemPosition(Position);
	if (ItemPos < 0)
		return;

	Items[ItemPos].SetCheck();
}

void VMenu::SetCustomCheck(wchar_t Char, int Position)
{
	const auto ItemPos = GetItemPosition(Position);
	if (ItemPos < 0)
		return;

	Items[ItemPos].SetCustomCheck(Char);
}

void VMenu::ClearCheck(int Position)
{
	const auto ItemPos = GetItemPosition(Position);
	if (ItemPos < 0)
		return;

	Items[ItemPos].ClearCheck();
}

void VMenu::RestoreFilteredItems()
{
	for (auto& i: Items)
	{
		if (!(i.Flags & MIF_FILTERED))
			continue;

		i.Flags &= ~MIF_FILTERED;
		if (item_is_visible(i))
			--ItemHiddenCount;
	}

	FilterUpdateHeight();

	// Подровнять, а то в нижней части списка может оставаться куча пустых строк
	const FarListPos pos{ sizeof(pos), SelectPos < 0? 0 : SelectPos, -1 };
	SetSelectPos(&pos);
}

void VMenu::FilterStringUpdated()
{
	int PrevSeparator = -1, PrevGroup = -1;
	int UpperVisible = -1, LowerVisible = -2;
	bool bBottomMode = false;

	if (SelectPos > 0)
	{
		// Определить, в верхней или нижней части расположен курсор
		const auto TopVisible = GetVisualPos(TopPos);
		const auto SelectedVisible = GetVisualPos(SelectPos);
		const auto BottomVisible = (TopVisible+MaxHeight > GetShowItemCount()) ? (TopVisible+MaxHeight-1) : (GetShowItemCount()-1);
		if (SelectedVisible >= ((TopVisible+BottomVisible)>>1))
			bBottomMode = true;
	}

	ItemHiddenCount=0;

	for (const auto& [CurItem, index]: enumerate(Items))
	{
		CurItem.Flags &= ~LIF_FILTERED;

		if (!item_is_visible(CurItem))
		{
			++ItemHiddenCount;
			continue;
		}

		if (CurItem.Flags & LIF_SEPARATOR)
		{
			// В предыдущей группе все элементы скрыты, разделитель перед группой - не нужен
			if (PrevSeparator != -1)
			{
				Items[PrevSeparator].Flags |= LIF_FILTERED;
				ItemHiddenCount++;
			}

			if (CurItem.Name.empty() && PrevGroup == -1)
			{
				CurItem.Flags |= LIF_FILTERED;
				ItemHiddenCount++;
				PrevSeparator = -1;
			}
			else
			{
				PrevSeparator = static_cast<int>(index);
			}
		}
		else
		{
			if(!contains_icase(remove_highlight(trim(CurItem.Name)), strFilter))
			{
				CurItem.Flags |= LIF_FILTERED;
				ItemHiddenCount++;
				if (SelectPos == static_cast<int>(index))
				{
					CurItem.Flags &= ~LIF_SELECTED;
					SelectPos = -1;
					LowerVisible = -1;
				}
			}
			else
			{
				PrevGroup = static_cast<int>(index);
				if (LowerVisible == -2)
				{
					if (item_can_have_focus(CurItem))
						UpperVisible = static_cast<int>(index);
				}
				else if (LowerVisible == -1)
				{
					if (item_can_have_focus(CurItem))
						LowerVisible = static_cast<int>(index);
				}
				// Этот разделитель - оставить видимым
				if (PrevSeparator != -1)
					PrevSeparator = -1;
			}
		}
	}

	// В предыдущей группе все элементы скрыты, разделитель перед группой - не нужен
	if (PrevSeparator != -1)
	{
		Items[PrevSeparator].Flags |= LIF_FILTERED;
		ItemHiddenCount++;
	}

	FilterUpdateHeight();

	if (GetShowItemCount()>0)
	{
		// Подровнять, а то в нижней части списка может оставаться куча пустых строк
		FarListPos pos{ sizeof(pos), SelectPos, -1 };
		if (SelectPos<0)
		{
			pos.SelectPos = bBottomMode ? ((LowerVisible>0) ? LowerVisible : UpperVisible) : UpperVisible;
			if (pos.SelectPos == -1)
				pos.SelectPos = bBottomMode ? VisualPosToReal(GetShowItemCount()-1) : 0;
		}
		SetSelectPos(&pos);
	}
}

void VMenu::FilterUpdateHeight(bool bShrink)
{
	const auto Parent = std::dynamic_pointer_cast<VMenu2>(GetDialog());

	if (WasAutoHeight || Parent)
	{
		int NewBottom;
		if (MaxHeight && MaxHeight<GetShowItemCount())
			NewBottom = m_Where.top + MaxHeight + 1;
		else
			NewBottom = m_Where.top + GetShowItemCount() + 1;
		if (NewBottom > ScrY)
			NewBottom = ScrY;
		if (NewBottom > m_Where.bottom || (bShrink && NewBottom < m_Where.bottom))
		{
			if (Parent)
				Parent->Resize();
			else
			{
				auto NewPosition = m_Where;
				NewPosition.bottom = NewBottom;
				SetPosition(NewPosition);
			}
		}
	}
}

static bool IsFilterEditKey(int Key)
{
	return (Key >= static_cast<int>(KEY_SPACE) && Key < 0xffff) || Key == KEY_BS;
}

bool VMenu::ShouldSendKeyToFilter(unsigned const Key) const
{
	if (any_of(Key, KEY_CTRLALTF, KEY_RCTRLRALTF, KEY_CTRLRALTF, KEY_RCTRLALTF))
		return true;

	if (bFilterEnabled)
	{
		if (any_of(Key, KEY_CTRLALTL, KEY_RCTRLRALTL, KEY_CTRLRALTL, KEY_RCTRLALTL))
			return true;

		if (!bFilterLocked && IsFilterEditKey(Key))
			return true;
	}

	return false;
}

long long VMenu::VMProcess(int OpCode, void* vParam, long long iParam)
{
	switch (OpCode)
	{
		case MCODE_C_EMPTY:
			return GetShowItemCount()<=0;
		case MCODE_C_EOF:
			return GetVisualPos(SelectPos)==GetShowItemCount()-1;
		case MCODE_C_BOF:
			return GetVisualPos(SelectPos)<=0;
		case MCODE_C_SELECTED:
			return !Items.empty() && SelectPos >= 0;
		case MCODE_V_ITEMCOUNT:
			return GetShowItemCount();
		case MCODE_V_CURPOS:
			return GetVisualPos(SelectPos)+1;
		case MCODE_F_MENU_CHECKHOTKEY:
		{
			const auto str = static_cast<const wchar_t*>(vParam);
			return GetVisualPos(CheckHighlights(*str, VisualPosToReal(static_cast<int>(iParam)))) + 1;
		}
		case MCODE_F_MENU_SELECT:
		{
			const auto StrParam = static_cast<const wchar_t*>(vParam);
			if (!*StrParam)
				return 0;

			const string str = StrParam;

				int Direct=(iParam >> 8)&0xFF;
				/*
					Direct:
						0 - от начала в конец списка;
						1 - от текущей позиции в начало;
						2 - от текущей позиции в конец списка пунктов меню.
				*/
				iParam&=0xFF;
				const auto StartPos=Direct?SelectPos:0;
				int EndPos=static_cast<int>(Items.size()-1);

				if (Direct == 1)
				{
					EndPos=0;
					Direct=-1;
				}
				else
				{
					Direct=1;
				}

				for (int I=StartPos; ;I+=Direct)
				{
					if (Direct > 0)
					{
						if (I > EndPos)
							break;
					}
					else
					{
						if (I < EndPos)
							break;
					}

					const auto& Item = at(I);

					if (!item_can_have_focus(Item))
						continue;

					int Res = 0;
					const auto strTemp = trim(HiText2Str(Item.Name));

					switch (iParam)
					{
						case 0: // full compare
							Res = equal_icase(strTemp, str);
							break;
						case 1: // begin compare
							Res = starts_with_icase(strTemp, str);
							break;
						case 2: // end compare
							Res = ends_with_icase(strTemp, str);
							break;
						case 3: // in str
							Res = contains_icase(strTemp, str);
							break;
					}

					if (Res)
					{
						SetSelectPos(I,1);

						DrawMenu();

						return GetVisualPos(SelectPos)+1;
					}
				}

			return 0;
		}

		case MCODE_F_MENU_GETHOTKEY:
		case MCODE_F_MENU_GETVALUE: // S=Menu.GetValue([N])
		{
			if (iParam == -1)
				iParam = SelectPos;
			else
				iParam = VisualPosToReal(static_cast<int>(iParam));

			if (iParam < 0 || iParam >= static_cast<long long>(Items.size()))
				return 0;

			const auto& menuEx = at(iParam);
			if (OpCode == MCODE_F_MENU_GETVALUE)
			{
				*static_cast<string*>(vParam) = menuEx.Name;
				return 1;
			}
			else
			{
				return GetHighlights(&menuEx);
			}
		}

		case MCODE_F_MENU_ITEMSTATUS: // N=Menu.ItemStatus([N])
		{
			if (iParam == -1)
				iParam = SelectPos;

			if (iParam < 0 || iParam >= static_cast<long long>(size()))
				return -1;

			const auto& menuEx = at(iParam);

			auto RetValue = menuEx.Flags;

			if (iParam == SelectPos)
				RetValue |= LIF_SELECTED;

			// Yes, it flips words.
			// Yes, it is intentional.
			// No, no idea why /o
			return make_integer<uint32_t>(
				extract_integer<WORD, 1>(RetValue),
				extract_integer<WORD, 0>(RetValue)
			);
		}

		case MCODE_V_MENU_VALUE: // Menu.Value
		{
			if (!HasVisible())
				return 0;
			*static_cast<string*>(vParam) = at(SelectPos).Name;
			return 1;
		}

		case MCODE_F_MENU_FILTER:
		{
			long long RetValue = 0;
			/*
			Action
			  0 - фильтр
			    Mode
			      -1 - (по умолчанию) вернуть 1 если фильтр уже включен, 0 - фильтр выключен
				   1 - включить фильтр, если фильтр уже включен - ничего не делает
				   0 - выключить фильтр
			  1 - фиксация текста фильтра
			    Mode
			      -1 - (по умолчанию) вернуть 1 если текст фильтра зафиксирован, 0 - фильтр можно менять с клавиатуры
				   1 - зафиксировать фильтр
				   0 - отменить фиксацию фильтра
			  2 - вернуть 1 если фильтр включен и строка фильтра не пуста
			  3 - вернуть количество отфильтрованных (невидимых) строк\
			  4 - (по умолчанию) подправить высоту списка под количество элементов
			*/
			const auto Parameter = std::bit_cast<intptr_t>(vParam);
			switch (iParam)
			{
				case 0:
					switch (Parameter)
					{
						case 0:
						case 1:
							if (bFilterEnabled != (Parameter == 1))
							{
								EnableFilter(Parameter == 1);
								DisplayObject();
							}
							RetValue = 1;
							break;

						case -1:
							RetValue = bFilterEnabled;
							break;
					}
					break;

				case 1:
					switch (Parameter)
					{
						case 0:
						case 1:
							bFilterLocked = Parameter == 1;
							DisplayObject();
							RetValue = 1;
							break;

						case -1:
							RetValue = bFilterLocked;
							break;
					}
					break;

				case 2:
					RetValue = bFilterEnabled && !strFilter.empty();
					break;

				case 3:
					// Don't use ItemHiddenCount here - it also includes invisible (LIF_HIDDEN), but non-filtered items
					RetValue = std::ranges::count_if(Items, [](const MenuItemEx& Item) { return (Item.Flags & LIF_FILTERED) != 0; });
					break;

				case 4:
					FilterUpdateHeight(true);
					DisplayObject();
					RetValue = 1;
					break;
			}
			return RetValue;
		}
		case MCODE_F_MENU_FILTERSTR:
		{
			/*
			Action
			  0 - (по умолчанию) вернуть текущую строку, если фильтр включен
			  1 - установить в фильтре строку S.
			      Если фильтр не был включен - включает его, режим фиксации не трогается, но игнорируется.
				  Возвращает предыдущее значение строки фильтра.
			*/
			switch (iParam)
			{
				case 0:
					if (bFilterEnabled)
					{
						*static_cast<string *>(vParam) = strFilter;
						return 1;
					}
					break;
				case 1:
					if (!bFilterEnabled)
						bFilterEnabled=true;
					const auto prevLocked = bFilterLocked;
					bFilterLocked = false;
					RestoreFilteredItems();
					auto oldFilter = std::move(strFilter);
					strFilter.clear();
					AddToFilter(*static_cast<const string*>(vParam));
					FilterStringUpdated();
					bFilterLocked = prevLocked;
					DisplayObject();
					*static_cast<string*>(vParam) = std::move(oldFilter);
					return 1;
			}

			return 0;
		}
		case MCODE_V_MENUINFOID:
		{
			static string strId;
			strId = uuid::str(MenuId);
			return std::bit_cast<intptr_t>(UNSAFE_CSTR(strId));
		}

	}

	return 0;
}

bool VMenu::AddToFilter(string_view const Str)
{
	if (!bFilterEnabled || bFilterLocked)
		return false;

	for (const auto Key: Str)
	{
		if (IsFilterEditKey(Key))
		{
			if (Key == KEY_BS && !strFilter.empty())
				strFilter.pop_back();
			else
				strFilter += Key;
		}
	}
	return true;
}

bool VMenu::ProcessFilterKey(int Key)
{
	if (!bFilterEnabled || bFilterLocked || !IsFilterEditKey(Key))
		return false;

	if (Key==KEY_BS)
	{
		if (!strFilter.empty())
		{
			strFilter.pop_back();

			if (strFilter.empty())
			{
				RestoreFilteredItems();
				DisplayObject();
				return true;
			}
		}
		else
		{
			return true;
		}
	}
	else
	{
		if (!GetShowItemCount())
			return true;

		strFilter += static_cast<wchar_t>(Key);
	}

	FilterStringUpdated();
	DisplayObject();

	return true;
}

bool VMenu::ProcessKey(const Manager::Key& Key)
{
	auto LocalKey = Key();
	auto Parent = GetDialog();
	if (IsComboBox() && !Parent->GetDropDownOpened())
	{
		Close(-1);
		return false;
	}

	if (IsComboBox() && CheckFlags(VMENU_COMBOBOXEVENTKEY))
	{
		auto Event = Key.Event();
		if (!Parent->DlgProc(DN_INPUT, 0, &Event))
			return true;
	}

	if (LocalKey == KEY_NONE)
		return false;

	if (IsComboBox() && CheckFlags(VMENU_COMBOBOXEVENTKEY))
	{
		auto Event = Key.Event();
		if (Parent->DlgProc(DN_CONTROLINPUT, Parent->GetDlgFocusPos(), &Event))
			return true;
	}

	if (LocalKey == KEY_OP_PLAINTEXT)
	{
		const auto str = Global->CtrlObject->Macro.GetStringToPrint();

		if (str.empty())
			return false;

		if ( AddToFilter(str) ) // для фильтра: всю строку целиком в фильтр, а там разберемся.
		{
			if (strFilter.empty())
				RestoreFilteredItems();
			else
				FilterStringUpdated();

			DisplayObject();

			return true;
		}
		else // не для фильтра: по старинке, первый символ последовательности, остальное игнорируем (ибо некуда)
			LocalKey = str.front();
	}

	SetMenuFlags(VMENU_UPDATEREQUIRED);

	if (!GetShowItemCount())
	{
		if (none_of(LocalKey, KEY_F1, KEY_SHIFTF1, KEY_F10, KEY_ESC, KEY_ALTF9, KEY_RALTF9))
		{
			if (!bFilterEnabled || none_of(LocalKey, KEY_BS, KEY_CTRLALTF, KEY_RCTRLRALTF, KEY_CTRLRALTF, KEY_RCTRLALTF, KEY_RALT, KEY_OP_XLAT))
			{
				m_ExitCode = -1;
				return false;
			}
		}
	}

	if (!((LocalKey >= KEY_MACRO_BASE && LocalKey <= KEY_MACRO_ENDBASE) || (LocalKey >= KEY_OP_BASE && LocalKey <= KEY_OP_ENDBASE)))
	{
		DWORD S=LocalKey&(KEY_CTRL|KEY_ALT|KEY_SHIFT|KEY_RCTRL|KEY_RALT);
		DWORD K=LocalKey&(~(KEY_CTRL|KEY_ALT|KEY_SHIFT|KEY_RCTRL|KEY_RALT));

		if (K==KEY_MULTIPLY)
			LocalKey = L'*'|S;
		else if (K==KEY_ADD)
			LocalKey = L'+'|S;
		else if (K==KEY_SUBTRACT)
			LocalKey = L'-'|S;
		else if (K==KEY_DIVIDE)
			LocalKey = L'/'|S;
	}

	const auto ProcessEnter = [this]()
	{
		if (item_can_be_entered(Items[SelectPos]))
		{
			if (IsComboBox())
			{
				Close(SelectPos);
			}
			else
			{
				SetExitCode(SelectPos);
			}
		}
	};

	const auto HScrollSmartPos = [&]
	{
		return any_of(LocalKey & ~KEY_CTRLMASK, KEY_HOME, KEY_NUMPAD7) ? 0 : -1;
	};

	const auto HScrollShiftSign = [&]
	{
		return any_of(LocalKey & ~KEY_CTRLMASK, KEY_LEFT, KEY_NUMPAD4, KEY_MSWHEEL_LEFT) ? 1 : -1;
	};

	switch (LocalKey)
	{
		case KEY_ALTF9:
		case KEY_RALTF9:
			Global->WindowManager->ProcessKey(Manager::Key(KEY_ALTF9));
			break;
		case KEY_NUMENTER:
		case KEY_ENTER:
		{
			if (!Parent || CheckFlags(VMENU_COMBOBOX))
				ProcessEnter();
			break;
		}
		case KEY_ESC:
		case KEY_F10:
		{
			if (IsComboBox())
			{
				Close(-1);
			}
			else if(!Parent)
			{
				SetExitCode(-1);
			}
			break;
		}
		case KEY_HOME:         case KEY_NUMPAD7:
		case KEY_CTRLHOME:     case KEY_CTRLNUMPAD7:
		case KEY_RCTRLHOME:    case KEY_RCTRLNUMPAD7:
		case KEY_CTRLPGUP:     case KEY_CTRLNUMPAD9:
		case KEY_RCTRLPGUP:    case KEY_RCTRLNUMPAD9:
		{
			FarListPos pos{ sizeof(pos), 0, -1 };
			SetSelectPos(&pos, 1);
			DrawMenu();
			break;
		}
		case KEY_END:          case KEY_NUMPAD1:
		case KEY_CTRLEND:      case KEY_CTRLNUMPAD1:
		case KEY_RCTRLEND:     case KEY_RCTRLNUMPAD1:
		case KEY_CTRLPGDN:     case KEY_CTRLNUMPAD3:
		case KEY_RCTRLPGDN:    case KEY_RCTRLNUMPAD3:
		{
			int p = static_cast<int>(Items.size())-1;
			FarListPos pos{ sizeof(pos), p, std::max(0, p - MaxHeight + 1) };
			SetSelectPos(&pos, -1);
			DrawMenu();
			break;
		}
		case KEY_PGUP:         case KEY_NUMPAD9:
		{
			const auto dy = m_Where.height() - (menu_layout::need_box(*this)? 2 : 1);

			int p = VisualPosToReal(GetVisualPos(SelectPos)-dy);

			if (p < 0)
				p = 0;

			FarListPos pos{ sizeof(pos), p, p };
			SetSelectPos(&pos, 1);
			DrawMenu();
			break;
		}
		case KEY_PGDN:         case KEY_NUMPAD3:
		{
			const auto dy = m_Where.height() - (menu_layout::need_box(*this)? 2 : 1);

			int pSel = VisualPosToReal(GetVisualPos(SelectPos)+dy);
			int pTop = VisualPosToReal(GetVisualPos(TopPos + 1));

			pSel = std::min(pSel, static_cast<int>(Items.size())-1);
			pTop = std::min(pTop, static_cast<int>(Items.size())-1);

			FarListPos pos{ sizeof(pos), pSel, pTop };
			SetSelectPos(&pos, -1);
			DrawMenu();
			break;
		}
		case KEY_ALTHOME:           case KEY_ALT|KEY_NUMPAD7:
		case KEY_RALTHOME:          case KEY_RALT|KEY_NUMPAD7:
		case KEY_ALTEND:            case KEY_ALT|KEY_NUMPAD1:
		case KEY_RALTEND:           case KEY_RALT|KEY_NUMPAD1:
		{
			if (SetAllItemsSmartHPos(HScrollSmartPos()))
				DrawMenu();

			break;
		}
		case KEY_ALTSHIFTHOME:      case KEY_ALTSHIFT|KEY_NUMPAD7:
		case KEY_ALTSHIFTEND:       case KEY_ALTSHIFT|KEY_NUMPAD1:
		{
			if (SetCurItemSmartHPos(HScrollSmartPos()))
				DrawMenu();

			break;
		}
		case KEY_ALTLEFT:   case KEY_ALT|KEY_NUMPAD4:  case KEY_MSWHEEL_LEFT:
		case KEY_RALTLEFT:  case KEY_RALT|KEY_NUMPAD4:
		case KEY_ALTRIGHT:  case KEY_ALT|KEY_NUMPAD6:  case KEY_MSWHEEL_RIGHT:
		case KEY_RALTRIGHT: case KEY_RALT|KEY_NUMPAD6:
		{
			if (ShiftAllItemsHPos(HScrollShiftSign()))
				DrawMenu();

			break;
		}
		case KEY_CTRLALTLEFT:   case KEY_CTRLALTNUMPAD4:  case KEY_CTRL|KEY_MSWHEEL_LEFT:
		case KEY_CTRLRALTLEFT:  case KEY_CTRLRALTNUMPAD4:
		case KEY_CTRLALTRIGHT:  case KEY_CTRLALTNUMPAD6:  case KEY_CTRL|KEY_MSWHEEL_RIGHT:
		case KEY_CTRLRALTRIGHT: case KEY_CTRLRALTNUMPAD6:
		{
			if (ShiftAllItemsHPos(20 * HScrollShiftSign()))
				DrawMenu();

			break;
		}
		case KEY_ALTSHIFTLEFT:      case KEY_ALTSHIFTNUMPAD4:
		case KEY_RALTSHIFTLEFT:     case KEY_RALTSHIFTNUMPAD4:
		case KEY_ALTSHIFTRIGHT:     case KEY_ALTSHIFTNUMPAD6:
		case KEY_RALTSHIFTRIGHT:    case KEY_RALTSHIFTNUMPAD6:
		{
			if (ShiftCurItemHPos(HScrollShiftSign()))
				DrawMenu();

			break;
		}
		case KEY_CTRLSHIFTLEFT:     case KEY_CTRLSHIFTNUMPAD4:
		case KEY_CTRLSHIFTRIGHT:    case KEY_CTRLSHIFTNUMPAD6:
		{
			if (ShiftCurItemHPos(20 * HScrollShiftSign()))
				DrawMenu();

			break;
		}
		case KEY_CTRLNUMPAD5:
		case KEY_RCTRLNUMPAD5:
		{
			if (AlignAnnotations())
				DrawMenu();

			break;
		}
		case KEY_MSWHEEL_UP:
		{
			SetSelectPos(SelectPos - 1, -1, true);
			DrawMenu();
			break;
		}
		case KEY_MSWHEEL_DOWN:
		{
			SetSelectPos(SelectPos + 1, 1, true);
			DrawMenu();
			break;
		}

		case KEY_LEFT:         case KEY_NUMPAD4:
		case KEY_UP:           case KEY_NUMPAD8:
		{
			SetSelectPos(SelectPos-1,-1,IsRepeatedKey());
			DrawMenu();
			break;
		}

		case KEY_RIGHT:        case KEY_NUMPAD6:
		case KEY_DOWN:         case KEY_NUMPAD2:
		{
			SetSelectPos(SelectPos+1,1,IsRepeatedKey());
			DrawMenu();
			break;
		}

		case KEY_RALT:
		case KEY_CTRLALTF:
		case KEY_RCTRLRALTF:
		case KEY_CTRLRALTF:
		case KEY_RCTRLALTF:
		{
			EnableFilter(!bFilterEnabled);
			DisplayObject();
			break;
		}
		case KEY_CTRLV:
		case KEY_RCTRLV:
		case KEY_SHIFTINS:    case KEY_SHIFTNUMPAD0:
		{
			if (bFilterEnabled && !bFilterLocked)
			{
				string ClipText;
				if (!GetClipboardText(ClipText))
					return true;

				if (AddToFilter(ClipText))
				{
					if (strFilter.empty())
						RestoreFilteredItems();
					else
						FilterStringUpdated();

					DisplayObject();
				}
			}
			return true;
		}
		case KEY_CTRLALTL:
		case KEY_RCTRLRALTL:
		case KEY_CTRLRALTL:
		case KEY_RCTRLALTL:
			if (bFilterEnabled)
			{
				bFilterLocked=!bFilterLocked;
				DisplayObject();
			}
			break;

		case KEY_OP_XLAT:
		{
			if (bFilterEnabled && !bFilterLocked)
			{
				int start = static_cast<int>(strFilter.size());
				bool DoXlat = true;

				if (IsWordDiv(Global->Opt->XLat.strWordDivForXlat, strFilter[start]))
				{
					if (start) start--;
					DoXlat = !IsWordDiv(Global->Opt->XLat.strWordDivForXlat, strFilter[start]);
				}

				if (DoXlat)
				{
					while (start >= 0 && !IsWordDiv(Global->Opt->XLat.strWordDivForXlat, strFilter[start]))
						start--;

					start++;
					Xlat(std::span(strFilter).subspan(start), Global->Opt->XLat.Flags);
					FilterStringUpdated();
					DisplayObject();
				}
			}
			break;
		}
		case KEY_TAB:
			if (IsComboBox())
			{
				ProcessEnter();
				break;
			}
			[[fallthrough]];
		default:
		{
			if (ProcessFilterKey(LocalKey))
				return true;

			int OldSelectPos=SelectPos;
			int NewPos=SelectPos;

			bool IsHotkey=true;
			if (!CheckKeyHiOrAcc(LocalKey, 0, false, !(Parent && !Parent->CheckDialogMode(DMODE_ISMENU) && CheckFlags(VMENU_COMBOBOX | VMENU_LISTBOX)), NewPos))
			{
				if (any_of(LocalKey, KEY_F1, KEY_SHIFTF1))
				{
					if (Parent)
						;//ParentDialog->ProcessKey(Key);
					else
						ShowHelp();

					break;
				}
				else
				{
					if (!CheckKeyHiOrAcc(LocalKey,1,FALSE,!(Parent && !Parent->CheckDialogMode(DMODE_ISMENU) && CheckFlags(VMENU_COMBOBOX|VMENU_LISTBOX)),NewPos))
						if (!CheckKeyHiOrAcc(LocalKey,1,TRUE,!(Parent && !Parent->CheckDialogMode(DMODE_ISMENU) && CheckFlags(VMENU_COMBOBOX|VMENU_LISTBOX)),NewPos))
							IsHotkey=false;
				}
			}

			if (IsHotkey && Parent)
			{
				if (Parent->SendMessage(DN_LISTHOTKEY,DialogItemID,ToPtr(NewPos)))
				{
					DrawMenu();
					ClearDone();
					break;
				}
				else
				{
					if (NewPos != OldSelectPos && SetSelectPos(NewPos, 1) < 0)
					{
						ClearDone();
					}
					else
					{
						CheckFlags(VMENU_COMBOBOX) ? Close(GetExitCode()) : ClearDone();
					}

					break;
				}
			}

			return false;
		}
	}

	return true;
}

bool VMenu::ClickHandler(window* Menu, int const MenuClick)
{
	switch (MenuClick)
	{
	case VMENUCLICK_APPLY:
		return Menu->ProcessKey(Manager::Key(KEY_ENTER));

	case VMENUCLICK_CANCEL:
		return Menu->ProcessKey(Manager::Key(KEY_ESC));

	case VMENUCLICK_IGNORE:
	default:
		return true;
	}
}

bool VMenu::ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent)
{
	if (IsMouseButtonEvent(MouseEvent->dwEventFlags) && MouseEvent->dwButtonState && !m_Where.contains(MouseEvent->dwMousePosition))
	{
		const auto NewButtonState = MouseEvent->dwButtonState & ~IntKeyState.PrevMouseButtonState;
		if (NewButtonState & FROM_LEFT_1ST_BUTTON_PRESSED)
			return ClickHandler(this, Global->Opt->VMenu.LBtnClick);
		else if (NewButtonState & FROM_LEFT_2ND_BUTTON_PRESSED)
			return ClickHandler(this, Global->Opt->VMenu.MBtnClick);
		else if (NewButtonState & RIGHTMOST_BUTTON_PRESSED)
			return ClickHandler(this, Global->Opt->VMenu.RBtnClick);
	}

	const auto Parent = GetDialog();
	if (IsComboBox() && !Parent->GetDropDownOpened())
	{
		Close(-1);
		return false;
	}

	SetMenuFlags(VMENU_UPDATEREQUIRED);

	if (IsComboBox() && CheckFlags(VMENU_COMBOBOXEVENTMOUSE))
	{
		INPUT_RECORD Event{ MOUSE_EVENT };
		Event.Event.MouseEvent = *MouseEvent;
		if (!Parent->DlgProc(DN_INPUT, 0, &Event))
			return true;
	}

	if (!GetShowItemCount())
	{
		if (MouseEvent->dwButtonState && IsMouseButtonEvent(MouseEvent->dwEventFlags))
			SetExitCode(-1);
		return false;
	}

	const int MsX{ MouseEvent->dwMousePosition.X };
	const int MsY{ MouseEvent->dwMousePosition.Y };
	const auto NeedBox{ menu_layout::need_box(*this) };

	// необходимо знать, что RBtn был нажат ПОСЛЕ появления VMenu, а не до
	if ((MouseEvent->dwButtonState & RIGHTMOST_BUTTON_PRESSED) && IsMouseButtonEvent(MouseEvent->dwEventFlags))
		bRightBtnPressed=true;

	if ((MouseEvent->dwButtonState & FROM_LEFT_2ND_BUTTON_PRESSED) && IsMouseButtonEvent(MouseEvent->dwEventFlags))
	{
		if (NeedBox?
			MsX > m_Where.left && MsX < m_Where.right && MsY > m_Where.top && MsY < m_Where.bottom :
			MsX >= m_Where.left && MsX <= m_Where.right && MsY >= m_Where.top && MsY <= m_Where.bottom)
		{
			ProcessKey(Manager::Key(KEY_ENTER));
		}
		return true;
	}

	if ((MouseEvent->dwButtonState&FROM_LEFT_1ST_BUTTON_PRESSED) && (MsX == m_Where.left + 2 || MsX == m_Where.right - 1 - (CheckFlags(VMENU_COMBOBOX | VMENU_LISTBOX) ? 0 : 2)))
	{
		// Click [«] or [»]
		while_mouse_button_pressed([&](DWORD const Button)
		{
			if (Button != FROM_LEFT_1ST_BUTTON_PRESSED)
				return false;

			ProcessKey(Manager::Key(MsX == m_Where.left + 2? KEY_ALTLEFT : KEY_ALTRIGHT));
			return true;
		});
		return true;
	}

	const auto SbY1 = m_Where.top + (NeedBox? 1 : 0);
	const auto SbY2 = m_Where.bottom - (NeedBox? 1 : 0);
	bool bShowScrollBar = false;

	if (CheckFlags(VMENU_LISTBOX|VMENU_ALWAYSSCROLLBAR) || Global->Opt->ShowMenuScrollbar)
		bShowScrollBar = true;

	if (bShowScrollBar && MsX == m_Where.right && (m_Where.height() - (NeedBox? 2 : 0)) < static_cast<int>(Items.size()) && (MouseEvent->dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED))
	{
		const auto WrapState = CheckFlags(VMENU_WRAPMODE);
		if (WrapState)
			ClearFlags(VMENU_WRAPMODE);
		SCOPE_EXIT{ if (WrapState) SetMenuFlags(VMENU_WRAPMODE); };

		if (MsY==SbY1)
		{
			// Press and hold the [▲] button
			while_mouse_button_pressed([&](DWORD const Button)
			{
				if (Button != FROM_LEFT_1ST_BUTTON_PRESSED)
					return false;

				ProcessKey(Manager::Key(KEY_UP));
				DrawMenu();
				return true;
			});

			return true;
		}

		if (MsY==SbY2)
		{
			// Press and hold the [▼] button
			while_mouse_button_pressed([&](DWORD const Button)
			{
				if (Button != FROM_LEFT_1ST_BUTTON_PRESSED)
					return false;

				ProcessKey(Manager::Key(KEY_DOWN));
				DrawMenu();
				return true;
			});

			return true;
		}

		if (MsY>SbY1 && MsY<SbY2)
		{
			// Drag the thumb
			int Delta=0;

			while (IsMouseButtonPressed() == FROM_LEFT_1ST_BUTTON_PRESSED)
			{
				const auto SbHeight = m_Where.height() - 3;
				int MsPos = (GetShowItemCount() - 1) * (IntKeyState.MousePos.y - m_Where.top) / SbHeight;

				if (MsPos >= GetShowItemCount())
				{
					MsPos=GetShowItemCount()-1;
					Delta=-1;
				}

				if (MsPos < 0)
				{
					MsPos=0;
					Delta=1;
				}


				SetSelectPos(VisualPosToReal(MsPos),Delta);

				DrawMenu();
			}

			return true;
		}
	}

	// dwButtonState & 3 - Left & Right button
	if (NeedBox && (MouseEvent->dwButtonState & 3) && MsX > m_Where.left && MsX < m_Where.right)
	{
		const auto WrapState = CheckFlags(VMENU_WRAPMODE);
		if (WrapState)
			ClearFlags(VMENU_WRAPMODE);
		SCOPE_EXIT{ if (WrapState) SetMenuFlags(VMENU_WRAPMODE); };

		if (MsY == m_Where.top)
		{
			while_mouse_button_pressed([&](DWORD)
			{
				if (MsY != m_Where.top || !GetVisualPos(SelectPos))
					return false;

				ProcessKey(Manager::Key(KEY_UP));
				return true;
			});

			return true;
		}

		if (MsY == m_Where.bottom)
		{
			while_mouse_button_pressed([&](DWORD)
			{
				if (MsY != m_Where.bottom || GetVisualPos(SelectPos) == GetShowItemCount() - 1)
					return false;

				ProcessKey(Manager::Key(KEY_DOWN));
				return true;
			});

			return true;
		}
	}

	if (NeedBox?
		MsX > m_Where.left && MsX < m_Where.right && MsY > m_Where.top && MsY < m_Where.bottom :
		MsX >= m_Where.left && MsX <= m_Where.right && MsY >= m_Where.top && MsY <= m_Where.bottom)
	{
		const auto MsPos = VisualPosToReal(GetVisualPos(TopPos) + MsY - m_Where.top - (NeedBox? 1 : 0));

		if (MsPos>=0 && MsPos<static_cast<int>(Items.size()) && item_can_have_focus(Items[MsPos]))
		{
			if (IntKeyState.MousePos.x != IntKeyState.MousePrevPos.x || IntKeyState.MousePos.y != IntKeyState.MousePrevPos.y || IsMouseButtonEvent(MouseEvent->dwEventFlags))
			{
				/* TODO:

				   Это заготовка для управления поведением листов "не в стиле меню" - когда текущий
				   указатель списка (позиция) следит за мышой...

				        if(!CheckFlags(VMENU_LISTBOX|VMENU_COMBOBOX) && MouseEvent->dwEventFlags==MOUSE_MOVED ||
				            CheckFlags(VMENU_LISTBOX|VMENU_COMBOBOX) && MouseEvent->dwEventFlags!=MOUSE_MOVED)
				*/
				const auto MouseTrackingEnabled = CheckFlags(VMENU_MOUSEREACTION);
				const auto IsMouseMoved = MouseEvent->dwEventFlags == MOUSE_MOVED;

				if ((MouseTrackingEnabled == IsMouseMoved)
			        ||
			        (MouseEvent->dwButtonState & (FROM_LEFT_1ST_BUTTON_PRESSED|RIGHTMOST_BUTTON_PRESSED))
				   )
				{
					SetSelectPos(MsPos,1);
				}

				DrawMenu();
			}

			/* $ 13.10.2001 VVM
			  + Запомнить нажатие клавиши мышки и только в этом случае реагировать при отпускании */
			if (IsMouseButtonEvent(MouseEvent->dwEventFlags) && (MouseEvent->dwButtonState & (FROM_LEFT_1ST_BUTTON_PRESSED|RIGHTMOST_BUTTON_PRESSED)))
				SetMenuFlags(VMENU_MOUSEDOWN);

			if (IsMouseButtonEvent(MouseEvent->dwEventFlags) && !(MouseEvent->dwButtonState & (FROM_LEFT_1ST_BUTTON_PRESSED|RIGHTMOST_BUTTON_PRESSED)) && CheckFlags(VMENU_MOUSEDOWN))
			{
				ClearFlags(VMENU_MOUSEDOWN);
				ProcessKey(Manager::Key(KEY_ENTER));
			}
		}

		return true;
	}

	return false;
}

int VMenu::GetVisualPos(int Pos) const
{
	if (!ItemHiddenCount)
		return Pos;

	if (Pos < 0)
		return -1;

	if (Pos >= static_cast<int>(Items.size()))
		return GetShowItemCount();

	return std::count_if(Items.cbegin(), Items.cbegin() + Pos, [](const auto& Item) { return item_is_visible(Item); });
}

int VMenu::VisualPosToReal(int VPos) const
{
	if (!ItemHiddenCount)
		return VPos;

	if (VPos < 0)
		return -1;

	if (VPos >= GetShowItemCount())
		return static_cast<int>(Items.size());

	const auto ItemIterator = std::ranges::find_if(Items, [&](MenuItemEx const& i){ return item_is_visible(i) && !VPos--; });
	return ItemIterator != Items.cend()? ItemIterator - Items.cbegin() : -1;
}

bool VMenu::SetItemHPos(MenuItemEx& Item, const auto& GetNewHPos)
{
	if (Item.Flags & LIF_SEPARATOR) return false;

	const auto ItemLength{ get_item_visual_length(CheckFlags(VMENU_SHOWAMPERSAND), Item.Name) };
	if (ItemLength <= 0) return false;

	const auto NewHPos = [&]
	{
		if constexpr (requires { GetNewHPos(Item); })
			return GetNewHPos(Item);
		if constexpr (requires { GetNewHPos(ItemLength); })
			return GetNewHPos(ItemLength);
		if constexpr (requires { GetNewHPos(Item.HorizontalPosition, ItemLength); })
			return GetNewHPos(Item.HorizontalPosition, ItemLength);
	}();

	UpdateAllItemsBoundaries(NewHPos, ItemLength);

	if (Item.HorizontalPosition == NewHPos) return false;

	Item.HorizontalPosition = NewHPos;
	return true;
}

bool VMenu::SetCurItemSmartHPos(const int NewHPos)
{
	const auto TextAreaWidth{ CalculateTextAreaWidth() };
	if (TextAreaWidth <= 0) return false;

	const auto Policy{ CheckFlags(VMENU_ENABLEALIGNANNOTATIONS) ? item_hscroll_policy::cling_to_edge : item_hscroll_policy::bound_stick_to_left };

	if (SetItemHPos(
		Items[SelectPos],
		[&](const int ItemLength) { return get_item_smart_hpos(NewHPos, ItemLength, TextAreaWidth, Policy); }))
	{
		SetMenuFlags(VMENU_UPDATEREQUIRED);
		return true;
	}

	return false;
}

bool VMenu::ShiftCurItemHPos(const int Shift)
{
	const auto TextAreaWidth{ CalculateTextAreaWidth() };
	if (TextAreaWidth <= 0) return false;

	const auto Policy{ CheckFlags(VMENU_ENABLEALIGNANNOTATIONS) ? item_hscroll_policy::cling_to_edge : item_hscroll_policy::bound_stick_to_left };

	if (SetItemHPos(
		Items[SelectPos],
		[&](const int ItemHPos, const int ItemLength) { return get_item_absolute_hpos(ItemHPos + Shift, ItemLength, TextAreaWidth, Policy); }))
	{
		SetMenuFlags(VMENU_UPDATEREQUIRED);
		return true;
	}

	return false;
}

bool VMenu::SetAllItemsHPos(const auto& GetNewHPos)
{
	ResetAllItemsBoundaries();
	bool NeedRedraw{};

	for (auto& Item : Items)
		NeedRedraw |= SetItemHPos(Item, GetNewHPos);

	if (NeedRedraw) SetMenuFlags(VMENU_UPDATEREQUIRED);
	return NeedRedraw;
}

bool VMenu::SetAllItemsSmartHPos(const int NewHPos)
{
	const auto TextAreaWidth{ CalculateTextAreaWidth() };
	if (TextAreaWidth <= 0) return false;

	const auto Policy{ CheckFlags(VMENU_ENABLEALIGNANNOTATIONS) ? item_hscroll_policy::unbound : item_hscroll_policy::bound_stick_to_left };

	return SetAllItemsHPos(
		[&](const int ItemLength) { return get_item_smart_hpos(NewHPos, ItemLength, TextAreaWidth, Policy); });
}

bool VMenu::ShiftAllItemsHPos(const int Shift)
{
	const auto TextAreaWidth{ CalculateTextAreaWidth() };
	if (TextAreaWidth <= 0) return false;

	const auto AdjustedShift{ adjust_hpos_shift(Shift, m_AllItemsBoundaries.first, m_AllItemsBoundaries.second, TextAreaWidth) };
	if (!AdjustedShift) return false;

	const auto Policy{ CheckFlags(VMENU_ENABLEALIGNANNOTATIONS) ? item_hscroll_policy::unbound : item_hscroll_policy::bound_stick_to_left };

	return SetAllItemsHPos(
		[&](const int ItemHPos, const int ItemLength) { return get_item_absolute_hpos(ItemHPos + AdjustedShift, ItemLength, TextAreaWidth, Policy); });
}

bool VMenu::AlignAnnotations()
{
	if (!CheckFlags(VMENU_ENABLEALIGNANNOTATIONS)) return false;

	const auto TextAreaWidth{ CalculateTextAreaWidth() };
	if (TextAreaWidth <= 0 || TextAreaWidth + 2 <= 0) return false;
	const auto AlignPos{ (TextAreaWidth + 2) / 4 };

	return SetAllItemsHPos(
		[&](const MenuItemEx& Item) { return Item.Annotations.empty() ? 0 : AlignPos - Item.Annotations.front().first; });
}

void VMenu::Show()
{
	const auto BoxType{ menu_layout::get_box_type(*this) };
	const auto ServiceAreaSize = menu_layout::get_service_area_size(*this, BoxType);

	if (!CheckFlags(VMENU_LISTBOX))
	{
		bool AutoHeight = false;

		if (!CheckFlags(VMENU_COMBOBOX))
		{
			const auto VisibleMaxItemLength = std::min(ScrX > ServiceAreaSize? ScrX - ServiceAreaSize : 0, m_MaxItemLength);
			const auto MenuWidth = ServiceAreaSize + VisibleMaxItemLength;

			bool AutoCenter = false;

			if (m_Where.left == -1)
			{
				m_Where.left = static_cast<short>(ScrX - MenuWidth) / 2;
				AutoCenter = true;
			}

			if (m_Where.left < 2)
				m_Where.left = 2;

			if (m_Where.right <= 0)
				m_Where.right = static_cast<short>(m_Where.left + MenuWidth);

			if (!AutoCenter && m_Where.right > ScrX-4+2*(BoxType==SHORT_DOUBLE_BOX || BoxType==SHORT_SINGLE_BOX))
			{
				m_Where.left += ScrX - 4 - m_Where.right;
				m_Where.right = ScrX - 4;

				if (m_Where.left < 2)
				{
					m_Where.left = 2;
					m_Where.right = ScrX - 2;
				}
			}

			if (m_Where.right > ScrX - 2)
				m_Where.right = ScrX - 2;

			if (m_Where.top == -1)
			{
				if (MaxHeight && MaxHeight<GetShowItemCount())
					m_Where.top = (ScrY - MaxHeight - 2) / 2;
				else if ((m_Where.top = (ScrY - GetShowItemCount() - 2) / 2) < 0)
					m_Where.top = 0;

				AutoHeight=true;
			}
		}

		WasAutoHeight = false;
		if (m_Where.bottom <= 0)
		{
			WasAutoHeight = true;
			if (MaxHeight && MaxHeight<GetShowItemCount())
				m_Where.bottom = m_Where.top + MaxHeight + 1;
			else
				m_Where.bottom = m_Where.top + GetShowItemCount() + 1;
		}

		if (m_Where.bottom > ScrY)
			m_Where.bottom = ScrY;

		if (AutoHeight && m_Where.top < 3 && m_Where.bottom > ScrY - 3)
		{
			m_Where.top = 2;
			m_Where.bottom = ScrY - 2;
		}
	}

	if (m_Where.right > m_Where.left && m_Where.bottom + (CheckFlags(VMENU_SHOWNOBOX)? 1 : 0) > m_Where.top)
	{
		if (!CheckFlags(VMENU_LISTBOX))
		{
			ScreenObjectWithShadow::Show();
		}
		else
		{
			SetMenuFlags(VMENU_UPDATEREQUIRED);
			DisplayObject();
		}
	}
}

void VMenu::Hide()
{
	if (!CheckFlags(VMENU_LISTBOX) && SaveScr)
	{
		SaveScr.reset();
		ScreenObjectWithShadow::Hide();
	}

	SetMenuFlags(VMENU_UPDATEREQUIRED);
}

void VMenu::DisplayObject()
{
	const auto Parent = GetDialog();
	if (Parent && !Parent->IsRedrawEnabled()) return;
	ClearFlags(VMENU_UPDATEREQUIRED);

	if (CheckFlags(VMENU_REFILTERREQUIRED)!=0)
	{
		if (bFilterEnabled)
		{
			RestoreFilteredItems();
			if (!strFilter.empty())
				FilterStringUpdated();
		}
		ClearFlags(VMENU_REFILTERREQUIRED);
	}

	HideCursor();

	const auto BoxType{ menu_layout::get_box_type(*this) };

	if (!CheckFlags(VMENU_LISTBOX) && !SaveScr)
	{
		if (!CheckFlags(VMENU_DISABLEDRAWBACKGROUND) && !(BoxType==SHORT_DOUBLE_BOX || BoxType==SHORT_SINGLE_BOX))
			SaveScr = std::make_unique<SaveScreen>(rectangle{ m_Where.left - 2, m_Where.top - 1, m_Where.right + 4, m_Where.bottom + 2 });
		else
			SaveScr = std::make_unique<SaveScreen>(rectangle{ m_Where.left, m_Where.top, m_Where.right + 2, m_Where.bottom + 1 });
	}

	if (!CheckFlags(VMENU_DISABLEDRAWBACKGROUND) && !CheckFlags(VMENU_LISTBOX))
	{
		// BUGBUG, dead code -- 2023-07-08 MZK: I've got here when pressed hotkey of "Select search area" combobox on "find file" dialog.
		// It may be related to the CheckFlags(VMENU_DISABLEDRAWBACKGROUND) part of the condition only. Need to investigate.

		if (BoxType==SHORT_DOUBLE_BOX || BoxType==SHORT_SINGLE_BOX)
		{
			SetScreen(m_Where, L' ', get_color(Colors, color_indices::Body));
			Box(m_Where, get_color(Colors, color_indices::Box), BoxType);
		}
		else
		{
			if (BoxType!=NO_BOX)
				SetScreen({ m_Where.left - 2, m_Where.top - 1, m_Where.right + 2, m_Where.bottom + 1 }, L' ', get_color(Colors, color_indices::Body));
			else
				SetScreen(m_Where, L' ', get_color(Colors, color_indices::Body));

			if (BoxType!=NO_BOX)
				Box(m_Where, get_color(Colors, color_indices::Box), BoxType);
		}

		//SetMenuFlags(VMENU_DISABLEDRAWBACKGROUND);
	}

	if (!CheckFlags(VMENU_LISTBOX))
		DrawTitles();

	DrawMenu();
}

void VMenu::DrawMenu()
{
	const menu_layout Layout{ *this };

	if (m_Where.right <= m_Where.left || m_Where.bottom <= m_Where.top)
	{
		if (!(CheckFlags(VMENU_SHOWNOBOX) && m_Where.bottom == m_Where.top))
			return;
	}

	// 2023-12-09 MZK: Do we need this? Why?
	// 2023-12-28 MZK: OK, at least combo box maybe refreshed without DisplayObject(); then vertical borders are garbled by separators.
	if (CheckFlags(VMENU_LISTBOX | VMENU_COMBOBOX))
	{
		if (!GetShowItemCount())
			SetScreen(m_Where, L' ', get_color(Colors, color_indices::Body));

		if (Layout.BoxType!=NO_BOX)
			Box(m_Where, get_color(Colors, color_indices::Box), Layout.BoxType);

		DrawTitles();
	}

	if (GetShowItemCount() <= 0)
		return;

	if (CheckFlags(VMENU_AUTOHIGHLIGHT))
		AssignHighlights(CheckFlags(VMENU_REVERSEHIGHLIGHT));

	const auto VisualTopPos{ AdjustTopPos(Layout.BoxType) };

	if (Layout.ClientRect.width() <= 0)
		return;

	std::vector<int> HighlightMarkup;
	const string BlankLine(Layout.ClientRect.width(), L' ');

	for (int Y = Layout.ClientRect.top, I = TopPos; Y <= Layout.ClientRect.bottom; ++Y, ++I)
	{
		if (I >= static_cast<int>(Items.size()))
		{
			GotoXY(Layout.ClientRect.left, Y);
			set_color(Colors, color_indices::Text);
			Text(BlankLine);
			continue;
		}

		if (!item_is_visible(Items[I]))
		{
			Y--;
			continue;
		}

		if (Items[I].Flags & LIF_SEPARATOR)
		{
			DrawSeparator(I, Layout.BoxType, Y);
			continue;
		}

		DrawRegularItem(Items[I], Layout, Y, HighlightMarkup, BlankLine);
	}

	if (Layout.Scrollbar)
	{
		set_color(Colors, color_indices::ScrollBar);
		ScrollBar(Layout.Scrollbar.value(), Layout.ClientRect.top, Layout.ClientRect.height(), VisualTopPos, GetShowItemCount());
	}
}

void VMenu::DrawTitles() const
{
	if (CheckFlags(VMENU_SHOWNOBOX)) return;

	const auto MaxTitleLength = m_Where.width() - 3;

	if (!strTitle.empty() || bFilterEnabled)
	{
		string strDisplayTitle = strTitle;

		if (bFilterEnabled)
		{
			if (bFilterLocked)
				strDisplayTitle += L' ';
			else
				strDisplayTitle.clear();

			append(strDisplayTitle, bFilterLocked? L'<' : L'[', strFilter, bFilterLocked? L'>' : L']');
		}

		auto WidthTitle = static_cast<int>(strDisplayTitle.size());

		if (WidthTitle > MaxTitleLength)
			WidthTitle = MaxTitleLength - 1;

		GotoXY(m_Where.left + (m_Where.width() - 2 - WidthTitle) / 2, m_Where.top);
		set_color(Colors, color_indices::Title);

		Text(concat(L' ', string_view(strDisplayTitle).substr(0, WidthTitle), L' '));
	}

	if (!strBottomTitle.empty())
	{
		auto WidthTitle = static_cast<int>(strBottomTitle.size());

		if (WidthTitle > MaxTitleLength)
			WidthTitle = MaxTitleLength - 1;

		GotoXY(m_Where.left + (m_Where.width() - 2 - WidthTitle) / 2, m_Where.bottom);
		set_color(Colors, color_indices::Title);

		Text(concat(L' ', string_view(strBottomTitle).substr(0, WidthTitle), L' '));
	}

	if constexpr ((false))
	{
		set_color(Colors, color_indices::Title);

		const auto AllItemsBoundariesLabel{ std::format(L" [{}, {}] ", m_AllItemsBoundaries.first, m_AllItemsBoundaries.second) };
		GotoXY(m_Where.left + 2, m_Where.bottom);
		Text(AllItemsBoundariesLabel);

		const auto TextAreaWidthLabel{ std::format(L" [{}] ", CalculateTextAreaWidth()) };
		GotoXY(m_Where.right - 1 - static_cast<int>(TextAreaWidthLabel.size()), m_Where.bottom);
		Text(TextAreaWidthLabel);
	}
}

int VMenu::AdjustTopPos(const int BoxType)
{
	// 2023-12-26 MZK: The magik here is beyond my comprehension

	int VisualSelectPos = GetVisualPos(SelectPos);
	int VisualTopPos = GetVisualPos(TopPos);

	// коррекция Top`а
	if (VisualTopPos+GetShowItemCount() >= m_Where.height() - 1 && VisualSelectPos == GetShowItemCount()-1)
	{
		VisualTopPos--;

		if (VisualTopPos<0)
			VisualTopPos=0;
	}

	// 2023-12-26 MZK: (m_Where.height() - 2 - (BoxType == NO_BOX ? 2 : 0)) == (ClientRect.height() - (BoxType == NO_BOX ? 4 : 0))
	VisualTopPos = std::min(VisualTopPos, GetShowItemCount() - (m_Where.height() - 2 - (BoxType == NO_BOX ? 2 : 0)));

	// 2023-12-26 MZK: (m_Where.height() - 1 - (BoxType == NO_BOX ? 0 : 2)) == (ClientRect.height() - 1)
	if (VisualSelectPos > VisualTopPos + (m_Where.height() - 1 - (BoxType == NO_BOX ? 0 : 2)))
	{
		VisualTopPos = VisualSelectPos - (m_Where.height() - 1 - (BoxType == NO_BOX ? 0 : 2));
	}

	if (VisualSelectPos < VisualTopPos)
	{
		TopPos=SelectPos;
		VisualTopPos=VisualSelectPos;
	}
	else
	{
		TopPos=VisualPosToReal(VisualTopPos);
	}

	if (VisualTopPos<0)
		VisualTopPos=0;

	if (TopPos<0)
		TopPos=0;

	return VisualTopPos;
}

void VMenu::DrawSeparator(const size_t ItemIndex, const int BoxType, const int Y) const
{
	auto separator{ MakeLine(
		m_Where.width(),
		BoxType == NO_BOX
			? line_type::h1_to_none
			: (BoxType == SINGLE_BOX || BoxType == SHORT_SINGLE_BOX? line_type::h1_to_v1 : line_type::h1_to_v2)) };

	ConnectSeparator(ItemIndex, separator, BoxType);
	ApplySeparatorName(Items[ItemIndex], separator);
	set_color(Colors, color_indices::Separator);
	GotoXY(m_Where.left, Y);
	Text(separator);
}

void VMenu::ConnectSeparator(const size_t ItemIndex, string& separator, const int BoxType) const
{
	if (CheckFlags(VMENU_NOMERGEBORDER) || separator.size() <= 3)
		return;

	for (const auto I : std::views::iota(0uz, separator.size() - 3))
	{
		const auto AnyPrev = ItemIndex > 0;
		const auto AnyNext = ItemIndex < Items.size() - 1;

		const auto PCorrection = AnyPrev && !CheckFlags(VMENU_SHOWAMPERSAND)? HiFindRealPos(Items[ItemIndex - 1].Name, I) - I : 0;
		const auto NCorrection = AnyNext && !CheckFlags(VMENU_SHOWAMPERSAND)? HiFindRealPos(Items[ItemIndex + 1].Name, I) - I : 0;

		wchar_t PrevItem = (AnyPrev && Items[ItemIndex - 1].Name.size() > I + PCorrection)? Items[ItemIndex - 1].Name[I + PCorrection] : 0;
		wchar_t NextItem = (AnyNext && Items[ItemIndex + 1].Name.size() > I + NCorrection)? Items[ItemIndex + 1].Name[I + NCorrection] : 0;

		if (!PrevItem && !NextItem)
			break;

		if (PrevItem == BoxSymbols[BS_V1])
		{
			if (NextItem == BoxSymbols[BS_V1])
				separator[I + (BoxType == NO_BOX?1:2) + 1] = BoxSymbols[BS_C_H1V1];
			else
				separator[I + (BoxType == NO_BOX?1:2) + 1] = BoxSymbols[BS_B_H1V1];
		}
		else if (NextItem == BoxSymbols[BS_V1])
		{
			separator[I + (BoxType == NO_BOX?1:2) + 1] = BoxSymbols[BS_T_H1V1];
		}
	}
}

void VMenu::ApplySeparatorName(const MenuItemEx& Item, string& separator) const
{
	if (Item.Name.empty() || separator.size() <= 3)
		return;

	auto NameWidth{ std::min(Item.Name.size(), separator.size() - 2) };
	auto NamePos{ (separator.size() - NameWidth) / 2 };

	separator[NamePos - 1] = L' ';
	separator.replace(NamePos, NameWidth, fit_to_left(Item.Name, NameWidth));
	separator[NamePos + NameWidth] = L' ';
}

void VMenu::DrawRegularItem(const MenuItemEx& Item, const menu_layout& Layout, const int Y, std::vector<int>& HighlightMarkup, const string_view BlankLine) const
{
	if (!Layout.TextArea) return;

	const auto [TextAreaBegin, TextAreaWidth] { Layout.TextArea.value() };

	GotoXY(TextAreaBegin, Y);

	const item_color_indicies ColorIndices{ Item };
	auto CurColorIndex{ ColorIndices.Normal };
	auto AltColorIndex{ ColorIndices.Highlighted };

	size_t HotkeyPos = string::npos;
	auto ItemTextToDisplay = CheckFlags(VMENU_SHOWAMPERSAND)? Item.Name : HiText2Str(Item.Name, &HotkeyPos);
	std::ranges::replace(ItemTextToDisplay, L'\t', L' ');
	const auto ItemTextSize{ static_cast<int>(ItemTextToDisplay.size()) };

	const auto [ItemTextBegin, ItemTextEnd] { intersect({ 0, ItemTextSize }, { -Item.HorizontalPosition, TextAreaWidth - Item.HorizontalPosition }) };

	if (ItemTextBegin < ItemTextEnd)
	{
		HighlightMarkup.clear();
		if (!Item.Annotations.empty())
		{
			markup_slice_boundaries(
				std::pair{ ItemTextBegin, ItemTextEnd },
				Item.Annotations | std::views::transform([](const auto Ann) { return std::pair{ Ann.first, Ann.first + Ann.second }; }),
				HighlightMarkup);
		}
		else if (HotkeyPos != string::npos || Item.AutoHotkey)
		{
			const auto HighlightPos = static_cast<int>(HotkeyPos != string::npos? HotkeyPos : Item.AutoHotkeyPos);
			markup_slice_boundaries(
				std::pair{ ItemTextBegin, ItemTextEnd },
				std::views::single(std::pair{ HighlightPos, HighlightPos + 1 }),
				HighlightMarkup);
		}
		else
		{
			HighlightMarkup.emplace_back(ItemTextEnd);
		}

		set_color(Colors, ColorIndices.Normal);
		Text(BlankLine.substr(0, std::max(Item.HorizontalPosition, 0)));

		auto ItemTextPos{ ItemTextBegin };

		for (const auto SliceEnd : HighlightMarkup)
		{
			set_color(Colors, CurColorIndex);
			Text(string_view{ ItemTextToDisplay }.substr(ItemTextPos, SliceEnd - ItemTextPos));
			std::ranges::swap(CurColorIndex, AltColorIndex);
			ItemTextPos = SliceEnd;
		}
	}

	set_color(Colors, ColorIndices.Normal);
	Text(BlankLine.substr(0, TextAreaBegin + TextAreaWidth - WhereX()));

	const auto DrawDecorator = [&](const int X, std::tuple<color_indices, wchar_t> ColorAndChar)
		{
			GotoXY(X, Y);
			set_color(Colors, std::get<color_indices>(ColorAndChar));
			Text(std::get<wchar_t>(ColorAndChar));
		};

	if (Layout.CheckMark)
		DrawDecorator(Layout.CheckMark.value(), get_item_check_mark(Item, ColorIndices));

	if (Layout.LeftHScroll)
		DrawDecorator(Layout.LeftHScroll.value(), get_item_left_hscroll(Item.HorizontalPosition < 0, ColorIndices));

	if (Layout.SubMenu)
		DrawDecorator(Layout.SubMenu.value(), get_item_submenu(Item, ColorIndices));

	if (Layout.RightHScroll)
		DrawDecorator(Layout.RightHScroll.value(), get_item_right_hscroll(Item.HorizontalPosition + ItemTextSize > TextAreaWidth, ColorIndices));
}

int VMenu::CheckHighlights(wchar_t CheckSymbol, int StartPos) const
{
	if (CheckSymbol)
		CheckSymbol=upper(CheckSymbol);

	for (const auto I: std::views::iota(static_cast<size_t>(StartPos), Items.size()))
	{
		if (!item_is_visible(Items[I]))
			continue;

		if (const auto Ch = GetHighlights(&Items[I]))
		{
			if (CheckSymbol == upper(Ch) || CheckSymbol == upper(KeyToKeyLayout(Ch)))
				return static_cast<int>(I);
		}
		else if (!CheckSymbol)
		{
			return static_cast<int>(I);
		}
	}

	return -1;
}

wchar_t VMenu::GetHighlights(const MenuItemEx* const Item) const
{
	if (!Item)
		return 0;

	if (Item->AutoHotkey)
		return Item->AutoHotkey;

	if (CheckFlags(VMENU_SHOWAMPERSAND))
		return 0;

	wchar_t Ch;
	return HiTextHotkey(Item->Name, Ch)? Ch : 0;
}

void VMenu::AssignHighlights(bool Reverse)
{
	static_assert(sizeof(wchar_t) == 2, "512 MB for a bitset is too much, rewrite it.");
	std::bitset<std::numeric_limits<wchar_t>::max() + 1> Used;

	const auto Delta = Reverse? -1 : 1;

	const auto RegisterHotkey = [&](wchar_t const Hotkey)
	{
		const auto Upper = upper(Hotkey);
		if (Used[Upper])
			return false;

		Used[Upper] = true;
		Used[lower(Hotkey)] = true;

		const auto OtherHotkey = KeyToKeyLayout(Hotkey);
		Used[upper(OtherHotkey)] = true;
		Used[lower(OtherHotkey)] = true;

		return true;
	};

	const auto ShowAmpersand = CheckFlags(VMENU_SHOWAMPERSAND);
	// проверка заданных хоткеев
	for (int I = Reverse? static_cast<int>(Items.size() - 1) : 0; I >= 0 && I < static_cast<int>(Items.size()); I += Delta)
	{
		wchar_t Hotkey{};
		size_t HotkeyVisualPos{};
		// TODO: проверка на LIF_HIDDEN
		if (!ShowAmpersand && HiTextHotkey(Items[I].Name, Hotkey, &HotkeyVisualPos) && RegisterHotkey(Hotkey))
		{
			Items[I].AutoHotkey = Hotkey;
			Items[I].AutoHotkeyPos = HotkeyVisualPos;
		}
	}

	// TODO:  ЭТОТ цикл нужно уточнить - возможно вылезут артефакты (хотя не уверен)
	for (int I = Reverse? static_cast<int>(Items.size() - 1) : 0; I >= 0 && I < static_cast<int>(Items.size()); I += Delta)
	{
		size_t HotkeyVisualPos;
		auto MenuItemForDisplay = HiText2Str(Items[I].Name, &HotkeyVisualPos);
		if (!ShowAmpersand && HotkeyVisualPos != string::npos)
			continue;

		MenuItemForDisplay.erase(0, std::max(-Items[I].HorizontalPosition, 0));

		// TODO: проверка на LIF_HIDDEN
		for (const auto& Ch: MenuItemForDisplay)
		{
			if ((Ch == L'&' || is_alpha(Ch) || std::iswdigit(Ch)) && RegisterHotkey(Ch))
			{
				Items[I].AutoHotkey = Ch;
				Items[I].AutoHotkeyPos = &Ch - MenuItemForDisplay.data();
				break;
			}
		}
	}

	SetMenuFlags(VMENU_AUTOHIGHLIGHT | (Reverse? VMENU_REVERSEHIGHLIGHT : VMENU_NONE));
}

bool VMenu::CheckKeyHiOrAcc(DWORD Key, int Type, bool Translate, bool ChangePos, int& NewPos)
{
	//не забудем сбросить EndLoop для листбокса, иначе не будут работать хоткеи в активном списке
	if (CheckFlags(VMENU_LISTBOX))
		ClearDone();

	FOR_CONST_RANGE(Items, Iterator)
	{
		auto& CurItem = *Iterator;
		if (item_can_have_focus(CurItem) && ((!Type && CurItem.AccelKey && Key == CurItem.AccelKey) || (Type && (CurItem.AutoHotkey || !CheckFlags(VMENU_SHOWAMPERSAND)) && IsKeyHighlighted(CurItem.Name, Key, Translate, CurItem.AutoHotkey))))
		{
			NewPos=static_cast<int>(Iterator - Items.cbegin());
			if (ChangePos)
			{
				SetSelectPos(NewPos, 1);
				DrawMenu();
			}

			if ((!GetDialog() || CheckFlags(VMENU_COMBOBOX|VMENU_LISTBOX)) && item_can_be_entered(Items[SelectPos]))
			{
				SetExitCode(NewPos);
			}

			return true;
		}
	}

	return Done();
}

void VMenu::UpdateMaxLengthFromTitles()
{
	//тайтл + 2 пробела вокруг
	UpdateMaxLength(static_cast<int>(std::max(strTitle.size(), strBottomTitle.size()) + 2));
}

void VMenu::UpdateMaxLength(int const ItemLength)
{
	m_MaxItemLength = std::max(m_MaxItemLength, ItemLength);
}

void VMenu::ResetAllItemsBoundaries()
{
	m_AllItemsBoundaries = { std::numeric_limits<int>::max(), std::numeric_limits<int>::min() };
}

void VMenu::UpdateAllItemsBoundaries(int const ItemHPos, int const ItemLength)
{
	m_AllItemsBoundaries =
	{
		std::min(m_AllItemsBoundaries.first, ItemHPos),
		std::max(m_AllItemsBoundaries.second, ItemHPos + ItemLength)
	};
}

void VMenu::SetMaxHeight(int NewMaxHeight)
{
	MaxHeight = NewMaxHeight;

	if (MaxHeight > ScrY-6)
		MaxHeight = ScrY-6;
}

string VMenu::GetTitle() const
{
	return strTitle;
}

string &VMenu::GetBottomTitle(string &strDest) const
{
	strDest = strBottomTitle;
	return strDest;
}

void VMenu::SetBottomTitle(string_view const BottomTitle)
{
	SetMenuFlags(VMENU_UPDATEREQUIRED);

	strBottomTitle = BottomTitle;

	UpdateMaxLength(static_cast<int>(strBottomTitle.size() + 2));
}

void VMenu::SetTitle(string_view const Title)
{
	SetMenuFlags(VMENU_UPDATEREQUIRED);

	strTitle = Title;

	UpdateMaxLength(static_cast<int>(strTitle.size() + 2));
}

void VMenu::ResizeConsole()
{
	if (SaveScr)
	{
		SaveScr->Discard();
		SaveScr.reset();
	}
}

void VMenu::SetDeleting()
{
}

void VMenu::ShowConsoleTitle()
{
	if (CheckFlags(VMENU_CHANGECONSOLETITLE))
	{
		ConsoleTitle::SetFarTitle(strTitle);
	}
}

void VMenu::OnClose()
{
	EnableFilter(false);
}

void VMenu::SetColors(const FarDialogItemColors *ColorsIn)
{
	if (ColorsIn)
	{
		std::ranges::copy_n(ColorsIn->Colors, std::min(Colors.size(), ColorsIn->ColorsCount), Colors.begin());
	}
	else
	{
		const auto TypeMenu  = CheckFlags(VMENU_LISTBOX)? 0 : (CheckFlags(VMENU_COMBOBOX)? 1 : 2);
		const auto StyleMenu = CheckFlags(VMENU_WARNDIALOG)? 1 : 0;

		if (CheckFlags(VMENU_DISABLED))
		{
			std::ranges::fill(Colors, colors::PaletteColorToFarColor(StyleMenu? COL_WARNDIALOGDISABLED : COL_DIALOGDISABLED));
		}
		else
		{
			static const PaletteColors StdColor[2][3][std::tuple_size_v<vmenu_colors_t>] =
			{
				// Not VMENU_WARNDIALOG
				{
					// VMENU_LISTBOX
					{
						COL_DIALOGLISTTEXT,                        // подложка
						COL_DIALOGLISTBOX,                         // рамка
						COL_DIALOGLISTTITLE,                       // заголовок - верхний и нижний
						COL_DIALOGLISTTEXT,                        // Текст пункта
						COL_DIALOGLISTHIGHLIGHT,                   // HotKey
						COL_DIALOGLISTBOX,                         // separator
						COL_DIALOGLISTSELECTEDTEXT,                // Выбранный
						COL_DIALOGLISTSELECTEDHIGHLIGHT,           // Выбранный - HotKey
						COL_DIALOGLISTSCROLLBAR,                   // ScrollBar
						COL_DIALOGLISTDISABLED,                    // Disabled
						COL_DIALOGLISTARROWS,                      // Arrow
						COL_DIALOGLISTARROWSSELECTED,              // Выбранный - Arrow
						COL_DIALOGLISTARROWSDISABLED,              // Arrow Disabled
						COL_DIALOGLISTGRAY,                        // "серый"
						COL_DIALOGLISTSELECTEDGRAYTEXT,            // выбранный "серый"
					},
					// VMENU_COMBOBOX
					{
						COL_DIALOGCOMBOTEXT,                       // подложка
						COL_DIALOGCOMBOBOX,                        // рамка
						COL_DIALOGCOMBOTITLE,                      // заголовок - верхний и нижний
						COL_DIALOGCOMBOTEXT,                       // Текст пункта
						COL_DIALOGCOMBOHIGHLIGHT,                  // HotKey
						COL_DIALOGCOMBOBOX,                        // separator
						COL_DIALOGCOMBOSELECTEDTEXT,               // Выбранный
						COL_DIALOGCOMBOSELECTEDHIGHLIGHT,          // Выбранный - HotKey
						COL_DIALOGCOMBOSCROLLBAR,                  // ScrollBar
						COL_DIALOGCOMBODISABLED,                   // Disabled
						COL_DIALOGCOMBOARROWS,                     // Arrow
						COL_DIALOGCOMBOARROWSSELECTED,             // Выбранный - Arrow
						COL_DIALOGCOMBOARROWSDISABLED,             // Arrow Disabled
						COL_DIALOGCOMBOGRAY,                       // "серый"
						COL_DIALOGCOMBOSELECTEDGRAYTEXT,           // выбранный "серый"
					},
					// VMenu
					{
						COL_MENUBOX,                               // подложка
						COL_MENUBOX,                               // рамка
						COL_MENUTITLE,                             // заголовок - верхний и нижний
						COL_MENUTEXT,                              // Текст пункта
						COL_MENUHIGHLIGHT,                         // HotKey
						COL_MENUBOX,                               // separator
						COL_MENUSELECTEDTEXT,                      // Выбранный
						COL_MENUSELECTEDHIGHLIGHT,                 // Выбранный - HotKey
						COL_MENUSCROLLBAR,                         // ScrollBar
						COL_MENUDISABLEDTEXT,                      // Disabled
						COL_MENUARROWS,                            // Arrow
						COL_MENUARROWSSELECTED,                    // Выбранный - Arrow
						COL_MENUARROWSDISABLED,                    // Arrow Disabled
						COL_MENUGRAYTEXT,                          // "серый"
						COL_MENUSELECTEDGRAYTEXT,                  // выбранный "серый"
					}
				},

				// VMENU_WARNDIALOG
				{
					// VMENU_LISTBOX
					{
						COL_WARNDIALOGLISTTEXT,                    // подложка
						COL_WARNDIALOGLISTBOX,                     // рамка
						COL_WARNDIALOGLISTTITLE,                   // заголовок - верхний и нижний
						COL_WARNDIALOGLISTTEXT,                    // Текст пункта
						COL_WARNDIALOGLISTHIGHLIGHT,               // HotKey
						COL_WARNDIALOGLISTBOX,                     // separator
						COL_WARNDIALOGLISTSELECTEDTEXT,            // Выбранный
						COL_WARNDIALOGLISTSELECTEDHIGHLIGHT,       // Выбранный - HotKey
						COL_WARNDIALOGLISTSCROLLBAR,               // ScrollBar
						COL_WARNDIALOGLISTDISABLED,                // Disabled
						COL_WARNDIALOGLISTARROWS,                  // Arrow
						COL_WARNDIALOGLISTARROWSSELECTED,          // Выбранный - Arrow
						COL_WARNDIALOGLISTARROWSDISABLED,          // Arrow Disabled
						COL_WARNDIALOGLISTGRAY,                    // "серый"
						COL_WARNDIALOGLISTSELECTEDGRAYTEXT,        // выбранный "серый"
					},
					// VMENU_COMBOBOX
					{
						COL_WARNDIALOGCOMBOTEXT,                   // подложка
						COL_WARNDIALOGCOMBOBOX,                    // рамка
						COL_WARNDIALOGCOMBOTITLE,                  // заголовок - верхний и нижний
						COL_WARNDIALOGCOMBOTEXT,                   // Текст пункта
						COL_WARNDIALOGCOMBOHIGHLIGHT,              // HotKey
						COL_WARNDIALOGCOMBOBOX,                    // separator
						COL_WARNDIALOGCOMBOSELECTEDTEXT,           // Выбранный
						COL_WARNDIALOGCOMBOSELECTEDHIGHLIGHT,      // Выбранный - HotKey
						COL_WARNDIALOGCOMBOSCROLLBAR,              // ScrollBar
						COL_WARNDIALOGCOMBODISABLED,               // Disabled
						COL_WARNDIALOGCOMBOARROWS,                 // Arrow
						COL_WARNDIALOGCOMBOARROWSSELECTED,         // Выбранный - Arrow
						COL_WARNDIALOGCOMBOARROWSDISABLED,         // Arrow Disabled
						COL_WARNDIALOGCOMBOGRAY,                   // "серый"
						COL_WARNDIALOGCOMBOSELECTEDGRAYTEXT,       // выбранный "серый"
					},
					// VMenu
					{
						COL_MENUBOX,                               // подложка
						COL_MENUBOX,                               // рамка
						COL_MENUTITLE,                             // заголовок - верхний и нижний
						COL_MENUTEXT,                              // Текст пункта
						COL_MENUHIGHLIGHT,                         // HotKey
						COL_MENUBOX,                               // separator
						COL_MENUSELECTEDTEXT,                      // Выбранный
						COL_MENUSELECTEDHIGHLIGHT,                 // Выбранный - HotKey
						COL_MENUSCROLLBAR,                         // ScrollBar
						COL_MENUDISABLEDTEXT,                      // Disabled
						COL_MENUARROWS,                            // Arrow
						COL_MENUARROWSSELECTED,                    // Выбранный - Arrow
						COL_MENUARROWSDISABLED,                    // Arrow Disabled
						COL_MENUGRAYTEXT,                          // "серый"
						COL_MENUSELECTEDGRAYTEXT,                  // выбранный "серый"
					}
				}
			};

			std::ranges::transform(StdColor[StyleMenu][TypeMenu], Colors.begin(), colors::PaletteColorToFarColor);
		}
	}
}

void VMenu::GetColors(FarDialogItemColors *ColorsOut)
{
	std::ranges::copy_n(Colors.begin(), std::min(Colors.size(), ColorsOut->ColorsCount), ColorsOut->Colors);
}

void VMenu::SetOneColor(int Index, PaletteColors Color)
{
	if (0 <= Index && static_cast<size_t>(Index) < Colors.size())
		Colors[Index] = colors::PaletteColorToFarColor(Color);
}

bool VMenu::GetVMenuInfo(FarListInfo* Info) const
{
	if (Info)
	{
		Info->Flags = GetFlags() & (LINFO_SHOWNOBOX|LINFO_AUTOHIGHLIGHT|LINFO_REVERSEHIGHLIGHT|LINFO_WRAPMODE|LINFO_SHOWAMPERSAND);
		Info->ItemsNumber = Items.size();
		Info->SelectPos = SelectPos;
		Info->TopPos = TopPos;
		Info->MaxHeight = MaxHeight;
		// BUGBUG
		const auto ServiceAreaSize = menu_layout::get_service_area_size(*this);
		if (m_Where.width() > ServiceAreaSize)
			Info->MaxLength = m_Where.width() - ServiceAreaSize;
		else
			Info->MaxLength = 0;
		return true;
	}

	return false;
}

// Функция GetItemPtr - получить указатель на нужный Items.
MenuItemEx& VMenu::at(size_t n)
{
	const auto ItemPos = GetItemPosition(static_cast<int>(n));

	if (ItemPos < 0)
		throw far_fatal_exception(L"menu index out of range"sv);

	return Items[ItemPos];
}

intptr_t VMenu::GetSimpleUserData(int Position) const
{
	const auto ItemPos = GetItemPosition(Position);
	if (ItemPos < 0 || static_cast<size_t>(ItemPos) >= Items.size())
		return 0;

	return Items[ItemPos].SimpleUserData;
}

// Присовокупить к элементу данные.
void VMenu::SetComplexUserData(const std::any& Data, int Position)
{
	const auto ItemPos = GetItemPosition(Position);
	if (ItemPos < 0 || static_cast<size_t>(ItemPos) >= Items.size())
		return;

	Items[ItemPos].ComplexUserData = Data;
}

// Получить данные
std::any* VMenu::GetComplexUserData(int Position)
{
	const auto ItemPos = GetItemPosition(Position);
	if (ItemPos < 0 || static_cast<size_t>(ItemPos) >= Items.size())
		return nullptr;

	return &Items[ItemPos].ComplexUserData;
}

FarListItem *VMenu::MenuItem2FarList(const MenuItemEx *MItem, FarListItem *FItem)
{
	if (FItem && MItem)
	{
		*FItem = {};
		FItem->Flags = MItem->Flags;
		FItem->Text = MItem->Name.c_str();
		FItem->UserData = MItem->SimpleUserData;
		return FItem;
	}

	return nullptr;
}

FARMACROAREA VMenu::GetMacroArea() const
{
	if (IsComboBox())
		return MACROAREA_DIALOG;
	return Modal::GetMacroArea();
}

int VMenu::GetTypeAndName(string &strType, string &strName)
{
	strType = msg(lng::MVMenuType);
	strName = strTitle;
	return CheckFlags(VMENU_COMBOBOX) ? windowtype_combobox : windowtype_menu;
}

// return Pos || -1
int VMenu::FindItem(const FarListFind *FItem) const
{
	return FindItem(FItem->StartIndex,FItem->Pattern,FItem->Flags);
}

int VMenu::FindItem(int StartIndex, string_view const Pattern, unsigned long long Flags) const
{
	if (static_cast<size_t>(StartIndex) < Items.size())
	{
		for (const auto I: std::views::iota(static_cast<size_t>(StartIndex), Items.size()))
		{
			const auto strTmpBuf = remove_highlight(Items[I].Name);

			if (Flags&LIFIND_EXACTMATCH)
			{
				if (starts_with_icase(strTmpBuf, Pattern))
					return static_cast<int>(I);
			}
			else
			{
				if (CmpName(Pattern, strTmpBuf, true))
					return static_cast<int>(I);
			}
		}
	}

	return -1;
}

// Сортировка элементов списка
// Offset - начало сравнения! по умолчанию =0
void VMenu::SortItems(bool Reverse, int Offset)
{
	SortItems([](const MenuItemEx& a, const MenuItemEx& b, const SortItemParam& Param)
	{
		const auto
			strName1 = remove_highlight(a.Name),
			strName2 = remove_highlight(b.Name);

		const auto Less = string_sort::less(string_view(strName1).substr(Param.Offset), string_view(strName2).substr(Param.Offset));
		return Param.Reverse? !Less : Less;
	}, Reverse, Offset);
}

bool VMenu::Pack()
{
	const auto OldItemCount = Items.size();
	size_t FirstIndex=0;

	while (FirstIndex<Items.size())
	{
		size_t LastIndex=Items.size()-1;
		while (LastIndex>FirstIndex)
		{
			if (!(Items[FirstIndex].Flags & LIF_SEPARATOR) && !(Items[LastIndex].Flags & LIF_SEPARATOR))
			{
				if (Items[FirstIndex].Name == Items[LastIndex].Name)
				{
					DeleteItem(static_cast<int>(LastIndex));
				}
			}
			LastIndex--;
		}
		FirstIndex++;
	}
	return OldItemCount != Items.size();
}

void VMenu::SetId(const UUID& Id)
{
	MenuId=Id;
}

const UUID& VMenu::Id() const
{
	return MenuId;
}

std::vector<string> VMenu::AddHotkeys(std::span<menu_item> const MenuItems)
{
	std::vector<string> Result(MenuItems.size());

	const size_t MaxLength = std::ranges::fold_left(MenuItems, 0uz, [](size_t Value, const auto& i)
	{
		return std::max(Value, i.Name.size());
	});

	for (const auto& [Item, Str]: zip(MenuItems, Result))
	{
		if (Item.Flags & LIF_SEPARATOR || !Item.AccelKey)
			continue;

		const auto Key = KeyToLocalizedText(Item.AccelKey);
		const auto Hl = HiStrlen(Item.Name) != Item.Name.size();
		Str = fit_to_left(Item.Name, MaxLength + (Hl? 2 : 1)) + Key;
		Item.Name = Str;
	}

	return Result;
}

size_t VMenu::GetNaturalMenuWidth() const
{
	return static_cast<size_t>(m_MaxItemLength) + menu_layout::get_service_area_size(*this);
}

void VMenu::EnableFilter(bool const Enable)
{
	bFilterEnabled = Enable;
	bFilterLocked = false;
	strFilter.clear();

	if (!Enable)
		RestoreFilteredItems();
}

int VMenu::CalculateTextAreaWidth() const
{
	const auto TextArea = menu_layout{ *this }.TextArea;
	return TextArea ? TextArea.value().second : 0;
}

size_t VMenu::Text(string_view const Str) const
{
	return ::Text(Str, m_Where.width() - (WhereX() - m_Where.left));
}

size_t VMenu::Text(wchar_t const Char) const
{
	return ::Text(Char, m_Where.width() - (WhereX() - m_Where.left));
}

#ifdef ENABLE_TESTS

#include "testing.hpp"

TEST_CASE("find.nearest.selectable.item")
{
	std::array<int, 10> arr{};

	const auto Pred{ [](const int b) { return b != 0; } };

	const auto TestAllPositions{
		[&](const int Found)
		{
			for (const auto Pos : std::views::iota(0, static_cast<int>(arr.size())))
			{
				REQUIRE(find_nearest(arr, Pos, Pred, false, false) == Found);
				REQUIRE(find_nearest(arr, Pos, Pred, false, true) == Found);
				REQUIRE(find_nearest(arr, Pos, Pred, true, false) == Found);
				REQUIRE(find_nearest(arr, Pos, Pred, true, true) == Found);
			}
		} };

	TestAllPositions(-1);

	for (const auto Found : std::views::iota(0, static_cast<int>(arr.size())))
	{
		std::ranges::fill(arr, int{});
		arr[Found] = true;
		TestAllPositions(Found);
	}

	std::ranges::fill(arr, int{});
	arr[3] = arr[7] = true;

	static constexpr struct
	{
		int Pos;
		bool GoBackward;
		bool DoWrap;
		int Expected;
	} TestDataPoints[]
	{
		{ 1, false, false, 3 },
		{ 1, false, true, 3 },
		{ 5, false, false, 7 },
		{ 5, false, true, 7 },
		{ 9, false, false, 7 },
		{ 9, false, true, 3 },
		{ 1, true, false, 3 },
		{ 1, true, true, 7 },
		{ 5, true, false, 3 },
		{ 5, true, true, 3 },
		{ 9, true, false, 7 },
		{ 9, true, true, 7 },
	};

	for (const auto& TestDataPoint : TestDataPoints)
	{
		REQUIRE(find_nearest(arr, TestDataPoint.Pos, Pred, TestDataPoint.GoBackward, TestDataPoint.DoWrap) == TestDataPoint.Expected);
	}
}

TEST_CASE("intersect.segments")
{
	static constexpr struct test_data
	{
		std::pair<int, int> A, B, Intersection;
	} TestDataPoints[] =
	{
		{ { 10, 20 }, { -1, 5 }, { 0, 0 } },
		{ { 10, 20 }, { -1, 10 }, { 0, 0 } },
		{ { 10, 20 }, { -1, 15 }, { 10, 15 } },
		{ { 10, 20 }, { -1, 20 }, { 10, 20 } },
		{ { 10, 20 }, { -1, 25 }, { 10, 20 } },
		{ { 10, 20 }, { 10, 15 }, { 10, 15 } },
		{ { 10, 20 }, { 10, 20 }, { 10, 20 } },
		{ { 10, 20 }, { 10, 25 }, { 10, 20 } },
		{ { 10, 20 }, { 15, 20 }, { 15, 20 } },
		{ { 10, 20 }, { 15, 25 }, { 15, 20 } },
		{ { 10, 20 }, { 20, 25 }, { 0, 0 } },
		{ { 10, 20 }, { 25, 30 }, { 0, 0 } },
		{ { 10, 20 }, { 0, 0 }, { 0, 0 } },
		{ { 10, 20 }, { 15, 15 }, { 0, 0 } },
		{ { 10, 20 }, { 30, 30 }, { 0, 0 } },
	};

	for (const auto& TestDataPoint : TestDataPoints)
	{
		REQUIRE(TestDataPoint.Intersection == intersect(TestDataPoint.A, TestDataPoint.B));
		REQUIRE(TestDataPoint.Intersection == intersect(TestDataPoint.B, TestDataPoint.A));
	}
}

TEST_CASE("markup.slice.boundaries")
{
	struct test_data
	{
		std::pair<int, int> Segment;
		std::initializer_list<std::pair<int, int>> Slices;
		std::initializer_list<int> Markup;
	};

	static const test_data TestDataPoints[] =
	{
		{ { 20, 50 }, { { 10, 15 } }, { 50 } },
		{ { 20, 50 }, { { 10, 20 } }, { 50 } },
		{ { 20, 50 }, { { 10, 30 } }, { 20, 30, 50 } },
		{ { 20, 50 }, { { 10, 50 } }, { 20, 50 } },
		{ { 20, 50 }, { { 10, 70 } }, { 20, 50 } },
		{ { 20, 50 }, { { 20, 30 } }, { 20, 30, 50 } },
		{ { 20, 50 }, { { 20, 50 } }, { 20, 50 } },
		{ { 20, 50 }, { { 20, 70 } }, { 20, 50 } },
		{ { 20, 50 }, { { 30, 40 } }, { 30, 40, 50 } },
		{ { 20, 50 }, { { 30, 50 } }, { 30, 50 } },
		{ { 20, 50 }, { { 30, 70 } }, { 30, 50 } },
		{ { 20, 50 }, { { 50, 50 } }, { 50 } },
		{ { 20, 50 }, { { 50, 70 } }, { 50 } },
		{ { 20, 50 }, { { 60, 70 } }, { 50 } },
		{ { 20, 70 }, { { 30, 40 }, { 50, 60 } }, { 30, 40, 50, 60, 70 } },
		{ { 20, 70 }, { { 30, 40 }, { 40, 60 } }, { 30, 40, 40, 60, 70 } },
		{ { 20, 70 }, { { 30, 50 }, { 40, 60 } }, { 30, 50, 50, 60, 70 } },
		{ { 20, 70 }, { { 50, 60 }, { 30, 40 } }, { 50, 60, 70 } },
		{ { 20, 50 }, { { 40, 30 } }, { 50 } },
		{ { 20, 50 }, { { 0, 0 } }, { 50 } },
		{ { 20, 20 }, { { 20, 30 } }, { 20 } },
		{ { 0, 0 }, { { 20, 30 } }, { 0 } },
		{ { 0, 0 }, { { 0, 0 } }, { 0 } },
	};

	std::vector<int> Markup;

	for (const auto& TestDataPoint : TestDataPoints)
	{
		Markup.clear();
		markup_slice_boundaries(TestDataPoint.Segment, TestDataPoint.Slices, Markup);
		REQUIRE(std::ranges::equal(TestDataPoint.Markup, Markup));
	}
}

TEST_CASE("item.hpos.limits")
{
	using enum item_hscroll_policy;

	struct test_data
	{
		int ItemLength;
		int TextAreaWidth;
		// cling_to_edge, bound, bound_stick_to_left
		std::initializer_list<std::pair<int, int>> Expected;
	};

	static const test_data TestDataPoints[] =
	{
		{ 1, 5, { { 0, 4 }, { 0, 4 }, { 0, 0 } } },
		{ 3, 5, { { -2, 4 }, { 0, 2 }, { 0, 0 } } },
		{ 5, 5, { { -4, 4 }, { 0, 0 }, { 0, 0 } } },
		{ 7, 5, { { -6, 4 }, { -2, 0 }, { -2, 0 } } },
		{ 10, 5, { { -9, 4 }, { -5, 0 }, { -5, 0 } } },
	};

	for (const auto& TestDataPoint : TestDataPoints)
	{
		REQUIRE(std::pair{ std::numeric_limits<int>::min(), std::numeric_limits<int>::max() }
			== item_hpos_limits(TestDataPoint.ItemLength, TestDataPoint.TextAreaWidth, unbound));

		for (const auto& Policy : { cling_to_edge, bound, bound_stick_to_left })
		{
			REQUIRE(std::ranges::cdata(TestDataPoint.Expected)[std::to_underlying(Policy) - 1]
				== item_hpos_limits(TestDataPoint.ItemLength, TestDataPoint.TextAreaWidth, Policy));
		}
	}
}

TEST_CASE("adjust.hpos.shift")
{
	static constexpr int TextAreaWidth{ 10 };
	static constexpr std::array Shifts{ -20, -19, -18, -17, -15, -14, -13, -11, -10, -9, -7, -5, -3, -1, 0, 1, 3, 5, 7, 9, 10, 11, 13, 14, 15, 17, 18, 19, 20 };

	struct test_data
	{
		int Left;
		int Right;
		std::array<int, Shifts.size()> Expected;
	};

	static constexpr test_data TestDataPoints[] =
	{
		//   Shifts{ -20, -19, -18, -17, -15, -14, -13, -11, -10,  -9,  -7,  -5, -3, -1, 0, 1, 3,  5,  7,  9, 10, 11, 13, 14, 15, 17, 18, 19, 20 }
		{ -10, -5, {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  0,  0, 0, 6, 8, 10, 12, 14, 15, 16, 18, 19, 19, 19, 19, 19, 19 } },
		{ -10, -1, {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  0,  0, 0, 2, 4,  6,  8, 10, 11, 12, 14, 15, 16, 18, 19, 19, 19 } },
		{ -10,  0, {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  0,  0, 0, 1, 3,  5,  7,  9, 10, 11, 13, 14, 15, 17, 18, 19, 19 } },
		{ -10,  1, {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  0,  0, 0, 1, 3,  5,  7,  9, 10, 11, 13, 14, 15, 17, 18, 19, 19 } },
		{ -10,  5, {  -4,  -4,  -4,  -4,  -4,  -4,  -4,  -4,  -4,  -4,  -4,  -4, -3, -1, 0, 1, 3,  5,  7,  9, 10, 11, 13, 14, 15, 17, 18, 19, 19 } },
		{ -10,  9, {  -8,  -8,  -8,  -8,  -8,  -8,  -8,  -8,  -8,  -8,  -7,  -5, -3, -1, 0, 1, 3,  5,  7,  9, 10, 11, 13, 14, 15, 17, 18, 19, 19 } },
		{ -10, 10, {  -9,  -9,  -9,  -9,  -9,  -9,  -9,  -9,  -9,  -9,  -7,  -5, -3, -1, 0, 1, 3,  5,  7,  9, 10, 11, 13, 14, 15, 17, 18, 19, 19 } },
		{ -10, 11, { -10, -10, -10, -10, -10, -10, -10, -10, -10,  -9,  -7,  -5, -3, -1, 0, 1, 3,  5,  7,  9, 10, 11, 13, 14, 15, 17, 18, 19, 19 } },
		{ -10, 15, { -14, -14, -14, -14, -14, -14, -13, -11, -10,  -9,  -7,  -5, -3, -1, 0, 1, 3,  5,  7,  9, 10, 11, 13, 14, 15, 17, 18, 19, 19 } },
		{ -10, 20, { -19, -19, -18, -17, -15, -14, -13, -11, -10,  -9,  -7,  -5, -3, -1, 0, 1, 3,  5,  7,  9, 10, 11, 13, 14, 15, 17, 18, 19, 19 } },

		//   Shifts{ -20, -19, -18, -17, -15, -14, -13, -11, -10,  -9,  -7,  -5, -3, -1, 0, 1, 3,  5,  7,  9, 10, 11, 13, 14, 15, 17, 18, 19, 20 }
		{  -5, -1, {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  0,  0, 0, 2, 4,  6,  8, 10, 11, 12, 14, 14, 14, 14, 14, 14, 14 } },
		{  -5,  0, {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  0,  0, 0, 1, 3,  5,  7,  9, 10, 11, 13, 14, 14, 14, 14, 14, 14 } },
		{  -5,  1, {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  0,  0, 0, 1, 3,  5,  7,  9, 10, 11, 13, 14, 14, 14, 14, 14, 14 } },
		{  -5,  5, {  -4,  -4,  -4,  -4,  -4,  -4,  -4,  -4,  -4,  -4,  -4,  -4, -3, -1, 0, 1, 3,  5,  7,  9, 10, 11, 13, 14, 14, 14, 14, 14, 14 } },
		{  -5,  9, {  -8,  -8,  -8,  -8,  -8,  -8,  -8,  -8,  -8,  -8,  -7,  -5, -3, -1, 0, 1, 3,  5,  7,  9, 10, 11, 13, 14, 14, 14, 14, 14, 14 } },
		{  -5, 10, {  -9,  -9,  -9,  -9,  -9,  -9,  -9,  -9,  -9,  -9,  -7,  -5, -3, -1, 0, 1, 3,  5,  7,  9, 10, 11, 13, 14, 14, 14, 14, 14, 14 } },
		{  -5, 11, { -10, -10, -10, -10, -10, -10, -10, -10, -10,  -9,  -7,  -5, -3, -1, 0, 1, 3,  5,  7,  9, 10, 11, 13, 14, 14, 14, 14, 14, 14 } },
		{  -5, 15, { -14, -14, -14, -14, -14, -14, -13, -11, -10,  -9,  -7,  -5, -3, -1, 0, 1, 3,  5,  7,  9, 10, 11, 13, 14, 14, 14, 14, 14, 14 } },
		{  -5, 20, { -19, -19, -18, -17, -15, -14, -13, -11, -10,  -9,  -7,  -5, -3, -1, 0, 1, 3,  5,  7,  9, 10, 11, 13, 14, 14, 14, 14, 14, 14 } },

		//   Shifts{ -20, -19, -18, -17, -15, -14, -13, -11, -10,  -9,  -7,  -5, -3, -1, 0, 1, 3,  5,  7,  9, 10, 11, 13, 14, 15, 17, 18, 19, 20 }
		{  -1,  0, {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  0,  0, 0, 1, 3,  5,  7,  9, 10, 10, 10, 10, 10, 10, 10, 10, 10 } },
		{  -1,  1, {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  0,  0, 0, 1, 3,  5,  7,  9, 10, 10, 10, 10, 10, 10, 10, 10, 10 } },
		{  -1,  5, {  -4,  -4,  -4,  -4,  -4,  -4,  -4,  -4,  -4,  -4,  -4,  -4, -3, -1, 0, 1, 3,  5,  7,  9, 10, 10, 10, 10, 10, 10, 10, 10, 10 } },
		{  -1,  9, {  -8,  -8,  -8,  -8,  -8,  -8,  -8,  -8,  -8,  -8,  -7,  -5, -3, -1, 0, 1, 3,  5,  7,  9, 10, 10, 10, 10, 10, 10, 10, 10, 10 } },
		{  -1, 10, {  -9,  -9,  -9,  -9,  -9,  -9,  -9,  -9,  -9,  -9,  -7,  -5, -3, -1, 0, 1, 3,  5,  7,  9, 10, 10, 10, 10, 10, 10, 10, 10, 10 } },
		{  -1, 11, { -10, -10, -10, -10, -10, -10, -10, -10, -10,  -9,  -7,  -5, -3, -1, 0, 1, 3,  5,  7,  9, 10, 10, 10, 10, 10, 10, 10, 10, 10 } },
		{  -1, 15, { -14, -14, -14, -14, -14, -14, -13, -11, -10,  -9,  -7,  -5, -3, -1, 0, 1, 3,  5,  7,  9, 10, 10, 10, 10, 10, 10, 10, 10, 10 } },
		{  -1, 20, { -19, -19, -18, -17, -15, -14, -13, -11, -10,  -9,  -7,  -5, -3, -1, 0, 1, 3,  5,  7,  9, 10, 10, 10, 10, 10, 10, 10, 10, 10 } },

		//   Shifts{ -20, -19, -18, -17, -15, -14, -13, -11, -10,  -9,  -7,  -5, -3, -1, 0, 1, 3,  5,  7,  9, 10, 11, 13, 14, 15, 17, 18, 19, 20 }
		{   0,  1, {   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  0,  0, 0, 1, 3,  5,  7,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9 } },
		{   0,  5, {  -4,  -4,  -4,  -4,  -4,  -4,  -4,  -4,  -4,  -4,  -4,  -4, -3, -1, 0, 1, 3,  5,  7,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9 } },
		{   0,  9, {  -8,  -8,  -8,  -8,  -8,  -8,  -8,  -8,  -8,  -8,  -7,  -5, -3, -1, 0, 1, 3,  5,  7,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9 } },
		{   0, 10, {  -9,  -9,  -9,  -9,  -9,  -9,  -9,  -9,  -9,  -9,  -7,  -5, -3, -1, 0, 1, 3,  5,  7,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9 } },
		{   0, 11, { -10, -10, -10, -10, -10, -10, -10, -10, -10,  -9,  -7,  -5, -3, -1, 0, 1, 3,  5,  7,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9 } },
		{   0, 15, { -14, -14, -14, -14, -14, -14, -13, -11, -10,  -9,  -7,  -5, -3, -1, 0, 1, 3,  5,  7,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9 } },
		{   0, 20, { -19, -19, -18, -17, -15, -14, -13, -11, -10,  -9,  -7,  -5, -3, -1, 0, 1, 3,  5,  7,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9 } },

		//   Shifts{ -20, -19, -18, -17, -15, -14, -13, -11, -10,  -9,  -7,  -5, -3, -1, 0, 1, 3,  5,  7,  9, 10, 11, 13, 14, 15, 17, 18, 19, 20 }
		{   1,  5, {  -4,  -4,  -4,  -4,  -4,  -4,  -4,  -4,  -4,  -4,  -4,  -4, -3, -1, 0, 1, 3,  5,  7,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8 } },
		{   1,  9, {  -8,  -8,  -8,  -8,  -8,  -8,  -8,  -8,  -8,  -8,  -7,  -5, -3, -1, 0, 1, 3,  5,  7,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8 } },
		{   1, 10, {  -9,  -9,  -9,  -9,  -9,  -9,  -9,  -9,  -9,  -9,  -7,  -5, -3, -1, 0, 1, 3,  5,  7,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8 } },
		{   1, 11, { -10, -10, -10, -10, -10, -10, -10, -10, -10,  -9,  -7,  -5, -3, -1, 0, 1, 3,  5,  7,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8 } },
		{   1, 15, { -14, -14, -14, -14, -14, -14, -13, -11, -10,  -9,  -7,  -5, -3, -1, 0, 1, 3,  5,  7,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8 } },
		{   1, 20, { -19, -19, -18, -17, -15, -14, -13, -11, -10,  -9,  -7,  -5, -3, -1, 0, 1, 3,  5,  7,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8 } },

		//   Shifts{ -20, -19, -18, -17, -15, -14, -13, -11, -10,  -9,  -7,  -5, -3, -1, 0, 1, 3,  5,  7,  9, 10, 11, 13, 14, 15, 17, 18, 19, 20 }
		{   5,  9, {  -8,  -8,  -8,  -8,  -8,  -8,  -8,  -8,  -8,  -8,  -7,  -5, -3, -1, 0, 1, 3,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4 } },
		{   5, 10, {  -9,  -9,  -9,  -9,  -9,  -9,  -9,  -9,  -9,  -9,  -7,  -5, -3, -1, 0, 1, 3,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4 } },
		{   5, 11, { -10, -10, -10, -10, -10, -10, -10, -10, -10,  -9,  -7,  -5, -3, -1, 0, 1, 3,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4 } },
		{   5, 15, { -14, -14, -14, -14, -14, -14, -13, -11, -10,  -9,  -7,  -5, -3, -1, 0, 1, 3,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4 } },
		{   5, 20, { -19, -19, -18, -17, -15, -14, -13, -11, -10,  -9,  -7,  -5, -3, -1, 0, 1, 3,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4 } },

		//   Shifts{ -20, -19, -18, -17, -15, -14, -13, -11, -10,  -9,  -7,  -5, -3, -1, 0, 1, 3,  5,  7,  9, 10, 11, 13, 14, 15, 17, 18, 19, 20 }
		{   9, 10, {  -9,  -9,  -9,  -9,  -9,  -9,  -9,  -9,  -9,  -9,  -7,  -5, -3, -1, 0, 0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 } },
		{   9, 11, { -10, -10, -10, -10, -10, -10, -10, -10, -10,  -9,  -7,  -5, -3, -1, 0, 0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 } },
		{   9, 15, { -14, -14, -14, -14, -14, -14, -13, -11, -10,  -9,  -7,  -5, -3, -1, 0, 0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 } },
		{   9, 20, { -19, -19, -18, -17, -15, -14, -13, -11, -10,  -9,  -7,  -5, -3, -1, 0, 0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 } },

		//   Shifts{ -20, -19, -18, -17, -15, -14, -13, -11, -10,  -9,  -7,  -5, -3, -1, 0, 1, 3,  5,  7,  9, 10, 11, 13, 14, 15, 17, 18, 19, 20 }
		{  10, 11, { -10, -10, -10, -10, -10, -10, -10, -10, -10,  -9,  -7,  -5, -3, -1, 0, 0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 } },
		{  10, 15, { -14, -14, -14, -14, -14, -14, -13, -11, -10,  -9,  -7,  -5, -3, -1, 0, 0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 } },
		{  10, 20, { -19, -19, -18, -17, -15, -14, -13, -11, -10,  -9,  -7,  -5, -3, -1, 0, 0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 } },

		//   Shifts{ -20, -19, -18, -17, -15, -14, -13, -11, -10,  -9,  -7,  -5, -3, -1, 0, 1, 3,  5,  7,  9, 10, 11, 13, 14, 15, 17, 18, 19, 20 }
		{  11, 15, { -14, -14, -14, -14, -14, -14, -14, -12, -11, -10,  -8,  -6, -4, -2, 0, 0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 } },
		{  11, 20, { -19, -19, -19, -18, -16, -15, -14, -12, -11, -10,  -8,  -6, -4, -2, 0, 0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 } },

		//   Shifts{ -20, -19, -18, -17, -15, -14, -13, -11, -10,  -9,  -7,  -5, -3, -1, 0, 1, 3,  5,  7,  9, 10, 11, 13, 14, 15, 17, 18, 19, 20 }
		{  15, 20, { -19, -19, -19, -19, -19, -19, -18, -16, -15, -14, -12, -10, -8, -6, 0, 0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 } },
	};

	for (const auto& TestDataPoint : TestDataPoints)
	{
		for (const auto I: std::views::iota(0uz, Shifts.size()))
		{
			REQUIRE(TestDataPoint.Expected[I] == adjust_hpos_shift(Shifts[I], TestDataPoint.Left, TestDataPoint.Right, TextAreaWidth));
		}
	}
}

#endif
