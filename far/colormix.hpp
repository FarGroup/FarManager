#ifndef COLORMIX_HPP_2A689A10_E8AA_4B87_B167_FAAF812AC90F
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

namespace colors
{
	COLORREF index_value(COLORREF Colour);
	COLORREF color_value(COLORREF Colour);
	COLORREF alpha_value(COLORREF Colour);

	bool is_opaque(COLORREF Colour);
	bool is_transparent(COLORREF Colour);

	COLORREF opaque(COLORREF Colour);
	COLORREF transparent(COLORREF Colour);

	void make_opaque(COLORREF& Colour);
	void make_transparent(COLORREF& Colour);

	size_t color_hash(const FarColor& Value);

	FarColor merge(const FarColor& Bottom, const FarColor& Top);
	WORD FarColorToConsoleColor(const FarColor& Color);
	FarColor ConsoleColorToFarColor(WORD Color);
	COLORREF ConsoleIndexToTrueColor(size_t Index);
	const FarColor& PaletteColorToFarColor(PaletteColors ColorIndex);
	const FarColor* StoreColor(const FarColor& Value);
	COLORREF ARGB2ABGR(int Color);
	// ([[T]FFFFFFFF][:[T]BBBBBBBB])
	string_view ExtractColorInNewFormat(string_view Str, FarColor& Color, bool& Stop);
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
