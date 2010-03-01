#pragma once

/*
poscache.hpp

Кэш позиций в файлах для viewer/editor
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

const DWORD64 POS_NONE=_UI64_MAX;

// Максимальное количество элементов в кэше
#define MAX_POSITIONS 512

// Количество закладок в редакторе/вьювере на одну позицию
#define BOOKMARK_COUNT   10

struct PosCache
{
    /*
    Param:
    	Editor:
			Param[0] = Line
			Param[1] = ScreenLine
			Param[2] = LinePos
			Param[3] = LeftPos
			Param[4] = CodePage or 0
		Viewer:
			Param[0] = FilePos
			Param[1] = LeftPos
			Param[2] = Hex?
			Param[3] = 0
			Param[4] = CodePage
    */
	DWORD64 Param[5];

    /*
    Position
    	Editor:
			Position[0] = [BOOKMARK_COUNT] Line
			Position[1] = [BOOKMARK_COUNT] Cursor
			Position[2] = [BOOKMARK_COUNT] ScreenLine
			Position[3] = [BOOKMARK_COUNT] LeftPos
		Viewer:
			Position[0] = [BOOKMARK_COUNT] SavePosAddr
			Position[1] = [BOOKMARK_COUNT] SavePosLeft
			Position[2] = [BOOKMARK_COUNT] 0
			Position[3] = [BOOKMARK_COUNT] 0
    */
	DWORD64 *Position[4];
};

class FilePositionCache
{
	private:
		int IsMemory;
		int CurPos;

		string *Names;
		BYTE *Param;
		BYTE *Position;

	private:
		int FindPosition(const wchar_t *FullName);

	public:
		FilePositionCache();
		~FilePositionCache();

	public:
		void AddPosition(const wchar_t *Name,PosCache& poscache);
		bool GetPosition(const wchar_t *Name,PosCache& poscache);

		bool Read(const wchar_t *Key);
		bool Save(const wchar_t *Key);
};
