/*
constitle.cpp

«аголовок консоли
*/
/*
Copyright (c) 1996 Eugene Roshal
Copyright (c) 2000 Far Group
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
#include "lang.hpp"
#include "language.hpp"
#include "interf.hpp"
#include "config.hpp"
#include "ctrlobj.hpp"
#include "CriticalSections.hpp"

static const string& GetFarTitleAddons();

bool ConsoleTitle::TitleModified = false;
DWORD ConsoleTitle::ShowTime = 0;

CriticalSection TitleCS;

ConsoleTitle::ConsoleTitle(const wchar_t *title)
{
	CriticalSectionLock Lock(TitleCS);
	apiGetConsoleTitle(strOldTitle);

	if (title)
		ConsoleTitle::SetFarTitle(title);
}

ConsoleTitle::~ConsoleTitle()
{
	CriticalSectionLock Lock(TitleCS);
	const string &strTitleAddons = GetFarTitleAddons();
	size_t OldLen = strOldTitle.GetLength();
	size_t AddonsLen = strTitleAddons.GetLength();

	if (AddonsLen <= OldLen)
	{

		if (!StrCmpI(strOldTitle.CPtr()+OldLen-AddonsLen, strTitleAddons))
			strOldTitle.SetLength(OldLen-AddonsLen);
	}

	ConsoleTitle::SetFarTitle(strOldTitle);
}

void ConsoleTitle::Set(const wchar_t *fmt, ...)
{
	CriticalSectionLock Lock(TitleCS);
	wchar_t msg[2048];
	va_list argptr;
	va_start(argptr, fmt);
	vsnwprintf(msg, countof(msg)-1, fmt, argptr);
	va_end(argptr);
	SetFarTitle(msg);
}

void ConsoleTitle::SetFarTitle(const wchar_t *Title)
{
	CriticalSectionLock Lock(TitleCS);
	static string strFarTitle;
	string strOldFarTitle;

	if (Title)
	{
		apiGetConsoleTitle(strOldFarTitle);
		strFarTitle=Title;
		strFarTitle.SetLength(0x100);
		strFarTitle+=GetFarTitleAddons();
		TitleModified=true;

		if (StrCmp(strOldFarTitle, strFarTitle) &&
		        ((CtrlObject->Macro.IsExecuting() && !CtrlObject->Macro.IsDsableOutput()) ||
		         !CtrlObject->Macro.IsExecuting() || CtrlObject->Macro.IsExecutingLastKey()))
		{
			DWORD CurTime=GetTickCount();
			if(CurTime-ShowTime>RedrawTimeout)
			{
				ShowTime=CurTime;
				SetConsoleTitle(strFarTitle);
				TitleModified=true;
			}
		}
	}
	else
	{
		/*
			Title=nullptr дл€ случа€, когда нужно выставить пред.заголовок
			SetFarTitle(nullptr) - это не дл€ всех!
			Ётот вызов имеет право делать только макро-движок!
		*/
		SetConsoleTitle(strFarTitle);
		TitleModified=false;
		//_SVS(SysLog(L"  (nullptr)FarTitle='%s'",FarTitle));
	}
}

static const string& GetFarTitleAddons()
{
	// " - Far%Ver%Admin"
	/*
		%Ver      - 2.0
		%Build    - 1259
		%Platform - x86
		%Admin    - MFarTitleAddonsAdmin
    */
	static FormatString strVer, strBuild;
	static bool bFirstRun = true;
	static string strTitleAddons;

	strTitleAddons.Copy(L" - Far ",7);
	strTitleAddons += Opt.strTitleAddons;

	if (bFirstRun)
	{
		bFirstRun = false;
		strVer<<HIBYTE(LOWORD(FAR_VERSION))<<L"."<<LOBYTE(LOWORD(FAR_VERSION));
		strBuild<<HIWORD(FAR_VERSION);
	}

	ReplaceStrings(strTitleAddons,L"%Ver",strVer,-1,true);
	ReplaceStrings(strTitleAddons,L"%Build",strBuild,-1,true);
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
	-1,true);
	ReplaceStrings(strTitleAddons,L"%Admin",Opt.IsUserAdmin?MSG(MFarTitleAddonsAdmin):L"",-1,true);
	RemoveTrailingSpaces(strTitleAddons);

	return strTitleAddons;
}
