/*
color_picker.cpp

Standard color picker
*/
/*
Copyright © 2022 Far Group
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
#include "color_picker.hpp"

// Internal:
#include "color_picker_common.hpp"
#include "color_picker_256.hpp"
#include "color_picker_rgb.hpp"
#include "farcolor.hpp"
#include "dialog.hpp"
#include "interf.hpp"
#include "config.hpp"
#include "colormix.hpp"
#include "lang.hpp"
#include "global.hpp"
#include "strmix.hpp"
#include "console.hpp"

// Platform:

// Common:
#include "common.hpp"
#include "common/2d/algorithm.hpp"
#include "common/from_string.hpp"
#include "common/null_iterator.hpp"
#include "common/scope_exit.hpp"

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

static constexpr auto IndexColors = []
{
	std::array<uint8_t, colors::index::nt_size> Result;

	for (uint8_t i = 0; i != Result.size(); ++i)
		Result[i] = detail::bg(i) | detail::fg(colors::index::nt_last - i);

	return Result;
}();
static_assert(std::size(IndexColors) == colors::index::nt_size);

static constexpr auto
	ColorsWidth = 8,
	ColorsHeight = colors::index::nt_size / ColorsWidth;

static constexpr auto control_by_color = column_major_iota<uint8_t, ColorsWidth, ColorsHeight>();
static_assert(std::size(control_by_color) == std::size(IndexColors));

static constexpr auto color_by_control = column_major_iota<uint8_t, ColorsHeight, ColorsWidth>();
static_assert(std::size(color_by_control) == std::size(IndexColors));

static auto color_code(colors::single_color const Color)
{
	return Color.IsIndex? colors::is_default(Color.Value)?
		far::format(L"{:02X}      "sv, colors::alpha_value(Color.Value)) :
		far::format(L"{:02X}    {:02X}"sv, colors::alpha_value(Color.Value), colors::index_value(Color.Value)) :
		far::format(L"{:08X}"sv, colors::ARGB2ABGR(Color.Value));
}

static std::optional<COLORREF> parse_color(string_view const Str, bool const IsIndex, bool const IsDefault)
{
	if (IsIndex)
	{
		unsigned Alpha;
		if (!from_string(Str.substr(0, 2), Alpha, {}, 16))
			return {};

		COLORREF Result{};

		if (IsDefault)
		{
			Result = colors::default_colorref();
		}
		else
		{
			unsigned Index;
			if (!from_string(Str.substr(6, 2), Index, {}, 16))
				return {};

			colors::set_index_value(Result, Index);
		}

		colors::set_alpha_value(Result, Alpha);

		return Result;
	}

	unsigned Value;
	if (!from_string(Str, Value, {}, 16))
		return {};

	return colors::ARGB2ABGR(Value);
}

// BUGBUG
static bool IgnoreEditChange = false;
static bool IgnoreColorIndexClick = false;

constexpr auto
	MaskIndex = L"HH    HH"sv,
	MaskARGB  = L"HHHHHHHH"sv,
	MaskDef   = L"HH      "sv;

namespace color_basic
{
	enum controls
	{
		color_text,
		color_active_checkbox,
		color_first_radio,
		color_last_radio = color_first_radio + colors::index::nt_last + 1,
		color_default_radio = color_last_radio,
		color_default_text,

		colorcode_text,
		colorcode_edit,

		button_256,
		button_rgb,

		count
	};
}

namespace cb = color_basic;

namespace color_dialog
{
	enum controls
	{
		border,

		separator_before_hint,
		separator_before_buttons,
		separator_vertical_before_style,
		separator_after_foreground,
		separator_after_background,
		separator_style,

		fg_first,
		fg_last = fg_first + std::to_underlying(cb::count) - 1,

		bg_first,
		bg_last = bg_first + std::to_underlying(cb::count) - 1,

		style_text,
		style_checkbox_first,
		style_checkbox_inherit = style_checkbox_first,
		style_checkbox_bold,
		style_checkbox_italic,
		style_checkbox_overline,
		style_checkbox_strikeout,
		style_checkbox_faint,
		style_checkbox_blink,
		style_checkbox_inverse,
		style_checkbox_invisible,
		style_checkbox_last = style_checkbox_invisible,
		style_underline_text,
		style_underline_combo,
		style_underline_color_button,

		sample_text_first,
		sample_text_last = sample_text_first + 2,

		vt_hint_text_intro,
		vt_hint_text_supported,
		vt_hint_text_not_supported,

		button_ok,
		button_reset,
		button_enable_vt,
		button_cancel,

		count
	};
}

namespace cd = color_dialog;

static consteval auto fg_item(cb::controls const Item)
{
	return static_cast<cd::controls>(cd::fg_first + static_cast<size_t>(Item));
}

static consteval auto bg_item(cb::controls const Item)
{
	return static_cast<cd::controls>(cd::bg_first + static_cast<size_t>(Item));
}

static const std::pair<cd::controls, FARCOLORFLAGS> StyleMapping[]
{
	{ cd::style_checkbox_inherit,     FCF_INHERIT_STYLE  },
	{ cd::style_checkbox_bold,        FCF_FG_BOLD        },
	{ cd::style_checkbox_italic,      FCF_FG_ITALIC      },
	{ cd::style_checkbox_overline,    FCF_FG_OVERLINE    },
	{ cd::style_checkbox_strikeout,   FCF_FG_STRIKEOUT   },
	{ cd::style_checkbox_faint,       FCF_FG_FAINT       },
	{ cd::style_checkbox_blink,       FCF_FG_BLINK       },
	{ cd::style_checkbox_inverse,     FCF_FG_INVERSE     },
	{ cd::style_checkbox_invisible,   FCF_FG_INVISIBLE   },
};

struct single_color_state
{
	colors::single_color CurColor;
	bool TransparencyEnabled;
	std::function<void()> RefreshColor;
	size_t Offset;

	intptr_t GetSingleColorDlgProc(Dialog* Dlg, intptr_t Msg, intptr_t Param1, void* Param2);
};

struct color_state
{
	FarColor CurColor, BaseColor;
	bool TransparencyEnabled;
	single_color_state Fg, Bg;

	void refresh_fg()
	{
		CurColor.SetFgIndex(Fg.CurColor.IsIndex);
		CurColor.ForegroundColor = Fg.CurColor.Value;
	}

	void refresh_bg()
	{
		CurColor.SetBgIndex(Bg.CurColor.IsIndex);
		CurColor.BackgroundColor = Bg.CurColor.Value;
	}

	std::optional<intptr_t> delegate_proc(Dialog* Dlg, intptr_t Msg, intptr_t Param1, void* Param2)
	{
		if (in_closed_range(cd::fg_first, Param1, cd::fg_last))
			return Fg.GetSingleColorDlgProc(Dlg, Msg, Param1, Param2);

		if (in_closed_range(cd::bg_first, Param1, cd::bg_last))
			return Bg.GetSingleColorDlgProc(Dlg, Msg, Param1, Param2);

		return {};
	}

	intptr_t GetColorDlgProc(Dialog* Dlg, intptr_t Msg, intptr_t Param1, void* Param2);
};

static const auto
	DM_UPDATECOLORCODE = DM_USER + 1,
	DM_UPDATEPREVIEW   = DM_USER + 2;

intptr_t single_color_state::GetSingleColorDlgProc(Dialog* Dlg, intptr_t Msg, intptr_t Param1, void* Param2)
{
	const auto GetColor = [&]
	{
		if (Param1 - Offset == cb::color_default_radio)
		{
			auto Color = colors::default_color();
			if (Offset != cd::bg_first)
				Color.Flags |= FCF_FG_INVERSE;

			return Color;
		}

		auto Color = colors::NtColorToFarColor(IndexColors[color_by_control[Param1 - Offset - cb::color_first_radio]]);
		flags::clear(Color.Flags, FCF_INHERIT_STYLE);
		return Color;
	};

	switch (Msg)
	{
	case DN_INITDIALOG:
		Dlg->SendMessage(DM_EDITUNCHANGEDFLAG, Offset + cb::colorcode_edit, {});
		break;

	case DN_CTLCOLORDLGITEM:
		if (in_closed_range(cb::color_first_radio, as_signed(Param1 - Offset), cb::color_last_radio))
		{
			if (TransparencyEnabled && colors::is_transparent(CurColor.Value))
				return FALSE;

			const auto& Colors = *static_cast<FarDialogItemColors const*>(Param2);
			Colors.Colors[0] = GetColor();

			return TRUE;
		}
		break;

	case DN_BTNCLICK:
		if (Param2 && in_closed_range(cb::color_first_radio, as_signed(Param1 - Offset), cb::color_last_radio))
		{
			if (IgnoreColorIndexClick)
				return TRUE;

			CurColor.Value = Param1 - Offset == cb::color_default_radio?
				colors::default_colorref() :
				GetColor().BackgroundColor;

			CurColor.IsIndex = true;

			RefreshColor();

			Dlg->SendMessage(DM_UPDATECOLORCODE, Offset + cb::colorcode_edit, {});

			return TRUE;
		}

		if (Param1 - Offset == cb::color_active_checkbox)
		{
			Param2? colors::make_opaque(CurColor.Value) : colors::make_transparent(CurColor.Value);
			RefreshColor();

			SCOPED_ACTION(Dialog::suppress_redraw)(Dlg);
			for (const auto i: std::views::iota(0, colors::index::nt_size + 2))
			{
				Dlg->SendMessage(DM_ENABLE, Offset + cb::color_first_radio + i, Param2);
			}

			Dlg->SendMessage(DM_UPDATECOLORCODE, Offset + cb::colorcode_edit, {});

			Dlg->SendMessage(DM_ENABLE, Offset + cb::colorcode_edit, Param2);
			Dlg->SendMessage(DM_ENABLE, Offset + cb::colorcode_text, Param2);
			Dlg->SendMessage(DM_ENABLE, Offset + cb::button_256, Param2);
			Dlg->SendMessage(DM_ENABLE, Offset + cb::button_rgb, Param2);

			return TRUE;
		}

		if (Param1 - Offset == cb::button_256)
		{
			const auto ResolvedColor = CurColor.IsIndex? colors::resolve_default(CurColor.Value, Offset != cd::bg_first) : CurColor.Value;

			FarColor FakeColor{ .BackgroundColor = ResolvedColor };
			FakeColor.SetBgIndex(CurColor.IsIndex);

			if (auto Color = colors::index_value(colors::FarColorToConsole256Color(FakeColor).BackgroundIndex); pick_color_256(Color))
			{
				colors::set_index_value(CurColor.Value, Color);
				CurColor.IsIndex = true;
				RefreshColor();

				Dlg->SendMessage(DM_SETCHECK, Offset + cb::color_first_radio, ToPtr(BSTATE_3STATE));
				Dlg->SendMessage(DM_UPDATECOLORCODE, Offset + cb::colorcode_edit, {});

			}
			return TRUE;
		}

		if (Param1 - Offset == cb::button_rgb)
		{
			const auto ResolvedColor = CurColor.IsIndex? colors::resolve_default(CurColor.Value, Offset != cd::bg_first) : CurColor.Value;

			auto Color = colors::color_value(
				CurColor.IsIndex?
				colors::ConsoleIndexToTrueColor(ResolvedColor) :
				ResolvedColor
			);

			if (auto CustomColors = Global->Opt->Palette.GetCustomColors(); pick_color_rgb(Color, CustomColors))
			{
				colors::set_color_value(CurColor.Value, Color);
				CurColor.IsIndex = false;
				RefreshColor();

				Dlg->SendMessage(DM_SETCHECK, Offset + cb::color_first_radio, ToPtr(BSTATE_3STATE));
				Dlg->SendMessage(DM_UPDATECOLORCODE, Offset + cb::colorcode_edit, {});
				Global->Opt->Palette.SetCustomColors(CustomColors);
			}

			return TRUE;
		}
		break;

	case DN_EDITCHANGE:
		if (!IgnoreEditChange && Param1 - Offset == cb::colorcode_edit)
		{
			const auto& Item = *static_cast<const FarDialogItem*>(Param2);
			const auto Iterator = null_iterator(Item.Data);
			if (!std::any_of(Iterator, Iterator.end(), std::iswxdigit))
				return false;

			const auto ParsedColor = parse_color(Item.Data, CurColor.IsIndex, colors::is_default(CurColor.Value));
			if (!ParsedColor)
				return false;

			const auto OldColorValue = colors::color_value(CurColor.Value);
			CurColor.Value = *ParsedColor;
			RefreshColor();

			if (CurColor.IsIndex)
			{
				if (const auto NewColorValue = colors::color_value(CurColor.Value); NewColorValue != OldColorValue)
				{
					IgnoreColorIndexClick = true;
					SCOPE_EXIT{ IgnoreColorIndexClick = false; };
					const auto ColorButtonIndex = NewColorValue < 8? NewColorValue * 2 : (NewColorValue - 8) * 2 + 1;
					Dlg->SendMessage(DM_SETCHECK, Offset + cb::color_first_radio + ColorButtonIndex, ToPtr(BSTATE_CHECKED));
				}
			}

			if (TransparencyEnabled && colors::is_transparent(CurColor.Value))
			{
				Dlg->SendMessage(DM_SETCHECK, Offset + cb::color_active_checkbox, ToPtr(BSTATE_UNCHECKED));
				Dlg->SendMessage(DN_BTNCLICK, Offset + cb::color_active_checkbox, ToPtr(BSTATE_UNCHECKED));
			}
		}
		break;

	case DM_UPDATECOLORCODE:
		{
			IgnoreEditChange = true;
			SCOPE_EXIT{ IgnoreEditChange = false; };

			FarDialogItem Item;
			if (!Dlg->SendMessage(DM_GETDLGITEMSHORT, Offset + cb::colorcode_edit, &Item))
				return false;

			const auto IsDefault = colors::is_default(CurColor.Value);
			const auto Value = color_code(CurColor);

			Item.Mask = (CurColor.IsIndex? IsDefault? MaskDef : MaskIndex: MaskARGB).data();
			Item.Data = UNSAFE_CSTR(Value);

			Dlg->SendMessage(DM_SETDLGITEM, Offset + cb::colorcode_edit, &Item);

			constexpr lng Titles[][3]
			{
				{ lng::MSetColorForeIndex, lng::MSetColorForeAARRGGBB, lng::MSetColorForeDefault },
				{ lng::MSetColorBackIndex, lng::MSetColorBackAARRGGBB, lng::MSetColorBackDefault },
			};

			Dlg->SendMessage(DM_SETTEXTPTR,
				Offset + cb::colorcode_text,
				UNSAFE_CSTR(msg(Titles[Offset == cd::fg_first? 0 : 1][CurColor.IsIndex? IsDefault? 2 : 0 : 1]))
			);
		}
		return TRUE;

	default:
		break;
	}

	return Dlg->DefProc(Msg, Param1, Param2);
}

#define COLOR_COLUMN(x, y, index) \
	COLOR_CELL(x + 3 * index, y + 0), \
	COLOR_CELL(x + 3 * index, y + 1)

#define COLOR_PLANE(column, x, y) \
	column(x, y,  0), \
	column(x, y,  1), \
	column(x, y,  2), \
	column(x, y,  3), \
	column(x, y,  4), \
	column(x, y,  5), \
	column(x, y,  6), \
	column(x, y,  7)

namespace single_color_dialog
{
	enum controls
	{
		border,

		separator_before_buttons,

		color_first,
		color_last = color_first + std::to_underlying(cb::count) - 1,

		button_ok,
		button_cancel,

		count
	};
}

namespace scd = single_color_dialog;

static consteval auto scd_item(cb::controls const Item)
{
	return static_cast<scd::controls>(scd::color_first + static_cast<size_t>(Item));
}

static std::optional<size_t> get_control_id(COLORREF const ColorPart, size_t const Offset)
{
	if (colors::is_default(ColorPart))
		return Offset + cb::color_first_radio + colors::index::nt_size;

	const auto Index = colors::index_value(ColorPart);
	if (Index > colors::index::nt_last)
		return {};

	return Offset + cb::color_first_radio + control_by_color[Index];
}

static auto activate_control(COLORREF const Color, std::span<DialogItemEx> ColorDlgItems, size_t const Offset)
{
	const auto ControlId = get_control_id(Color, Offset);
	if (!ControlId)
		return false;

	ColorDlgItems[*ControlId].Selected = true;
	ColorDlgItems[*ControlId].Flags |= DIF_FOCUS;
	return true;
}

static void disable_if_needed(COLORREF const Color, std::span<DialogItemEx> ColorDlgItems, size_t const Offset)
{
	if (colors::is_transparent(Color))
	{
		for (const auto i: std::views::iota(cb::color_first_radio + 0, cb::color_last_radio + 2))
		{
			ColorDlgItems[Offset + i].Flags |= DIF_DISABLE;
		}

		ColorDlgItems[Offset + cb::colorcode_edit].Flags |= DIF_DISABLE;
		ColorDlgItems[Offset + cb::colorcode_text].Flags |= DIF_DISABLE;
		ColorDlgItems[Offset + cb::button_256].Flags |= DIF_DISABLE;
		ColorDlgItems[Offset + cb::button_rgb].Flags |= DIF_DISABLE;
	}
	else
	{
		ColorDlgItems[Offset + cb::color_active_checkbox].Selected = BSTATE_CHECKED;
	}
}

static bool pick_color_single(colors::single_color& Color, colors::single_color const BaseColor, std::array<COLORREF, 16>& CustomColors)
{
	const auto
		Cl4X = 5,
		Cl4Y = 2,
		ButtonY = 7;

	auto ColorDlg = MakeDialogItems<scd::count>(
	{
		{ DI_DOUBLEBOX,   {{3,  1 }, {39, ButtonY+1}}, DIF_NONE, msg(lng::MSetColorTitle), },
		{ DI_TEXT,        {{-1, ButtonY-1}, {0, ButtonY-1}}, DIF_SEPARATOR, },
		{ DI_TEXT,        {{Cl4X, Cl4Y}, {0, Cl4Y}}, DIF_NONE, msg(lng::MSetColorStyleUnderline), },
		{ DI_CHECKBOX,    {{Cl4X, Cl4Y}, {0, Cl4Y}}, DIF_NONE, msg(lng::MSetColorStyleUnderline), },
		COLOR_PLANE(COLOR_COLUMN, Cl4X, Cl4Y + 1),
		COLOR_CELL(Cl4X, Cl4Y + 3),
		{ DI_TEXT, { { Cl4X + 4, Cl4Y + 3 }, { 0, Cl4Y + 3 } }, DIF_NONE, msg(lng::MSetColorForegroundDefault) },

		{ DI_TEXT,        {{30, 2 }, {0,  2 }}, DIF_NONE, msg(Color.IsIndex? colors::is_default(Color.Value)? lng::MSetColorForeDefault : lng::MSetColorForeIndex : lng::MSetColorForeAARRGGBB) },
		{ DI_FIXEDIT,     {{30, 3 }, {37, 3 }}, DIF_MASKEDIT, },
		{ DI_BUTTON,      {{30, 4 }, {37, 4 }}, DIF_NONE, msg(lng::MSetColorFore256), },
		{ DI_BUTTON,      {{30, 5 }, {37, 5 }}, DIF_NONE, msg(lng::MSetColorForeRGB), },

		{ DI_BUTTON,      {{0, ButtonY}, {0, ButtonY}}, DIF_CENTERGROUP | DIF_DEFAULTBUTTON, msg(lng::MSetColorSet), },
		{ DI_BUTTON,      {{0, ButtonY}, {0, ButtonY}}, DIF_CENTERGROUP, msg(lng::MSetColorCancel), },
	});

	ColorDlg[scd::color_first].Flags |= DIF_GROUP;

	ColorDlg[scd_item(cb::colorcode_edit)].strData = color_code(Color);
	ColorDlg[scd_item(cb::colorcode_edit)].strMask = Color.IsIndex? colors::is_default(Color.Value)? MaskDef : MaskIndex : MaskARGB;

	single_color_state ColorState
	{
		.CurColor = Color,
		.TransparencyEnabled = true,
		.RefreshColor = []{},
		.Offset = scd::color_first,
	};

	auto UndelineColorControlActivated = false;

	if (Color.IsIndex)
		UndelineColorControlActivated = activate_control(Color.Value, ColorDlg, scd::color_first);

	if (!UndelineColorControlActivated)
		ColorDlg[scd_item(cb::colorcode_edit)].Flags |= DIF_FOCUS;

	if constexpr ((true))
	{
		ColorDlg[scd_item(cb::color_text)].Flags |= DIF_HIDDEN | DIF_DISABLE;
		disable_if_needed(Color.Value, ColorDlg, scd::color_first);
	}
	else
	{
		ColorDlg[scd_item(cb::color_active_checkbox)].Flags |= DIF_HIDDEN | DIF_DISABLE;
	}

	const auto Dlg = Dialog::create(ColorDlg, std::bind_front(&single_color_state::GetSingleColorDlgProc, &ColorState));

	const auto
		DlgWidth = static_cast<int>(ColorDlg[scd::border].X2) + 4,
		DlgHeight = static_cast<int>(ColorDlg[scd::border].Y2) + 2;

	Dlg->SetPosition({ -1, -1, DlgWidth, DlgHeight });

	Dlg->SetHelp(L"ColorPicker"sv);
	Dlg->Process();

	switch (Dlg->GetExitCode())
	{
	case scd::button_ok:
		Color = ColorState.CurColor;
		return true;

	default:
		return false;
	}
}

intptr_t color_state::GetColorDlgProc(Dialog* Dlg, intptr_t Msg, intptr_t Param1, void* Param2)
{
	switch (Msg)
	{
	case DN_INITDIALOG:
		Fg.GetSingleColorDlgProc(Dlg, Msg, Param1, Param2);
		Bg.GetSingleColorDlgProc(Dlg, Msg, Param1, Param2);
		break;

	case DN_CTLCOLORDLGITEM:
		if (const auto Result = delegate_proc(Dlg, Msg, Param1, Param2))
			return *Result;

		if (in_closed_range(cd::sample_text_first, Param1, cd::sample_text_last))
		{
			const auto& Colors = *static_cast<FarDialogItemColors const*>(Param2);
			Colors.Colors[0] = colors::merge(BaseColor, CurColor);
			return TRUE;
		}
		break;

	case DN_BTNCLICK:
		if (const auto Result = delegate_proc(Dlg, Msg, Param1, Param2))
			return *Result;

		if (in_closed_range(cd::style_checkbox_first, Param1, cd::style_checkbox_last))
		{
			flags::change(CurColor.Flags, StyleMapping[Param1 - cd::style_checkbox_first].second, Param2 != nullptr);
			return TRUE;
		}

		if (Param1 == cd::style_underline_color_button)
		{
			auto Color = colors::single_color::underline(CurColor);
			if (auto CustomColors = Global->Opt->Palette.GetCustomColors(); pick_color_single(Color, colors::single_color::underline(BaseColor), CustomColors))
			{
				CurColor.SetUnderlineIndex(Color.IsIndex);
				CurColor.UnderlineColor = Color.Value;
				Global->Opt->Palette.SetCustomColors(CustomColors);
				Dlg->SendMessage(DM_UPDATEPREVIEW, 0, {});
			}
			return TRUE;
		}

		if (Param1 == cd::button_enable_vt)
		{
			Global->Opt->VirtualTerminalRendering = true;
			SetFarConsoleMode();

			Dlg->SendMessage(DM_ENABLE, cd::button_enable_vt, ToPtr(FALSE));

			return TRUE;
		}
		break;

	case DN_EDITCHANGE:
		if (const auto Result = delegate_proc(Dlg, Msg, Param1, Param2))
			return *Result;

		if (Param1 == cd::style_underline_combo)
			CurColor.SetUnderline(static_cast<UNDERLINE_STYLE>(Dlg->SendMessage(DM_LISTGETCURPOS, cd::style_underline_combo, {})));

		break;

	case DM_UPDATEPREVIEW:
		{
			SCOPED_ACTION(Dialog::suppress_redraw)(Dlg);
			for (const auto i: std::views::iota(cd::sample_text_first + 0, cd::sample_text_last + 1))
			{
				Dlg->SendMessage(DM_SHOWITEM, i, ToPtr(1));
			}
			return TRUE;
		}

	case DM_UPDATECOLORCODE:
		if (delegate_proc(Dlg, Msg, Param1, Param2))
		{
			Dlg->SendMessage(DM_UPDATEPREVIEW, 0, {});
			return TRUE;
		}
		return FALSE;

	default:
		break;
	}

	return Dlg->DefProc(Msg, Param1, Param2);
}

bool GetColorDialog(FarColor& Color, bool const bCentered, const FarColor* const BaseColor, bool* const Reset)
{
	colors::invalidate_cache();

	const auto IsVtActive = console.IsVtActive();
	const auto IsVtSupported = console.IsVtSupported();
	const auto ShowVtHint = !IsVtActive && !console.ExternalRendererLoaded();

	const auto
		Fg4X = 5,
		Fg4Y = 2,
		Bg4X = 5,
		Bg4Y = 7,
		SampleX = 5,
		SampleY = 12,
		SampleW = 32,
		StyleX = 41,
		StyleY = 2,
		HintX = 5,
		HintY = 16,
		HintW = 55,
		ButtonY = 19 - (ShowVtHint? 0 : 3);

	auto ColorDlg = MakeDialogItems<cd::count>(
	{
		{ DI_DOUBLEBOX,   {{3,  1 }, {62, ButtonY+1}}, DIF_NONE, msg(lng::MSetColorTitle), },

		{ DI_TEXT,        {{-1, HintY-1}, {0, HintY - 1}}, DIF_SEPARATOR, },
		{ DI_TEXT,        {{-1, ButtonY-1}, {0, ButtonY-1}}, DIF_SEPARATOR, },
		{ DI_VTEXT,       {{39, 1 }, {39, HintY-1}}, DIF_SEPARATORUSER, },
		{ DI_TEXT,        {{3,  6 }, {39, 6 }}, DIF_SEPARATORUSER, },
		{ DI_TEXT,        {{3,  11}, {39, 11}}, DIF_SEPARATORUSER, },
		{ DI_TEXT,        {{41, 4 }, {60, 4 }}, DIF_SEPARATORUSER, },

		{ DI_TEXT,        {{Fg4X, Fg4Y}, {0, Fg4Y}}, DIF_NONE, msg(lng::MSetColorForeground), },
		{ DI_CHECKBOX,    {{Fg4X, Fg4Y}, {0, Fg4Y}}, DIF_NONE, msg(lng::MSetColorForeground), },

		COLOR_PLANE(COLOR_COLUMN, Fg4X, Fg4Y + 1),
		COLOR_CELL(Fg4X, Fg4Y + 3),
		{ DI_TEXT, { { Fg4X + 4, Fg4Y + 3 }, { 0, Fg4Y + 3 } }, DIF_NONE, msg(lng::MSetColorForegroundDefault) },

		{ DI_TEXT,        {{30, 2 }, {0,  2 }}, DIF_NONE, msg(Color.IsFgIndex()? Color.IsFgDefault()? lng::MSetColorForeDefault : lng::MSetColorForeIndex : lng::MSetColorForeAARRGGBB) },
		{ DI_FIXEDIT,     {{30, 3 }, {37, 3 }}, DIF_MASKEDIT, },
		{ DI_BUTTON,      {{30, 4 }, {37, 4 }}, DIF_NONE, msg(lng::MSetColorFore256), },
		{ DI_BUTTON,      {{30, 5 }, {37, 5 }}, DIF_NONE, msg(lng::MSetColorForeRGB), },

		{ DI_TEXT,        {{Bg4X, Bg4Y}, {0, Bg4Y}}, DIF_NONE, msg(lng::MSetColorBackground), },
		{ DI_CHECKBOX,    {{Bg4X, Bg4Y}, {0, Bg4Y}}, DIF_NONE, msg(lng::MSetColorBackground), },

		COLOR_PLANE(COLOR_COLUMN, Bg4X, Bg4Y + 1),
		COLOR_CELL(Bg4X, Bg4Y + 3),
		{ DI_TEXT, { { Bg4X + 4, Bg4Y + 3 }, { 0, Bg4Y + 3 } }, DIF_NONE, msg(lng::MSetColorBackgroundDefault) },

		{ DI_TEXT,        {{30, 7 }, {0,  7 }}, DIF_NONE, msg(Color.IsBgIndex()? Color.IsBgDefault()? lng::MSetColorBackDefault : lng::MSetColorBackIndex : lng::MSetColorBackAARRGGBB) },
		{ DI_FIXEDIT,     {{30, 8 }, {37, 8 }}, DIF_MASKEDIT, },
		{ DI_BUTTON,      {{30, 9 }, {37, 9 }}, DIF_NONE, msg(lng::MSetColorBack256), },
		{ DI_BUTTON,      {{30, 10}, {37, 10}}, DIF_NONE, msg(lng::MSetColorBackRGB), },

		{ DI_TEXT,        {{StyleX, StyleY+0 }, {0, StyleY+0 }}, DIF_NONE, msg(lng::MSetColorStyle), },
		{ DI_CHECKBOX,    {{StyleX, StyleY+1 }, {0, StyleY+1 }}, DIF_NONE, msg(lng::MSetColorStyleInherit), },
		{ DI_CHECKBOX,    {{StyleX, StyleY+3 }, {0, StyleY+3 }}, DIF_NONE, msg(lng::MSetColorStyleBold), },
		{ DI_CHECKBOX,    {{StyleX, StyleY+4 }, {0, StyleY+4 }}, DIF_NONE, msg(lng::MSetColorStyleItalic), },
		{ DI_CHECKBOX,    {{StyleX, StyleY+5 }, {0, StyleY+5 }}, DIF_NONE, msg(lng::MSetColorStyleOverline), },
		{ DI_CHECKBOX,    {{StyleX, StyleY+6 }, {0, StyleY+6 }}, DIF_NONE, msg(lng::MSetColorStyleStrikeout), },
		{ DI_CHECKBOX,    {{StyleX, StyleY+7 }, {0, StyleY+7 }}, DIF_NONE, msg(lng::MSetColorStyleFaint), },
		{ DI_CHECKBOX,    {{StyleX, StyleY+8 }, {0, StyleY+8 }}, DIF_NONE, msg(lng::MSetColorStyleBlink), },
		{ DI_CHECKBOX,    {{StyleX, StyleY+9 }, {0, StyleY+9 }}, DIF_NONE, msg(lng::MSetColorStyleInverse), },
		{ DI_CHECKBOX,    {{StyleX, StyleY+10}, {0, StyleY+10}}, DIF_NONE, msg(lng::MSetColorStyleInvisible), },
		{ DI_TEXT,        {{StyleX, StyleY+11}, {0, StyleY+11}}, DIF_NONE, msg(lng::MSetColorStyleUnderline), },
		{ DI_COMBOBOX,    {{StyleX, StyleY+12}, {51,StyleY+12}}, DIF_DROPDOWNLIST, },
		{ DI_BUTTON,      {{54,     StyleY+12}, {0,StyleY+12}}, DIF_BTNNOCLOSE, L"..."sv, },

		{ DI_TEXT,        {{SampleX,  SampleY+0}, {SampleX+SampleW, SampleY+0}}, DIF_NONE, msg(lng::MSetColorSample), },
		{ DI_TEXT,        {{SampleX,  SampleY+1}, {SampleX+SampleW, SampleY+1}}, DIF_NONE, msg(lng::MSetColorSample), },
		{ DI_TEXT,        {{SampleX,  SampleY+2}, {SampleX+SampleW, SampleY+2}}, DIF_NONE, msg(lng::MSetColorSample), },

		{ DI_TEXT,        {{HintX,  HintY+0}, {HintX+HintW, HintY+0}}, ShowVtHint? DIF_NONE : DIF_HIDDEN, msg(lng::MSetColorVTHintInfo), },
		{ DI_TEXT,        {{HintX,  HintY+1}, {HintX+HintW, HintY+1}}, ShowVtHint && IsVtSupported? DIF_NONE : DIF_HIDDEN, msg(lng::MSetColorVTHintSupported), },
		{ DI_TEXT,        {{HintX,  HintY+1}, {HintX+HintW, HintY+1}}, ShowVtHint && !IsVtSupported? DIF_NONE : DIF_HIDDEN, msg(lng::MSetColorVTHintNotSupported), },

		{ DI_BUTTON,      {{0, ButtonY}, {0, ButtonY}}, DIF_CENTERGROUP | DIF_DEFAULTBUTTON, msg(lng::MSetColorSet), },
		{ DI_BUTTON,      {{0, ButtonY}, {0, ButtonY}}, DIF_CENTERGROUP | (Reset? DIF_NONE : DIF_DISABLE | DIF_HIDDEN), msg(lng::MReset), },
		{ DI_BUTTON,      {{0, ButtonY}, {0, ButtonY}}, DIF_CENTERGROUP | (ShowVtHint && IsVtSupported? DIF_NONE : DIF_HIDDEN), msg(lng::MSetColorEnableVT), },
		{ DI_BUTTON,      {{0, ButtonY}, {0, ButtonY}}, DIF_CENTERGROUP, msg(lng::MSetColorCancel), },
	});

	ColorDlg[fg_item(cb::color_first_radio)].Flags |= DIF_GROUP;
	ColorDlg[bg_item(cb::color_first_radio)].Flags |= DIF_GROUP;

	ColorDlg[cd::separator_vertical_before_style].strMask = { BoxSymbols[BS_T_H2V1], BoxSymbols[BS_V1], BoxSymbols[BS_B_H1V1] };
	ColorDlg[cd::separator_after_foreground].strMask = ColorDlg[cd::separator_after_background].strMask = { BoxSymbols[BS_L_H1V2], BoxSymbols[BS_H1], BoxSymbols[BS_R_H1V1] };
	ColorDlg[cd::separator_style].strMask = { BoxSymbols[BS_H1], BoxSymbols[BS_H1], BoxSymbols[BS_H1] };

	ColorDlg[fg_item(cb::colorcode_edit)].strData = color_code(colors::single_color::foreground(Color));
	ColorDlg[bg_item(cb::colorcode_edit)].strData = color_code(colors::single_color::background(Color));
	ColorDlg[fg_item(cb::colorcode_edit)].strMask = Color.IsFgIndex()? Color.IsFgDefault()? MaskDef : MaskIndex : MaskARGB;
	ColorDlg[bg_item(cb::colorcode_edit)].strMask = Color.IsBgIndex()? Color.IsBgDefault()? MaskDef : MaskIndex : MaskARGB;

	FarListItem UnderlineTypes[]
	{
		{ 0, msg(lng::MSetColorStyleUnderlineNone).c_str() },
		{ 0, msg(lng::MSetColorStyleUnderlineSingle).c_str() },
		{ 0, msg(lng::MSetColorStyleUnderlineDouble).c_str() },
		{ 0, msg(lng::MSetColorStyleUnderlineCurly).c_str() },
		{ 0, msg(lng::MSetColorStyleUnderlineDotted).c_str() },
		{ 0, msg(lng::MSetColorStyleUnderlineDashed).c_str() },
	};

	UnderlineTypes[Color.GetUnderline()].Flags = LIF_SELECTED;

	FarList ComboList
	{
		sizeof(ComboList),
		std::size(UnderlineTypes),
		UnderlineTypes
	};

	ColorDlg[cd::style_underline_combo].ListItems = &ComboList;

	color_state ColorState
	{
		.CurColor = Color,
		.BaseColor = BaseColor? *BaseColor : colors::NtColorToFarColor(F_BLACK | B_BLACK),
		.TransparencyEnabled = BaseColor != nullptr,
		.Fg
		{
			.CurColor = colors::single_color::foreground(Color),
			.TransparencyEnabled = !!BaseColor,
			.RefreshColor = [&]{ ColorState.refresh_fg(); },
			.Offset = cd::fg_first,
		},
		.Bg
		{
			.CurColor = colors::single_color::background(Color),
			.TransparencyEnabled = !!BaseColor,
			.RefreshColor = [&]{ ColorState.refresh_bg(); },
			.Offset = cd::bg_first,
		},
	};

	auto
		ForegroundColorControlActivated = false,
		BackgroundColorControlActivated = false;

	if (Color.IsFgIndex())
		ForegroundColorControlActivated = activate_control(Color.ForegroundColor, ColorDlg, cd::fg_first);

	if (Color.IsBgIndex())
		BackgroundColorControlActivated = activate_control(Color.BackgroundColor, ColorDlg, cd::bg_first);

	if (!ForegroundColorControlActivated && !BackgroundColorControlActivated)
		ColorDlg[fg_item(cb::colorcode_edit)].Flags |= DIF_FOCUS;

	for (const auto& [Index, Flag]: StyleMapping)
	{
		ColorDlg[Index].Selected = Color.Flags & Flag? BSTATE_CHECKED : BSTATE_UNCHECKED;
	}

	if (BaseColor)
	{
		ColorDlg[fg_item(cb::color_text)].Flags |= DIF_HIDDEN | DIF_DISABLE;
		ColorDlg[bg_item(cb::color_text)].Flags |= DIF_HIDDEN | DIF_DISABLE;

		disable_if_needed(Color.ForegroundColor, ColorDlg, cd::fg_first);
		disable_if_needed(Color.BackgroundColor, ColorDlg, cd::bg_first);
	}
	else
	{
		ColorDlg[fg_item(cb::color_active_checkbox)].Flags |= DIF_HIDDEN | DIF_DISABLE;
		ColorDlg[bg_item(cb::color_active_checkbox)].Flags |= DIF_HIDDEN | DIF_DISABLE;
	}

	const auto Dlg = Dialog::create(ColorDlg, std::bind_front(&color_state::GetColorDlgProc, &ColorState));

	const auto
		DlgWidth = static_cast<int>(ColorDlg[cd::border].X2) + 4,
		DlgHeight = static_cast<int>(ColorDlg[cd::border].Y2) + 2;

	if (bCentered)
		Dlg->SetPosition({ -1, -1, DlgWidth, DlgHeight });
	else
	{
		constexpr auto
			DlgLeft = 14,
			DlgTop = 2;

		Dlg->SetPosition({DlgLeft, DlgTop, DlgLeft + DlgWidth - 1, DlgTop + DlgHeight - 1 });
	}

	Dlg->SetHelp(L"ColorPicker"sv);
	Dlg->Process();

	switch (Dlg->GetExitCode())
	{
	case cd::button_ok:
		Color = ColorState.CurColor;
		return true;

	case cd::button_reset:
		if (Reset)
			*Reset = true;
		[[fallthrough]];

	default:
		return false;

	}
}

#undef COLOR_PLANE
#undef COLOR_COLUMN
