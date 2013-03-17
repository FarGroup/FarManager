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
	Global->Db->HistoryCfg()->BeginTransaction();

	Global->Db->HistoryCfg()->DeleteOldPositions(90,1000);

	Global->Db->HistoryCfg()->EndTransaction();
}

bool FilePositionCache::AddPosition(const wchar_t *Name, const EditorPosCache& poscache)
{
	if (!Global->Opt->EdOpt.SavePos)
		return false;

	string strFullName;
	GetFullName(Name,strFullName);

	Global->Db->HistoryCfg()->BeginTransaction();

	bool ret = false;
	unsigned __int64 id = Global->Db->HistoryCfg()->SetEditorPos(strFullName, poscache.cur.Line, poscache.cur.LinePos, poscache.cur.ScreenLine, poscache.cur.LeftPos, poscache.CodePage);

	if (id)
	{
		if (Global->Opt->EdOpt.SaveShortPos)
		{
			int index = 0;
			std::for_each(CONST_RANGE(poscache.bm, i)
			{
				if (i.Line != POS_NONE)
					Global->Db->HistoryCfg()->SetEditorBookmark(id, index, i.Line, i.LinePos, i.ScreenLine, i.LeftPos);
				++index;
			});
		}
		ret = true;
	}

	Global->Db->HistoryCfg()->EndTransaction();

	return ret;
}

bool FilePositionCache::GetPosition(const wchar_t *Name, EditorPosCache& poscache)
{
	poscache.Clear();

	if (!Global->Opt->EdOpt.SavePos)
		return false;

	string strFullName;
	GetFullName(Name,strFullName);

	unsigned __int64 id = Global->Db->HistoryCfg()->GetEditorPos(strFullName, &poscache.cur.Line, &poscache.cur.LinePos, &poscache.cur.ScreenLine, &poscache.cur.LeftPos, &poscache.CodePage);

	if (id)
	{
		if (!Global->Opt->EdOpt.SaveShortPos)
			return true;

		int index = 0;
		std::for_each(RANGE(poscache.bm, i)
		{
			Global->Db->HistoryCfg()->GetEditorBookmark(id, index, &i.Line, &i.LinePos, &i.ScreenLine, &i.LeftPos);
			++index;
		});

		return true;
	}

	return false;
}

bool FilePositionCache::AddPosition(const wchar_t *Name, const ViewerPosCache& poscache)
{
	if (!Global->Opt->ViOpt.SavePos && !Global->Opt->ViOpt.SaveCodepage && !Global->Opt->ViOpt.SaveWrapMode)
		return false;

	string strFullName;
	GetFullName(Name,strFullName);

	Global->Db->HistoryCfg()->BeginTransaction();

	bool ret = false;
	unsigned __int64 id = Global->Db->HistoryCfg()->SetViewerPos(strFullName, poscache.cur.FilePos, poscache.cur.LeftPos, poscache.Hex_Wrap, poscache.CodePage);

	if (id)
	{
		if (Global->Opt->ViOpt.SavePos && Global->Opt->ViOpt.SaveShortPos)
		{
			int index = 0;
			std::for_each(CONST_RANGE(poscache.bm, i)
			{
				if (i.FilePos != POS_NONE)
					Global->Db->HistoryCfg()->SetViewerBookmark(id, index, i.FilePos, i.LeftPos);
				++index;
			});
		}
		ret = true;
	}

	Global->Db->HistoryCfg()->EndTransaction();

	return ret;
}

bool FilePositionCache::GetPosition(const wchar_t *Name, ViewerPosCache& poscache)
{
	poscache.Clear();

	if (!Global->Opt->ViOpt.SavePos && !Global->Opt->ViOpt.SaveCodepage && !Global->Opt->ViOpt.SaveWrapMode)
		return false;

	string strFullName;
	GetFullName(Name,strFullName);

	unsigned __int64 id = Global->Db->HistoryCfg()->GetViewerPos(strFullName, &poscache.cur.FilePos, &poscache.cur.LeftPos, &poscache.Hex_Wrap, &poscache.CodePage);

	if (id)
	{
		if (!Global->Opt->ViOpt.SavePos || !Global->Opt->ViOpt.SaveShortPos)
			return true;

		int index = 0;
		std::for_each(RANGE(poscache.bm, i)
		{
			Global->Db->HistoryCfg()->GetViewerBookmark(id, index, &i.FilePos, &i.LeftPos);
			++index;
		});

		return true;
	}

	return false;
}
