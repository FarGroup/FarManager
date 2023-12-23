#ifndef MESSAGE_HPP_640AC104_875B_41AE_8EF5_8A99913A6896
#define MESSAGE_HPP_640AC104_875B_41AE_8EF5_8A99913A6896
#pragma once

/*
message.hpp

Вывод MessageBox
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
#include "exception.hpp"

// Platform:

// Common:
#include "common/span.hpp"

// External:

//----------------------------------------------------------------------------

enum class lng;

enum
{
	MSG_WARNING        = 0_bit,
	MSG_KEEPBACKGROUND = 2_bit,
	MSG_LEFTALIGN      = 3_bit,

	MSG_KILLSAVESCREEN = 28_bit,
	MSG_NOPLUGINS      = 29_bit,
};

class Plugin;

enum class message_result
{
	cancelled = -1,

	first_button = 0,
	second_button,
	third_button,
	fourth_button,
	fifth_button,
};

message_result Message(
	unsigned Flags,
	string_view Title,
	std::vector<string> Strings,
	span<lng const> Buttons,
	string_view HelpTopic = {},
	const UUID* Id = nullptr
);

message_result Message(
	unsigned Flags,
	const error_state_ex& ErrorState,
	string_view Title,
	std::vector<string> Strings,
	span<lng const> Buttons,
	string_view HelpTopic = {},
	const UUID* Id = nullptr,
	span<string const> Inserts = {}
);

message_result Message(
	unsigned Flags,
	const error_state_ex* ErrorState,
	string_view Title,
	std::vector<string> Strings,
	std::vector<string> Buttons,
	string_view HelpTopic,
	const UUID* Id,
	Plugin* PluginNumber
);

#endif // MESSAGE_HPP_640AC104_875B_41AE_8EF5_8A99913A6896
