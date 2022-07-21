﻿/*
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
#include "config.hpp"
#include "console.hpp"
#include "global.hpp"

// Platform:

// Common:

// External:

//----------------------------------------------------------------------------

bool pick_color_rgb(COLORREF& Color)
{
	CHOOSECOLOR Params{ sizeof(Params) };

	Params.hwndOwner = console.GetWindow();
	Params.Flags = CC_ANYCOLOR | CC_FULLOPEN | CC_RGBINIT;

	auto CustomColors = Global->Opt->Palette.GetCustomColors();
	Params.lpCustColors = CustomColors.data();
	Params.rgbResult = Color;

	if (!ChooseColor(&Params))
		return false;

	Global->Opt->Palette.SetCustomColors(CustomColors);
	Color = Params.rgbResult;

	return true;
}
