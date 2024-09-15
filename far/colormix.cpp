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
#include "strmix.hpp"

// Platform:

// Common:
#include "common/enum_tokens.hpp"
#include "common/from_string.hpp"
#include "common/view/zip.hpp"

// External:

//----------------------------------------------------------------------------

struct color_mapping
{
	colors::single_color From;
	uint8_t To{};
};

struct colors_mapping
{
	color_mapping
	Foreground,
	Background;

	colors_mapping(FarColor const& Color, colors::index_color_256 const Index):
		Foreground{ { Color.ForegroundColor, Color.IsFgIndex() }, Index.ForegroundIndex },
		Background{ { Color.BackgroundColor, Color.IsBgIndex() }, Index.BackgroundIndex }
	{
	}
};

static class colors_cache
{
public:
	const auto& palette()
	{
		if (!m_Palette)
		{
			m_Palette = colors::default_palette();
			(void)console.GetPalette(*m_Palette);
		}

		return *m_Palette;
	}

	auto& closest_index_16()
	{
		return m_ClosestIndex16;
	}

	auto& closest_index_256()
	{
		return m_ClosestIndex256;
	}

	auto& last_16()
	{
		return m_Last16;
	}

	auto& last_256()
	{
		return m_Last256;
	}

	void invalidate()
	{
		m_Palette.reset();
		m_ClosestIndex16.clear();
		m_ClosestIndex256.clear();
		m_Last16.reset();
		m_Last256.reset();
	}

private:
	std::optional<std::array<COLORREF, 256>> m_Palette;
	std::unordered_map<COLORREF, uint8_t>
		m_ClosestIndex16,
		m_ClosestIndex256;


	std::optional<colors_mapping>
		m_Last16,
		m_Last256;
}
ColorsCache;

namespace colors
{
	single_color single_color::foreground(FarColor const& Color)
	{
		return { Color.ForegroundColor, Color.IsFgIndex() };
	}

	single_color single_color::background(FarColor const& Color)
	{
		return { Color.BackgroundColor, Color.IsBgIndex() };
	}

	single_color single_color::underline(FarColor const& Color)
	{
		return { Color.UnderlineColor, Color.IsUnderlineIndex() };
	}

	single_color single_color::default_color()
	{
		return { default_colorref(), true };
	}

	uint8_t index_bits(COLORREF const Colour)
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

	uint8_t index_value(COLORREF const Colour)
	{
		return index_bits(Colour) >> 0;
	}

	COLORREF color_value(COLORREF const Colour)
	{
		return color_bits(Colour) >> 0;
	}

	uint8_t alpha_value(COLORREF const Colour)
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

	void set_index_bits(COLORREF& Value, uint8_t const Index)
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

	void set_index_value(COLORREF& Value, uint8_t const Index)
	{
		set_index_bits(Value, Index);
	}

	void set_color_value(COLORREF& Value, COLORREF const Colour)
	{
		set_color_bits(Value, Colour);
	}

