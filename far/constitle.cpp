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
#include "interf.hpp"
#include "config.hpp"
#include "ctrlobj.hpp"
#include "CriticalSections.hpp"
#include "console.hpp"
#include "farversion.hpp"
#include "scrbuf.hpp"

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
	static FormatString strVer, strBuild, strPID;
	static bool bFirstRun = true;
	static string strTitleAddons;

	strTitleAddons.Copy(L" - Far ",7);
	strTitleAddons += Opt.strTitleAddons.Get();

	if (bFirstRun)
	{
		bFirstRun = false;
		strVer<<FAR_VERSION.Major<<L"."<<FAR_VERSION.Minor;
		strBuild<<FAR_VERSION.Build;
		strPID<<GetCurrentProcessId();
	}

	ReplaceStrings(strTitleAddons,L"%PID",strPID,-1,true);
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

bool ConsoleTitle::TitleModified = false;
DWORD ConsoleTitle::ShowTime = 0;

CriticalSection TitleCS;

ConsoleTitle::ConsoleTitle(const wchar_t *title)
{
	CriticalSectionLock Lock(TitleCS);
	Console.GetTitle(strOldTitle);

	if (title)
		SetFarTitle(title, true);
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

	SetFarTitle(strOldTitle, true);
}

BaseFormat& ConsoleTitle::Flush()
{
	SetFarTitle(*this);
	Clear();
	return *this;
}

void ConsoleTitle::SetFarTitle(const wchar_t *Title, bool Force)
{
	CriticalSectionLock Lock(TitleCS);
	static string strFarTitle;
	string strOldFarTitle;

	if (Title)
	{
		Console.GetTitle(strOldFarTitle);
		strFarTitle=Title;
		strFarTitle.SetLength(0x100);
		strFarTitle+=GetFarTitleAddons();
		TitleModified=true;

		if (StrCmp(strOldFarTitle, strFarTitle) &&
		        /*((CtrlObject->Macro.IsExecuting() && !CtrlObject->Macro.IsDsableOutput()) ||
		         !CtrlObject->Macro.IsExecuting() || CtrlObject->Macro.IsExecutingLastKey())*/ ScrBuf.GetLockCount()==0)
		{
			DWORD CurTime=GetTickCount();
			if(CurTime-ShowTime>(DWORD)Opt.RedrawTimeout || Force)
			{
				ShowTime=CurTime;
				Console.SetTitle(strFarTitle);
				TitleModified=true;
			}
		}
	}
	else if(ScrBuf.GetLockCount()==0)
	{
		/*
			Title=nullptr для случая, когда нужно выставить пред.заголовок
			SetFarTitle(nullptr) - это не для всех!
			Этот вызов имеет право делать только макро-движок!
		*/
		Console.SetTitle(strFarTitle);
		TitleModified=false;
		//_SVS(SysLog(L"  (nullptr)FarTitle='%s'",FarTitle));
	}
}
