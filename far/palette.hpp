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
#include "common/range.hpp"

// External:

//----------------------------------------------------------------------------

enum
{
	F_BLACK        = 0,
	F_BLUE         = FOREGROUND_BLUE,
	F_GREEN        = FOREGROUND_GREEN,
	F_CYAN         = FOREGROUND_BLUE | FOREGROUND_GREEN,
	F_RED          = FOREGROUND_RED,
	F_MAGENTA      = FOREGROUND_BLUE | FOREGROUND_RED,
	F_BROWN        = FOREGROUND_GREEN | FOREGROUND_RED,
	F_LIGHTGRAY    = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED,
	F_DARKGRAY     = FOREGROUND_INTENSITY,
	F_LIGHTBLUE    = FOREGROUND_BLUE | FOREGROUND_INTENSITY,
	F_LIGHTGREEN   = FOREGROUND_GREEN | FOREGROUND_INTENSITY,
	F_LIGHTCYAN    = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY,
	F_LIGHTRED     = FOREGROUND_RED | FOREGROUND_INTENSITY,
	F_LIGHTMAGENTA = FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_INTENSITY,
	F_YELLOW       = FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY,
	F_WHITE        = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY,
	B_BLACK        = 0,
	B_BLUE         = BACKGROUND_BLUE,
	B_GREEN        = BACKGROUND_GREEN,
	B_CYAN         = BACKGROUND_BLUE | BACKGROUND_GREEN,
	B_RED          = BACKGROUND_RED,
	B_MAGENTA      = BACKGROUND_BLUE | BACKGROUND_RED,
	B_BROWN        = BACKGROUND_GREEN | BACKGROUND_RED,
	B_LIGHTGRAY    = BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED,
	B_DARKGRAY     = BACKGROUND_INTENSITY,
	B_LIGHTBLUE    = BACKGROUND_BLUE | BACKGROUND_INTENSITY,
	B_LIGHTGREEN   = BACKGROUND_GREEN | BACKGROUND_INTENSITY,
	B_LIGHTCYAN    = BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_INTENSITY,
	B_LIGHTRED     = BACKGROUND_RED | BACKGROUND_INTENSITY,
	B_LIGHTMAGENTA = BACKGROUND_BLUE | BACKGROUND_RED | BACKGROUND_INTENSITY,
	B_YELLOW       = BACKGROUND_GREEN | BACKGROUND_RED | BACKGROUND_INTENSITY,
	B_WHITE        = BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED | BACKGROUND_INTENSITY,
	F_MASK         = F_WHITE,
	B_MASK         = B_WHITE
};

struct FarColor;

class palette: noncopyable
{
public:
	palette();
	void Load();
	void Save(bool always);
	void ResetToDefault();
	void ResetToBlack();
	void Set(size_t StartOffset, span<FarColor> Values);
	void CopyTo(span<FarColor> Destination) const;
	const FarColor& operator[](size_t Index) const {return CurrentPalette[Index];}
	size_t size() const {return CurrentPalette.size();}

	using custom_colors = std::array<COLORREF, 16>;
	custom_colors GetCustomColors() const;
	void SetCustomColors(const custom_colors& Colors);

private:
	void Reset(bool Black);
	std::vector<FarColor> CurrentPalette;
	bool PaletteChanged{};
	bool CustomColorsChanged{};
};

#endif // PALETTE_HPP_8CFE8272_39B6_4198_9046_E94FEAD9832C
