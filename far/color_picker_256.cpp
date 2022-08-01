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
#include "dialog.hpp"
#include "filepanels.hpp"
#include "ctrlobj.hpp"
#include "scrbuf.hpp"
#include "panel.hpp"
#include "interf.hpp"
#include "console.hpp"
#include "colormix.hpp"
#include "lang.hpp"
#include "strmix.hpp"

// Platform:

// Common:
#include "common.hpp"
#include "common/2d/point.hpp"

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

// Cube controls are column-major

static constexpr uint8_t cube_rc_mapping[]
{
	0,  6, 12, 18, 24, 30,
	1,  7, 13, 19, 25, 31,
	2,  8, 14, 20, 26, 32,
	3,  9, 15, 21, 27, 33,
	4, 10, 16, 22, 28, 34,
	5, 11, 17, 23, 29, 35
};

static_assert(std::size(cube_rc_mapping) == colors::index::cube_size * colors::index::cube_size);

static constexpr uint16_t distinct_cube_index(uint8_t const Index)
{
	return Index << 8 | (colors::index::cube_last - (Index - colors::index::cube_first));
}

static constexpr uint16_t distinct_grey(uint8_t const Value)
{
	// Naive inversion doesn't work nicely in the middle of the spectre.
	// This way the distance between foreground and background colors is constant and should always produce readable results.
	return (colors::index::grey_first + Value) << 8 | (colors::index::grey_first + Value + (Value < 12? +12 : -12));
}

static constexpr uint16_t GreyColorIndex[]
{
	distinct_grey(0),
	distinct_grey(1),
	distinct_grey(2),
	distinct_grey(3),
	distinct_grey(4),
	distinct_grey(5),
	distinct_grey(6),
	distinct_grey(7),
	distinct_grey(8),
	distinct_grey(9),
	distinct_grey(10),
	distinct_grey(11),
	distinct_grey(12),
	distinct_grey(13),
	distinct_grey(14),
	distinct_grey(15),
	distinct_grey(16),
	distinct_grey(17),
	distinct_grey(18),
	distinct_grey(19),
	distinct_grey(20),
	distinct_grey(21),
	distinct_grey(22),
	distinct_grey(23)
};

static_assert(std::size(GreyColorIndex) == colors::index::grey_last - colors::index::grey_first + 1);

// Grey controls are column-major

static constexpr uint8_t grey_control_by_color[]
{
	0,  4,  8, 12, 16, 20,
	1,  5,  9, 13, 17, 21,
	2,  6, 10, 14, 18, 22,
	3,  7, 11, 15, 19, 23
};

static_assert(std::size(grey_control_by_color) == std::size(GreyColorIndex));

static constexpr uint8_t grey_color_by_control[]
{
	0,  6, 12, 18,
	1,  7, 13, 19,
	2,  8, 14, 20,
	3,  9, 15, 21,
	4, 10, 16, 22,
	5, 11, 17, 23
};

static_assert(std::size(grey_color_by_control) == std::size(GreyColorIndex));

static constexpr uint8_t grey_stripe_mapping[]
{
	 0,  1,  2,  3,  4,  5,
	11, 10,  9,  8,  7,  6,
	12, 13, 14, 15, 16, 17,
	23, 22, 21, 20, 19, 18
};

static_assert(std::size(grey_stripe_mapping) == std::size(GreyColorIndex));

using cube = unsigned char[colors::index::cube_size][colors::index::cube_size][colors::index::cube_size];

struct rgb6
{
	rgb6() = default;

	rgb6(COLORREF const Color):
		r((Color - 16) / 36),
		g((Color - 16 - r * 36) / 6),
		b(Color - 16 - r * 36 - g * 6)

	{
	}

	operator COLORREF() const
	{
		return 16 + r * 36 + g * 6 + b;
	}

	uint8_t r{}, g{}, b{};
};

struct color256_state
{
	COLORREF CurColor;
	rgb6 RGB;
	cube Cube;
	int CubeSlice{};
	unsigned char CurrentIndex{};
};

