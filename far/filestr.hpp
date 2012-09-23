#pragma once

/*
filestr.hpp

 Î‡ÒÒ GetFileString
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

// BUGBUG, delete!
class OldGetFileString
{
	private:
		FILE *SrcFile;
		wchar_t* wReadBuf;
		char* ReadBuf;
		int m_nwStrLength;
		int m_nStrLength;
		wchar_t *wStr;
		char *Str;
		int ReadPos;
		int ReadSize;
		bool SomeDataLost;
		bool bCrCr;

	private:
		int GetAnsiString(char **DestStr, int &Length);
		int GetUnicodeString(wchar_t **DestStr, int &Length, bool bBigEndian);

	public:
		OldGetFileString(FILE *SrcFile);
		~OldGetFileString();

	public:
		int GetString(wchar_t **DestStr, int nCodePage, int &Length);
		bool IsConversionValid() { return !SomeDataLost; };
};

// BUGBUG, delete!
bool OldGetFileFormat(FILE *file, uintptr_t &nCodePage, bool *pSignatureFound = nullptr, bool bUseHeuristics = true);
wchar_t *ReadString(FILE *file, wchar_t *lpwszDest, int nDestLength, int nCodePage);

//-----------------------------------------------------------------------------

class GetFileString
{
	public:
		GetFileString(File& SrcFile);
		~GetFileString();
		int PeekString(LPWSTR* DestStr, uintptr_t nCodePage, int& Length);
		int GetString(LPWSTR* DestStr, uintptr_t nCodePage, int& Length);
		bool IsConversionValid() { return !SomeDataLost; }

	private:
		File& SrcFile;
		DWORD ReadPos, ReadSize;

		bool Peek;
		int LastLength;
		LPWSTR* LastString;
		int LastResult;

		char* ReadBuf;
		wchar_t* wReadBuf;

		int m_nStrLength;
		char *Str;

		int m_nwStrLength;
		wchar_t *wStr;

		bool SomeDataLost;
		bool bCrCr;

		int GetAnsiString(LPSTR* DestStr, int& Length);
		int GetUnicodeString(LPWSTR* DestStr, int& Length, bool bBigEndian);
};

bool GetFileFormat(File& file, uintptr_t& nCodePage, bool* pSignatureFound = nullptr, bool bUseHeuristics = true);