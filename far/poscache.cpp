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

#include "headers.hpp"
#pragma hdrstop

#include "poscache.hpp"
#include "config.hpp"
#include "configdb.hpp"

void GetFullName(const wchar_t * Name, string &strFullName)
{
	if (*Name==L'<')
		strFullName = Name;
	else
		ConvertNameToFull(Name,strFullName);
}

void FilePositionCache::CompactHistory()
{
	HistoryCfg->BeginTransaction();

	HistoryCfg->DeleteOldPositions(90,1000);

	HistoryCfg->EndTransaction();
}

bool FilePositionCache::AddPosition(const wchar_t *Name, const EditorPosCache& poscache)
{
	if (!Opt.EdOpt.SavePos)
		return false;

	string strFullName;
	GetFullName(Name,strFullName);

	HistoryCfg->BeginTransaction();

	bool ret = false;
	unsigned __int64 id = HistoryCfg->SetEditorPos(strFullName, poscache.Line, poscache.LinePos, poscache.ScreenLine, poscache.LeftPos, poscache.CodePage);

	if (id)
	{
		if (Opt.EdOpt.SaveShortPos)
		{
			for (int i=0; i<BOOKMARK_COUNT; i++)
			{
				if (poscache.bm.Line[i] != POS_NONE)
					HistoryCfg->SetEditorBookmark(id, i, poscache.bm.Line[i], poscache.bm.LinePos[i], poscache.bm.ScreenLine[i], poscache.bm.LeftPos[i]);
			}
		}
		ret = true;
	}

	HistoryCfg->EndTransaction();

	return ret;
}

bool FilePositionCache::GetPosition(const wchar_t *Name, EditorPosCache& poscache)
{
	poscache.Clear();

	if (!Opt.EdOpt.SavePos)
		return false;

	string strFullName;
	GetFullName(Name,strFullName);

	unsigned __int64 id = HistoryCfg->GetEditorPos(strFullName, &poscache.Line, &poscache.LinePos, &poscache.ScreenLine, &poscache.LeftPos, &poscache.CodePage);

	if (id)
	{
		if (!Opt.EdOpt.SaveShortPos)
			return true;

		for (int i=0; i<BOOKMARK_COUNT; i++)
		{
			HistoryCfg->GetEditorBookmark(id, i, poscache.bm.Line+i, poscache.bm.LinePos+i, poscache.bm.ScreenLine+i, poscache.bm.LeftPos+i);
		}

		return true;
	}

	return false;
}

bool FilePositionCache::AddPosition(const wchar_t *Name, const ViewerPosCache& poscache)
{
	if (!Opt.ViOpt.SavePos && !Opt.ViOpt.SaveCodepage && !Opt.ViOpt.SaveWrapMode)
		return false;

	string strFullName;
	GetFullName(Name,strFullName);

	HistoryCfg->BeginTransaction();

	bool ret = false;
	unsigned __int64 id = HistoryCfg->SetViewerPos(strFullName, poscache.FilePos, poscache.LeftPos, poscache.Hex_Wrap, poscache.CodePage);

	if (id)
	{
		if (Opt.ViOpt.SavePos && Opt.ViOpt.SaveShortPos)
		{
			for (int i=0; i<BOOKMARK_COUNT; i++)
			{
				if (poscache.bm.FilePos[i] != POS_NONE)
					HistoryCfg->SetViewerBookmark(id, i, poscache.bm.FilePos[i], poscache.bm.LeftPos[i]);
			}
		}
		ret = true;
	}

	HistoryCfg->EndTransaction();

	return ret;
}

bool FilePositionCache::GetPosition(const wchar_t *Name, ViewerPosCache& poscache)
{
	poscache.Clear();

	if (!Opt.ViOpt.SavePos && !Opt.ViOpt.SaveCodepage && !Opt.ViOpt.SaveWrapMode)
		return false;

	string strFullName;
	GetFullName(Name,strFullName);

	unsigned __int64 id = HistoryCfg->GetViewerPos(strFullName, &poscache.FilePos, &poscache.LeftPos, &poscache.Hex_Wrap, &poscache.CodePage);

	if (id)
	{
		if (!Opt.ViOpt.SavePos || !Opt.ViOpt.SaveShortPos)
			return true;

		for (int i=0; i<BOOKMARK_COUNT; i++)
		{
			HistoryCfg->GetViewerBookmark(id, i, poscache.bm.FilePos+i, poscache.bm.LeftPos+i);
		}

		return true;
	}

	return false;
}