static void rorate_coord(int& A, int& B, bool const Clockwise)
{
	// (A', B') = (-B,  A) // CW
	// (A', B') = ( B, -A) // CCW

	if (Clockwise)
		B = colors::index::cube_size - 1 - B;
	else
		A = colors::index::cube_size - 1 - A;

	using std::swap;
	swap(A, B);
}

enum class axis
{
	x,
	y,
	z
};

static void rotate_cube(cube& Cube, axis const Axis, bool const Clockwise)
{
	enum dimension
	{
		z,
		y,
		x
	};

	size_t A = 0, B = 0;

	switch (Axis)
	{
	case axis::x:
		A = dimension::y;
		B = dimension::z;
		break;

	case axis::y:
		A = dimension::z;
		B = dimension::x;
		break;

	case axis::z:
		A = dimension::x;
		B = dimension::y;
		break;
	}

	cube NewCube{};

	for (const auto& Plane: Cube)
	{
		for (const auto& Line: Plane)
		{
			for (const auto& Point: Line)
			{
				std::array const Coord
				{
					static_cast<int>(&Plane - &Cube[0]),
					static_cast<int>(&Line - &Plane[0]),
					static_cast<int>(&Point - &Line[0])
				};

				auto NewCoord = Coord;

				rorate_coord(NewCoord[A], NewCoord[B], Clockwise);

				NewCube[NewCoord[0]][NewCoord[1]][NewCoord[2]] = Cube[Coord[0]][Coord[1]][Coord[2]];
			}
		}
	}

	using std::swap;
	swap(Cube, NewCube);
}

static FarColor Console256ColorToFarColor(WORD const Color)
{
	return
	{
		FCF_FG_INDEX | FCF_BG_INDEX | FCF_INHERIT_STYLE,
		{ colors::opaque(colors::index_bits(Color >> 0)) },
		{ colors::opaque(colors::index_bits(Color >> 8)) }
	};
}

enum color256_dialog_items
{
	cd_border,

	cd_cube_first,
	cd_cube_last = cd_cube_first + 35,

	cd_grey_first,
	cd_grey_last = cd_grey_first + 23,

	cd_radio_rgb,

	cd_button_up,
	cd_button_left,
	cd_button_right,
	cd_button_down,

