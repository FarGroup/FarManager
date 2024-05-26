#ifndef REGEX_HELPERS_HPP_6234E87B_6989_44C2_8B6E_1E2B4EA40099
#define REGEX_HELPERS_HPP_6234E87B_6989_44C2_8B6E_1E2B4EA40099
#pragma once

/*
regex_helpers.hpp

Some macro spells to make this witchcraft readable
*/
/*
Copyright © 2015 Far Group
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

// External:

//----------------------------------------------------------------------------

#define RE_BEGIN L"^"
#define RE_END L"$"

#define RE_ESCAPE(x) L"\\" x

#define RE_BACKSLASH RE_ESCAPE(L"\\")
#define RE_SLASH RE_ESCAPE(L"/")
#define RE_Q_MARK RE_ESCAPE(L"?")
#define RE_DOT RE_ESCAPE(L".")
#define RE_SPACE L" "
#define RE_TAB L"\\t"
#define RE_ANY L"."

#define RE_ANY_OF(x) L"[" x L"]"
#define RE_NONE_OF(x) L"[^" x L"]"
#define RE_OR L"|"

#define RE_ZERO_OR_ONE_GREEDY L"?"
#define RE_ZERO_OR_ONE_LAZY L"??"
#define RE_ZERO_OR_MORE_GREEDY L"*"
#define RE_ZERO_OR_MORE_LAZY L"*?"
#define RE_ONE_OR_MORE_GREEDY L"+"
#define RE_ONE_OR_MORE_LAZY L"+?"

#define RE_REPEAT(x) L"{" EXPAND_TO_WIDE_LITERAL(x) L"}"
#define RE_C_GROUP(x) L"(" x L")"
#define RE_NC_GROUP(x) L"(?:" x L")"

#define RE_SLASHES RE_SLASH RE_BACKSLASH
#define RE_ANY_SLASH RE_ANY_OF(RE_SLASHES)
#define RE_ANY_SLASHES RE_ANY_SLASH RE_ONE_OR_MORE_LAZY
#define RE_BACKSLASH_OR_NONE RE_NC_GROUP(RE_BACKSLASH RE_OR RE_END)
#define RE_ANY_SLASH_OR_NONE RE_NC_GROUP(RE_ANY_SLASH RE_OR RE_END)
#define RE_ANY_HEX RE_ANY_OF(L"0-9A-Fa-f")
#define RE_ANY_UUID RE_ANY_HEX RE_REPEAT(8) L"-" RE_NC_GROUP(RE_ANY_HEX RE_REPEAT(4) L"-") RE_REPEAT(3) RE_ANY_HEX RE_REPEAT(12)
#define RE_ANY_WHITESPACE RE_ANY_OF(RE_SPACE RE_TAB) RE_ZERO_OR_MORE_GREEDY

#endif // REGEX_HELPERS_HPP_6234E87B_6989_44C2_8B6E_1E2B4EA40099