	void set_alpha_value(COLORREF& Value, uint8_t const Alpha)
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
			assert(!is_default(Colour));

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
		return std::bit_cast<rgba>(Color);
	}

	COLORREF to_color(rgba const Rgba)
	{
		return std::bit_cast<COLORREF>(Rgba);
	}

	size_t color_hash(const FarColor& Value)
	{
		return hash_combine_all(
			Value.Flags,
			Value.BackgroundColor,
			Value.ForegroundColor,
			Value.UnderlineColor);
	}

	FarColor merge(FarColor Bottom, FarColor Top)
	{
		static FarColor LastResult, LastBottom, LastTop;

		if (Bottom == LastBottom && Top == LastTop)
		{
			LastResult.Reserved = Bottom.Reserved;
			return LastResult;
		}

		auto Result = Bottom;

		flags::clear(Result.Flags, FCF_FOREIGN);

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

			const auto to_rgba = [&](COLORREF const Color, bool const IsIndex)
			{
				return colors::to_rgba(IsIndex? ConsoleIndexToTrueColor(resolve_default(Color, Flag == FCF_FG_INDEX)) : Color);
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

		if (colors::is_transparent(Bottom.UnderlineColor))
		{
			Bottom.SetUnderlineIndex(Bottom.IsFgIndex());
			Bottom.UnderlineColor = Bottom.ForegroundColor;
		}

		merge_part(&FarColor::UnderlineColor, FCF_FG_UNDERLINE_INDEX);

		if (Top.Flags & FCF_INHERIT_STYLE)
		{
			constexpr auto StyleMaskWithoutUnderline = FCF_STYLEMASK & ~FCF_FG_UNDERLINE_MASK;
			flags::set(Result.Flags, Top.Flags & StyleMaskWithoutUnderline);

			if (const auto TopUnderline = Top.GetUnderline(); TopUnderline != UNDERLINE_NONE)
				Result.SetUnderline(TopUnderline);
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

	static constexpr auto DefaultPalette = []
	{
		std::array<COLORREF, 256> Result;

		// The system colors
		// Note: for historic reasons these are in NT order and must stay so.
		// The console layer will translate to & from VT as needed.
		{
			enum
			{
				C0 = 0,
				C1 = 128,
				C2 = 192,
				C3 = 255,
			};

			Result[0x0] = RGB(C0, C0, C0); // black
			Result[0x1] = RGB(C0, C0, C1); // blue
			Result[0x2] = RGB(C0, C1, C0); // green
			Result[0x3] = RGB(C0, C1, C1); // cyan
			Result[0x4] = RGB(C1, C0, C0); // red
			Result[0x5] = RGB(C1, C0, C1); // magenta
			Result[0x6] = RGB(C1, C1, C0); // yellow
			Result[0x7] = RGB(C2, C2, C2); // white

			Result[0x8] = RGB(C1, C1, C1); // bright black
			Result[0x9] = RGB(C0, C0, C3); // bright blue
			Result[0xA] = RGB(C0, C3, C0); // bright green
			Result[0xB] = RGB(C0, C3, C3); // bright cyan
			Result[0xC] = RGB(C3, C0, C0); // bright red
			Result[0xD] = RGB(C3, C0, C3); // bright magenta
			Result[0xE] = RGB(C3, C3, C0); // bright yellow
			Result[0xF] = RGB(C3, C3, C3); // bright white
		}

		// 6x6x6 color cube

		const auto step = [](uint8_t const Step)
		{
			return 255 - 40 * (5 - Step);
		};

		constexpr uint8_t channel_value[]
		{
			0,
			step(1),
			step(2),
			step(3),
			step(4),
			step(5)
		};

		for (const auto r: std::views::iota(uint8_t{}, index::cube_size))
		{
			for (const auto g: std::views::iota(uint8_t{}, index::cube_size))
			{
				for (const auto b: std::views::iota(uint8_t{}, index::cube_size))
				{
					Result[rgb6(r, g, b)] = RGB(
						channel_value[r],
						channel_value[g],
						channel_value[b]
					);
				}
			}
		}

		// 24 shades of grey
		for (const auto i: std::views::iota(uint8_t{}, index::grey_count))
		{
			const auto Value = 8 + 10 * i;
			Result[index::grey_first + i] = RGB(Value, Value, Value);
		}

		return Result;
	}();

	palette_t const& default_palette()
	{
		return DefaultPalette;
	}

	static uint8_t get_closest_palette_index(COLORREF const Color, std::span<COLORREF const> const Palette, std::unordered_map<COLORREF, uint8_t>& Map)
	{
		const auto Skip = Palette.size() == index::nt_size? 0 : index::nt_size;

		if (const auto Iterator = std::ranges::find(Palette.begin() + Skip, Palette.end(), Color); Iterator != Palette.end())
			return Iterator - Palette.begin();

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

		return Map.emplace(Color, std::ranges::min_element(Palette.begin() + Skip, Palette.end(), {}, distance) - Palette.begin()).first->second;
	}

	struct index_color_16
	{
		constexpr index_color_16(uint8_t const Background, uint8_t const Foreground) noexcept:
			ForegroundIndex(Foreground),
			BackgroundIndex(Background)
		{
			assert(Foreground < colors::index::nt_size);
			assert(Background < colors::index::nt_size);
		}

		explicit(false) constexpr index_color_16(uint8_t const Byte) noexcept
		{
			*this = std::bit_cast<index_color_16>(Byte);
		}

		constexpr operator uint8_t() const noexcept
		{
			return std::bit_cast<uint8_t>(*this);
		}

		uint8_t ForegroundIndex: 4{};
		uint8_t BackgroundIndex: 4{};
	};

	static WORD emulate_styles(index_color_16 Color, FARCOLORFLAGS const Flags)
	{
		if (Flags & FCF_FG_BOLD)
			Color.ForegroundIndex |= F_INTENSE;

		if (Flags & FCF_FG_FAINT)
			Color.ForegroundIndex &= ~F_INTENSE;

		// COMMON_LVB_REVERSE_VIDEO is a better way, but it only works on Windows 10.
		// Manual swap works everywhere.
		if (Flags & FCF_FG_INVERSE)
		{
			const auto Tmp = Color.ForegroundIndex;
			Color.ForegroundIndex = Color.BackgroundIndex;
			Color.BackgroundIndex = Tmp;
		}

		if (Flags & FCF_FG_INVISIBLE)
		{
			Color.ForegroundIndex = Color.BackgroundIndex;
		}

		WORD Result = Color | (Flags & FCF_RAWATTR_MASK);

		if (Flags & FCF_FG_UNDERLINE_MASK)
			Result |= COMMON_LVB_UNDERSCORE;

		if (Flags & FCF_FG_OVERLINE)
			Result |= COMMON_LVB_GRID_HORIZONTAL;

		return Result;
	}

static index_color_256 color_to_palette_index(FarColor Color, std::optional<colors_mapping> const& Last, std::span<COLORREF const> const Palette, std::unordered_map<COLORREF, uint8_t>& Map)
{
	Color = resolve_defaults(Color);

	const auto convert = [&](single_color const From, color_mapping colors_mapping::* const Getter)
	{
		if (Last)
		{
			if (const auto& LastPart = std::invoke(Getter, *Last); From == LastPart.From)
				return LastPart.To;
		}

		COLORREF CurrentColorValue;

		if (From.IsIndex)
		{
			const auto CurrentIndex = index_value(From.Value);
			const auto IsNtPalette = Palette.size() == index::nt_size;

			if ((IsNtPalette && CurrentIndex <= index::nt_last) || (!IsNtPalette && CurrentIndex > index::nt_last))
				return CurrentIndex;

			CurrentColorValue = ConsoleIndexToTrueColor(CurrentIndex);
		}
		else
		{
			CurrentColorValue = color_value(From.Value);
		}

		return get_closest_palette_index(CurrentColorValue, Palette, Map);
	};

	return
	{
		convert(single_color::foreground(Color), &colors_mapping::Foreground),
		convert(single_color::background(Color), &colors_mapping::Background)
	};
}

static bool same_index(const FarColor& Color, colors_mapping const& Last)
{
	// No need to check underline here
	return
		Color.ForegroundColor == Last.Foreground.From.Value &&
		Color.BackgroundColor == Last.Background.From.Value &&
		Color.IsFgIndex() == Last.Foreground.From.IsIndex &&
		Color.IsBgIndex() == Last.Background.From.IsIndex;
}

WORD FarColorToConsoleColor(const FarColor& Color)
{
	auto& Last = ColorsCache.last_16();
	index_color_256 Result;

	if (Last && same_index(Color, *Last))
	{
		Result =
		{
			Last->Foreground.To,
			Last->Background.To
		};
	}
	else
	{
		Result = color_to_palette_index(Color, Last, { ColorsCache.palette().data(), index::nt_size }, ColorsCache.closest_index_16());

		if (
			Result.ForegroundIndex == Result.BackgroundIndex &&
			color_bits(Color.ForegroundColor) != color_bits(Color.BackgroundColor)
		)
		{
			// oops, unreadable
			// since background is more pronounced we adjust the foreground only
			flags::invert(Result.ForegroundIndex, C_INTENSE);
		}

		Last.emplace(Color, Result);
	}

	return emulate_styles({ Result.BackgroundIndex, Result.ForegroundIndex }, Color.Flags);
}

index_color_256 FarColorToConsole256Color(const FarColor& Color)
{
	auto& Last = ColorsCache.last_256();
	index_color_256 Result;

	if (Last && same_index(Color, *Last))
	{
		Result =
		{
			Last->Foreground.To,
			Last->Background.To
		};
	}
	else
	{
		Result = color_to_palette_index(Color, Last, ColorsCache.palette(), ColorsCache.closest_index_256());
		Last.emplace(Color, Result);
		// This conversion is only used to select the closest color in the picker, no need to care about readability
	}

	return Result;
}

FarColor NtColorToFarColor(WORD Color)
{
	index_color_16 const Color16 = Color;

	return
	{
		FCF_INDEXMASK | FCF_INHERIT_STYLE | (Color & FCF_RAWATTR_MASK),
		{ opaque(Color16.ForegroundIndex) },
		{ opaque(Color16.BackgroundIndex) }
	};
}

COLORREF ConsoleIndexToTrueColor(COLORREF const Color)
{
	assert(!is_default(Color));

	const auto Index = index_value(Color);
	return alpha_bits(Color) | ColorsCache.palette()[Index];
}

const FarColor& PaletteColorToFarColor(PaletteColors ColorIndex)
{
	return Global->Opt->Palette[ColorIndex];
}

const FarColor* StoreColor(const FarColor& Value)
{
	static std::unordered_set<FarColor> ColorSet;
	return std::to_address(ColorSet.emplace(Value).first);
}

COLORREF ARGB2ABGR(COLORREF Color)
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

	if (is_transparent(Target))
		make_opaque(Target);

	return true;
}

static bool ExtractStyle(string_view const Str, FARCOLORFLAGS& TargetFlags)
{
	const auto Flags = ColorStringToFlags(Str) & FCF_STYLEMASK;
	if (!Flags)
		return false;

	TargetFlags |= Flags;
	return true;
}

string_view ExtractColorInNewFormat(string_view const Str, FarColor& Color, bool& Stop)
{
	Stop = false;

	if (!Str.starts_with(L'('))
		return Str;

	const auto Token = Str.substr(1, Str.substr(1).find(L')'));

	if (Token.size() + 1 == Str.size())
	{
		Stop = true;
		return Str;
	}

	std::array<string_view, 5> Parts;

	for (const auto& [t, p]: zip(enum_tokens(Token, L":"sv), Parts))
	{
		if (&p == &Parts.back())
			return Str;

		p = t;
	}

	const auto& [FgColor, BgColor, Style, UlColor, _] = Parts;

	auto NewColor = Color;

	if (
		(FgColor.empty() || ExtractColor(FgColor, NewColor.ForegroundColor, NewColor.Flags, FCF_FG_INDEX)) &&
		(BgColor.empty() || ExtractColor(BgColor, NewColor.BackgroundColor, NewColor.Flags, FCF_BG_INDEX)) &&
		(UlColor.empty() || ExtractColor(UlColor, NewColor.UnderlineColor, NewColor.Flags, FCF_FG_UNDERLINE_INDEX)) &&
		(Style.empty() || ExtractStyle(Style, NewColor.Flags))
	)
	{
		Color = NewColor;
		return Str.substr(Token.size() + 2);
	}

	return Str;
}

static auto UnderlineToString(FARCOLORFLAGS const Flags)
{
	switch (Flags & FCF_FG_UNDERLINE_MASK)
	{
	default:
	case FCF_NONE:                        return L""sv;
	case FCF_FG_U_DATA0:                  return L"underline"sv;
	case FCF_FG_U_DATA1:                  return L"underline_double"sv;
	case FCF_FG_U_DATA1 | FCF_FG_U_DATA0: return L"underline_curly"sv;
	case FCF_FG_U_DATA2:                  return L"underline_dot"sv;
	case FCF_FG_U_DATA2 | FCF_FG_U_DATA0: return L"underline_dash"sv;
	}
}

static auto StringToUnderline(string_view const UnderlineStyle)
{
	constexpr std::pair<FARCOLORFLAGS, string_view> UnderlineNames[]
	{
		{ FCF_FG_U_DATA0,                  L"underline"sv        },
		{ FCF_FG_U_DATA1,                  L"underline_double"sv },
		{ FCF_FG_U_DATA1 | FCF_FG_U_DATA0, L"underline_curly"sv  },
		{ FCF_FG_U_DATA2,                  L"underline_dot"sv    },
		{ FCF_FG_U_DATA2 | FCF_FG_U_DATA0, L"underline_dash"sv   },
	};

	return StringToFlags(UnderlineStyle, UnderlineNames);
}

const std::pair<FARCOLORFLAGS, string_view> ColorFlagNames[]
{
	{ FCF_FG_INDEX,           L"fgindex"sv      },
	{ FCF_BG_INDEX,           L"bgindex"sv      },
	{ FCF_FG_UNDERLINE_INDEX, L"ulindex"sv      },
	{ FCF_INHERIT_STYLE,      L"inherit"sv      },
	{ FCF_FG_BOLD,            L"bold"sv         },
	{ FCF_FG_ITALIC,          L"italic"sv       },
	{ FCF_FG_OVERLINE,        L"overline"sv     },
	{ FCF_FG_STRIKEOUT,       L"strikeout"sv    },
	{ FCF_FG_FAINT,           L"faint"sv        },
	{ FCF_FG_BLINK,           L"blink"sv        },
	{ FCF_FG_INVERSE,         L"inverse"sv      },
	{ FCF_FG_INVISIBLE,       L"invisible"sv    },
};

string ColorFlagsToString(unsigned long long const Flags)
{
	const auto StrFlags = FlagsToString(Flags, ColorFlagNames);
	const auto StrUnderline = UnderlineToString(Flags);
	const auto Separator = !StrFlags.empty() && !StrUnderline.empty()? L" "sv : L""sv;
	return concat(StrFlags, Separator, StrUnderline);
}

unsigned long long ColorStringToFlags(string_view const Flags)
{
	return StringToFlags(Flags, ColorFlagNames) | StringToUnderline(Flags);
}

static FarColor s_ResolvedDefaultColor
{
	FCF_INDEXMASK,
	{ opaque(C_LIGHTGRAY) },
	{ opaque(C_BLACK) },
	{ transparent(C_BLACK) }
};

COLORREF resolve_default(COLORREF Color, bool IsForeground)
{
	if (!is_default(Color))
		return Color;

	const auto ResolvedColor = IsForeground?
		s_ResolvedDefaultColor.ForegroundColor:
		s_ResolvedDefaultColor.BackgroundColor;

	return alpha_bits(Color) | color_bits(ResolvedColor);
}

FarColor resolve_defaults(FarColor const& Color)
{
	auto Result = Color;

	if (Result.IsFgDefault())
	{
		Result.ForegroundColor = alpha_bits(Result.ForegroundColor) | color_bits(s_ResolvedDefaultColor.ForegroundColor);
	}

	if (Result.IsBgDefault())
	{
		Result.BackgroundColor = alpha_bits(Result.BackgroundColor) | color_bits(s_ResolvedDefaultColor.BackgroundColor);
	}

	if (Result.IsUnderlineDefault())
	{
		// Use foreground bits
		Result.UnderlineColor = alpha_bits(Result.UnderlineColor) | color_bits(s_ResolvedDefaultColor.ForegroundColor);
	}

	return Result;
}

FarColor unresolve_defaults(FarColor const& Color)
{
	auto Result = Color;

	if (single_color::foreground(Result) == single_color::foreground(s_ResolvedDefaultColor))
		Result.SetFgDefault();

	if (single_color::background(Result) == single_color::background(s_ResolvedDefaultColor))
		Result.SetBgDefault();

	// We can't read underline color from the console, so no need to worry about it

	return Result;
}

constexpr auto default_color_bit = 23_bit;

COLORREF default_colorref()
{
	return opaque(default_color_bit);
}

FarColor default_color()
{
	return
	{
		FCF_INDEXMASK | FCF_INHERIT_STYLE,
		{ default_colorref() },
		{ default_colorref() },
		{ default_colorref() },
	};
}

bool is_default(COLORREF const Color)
{
	return color_value(Color) == default_color_bit;
}

void store_default_color(FarColor const& Color)
{
	s_ResolvedDefaultColor = Color;
}

void invalidate_cache()
{
	ColorsCache.invalidate();
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

TEST_CASE("colors.default")
{
	FarColor Color
	{
		0,
		{ 0xFFFFFFFF },
		{ 0xFFFFFFFF },
		{ 0xFFFFFFFF },
	};

	REQUIRE(!Color.IsFgDefault());
	REQUIRE(!Color.IsBgDefault());
	REQUIRE(!Color.IsUnderlineDefault());

	Color.SetFgDefault();
	Color.SetBgDefault();
	Color.SetUnderlineDefault();

	REQUIRE(Color.IsFgDefault());
	REQUIRE(Color.IsBgDefault());
	REQUIRE(Color.IsUnderlineDefault());

	REQUIRE(Color.ForegroundColor == colors::default_colorref());
	REQUIRE(Color.ForegroundColor == colors::default_colorref());
	REQUIRE(Color.UnderlineColor == colors::default_colorref());
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
		{ L"()"sv,    },
		{ L"(:)"sv,   },
		{ L"(::)"sv,  },
		{ L"(:::)"sv, },
		{ L"(E)"sv,               { FCF_FG_INDEX, { 0xFF00000E } } },
		{ L"(:F)"sv,              { FCF_BG_INDEX, {}, { 0xFF00000F } } },
		{ L"(B:C)"sv,             { FCF_FG_INDEX | FCF_BG_INDEX, { 0xFF00000B }, { 0xFF00000C } } },
		{ L"(AE)"sv,              { FCF_FG_INDEX, { 0xFF0000AE } } },
		{ L"(:AF)"sv,             { FCF_BG_INDEX, {}, { 0xFF0000AF } } },
		{ L"(AB:AC:blink)"sv,     { FCF_FG_INDEX | FCF_BG_INDEX | FCF_FG_BLINK, { 0xFF0000AB }, { 0xFF0000AC } } },
		{ L"(T00CCCC:TE34234)"sv, { FCF_NONE, { 0xFFCCCC00 }, { 0xFF3442E3 } } },
		{ L"(::bold italic)"sv,   { FCF_FG_BOLD | FCF_FG_ITALIC } },
		{ L"(::bold:T223344)"sv,  { FCF_FG_BOLD, {}, {}, {0xFF443322} } },
		{ L"(::bold:T11223344)"sv,{ FCF_FG_BOLD, {}, {}, {0x11443322} } },
		{ L"(::underline_dash)"sv,{ FCF_FG_U_DATA2 | FCF_FG_U_DATA0 } },
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
		{ L"(::meow)"sv, false },
		{ L"(::::)"sv,   false },
		{ L"(::::1)"sv,  false },
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

TEST_CASE("colors.index_color_16")
{
#if COMPILER(CLANG)
// "constexpr bit_cast involving bit-field is not yet supported"
#define CONSTEXPR_OPT const
#define STATIC_REQUIRE_OPT REQUIRE
#else
#define CONSTEXPR_OPT constexpr
#define STATIC_REQUIRE_OPT STATIC_REQUIRE
#endif

	{
		CONSTEXPR_OPT colors::index_color_16 Color(0xAB);

		STATIC_REQUIRE_OPT(Color == 0xAB);
		STATIC_REQUIRE_OPT(Color.BackgroundIndex == 0xA);
		STATIC_REQUIRE_OPT(Color.ForegroundIndex == 0xB);
		STATIC_REQUIRE_OPT(Color == colors::index_color_16(0xA, 0xB));
	}

#undef STATIC_REQUIRE_OPT
#undef CONSTEXPR_OPT
}

TEST_CASE("colors.closest_palette_index")
{
	{
		const auto self_test = [](std::span<COLORREF const> const Palette, size_t const Begin)
		{
			// By definition, all palette colors should map into the palette as is
			std::unordered_map<COLORREF, uint8_t> Map;
			REQUIRE(std::ranges::all_of(Palette | std::views::drop(Begin), [&](COLORREF const& Color){ return colors::get_closest_palette_index(Color, Palette, Map) == &Color - Palette.data(); }));
		};

		self_test({ colors::DefaultPalette.data(), colors::index::nt_size }, 0);
		self_test(colors::DefaultPalette, colors::index::nt_size);
	}

	static const struct
	{
		COLORREF Color;
		uint8_t Index16, Index256;
	}
	Tests[]
	{
		{ 0x000000, 0x0, 0x10 },
		{ 0x7F7F7F, 0x8, 0xF4 },
		{ 0xFF0000, 0x9, 0x15 },
		{ 0x00FF00, 0xA, 0x2E },
		{ 0xFFFF00, 0xB, 0x33 },
		{ 0x0000FF, 0xC, 0xC4 },
		{ 0xFF00FF, 0xD, 0xC9 },
		{ 0x00FFFF, 0xE, 0xE2 },
		{ 0xFFFFFF, 0xF, 0xE7 },
		{ 0x692101, 0x1, 0x11 },
		{ 0x2E10C8, 0xC, 0xA0 },
		{ 0x6B9AC1, 0x8, 0x89 },
		{ 0x09A84E, 0x6, 0x46 },
		{ 0xB68260, 0x8, 0x43 },
	};

	std::unordered_map<COLORREF, uint8_t> Map16, Map256;

	for (const auto& i: Tests)
	{
		REQUIRE(colors::get_closest_palette_index(i.Color, { colors::DefaultPalette.data(), colors::index::nt_size }, Map16) == i.Index16);
		REQUIRE(colors::get_closest_palette_index(i.Color, colors::DefaultPalette, Map256) == i.Index256);
	}
}
#endif