	cd_button_plus,
	cd_button_minus,

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

struct rgb_context
{
	uint8_t rgb6::*Channel;
	color256_dialog_items TextId;
	int Multiplier;

};
static rgb_context get_rgb_context(color256_dialog_items const Item)
{
	switch (Item)
	{
	case cd_text_r:
	case cd_button_r_plus:
	case cd_button_r_minus:
		return { &rgb6::r, cd_text_r, 36 };

	case cd_text_g:
	case cd_button_g_plus:
	case cd_button_g_minus:
		return { &rgb6::g, cd_text_g, 6 };

	case cd_text_b:
	case cd_button_b_plus:
	case cd_button_b_minus:
		return { &rgb6::b, cd_text_b, 1 };

	default:
		assert(false);
		UNREACHABLE;
	}
}

static auto get_channel_operation(color256_dialog_items const Button)
{
	switch (Button)
	{
	case cd_button_r_plus:
	case cd_button_g_plus:
	case cd_button_b_plus:
		return +1;

	case cd_button_r_minus:
	case cd_button_g_minus:
	case cd_button_b_minus:
		return -1;

	default:
		assert(false);
		UNREACHABLE;
	}
}

static auto channel_value(uint8_t const Channel)
{
	return format(FSTR(L" {} "sv), Channel);
}

static intptr_t GetColorDlgProc(Dialog* Dlg, intptr_t Msg, intptr_t Param1, void* Param2)
{
	auto& ColorState = edit_as<color256_state>(Dlg->SendMessage(DM_GETDLGDATA, 0, nullptr));

	const auto cube_index = [&](intptr_t ControlId)
	{
		const auto CubeIndex = cube_rc_mapping[Param1 - cd_cube_first];
		const auto
			Y = CubeIndex / colors::index::cube_size,
			X = CubeIndex % colors::index::cube_size;

		return ColorState.Cube[ColorState.CubeSlice][Y][X];
	};

	const auto DM_ONCUBECHANGE = DM_USER + 1;

	switch (Msg)
	{
	case DN_CTLCOLORDLGITEM:
		{
			const auto Colors = static_cast<FarDialogItemColors*>(Param2);

			if (in_closed_range(cd_cube_first, Param1, cd_cube_last))
			{
				const auto ColorIndex = cube_index(Param1);
				Colors->Colors[0] = Console256ColorToFarColor(distinct_cube_index(ColorIndex));
				return TRUE;
			}

			if (in_closed_range(cd_grey_first, Param1, cd_grey_last))
			{
				Colors->Colors[0] = Console256ColorToFarColor(GreyColorIndex[grey_stripe_mapping[grey_color_by_control[Param1 - cd_grey_first]]]);
				return TRUE;
			}

			switch (const auto Item = static_cast<color256_dialog_items>(Param1))
			{
			case cd_radio_rgb:
				{
					const COLORREF ColorIndex = ColorState.RGB;
					Colors->Colors[0] = Console256ColorToFarColor(distinct_cube_index(ColorIndex));
					return TRUE;
				}

			case cd_text_r:
			case cd_text_g:
			case cd_text_b:
				{
					const auto Context = get_rgb_context(Item);
					const auto Channel = std::invoke(Context.Channel, ColorState.RGB);
					const auto ColorIndex = 16 + Channel * Context.Multiplier;
					Colors->Colors[0] = Console256ColorToFarColor(distinct_cube_index(ColorIndex));
					return TRUE;
				}

			default:
				break;
			}

			return FALSE;
		}

	case DN_BTNCLICK:
		{
			switch(const auto Button = static_cast<color256_dialog_items>(Param1))
			{
			case cd_button_up:
			case cd_button_down:
			case cd_button_right:
			case cd_button_left:
				{
					const auto Axis = any_of(Button, cd_button_up, cd_button_down)? axis::x : axis::y;
					const auto Cw = any_of(Button, cd_button_down, cd_button_left);
					rotate_cube(ColorState.Cube, Axis, Cw);
					Dlg->SendMessage(DM_ONCUBECHANGE, 0, {});
					return TRUE;
				}

			case cd_button_plus:
			case cd_button_minus:
				{
					ColorState.CubeSlice = std::clamp(ColorState.CubeSlice + (Button == cd_button_plus? +1 : -1), 0, colors::index::cube_size - 1);
					Dlg->SendMessage(DM_ONCUBECHANGE, 0, {});
					return TRUE;
				}

			case cd_button_r_plus:
			case cd_button_r_minus:
			case cd_button_g_plus:
			case cd_button_g_minus:
			case cd_button_b_plus:
			case cd_button_b_minus:
				{
					const auto Context = get_rgb_context(Button);
					auto& Channel = std::invoke(Context.Channel, ColorState.RGB);
					switch (get_channel_operation(Button))
					{
					case -1:
						if (Channel)
							--Channel;
						break;

					case +1:
						if (Channel < 5)
							++Channel;
						break;
					}

					Dlg->SendMessage(DM_SETTEXTPTR, Context.TextId, UNSAFE_CSTR(channel_value(Channel)));
					return TRUE;
				}

			default:
				break;
			}

			if (Param2 && in_closed_range(cd_cube_first, Param1, cd_cube_last))
			{
				ColorState.CurColor = cube_index(Param1);
				ColorState.RGB = ColorState.CurColor;

				SCOPED_ACTION(Dialog::suppress_redraw)(Dlg);
				Dlg->SendMessage(DM_SETTEXTPTR, cd_text_r, UNSAFE_CSTR(channel_value(ColorState.RGB.r)));
				Dlg->SendMessage(DM_SETTEXTPTR, cd_text_g, UNSAFE_CSTR(channel_value(ColorState.RGB.g)));
				Dlg->SendMessage(DM_SETTEXTPTR, cd_text_b, UNSAFE_CSTR(channel_value(ColorState.RGB.b)));

				return TRUE;
			}

			if (Param2 && in_closed_range(cd_grey_first, Param1, cd_grey_last))
			{
				ColorState.CurColor = GreyColorIndex[grey_stripe_mapping[grey_color_by_control[Param1 - cd_grey_first]]] >> 8;
				return TRUE;
			}

			if (Param2 && Param1 == cd_radio_rgb)
			{
				ColorState.CurColor = ColorState.RGB;
				return TRUE;
			}
		}
		break;

	case DM_ONCUBECHANGE:
		{
			if (in_closed_range(colors::index::cube_first, static_cast<int>(ColorState.CurColor), colors::index::cube_last))
				Dlg->SendMessage(DM_SETCHECK, cd_cube_first, ToPtr(BSTATE_3STATE));

			const auto& Plane = ColorState.Cube[ColorState.CubeSlice];
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
		return TRUE;

	default:
		break;
	}

	return Dlg->DefProc(Msg, Param1, Param2);
}

bool pick_color_256(COLORREF& Color)
{
	const auto
		CubeX = 5,
		CubeY = 2,
		PadX = CubeX + colors::index::cube_size * 3 + 1,
		PadY = CubeY,
		GreyX = CubeX,
		GreyY = CubeY + colors::index::cube_size + 1,
		GreyH = 4,
		RGBX = PadX,
		RGBY = GreyY,
		ButtonY = GreyY + GreyH + 1;

	auto ColorDlg = MakeDialogItems<cd_count>(
	{
		{ DI_DOUBLEBOX,   {{3,  1 }, {PadX+10, ButtonY+1}}, DIF_NONE, msg(lng::MSetColorTitle), },

		{ DI_RADIOBUTTON, {{CubeX+3*0, CubeY+0}, {0, CubeY+0}}, DIF_MOVESELECT | DIF_GROUP, },
		{ DI_RADIOBUTTON, {{CubeX+3*0, CubeY+1}, {0, CubeY+1}}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{CubeX+3*0, CubeY+2}, {0, CubeY+2}}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{CubeX+3*0, CubeY+3}, {0, CubeY+3}}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{CubeX+3*0, CubeY+4}, {0, CubeY+4}}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{CubeX+3*0, CubeY+5}, {0, CubeY+5}}, DIF_MOVESELECT, },

