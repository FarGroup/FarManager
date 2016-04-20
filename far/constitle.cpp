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

#include "headers.hpp"
#pragma hdrstop

#include "constitle.hpp"
#include "language.hpp"
#include "config.hpp"
#include "synchro.hpp"
#include "console.hpp"
#include "farversion.hpp"
#include "scrbuf.hpp"
#include "strmix.hpp"

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

	strTitleAddons.assign(L" - Far ",7);
	strTitleAddons += Global->Opt->strTitleAddons.Get();

	static const string strVer = std::to_wstring(FAR_VERSION.Major) + L"." + std::to_wstring(FAR_VERSION.Minor);
	static const string strBuild = std::to_wstring(FAR_VERSION.Build);
	static const string strPID = std::to_wstring(GetCurrentProcessId());

	ReplaceStrings(strTitleAddons, L"%PID", strPID, true);
	ReplaceStrings(strTitleAddons, L"%Ver", strVer, true);
	ReplaceStrings(strTitleAddons, L"%Build", strBuild, true);
	ReplaceStrings(strTitleAddons,L"%Platform",
#ifdef _WIN64
#ifdef _M_IA64
	L"IA64",
#else
	L"x64",
#endif
#else
	L"x86",
#endif
	true);
	ReplaceStrings(strTitleAddons, L"%Admin", os::security::is_admin() ? MSG(MFarTitleAddonsAdmin) : L"", true);
	RemoveTrailingSpaces(strTitleAddons);

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

void ConsoleTitle::SetUserTitle(const string& Title)
{
	UserTitle() = Title;
}

CriticalSection TitleCS;

void ConsoleTitle::SetFarTitle(const string& Title)
{
	SCOPED_ACTION(CriticalSectionLock)(TitleCS);

	FarTitle() = Title;

	if (!Global->ScrBuf->GetLockCount())
	{
		Global->ScrBuf->SetTitle(UserTitle().empty() ? FarTitle() + GetFarTitleAddons() : UserTitle());
	}
}

const string& ConsoleTitle::GetTitle()
{
	return FarTitle();
}
