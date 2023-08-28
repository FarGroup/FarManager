﻿#ifndef COLORMIX_HPP_2A689A10_E8AA_4B87_B167_FAAF812AC90F
#define COLORMIX_HPP_2A689A10_E8AA_4B87_B167_FAAF812AC90F
#pragma once

/*
colormix.hpp

Работа с цветами
*/
/*
Copyright © 2011 Far Group
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
#include "farcolor.hpp"

// Platform:

// Common:

// External:

//----------------------------------------------------------------------------

struct FarColor;
struct rgba;

namespace colors
{
	namespace index
	{
		constexpr inline auto
			nt_mask = 0xf,
			nt_last = 15,
			nt_size = nt_last + 1,
			cube_first = nt_last + 1,
			cube_size = 6,
			cube_last = cube_first + cube_size * cube_size * cube_size - 1,
			grey_first = cube_last + 1,
			grey_last = 255;
	}

	uint8_t  index_bits(COLORREF Colour);
	COLORREF color_bits(COLORREF Colour);
	COLORREF alpha_bits(COLORREF Colour);

	uint8_t  index_value(COLORREF Colour);
	COLORREF color_value(COLORREF Colour);
	uint8_t  alpha_value(COLORREF Colour);

	bool is_opaque(COLORREF Colour);
	bool is_transparent(COLORREF Colour);

	void set_index_bits(COLORREF& Value, uint8_t Index);
	void set_color_bits(COLORREF& Value, COLORREF Colour);
	void set_alpha_bits(COLORREF& Value, COLORREF Alpha);

	void set_index_value(COLORREF& Value, uint8_t Index);
	void set_color_value(COLORREF& Value, COLORREF Colour);
	void set_alpha_value(COLORREF& Value, uint8_t Alpha);

	COLORREF opaque(COLORREF Colour);
	COLORREF transparent(COLORREF Colour);

	void make_opaque(COLORREF& Colour);
	void make_transparent(COLORREF& Colour);

	COLORREF invert(COLORREF Colour, bool IsIndex);
	void make_invert(COLORREF& Colour, bool IsIndex);

	rgba to_rgba(COLORREF Color);
	COLORREF to_color(rgba Rgba);

	struct index_color_256
	{
		uint8_t
			ForegroundIndex,
			BackgroundIndex;
	};

	size_t color_hash(const FarColor& Value);

	FarColor merge(const FarColor& Bottom, const FarColor& Top);

	using nt_palette_t = std::array<COLORREF, index::nt_size>;
	nt_palette_t nt_palette();

	// TODO: Rename these uniformly
	WORD FarColorToConsoleColor(const FarColor& Color);
	index_color_256 FarColorToConsole256Color(const FarColor& Color);
	FarColor NtColorToFarColor(WORD Color);
	COLORREF ConsoleIndexToTrueColor(COLORREF Color);
	uint8_t ConsoleIndex16to256(uint8_t Color);
	const FarColor& PaletteColorToFarColor(PaletteColors ColorIndex);
	const FarColor* StoreColor(const FarColor& Value);
	COLORREF ARGB2ABGR(int Color);
	// ([[T]FFFFFFFF][:[T]BBBBBBBB])
	string_view ExtractColorInNewFormat(string_view Str, FarColor& Color, bool& Stop);

	struct rgb6
	{
		rgb6() = default;

		constexpr rgb6(uint8_t const R, uint8_t const G, uint8_t const B):
			r(R),
			g(G),
			b(B)
		{
		}

		explicit(false) constexpr rgb6(uint8_t const Color):
			r((Color - index::nt_size) / (index::cube_size * index::cube_size)),
			g((Color - index::nt_size - r * index::cube_size * index::cube_size) / index::cube_size),
			b(Color - index::nt_size - r * index::cube_size * index::cube_size - g * index::cube_size)
		{
		}

		explicit(false) constexpr operator uint8_t() const
		{
			return
				index::nt_size +
				r * index::cube_size * index::cube_size +
				g * index::cube_size +
				b;
		}

		uint8_t r{}, g{}, b{};
	};

	[[nodiscard]]
	string ColorFlagsToString(unsigned long long Flags);

	[[nodiscard]]
	unsigned long long ColorStringToFlags(string_view Flags);
}

template<>
struct std::hash<FarColor>
{
	size_t operator()(const FarColor& Value) const noexcept
	{
		return colors::color_hash(Value);
	}
};

#endif // COLORMIX_HPP_2A689A10_E8AA_4B87_B167_FAAF812AC90F
