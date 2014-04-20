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

class GetFileString: NonCopyable
{
public:
	GetFileString(api::File& SrcFile, uintptr_t CodePage);
	bool PeekString(LPWSTR* DestStr, size_t& Length);
	bool GetString(LPWSTR* DestStr, size_t& Length);
	bool GetString(string& str);
	bool IsConversionValid() const { return !SomeDataLost; }

private:
	template<class T>
	bool GetTString(std::vector<T>& From, std::vector<T>& To, bool bBigEndian = false);

	api::File& SrcFile;
	uintptr_t m_CodePage;
	DWORD ReadPos, ReadSize;

	bool Peek;
	size_t LastLength;
	LPWSTR LastString;
	bool LastResult;

	std::vector<char> m_ReadBuf;
	std::vector<wchar_t> m_wReadBuf;
	std::vector<wchar_t> m_wStr;

	bool SomeDataLost;
	bool bCrCr;
};

bool GetFileFormat(api::File& file, uintptr_t& nCodePage, bool* pSignatureFound = nullptr, bool bUseHeuristics = true);
