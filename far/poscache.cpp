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
#include "cvtname.hpp"

void GetFullName(const string& Name, string &strFullName)
{
	strFullName = Name[0] == L'<'? Name : ConvertNameToFull(Name);
}

void FilePositionCache::CompactHistory()
{
	SCOPED_ACTION(auto)(ConfigProvider().HistoryCfg()->ScopedTransaction());
	ConfigProvider().HistoryCfg()->DeleteOldPositions(90,1000);
}

bool FilePositionCache::AddPosition(const string& Name, const EditorPosCache& poscache)
{
	if (!(Global->Opt->EdOpt.SavePos || Global->Opt->EdOpt.SaveShortPos))
		return false;

	string strFullName;
	GetFullName(Name,strFullName);

	SCOPED_ACTION(auto)(ConfigProvider().HistoryCfg()->ScopedTransaction());

	bool ret = false;

	unsigned long long id = 0;

	if (Global->Opt->EdOpt.SavePos)
		id=ConfigProvider().HistoryCfg()->SetEditorPos(strFullName, poscache.cur.Line, poscache.cur.LinePos, poscache.cur.ScreenLine, poscache.cur.LeftPos, poscache.CodePage);
	else if (Global->Opt->EdOpt.SaveShortPos)
		id=ConfigProvider().HistoryCfg()->SetEditorPos(strFullName, 0, 0, 0, 0, 0);

	if (id)
	{
		if (Global->Opt->EdOpt.SaveShortPos)
		{
			for_each_cnt(CONST_RANGE(poscache.bm, i, size_t index)
			{
				if (i.Line != POS_NONE)
					ConfigProvider().HistoryCfg()->SetEditorBookmark(id, index, i.Line, i.LinePos, i.ScreenLine, i.LeftPos);
			});
		}
		ret = true;
	}

	return ret;
}

bool FilePositionCache::GetPosition(const string& Name, EditorPosCache& poscache)
{
	poscache.Clear();

	string strFullName;
	GetFullName(Name,strFullName);

	unsigned long long id = 0;

	if (Global->Opt->EdOpt.SavePos || Global->Opt->EdOpt.SaveShortPos)
		id = ConfigProvider().HistoryCfg()->GetEditorPos(strFullName, &poscache.cur.Line, &poscache.cur.LinePos, &poscache.cur.ScreenLine, &poscache.cur.LeftPos, &poscache.CodePage);

	if (!Global->Opt->EdOpt.SavePos)
	{
		poscache.Clear();
	}

	if (id)
	{
		if (!Global->Opt->EdOpt.SaveShortPos)
			return true;

		for_each_cnt(RANGE(poscache.bm, i, size_t index)
		{
			ConfigProvider().HistoryCfg()->GetEditorBookmark(id, index, &i.Line, &i.LinePos, &i.ScreenLine, &i.LeftPos);
		});

		return true;
	}

	return false;
}

bool FilePositionCache::AddPosition(const string& Name, const ViewerPosCache& poscache)
{
	if (!(Global->Opt->ViOpt.SavePos || Global->Opt->ViOpt.SaveCodepage || Global->Opt->ViOpt.SaveWrapMode || Global->Opt->ViOpt.SaveShortPos))
		return false;

	string strFullName;
	GetFullName(Name,strFullName);

	SCOPED_ACTION(auto)(ConfigProvider().HistoryCfg()->ScopedTransaction());

	bool ret = false;
	unsigned long long id = 0;

	if (Global->Opt->ViOpt.SavePos || Global->Opt->ViOpt.SaveCodepage || Global->Opt->ViOpt.SaveWrapMode)
		id=ConfigProvider().HistoryCfg()->SetViewerPos(strFullName,
				Global->Opt->ViOpt.SavePos?poscache.cur.FilePos:0,
				Global->Opt->ViOpt.SavePos?poscache.cur.LeftPos:0,
				Global->Opt->ViOpt.SaveWrapMode?poscache.ViewModeAndWrapState:0,
				Global->Opt->ViOpt.SaveCodepage?poscache.CodePage:0);
	else if (Global->Opt->ViOpt.SaveShortPos)
		id=ConfigProvider().HistoryCfg()->SetViewerPos(strFullName, 0, 0, 0, 0);

	if (id)
	{
		if (Global->Opt->ViOpt.SaveShortPos)
		{
			for_each_cnt(CONST_RANGE(poscache.bm, i, size_t index)
			{
				if (i.FilePos != POS_NONE)
					ConfigProvider().HistoryCfg()->SetViewerBookmark(id, index, i.FilePos, i.LeftPos);
			});
		}
		ret = true;
	}

	return ret;
}

bool FilePositionCache::GetPosition(const string& Name, ViewerPosCache& poscache)
{
	poscache.Clear();

	string strFullName;
	GetFullName(Name,strFullName);

	unsigned long long id = 0;

	if (Global->Opt->ViOpt.SavePos || Global->Opt->ViOpt.SaveCodepage || Global->Opt->ViOpt.SaveWrapMode || Global->Opt->ViOpt.SaveShortPos)
		id = ConfigProvider().HistoryCfg()->GetViewerPos(strFullName, &poscache.cur.FilePos, &poscache.cur.LeftPos, &poscache.ViewModeAndWrapState, &poscache.CodePage);

	if (!Global->Opt->ViOpt.SavePos && !Global->Opt->ViOpt.SaveCodepage && !Global->Opt->ViOpt.SaveWrapMode)
	{
		poscache.Clear();
	}

	if (id)
	{
		if (!Global->Opt->ViOpt.SaveShortPos)
			return true;

		for_each_cnt(RANGE(poscache.bm, i, size_t index)
		{
			ConfigProvider().HistoryCfg()->GetViewerBookmark(id, index, &i.FilePos, &i.LeftPos);
		});

		return true;
	}

	return false;
}
