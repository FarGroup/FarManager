/*
poscache.cpp

Кэш позиций в файлах для viewer/editor
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
#include "poscache.hpp"

// Internal:
#include "config.hpp"
#include "configdb.hpp"
#include "cvtname.hpp"
#include "global.hpp"

// Platform:

// Common:
#include "common/view/enumerate.hpp"

// External:

//----------------------------------------------------------------------------

static auto GetFullName(string_view const Name)
{
	return Name[0] == L'<'? string(Name) : ConvertNameToFull(Name);
}

void FilePositionCache::CompactHistory()
{
	SCOPED_ACTION(auto)(ConfigProvider().HistoryCfg()->ScopedTransaction());
	ConfigProvider().HistoryCfg()->DeleteOldPositions(Global->Opt->ViewHistoryLifetime, Global->Opt->ViewHistoryCount);
}

bool FilePositionCache::AddPosition(string_view const Name, const EditorPosCache& poscache)
{
	if (!(Global->Opt->EdOpt.SavePos || Global->Opt->EdOpt.SaveShortPos))
		return false;

	const auto Time = os::chrono::nt_clock::now();
	const auto strFullName = GetFullName(Name);

	SCOPED_ACTION(auto)(ConfigProvider().HistoryCfg()->ScopedTransaction());

	unsigned long long id;

	if (Global->Opt->EdOpt.SavePos)
		id=ConfigProvider().HistoryCfg()->SetEditorPos(strFullName, Time, poscache.cur.Line, poscache.cur.LinePos, poscache.cur.ScreenLine, poscache.cur.LeftPos, poscache.CodePage);
	else if (Global->Opt->EdOpt.SaveShortPos)
		id=ConfigProvider().HistoryCfg()->SetEditorPos(strFullName, Time, 0, 0, 0, 0, 0);
	else
		return false;

	if (Global->Opt->EdOpt.SaveShortPos)
	{
		for (const auto& [i, index]: enumerate(poscache.bm))
		{
			if (i.Line != POS_NONE)
				ConfigProvider().HistoryCfg()->SetEditorBookmark(id, index, i.Line, i.LinePos, i.ScreenLine, i.LeftPos);
		}
	}

	return true;
}

bool FilePositionCache::GetPosition(string_view const Name, EditorPosCache& poscache)
{
	poscache.Clear();

	const auto strFullName = GetFullName(Name);

	unsigned long long id = 0;

	if (Global->Opt->EdOpt.SavePos || Global->Opt->EdOpt.SaveShortPos)
		id = ConfigProvider().HistoryCfg()->GetEditorPos(strFullName, poscache.cur.Line, poscache.cur.LinePos, poscache.cur.ScreenLine, poscache.cur.LeftPos, poscache.CodePage);

	if (!Global->Opt->EdOpt.SavePos)
	{
		poscache.Clear();
	}

	if (id)
	{
		if (!Global->Opt->EdOpt.SaveShortPos)
			return true;

		for (const auto& [i, index]: enumerate(poscache.bm))
		{
			ConfigProvider().HistoryCfg()->GetEditorBookmark(id, index, i.Line, i.LinePos, i.ScreenLine, i.LeftPos);
		}

		return true;
	}

	return false;
}

bool FilePositionCache::AddPosition(string_view const Name, const ViewerPosCache& poscache)
{
	if (!(Global->Opt->ViOpt.SavePos || Global->Opt->ViOpt.SaveCodepage || Global->Opt->ViOpt.SaveWrapMode || Global->Opt->ViOpt.SaveShortPos))
		return false;

	const auto Time = os::chrono::nt_clock::now();
	const auto strFullName = GetFullName(Name);

	SCOPED_ACTION(auto)(ConfigProvider().HistoryCfg()->ScopedTransaction());

	bool ret = false;
	unsigned long long id = 0;
	const auto& vo = Global->Opt->ViOpt;

	if (vo.SavePos || vo.SaveCodepage || vo.SaveViewMode || vo.SaveWrapMode)
		id=ConfigProvider().HistoryCfg()->SetViewerPos(strFullName, Time, poscache.cur.FilePos, poscache.cur.LeftPos, poscache.ViewModeAndWrapState, poscache.CodePage);
	else if (vo.SaveShortPos)
		id=ConfigProvider().HistoryCfg()->SetViewerPos(strFullName, Time, 0, 0, 0, 0);

	if (id)
	{
		if (vo.SaveShortPos)
		{
			for (const auto& [i, index]: enumerate(poscache.bm))
			{
				if (i.FilePos != POS_NONE)
					ConfigProvider().HistoryCfg()->SetViewerBookmark(id, index, i.FilePos, i.LeftPos);
			}
		}
		ret = true;
	}

	return ret;
}

bool FilePositionCache::GetPosition(string_view const Name, ViewerPosCache& poscache)
{
	poscache.Clear();

	const auto strFullName = GetFullName(Name);

	unsigned long long id = 0;

	if (Global->Opt->ViOpt.SavePos || Global->Opt->ViOpt.SaveCodepage || Global->Opt->ViOpt.SaveWrapMode || Global->Opt->ViOpt.SaveShortPos)
		id = ConfigProvider().HistoryCfg()->GetViewerPos(strFullName, poscache.cur.FilePos, poscache.cur.LeftPos, poscache.ViewModeAndWrapState, poscache.CodePage);

	if (!Global->Opt->ViOpt.SavePos && !Global->Opt->ViOpt.SaveCodepage && !Global->Opt->ViOpt.SaveWrapMode)
	{
		poscache.Clear();
	}

	if (id)
	{
		if (!Global->Opt->ViOpt.SaveShortPos)
			return true;

		for (const auto& [i, index]: enumerate(poscache.bm))
		{
			ConfigProvider().HistoryCfg()->GetViewerBookmark(id, index, i.FilePos, i.LeftPos);
		}

		return true;
	}

	return false;
}
