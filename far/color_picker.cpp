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

static string color_code(COLORREF const Color, bool const IsIndex, bool const IsDefault)
{
	return IsIndex? IsDefault?
		far::format(L"{:02X}      "sv, colors::alpha_value(Color)) :
		far::format(L"{:02X}    {:02X}"sv, colors::alpha_value(Color), colors::index_value(Color)) :
		far::format(L"{:08X}"sv, colors::ARGB2ABGR(Color));
}

static std::optional<COLORREF> parse_color(string_view const Str, bool const IsIndex)
{
	if (IsIndex)
	{
		unsigned Alpha;
		if (!from_string(Str.substr(0, 2), Alpha, {}, 16))
			return {};

		unsigned Index;
		if (!from_string(Str.substr(6, 2), Index, {}, 16))
			return {};

		COLORREF Result{};
		colors::set_alpha_value(Result, Alpha);
		colors::set_index_value(Result, Index);

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

struct color_state
{
	FarColor CurColor, BaseColor;
	bool TransparencyEnabled;
};

constexpr auto
	MaskIndex = L"HH    HH"sv,
	MaskARGB  = L"HHHHHHHH"sv,
	MaskDef   = L"HH      "sv;

enum color_dialog_items
{
	cd_border,

	cd_separator_before_hint,
	cd_separator_before_buttons,
	cd_separator_vertical_before_style,
	cd_separator_after_foreground,
	cd_separator_after_background,
	cd_separator_style,

	cd_fg_text,
	cd_fg_active,
	cd_fg_color_first,
	cd_fg_color_last = cd_fg_color_first + colors::index::nt_last + 1,
	cd_fg_color_default = cd_fg_color_last,
	cd_fg_color_default_label,

	cd_fg_colorcode_title,
	cd_fg_colorcode,
	cd_fg_256,
	cd_fg_rgb,

	cd_bg_text,
	cd_bg_active,
	cd_bg_color_first,
	cd_bg_color_last = cd_bg_color_first + colors::index::nt_last + 1,
	cd_bg_color_default = cd_bg_color_last,
	cd_bg_color_default_label,

	cd_bg_colorcode_title,
	cd_bg_colorcode,
	cd_bg_256,
	cd_bg_rgb,

	cd_style,
	cd_style_first,
	cd_style_inherit = cd_style_first,
	cd_style_bold,
	cd_style_italic,
	cd_style_underline,
	cd_style_underline2,
	cd_style_overline,
	cd_style_strikeout,
	cd_style_faint,
	cd_style_blink,
	cd_style_inverse,
	cd_style_invisible,
	cd_style_last = cd_style_invisible,

	cd_sample_first,
	cd_sample_last = cd_sample_first + 2,

	cd_vt_hint_text_intro,
	cd_vt_hint_text_supported,
	cd_vt_hint_text_not_supported,

	cd_button_ok,
	cd_button_reset,
	cd_button_enable_vt,
	cd_button_cancel,

	cd_count
};

static const std::pair<color_dialog_items, FARCOLORFLAGS> StyleMapping[]
{
	{ cd_style_inherit,     FCF_INHERIT_STYLE  },
	{ cd_style_bold,        FCF_FG_BOLD        },
	{ cd_style_italic,      FCF_FG_ITALIC      },
	{ cd_style_underline,   FCF_FG_UNDERLINE   },
	{ cd_style_underline2,  FCF_FG_UNDERLINE2  },
	{ cd_style_overline,    FCF_FG_OVERLINE    },
	{ cd_style_strikeout,   FCF_FG_STRIKEOUT   },
	{ cd_style_faint,       FCF_FG_FAINT       },
	{ cd_style_blink,       FCF_FG_BLINK       },
	{ cd_style_inverse,     FCF_FG_INVERSE     },
	{ cd_style_invisible,   FCF_FG_INVISIBLE   },
};

static intptr_t GetColorDlgProc(Dialog* Dlg, intptr_t Msg, intptr_t Param1, void* Param2)
{
	auto& ColorState = edit_as<color_state>(Dlg->SendMessage(DM_GETDLGDATA, 0, nullptr));
	auto& CurColor = ColorState.CurColor;

	const auto GetColor = [Param1](size_t const Offset)
	{
		if (any_of(Param1, cd_fg_color_default, cd_bg_color_default))
		{
			auto Color = colors::resolve_defaults(colors::default_color());
			if (Offset == cd_fg_color_first)
			{
				using std::swap;
				swap(Color.ForegroundColor, Color.BackgroundColor);
			}

			return Color;
		}

		auto Color = colors::NtColorToFarColor(IndexColors[color_by_control[Param1 - Offset]]);
		flags::clear(Color.Flags, FCF_INHERIT_STYLE);
		return Color;
	};

	const auto FlagIndex = [](bool const IsFg)
	{
		return IsFg? FCF_FG_INDEX : FCF_BG_INDEX;
	};

	const auto SetComponentColorValue = [&CurColor](bool const IsFg, COLORREF const Value)
	{
		auto& Component = IsFg? CurColor.ForegroundColor : CurColor.BackgroundColor;
		colors::set_color_value(Component, Value);
	};

	const auto DM_UPDATECOLORCODE = DM_USER + 1;

	switch (Msg)
	{
	case DN_INITDIALOG:
		Dlg->SendMessage(DM_EDITUNCHANGEDFLAG, cd_fg_colorcode, {});
		Dlg->SendMessage(DM_EDITUNCHANGEDFLAG, cd_bg_colorcode, {});
		break;

	case DN_CTLCOLORDLGITEM:
		{
			const auto preview_or_disabled = [&](COLORREF const Color, size_t const Index)
			{
				return ColorState.TransparencyEnabled && colors::is_transparent(Color)?
					colors::PaletteColorToFarColor(COL_DIALOGDISABLED) :
					GetColor(Index);
			};

			const auto& Colors = *static_cast<FarDialogItemColors const*>(Param2);

			if (Param1 >= cd_fg_color_first && Param1 <= cd_fg_color_last)
			{
				Colors.Colors[0] = preview_or_disabled(CurColor.ForegroundColor, cd_fg_color_first);
				return TRUE;
			}

			if (Param1 >= cd_bg_color_first && Param1 <= cd_bg_color_last)
			{
				Colors.Colors[0] = preview_or_disabled(CurColor.BackgroundColor, cd_bg_color_first);
				return TRUE;
			}

			if (Param1 >= cd_sample_first && Param1 <= cd_sample_last)
			{
				Colors.Colors[0] = colors::merge(ColorState.BaseColor, ColorState.CurColor);
				return TRUE;
			}

		}
		break;

	case DN_BTNCLICK:
		{
			if (Param2 && (in_closed_range(cd_fg_color_first, Param1, cd_fg_color_last) || in_closed_range(cd_bg_color_first, Param1, cd_bg_color_last)))
			{
				if (IgnoreColorIndexClick)
					return TRUE;

				const auto IsFg = in_closed_range(cd_fg_color_first, Param1, cd_fg_color_last);
				const auto First = IsFg? cd_fg_color_first : cd_bg_color_first;

				auto& Component = IsFg? CurColor.ForegroundColor : CurColor.BackgroundColor;
				Component = GetColor(First).BackgroundColor;

				CurColor.Flags |= FlagIndex(IsFg);

				if (Param1 == cd_fg_color_default)
					CurColor.SetFgDefault();
				else if (Param1 == cd_bg_color_default)
					CurColor.SetBgDefault();

				Dlg->SendMessage(DM_UPDATECOLORCODE, IsFg? cd_fg_colorcode : cd_bg_colorcode, {});

				return TRUE;
			}

			if (in_closed_range(cd_style_first, Param1, cd_style_last))
			{
				flags::change(CurColor.Flags, StyleMapping[Param1 - cd_style_first].second, Param2 != nullptr);
				return TRUE;
			}

			if (any_of(Param1, cd_fg_active, cd_bg_active))
			{
				const auto IsFg = Param1 == cd_fg_active;
				auto& Component = IsFg? CurColor.ForegroundColor : CurColor.BackgroundColor;
				Param2? colors::make_opaque(Component) : colors::make_transparent(Component);

				SCOPED_ACTION(Dialog::suppress_redraw)(Dlg);
				const auto Offset = IsFg? cd_fg_color_first : cd_bg_color_first;
				for (const auto& i: irange(colors::index::nt_size))
				{
					Dlg->SendMessage(DM_ENABLE, i + Offset, Param2);
				}

				Dlg->SendMessage(DM_UPDATECOLORCODE, IsFg? cd_fg_colorcode : cd_bg_colorcode, {});

				Dlg->SendMessage(DM_ENABLE, IsFg? cd_fg_colorcode : cd_bg_colorcode, Param2);
				Dlg->SendMessage(DM_ENABLE, IsFg? cd_fg_256 : cd_bg_256, Param2);
				Dlg->SendMessage(DM_ENABLE, IsFg? cd_fg_rgb : cd_bg_rgb, Param2);

				return TRUE;
			}

			if (any_of(Param1, cd_fg_256, cd_bg_256))
			{
				const auto IsFg = Param1 == cd_fg_256;
				const auto Index = colors::FarColorToConsole256Color(CurColor);
				const auto& Component = IsFg? Index.ForegroundIndex : Index.BackgroundIndex;

				auto Color = colors::index_value(Component);

				if (pick_color_256(Color))
				{
					SetComponentColorValue(IsFg, Color);

					CurColor.Flags |= FlagIndex(IsFg);

					Dlg->SendMessage(DM_SETCHECK, IsFg? cd_fg_color_first : cd_bg_color_first, ToPtr(BSTATE_3STATE));
					Dlg->SendMessage(DM_UPDATECOLORCODE, IsFg? cd_fg_colorcode : cd_bg_colorcode, {});

				}
				return TRUE;
			}

			if (any_of(Param1, cd_fg_rgb, cd_bg_rgb))
			{
				const auto IsFg = Param1 == cd_fg_rgb;
				const auto ResolvedColor = colors::resolve_defaults(CurColor);
				const auto& Component = IsFg? ResolvedColor.ForegroundColor : ResolvedColor.BackgroundColor;

				auto Color = colors::color_value(
					CurColor.Flags & FlagIndex(IsFg)?
						colors::ConsoleIndexToTrueColor(Component) :
						Component
				);

				if (auto CustomColors = Global->Opt->Palette.GetCustomColors(); pick_color_rgb(Color, CustomColors))
				{
					SetComponentColorValue(IsFg, Color);
					CurColor.Flags &= ~FlagIndex(IsFg);

					Dlg->SendMessage(DM_SETCHECK, IsFg? cd_fg_color_first : cd_bg_color_first, ToPtr(BSTATE_3STATE));
					Dlg->SendMessage(DM_UPDATECOLORCODE, IsFg? cd_fg_colorcode : cd_bg_colorcode, {});
					Global->Opt->Palette.SetCustomColors(CustomColors);
				}

				return TRUE;
			}

			if (Param1 == cd_button_enable_vt)
			{
				Global->Opt->VirtualTerminalRendering = true;
				SetFarConsoleMode();

				Dlg->SendMessage(DM_ENABLE, cd_button_enable_vt, ToPtr(FALSE));

				return TRUE;
			}
		}
		break;

	case DN_EDITCHANGE:
		if (!IgnoreEditChange && any_of(Param1, cd_fg_colorcode, cd_bg_colorcode))
		{
			const auto& Item = *static_cast<const FarDialogItem*>(Param2);
			const auto Iterator = null_iterator(Item.Data);
			if (std::any_of(Iterator, Iterator.end(), std::iswxdigit))
			{
				const auto IsFg = Param1 == cd_fg_colorcode;
				const auto IsIndex = IsFg? CurColor.IsFgIndex() : CurColor.IsBgIndex();
				auto& Component = IsFg? CurColor.ForegroundColor : CurColor.BackgroundColor;

				const auto ParsedColor = parse_color(Item.Data, IsIndex);
				if (!ParsedColor)
					return false;

				const auto OldColorValue =  colors::color_value(Component);
				Component = *ParsedColor;

				if (IsIndex)
				{
					if (const auto NewColorValue = colors::color_value(Component); NewColorValue != OldColorValue)
					{
						IgnoreColorIndexClick = true;
						SCOPE_EXIT{ IgnoreColorIndexClick = false; };
						const auto ColorButtonIndex = NewColorValue < 8? NewColorValue * 2 : (NewColorValue - 8) * 2 + 1;
						Dlg->SendMessage(DM_SETCHECK, (IsFg? cd_fg_color_first : cd_bg_color_first) + ColorButtonIndex, ToPtr(BSTATE_CHECKED));
					}
				}

				if (ColorState.TransparencyEnabled && colors::is_transparent(Component))
				{
					Dlg->SendMessage(DM_SETCHECK, IsFg? cd_fg_active : cd_bg_active, ToPtr(BSTATE_UNCHECKED));
					Dlg->SendMessage(DN_BTNCLICK, IsFg? cd_fg_active : cd_bg_active, ToPtr(BSTATE_UNCHECKED));
				}
			}
		}
		break;

	case DM_UPDATECOLORCODE:
		{
			IgnoreEditChange = true;
			SCOPE_EXIT{ IgnoreEditChange = false; };
			const auto IsFg = Param1 == cd_fg_colorcode;

			FarDialogItem Item;
			if (!Dlg->SendMessage(DM_GETDLGITEMSHORT, Param1, &Item))
				return false;

			const auto Color = IsFg? CurColor.ForegroundColor : CurColor.BackgroundColor;
			const auto IsIndex = IsFg? CurColor.IsFgIndex() : CurColor.IsBgIndex();
			const auto IsDefault = IsFg? CurColor.IsFgDefault() : CurColor.IsBgDefault();
			const auto Value = color_code(Color, IsIndex, IsDefault);

			Item.Mask = (IsIndex? IsDefault? MaskDef : MaskIndex: MaskARGB).data();
			Item.Data = UNSAFE_CSTR(Value);

			Dlg->SendMessage(DM_SETDLGITEM, Param1, &Item);

			constexpr lng Titles[][3]
			{
				{ lng::MSetColorForeIndex, lng::MSetColorForeAARRGGBB, lng::MSetColorForeDefault },
				{ lng::MSetColorBackIndex, lng::MSetColorBackAARRGGBB, lng::MSetColorBackDefault },
			};

			Dlg->SendMessage(DM_SETTEXTPTR,
				IsFg? cd_fg_colorcode_title : cd_bg_colorcode_title,
				UNSAFE_CSTR(msg(Titles[IsFg? 0 : 1][IsIndex? IsDefault? 2 : 0 : 1]))
			);
		}
		return TRUE;

	default:
		break;
	}

	return Dlg->DefProc(Msg, Param1, Param2);
}

bool GetColorDialog(FarColor& Color, bool const bCentered, const FarColor* const BaseColor, bool* const Reset)
{
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

	auto ColorDlg = MakeDialogItems<cd_count>(
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

#undef COLOR_PLANE
#undef COLOR_COLUMN

		{ DI_TEXT,        {{30, 7 }, {0,  7 }}, DIF_NONE, msg(Color.IsBgIndex()? Color.IsBgDefault()? lng::MSetColorBackDefault : lng::MSetColorBackIndex : lng::MSetColorBackAARRGGBB) },
		{ DI_FIXEDIT,     {{30, 8 }, {37, 8 }}, DIF_MASKEDIT, },
		{ DI_BUTTON,      {{30, 9 }, {37, 9 }}, DIF_NONE, msg(lng::MSetColorBack256), },
		{ DI_BUTTON,      {{30, 10}, {37, 10}}, DIF_NONE, msg(lng::MSetColorBackRGB), },

		{ DI_TEXT,        {{StyleX, StyleY+0 }, {0, StyleY+0 }}, DIF_NONE, msg(lng::MSetColorStyle), },
		{ DI_CHECKBOX,    {{StyleX, StyleY+1 }, {0, StyleY+1 }}, DIF_NONE, msg(lng::MSetColorStyleInherit), },
		{ DI_CHECKBOX,    {{StyleX, StyleY+3 }, {0, StyleY+3 }}, DIF_NONE, msg(lng::MSetColorStyleBold), },
		{ DI_CHECKBOX,    {{StyleX, StyleY+4 }, {0, StyleY+4 }}, DIF_NONE, msg(lng::MSetColorStyleItalic), },
		{ DI_CHECKBOX,    {{StyleX, StyleY+5 }, {0, StyleY+5 }}, DIF_NONE, msg(lng::MSetColorStyleUnderline), },
		{ DI_CHECKBOX,    {{StyleX, StyleY+6 }, {0, StyleY+6 }}, DIF_NONE, msg(lng::MSetColorStyleUnderline2), },
		{ DI_CHECKBOX,    {{StyleX, StyleY+7 }, {0, StyleY+7 }}, DIF_NONE, msg(lng::MSetColorStyleOverline), },
		{ DI_CHECKBOX,    {{StyleX, StyleY+8 }, {0, StyleY+8 }}, DIF_NONE, msg(lng::MSetColorStyleStrikeout), },
		{ DI_CHECKBOX,    {{StyleX, StyleY+9 }, {0, StyleY+9 }}, DIF_NONE, msg(lng::MSetColorStyleFaint), },
		{ DI_CHECKBOX,    {{StyleX, StyleY+10}, {0, StyleY+10}}, DIF_NONE, msg(lng::MSetColorStyleBlink), },
		{ DI_CHECKBOX,    {{StyleX, StyleY+11}, {0, StyleY+11}}, DIF_NONE, msg(lng::MSetColorStyleInverse), },
		{ DI_CHECKBOX,    {{StyleX, StyleY+12}, {0, StyleY+12}}, DIF_NONE, msg(lng::MSetColorStyleInvisible), },

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

	ColorDlg[cd_fg_color_first].Flags |= DIF_GROUP;
	ColorDlg[cd_bg_color_first].Flags |= DIF_GROUP;

	ColorDlg[cd_separator_vertical_before_style].strMask = { BoxSymbols[BS_T_H2V1], BoxSymbols[BS_V1], BoxSymbols[BS_B_H1V1] };
	ColorDlg[cd_separator_after_foreground].strMask = ColorDlg[cd_separator_after_background].strMask = { BoxSymbols[BS_L_H1V2], BoxSymbols[BS_H1], BoxSymbols[BS_R_H1V1] };
	ColorDlg[cd_separator_style].strMask = { BoxSymbols[BS_H1], BoxSymbols[BS_H1], BoxSymbols[BS_H1] };

	ColorDlg[cd_fg_colorcode].strData = color_code(Color.ForegroundColor, Color.IsFgIndex(), Color.IsFgDefault());
	ColorDlg[cd_bg_colorcode].strData = color_code(Color.BackgroundColor, Color.IsBgIndex(), Color.IsBgDefault());
	ColorDlg[cd_fg_colorcode].strMask = Color.IsFgIndex()? Color.IsFgDefault()? MaskDef : MaskIndex : MaskARGB;
	ColorDlg[cd_bg_colorcode].strMask = Color.IsBgIndex()? Color.IsBgDefault()? MaskDef : MaskIndex : MaskARGB;

	color_state ColorState
	{
		Color,
		BaseColor? *BaseColor : colors::NtColorToFarColor(F_BLACK | B_BLACK),
		BaseColor != nullptr
	};

	auto
		ForegroundColorControlActivated = false,
		BackgroundColorControlActivated = false;

	const auto get_control_id = [&](COLORREF const ColorPart, int const ControlGroup) -> std::optional<size_t>
	{
		if (colors::is_default(ColorPart))
			return ControlGroup + colors::index::nt_size;

		const auto Index = colors::index_value(ColorPart);
		if (Index > colors::index::nt_last)
			return {};

		return ControlGroup + control_by_color[Index];
	};

	const auto activate_control = [&](COLORREF const ColorPart, int const ControlGroup)
	{
		const auto ControlId = get_control_id(ColorPart, ControlGroup);
		if (!ControlId)
			return false;

		ColorDlg[*ControlId].Selected = true;
		ColorDlg[*ControlId].Flags |= DIF_FOCUS;
		return true;
	};

	if (Color.IsFgIndex())
		ForegroundColorControlActivated = activate_control(Color.ForegroundColor, cd_fg_color_first);

	if (Color.IsBgIndex())
		BackgroundColorControlActivated = activate_control(Color.BackgroundColor, cd_bg_color_first);

	if (!ForegroundColorControlActivated && !BackgroundColorControlActivated)
		ColorDlg[cd_fg_colorcode].Flags |= DIF_FOCUS;

	for (const auto& [Index, Flag]: StyleMapping)
	{
		ColorDlg[Index].Selected = Color.Flags & Flag? BSTATE_CHECKED : BSTATE_UNCHECKED;
	}

	if (BaseColor)
	{
		ColorDlg[cd_fg_text].Flags |= DIF_HIDDEN | DIF_DISABLE;
		ColorDlg[cd_bg_text].Flags |= DIF_HIDDEN | DIF_DISABLE;

		if (colors::is_transparent(Color.ForegroundColor))
		{
			for (const auto& i: irange(cd_fg_color_first, cd_fg_color_last + 1))
			{
				ColorDlg[i].Flags |= DIF_DISABLE;
			}

			ColorDlg[cd_fg_color_default_label].Flags |= DIF_DISABLE;
			ColorDlg[cd_fg_colorcode].Flags |= DIF_DISABLE;
			ColorDlg[cd_fg_256].Flags |= DIF_DISABLE;
			ColorDlg[cd_fg_rgb].Flags |= DIF_DISABLE;
		}
		else
		{
			ColorDlg[cd_fg_active].Selected = BSTATE_CHECKED;
		}

		if (colors::is_transparent(Color.BackgroundColor))
		{
			for (const auto& i: irange(cd_bg_color_first, cd_bg_color_last + 1))
			{
				ColorDlg[i].Flags |= DIF_DISABLE;
			}

			ColorDlg[cd_bg_color_default_label].Flags |= DIF_DISABLE;
			ColorDlg[cd_bg_colorcode].Flags |= DIF_DISABLE;
			ColorDlg[cd_bg_256].Flags |= DIF_DISABLE;
			ColorDlg[cd_bg_rgb].Flags |= DIF_DISABLE;
		}
		else
		{
			ColorDlg[cd_bg_active].Selected = BSTATE_CHECKED;
		}

	}
	else
	{
		ColorDlg[cd_fg_active].Flags |= DIF_HIDDEN | DIF_DISABLE;
		ColorDlg[cd_bg_active].Flags |= DIF_HIDDEN | DIF_DISABLE;
	}

	const auto Dlg = Dialog::create(ColorDlg, GetColorDlgProc, &ColorState);

	const auto
		DlgWidth = static_cast<int>(ColorDlg[cd_border].X2) + 4,
		DlgHeight = static_cast<int>(ColorDlg[cd_border].Y2) + 2;

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
	case cd_button_ok:
		Color = ColorState.CurColor;
		return true;

	case cd_button_reset:
		if (Reset)
			*Reset = true;
		[[fallthrough]];

	default:
		return false;

	}
}
