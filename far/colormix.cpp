/*
colormix.cpp

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

// BUGBUG
#include "platform.headers.hpp"

// Self:
#include "colormix.hpp"

// Internal:
#include "config.hpp"
#include "console.hpp"
#include "global.hpp"

// Platform:

// Common:
#include "common/from_string.hpp"

// External:

//----------------------------------------------------------------------------

enum
{
	ConsoleBgShift=4,
	ConsoleFgShift=0,
};

namespace colors
{
	COLORREF index_bits(COLORREF const Colour)
	{
		return Colour & INDEXMASK;
	}

	COLORREF color_bits(COLORREF const Colour)
	{
		return Colour & COLORMASK;
	}

	COLORREF alpha_bits(COLORREF const Colour)
	{
		return Colour & ALPHAMASK;
	}

	COLORREF index_value(COLORREF const Colour)
	{
		return index_bits(Colour) >> 0;
	}

	COLORREF color_value(COLORREF const Colour)
	{
		return color_bits(Colour) >> 0;
	}

	COLORREF alpha_value(COLORREF const Colour)
	{
		return alpha_bits(Colour) >> 24;
	}

	bool is_opaque(COLORREF const Colour)
	{
		return alpha_bits(Colour) == ALPHAMASK;
	}

	bool is_transparent(COLORREF const Colour)
	{
		return !alpha_bits(Colour);
	}

	void set_index_bits(COLORREF& Value, COLORREF const Index)
	{
		flags::copy(Value, INDEXMASK, Index);
	}

	void set_color_bits(COLORREF& Value, COLORREF const Colour)
	{
		flags::copy(Value, COLORMASK, Colour);
	}

	void set_alpha_bits(COLORREF& Value, COLORREF const Alpha)
	{
		flags::copy(Value, ALPHAMASK, alpha_value(Alpha) << 24);
	}

	void set_index_value(COLORREF& Value, COLORREF const Index)
	{
		set_index_bits(Value, Index);
	}

	void set_color_value(COLORREF& Value, COLORREF const Colour)
	{
		set_color_bits(Value, Colour);
	}

	void set_alpha_value(COLORREF& Value, COLORREF const Alpha)
	{
		set_alpha_bits(Value, (Alpha & 0xFF) << 24);
	}

	COLORREF opaque(COLORREF const Colour)
	{
		return Colour | ALPHAMASK;
	}

	COLORREF transparent(COLORREF const Colour)
	{
		return Colour & COLORMASK;
	}

	void make_opaque(COLORREF& Colour)
	{
		Colour = opaque(Colour);
	}

	void make_transparent(COLORREF& Colour)
	{
		Colour = transparent(Colour);
	}

	COLORREF invert(COLORREF Colour, bool const IsIndex)
	{
		const auto Alpha = alpha_bits(Colour);

		if (IsIndex)
		{
			const auto Index = index_value(Colour);

			if (Index <= index::nt_last)
				return Alpha | (index::nt_last - Index);

			if (Index <= index::cube_last)
			{
				const auto CubeIndex = Index - index::cube_first;
				return Alpha | (index::cube_last - CubeIndex);
			}

			const auto GreyIndex = Index - index::grey_first;
			return Alpha | (index::grey_last - GreyIndex);
		}

		return Alpha | color_bits(~color_bits(Colour));
	}

	void make_invert(COLORREF& Colour, bool const IsIndex)
	{
		Colour = invert(Colour, IsIndex);
	}

	rgba to_rgba(COLORREF const Color)
	{
		rgba Rgba;
		static_assert(sizeof(Rgba) == sizeof(Color));

		copy_memory(&Color, &Rgba, sizeof(Color));
		return Rgba;
	}

	COLORREF to_color(rgba const Rgba)
	{
		COLORREF Color;
		static_assert(sizeof(Color) == sizeof(Rgba));

		copy_memory(&Rgba, &Color, sizeof(Rgba));
		return Color;
	}

	size_t color_hash(const FarColor& Value)
	{
		return hash_combine_all(
			Value.Flags,
			Value.BackgroundColor,
			Value.ForegroundColor);
	}

	FarColor merge(const FarColor& Bottom, const FarColor& Top)
	{
		static FarColor LastResult, LastBottom, LastTop;

		if (Bottom == LastBottom && Top == LastTop)
		{
			LastResult.Reserved[0] = Bottom.Reserved[0];
			LastResult.Reserved[1] = Bottom.Reserved[1];
			return LastResult;
		}

		auto Result = Bottom;

		const auto merge_part = [&](COLORREF FarColor::*ColorAccessor, const FARCOLORFLAGS Flag)
		{
			const auto TopValue = std::invoke(ColorAccessor, Top);

			// Nothing to apply
			if (is_transparent(TopValue))
				return;

			auto& ResultValue = std::invoke(ColorAccessor, Result);

			// Simple case
			if (is_opaque(TopValue))
			{
				ResultValue = TopValue;
				flags::copy(Result.Flags, Flag, Top.Flags);
				return;
			}

			// Alpha blending
			const auto BottomValue = std::invoke(ColorAccessor, Bottom);

			if (
				TopValue == std::invoke(ColorAccessor, LastTop) && (Top.Flags & Flag) == (LastTop.Flags & Flag) &&
				BottomValue == std::invoke(ColorAccessor, LastBottom) && (Bottom.Flags & Flag) == (LastBottom.Flags & Flag)
			)
			{
				ResultValue = std::invoke(ColorAccessor, LastResult);
				flags::clear(Result.Flags, Flag);
				return;
			}

			const auto to_rgba = [](COLORREF const Color, bool const IsIndex)
			{
				return colors::to_rgba(IsIndex? ConsoleIndexToTrueColor(Color) : Color);
			};

			const auto TopRGBA = to_rgba(TopValue, (Top.Flags & Flag) != 0);
			const auto BottomRGBA = to_rgba(BottomValue, (Bottom.Flags & Flag) != 0);

			const auto calc_channel = [&](unsigned char rgba::*Accessor)
			{
				return static_cast<unsigned char>((std::invoke(Accessor, TopRGBA) * TopRGBA.a + (0xFF - TopRGBA.a) * std::invoke(Accessor, BottomRGBA)) / 0xFF);
			};

			rgba const MergedRGBA
			{
				calc_channel(&rgba::r),
				calc_channel(&rgba::g),
				calc_channel(&rgba::b),
				static_cast<unsigned char>(BottomRGBA.a | ((0xFF - BottomRGBA.a) * TopRGBA.a / 0xFF))
			};

			ResultValue = to_color(MergedRGBA);
			flags::clear(Result.Flags, Flag);
		};

		merge_part(&FarColor::BackgroundColor, FCF_BG_INDEX);
		merge_part(&FarColor::ForegroundColor, FCF_FG_INDEX);

		if (Top.Flags & FCF_INHERIT_STYLE)
		{
			flags::set(Result.Flags, Top.Flags & FCF_STYLEMASK);
		}
		else
		{
			flags::copy(Result.Flags, FCF_STYLEMASK, Top.Flags);
			flags::clear(Result.Flags, FCF_INHERIT_STYLE);
		}

		LastTop = Top;
		LastBottom = Bottom;
		LastResult = Result;

		return Result;
	}

	std::array<COLORREF, 16> nt_palette()
	{
		enum
		{
			C0 = 0,
			C1 = 128,
			C2 = 192,
			C3 = 255,
		};

		return std::array
		{
			RGB(C0, C0, C0), // black
			RGB(C0, C0, C1), // blue
			RGB(C0, C1, C0), // green
			RGB(C0, C1, C1), // cyan
			RGB(C1, C0, C0), // red
			RGB(C1, C0, C1), // magenta
			RGB(C1, C1, C0), // yellow
			RGB(C2, C2, C2), // white

			RGB(C1, C1, C1), // bright black
			RGB(C0, C0, C3), // bright blue
			RGB(C0, C3, C0), // bright green
			RGB(C0, C3, C3), // bright cyan
			RGB(C3, C0, C0), // bright red
			RGB(C3, C0, C3), // bright magenta
			RGB(C3, C3, C0), // bright yellow
			RGB(C3, C3, C3)  // bright white
		};
	}

	static auto console_palette()
	{
		if (std::array<COLORREF, 16> Palette; console.GetPalette(Palette))
			return Palette;

		return nt_palette();
	}

	constexpr unsigned char Index8ToIndex4[]
	{
		 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
		 0,  1,  1,  1,  9,  9,  2,  1,  1,  1,  1,  1,  2,  2,  3,  3,
		 3,  3,  2,  2, 11, 11,  3,  3, 10, 10, 11, 11, 11, 11, 10, 10,
		10, 10, 11, 11,  5,  5,  5,  5,  1,  1,  8,  8,  1,  1,  9,  9,
		 2,  2,  3,  3,  3,  3,  2,  2, 11, 11,  3,  3, 10, 10, 11, 11,
		11, 11, 10, 10, 10, 10, 11, 11,  4, 13,  5,  5,  5,  5,  4, 13,
		13, 13, 13, 13,  6,  8,  8,  8,  9,  9, 10, 10, 11, 11,  3,  3,
		10, 10, 11, 11, 11, 11, 10, 10, 10, 10, 11, 11,  4, 13, 13, 13,
		13, 13,  4, 12, 13, 13, 13, 13,  6,  6,  8,  8,  9,  9,  6,  6,
		 7,  7,  7,  7, 10, 14, 14, 14,  7,  7, 10, 10, 14, 14, 11, 11,
		 4, 12, 13, 13, 13, 13,  4, 12, 13, 13, 13, 13,  6,  6, 12, 12,
		 7,  7,  6,  6,  7,  7,  7,  7,  6, 14, 14, 14,  7,  7, 14, 14,
		14, 14, 15, 15, 12, 12, 13, 13, 13, 13, 12, 12, 12, 12, 13, 13,
		 6, 12, 12, 12,  7,  7,  6,  6,  7,  7,  7,  7,  6, 14, 14, 14,
		 7,  7, 14, 14, 14, 14, 15, 15,  0,  0,  0,  0,  0,  0,  8,  8,
		 8,  8,  8,  8,  8,  8,  8,  8,  7,  7,  7,  7,  7,  7, 15, 15
	};

	enum
	{
		C_Step = 40,

		C0 = 0,
		C1 = 95,
		C2 = C1 + C_Step,
		C3 = C2 + C_Step,
		C4 = C3 + C_Step,
		C5 = C4 + C_Step
	};

	constexpr auto gray(int const Shade)
	{
		const auto Value = 8 + 10 * Shade;
		return RGB(Value, Value, Value);
	}

	static constexpr COLORREF Index8ToRGB[]
	{
		// First 16 colors are dynamic, see console_palette().
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

		// 6x6x6 color cube
		RGB(C0, C0, C0), RGB(C0, C0, C1), RGB(C0, C0, C2), RGB(C0, C0, C3), RGB(C0, C0, C4), RGB(C0, C0, C5),
		RGB(C0, C1, C0), RGB(C0, C1, C1), RGB(C0, C1, C2), RGB(C0, C1, C3), RGB(C0, C1, C4), RGB(C0, C1, C5),
		RGB(C0, C2, C0), RGB(C0, C2, C1), RGB(C0, C2, C2), RGB(C0, C2, C3), RGB(C0, C2, C4), RGB(C0, C2, C5),
		RGB(C0, C3, C0), RGB(C0, C3, C1), RGB(C0, C3, C2), RGB(C0, C3, C3), RGB(C0, C3, C4), RGB(C0, C3, C5),
		RGB(C0, C4, C0), RGB(C0, C4, C1), RGB(C0, C4, C2), RGB(C0, C4, C3), RGB(C0, C4, C4), RGB(C0, C4, C5),
		RGB(C0, C5, C0), RGB(C0, C5, C1), RGB(C0, C5, C2), RGB(C0, C5, C3), RGB(C0, C5, C4), RGB(C0, C5, C5),
		RGB(C1, C0, C0), RGB(C1, C0, C1), RGB(C1, C0, C2), RGB(C1, C0, C3), RGB(C1, C0, C4), RGB(C1, C0, C5),
		RGB(C1, C1, C0), RGB(C1, C1, C1), RGB(C1, C1, C2), RGB(C1, C1, C3), RGB(C1, C1, C4), RGB(C1, C1, C5),
		RGB(C1, C2, C0), RGB(C1, C2, C1), RGB(C1, C2, C2), RGB(C1, C2, C3), RGB(C1, C2, C4), RGB(C1, C2, C5),
		RGB(C1, C3, C0), RGB(C1, C3, C1), RGB(C1, C3, C2), RGB(C1, C3, C3), RGB(C1, C3, C4), RGB(C1, C3, C5),
		RGB(C1, C4, C0), RGB(C1, C4, C1), RGB(C1, C4, C2), RGB(C1, C4, C3), RGB(C1, C4, C4), RGB(C1, C4, C5),
		RGB(C1, C5, C0), RGB(C1, C5, C1), RGB(C1, C5, C2), RGB(C1, C5, C3), RGB(C1, C5, C4), RGB(C1, C5, C5),
		RGB(C2, C0, C0), RGB(C2, C0, C1), RGB(C2, C0, C2), RGB(C2, C0, C3), RGB(C2, C0, C4), RGB(C2, C0, C5),
		RGB(C2, C1, C0), RGB(C2, C1, C1), RGB(C2, C1, C2), RGB(C2, C1, C3), RGB(C2, C1, C4), RGB(C2, C1, C5),
		RGB(C2, C2, C0), RGB(C2, C2, C1), RGB(C2, C2, C2), RGB(C2, C2, C3), RGB(C2, C2, C4), RGB(C2, C2, C5),
		RGB(C2, C3, C0), RGB(C2, C3, C1), RGB(C2, C3, C2), RGB(C2, C3, C3), RGB(C2, C3, C4), RGB(C2, C3, C5),
		RGB(C2, C4, C0), RGB(C2, C4, C1), RGB(C2, C4, C2), RGB(C2, C4, C3), RGB(C2, C4, C4), RGB(C2, C4, C5),
		RGB(C2, C5, C0), RGB(C2, C5, C1), RGB(C2, C5, C2), RGB(C2, C5, C3), RGB(C2, C5, C4), RGB(C2, C5, C5),
		RGB(C3, C0, C0), RGB(C3, C0, C1), RGB(C3, C0, C2), RGB(C3, C0, C3), RGB(C3, C0, C4), RGB(C3, C0, C5),
		RGB(C3, C1, C0), RGB(C3, C1, C1), RGB(C3, C1, C2), RGB(C3, C1, C3), RGB(C3, C1, C4), RGB(C3, C1, C5),
		RGB(C3, C2, C0), RGB(C3, C2, C1), RGB(C3, C2, C2), RGB(C3, C2, C3), RGB(C3, C2, C4), RGB(C3, C2, C5),
		RGB(C3, C3, C0), RGB(C3, C3, C1), RGB(C3, C3, C2), RGB(C3, C3, C3), RGB(C3, C3, C4), RGB(C3, C3, C5),
		RGB(C3, C4, C0), RGB(C3, C4, C1), RGB(C3, C4, C2), RGB(C3, C4, C3), RGB(C3, C4, C4), RGB(C3, C4, C5),
		RGB(C3, C5, C0), RGB(C3, C5, C1), RGB(C3, C5, C2), RGB(C3, C5, C3), RGB(C3, C5, C4), RGB(C3, C5, C5),
		RGB(C4, C0, C0), RGB(C4, C0, C1), RGB(C4, C0, C2), RGB(C4, C0, C3), RGB(C4, C0, C4), RGB(C4, C0, C5),
		RGB(C4, C1, C0), RGB(C4, C1, C1), RGB(C4, C1, C2), RGB(C4, C1, C3), RGB(C4, C1, C4), RGB(C4, C1, C5),
		RGB(C4, C2, C0), RGB(C4, C2, C1), RGB(C4, C2, C2), RGB(C4, C2, C3), RGB(C4, C2, C4), RGB(C4, C2, C5),
		RGB(C4, C3, C0), RGB(C4, C3, C1), RGB(C4, C3, C2), RGB(C4, C3, C3), RGB(C4, C3, C4), RGB(C4, C3, C5),
		RGB(C4, C4, C0), RGB(C4, C4, C1), RGB(C4, C4, C2), RGB(C4, C4, C3), RGB(C4, C4, C4), RGB(C4, C4, C5),
		RGB(C4, C5, C0), RGB(C4, C5, C1), RGB(C4, C5, C2), RGB(C4, C5, C3), RGB(C4, C5, C4), RGB(C4, C5, C5),
		RGB(C5, C0, C0), RGB(C5, C0, C1), RGB(C5, C0, C2), RGB(C5, C0, C3), RGB(C5, C0, C4), RGB(C5, C0, C5),
		RGB(C5, C1, C0), RGB(C5, C1, C1), RGB(C5, C1, C2), RGB(C5, C1, C3), RGB(C5, C1, C4), RGB(C5, C1, C5),
		RGB(C5, C2, C0), RGB(C5, C2, C1), RGB(C5, C2, C2), RGB(C5, C2, C3), RGB(C5, C2, C4), RGB(C5, C2, C5),
		RGB(C5, C3, C0), RGB(C5, C3, C1), RGB(C5, C3, C2), RGB(C5, C3, C3), RGB(C5, C3, C4), RGB(C5, C3, C5),
		RGB(C5, C4, C0), RGB(C5, C4, C1), RGB(C5, C4, C2), RGB(C5, C4, C3), RGB(C5, C4, C4), RGB(C5, C4, C5),
		RGB(C5, C5, C0), RGB(C5, C5, C1), RGB(C5, C5, C2), RGB(C5, C5, C3), RGB(C5, C5, C4), RGB(C5, C5, C5),

		// 24 shades of grey
		gray(0),
		gray(1),
		gray(2),
		gray(3),
		gray(4),
		gray(5),
		gray(6),
		gray(7),
		gray(8),
		gray(9),
		gray(10),
		gray(11),
		gray(12),
		gray(13),
		gray(14),
		gray(15),
		gray(16),
		gray(17),
		gray(18),
		gray(19),
		gray(20),
		gray(21),
		gray(22),
		gray(23),
	};


	static_assert(sizeof(Index8ToIndex4) == 256);

	static WORD get_closest_palette_index(COLORREF const Color)
	{
		static const auto Palette = console_palette();

		static std::unordered_map<COLORREF, WORD> Map;

		if (const auto Iterator = Map.find(Color); Iterator != Map.cend())
			return Iterator->second;

		const auto PointRGBA = to_rgba(Color);

		const auto distance = [&](COLORREF const PaletteColor)
		{
			const auto PaletteRGBA = to_rgba(PaletteColor);

			const auto distance_part = [&](unsigned char rgba::* const Getter)
			{
				return std::abs(
					int{ std::invoke(Getter, PointRGBA) } -
					int{ std::invoke(Getter, PaletteRGBA) }
				);
			};

			return std::sqrt(
				std::pow(distance_part(&rgba::r), 2) +
				std::pow(distance_part(&rgba::g), 2) +
				std::pow(distance_part(&rgba::b), 2)
			);
		};

		const auto ClosestPointIterator = std::min_element(ALL_CONST_RANGE(Palette), [&](COLORREF const Item1, COLORREF const Item2)
		{
			return distance(Item1) < distance(Item2);
		});

		const auto ClosestIndex = ClosestPointIterator - Palette.cbegin();

		Map.emplace(Color, ClosestIndex);

		return ClosestIndex;
	}

	static WORD emulate_styles(WORD Color, FARCOLORFLAGS const Flags)
	{
		if (Flags & FCF_FG_BOLD)
			Color |= FOREGROUND_INTENSITY;

		if (Flags & FCF_FG_FAINT)
			Color &= ~FOREGROUND_INTENSITY;

		if (Flags & (FCF_FG_UNDERLINE | FCF_FG_UNDERLINE2))
			Color |= COMMON_LVB_UNDERSCORE;

		if (Flags & FCF_FG_OVERLINE)
			Color |= COMMON_LVB_GRID_HORIZONTAL;

		// COMMON_LVB_REVERSE_VIDEO is a better way, but it only works on Windows 10.
		// Manual swap works everywhere.
		if (Flags & FCF_FG_INVERSE)
			Color = ((Color & 0x00F0) >> 4) | ((Color & 0x000F) << 4);

		if (Flags & FCF_FG_INVISIBLE)
			Color = (Color & 0x00F0) | ((Color & 0x00F0) >> 4);

		return Color | (Flags & FCF_RAWATTR_MASK);
	}

WORD FarColorToConsoleColor(const FarColor& Color)
{
	static FarColor LastColor{};
	static WORD Result = 0;

	if (
		Color.BackgroundColor == LastColor.BackgroundColor &&
		Color.ForegroundColor == LastColor.ForegroundColor &&
		(Color.Flags & FCF_INDEXMASK) == (LastColor.Flags & FCF_INDEXMASK)
	)
		return emulate_styles(Result, Color.Flags);

	const auto convert_and_save = [&](COLORREF FarColor::* const Getter, FARCOLORFLAGS const Flag, BYTE& IndexColor)
	{
		const auto Current = std::invoke(Getter, Color);
		auto& Last = std::invoke(Getter, LastColor);

		if (Current == Last)
			return;

		Last = Current;

		if (Color.Flags & Flag)
		{
			IndexColor = Index8ToIndex4[index_value(Current)];
			return;
		}

		IndexColor = get_closest_palette_index(color_value(Current));
	};

	static struct
	{
		uint8_t
			Foreground,
			Background;
	}
	Index{};

	convert_and_save(&FarColor::ForegroundColor, FCF_FG_INDEX, Index.Foreground);
	convert_and_save(&FarColor::BackgroundColor, FCF_BG_INDEX, Index.Background);

	LastColor.Flags = Color.Flags;

	auto FinalIndex = Index;

	if (
		FinalIndex.Foreground == FinalIndex.Background &&
		color_bits(Color.ForegroundColor) != color_bits(Color.BackgroundColor)
	)
	{
		// oops, unreadable
		// since background is more pronounced we adjust the foreground only
		flags::invert(FinalIndex.Foreground, FOREGROUND_INTENSITY);
	}

	Result =
		(FinalIndex.Foreground << ConsoleFgShift) |
		(FinalIndex.Background << ConsoleBgShift);

	return emulate_styles(Result, Color.Flags);
}

FarColor NtColorToFarColor(WORD Color)
{
	return
	{
		FCF_FG_INDEX | FCF_BG_INDEX | FCF_INHERIT_STYLE | (Color & FCF_RAWATTR_MASK),
		{ opaque(index_bits(Color >> ConsoleFgShift) & index::nt_mask) },
		{ opaque(index_bits(Color >> ConsoleBgShift) & index::nt_mask) }
	};
}

COLORREF ConsoleIndexToTrueColor(COLORREF const Color)
{
	const auto Index = index_value(Color);
	return alpha_bits(Color) | (Index < 16? console_palette()[Index] : Index8ToRGB[Index]);
}

const FarColor& PaletteColorToFarColor(PaletteColors ColorIndex)
{
	return Global->Opt->Palette[ColorIndex];
}

const FarColor* StoreColor(const FarColor& Value)
{
	static std::unordered_set<FarColor> ColorSet;
	return &*ColorSet.emplace(Value).first;
}

COLORREF ARGB2ABGR(int Color)
{
	return (Color & 0xFF00FF00) | ((Color & 0x00FF0000) >> 16) | ((Color & 0x000000FF) << 16);
}

static bool ExtractColor(string_view const Str, COLORREF& Target, FARCOLORFLAGS& TargetFlags, FARCOLORFLAGS SetFlag)
{
	const auto IsTrueColour = Str.front() == L'T';

	if (!from_string(Str.substr(IsTrueColour? 1 : 0), Target, nullptr, 16))
		return false;

	if (IsTrueColour)
	{
		Target = ARGB2ABGR(Target);
		TargetFlags &= ~SetFlag;
	}
	else
	{
		TargetFlags |= SetFlag;
	}

	return true;
}

string_view ExtractColorInNewFormat(string_view const Str, FarColor& Color, bool& Stop)
{
	Stop = false;

	if (!starts_with(Str, L'('))
		return Str;

	const auto Token = Str.substr(1, Str.substr(1).find(L')'));

	if (Token.size() + 1 == Str.size())
	{
		Stop = true;
		return Str;
	}

	const auto [FgColor, BgColor] = split(Token, L':');

	auto NewColor = Color;

	if (
		(FgColor.empty() || ExtractColor(FgColor, NewColor.ForegroundColor, NewColor.Flags, FCF_FG_INDEX)) &&
		(BgColor.empty() || ExtractColor(BgColor, NewColor.BackgroundColor, NewColor.Flags, FCF_BG_INDEX))
	)
	{
		Color = NewColor;
		return Str.substr(Token.size() + 2);
	}

	return Str;
}

}

#ifdef ENABLE_TESTS

#include "testing.hpp"

TEST_CASE("colors.COLORREF")
{
	static const struct
	{
		COLORREF Src, Alpha, Color, ABGR, Index;
		bool Opaque, Transparent;

	}
	Tests[]
	{
		{ 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00, false, true  },
		{ 0x00000001, 0x00000000, 0x00000001, 0x00010000, 0x01, false, true  },
		{ 0x01000000, 0x01000000, 0x00000000, 0x01000000, 0x00, false, false },
		{ 0xFF000000, 0xFF000000, 0x00000000, 0xFF000000, 0x00, true,  false },
		{ 0x00ABCDEF, 0x00000000, 0x00ABCDEF, 0x00EFCDAB, 0xEF, false, true  },
		{ 0xFFFFFFFF, 0xFF000000, 0x00FFFFFF, 0xFFFFFFFF, 0xFF, true,  false },
	};

	for (const auto& i: Tests)
	{
		REQUIRE(colors::alpha_bits(i.Src) == i.Alpha);
		REQUIRE(colors::color_bits(i.Src) == i.Color);
		REQUIRE(colors::index_bits(i.Src) == i.Index);
		REQUIRE(colors::is_opaque(i.Src) == i.Opaque);
		REQUIRE(colors::is_transparent(i.Src) == i.Transparent);
		REQUIRE(colors::is_opaque(colors::opaque(i.Src)));
		REQUIRE(colors::is_transparent(colors::transparent(i.Src)));
		REQUIRE(colors::ARGB2ABGR(i.Src) == i.ABGR);
	}

	COLORREF Color;
	colors::set_index_value(Color = 0xffffffff, 0x7);
	REQUIRE(Color == 0xffffff07);
	colors::set_color_value(Color = 0xffffffff, 0x42);
	REQUIRE(Color == 0xff000042);
	colors::set_alpha_value(Color = 0xffffffff, 0x42);
	REQUIRE(Color == 0x42ffffff);
}

TEST_CASE("colors.merge")
{
	static const struct
	{
		FarColor Bottom, Top, Merged;
	}
	Tests[]
	{
		{ { 0, {0x00000000}, {0x00000000} }, { 0, {0x00000000}, {0x00000000} }, { 0, {0x00000000}, {0x00000000} } },
		{ { 0, {0xFF123456}, {0xFF654321} }, { 0, {0xFFABCDEF}, {0xFFFEDCBA} }, { 0, {0xFFABCDEF}, {0xFFFEDCBA} } },
		{ { 0, {0x80000000}, {0xFF000000} }, { 0, {0x80000000}, {0x01000000} }, { 0, {0xBF000000}, {0xFF000000} } },
		{ { 0, {0xFFFFFFFF}, {0xFF000000} }, { 0, {0x80000000}, {0x80FFFFFF} }, { 0, {0xFF7F7F7F}, {0xFF808080} } },
		{ { 0, {0xFF00D5FF}, {0xFFBB5B00} }, { 0, {0x800000FF}, {0x80000000} }, { 0, {0xFF006AFF}, {0xFF5D2D00} } },
	};

	for (const auto& i: Tests)
	{
		const auto Color = colors::merge(i.Bottom, i.Top);
		REQUIRE(Color == i.Merged);
	}
}

TEST_CASE("colors.parser")
{
	static const struct
	{
		string_view Input;
		FarColor Color;
	}
	ValidTests[]
	{
		{ L"()"sv,                { } },
		{ L"(E)"sv,               { FCF_FG_INDEX, {0xE}, {0} } },
		{ L"(:F)"sv,              { FCF_BG_INDEX, {0}, {0xF} } },
		{ L"(B:C)"sv,             { FCF_FG_INDEX | FCF_BG_INDEX, {0xB}, {0xC} } },
		{ L"(AE)"sv,              { FCF_FG_INDEX, { 0xAE }, { 0 } } },
		{ L"(:AF)"sv,             { FCF_BG_INDEX, { 0 }, { 0xAF } } },
		{ L"(AB:AC)"sv,           { FCF_FG_INDEX | FCF_BG_INDEX, {0xAB}, {0xAC} } },
		{ L"(T00CCCC:TE34234)"sv, { 0, {0x00CCCC00}, {0x003442E3} } },
	};

	for (const auto& i: ValidTests)
	{
		FarColor Color{};
		bool Stop = false;
		const auto Tail = colors::ExtractColorInNewFormat(i.Input, Color, Stop);
		REQUIRE(Color == i.Color);
		REQUIRE(Tail.empty());
		REQUIRE(!Stop);
	}

	static const struct
	{
		string_view Input;
		bool Stop;
	}
	InvalidTests[]
	{
		{ {},            false },
		{ L"("sv,        true  },
		{ L"(z"sv,       true  },
		{ L"(z)"sv,      false },
		{ L"(0:z)"sv,    false },
		{ L"(z:0)"sv,    false },
		{ L"(Tz)"sv,     false },
		{ L"( )"sv,      false },
		{ L"( 0)"sv,     false },
		{ L"( -0)"sv,    false },
		{ L"( +0)"sv,    false },
	};

	for (const auto& i: InvalidTests)
	{
		FarColor Color{};
		bool Stop = false;
		const auto Tail = colors::ExtractColorInNewFormat(i.Input, Color, Stop);
		REQUIRE(Tail.size() == i.Input.size());
		REQUIRE(Stop == i.Stop);
	}
}
#endif
