/*
constitle.cpp

Заголовок консоли
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

// BUGBUG
#include "platform.headers.hpp"

// Self:
#include "constitle.hpp"

// Internal:
#include "lang.hpp"
#include "config.hpp"
#include "farversion.hpp"
#include "scrbuf.hpp"
#include "strmix.hpp"
#include "global.hpp"
#include "mix.hpp"

// Platform:
#include "platform.concurrency.hpp"
#include "platform.env.hpp"
#include "platform.security.hpp"

// Common:

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

static string expand_title_variables(string_view const Str)
{
	// " - Far%Ver%Admin"
	/*
		%Ver      - 3.0.5660.0
		%Platform - x86
		%Admin    - MFarTitleAddonsAdmin
		%PID      - current PID
	*/

	static const auto Version = version_to_string(build::version());
	static const auto Platform = build::platform();
	static const auto Pid = str(GetCurrentProcessId());

	const auto IsAdmin = os::security::is_admin()? msg(lng::MFarTitleAddonsAdmin) : L""sv;

	auto ExpandedStr = os::env::expand(Str);

	replace_icase(ExpandedStr, L"%Ver.%Build"sv, Version); // For compatibility
	replace_icase(ExpandedStr, L"%Ver"sv, Version);
	replace_icase(ExpandedStr, L"%Platform"sv, Platform);
	replace_icase(ExpandedStr, L"%Admin"sv, IsAdmin);
	replace_icase(ExpandedStr, L"%PID"sv, Pid);

	inplace::trim_right(ExpandedStr);

	return ExpandedStr;
}

static string& user_title()
{
	static string s_UserTitle;
	return s_UserTitle;
}

static string& far_title()
{
	static string s_FarTitle;
	return s_FarTitle;
}

static string bake_title(string_view const NewTitle)
{
	if (user_title().empty())
		return concat(NewTitle, L" - Far "sv, expand_title_variables(Global->Opt->strTitleAddons));

	auto CustomizedTitle = expand_title_variables(user_title());
	replace_icase(CustomizedTitle, L"%Default"sv, NewTitle);
	return CustomizedTitle;
}

void ConsoleTitle::SetUserTitle(string_view const Title)
{
	user_title() = Title;
}

static os::critical_section TitleCS;

void ConsoleTitle::SetFarTitle(string_view const Title, bool Flush)
{
	SCOPED_ACTION(std::scoped_lock)(TitleCS);

	far_title() = Title;
	Global->ScrBuf->SetTitle(bake_title(Title));

	if (Flush)
	{
		Global->ScrBuf->Flush(flush_type::title);
	}
}

const string& ConsoleTitle::GetTitle()
{
	return far_title();
}
