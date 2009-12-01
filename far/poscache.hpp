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



#define MAX_POSITIONS 64

// Количество закладок в редакторе/вьювере на одну позицию
#define BOOKMARK_COUNT   10

enum
{
	FPOSCACHE_32,
	FPOSCACHE_64,
};

struct TPosCache32
{
	DWORD Param[5];
	DWORD *Position[4];
};

struct TPosCache64
{
	__int64 Param[5];
	__int64 *Position[4];
};

class FilePositionCache
{
	private:
		int IsMemory;
		string *Names;
		int SizeValue;
		int CurPos;

		BYTE *Param;
		BYTE *Position;

	private:
		int FindPosition(const wchar_t *FullName);

	public:
		FilePositionCache(int TypeCache);
		~FilePositionCache();

	public:
		void AddPosition(const wchar_t *Name,void *PosCache);
		BOOL GetPosition(const wchar_t *Name,void *PosCache);

		BOOL Read(const wchar_t *Key);
		BOOL Save(const wchar_t *Key);
};
