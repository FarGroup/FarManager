#ifndef COLOR_PICKER_COMMON_HPP_534BE271_FCC7_417F_AE17_5FF0E2E7AC3C
#define COLOR_PICKER_COMMON_HPP_534BE271_FCC7_417F_AE17_5FF0E2E7AC3C
#pragma once

/*
color_picker_common.hpp

*/
/*
Copyright © 2023 Far Group
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
#include "dialog.hpp"
#include "strmix.hpp"

// Platform:
#include "platform.concurrency.hpp"

// Common:
#include "common.hpp"
#include "common/algorithm.hpp"

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

namespace color_picker_common
{
	template<typename cell_t, size_t Size>
	struct shapes
	{
		using cell = cell_t;
		using row = std::array<cell, Size>;
		using plane = std::array<row, Size>;
		using cube = std::array<plane, Size>;

	};

	enum class axis
	{
		x,
		y,
		z
	};

	inline void rotate_coord(int const Size, int& A, int& B, bool const Clockwise)
	{
		// (A', B') = (-B,  A) // CW
		// (A', B') = ( B, -A) // CCW

		if (Clockwise)
			B = Size - 1 - B;
		else
			A = Size - 1 - A;

		std::ranges::swap(A, B);
	}

	template<typename cube>
	void rotate_cube(cube& Cube, axis const Axis, bool const Clockwise)
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
					rotate_coord(static_cast<int>(Cube.size()), NewCoord[A], NewCoord[B], Clockwise);
					NewCube[NewCoord[0]][NewCoord[1]][NewCoord[2]] = Cube[Coord[0]][Coord[1]][Coord[2]];
				}
			}
		}

		std::ranges::swap(Cube, NewCube);
	}

	template<typename plane>
	void copy_row(plane const& SrcPlane, size_t const SrcRow, plane& DestPlane, size_t const DestRow)
	{
		for (size_t Col = 0; Col != SrcPlane.size(); ++Col)
		{
			DestPlane[DestRow][Col] = SrcPlane[SrcRow][Col];
		}
	}

	template<typename plane>
	void copy_col(plane const& SrcPlane, size_t const SrcCol, plane& DestPlane, size_t const DestCol)
	{
		for (size_t Row = 0; Row != SrcPlane.size(); ++Row)
		{
			DestPlane[Row][DestCol] = SrcPlane[Row][SrcCol];
		}
	}

	template<typename plane>
	void move_step(plane const& NewPlane, plane& Plane, size_t const Step, bool const Row, size_t const From, size_t const To)
	{
		const auto Copy = Row? copy_row<plane> : copy_col<plane>;
		const auto Direction = From < To? +1 : -1;

		for (auto i = From; i != To; i += Direction)
			Copy(Plane, i + Direction, Plane, i);

		Copy(NewPlane, Direction > 0? From + Step : From - Step, Plane, To);
	}

	template<typename plane, typename dialog_items>
	void move_plane(plane const& NewPlane, plane& Plane, dialog_items const Direction, size_t const Slice, size_t const Step)
	{
		const auto
			ForwardKey = any_of(Direction, dialog_items::cd_button_up, dialog_items::cd_button_left),
			Forward = Slice > Plane.size() / 2? !ForwardKey : ForwardKey,
			Row = any_of(Direction, dialog_items::cd_button_up, dialog_items::cd_button_down);

		const auto
			First = Forward? 0 : Plane.size() - 1,
			Last = Forward? Plane.size() - 1 : 0;

		move_step(NewPlane, Plane, Step, Row, First, Last);
	}

	enum
	{
		DM_ONCUBECHANGE = DM_USER + 1,
	};

	template<typename cube>
	struct cube_data
	{
		cube Cube;
		uint8_t Slice{}, Index{};

		auto slice_str() const
		{
			return far::format(L"{:X}"sv, Slice);
		}
	};

	template<typename cube_data, typename dialog_items>
	void rotate(cube_data& Cube, Dialog* const Dlg, dialog_items const Button)
	{
		const auto Axis = any_of(Button, dialog_items::cd_button_up, dialog_items::cd_button_down)? axis::x : axis::y;
		const auto Cw = any_of(Button, dialog_items::cd_button_down, dialog_items::cd_button_left);

		const auto OldPlane = Cube.Cube[Cube.Slice];

		rotate_cube(Cube.Cube, Axis, Cw);

		if (Cube.Slice == 0 || Cube.Slice == Cube.Cube.size() - 1)
		{
			// Undo the top plane and refill it step by step for a smooth transition effect
			auto& Plane = Cube.Cube[Cube.Slice];
			const auto NewPlane = Plane;
			Plane = OldPlane;

			// N steps -> N - 1 delays
			const auto Delay = 250ms / (Cube.Cube.size() - 1);

			// Sleep won't do, the rendering time is non-negligible and unpredictable
			os::concurrency::event const Event(os::event::type::automatic, os::event::state::nonsignaled);
			os::concurrency::timer const Timer(Delay, Delay, [&]{ Event.set(); });

			Dlg->SendMessage(DM_SETCHECK, dialog_items::cd_cube_first, ToPtr(BSTATE_3STATE));

			for (const auto i: std::views::iota(0uz, Cube.Cube.size()))
			{
				move_plane(NewPlane, Plane, Button, Cube.Slice, i);
				Dlg->SendMessage(DM_REDRAW, 0, {});

				if (i != Cube.Cube.size() - 1)
					Event.wait();
			}
		}

		Dlg->SendMessage(DM_ONCUBECHANGE, 0, {});
	}

	template<typename cube_data>
	void slice(cube_data& Cube, Dialog* const Dlg, bool const Down)
	{
		if (Cube.Slice == (Down? Cube.Cube.size() - 1 : 0))
			return;

		Cube.Slice += Down? 1 : -1;
		Dlg->SendMessage(DM_ONCUBECHANGE, 0, {});
	}

	template<typename dialog_items>
	auto get_channel_operation(dialog_items const Button)
	{
		switch (Button)
		{
		case dialog_items::cd_button_r_plus:
		case dialog_items::cd_button_g_plus:
		case dialog_items::cd_button_b_plus:
			return +1;

		case dialog_items::cd_button_r_minus:
		case dialog_items::cd_button_g_minus:
		case dialog_items::cd_button_b_minus:
			return -1;

		default:
			std::unreachable();
		}
	}

	template<typename rgb_t, typename dialog_items>
	struct rgb_context_t
	{
		using rgb = rgb_t;
		uint8_t rgb::* Channel;
		dialog_items TextId;
	};

	template<typename rgb, typename dialog_items>
	rgb_context_t<rgb, dialog_items> get_rgb_context(dialog_items const Item)
	{
		switch (Item)
		{
		case dialog_items::cd_text_r:
		case dialog_items::cd_button_r_plus:
		case dialog_items::cd_button_r_minus:
			return { &rgb::r, dialog_items::cd_text_r };

		case dialog_items::cd_text_g:
		case dialog_items::cd_button_g_plus:
		case dialog_items::cd_button_g_minus:
			return { &rgb::g, dialog_items::cd_text_g };

		case dialog_items::cd_text_b:
		case dialog_items::cd_button_b_plus:
		case dialog_items::cd_button_b_minus:
			return { &rgb::b, dialog_items::cd_text_b };

		default:
			std::unreachable();
		}
	}

	template<typename dialog_items, typename color_state>
	bool on_button_click(Dialog* const Dlg, dialog_items const Button, color_state& ColorState)
	{
		switch (Button)
		{
		case dialog_items::cd_button_up:
		case dialog_items::cd_button_down:
		case dialog_items::cd_button_right:
		case dialog_items::cd_button_left:
			rotate(ColorState.Cube, Dlg, Button);
			return true;

		case dialog_items::cd_button_plus:
		case dialog_items::cd_button_minus:
			slice(ColorState.Cube, Dlg, Button == dialog_items::cd_button_plus);
			return true;

		case dialog_items::cd_button_r_plus:
		case dialog_items::cd_button_r_minus:
		case dialog_items::cd_button_g_plus:
		case dialog_items::cd_button_g_minus:
		case dialog_items::cd_button_b_plus:
		case dialog_items::cd_button_b_minus:
			{
				const auto Context = get_rgb_context<decltype(ColorState.as_rgb())>(Button);
				auto RGB = ColorState.as_rgb();
				auto& Channel = std::invoke(Context.Channel, RGB);

				switch (get_channel_operation(Button))
				{
				case -1:
					if (!Channel)
						return true;
					--Channel;
					break;

				case +1:
					if (Channel == ColorState.Depth - 1)
						return true;
					++Channel;
					break;
				}

				ColorState.from_rgb(RGB);
				Dlg->SendMessage(DM_SETTEXTPTR, Context.TextId, UNSAFE_CSTR(ColorState.channel_value(Channel)));
				Dlg->SendMessage(DM_SETCHECK, dialog_items::cd_cube_first, ToPtr(BSTATE_3STATE));
				return true;
			}

		default:
			return false;
		}
	}

	template<typename color_state>
	void update_rgb_control_channel(Dialog* const Dlg, typename color_state::items const Id, uint8_t const Value)
	{
		Dlg->SendMessage(DM_SETTEXTPTR, Id, UNSAFE_CSTR(color_state::channel_value(Value)));
	}

	template<typename color_state>
	void update_rgb_control(Dialog* const Dlg, typename color_state::rgb_context::rgb const RGB)
	{
		SCOPED_ACTION(Dialog::suppress_redraw)(Dlg);
		update_rgb_control_channel<color_state>(Dlg, color_state::items::cd_text_r, RGB.r);
		update_rgb_control_channel<color_state>(Dlg, color_state::items::cd_text_g, RGB.g);
		update_rgb_control_channel<color_state>(Dlg, color_state::items::cd_text_b, RGB.b);
	}
}

#define COLOR_CELL(x, y) \
	{ DI_RADIOBUTTON, {{x, y}, {0, y}}, DIF_MOVESELECT, }

#define PAD_CONTROL(x, y) \
		{ DI_BUTTON,      {{x+3, y+0}, {0, y+0}}, DIF_NOBRACKETS, L"[▲]"sv, }, \
		{ DI_BUTTON,      {{x+0, y+1}, {0, y+1}}, DIF_NOBRACKETS, L"[◄]"sv, }, \
		{ DI_BUTTON,      {{x+6, y+1}, {0, y+1}}, DIF_NOBRACKETS, L"[►]"sv, }, \
		{ DI_BUTTON,      {{x+3, y+2}, {0, y+2}}, DIF_NOBRACKETS, L"[▼]"sv, }, \
		{ DI_BUTTON,      {{x+3, y+1}, {0, y+1}}, DIF_NOBRACKETS, L"[⌂]"sv, }, \
		{ DI_BUTTON,      {{x+0, y+4}, {0, y+4}}, DIF_NOBRACKETS, L"[-]"sv, }, \
		{ DI_TEXT,        {{x+4, y+4}, {0, y+4}},                           }, \
		{ DI_BUTTON,      {{x+6, y+4}, {0, y+4}}, DIF_NOBRACKETS, L"[+]"sv, }


#define SCROLL_CONTROL(x, y) \
		{ DI_TEXT,        {{x, y+0}, {0, y+0}}, DIF_NONE,       L"  0"sv, }, \
		{ DI_BUTTON,      {{x, y+1}, {0, y+1}}, DIF_NOBRACKETS, L"[▲]"sv, }, \
		{ DI_BUTTON,      {{x, y+2}, {0, y+2}}, DIF_NOBRACKETS, L"[▼]"sv, }

#define RGB_CONTROL(x, y) \
		{ DI_TEXT,        {{x + 0, y + 0}, {0, y+0}}, DIF_NONE, L"         "sv, }, \
		SCROLL_CONTROL(x + 0, y + 1), \
		SCROLL_CONTROL(x + 3, y + 1), \
		SCROLL_CONTROL(x + 6, y + 1)


#endif // COLOR_PICKER_COMMON_HPP_534BE271_FCC7_417F_AE17_5FF0E2E7AC3C
