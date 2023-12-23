#ifndef PALETTE_HPP_8CFE8272_39B6_4198_9046_E94FEAD9832C
#define PALETTE_HPP_8CFE8272_39B6_4198_9046_E94FEAD9832C
#pragma once

/*
palette.hpp

Таблица цветов
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

// Internal:

// Platform:

// Common:
#include "common/noncopyable.hpp"
#include "common/utility.hpp"

// External:

//----------------------------------------------------------------------------

namespace detail
{
	namespace channel
	{
		constexpr int
			blue    = 0_bit,
			green   = 1_bit,
			red     = 2_bit,
			intense = 3_bit;
	}

	constexpr int
		black   = 0,
		blue    = channel::blue,
		green   = channel::green,
		cyan    = channel::blue  | channel::green,
		red     = channel::red,
		magenta = channel::blue  | channel::red,
		yellow  = channel::green | channel::red,
		white   = channel::blue  | channel::green | channel::red;

	constexpr int fg(int const Bits)
	{
		return Bits << 0;
	}

	constexpr int bg(int const Bits)
	{
		return Bits << 4;
	}
}

enum
{
	F_BLACK        = detail::fg(detail::black),
	F_BLUE         = detail::fg(detail::blue),
	F_GREEN        = detail::fg(detail::green),
	F_CYAN         = detail::fg(detail::cyan),
	F_RED          = detail::fg(detail::red),
	F_MAGENTA      = detail::fg(detail::magenta),
	F_BROWN        = detail::fg(detail::yellow),
	F_LIGHTGRAY    = detail::fg(detail::white),
	F_DARKGRAY     = detail::fg(detail::channel::intense | detail::black),
	F_LIGHTBLUE    = detail::fg(detail::channel::intense | detail::blue),
	F_LIGHTGREEN   = detail::fg(detail::channel::intense | detail::green),
	F_LIGHTCYAN    = detail::fg(detail::channel::intense | detail::cyan),
	F_LIGHTRED     = detail::fg(detail::channel::intense | detail::red),
	F_LIGHTMAGENTA = detail::fg(detail::channel::intense | detail::magenta),
	F_YELLOW       = detail::fg(detail::channel::intense | detail::yellow),
	F_WHITE        = detail::fg(detail::channel::intense | detail::white),

	B_BLACK        = detail::bg(detail::black),
	B_BLUE         = detail::bg(detail::blue),
	B_GREEN        = detail::bg(detail::green),
	B_CYAN         = detail::bg(detail::cyan),
	B_RED          = detail::bg(detail::red),
	B_MAGENTA      = detail::bg(detail::magenta),
	B_BROWN        = detail::bg(detail::yellow),
	B_LIGHTGRAY    = detail::bg(detail::white),
	B_DARKGRAY     = detail::bg(detail::channel::intense | detail::black),
	B_LIGHTBLUE    = detail::bg(detail::channel::intense | detail::blue),
	B_LIGHTGREEN   = detail::bg(detail::channel::intense | detail::green),
	B_LIGHTCYAN    = detail::bg(detail::channel::intense | detail::cyan),
	B_LIGHTRED     = detail::bg(detail::channel::intense | detail::red),
	B_LIGHTMAGENTA = detail::bg(detail::channel::intense | detail::magenta),
	B_YELLOW       = detail::bg(detail::channel::intense | detail::yellow),
	B_WHITE        = detail::bg(detail::channel::intense | detail::white),
};

struct FarColor;

class palette: noncopyable
{
public:
	palette();
	void Load();
	void Save(bool always);
	void ResetToDefaultIndex();
	void ResetToDefaultRGB();
	FarColor Default(size_t Index) const;
	void Set(size_t StartOffset, std::span<FarColor const> Values);
	void CopyTo(std::span<FarColor> Destination) const;
	const FarColor& operator[](size_t Index) const;
	size_t size() const;

	using custom_colors = std::array<COLORREF, 16>;
	custom_colors GetCustomColors() const;
	void SetCustomColors(const custom_colors& Colors);

private:
	void Reset(bool RGB);
	std::vector<FarColor> CurrentPalette;
	bool PaletteChanged{};
	bool CustomColorsChanged{};
};

#endif // PALETTE_HPP_8CFE8272_39B6_4198_9046_E94FEAD9832C
