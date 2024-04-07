/*
color_picker_rgb.cpp

RGB colors extension to the standard color picker
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
#include "color_picker_rgb.hpp"

// Internal:
#include "color_picker_common.hpp"
#include "colormix.hpp"
#include "console.hpp"
#include "lang.hpp"
#include "log.hpp"

// Platform:
#include "platform.hpp"

// Common:
#include "common/2d/algorithm.hpp"
#include "common/view/zip.hpp"

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

static bool pick_color_rgb_gui(COLORREF& Color, std::array<COLORREF, 16>& CustomColors)
{
	if (!os::is_interactive_user_session())
		return false;

	CHOOSECOLOR Params{ sizeof(Params) };

	// Console can be "invisible" in certain fancy scenarios, e.g. a terminal embedded into VS.
	if (IsWindowVisible(console.GetWindow()))
		Params.hwndOwner = console.GetWindow();

	Params.Flags = CC_ANYCOLOR | CC_FULLOPEN | CC_RGBINIT;

	auto CustomColorsCopy = CustomColors;
	Params.lpCustColors = CustomColorsCopy.data();
	Params.rgbResult = Color;

	if (!ChooseColor(&Params))
	{
		if (const auto Error = CommDlgExtendedError())
		{
			LOGWARNING(L"ChooseColor(): Error {:04X} ({})"sv, Error, os::last_error());
		}
		return false;
	}

	Color = Params.rgbResult;
	CustomColors = CustomColorsCopy;

	return true;
}


using namespace color_picker_common;
using rgb = rgba;

constexpr auto cube_size = 16;

static constexpr auto cube_rc_mapping = column_major_iota<uint8_t, cube_size, cube_size>();
static_assert(std::size(cube_rc_mapping) == cube_size * cube_size);

using shape = shapes<std::uint16_t, cube_size>;
using cell  = shape::cell;
using row   = shape::row;
using plane = shape::plane;
using cube  = shape::cube;

static FarColor TrueColorToFarColor(COLORREF const Color)
{
	return
	{
		0,
		{ colors::opaque(colors::invert(Color, false)) },
		{ colors::opaque(Color) }
	};
}

static FarColor TrueColorToFarColorDistinct(COLORREF const Color)
{
	auto Result = TrueColorToFarColor(Color);
	// Naive inversion doesn't work nicely in the middle of the spectre.
	// This way the distance between foreground and background colors is constant and should always produce readable results.
	Result.ForegroundColor ^= 0x3F3F3F;
	return Result;
}

enum color_rgb_dialog_items
{
	cd_border,

	cd_cube_first,
	cd_cube_last = cd_cube_first + 255,

	cd_custom_first,
	cd_custom_last = cd_custom_first + 16 - 1,

	cd_button_up,
	cd_button_left,
	cd_button_right,
	cd_button_down,

	cd_button_home,

	cd_button_minus,
	cd_text_slice,
	cd_button_plus,

	cd_button_zoom,

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

	cd_button_save,

	cd_separator,

	cd_button_ok,
	cd_button_system,
	cd_button_cancel,

	cd_count
};

using rgb_context = rgb_context_t<rgb, color_rgb_dialog_items>;

struct color_rgb_state
{
	using items = color_rgb_dialog_items;
	using rgb_context = rgb_context;
	static constexpr size_t Depth = 256;

	COLORREF CurColor{};

	cube_data<cube> Cube, OuterCube;

	bool IsZoomed{};

	std::array<COLORREF, 16> CustomColors{};
	std::optional<uint8_t> CustomIndex;

	std::bitset<cube_size * cube_size> Overlay;

	static auto channel_value(uint8_t const Channel)
	{
		return far::format(L"{:>3}"sv, Channel);
	}

	rgb as_rgb() const
	{
		return colors::to_rgba(CurColor);
	}

	void from_rgb(rgb const RGB)
	{
		CurColor = colors::to_color(RGB);
	}

	intptr_t GetColorDlgProc(Dialog* Dlg, intptr_t Msg, intptr_t Param1, void* Param2);

};

static rgb cube_rgb(cube const& Cube, uint8_t const x, uint8_t const y, uint8_t const z, bool const IsZoomed)
{
	const auto CellIndex = Cube[z][y][x];
	const auto Multiplier = IsZoomed? 1 : 16;

	return
	{
		static_cast<uint8_t>(CellIndex / 256      * Multiplier),
		static_cast<uint8_t>(CellIndex % 256 / 16 * Multiplier),
		static_cast<uint8_t>(CellIndex % 16       * Multiplier)
	};
}

static rgb cube_rgb(cube const& Cube, uint8_t const Slice, uint8_t const Index, bool const IsZoomed)
{
	const auto CubeIndex = cube_rc_mapping[Index];
	const auto
		Y = CubeIndex / cube_size,
		X = CubeIndex % cube_size;

	return cube_rgb(Cube, X, Y, Slice, IsZoomed);
}

static rgb cube_rgb(color_rgb_state const& ColorState, uint8_t const Index, bool UseZoom = false)
{
	if (UseZoom && ColorState.Overlay[Index])
		return cube_rgb(ColorState.OuterCube.Cube, ColorState.OuterCube.Slice, Index, false);

	if (!ColorState.IsZoomed)
		return cube_rgb(ColorState.Cube.Cube, ColorState.Cube.Slice, Index, false);

	const auto OuterRGB = cube_rgb(ColorState.OuterCube.Cube, ColorState.OuterCube.Slice, ColorState.OuterCube.Index, false);
	const auto InnerRGB = cube_rgb(ColorState.Cube.Cube, ColorState.Cube.Slice, Index, true);

	return
	{
		static_cast<uint8_t>(OuterRGB.r + InnerRGB.r),
		static_cast<uint8_t>(OuterRGB.g + InnerRGB.g),
		static_cast<uint8_t>(OuterRGB.b + InnerRGB.b),
	};
}

static bool match_rgb(rgb Cell, rgb const Color, bool const IsZoomed)
{
	static_assert(cube_size == 16);

	const auto Tolerance = IsZoomed? 0u : cube_size - 1;

	if (!IsZoomed)
	{
		Cell.r &= 0xf0;
		Cell.g &= 0xf0;
		Cell.b &= 0xf0;
	}

	return
		in_closed_range(Cell.r, Color.r, Cell.r + Tolerance) &&
		in_closed_range(Cell.g, Color.g, Cell.g + Tolerance) &&
		in_closed_range(Cell.b, Color.b, Cell.b + Tolerance);
}

static void init_cube(color_rgb_state& ColorState)
{
	const auto ColorRGB = ColorState.as_rgb();

	auto Index = 0;

	for (auto& Plane: ColorState.Cube.Cube)
	{
		for (auto& Line: Plane)
		{
			for (auto& Point: Line)
			{
				Point = Index++;

				const auto RGB = cube_rgb(
					ColorState.Cube.Cube,
					&Point - &Line[0],
					&Line - &Plane[0],
					&Plane - &ColorState.Cube.Cube[0],
					ColorState.IsZoomed
				);

				if (match_rgb(RGB, ColorRGB, ColorState.IsZoomed))
				{
					ColorState.Cube.Slice = &Plane - &ColorState.Cube.Cube[0];
				}
			}
		}
	}
}

static void zoom(Dialog* const Dlg, color_rgb_state& ColorState)
{
	const auto ZoomingIn = !ColorState.IsZoomed;

	if (ZoomingIn)
	{
		ColorState.OuterCube = ColorState.Cube;

		ColorState.Cube.Slice = 0;
		ColorState.Cube.Index = 0;

		ColorState.Overlay.set();
		ColorState.IsZoomed = true;
	}

	const auto index = [](int const r, int const c)
	{
		return r * cube_size + c;
	};

	using bits = std::bitset<cube_size* cube_size>;

	const auto check_neighbor = [&](bits const& Cells, int const r, int const c)
	{
		return
			in_closed_range(0, r, cube_size - 1) &&
			in_closed_range(0, c, cube_size - 1) &&
			Cells[index(r, c)];
	};

	const auto check_neighbors = [&](bits const& Cells, auto& NewCells, int const r, int const c)
	{
		const auto Index = index(r, c);
		if (Cells[Index])
			return;

		// x x x
		// x   x
		// x x x
		if (
			check_neighbor(Cells, r - 1, c - 1) ||
			check_neighbor(Cells, r - 1, c - 0) ||
			check_neighbor(Cells, r - 1, c + 1) ||
			check_neighbor(Cells, r - 0, c - 1) ||
			check_neighbor(Cells, r - 0, c + 1) ||
			check_neighbor(Cells, r + 1, c - 1) ||
			check_neighbor(Cells, r + 1, c + 0) ||
			check_neighbor(Cells, r + 1, c + 1)
		)
			NewCells[Index] = true;
	};

	std::array<bits, cube_size> Steps{};
	size_t StepsSize = 0;

	Steps[StepsSize++][ColorState.OuterCube.Index] = true;

	for (;;)
	{
		bits NewCells;

		for (const auto r: std::views::iota(0, cube_size))
		{
			for (const auto c: std::views::iota(0, cube_size))
			{
				check_neighbors(Steps[StepsSize - 1], NewCells, r, c);
			}
		}

		if (!NewCells.any())
			break;

		Steps[StepsSize] = Steps[StepsSize - 1] | NewCells;
		++StepsSize;
	}

	Dlg->SendMessage(DM_SETCHECK, cd_cube_first, ToPtr(BSTATE_3STATE));

	// N steps -> N - 1 delays
	const auto Delay = 250ms / (cube_size - 1);

	// Sleep won't do, the rendering time is non-negligible and unpredictable
	os::concurrency::event const Event(os::event::type::automatic, os::event::state::nonsignaled);
	os::concurrency::timer const Timer(Delay, Delay, [&]{ Event.set(); });

	for (const auto i: std::views::iota(0uz, StepsSize))
	{
		ColorState.Overlay = ZoomingIn?
			~Steps[i] :
			~Steps[StepsSize - 1 - i];

		Dlg->SendMessage(DM_REDRAW, 0, {});
		if (i != StepsSize - 1)
			Event.wait();
	}

	if (!ZoomingIn)
	{
		ColorState.Cube = ColorState.OuterCube;
		ColorState.IsZoomed = false;
	}

	ColorState.Overlay.reset();
	Dlg->SendMessage(DM_ONCUBECHANGE, 0, {});
}

intptr_t color_rgb_state::GetColorDlgProc(Dialog* Dlg, intptr_t Msg, intptr_t Param1, void* Param2)
{
	switch (Msg)
	{
	case DN_CTLCOLORDLGITEM:
		{
			const auto& Colors = *static_cast<FarDialogItemColors const*>(Param2);

			if (in_closed_range(cd_cube_first, Param1, cd_cube_last))
			{
				const auto RGB = cube_rgb(*this, Param1 - cd_cube_first, true);
				Colors.Colors[0] = TrueColorToFarColorDistinct(colors::to_color(RGB));
				return true;
			}

			if (in_closed_range(cd_custom_first, Param1, cd_custom_last))
			{
				Colors.Colors[0] = TrueColorToFarColorDistinct(CustomColors[Param1 - cd_custom_first]);
				return true;
			}

			switch (const auto Item = static_cast<color_rgb_dialog_items>(Param1))
			{
			case cd_text_rgb:
				{
					// Foreground color is irrelevant
					Colors.Colors[0] = TrueColorToFarColor(CurColor);
					return true;
				}

			case cd_text_r:
			case cd_text_g:
			case cd_text_b:
				{
					const auto Context = get_rgb_context<rgb>(Item);
					auto RGB = as_rgb();
					auto& Channel = std::invoke(Context.Channel, RGB);
					const auto SavedValue = Channel;
					RGB = {};
					Channel = SavedValue;
					// Primary colors don't need no safe distinction
					Colors.Colors[0] = TrueColorToFarColor(colors::to_color(RGB));
					return true;
				}
			default:
				break;
			}
		}
		break;

	case DN_BTNCLICK:
		{
			const auto Button = static_cast<color_rgb_dialog_items>(Param1);
			if (on_button_click(Dlg, Button, *this))
				return true;

			if (Param2 && in_closed_range(cd_cube_first, Button, cd_cube_last))
			{
				Cube.Index = Button - cd_cube_first;
				const auto RGB = cube_rgb(*this, Cube.Index);
				from_rgb(RGB);
				update_rgb_control<color_rgb_state>(Dlg, RGB);
				return true;
			}

			if (Param2 && in_closed_range(cd_custom_first, Button, cd_custom_last))
			{
				CustomIndex = Button - cd_custom_first;
				CurColor = CustomColors[*CustomIndex];
				update_rgb_control<color_rgb_state>(Dlg, as_rgb());
				return true;
			}

			switch (Button)
			{
			case cd_button_home:
				init_cube(*this);
				Cube.Slice = 0;
				Dlg->SendMessage(DM_ONCUBECHANGE, 0, {});
				return true;

			case cd_button_zoom:
				zoom(Dlg, *this);
				return true;

			case cd_button_system:
				{
					const auto SavedColor = CurColor;
					const auto SavedCustomColors = CustomColors;

					if (!pick_color_rgb_gui(CurColor, CustomColors))
						return true;

					if (CurColor != SavedColor)
					{
						update_rgb_control<color_rgb_state>(Dlg, as_rgb());
					}

					if (CustomColors != SavedCustomColors)
					{
						const auto Range = zip(SavedCustomColors, CustomColors, std::views::iota(0u));
						const auto Changed = [](auto const& Item){ return std::get<0>(Item) != std::get<1>(Item); };
						const auto Index = [](auto const& Item){ return std::get<2>(Item); };

						for (const auto i: Range | std::views::filter(Changed) | std::views::transform(Index))
						{
							Dlg->SendMessage(DM_SHOWITEM, i, ToPtr(1));
						}
					}
				}
				return true;

			case cd_button_save:
				{
					uint8_t Index = 0;
					if (CustomIndex)
						Index = *CustomIndex;
					else if (const auto Iterator = std::ranges::find(CustomColors, RGB(255, 255, 255)); Iterator != std::cend(CustomColors))
						Index = Iterator - std::cbegin(CustomColors);

					CustomColors[Index] = CurColor;
					Dlg->SendMessage(DM_SHOWITEM, Index, ToPtr(1));
					return true;
				}

			default:
				break;
			}
		}
		break;

	case DM_ONCUBECHANGE:
		{
			Dlg->SendMessage(DM_SETCHECK, cd_cube_first, ToPtr(BSTATE_3STATE));

			const auto& Plane = Cube.Cube[Cube.Slice];
			for (const auto& Line: Plane)
			{
				for (const auto& Point: Line)
				{
					const auto Index = &Point - &Plane[0][0];
					if (colors::to_color(cube_rgb(*this, Index)) == CurColor)
					{
						Dlg->SendMessage(DM_SETCHECK, cd_cube_first + Index, ToPtr(BSTATE_CHECKED));
						break;
					}
				}
			}

			Dlg->SendMessage(DM_SETTEXTPTR, cd_text_slice, UNSAFE_CSTR(Cube.slice_str()));

			Dlg->SendMessage(DM_REDRAW, 0, {});
		}
		return true;

	default:
		break;
	}

	return Dlg->DefProc(Msg, Param1, Param2);
}

static bool pick_color_rgb_tui(COLORREF& Color, [[maybe_unused]] std::array<COLORREF, 16>& CustomColors)
{
	const auto
		CubeX = 5,
		CubeY = 2,
		PadX = CubeX + cube_size * 3 + 1,
		PadY = CubeY,
		CustomX = CubeX,
		CustomY = CubeY + cube_size + 1,
		RGBW = 9,
		RGBH = 4,
		RGBX = PadX,
		RGBY = CubeY + cube_size * 1 - RGBH,
		ButtonY = CustomY + 1 + 1;

	auto ColorDlg = MakeDialogItems<cd_count>(
	{
		{ DI_DOUBLEBOX,   {{3, 1}, {RGBX+RGBW+1, ButtonY+1}}, DIF_NONE, msg(lng::MSetColorTitle), },

#define COLOR_COLUMN(x, y, index) \
	COLOR_CELL(x + 3 * index, y +  0), \
	COLOR_CELL(x + 3 * index, y +  1), \
	COLOR_CELL(x + 3 * index, y +  2), \
	COLOR_CELL(x + 3 * index, y +  3), \
	COLOR_CELL(x + 3 * index, y +  4), \
	COLOR_CELL(x + 3 * index, y +  5), \
	COLOR_CELL(x + 3 * index, y +  6), \
	COLOR_CELL(x + 3 * index, y +  7), \
	COLOR_CELL(x + 3 * index, y +  8), \
	COLOR_CELL(x + 3 * index, y +  9), \
	COLOR_CELL(x + 3 * index, y + 10), \
	COLOR_CELL(x + 3 * index, y + 11), \
	COLOR_CELL(x + 3 * index, y + 12), \
	COLOR_CELL(x + 3 * index, y + 13), \
	COLOR_CELL(x + 3 * index, y + 14), \
	COLOR_CELL(x + 3 * index, y + 15)

#define COLOR_PLANE(x, y) \
		COLOR_COLUMN(x, y,  0), \
		COLOR_COLUMN(x, y,  1), \
		COLOR_COLUMN(x, y,  2), \
		COLOR_COLUMN(x, y,  3), \
		COLOR_COLUMN(x, y,  4), \
		COLOR_COLUMN(x, y,  5), \
		COLOR_COLUMN(x, y,  6), \
		COLOR_COLUMN(x, y,  7), \
		COLOR_COLUMN(x, y,  8), \
		COLOR_COLUMN(x, y,  9), \
		COLOR_COLUMN(x, y, 10), \
		COLOR_COLUMN(x, y, 11), \
		COLOR_COLUMN(x, y, 12), \
		COLOR_COLUMN(x, y, 13), \
		COLOR_COLUMN(x, y, 14), \
		COLOR_COLUMN(x, y, 15)

		COLOR_PLANE(CubeX, CubeY),

#undef COLOR_PLANE
#undef COLOR_COLUMN

#define CUSTOM_COLOR_ROW(x, y) \
	COLOR_CELL(x + 3 *  0, y), \
	COLOR_CELL(x + 3 *  1, y), \
	COLOR_CELL(x + 3 *  2, y), \
	COLOR_CELL(x + 3 *  3, y), \
	COLOR_CELL(x + 3 *  4, y), \
	COLOR_CELL(x + 3 *  5, y), \
	COLOR_CELL(x + 3 *  6, y), \
	COLOR_CELL(x + 3 *  7, y), \
	COLOR_CELL(x + 3 *  8, y), \
	COLOR_CELL(x + 3 *  9, y), \
	COLOR_CELL(x + 3 * 10, y), \
	COLOR_CELL(x + 3 * 11, y), \
	COLOR_CELL(x + 3 * 12, y), \
	COLOR_CELL(x + 3 * 13, y), \
	COLOR_CELL(x + 3 * 14, y), \
	COLOR_CELL(x + 3 * 15, y)

		CUSTOM_COLOR_ROW(CustomX, CustomY),

#undef CUSTOM_COLOR_ROW

		PAD_CONTROL(PadX, PadY),

		{ DI_BUTTON,      {{PadX+3, PadY+6}, {0, PadY+6}}, DIF_NOBRACKETS, L"[]"sv, },

		RGB_CONTROL(RGBX, RGBY),

		{ DI_BUTTON,      {{PadX, CustomY}, {0, CustomY }}, DIF_NOBRACKETS, L"[«]"sv, },

		{ DI_TEXT,        {{-1, ButtonY-1}, {0, ButtonY-1}}, DIF_SEPARATOR, },

		{ DI_BUTTON,      {{0, ButtonY}, {0, ButtonY}}, DIF_CENTERGROUP | DIF_DEFAULTBUTTON, msg(lng::MOk), },
		{ DI_BUTTON,      {{0, ButtonY}, {0, ButtonY}}, DIF_CENTERGROUP, msg(lng::MSetColorSystemDialog), },
		{ DI_BUTTON,      {{0, ButtonY}, {0, ButtonY}}, DIF_CENTERGROUP, msg(lng::MCancel), },
	});

	ColorDlg[cd_cube_first].Flags |= DIF_GROUP;

	if (!os::is_interactive_user_session())
		ColorDlg[cd_button_system].Flags |= DIF_DISABLE;

	color_rgb_state ColorState
	{
		.CurColor = Color,
		.CustomColors = CustomColors
	};

	init_cube(ColorState);

	{
		const auto RGB = ColorState.as_rgb();
		ColorDlg[cd_text_r].strData = ColorState.channel_value(RGB.r);
		ColorDlg[cd_text_g].strData = ColorState.channel_value(RGB.g);
		ColorDlg[cd_text_b].strData = ColorState.channel_value(RGB.b);

		ColorState.Cube.Index = cube_rc_mapping[cube_size * (RGB.g >> 4) + (RGB.b >> 4)];
		const auto ControlId = cd_cube_first + ColorState.Cube.Index;
		ColorDlg[ControlId].Flags |= DIF_FOCUS;

		if (ColorState.CurColor == colors::to_color(cube_rgb(ColorState, ColorState.Cube.Index)))
			ColorDlg[ControlId].Selected = BSTATE_CHECKED;

		ColorDlg[cd_text_slice].strData = ColorState.Cube.slice_str();
	}

	const auto Dlg = Dialog::create(ColorDlg, std::bind_front(&color_rgb_state::GetColorDlgProc, &ColorState));

	const auto
		DlgWidth = static_cast<int>(ColorDlg[cd_border].X2) + 4,
		DlgHeight = static_cast<int>(ColorDlg[cd_border].Y2) + 2;

	Dlg->SetPosition({ -1, -1, DlgWidth, DlgHeight });
	Dlg->SetHelp(L"ColorPickerRGB"sv);
	Dlg->Process();

	if (Dlg->GetExitCode() != cd_button_ok)
		return false;

	Color = ColorState.CurColor;
	CustomColors = ColorState.CustomColors;
	return true;
}

bool pick_color_rgb(COLORREF& Color, std::array<COLORREF, 16>& CustomColors)
{
	return pick_color_rgb_tui(Color, CustomColors);
}
