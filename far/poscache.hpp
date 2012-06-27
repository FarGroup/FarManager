#pragma once

/*
poscache.hpp

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

const int POS_NONE = -1;

// Количество закладок в редакторе/вьювере на одну позицию
const int BOOKMARK_COUNT = 10;

struct EditorBookmark
{
	int Line[BOOKMARK_COUNT];
	int LinePos[BOOKMARK_COUNT];
	int ScreenLine[BOOKMARK_COUNT];
	int LeftPos[BOOKMARK_COUNT];

	void Clear() { memset(this, POS_NONE, sizeof(*this)); }
};

struct EditorPosCache
{
	int Line;
	int LinePos;
	int ScreenLine;
	int LeftPos;
	UINT CodePage;

	EditorBookmark bm;

	void Clear() { Line=LinePos=ScreenLine=LeftPos=0; CodePage=0; bm.Clear(); }
};

struct ViewerBookmark
{
	__int64 FilePos[BOOKMARK_COUNT];
	__int64 LeftPos[BOOKMARK_COUNT];

	void Clear() { memset(this, POS_NONE, sizeof(*this)); }
};

struct ViewerPosCache
{
	__int64 FilePos;
	__int64 LeftPos;
	int Hex_Wrap;
	UINT CodePage;

	ViewerBookmark bm;

	void Clear() { FilePos=LeftPos=0; Hex_Wrap=0; CodePage=0; bm.Clear(); }
};

class FilePositionCache
{
	public:
		static bool AddPosition(const wchar_t *Name, const EditorPosCache& poscache);
		static bool GetPosition(const wchar_t *Name, EditorPosCache& poscache);

		static bool AddPosition(const wchar_t *Name, const ViewerPosCache& poscache);
		static bool GetPosition(const wchar_t *Name, ViewerPosCache& poscache);

		static void CompactHistory();
};
