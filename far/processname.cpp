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
#include "strmix.hpp"
#include "pathmix.hpp"

/* $ 09.10.2000 IS
    Генерация нового имени по маске
    (взял из ShellCopy::ShellCopyConvertWildcards)
*/
// На основе имени файла (Src) и маски (Dest) генерируем новое имя
// SelectedFolderNameLength - длина каталога. Например, есть
// каталог dir1, а в нем файл file1. Нужно сгенерировать имя по маске для dir1.
// Параметры могут быть следующими: Src="dir1", SelectedFolderNameLength=0
// или Src="dir1\\file1", а SelectedFolderNameLength=4 (длина "dir1")
int ConvertWildcards(const wchar_t *SrcName, string &strDest, int SelectedFolderNameLength)
{
	string strPartAfterFolderName;
	string strSrc = SrcName;
	wchar_t *DestName = strDest.GetBuffer(strDest.GetLength()+strSrc.GetLength()+1);  //???
	wchar_t *DestNamePtr = (wchar_t*)PointToName(DestName);
	string strWildName = DestNamePtr;

	if (!wcschr(strWildName, L'*') && !wcschr(strWildName, L'?'))
	{
		//strDest.ReleaseBuffer (); не надо так как строка не поменялась
		return FALSE;
	}

	if (SelectedFolderNameLength)
	{
		strPartAfterFolderName = ((const wchar_t *)strSrc+SelectedFolderNameLength);
		strSrc.SetLength(SelectedFolderNameLength);
	}

	const wchar_t *Src = strSrc;

	const wchar_t *SrcNamePtr = PointToName(Src);

	int BeforeNameLength = DestNamePtr==DestName ? (int)(SrcNamePtr-Src) : 0;

	wchar_t *PartBeforeName = (wchar_t*)xf_malloc((BeforeNameLength+1)*sizeof(wchar_t));

	xwcsncpy(PartBeforeName, Src, BeforeNameLength+1);

	const wchar_t *SrcNameDot = wcsrchr(SrcNamePtr, L'.');

	const wchar_t *CurWildPtr = strWildName;

	while (*CurWildPtr)
	{
		switch (*CurWildPtr)
		{
			case L'?':
				CurWildPtr++;

				if (*SrcNamePtr)
					*(DestNamePtr++)=*(SrcNamePtr++);

				break;
			case L'*':
				CurWildPtr++;

				while (*SrcNamePtr)
				{
					if (*CurWildPtr==L'.' && SrcNameDot && !wcschr(CurWildPtr+1,L'.'))
					{
						if (SrcNamePtr==SrcNameDot)
							break;
					}
					else if (*SrcNamePtr==*CurWildPtr)
					{
						break;
					}

					*(DestNamePtr++)=*(SrcNamePtr++);
				}

				break;
			case L'.':
				CurWildPtr++;
				*(DestNamePtr++)=L'.';

				if (wcspbrk(CurWildPtr,L"*?"))
					while (*SrcNamePtr)
						if (*(SrcNamePtr++)==L'.')
							break;

				break;
			default:
				*(DestNamePtr++)=*(CurWildPtr++);

				if (*SrcNamePtr && *SrcNamePtr!=L'.')
					SrcNamePtr++;

				break;
		}
	}

	*DestNamePtr=0;

	if (DestNamePtr!=DestName && *(DestNamePtr-1)==L'.')
		*(DestNamePtr-1)=0;

	strDest.ReleaseBuffer();

	if (*PartBeforeName)
		strDest = PartBeforeName+strDest;

	if (SelectedFolderNameLength)
		strDest += strPartAfterFolderName; //BUGBUG???, was src in 1.7x

	xf_free(PartBeforeName);
	return TRUE;
}


// IS: это реальное тело функции сравнения с маской, но использовать
// IS: "снаружи" нужно не эту функцию, а CmpName (ее тело расположено
// IS: после CmpName_Body)
static int CmpName_Body(const wchar_t *pattern,const wchar_t *str, bool CmpNameSearchMode)
{
	wchar_t stringc,patternc,rangec;
	int match;

	for (;; ++str)
	{
		/* $ 01.05.2001 DJ
		   используем инлайновые версии
		*/
		stringc=Upper(*str);
		patternc=Upper(*pattern++);

		switch (patternc)
		{
			case 0:
				return !stringc;
			case L'?':

				if (!stringc)
					return FALSE;

				break;
			case L'*':

				if (!*pattern)
					return TRUE;

				/* $ 01.05.2001 DJ
				   оптимизированная ветка работает и для имен с несколькими
				   точками
				*/
				if (*pattern==L'.')
				{
					if (pattern[1]==L'*' && !pattern[2])
						return TRUE;

					if (!wcspbrk(pattern, L"*?["))
					{
						const wchar_t *dot = wcsrchr(str, L'.');

						if (!pattern[1])
							return (!dot || !dot[1]);

						const wchar_t *patdot = wcschr(pattern+1, L'.');

						if (patdot  && !dot)
							return FALSE;

						if (!patdot && dot )
							return !StrCmpI(pattern+1,dot+1);
					}
				}

				do
				{
					if(CmpName(pattern,str,false,CmpNameSearchMode))
						return TRUE;
				}
				while (*str++);

				return FALSE;
			case L'[':

				if (!wcschr(pattern,L']'))
				{
					if (patternc != stringc)
						return (FALSE);

					break;
				}

				if (*pattern && *(pattern+1)==L']')
				{
					if (*pattern!=*str)
						return FALSE;

					pattern+=2;
					break;
				}

				match = 0;

				while ((rangec = Upper(*pattern++)))
				{
					if (rangec == L']')
					{
						if (match)
							break;
						else
							return FALSE;
					}

					if (match)
						continue;

					if (rangec == L'-' && *(pattern - 2) != L'[' && *pattern != L']')
					{
						match = (stringc <= Upper(*pattern) &&
						         Upper(*(pattern - 2)) <= stringc);
						pattern++;
					}
					else
						match = (stringc == rangec);
				}

				if (!rangec)
					return FALSE;

				break;
			default:

				if (patternc != stringc)
				{
					if (patternc==L'.' && !stringc && !CmpNameSearchMode)
						return(*pattern!=L'.' && CmpName(pattern,str,true,CmpNameSearchMode));
					else
						return FALSE;
				}

				break;
		}
	}
}

// IS: функция для внешнего мира, использовать ее
int CmpName(const wchar_t *pattern,const wchar_t *str, bool skippath, bool CmpNameSearchMode)
{
	if (!pattern || !str)
		return FALSE;

	if (skippath)
		str=PointToName(str);

	return CmpName_Body(pattern,str,CmpNameSearchMode);
}
