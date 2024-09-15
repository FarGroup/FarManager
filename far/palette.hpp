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

// Use C_* notation in FarColor and other types where Foreground and Background are different fields
enum
{
	C_BLACK        = detail::black,
	C_BLUE         = detail::blue,
	C_GREEN        = detail::green,
	C_CYAN         = detail::cyan,
	C_RED          = detail::red,
	C_MAGENTA      = detail::magenta,
	C_BROWN        = detail::yellow,
	C_LIGHTGRAY    = detail::white,
	C_INTENSE      = detail::channel::intense,
	C_DARKGRAY     = detail::channel::intense | detail::black,
	C_LIGHTBLUE    = detail::channel::intense | detail::blue,
	C_LIGHTGREEN   = detail::channel::intense | detail::green,
	C_LIGHTCYAN    = detail::channel::intense | detail::cyan,
	C_LIGHTRED     = detail::channel::intense | detail::red,
	C_LIGHTMAGENTA = detail::channel::intense | detail::magenta,
	C_YELLOW       = detail::channel::intense | detail::yellow,
	C_WHITE        = detail::channel::intense | detail::white,
};

// Use F_* and B_* notations in CHAR_INFO.Attributes and other types where Foreground and Background are bits in the same field
enum
{
	F_BLACK        = detail::fg(C_BLACK),
	F_BLUE         = detail::fg(C_BLUE),
	F_GREEN        = detail::fg(C_GREEN),
	F_CYAN         = detail::fg(C_CYAN),
	F_RED          = detail::fg(C_RED),
	F_MAGENTA      = detail::fg(C_MAGENTA),
	F_BROWN        = detail::fg(C_BROWN),
	F_LIGHTGRAY    = detail::fg(C_LIGHTGRAY),
	F_INTENSE      = detail::fg(C_INTENSE),
	F_DARKGRAY     = detail::fg(C_DARKGRAY),
	F_LIGHTBLUE    = detail::fg(C_LIGHTBLUE),
	F_LIGHTGREEN   = detail::fg(C_LIGHTGREEN),
	F_LIGHTCYAN    = detail::fg(C_LIGHTCYAN),
	F_LIGHTRED     = detail::fg(C_LIGHTRED),
	F_LIGHTMAGENTA = detail::fg(C_LIGHTMAGENTA),
	F_YELLOW       = detail::fg(C_YELLOW),
	F_WHITE        = detail::fg(C_WHITE),

	B_BLACK        = detail::bg(C_BLACK),
	B_BLUE         = detail::bg(C_BLUE),
	B_GREEN        = detail::bg(C_GREEN),
	B_CYAN         = detail::bg(C_CYAN),
	B_RED          = detail::bg(C_RED),
	B_MAGENTA      = detail::bg(C_MAGENTA),
	B_BROWN        = detail::bg(C_BROWN),
	B_LIGHTGRAY    = detail::bg(C_LIGHTGRAY),
	B_INTENSE      = detail::bg(C_INTENSE),
	B_DARKGRAY     = detail::bg(C_DARKGRAY),
	B_LIGHTBLUE    = detail::bg(C_LIGHTBLUE),
	B_LIGHTGREEN   = detail::bg(C_LIGHTGREEN),
	B_LIGHTCYAN    = detail::bg(C_LIGHTCYAN),
	B_LIGHTRED     = detail::bg(C_LIGHTRED),
	B_LIGHTMAGENTA = detail::bg(C_LIGHTMAGENTA),
	B_YELLOW       = detail::bg(C_YELLOW),
	B_WHITE        = detail::bg(C_WHITE),
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
