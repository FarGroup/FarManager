﻿/*
api_test.c

The purpose of this file is to validate that generated API headers
are self-contained and valid from the C or C++ language perspective.
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

#include "Include/plugin.hpp"
#include "Include/farcolor.hpp"

#ifdef __cplusplus
static_assert(FCF_INDEXMASK == (FCF_FG_INDEX | FCF_BG_INDEX | FCF_FG_UNDERLINE_INDEX), "");
static_assert(FCF_FG_4BIT == FCF_FG_INDEX, "");
static_assert(FCF_BG_4BIT == FCF_BG_INDEX, "");
static_assert(FCF_4BITMASK == FCF_INDEXMASK, "");
static_assert(FCF_FG_UNDERLINE_MASK == (FCF_FG_U_DATA0 | FCF_FG_U_DATA1 | FCF_FG_U_DATA2), "");
#endif
