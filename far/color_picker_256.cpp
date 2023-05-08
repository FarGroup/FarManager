/*
color_picker_256.cpp

256 colors extension to the standard color picker
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
#include "color_picker_256.hpp"

// Internal:
#include "color_picker_common.hpp"
#include "colormix.hpp"
#include "lang.hpp"

// Platform:
#include "platform.hpp"

// Common:
#include "common/2d/algorithm.hpp"

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

using namespace color_picker_common;
using colors::index_color_256;
using rgb = colors::rgb6;

constexpr auto cube_size = colors::index::cube_size;

static constexpr auto cube_rc_mapping = column_major_iota<uint8_t, cube_size, cube_size>();
static_assert(std::size(cube_rc_mapping) == cube_size * cube_size);

using shape = shapes<uint8_t, cube_size>;
using cell  = shape::cell;
using row   = shape::row;
using plane = shape::plane;
using cube  = shape::cube;

static FarColor Console256ColorToFarColor(index_color_256 const Color)
{
	return
	{
		FCF_FG_INDEX | FCF_BG_INDEX,
		{ colors::opaque(colors::index_bits(Color.ForegroundIndex)) },
		{ colors::opaque(colors::index_bits(Color.BackgroundIndex)) }
	};
}

static constexpr index_color_256 distinct_cube_index(uint8_t const Index)
{
	rgb RGB = Index;
	// Naive inversion doesn't work nicely in the middle of the spectre.
	// This way the distance between foreground and background colors is constant and should always produce readable results.
	constexpr uint8_t Mapping[]{ 3, 4, 5, 0, 1, 2 };
	RGB.r = Mapping[RGB.r];
	RGB.g = Mapping[RGB.g];
	RGB.b = Mapping[RGB.b];

	return { RGB, Index };
}

static constexpr index_color_256 distinct_grey(uint8_t const Value)
{
	// Naive inversion doesn't work nicely in the middle of the spectre.
	// This way the distance between foreground and background colors is constant and should always produce readable results.
	return
	{
		static_cast<uint8_t>(colors::index::grey_first + Value + (Value < 12? +12 : -12)),
		static_cast<uint8_t>(colors::index::grey_first + Value)
	};
}

static constexpr auto distinct_grey_index = []
{
	std::array<index_color_256, colors::index::grey_last - colors::index::grey_first + 1> Result;

	for (uint8_t i = 0; i != Result.size(); ++i)
		Result[i] = distinct_grey(i);

	return Result;
}();
static_assert(std::size(distinct_grey_index) == colors::index::grey_last - colors::index::grey_first + 1);

static constexpr auto grey_control_by_color = column_major_iota<uint8_t, 6, 4>();
static_assert(std::size(grey_control_by_color) == std::size(distinct_grey_index));

static constexpr auto grey_color_by_control = column_major_iota<uint8_t, 4, 6>();
static_assert(std::size(grey_color_by_control) == std::size(distinct_grey_index));

static constexpr uint8_t grey_stripe_mapping[]
{
	 0,  1,  2,  3,  4,  5,
	11, 10,  9,  8,  7,  6,
	12, 13, 14, 15, 16, 17,
	23, 22, 21, 20, 19, 18
};

static_assert(std::size(grey_stripe_mapping) == std::size(distinct_grey_index));

static bool is_rgb(uint8_t const Color)
{
	return in_closed_range(colors::index::cube_first, static_cast<int>(Color), colors::index::cube_last);
}

enum color_256_dialog_items
{
	cd_border,

	cd_cube_first,
	cd_cube_last = cd_cube_first + (cube_size * cube_size - 1),

	cd_grey_first,
	cd_grey_last = cd_grey_first + (colors::index::grey_last - colors::index::grey_first),

	cd_button_up,
	cd_button_left,
	cd_button_right,
	cd_button_down,

	cd_button_home,

	cd_button_plus,
	cd_button_minus,

	cd_text_rgb,
	cd_text_r,
	cd_button_r_plus,
	cd_button_r_minus,
	cd_text_g,
	cd_button_g_plus,
	cd_button_g_minus,
	cd_text_b,
	cd_button_b_plus,
	cd_button_b_minus,

	cd_separator,

	cd_button_ok,
	cd_button_cancel,

	cd_count
};

using rgb_context = rgb_context_t<rgb, color_256_dialog_items>;

struct color_256_state
{
	using items = color_256_dialog_items;
	using rgb_context = rgb_context;
	static constexpr size_t Depth = 6;

	uint8_t CurColor{};

	cube_data<cube> Cube;

	static auto channel_value(uint8_t const Channel)
	{
		return far::format(L" {} "sv, Channel);
	}

	rgb as_rgb() const
	{
		return is_rgb(CurColor)? CurColor : colors::index::cube_first;
	}

	void from_rgb(rgb const RGB)
	{
		CurColor = RGB;
	}
};

static auto cube_index(color_256_state const& ColorState, color_256_dialog_items const Button)
{
	const auto CubeIndex = cube_rc_mapping[Button - cd_cube_first];
	const auto
		Y = CubeIndex / cube_size,
		X = CubeIndex % cube_size;

	return ColorState.Cube.Cube[ColorState.Cube.Slice][Y][X];
}

static void init_cube(color_256_state& ColorState)
{
	uint8_t Index = colors::index::cube_first;

	for (auto& Plane: ColorState.Cube.Cube)
	{
		for (auto& Line: Plane)
		{
			for (auto& Point: Line)
			{
				Point = Index++;

				if (Point == ColorState.CurColor)
				{
					ColorState.Cube.Slice = &Plane - &ColorState.Cube.Cube[0];
				}
			}
		}
	}
}

static intptr_t GetColorDlgProc(Dialog* Dlg, intptr_t Msg, intptr_t Param1, void* Param2)
{
	auto& ColorState = edit_as<color_256_state>(Dlg->SendMessage(DM_GETDLGDATA, 0, nullptr));

	switch (Msg)
	{
	case DN_CTLCOLORDLGITEM:
		{
			const auto& Colors = *static_cast<FarDialogItemColors const*>(Param2);

			if (in_closed_range(cd_cube_first, Param1, cd_cube_last))
			{
				Colors.Colors[0] = Console256ColorToFarColor(distinct_cube_index(cube_index(ColorState, static_cast<color_256_dialog_items>(Param1))));
				return true;
			}

			if (in_closed_range(cd_grey_first, Param1, cd_grey_last))
			{
				Colors.Colors[0] = Console256ColorToFarColor(distinct_grey_index[grey_stripe_mapping[grey_color_by_control[Param1 - cd_grey_first]]]);
				return true;
			}

			switch (const auto Item = static_cast<color_256_dialog_items>(Param1))
			{
			case cd_text_rgb:
				{
					// Foreground color is irrelevant
					Colors.Colors[0] = Console256ColorToFarColor({ 0, ColorState.CurColor });
					return true;
				}

			case cd_text_r:
			case cd_text_g:
			case cd_text_b:
				{
					const auto Context = get_rgb_context<rgb>(Item);
					auto RGB = ColorState.as_rgb();
					auto& Channel = std::invoke(Context.Channel, RGB);
					const auto SavedValue = Channel;
					RGB = {};
					Channel = SavedValue;
					// Primary colors don't need no safe distinction
					Colors.Colors[0] = Console256ColorToFarColor({ static_cast<uint8_t>(colors::invert(RGB, true)), RGB });
					return true;
				}

			default:
				break;
			}
		}
		break;

	case DN_BTNCLICK:
		{
			const auto Button = static_cast<color_256_dialog_items>(Param1);
			if (on_button_click(Dlg, Button, ColorState))
				return true;

			if (Param2 && in_closed_range(cd_cube_first, Button, cd_cube_last))
			{
				ColorState.CurColor = cube_index(ColorState, Button);
				update_rgb_control<color_256_state>(Dlg, ColorState.as_rgb());
				return true;
			}

			if (Param2 && in_closed_range(cd_grey_first, Button, cd_grey_last))
			{
				ColorState.CurColor = colors::index::grey_first + grey_stripe_mapping[grey_color_by_control[Button - cd_grey_first]];
				update_rgb_control<color_256_state>(Dlg, rgb{});
				return true;
			}

			switch (Button)
			{
			case cd_button_home:
				init_cube(ColorState);
				ColorState.Cube.Slice = 0;
				Dlg->SendMessage(DM_ONCUBECHANGE, 0, {});
				return true;

			default:
				break;
			}
		}
		break;

	case DM_ONCUBECHANGE:
		{
			if (is_rgb(ColorState.CurColor))
				Dlg->SendMessage(DM_SETCHECK, cd_cube_first, ToPtr(BSTATE_3STATE));

			const auto& Plane = ColorState.Cube.Cube[ColorState.Cube.Slice];
			for (const auto& Line: Plane)
			{
				for (const auto& Point: Line)
				{
					if (Point == ColorState.CurColor)
					{
						const auto ControlId = cd_cube_first + cube_rc_mapping[&Point - &Plane[0][0]];
						Dlg->SendMessage(DM_SETCHECK, ControlId, ToPtr(BSTATE_CHECKED));
						break;
					}
				}
			}

			Dlg->SendMessage(DM_REDRAW, 0, {});
		}
		return true;

	default:
		break;
	}

	return Dlg->DefProc(Msg, Param1, Param2);
}

bool pick_color_256(uint8_t& Color)
{
	const auto
		CubeX = 5,
		CubeY = 2,
		PadX = CubeX + cube_size * 3 + 1,
		PadY = CubeY,
		GreyX = CubeX,
		GreyY = CubeY + cube_size + 1,
		GreyH = 4,
		RGBX = PadX,
		RGBY = GreyY,
		ButtonY = GreyY + GreyH + 1;

	auto ColorDlg = MakeDialogItems<cd_count>(
	{
		{ DI_DOUBLEBOX,   {{3,  1 }, {PadX+10, ButtonY+1}}, DIF_NONE, msg(lng::MSetColorTitle), },

#define COLOR_COLUMN(x, y, index) \
	COLOR_CELL(x + 3 * index, y + 0), \
	COLOR_CELL(x + 3 * index, y + 1), \
	COLOR_CELL(x + 3 * index, y + 2), \
	COLOR_CELL(x + 3 * index, y + 3), \
	COLOR_CELL(x + 3 * index, y + 4), \
	COLOR_CELL(x + 3 * index, y + 5)

#define COLOR_PLANE(column, x, y) \
		column(x, y,  0), \
		column(x, y,  1), \
		column(x, y,  2), \
		column(x, y,  3), \
		column(x, y,  4), \
		column(x, y,  5)

		COLOR_PLANE(COLOR_COLUMN, CubeX, CubeY),

#undef COLOR_COLUMN

#define GRAY_COLUMN(x, y, index) \
	COLOR_CELL(x + 3 * index, y + 0), \
	COLOR_CELL(x + 3 * index, y + 1), \
	COLOR_CELL(x + 3 * index, y + 2), \
	COLOR_CELL(x + 3 * index, y + 3)

		COLOR_PLANE(GRAY_COLUMN, GreyX, GreyY),

#undef COLOR_PLANE
#undef GRAY_COLUMN

		PAD_CONTROL(PadX, PadY),

		RGB_CONTROL(RGBX, RGBY),

		{ DI_TEXT,        {{-1, ButtonY-1}, {0, ButtonY-1}}, DIF_SEPARATOR, },

		{ DI_BUTTON,      {{0, ButtonY}, {0, ButtonY}}, DIF_CENTERGROUP | DIF_DEFAULTBUTTON, msg(lng::MOk), },
		{ DI_BUTTON,      {{0, ButtonY}, {0, ButtonY}}, DIF_CENTERGROUP, msg(lng::MCancel), },
	});

	ColorDlg[cd_cube_first].Flags |= DIF_GROUP;

	color_256_state ColorState
	{
		.CurColor = Color,
	};

	init_cube(ColorState);

	{
		const auto RGB = ColorState.as_rgb();
		ColorDlg[cd_text_r].strData = ColorState.channel_value(RGB.r);
		ColorDlg[cd_text_g].strData = ColorState.channel_value(RGB.g);
		ColorDlg[cd_text_b].strData = ColorState.channel_value(RGB.b);
	}

	if (in_closed_range(colors::index::cube_first, static_cast<int>(Color), colors::index::cube_last))
	{
		const auto ControlId = cd_cube_first + cube_rc_mapping[(Color - colors::index::cube_first) % (cube_size * cube_size)];
		ColorDlg[ControlId].Selected = BSTATE_CHECKED;
		ColorDlg[ControlId].Flags |= DIF_FOCUS;
	}
	else if (in_closed_range(colors::index::grey_first, static_cast<int>(Color), colors::index::grey_last))
	{
		const auto ControlId = cd_grey_first + grey_control_by_color[grey_stripe_mapping[Color - colors::index::grey_first]];
		ColorDlg[ControlId].Selected = BSTATE_CHECKED;
		ColorDlg[ControlId].Flags |= DIF_FOCUS;
	}

	const auto Dlg = Dialog::create(ColorDlg, GetColorDlgProc, &ColorState);

	const auto
		DlgWidth = static_cast<int>(ColorDlg[cd_border].X2) + 4,
		DlgHeight = static_cast<int>(ColorDlg[cd_border].Y2) + 2;

	Dlg->SetPosition({ -1, -1, DlgWidth, DlgHeight });
	Dlg->SetHelp(L"ColorPicker256"sv);
	Dlg->Process();

	if (Dlg->GetExitCode() != cd_button_ok)
		return false;

	Color = ColorState.CurColor;
	return true;
}
