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
#include "common/algorithm.hpp"
#include "common/scope_exit.hpp"
#include "common/uuid.hpp"
#include "common/view/enumerate.hpp"
#include "common/view/zip.hpp"

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

// Must be in the TU scope because they are forward-declared in the header

// Indices in the color array
enum class color_indices
{
	Body = 0,     // background
	Box = 1,     // border
	Title = 2,     // title - top and bottom
	Text = 3,     // item text
	Highlight = 4,     // hot key
	Separator = 5,     // separator
	Selected = 6,     // selected
	HSelect = 7,     // selected - HotKey
	ScrollBar = 8,     // scrollBar
	Disabled = 9,     // disabled
	Arrows = 10,     // '«' & '»' normal
	ArrowsSelect = 11,     // '«' & '»' selected
	ArrowsDisabled = 12,     // '«' & '»' disabled
	Grayed = 13,     // grayed
	SelGrayed = 14,     // selected grayed

	COUNT                        // always the last - array dimension
};

static_assert(std::tuple_size_v<vmenu_colors_t> == std::to_underlying(color_indices::COUNT));

struct item_color_indices
{
	color_indices Normal, Highlighted, HScroller;

	explicit item_color_indices(const menu_item_ex& CurItem)
	{
		const auto Grayed{ !!(CurItem.Flags & LIF_GRAYED) };

		if (CurItem.Flags & LIF_DISABLE)
		{
			Normal = color_indices::Disabled;
			Highlighted = color_indices::Disabled;
			HScroller = color_indices::ArrowsDisabled;
			return;
		}

		if (CurItem.Flags & LIF_SELECTED)
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

struct menu_layout
{
	short BoxType{};
	small_rectangle ClientRect{};
	std::optional<short> LeftBox;
	std::optional<short> CheckMark;
	std::optional<small_segment> FixedColumnsArea;
	std::optional<short> LeftHScroll;
	std::optional<small_segment> TextArea;
	std::optional<short> RightHScroll;
	std::optional<short> SubMenu;
	std::optional<short> Scrollbar;
	std::optional<short> RightBox;

	explicit menu_layout(const VMenu& Menu)
		: BoxType{ get_box_type(Menu) }
		, ClientRect{ get_client_rect(Menu, BoxType) }
	{
		auto Left{ Menu.m_Where.left };
		if (need_box(BoxType)) LeftBox = Left++;
		if (need_check_mark()) CheckMark = Left++;
		if (const auto FixedColumnsWidth{ fixed_columns_width(Menu) })
		{
			FixedColumnsArea = { Left, small_segment::length_tag{ FixedColumnsWidth } };
			Left += FixedColumnsWidth;
		}
		if (need_left_hscroll()) LeftHScroll = Left++;

		auto Right{ Menu.m_Where.right };
		if (need_box(BoxType)) RightBox = Right;
		if (need_scrollbar(Menu, BoxType)) Scrollbar = Right;
		if (RightBox || Scrollbar) Right--;
		if (need_submenu(Menu)) SubMenu = Right--;
		if (need_right_hscroll()) RightHScroll = Right--;

		if (Left <= Right)
			TextArea = { Left, small_segment::sentinel_tag{ static_cast<short>(Right + 1) } };
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
			+ fixed_columns_width(Menu)
			+ need_left_hscroll()
			+ need_right_hscroll()
			+ need_submenu(Menu)
			+ (NeedBox || need_scrollbar(Menu, BoxType));
	}

	[[nodiscard]] static short fixed_columns_width(const VMenu& Menu) noexcept
	{
		const auto width{ std::ranges::fold_left(
			// Plus one position for separator
			Menu.m_FixedColumns
			| std::views::transform([](const auto& column) { return column.CurrentWidth + !!column.CurrentWidth; }), 0, std::plus{}) };
		assert(std::in_range<short>(width));
		return static_cast<short>(width);
	}

	[[nodiscard]] static int get_title_service_area_size(const bool NeedBox)
	{
		// ╚═ Ctrl+Enter F5 Gray + Ctrl+Up Ctrl+Down ══╝
		// ?^^                                      ^ ^?
		return NeedBox + 1 + 1 + 1 + 1 + NeedBox;
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

enum class item_hscroll_policy
{
	unbound,            // The item can move freely beyond window edges.
	cling_to_edge,      // The item can move beyond window edges, but at least one character is always visible.
	bound,              // The item can move only within the window boundaries.
	bound_stick_to_left // Like bound, but if the item shorter than TextAreaWidth, it is always attached to the left window edge.
};

// Keeps track of the horizontal state of all items.
// The tracking is best effort. See comments below.
// Everything is relative to menu_layout::TextArea::start (Left edge).
class vmenu_horizontal_tracker
{
	struct bulk_update_scope_guard
	{
		NONCOPYABLE(bulk_update_scope_guard);
		NONMOVABLE(bulk_update_scope_guard);
		explicit bulk_update_scope_guard(vmenu_horizontal_tracker* Tracker) noexcept: m_Tracker{ Tracker } {}
		~bulk_update_scope_guard() { m_Tracker->m_IsBulkUpdate = false; }
		vmenu_horizontal_tracker* m_Tracker{};
	};

public:
	enum class alignment { Left, Right, Annotation };

	void clear() { *this = {}; }

	void add_item(int const ItemHPos, int const ItemLength, int const ItemAnnotationPos)
	{
		update_boundaries(ItemHPos, ItemLength);

		if (!is_item_aligned(ItemHPos, ItemLength, ItemAnnotationPos))
			m_StrayItems++;
	}

	void remove_item(int const ItemHPos, int const ItemLength, int const ItemAnnotationPos)
	{
		if (!is_item_aligned(ItemHPos, ItemLength, ItemAnnotationPos))
			m_StrayItems--;
	}

	[[nodiscard]] bulk_update_scope_guard start_bulk_update_smart_left_right(
		int const SmartAlignedAtColumn, int const TextAreaWidth, item_hscroll_policy const Policy)
	{
		if (SmartAlignedAtColumn >= 0)
			return start_bulk_update(alignment::Left, SmartAlignedAtColumn, TextAreaWidth, Policy);
		else
			return start_bulk_update(alignment::Right, TextAreaWidth + SmartAlignedAtColumn + 1, TextAreaWidth, Policy);
	}

	[[nodiscard]] bulk_update_scope_guard start_bulk_update_annotation(int const AlignedAtColumn)
	{
		return start_bulk_update(alignment::Annotation, AlignedAtColumn);
	}

	[[nodiscard]] bulk_update_scope_guard start_bulk_update_shift(
		int const AlignedAtColumnShift, int const TextAreaWidth, item_hscroll_policy const Policy)
	{
		return start_bulk_update(m_Alignment, m_AlignedAtColumn + AlignedAtColumnShift, TextAreaWidth, Policy);
	}

	// Use only to update item's HorizontalPosition.
	// If either ItemLength or ItemAnnotationPos may have changed, use remove_item and add_item.
	// If called during bulk update operation, it does not decrement m_StrayItems.
	void update_item_hpos(int const OldItemHPos, int const NewItemHPos, int const ItemLength, int const ItemAnnotationPos)
	{
		if (!m_IsBulkUpdate)
			remove_item(OldItemHPos, ItemLength, ItemAnnotationPos);

		add_item(NewItemHPos, ItemLength, ItemAnnotationPos);
	}

	int get_left_boundary() const noexcept { return m_LBoundary; }
	int get_right_boundary() const noexcept { return m_RBoundary; }
	std::optional<alignment> get_alignment() const noexcept { return m_StrayItems ? std::nullopt : std::optional{ m_Alignment }; }
	bool all_items_are_at_home()const noexcept { return m_Alignment == alignment::Left && m_AlignedAtColumn == 0 && m_StrayItems == 0; }

	auto get_debug_string() const
	{
		const auto AlignmentMark{ [&]
			{
				switch (m_Alignment)
				{
				case alignment::Left:       return L'<';
				case alignment::Right:      return L'>';
				case alignment::Annotation: return L'^';
				default: std::unreachable();
				}
			} };

		return far::format(L" [{}:{} {} {} {}] "sv, m_LBoundary, m_RBoundary, AlignmentMark(), m_AlignedAtColumn, m_StrayItems);
	}

private:
	void update_boundaries(int const ItemHPos, int const ItemLength)
	{
		m_LBoundary = std::min(m_LBoundary, ItemHPos);
		m_RBoundary = std::max(m_RBoundary, ItemHPos + ItemLength);
	}

	bool is_item_aligned(int const ItemHPos, int const ItemLength, int const ItemAnnotationPos) const
	{
		const auto AnchorOffset{ [&]()
			{
				switch (m_Alignment)
				{
				case alignment::Left:       return 0;
				case alignment::Right:      return ItemLength;
				case alignment::Annotation: return ItemAnnotationPos;
				default: std::unreachable();
				}
			}() };

		return ItemHPos + AnchorOffset == m_AlignedAtColumn;
	}

	[[nodiscard]] bulk_update_scope_guard start_bulk_update(alignment const Alignment, int const AlignedAtColumn)
	{
		clear();

		m_Alignment = Alignment;
		m_AlignedAtColumn = AlignedAtColumn;
		m_IsBulkUpdate = true;

		return bulk_update_scope_guard{ this };
	}

	[[nodiscard]] bulk_update_scope_guard start_bulk_update(
		alignment const Alignment, int const AlignedAtColumn, int const TextAreaWidth, item_hscroll_policy const Policy)
	{
		if (Policy == item_hscroll_policy::unbound)
			return start_bulk_update(Alignment, AlignedAtColumn);

		if (Alignment == alignment::Left)
			return start_bulk_update(Alignment, std::min(AlignedAtColumn, Policy == item_hscroll_policy::cling_to_edge ? TextAreaWidth - 1 : 0));
		else
			return start_bulk_update(Alignment, std::max(AlignedAtColumn, Policy == item_hscroll_policy::cling_to_edge ? 1 : TextAreaWidth));
	}

	// All items are within the boundaries, always.
	// Boundaries track items as they are shifted horizontally.
	// The tracking is best effort. Boundaries are:
	// - Not tight, i.e., there may be no item actually touching a boundary.
	// - Expanded but never contracted.
	// - Fully recalculated during align operations (left, right, or at annotations).
	int m_LBoundary{ std::numeric_limits<int>::max() };
	int m_RBoundary{ std::numeric_limits<int>::min() };

	// All items have their corresponding anchor position (left, right,
	// or annotation) at the m_AlignedAtColumn. See also m_StrayItems.
	alignment m_Alignment{ alignment::Left };

	// The column at which all items' anchors are aligned.
	// Depending on item_hscroll_policy, may be an arbitrary column, even outside of menu_layout::TextArea.
	int m_AlignedAtColumn{};

	// The number of items which were shifted horizontally out of general alignment.
	// Incremented each time an item is shifted out of alignment.
	// Decremented each time an item clicks back into place. May become zero,
	// which is the whole point of the vmenu_horizontal_tracker contraption.
	// It is reset to zero at the beginning of bulk operations.
	// The tracking is best effort. For example, if m_Alignment is Right
	// and each item was shifted individually so that all of them became aligned
	// to the left edge of the text area, m_Alignment does not become Left.
	int m_StrayItems{};

	bool m_IsBulkUpdate{};
};

namespace
{
	FarListItem string_to_far_list_item(const wchar_t* NewStrItem)
	{
		if (!NewStrItem)
			return { .Flags = LIF_SEPARATOR };

		if (NewStrItem[0] == 0x1)
			return { .Flags = LIF_SEPARATOR, .Text = NewStrItem + 1 };

		return { .Text = NewStrItem };
	}

	menu_item_ex far_list_item_to_menu_item(const FarListItem& FItem)
	{
		menu_item_ex Result{ NullToEmpty(FItem.Text), FItem.Flags };
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

	void markup_slice_boundaries(segment Segment, std::ranges::input_range auto const& Slices, std::vector<int>& Markup)
	{
		assert(!Segment.empty());
		Markup.clear();

		for (const auto& Slice : Slices)
		{
			const auto Intersection{ intersect(Segment, Slice) };
			if (Intersection.empty()) continue;

			Markup.emplace_back(Intersection.start());
			Markup.emplace_back(Intersection.end());

			if (Intersection.end() >= Segment.end()) return;
			Segment = { Intersection.end(), segment::sentinel_tag{ Segment.end() } };
		}

		Markup.emplace_back(Segment.end());
	}

	void markup_slice_boundaries(
		segment Segment,
		const std::list<segment>& Annotations,
		const std::optional<int> HotkeyPos,
		std::vector<int>& Markup)
	{
		assert(!Segment.empty());

		if (!Annotations.empty())
		{
			markup_slice_boundaries(
				Segment,
				Annotations,
				Markup);
			return;
		}

		if (HotkeyPos)
		{
			markup_slice_boundaries(
				Segment,
				std::views::single(segment{ *HotkeyPos, segment::length_tag{ 1 } }),
				Markup);
			return;
		}

		Markup.assign(1, Segment.end());
	}

	bool item_flags_allow_focus(unsigned long long const Flags)
	{
		return !(Flags & (LIF_DISABLE | LIF_HIDDEN | LIF_FILTERED | LIF_SEPARATOR));
	}

	bool item_can_have_focus(menu_item_ex const& Item)
	{
		return item_flags_allow_focus(Item.Flags);
	}

	bool item_can_be_entered(menu_item_ex const& Item)
	{
		return item_can_have_focus(Item) && !(Item.Flags & LIF_GRAYED);
	}

	bool item_is_visible(menu_item_ex const& Item)
	{
		return !(Item.Flags & (LIF_HIDDEN | LIF_FILTERED));
	}

	string_view get_item_cell_text(string_view ItemName, segment CellSegment)
	{
		const auto Intersection{ intersect(segment{ 0, segment::length_tag{ static_cast<segment::domain_t>(ItemName.size()) } }, CellSegment) };
		if (Intersection.empty()) return {};
		return ItemName.substr(Intersection.start(), Intersection.length());
	}

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

	void toggle_fixed_columns(std::vector<vmenu_fixed_column_t>& FixedColumns)
	{
		assert(!FixedColumns.empty());

		if (auto firstHiddenColumn{ std::ranges::find(FixedColumns, 0, &vmenu_fixed_column_t::CurrentWidth) };
			firstHiddenColumn != FixedColumns.end())
		{
			firstHiddenColumn->CurrentWidth = firstHiddenColumn->TextSegment.length();
			return;
		}

		for (auto& column : FixedColumns)
		{
			column.CurrentWidth = 0;
		}
	}

	[[nodiscard]] const FarColor& get_color(const vmenu_colors_t& VMenuColors, color_indices ColorIndex) noexcept
	{
		return VMenuColors[std::to_underlying(ColorIndex)];
	}

	void set_color(const vmenu_colors_t& VMenuColors, color_indices ColorIndex)
	{
		SetColor(get_color(VMenuColors, ColorIndex));
	}

	std::tuple<color_indices, wchar_t> get_item_check_mark(const menu_item_ex& CurItem, item_color_indices ColorIndices) noexcept
	{
		return
		{
			ColorIndices.Normal,
			!(CurItem.Flags & LIF_CHECKED)
				? L' '
				: !(CurItem.Flags & 0x0000FFFF) ? L'√' : static_cast<wchar_t>(CurItem.Flags & 0x0000FFFF)
		};
	}

	std::tuple<color_indices, wchar_t> get_item_submenu(const menu_item_ex& CurItem, item_color_indices ColorIndices) noexcept
	{
		return
		{
			ColorIndices.Normal,
			(CurItem.Flags & MIF_SUBMENU) ? L'►' : L' '
		};
	}

	std::tuple<color_indices, wchar_t> get_item_left_hscroll(const bool NeedLeftHScroll, item_color_indices ColorIndices) noexcept
	{
		return
		{
			NeedLeftHScroll ? ColorIndices.HScroller : ColorIndices.Normal,
			NeedLeftHScroll ? L'«' : L' '
		};
	}

	std::tuple<color_indices, wchar_t> get_item_right_hscroll(const bool NeedRightHScroll, item_color_indices ColorIndices) noexcept
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
	m_HorizontalTracker{ std::make_unique<vmenu_horizontal_tracker>() },
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

	for (const auto& Item: Data)
	{
		AddItem(menu_item_ex{ Item });
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

	const auto DoWrap{ CheckFlags(VMENU_WRAPMODE) && Direct != 0 && !stop_on_edge };
	const auto GoBackward{ Direct < 0 };
	const auto ItemsSize{ sizeAsInt() };

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
		const auto SavedSelectPos{ SelectPos };

		for (auto& i : Items)
			i.Flags &= ~LIF_SELECTED;

		if (Pos >= 0)
			UpdateItemFlags(Pos, Items[Pos].Flags | LIF_SELECTED); // Changes SelectPos

		if (const auto Parent = GetDialog(); Parent && Parent->IsInited() && !Parent->SendMessage(DN_LISTCHANGE, DialogItemID, ToPtr(Pos)))
		{
			// Restore SelectPos
			if (SavedSelectPos >= 0)
				UpdateItemFlags(SavedSelectPos, Items[SavedSelectPos].Flags | LIF_SELECTED);

			return -1;
		}
	}

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
	const auto DataPos{ Position < 0 ? SelectPos : Position };

	return in_closed_range(0, DataPos, sizeAsInt() - 1)
		? DataPos
		: -1; //Items.size()-1; ???
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
		if (AddItem(far_list_item_to_menu_item(NewItem->Item), NewItem->Index) >= 0)
			return sizeAsInt();
	}

	return -1;
}

int VMenu::AddItem(const FarList* List)
{
	if (List && List->Items)
	{
		for (const auto& Item: std::span(List->Items, List->ItemsNumber))
		{
			AddItem(far_list_item_to_menu_item(Item));
		}
	}

	return sizeAsInt();
}

int VMenu::AddItem(const wchar_t *NewStrItem)
{
	return AddItem(far_list_item_to_menu_item(string_to_far_list_item(NewStrItem)));
}

int VMenu::AddItem(menu_item_ex&& NewItem,int PosAdd)
{
	PosAdd = std::clamp(PosAdd, 0, sizeAsInt());

	Items.emplace(Items.begin() + PosAdd, std::move(NewItem));
	auto& NewMenuItem = Items[PosAdd];

	NewMenuItem.AutoHotkey = {};
	NewMenuItem.AutoHotkeyPos = 0;
	NewMenuItem.HorizontalPosition = 0;

	if (PosAdd <= SelectPos)
		SelectPos++;

	const auto ItemLength{ GetItemVisualLength(NewMenuItem) };
	UpdateMaxLength(ItemLength);
	m_HorizontalTracker->add_item(NewMenuItem.HorizontalPosition, ItemLength, NewMenuItem.SafeGetFirstAnnotation());

	const auto NewFlags = NewMenuItem.Flags;
	NewMenuItem.Flags = 0;
	UpdateItemFlags(PosAdd, NewFlags);

	SetMenuFlags(VMENU_UPDATEREQUIRED | (bFilterEnabled ? VMENU_REFILTERREQUIRED : VMENU_NONE));

	return sizeAsInt() - 1;
}

bool VMenu::UpdateItem(const FarListUpdate *NewItem)
{
	if (!NewItem || static_cast<size_t>(NewItem->Index) >= Items.size())
		return false;

	auto& Item = Items[NewItem->Index];
	m_HorizontalTracker->remove_item(
		Item.HorizontalPosition, GetItemVisualLength(Item), Item.SafeGetFirstAnnotation());

	// Освободим память... от ранее занятого ;-)
	if (NewItem->Item.Flags&LIF_DELETEUSERDATA)
	{
		Item.ComplexUserData = {};
		Item.Annotations.clear();
	}

	Item.Name = NullToEmpty(NewItem->Item.Text);
	UpdateItemFlags(NewItem->Index, NewItem->Item.Flags);
	Item.SimpleUserData = NewItem->Item.UserData;

	const auto ItemLength{ GetItemVisualLength(Item) };
	UpdateMaxLength(ItemLength);
	m_HorizontalTracker->add_item(Item.HorizontalPosition, ItemLength, Item.SafeGetFirstAnnotation());

	SetMenuFlags(VMENU_UPDATEREQUIRED | (bFilterEnabled ? VMENU_REFILTERREQUIRED : VMENU_NONE));

	return true;
}

//функция удаления N пунктов меню
int VMenu::DeleteItem(int ID, int Count)
{
	if (ID < 0 || ID >= sizeAsInt() || Count <= 0)
		return sizeAsInt();

	if (ID+Count > sizeAsInt())
		Count = sizeAsInt() - ID;

	if (!ID && Count == sizeAsInt())
	{
		clear();
		return sizeAsInt();
	}

	for (const auto& I : std::span{ Items.cbegin() + ID, static_cast<size_t>(Count) })
	{
		if (I.Flags & MIF_SUBMENU)
			--ItemSubMenusCount;

		if (!item_is_visible(I))
			--ItemHiddenCount;

		m_HorizontalTracker->remove_item(
			I.HorizontalPosition, GetItemVisualLength(I), I.SafeGetFirstAnnotation());
	}

	// а вот теперь перемещения
	if (Items.size() > 1)
	{
		const auto FirstIter = Items.begin() + ID, LastIter = FirstIter + Count;
		Items.erase(FirstIter, LastIter);
	}

	// коррекция текущей позиции
	if (SelectPos >= ID && SelectPos < ID+Count)
	{
		if(SelectPos== sizeAsInt())
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

	return sizeAsInt();
}

void VMenu::clear()
{
	Items.clear();
	ItemHiddenCount=0;
	ItemSubMenusCount=0;
	SelectPos=-1;
	TopPos=0;
	m_MaxItemLength = 0;
	m_HorizontalTracker->clear();
	m_FixedColumns.clear();
	m_ItemTextSegment = segment::ray();

	SetMenuFlags(VMENU_UPDATEREQUIRED);
}

int VMenu::GetCheck(int Position) const
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
				int EndPos = sizeAsInt() - 1;

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
					const auto strTemp = trim(remove_highlight(Item.Name));

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

			if (iParam < 0 || iParam >= static_cast<long long>(size()))
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
					RetValue = std::ranges::count_if(Items, [](const menu_item_ex& Item) { return (Item.Flags & LIF_FILTERED) != 0; });
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
		case MCODE_V_MENU_HORIZONTALALIGNMENT:
		{
			if (const auto alignment{ m_HorizontalTracker->get_alignment() })
			{
				switch (*alignment)
				{
				case vmenu_horizontal_tracker::alignment::Left:
					return 1;
				case vmenu_horizontal_tracker::alignment::Right:
					return 2;
				case vmenu_horizontal_tracker::alignment::Annotation:
					return 4;
				}
			}
			return 0;
		}
		case MCODE_F_MENU_GETEXTENDEDDATA:
		{
			if (m_ExtendedDataProvider)
			{
				const auto ItemPos{ GetItemPosition(iParam) };
				if (ItemPos == -1) return -1;

				*static_cast<extended_item_data*>(vParam) = m_ExtendedDataProvider(Items[ItemPos]);
				return 1;
			}
			return -1;
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
			int p = sizeAsInt() - 1;
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

			pSel = std::min(pSel, sizeAsInt() - 1);
			pTop = std::min(pTop, sizeAsInt() - 1);

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
		case KEY_SHIFTF5:
		{
			if (ToggleFixedColumns())
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

	if (bShowScrollBar && MsX == m_Where.right && (m_Where.height() - (NeedBox? 2 : 0)) < sizeAsInt() && (MouseEvent->dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED))
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

		if (MsPos >= 0 && MsPos < sizeAsInt() && item_can_have_focus(Items[MsPos]))
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

	if (Pos >= sizeAsInt())
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
		return sizeAsInt();

	const auto ItemIterator = std::ranges::find_if(Items, [&](menu_item_ex const& i){ return item_is_visible(i) && !VPos--; });
	return ItemIterator != Items.cend()? ItemIterator - Items.cbegin() : -1;
}

bool VMenu::SetItemHPos(menu_item_ex& Item, const auto& GetNewHPos)
{
	if (Item.Flags & LIF_SEPARATOR) return false;

	const auto ItemLength{ GetItemVisualLength(Item) };
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

	m_HorizontalTracker->update_item_hpos(Item.HorizontalPosition, NewHPos, ItemLength, Item.SafeGetFirstAnnotation());

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

	const auto Guard{ m_HorizontalTracker->start_bulk_update_smart_left_right(NewHPos, TextAreaWidth, Policy) };
	return SetAllItemsHPos(
		[&](const int ItemLength) { return get_item_smart_hpos(NewHPos, ItemLength, TextAreaWidth, Policy); });
}

bool VMenu::ShiftAllItemsHPos(const int Shift)
{
	const auto TextAreaWidth{ CalculateTextAreaWidth() };
	if (TextAreaWidth <= 0) return false;

	const auto AdjustedShift{ adjust_hpos_shift(Shift, m_HorizontalTracker->get_left_boundary(), m_HorizontalTracker->get_right_boundary(), TextAreaWidth)};
	if (!AdjustedShift) return false;

	const auto Policy{ CheckFlags(VMENU_ENABLEALIGNANNOTATIONS) ? item_hscroll_policy::unbound : item_hscroll_policy::bound_stick_to_left };

	const auto Guard{ m_HorizontalTracker->start_bulk_update_shift(AdjustedShift, TextAreaWidth, Policy) };
	return SetAllItemsHPos(
		[&](const int ItemHPos, const int ItemLength) { return get_item_absolute_hpos(ItemHPos + AdjustedShift, ItemLength, TextAreaWidth, Policy); });
}

bool VMenu::AlignAnnotations()
{
	if (!CheckFlags(VMENU_ENABLEALIGNANNOTATIONS)) return false;

	const auto TextAreaWidth{ CalculateTextAreaWidth() };
	if (TextAreaWidth <= 0) return false;
	const auto AlignPos{ (TextAreaWidth + 2) / 4 };

	const auto Guard{ m_HorizontalTracker->start_bulk_update_annotation(AlignPos) };
	return SetAllItemsHPos(
		[&](const menu_item_ex& Item) { return AlignPos - Item.SafeGetFirstAnnotation(); });
}

bool VMenu::ToggleFixedColumns()
{
	if (m_FixedColumns.empty()) return false;

	const auto FixedColumnWidthBefore{
		m_HorizontalTracker->all_items_are_at_home() ? std::nullopt : std::optional{ menu_layout::fixed_columns_width(*this) } };

	toggle_fixed_columns(m_FixedColumns);

	if (FixedColumnWidthBefore)
		(void)ShiftAllItemsHPos(*FixedColumnWidthBefore - menu_layout::fixed_columns_width(*this));

	return true;
}

void VMenu::Show()
{
	if (!CheckFlags(VMENU_LISTBOX))
	{
		bool AutoHeight = false;

		if (!CheckFlags(VMENU_COMBOBOX))
		{
			const auto BoxType{ menu_layout::get_box_type(*this) };
			const auto MenuWidth = GetNaturalMenuWidth();

			bool AutoCenter = false;

			if (m_Where.left == -1)
			{
				m_Where.left = static_cast<short>(std::max((ScrX - MenuWidth) / 2, 0));
				AutoCenter = true;
			}

			if (m_Where.left < 2)
				m_Where.left = 2;

			if (m_Where.right <= 0)
				m_Where.right = static_cast<short>(std::min(m_Where.left + MenuWidth, ScrX) - 1);

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
	// 2023-12-28 MZK: OK, at least combo box may be refreshed without DisplayObject(); then vertical borders are garbled by separators.
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

	AssignHighlights(Layout);

	const auto VisualTopPos{ AdjustTopPos(Layout.BoxType) };

	if (Layout.ClientRect.width() <= 0)
		return;

	std::vector<int> HighlightMarkup;
	const string BlankLine(Layout.ClientRect.width(), L' ');

	for (int Y = Layout.ClientRect.top, I = TopPos; Y <= Layout.ClientRect.bottom; ++Y, ++I)
	{
		if (I >= sizeAsInt())
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
		ScrollBar(*Layout.Scrollbar, Layout.ClientRect.top, Layout.ClientRect.height(), VisualTopPos, GetShowItemCount());
	}
}

void VMenu::DrawTitles() const
{
	if (CheckFlags(VMENU_SHOWNOBOX)) return;

	const auto MaxTitleLength = m_Where.width() - 3;

	if (!strTitle.empty() || bFilterEnabled)
	{
		string strDisplayTitle;
		string_view DisplayTitle;

		if (bFilterEnabled)
		{
			strDisplayTitle = bFilterLocked?
				far::format(L"{}{}<{}>"sv, strTitle, strTitle.empty()? L""sv : L" "sv, strFilter) :
				far::format(L"[{}]"sv, strFilter);

			DisplayTitle = strDisplayTitle;
		}
		else
			DisplayTitle = strTitle;

		auto WidthTitle = static_cast<int>(visual_string_length(DisplayTitle));

		if (WidthTitle > MaxTitleLength)
			WidthTitle = MaxTitleLength - 1;

		GotoXY(m_Where.left + (m_Where.width() - 2 - WidthTitle) / 2, m_Where.top);
		set_color(Colors, color_indices::Title);

		Text(L' ');
		Text(DisplayTitle, MaxTitleLength - 1);
		Text(L' ');
	}

	if (!strBottomTitle.empty())
	{
		auto WidthTitle = static_cast<int>(visual_string_length(strBottomTitle));

		if (WidthTitle > MaxTitleLength)
			WidthTitle = MaxTitleLength - 1;

		GotoXY(m_Where.left + (m_Where.width() - 2 - WidthTitle) / 2, m_Where.bottom);
		set_color(Colors, color_indices::Title);

		Text(L' ');
		Text(strBottomTitle, MaxTitleLength - 1);
		Text(L' ');
	}

	if constexpr ((false))
	{
		set_color(Colors, color_indices::Title);

		GotoXY(m_Where.left + 2, m_Where.bottom);
		MenuText(m_HorizontalTracker->get_debug_string());

		const auto TextAreaWidthLabel{ far::format(L" [{}] "sv, CalculateTextAreaWidth()) };
		GotoXY(m_Where.right - 1 - static_cast<int>(TextAreaWidthLabel.size()), m_Where.bottom);
		MenuText(TextAreaWidthLabel);
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
	MenuText(separator);
}

void VMenu::ConnectSeparator(const size_t ItemIndex, string& separator, const int BoxType) const
{
	// We should think of how to deal with fixed columns and horizontally shifted items.
	// Maybe use fixed columns in the menus where it is necessary
	// and connect separators in trivial cases only (or not at all)?
	if (CheckFlags(VMENU_NOMERGEBORDER) || !m_FixedColumns.empty() || m_ItemTextSegment.start() > 0 || separator.size() <= 3)
		return;

	for (const auto I : std::views::iota(0uz, separator.size() - 3))
	{
		const auto AnyPrev = ItemIndex > 0 && Items[ItemIndex - 1].HorizontalPosition == 0;
		const auto AnyNext = ItemIndex < Items.size() - 1 && Items[ItemIndex + 1].HorizontalPosition == 0;

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

void VMenu::ApplySeparatorName(const menu_item_ex& Item, string& separator) const
{
	if (Item.Name.empty() || separator.size() <= 3)
		return;

	auto NameWidth{ std::min(Item.Name.size(), separator.size() - 2) };
	auto NamePos{ (separator.size() - NameWidth) / 2 };

	separator[NamePos - 1] = L' ';
	separator.replace(NamePos, NameWidth, fit_to_left(Item.Name, NameWidth));
	separator[NamePos + NameWidth] = L' ';
}

void VMenu::DrawRegularItem(const menu_item_ex& Item, const menu_layout& Layout, const int Y, std::vector<int>& HighlightMarkup, const string_view BlankLine) const
{
	const item_color_indices ColorIndices{ Item };

	if (Layout.FixedColumnsArea)
		DrawFixedColumns(Item, *Layout.FixedColumnsArea, Y, ColorIndices, BlankLine);

	const bool NeedRightHScroll{
		Layout.TextArea
		? DrawItemText(Item, *Layout.TextArea, Y, ColorIndices, HighlightMarkup, BlankLine)
		: false
	};

	const auto DrawDecorator = [&](const int X, std::tuple<color_indices, wchar_t> ColorAndChar)
		{
			GotoXY(X, Y);
			set_color(Colors, std::get<color_indices>(ColorAndChar));
			Text(std::get<wchar_t>(ColorAndChar));
		};

	if (Layout.CheckMark)
		DrawDecorator(*Layout.CheckMark, get_item_check_mark(Item, ColorIndices));

	if (Layout.LeftHScroll)
		DrawDecorator(*Layout.LeftHScroll, get_item_left_hscroll(Item.HorizontalPosition < 0, ColorIndices));

	if (Layout.SubMenu)
		DrawDecorator(*Layout.SubMenu, get_item_submenu(Item, ColorIndices));

	if (Layout.RightHScroll)
		DrawDecorator(*Layout.RightHScroll, get_item_right_hscroll(NeedRightHScroll, ColorIndices));
}

void VMenu::DrawFixedColumns(
	const menu_item_ex& Item,
	const small_segment FixedColumnsArea,
	const int Y,
	const item_color_indices& ColorIndices,
	const string_view BlankLine) const
{
	GotoXY(FixedColumnsArea.start(), Y);
	set_color(Colors, ColorIndices.Normal);

	auto CurCellAreaStart{ FixedColumnsArea.start() };
	for (const auto CurFixedColumn : m_FixedColumns | std::views::filter(&vmenu_fixed_column_t::CurrentWidth))
	{
		const segment CellArea{ CurCellAreaStart, segment::length_tag{ CurFixedColumn.CurrentWidth } };

		const auto CellText{ get_item_cell_text(Item.Name, CurFixedColumn.TextSegment) };
		const auto VisibleCellTextSegment{ intersect(
			segment{ 0, segment::length_tag{ static_cast<segment::domain_t>(CellText.size()) } },
			segment{ 0, segment::length_tag{ CellArea.length()}})};

		if (!VisibleCellTextSegment.empty())
			MenuText(CellText.substr(VisibleCellTextSegment.start(), VisibleCellTextSegment.length()));

		MenuText(BlankLine.substr(0, CellArea.end() - WhereX()));
		MenuText(CurFixedColumn.Separator);

		CurCellAreaStart = CellArea.end() + 1;
	}

	assert(WhereX() == FixedColumnsArea.end());
}

bool VMenu::DrawItemText(
	const menu_item_ex& Item,
	const small_segment TextArea,
	const int Y,
	const item_color_indices& ColorIndices,
	std::vector<int>& HighlightMarkup,
	string_view BlankLine) const
{
	GotoXY(TextArea.start(), Y);
	set_color(Colors, ColorIndices.Normal);

	Text(BlankLine.substr(0, std::clamp(Item.HorizontalPosition, 0, static_cast<int>(TextArea.length()))));

	const auto [ItemText, HighlightPos]{ [&]{
		const auto RawItemText_{ GetItemText(Item) };
		auto HotkeyPos_{ string::npos };
		auto ItemText_{ CheckFlags(VMENU_SHOWAMPERSAND) ? string{ RawItemText_ } : HiText2Str(RawItemText_, &HotkeyPos_) };
		std::ranges::replace(ItemText_, L'\t', L' ');

		std::optional<int> HighlightPos_;
		if (HotkeyPos_ != string::npos) HighlightPos_ = static_cast<int>(HotkeyPos_);
		if (!HighlightPos_ && Item.AutoHotkey) HighlightPos_ = static_cast<int>(Item.AutoHotkeyPos);

		return std::tuple{ ItemText_, HighlightPos_ };
	}() };

	const auto VisibleTextSegment{ intersect(
		segment{ 0, segment::length_tag{ static_cast<segment::domain_t>(ItemText.size()) } },
		segment::ray(-Item.HorizontalPosition))};

	if (!VisibleTextSegment.empty())
	{
		markup_slice_boundaries(VisibleTextSegment, Item.Annotations, HighlightPos, HighlightMarkup);

		auto CurColorIndex{ ColorIndices.Normal };
		auto AltColorIndex{ ColorIndices.Highlighted };
		auto CurTextPos{ VisibleTextSegment.start() };

		for (const auto SliceEnd : HighlightMarkup)
		{
			set_color(Colors, CurColorIndex);
			Text(string_view{ ItemText }.substr(CurTextPos, SliceEnd - CurTextPos), TextArea.end() - WhereX());
			std::ranges::swap(CurColorIndex, AltColorIndex);
			CurTextPos = SliceEnd;
		}
	}

	set_color(Colors, ColorIndices.Normal);

	if (WhereX() < TextArea.end())
	{
		Text(BlankLine, TextArea.end() - WhereX());
		assert(WhereX() == TextArea.end());
	}

	return Item.HorizontalPosition + static_cast<int>(visual_string_length(ItemText)) > TextArea.length();
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

wchar_t VMenu::GetHighlights(const menu_item_ex* const Item) const
{
	if (!Item)
		return 0;

	if (Item->AutoHotkey)
		return Item->AutoHotkey;

	if (CheckFlags(VMENU_SHOWAMPERSAND))
		return 0;

	wchar_t Ch;
	return HiTextHotkey(GetItemText(*Item), Ch)? Ch : 0;
}

void VMenu::AssignHighlights(const menu_layout& Layout)
{
	if (!CheckFlags(VMENU_AUTOHIGHLIGHT)) return;

	static_assert(sizeof(wchar_t) == 2, "512 MB for a bitset is too much, rewrite it.");
	std::bitset<std::numeric_limits<wchar_t>::max() + 1> Used;

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

	const auto SetItemHotkey{ [](auto& Item, const wchar_t Hotkey, const size_t HotkeyVisualPos)
	{
		Item.AutoHotkey = Hotkey;
		Item.AutoHotkeyPos = HotkeyVisualPos;
	} };

	const auto ClearItemHotkey{ [&](auto& Item)
	{
		SetItemHotkey(Item, {}, {});
	} };

	const auto Reverse{ CheckFlags(VMENU_REVERSEHIGHLIGHT) };
	const auto Delta{ Reverse ? -1 : 1 };
	const auto ItemsSize{ sizeAsInt() };

	// проверка заданных хоткеев
	if (CheckFlags(VMENU_SHOWAMPERSAND))
		for (auto I = Reverse ? ItemsSize - 1 : 0; I >= 0 && I < ItemsSize; I += Delta)
		{
			ClearItemHotkey(Items[I]);
		}
	else
		for (auto I = Reverse ? ItemsSize - 1 : 0; I >= 0 && I < ItemsSize; I += Delta)
		{
			auto& Item{ Items[I] };
			wchar_t Hotkey{};
			size_t HotkeyVisualPos{};
			// TODO: проверка на LIF_HIDDEN
			if (HiTextHotkey(GetItemText(Item), Hotkey, &HotkeyVisualPos) && RegisterHotkey(Hotkey))
				SetItemHotkey(Item, Hotkey, HotkeyVisualPos);
			else
				ClearItemHotkey(Item);
		}

	// TODO:  ЭТОТ цикл нужно уточнить - возможно вылезут артефакты (хотя не уверен)
	for (auto I = Reverse ? ItemsSize - 1 : 0; I >= 0 && I < ItemsSize; I += Delta)
	{
		auto& Item{ Items[I] };
		if (Item.AutoHotkey) continue;

		const auto ItemText = GetItemText(Item);
		const auto VisibleTextSegment{ intersect(
			segment{ 0, segment::length_tag{ static_cast<segment::domain_t>(ItemText.size()) } },
			segment::ray(-Item.HorizontalPosition)) };
		if (VisibleTextSegment.empty()) continue;

		if (const auto FoundHotKey{
			std::ranges::find_if(
				std::ranges::next(ItemText.cbegin(), VisibleTextSegment.start()),
				std::ranges::next(ItemText.cbegin(), VisibleTextSegment.end()),
				// TODO: проверка на LIF_HIDDEN
				[&](const auto Ch) { return (Ch == L'&' || is_alpha(Ch) || std::iswdigit(Ch)) && RegisterHotkey(Ch); }) };
			FoundHotKey != ItemText.cend())
		{
			SetItemHotkey(Item, *FoundHotKey, std::ranges::distance(ItemText.cbegin(), FoundHotKey));
		}
	}
}

bool VMenu::CheckKeyHiOrAcc(DWORD Key, int Type, bool Translate, bool ChangePos, int& NewPos)
{
	//не забудем сбросить EndLoop для листбокса, иначе не будут работать хоткеи в активном списке
	if (CheckFlags(VMENU_LISTBOX))
		ClearDone();

	for (const auto& Item : Items)
	{
		if (!item_can_have_focus(Item)) continue;

		if ((!Type && Item.AccelKey && Key == Item.AccelKey)
			|| (Type
				&& (Item.AutoHotkey || !CheckFlags(VMENU_SHOWAMPERSAND))
				&& IsKeyHighlighted(GetItemText(Item), Key, Translate, Item.AutoHotkey)))
		{
			NewPos = static_cast<int>(std::ranges::distance(Items.data(), &Item));
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

void VMenu::UpdateMaxLength(int const ItemLength)
{
	m_MaxItemLength = std::max(
		m_MaxItemLength,
		intersect(segment{ 0, segment::length_tag{ ItemLength } }, m_ItemTextSegment).length());
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
}

void VMenu::SetTitle(string_view const Title)
{
	SetMenuFlags(VMENU_UPDATEREQUIRED);
	strTitle = Title;
}

void VMenu::SetFixedColumns(std::vector<vmenu_fixed_column_t>&& FixedColumns, segment ItemTextSegment)
{
	m_FixedColumns = std::move(FixedColumns);
	for (auto& column : m_FixedColumns)
	{
		column.CurrentWidth = std::clamp(column.CurrentWidth, int{}, column.TextSegment.length());
	}
	m_ItemTextSegment = ItemTextSegment;
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

void VMenu::GetColors(FarDialogItemColors const* ColorsOut)
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
		Info->MaxLength = std::max(m_Where.width() - menu_layout::get_service_area_size(*this), 0);
		return true;
	}

	return false;
}

// Функция GetItemPtr - получить указатель на нужный Items.
menu_item_ex& VMenu::at(size_t n)
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

void VMenu::RegisterExtendedDataProvider(extended_item_data_provider&& ExtendedDataProvider)
{
	m_ExtendedDataProvider = std::move(ExtendedDataProvider);
}

FarListItem *VMenu::MenuItem2FarList(const menu_item_ex *MItem, FarListItem *FItem)
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
			// Consider: Strictly speaking, we should remove highlight
			// only within m_ItemTextSegment leaving everything else intact.
			const auto strTmpBuf = remove_highlight(Items[I].Name);

			if (Flags&LIFIND_EXACTMATCH)
			{
				if (strTmpBuf == Pattern)
					return static_cast<int>(I);
			}
			else
			{
				if (CmpName(Pattern, strTmpBuf, false))
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
	SortItems([](const menu_item_ex& a, const menu_item_ex& b, const SortItemParam& Param)
	{
		// Consider: Strictly speaking, we should remove highlight
		// only within m_ItemTextSegment leaving everything else intact.
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

// Consider: Do we need this function? Maybe client should rely on VMENU_AUTOHIGHLIGHT?
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
		const auto Hl = HiStrlen(Item.Name) != visual_string_length(Item.Name);
		Str = fit_to_left(Item.Name, MaxLength + (Hl? 2 : 1)) + Key;
		Item.Name = Str;
	}

	return Result;
}

int VMenu::GetNaturalMenuWidth() const
{
	const auto NeedBox = menu_layout::need_box(*this);
	return std::max(
		m_MaxItemLength + menu_layout::get_service_area_size(*this, NeedBox),
		static_cast<int>(std::max(visual_string_length(strTitle), visual_string_length(strBottomTitle)) + menu_layout::get_title_service_area_size(NeedBox))
	);
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
	return TextArea ? TextArea->length() : 0;
}

int VMenu::GetItemVisualLength(const menu_item_ex& Item) const
{
	const auto ItemCellText{ get_item_cell_text(Item.Name, m_ItemTextSegment) };
	return static_cast<int>(CheckFlags(VMENU_SHOWAMPERSAND) ? visual_string_length(ItemCellText) : HiStrlen(ItemCellText));
}

string_view VMenu::GetItemText(const menu_item_ex& Item) const
{
	return get_item_cell_text(Item.Name, m_ItemTextSegment);
}

size_t VMenu::MenuText(string_view const Str) const
{
	return Text(Str, m_Where.width() - (WhereX() - m_Where.left));
}

size_t VMenu::MenuText(wchar_t const Char) const
{
	return MenuText({ &Char, 1 });
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

TEST_CASE("markup.slice.boundaries")
{
	struct test_data
	{
		struct test_segment: public segment
		{
			test_segment(int const Begin, int const End)
				: segment{ Begin, segment::sentinel_tag{ End } }
			{}
		};
		test_segment Segment;
		std::initializer_list<test_segment> Slices;
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
		{ { 20, 50 }, { { 0, 0 } }, { 50 } },
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