		{ DI_RADIOBUTTON, {{CubeX+3*1, CubeY+0}, {0, CubeY+0}}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{CubeX+3*1, CubeY+1}, {0, CubeY+1}}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{CubeX+3*1, CubeY+2}, {0, CubeY+2}}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{CubeX+3*1, CubeY+3}, {0, CubeY+3}}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{CubeX+3*1, CubeY+4}, {0, CubeY+4}}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{CubeX+3*1, CubeY+5}, {0, CubeY+5}}, DIF_MOVESELECT, },

		{ DI_RADIOBUTTON, {{CubeX+3*2, CubeY+0}, {0, CubeY+0}}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{CubeX+3*2, CubeY+1}, {0, CubeY+1}}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{CubeX+3*2, CubeY+2}, {0, CubeY+2}}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{CubeX+3*2, CubeY+3}, {0, CubeY+3}}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{CubeX+3*2, CubeY+4}, {0, CubeY+4}}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{CubeX+3*2, CubeY+5}, {0, CubeY+5}}, DIF_MOVESELECT, },

		{ DI_RADIOBUTTON, {{CubeX+3*3, CubeY+0}, {0, CubeY+0}}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{CubeX+3*3, CubeY+1}, {0, CubeY+1}}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{CubeX+3*3, CubeY+2}, {0, CubeY+2}}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{CubeX+3*3, CubeY+3}, {0, CubeY+3}}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{CubeX+3*3, CubeY+4}, {0, CubeY+4}}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{CubeX+3*3, CubeY+5}, {0, CubeY+5}}, DIF_MOVESELECT, },

		{ DI_RADIOBUTTON, {{CubeX+3*4, CubeY+0}, {0, CubeY+0}}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{CubeX+3*4, CubeY+1}, {0, CubeY+1}}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{CubeX+3*4, CubeY+2}, {0, CubeY+2}}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{CubeX+3*4, CubeY+3}, {0, CubeY+3}}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{CubeX+3*4, CubeY+4}, {0, CubeY+4}}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{CubeX+3*4, CubeY+5}, {0, CubeY+5}}, DIF_MOVESELECT, },

		{ DI_RADIOBUTTON, {{CubeX+3*5, CubeY+0}, {0, CubeY+0}}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{CubeX+3*5, CubeY+1}, {0, CubeY+1}}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{CubeX+3*5, CubeY+2}, {0, CubeY+2}}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{CubeX+3*5, CubeY+3}, {0, CubeY+3}}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{CubeX+3*5, CubeY+4}, {0, CubeY+4}}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{CubeX+3*5, CubeY+5}, {0, CubeY+5}}, DIF_MOVESELECT, },


		{ DI_RADIOBUTTON, {{GreyX+3*0, GreyY+0}, {0, GreyY+0}}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{GreyX+3*0, GreyY+1}, {0, GreyY+1}}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{GreyX+3*0, GreyY+2}, {0, GreyY+2}}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{GreyX+3*0, GreyY+3}, {0, GreyY+3}}, DIF_MOVESELECT, },

		{ DI_RADIOBUTTON, {{GreyX+3*1, GreyY+0}, {0, GreyY+0}}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{GreyX+3*1, GreyY+1}, {0, GreyY+1}}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{GreyX+3*1, GreyY+2}, {0, GreyY+2}}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{GreyX+3*1, GreyY+3}, {0, GreyY+3}}, DIF_MOVESELECT, },

		{ DI_RADIOBUTTON, {{GreyX+3*2, GreyY+0}, {0, GreyY+0}}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{GreyX+3*2, GreyY+1}, {0, GreyY+1}}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{GreyX+3*2, GreyY+2}, {0, GreyY+2}}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{GreyX+3*2, GreyY+3}, {0, GreyY+3}}, DIF_MOVESELECT, },

		{ DI_RADIOBUTTON, {{GreyX+3*3, GreyY+0}, {0, GreyY+0}}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{GreyX+3*3, GreyY+1}, {0, GreyY+1}}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{GreyX+3*3, GreyY+2}, {0, GreyY+2}}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{GreyX+3*3, GreyY+3}, {0, GreyY+3}}, DIF_MOVESELECT, },

		{ DI_RADIOBUTTON, {{GreyX+3*4, GreyY+0}, {0, GreyY+0}}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{GreyX+3*4, GreyY+1}, {0, GreyY+1}}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{GreyX+3*4, GreyY+2}, {0, GreyY+2}}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{GreyX+3*4, GreyY+3}, {0, GreyY+3}}, DIF_MOVESELECT, },

		{ DI_RADIOBUTTON, {{GreyX+3*5, GreyY+0}, {0, GreyY+0}}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{GreyX+3*5, GreyY+1}, {0, GreyY+1}}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{GreyX+3*5, GreyY+2}, {0, GreyY+2}}, DIF_MOVESELECT, },
		{ DI_RADIOBUTTON, {{GreyX+3*5, GreyY+3}, {0, GreyY+3}}, DIF_MOVESELECT, },

		{ DI_RADIOBUTTON, {{RGBX+3, RGBY+0}, {0, RGBY+0}}, DIF_MOVESELECT, },

		{ DI_BUTTON,      {{PadX+3, PadY+0}, {0, PadY+0}}, DIF_NOBRACKETS, L"[▲]"sv, },
		{ DI_BUTTON,      {{PadX+0, PadY+1}, {0, PadY+1}}, DIF_NOBRACKETS, L"[◄]"sv, },
		{ DI_BUTTON,      {{PadX+6, PadY+1}, {0, PadY+1}}, DIF_NOBRACKETS, L"[►]"sv, },
		{ DI_BUTTON,      {{PadX+3, PadY+2}, {0, PadY+2}}, DIF_NOBRACKETS, L"[▼]"sv, },
		{ DI_BUTTON,      {{PadX+1, PadY+4}, {0, PadY+4}}, DIF_NOBRACKETS, L"[+]"sv, },
		{ DI_BUTTON,      {{PadX+5, PadY+4}, {0, PadY+4}}, DIF_NOBRACKETS, L"[-]"sv, },

		{ DI_TEXT,        {{RGBX+0, RGBY+1}, {0, RGBY+1}}, DIF_NONE,       L" 0 "sv, },
		{ DI_BUTTON,      {{RGBX+0, RGBY+2}, {0, RGBY+2}}, DIF_NOBRACKETS, L"[▲]"sv, },
		{ DI_BUTTON,      {{RGBX+0, RGBY+3}, {0, RGBY+3}}, DIF_NOBRACKETS, L"[▼]"sv, },
		{ DI_TEXT,        {{RGBX+3, RGBY+1}, {0, RGBY+1}}, DIF_NONE,       L" 0 "sv, },
		{ DI_BUTTON,      {{RGBX+3, RGBY+2}, {0, RGBY+2}}, DIF_NOBRACKETS, L"[▲]"sv, },
		{ DI_BUTTON,      {{RGBX+3, RGBY+3}, {0, RGBY+3}}, DIF_NOBRACKETS, L"[▼]"sv, },
		{ DI_TEXT,        {{RGBX+6, RGBY+1}, {0, RGBY+1}}, DIF_NONE,       L" 0 "sv, },
		{ DI_BUTTON,      {{RGBX+6, RGBY+2}, {0, RGBY+2}}, DIF_NOBRACKETS, L"[▲]"sv, },
		{ DI_BUTTON,      {{RGBX+6, RGBY+3}, {0, RGBY+3}}, DIF_NOBRACKETS, L"[▼]"sv, },

		{ DI_TEXT,        {{-1, ButtonY-1}, {0, ButtonY-1}}, DIF_SEPARATOR, },

		{ DI_BUTTON,      {{0, ButtonY}, {0, ButtonY}}, DIF_CENTERGROUP | DIF_DEFAULTBUTTON, msg(lng::MOk), },
		{ DI_BUTTON,      {{0, ButtonY}, {0, ButtonY}}, DIF_CENTERGROUP, msg(lng::MCancel), },
	});

	color256_state ColorState
	{
		Color,
	};

	if (in_closed_range(colors::index::cube_first, static_cast<int>(Color), colors::index::cube_last))
	{
		ColorState.RGB = Color;
		ColorDlg[cd_text_r].strData = channel_value(ColorState.RGB.r);
		ColorDlg[cd_text_g].strData = channel_value(ColorState.RGB.g);
		ColorDlg[cd_text_b].strData = channel_value(ColorState.RGB.b);
	}

	{
		unsigned char Index = colors::index::cube_first;
		for (auto& Plane: ColorState.Cube)
		{
			for (auto& Line: Plane)
			{
				for (auto& Point: Line)
				{
					Point = Index++;

					if (Point == Color)
					{
						ColorState.CubeSlice = &Plane - &ColorState.Cube[0];
						const auto ControlId = cd_cube_first + cube_rc_mapping[&Point - &Plane[0][0]];
						ColorDlg[ControlId].Selected = BSTATE_CHECKED;
						ColorDlg[ControlId].Flags |= DIF_FOCUS;
					}
				}
			}
		}
	}

	if (in_closed_range(colors::index::grey_first, static_cast<int>(Color), colors::index::grey_last))
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
