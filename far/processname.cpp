/*
processname.cpp

Обработать имя файла: сравнить с маской, масками, сгенерировать по маске
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

#include "processname.hpp"
#include "pathmix.hpp"
#include "string_utils.hpp"

/* $ 09.10.2000 IS
    Генерация нового имени по маске
    (взял из ShellCopy::ShellCopyConvertWildcards)
*/
// На основе имени файла (Src) и маски (Dest) генерируем новое имя
// SelectedFolderNameLength - длина каталога. Например, есть
// каталог dir1, а в нем файл file1. Нужно сгенерировать имя по маске для dir1.
// Параметры могут быть следующими: Src="dir1", SelectedFolderNameLength=0
// или Src="dir1\\file1", а SelectedFolderNameLength=4 (длина "dir1")
bool ConvertWildcards(const string& SrcName, string &strDest, int const SelectedFolderNameLength)
{
	size_t DestNamePos = strDest.size() - PointToName(strDest).size();

	if (strDest.find_first_of(L"*?", DestNamePos) == string::npos)
	{
		return false;
	}

	const string strWildName = strDest.substr(DestNamePos);

	const auto strSrc = SelectedFolderNameLength? SrcName.substr(0, SelectedFolderNameLength) : SrcName;
	const auto Src = strSrc.c_str();
	auto SrcNamePtr = PointToName(strSrc);

	strDest.resize(strDest.size() + SrcName.size());

	const auto BeforeNameLength = DestNamePos? 0 : strSrc.size() - SrcNamePtr.size();

	const auto SrcNameRDot = std::find(ALL_CONST_REVERSE_RANGE(SrcNamePtr), L'.');
	const auto SrcNameDot = SrcNameRDot == SrcNamePtr.crend()? SrcNamePtr.cend() : (SrcNameRDot + 1).base();

	auto CurWildPtr = strWildName.c_str();
	while (*CurWildPtr)
	{
		switch (*CurWildPtr)
		{
		case L'?':
			CurWildPtr++;

			if (!SrcNamePtr.empty())
			{
				strDest[DestNamePos] = SrcNamePtr[0];
				++DestNamePos;
				SrcNamePtr.remove_prefix(1);
			}
			break;

		case L'*':
			CurWildPtr++;
			while (!SrcNamePtr.empty())
			{
				if (*CurWildPtr==L'.' && SrcNameDot && !wcschr(CurWildPtr+1,L'.'))
				{
					if (SrcNamePtr.cbegin() == SrcNameDot)
						break;
				}
				else if (SrcNamePtr[0] == *CurWildPtr)
				{
					break;
				}

				strDest[DestNamePos] = SrcNamePtr[0];
				++DestNamePos;
				SrcNamePtr.remove_prefix(1);
			}
			break;

		case L'.':
			CurWildPtr++;
			strDest[DestNamePos++] = L'.';

			if (wcspbrk(CurWildPtr,L"*?"))
				while (!SrcNamePtr.empty())
				{
					const auto Char = SrcNamePtr[0];
					SrcNamePtr.remove_prefix(1);
					if (Char == L'.')
						break;
				}
			break;

		default:
			strDest[DestNamePos++] = *(CurWildPtr++);
			if (!SrcNamePtr.empty() && SrcNamePtr[0] != L'.')
				SrcNamePtr.remove_prefix(1);
			break;
		}
	}
	strDest.resize(DestNamePos);

	if (!strDest.empty() && strDest.back() == L'.')
		strDest.pop_back();

	strDest.insert(0, Src, BeforeNameLength);

	if (SelectedFolderNameLength)
	{
		strDest += SrcName.substr(SelectedFolderNameLength); //BUGBUG???, was src in 1.7x
	}

	return true;
}

bool CmpName(string_view pattern, string_view str, const bool skippath, const bool CmpNameSearchMode)
{
	if (pattern.empty() || str.empty())
		return false;

	if (skippath)
		str = PointToName(str);

	for (;; str.remove_prefix(1))
	{
		if (pattern.empty())
			return str.empty();

		const auto stringc = str.empty()? 0 : upper(str.front());
		const auto patternc = upper(pattern.front());
		pattern.remove_prefix(1);

		switch (patternc)
		{
		case L'?':
			if (str.empty())
				return false;
			break;

		case L'*':
			if (pattern.empty())
				return true;

			/* $ 01.05.2001 DJ
				оптимизированная ветка работает и для имен с несколькими
				точками
			*/
			if (pattern[0] == L'.')
			{
				if (pattern.size() == 2 && pattern[1]==L'*')
					return true;

				if (std::none_of(ALL_CONST_RANGE(pattern), [](wchar_t Char) { return wcschr(L"*?[", Char) != nullptr; }))
				{
					const auto RDotIt = std::find(ALL_CONST_REVERSE_RANGE(str), L'.');
					const auto DotIt = RDotIt == str.crend()? str.cend() : (RDotIt + 1).base();

					if (pattern.size() == 1)
						return DotIt == str.cend() || DotIt + 1 == str.cend();

					const auto PatternContainsDot = contains(pattern.substr(1), L'.');

					if (PatternContainsDot && DotIt == str.cend())
						return false;

					if (!PatternContainsDot && DotIt != str.cend())
						return equal_icase(pattern.substr(1), str.substr(DotIt + 1 - str.cbegin()));
				}
			}

			for(;;)
			{
				if(CmpName(pattern,str,false,CmpNameSearchMode))
					return true;

				if (str.empty())
					break;

				str.remove_prefix(1);
			}

			return false;

		case L'[':
			{
				if (!contains(pattern, L']'))
				{
					if (patternc != stringc)
						return false;

					break;
				}

				if (pattern.size() > 1 && pattern[1] == L']')
				{
					if (str.empty() || pattern[0] != str[0])
						return false;

					pattern.remove_prefix(2);
					break;
				}

				int match = 0;

				for(;;)
				{
					if (pattern.empty())
						return false;

					const auto rangec = upper(pattern[0]);

					pattern.remove_prefix(1);

					if (rangec == L']')
					{
						if (match)
							break;
						else
							return false;
					}

					if (match)
						continue;

					if (rangec == L'-' && *(pattern.cbegin() - 2) != L'[' && pattern[0] != L']')
					{
						match = (stringc <= upper(pattern[0]) &&
									upper(*(pattern.cbegin() - 2)) <= stringc);
						pattern.remove_prefix(1);
					}
					else
						match = (stringc == rangec);
				}
			}
			break;

		default:
			if (patternc != stringc)
			{
				if (patternc==L'.' && !stringc && !CmpNameSearchMode)
					return pattern[0] != L'.' && CmpName(pattern, str, true, CmpNameSearchMode);

				return false;
			}
			break;
		}
	}
}
