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

// Self:
#include "constitle.hpp"

// Internal:
#include "lang.hpp"
#include "config.hpp"
#include "farversion.hpp"
#include "scrbuf.hpp"
#include "strmix.hpp"
#include "global.hpp"

// Platform:
#include "platform.concurrency.hpp"
#include "platform.env.hpp"
#include "platform.security.hpp"

// Common:

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

static const string& GetFarTitleAddons()
{
	// " - Far%Ver%Admin"
	/*
		%Ver      - 2.0
		%Build    - 1259
		%Platform - x86
		%Admin    - MFarTitleAddonsAdmin
		%PID      - current PID
	*/
	static string strTitleAddons;

	strTitleAddons = concat(L" - Far "sv, os::env::expand(Global->Opt->strTitleAddons));

	static const auto Version = build::version();
	static const string strVer = concat(str(Version.Major), L'.', str(Version.Minor));
	static const string strBuild = str(Version.Build);
	static const string strPID = str(GetCurrentProcessId());

	replace_icase(strTitleAddons, L"%PID"sv, strPID);
	replace_icase(strTitleAddons, L"%Ver"sv, strVer);
	replace_icase(strTitleAddons, L"%Build"sv, strBuild);
	replace_icase(strTitleAddons,L"%Platform"sv,
#if defined (_M_X64)
	L"x64"sv
#elif defined (_M_IX86)
	L"x86"sv
#else
	L"Unknown"sv
#endif
	);

	replace_icase(strTitleAddons, L"%Admin"sv, os::security::is_admin()? msg(lng::MFarTitleAddonsAdmin) : L""sv);
	inplace::trim_right(strTitleAddons);

	return strTitleAddons;
}

static string& UserTitle()
{
	static string str;
	return str;
}

static string& FarTitle()
{
	static string strFarTitle;
	return strFarTitle;
}

void ConsoleTitle::SetUserTitle(string_view const Title)
{
	UserTitle() = Title;
}

static os::critical_section TitleCS;

void ConsoleTitle::SetFarTitle(string_view const Title, bool Flush)
{
	SCOPED_ACTION(std::lock_guard)(TitleCS);

	FarTitle() = Title;
	Global->ScrBuf->SetTitle(UserTitle().empty()? FarTitle() + GetFarTitleAddons() : UserTitle());
	if (Flush)
	{
		Global->ScrBuf->Flush(flush_type::title);
	}
}

const string& ConsoleTitle::GetTitle()
{
	return FarTitle();
}
