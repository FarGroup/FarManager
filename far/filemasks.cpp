/*
FileMasksWithExclude.cpp

Класс для работы со сложными масками файлов (учитывается наличие масок
исключения).
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

#include "filemasks.hpp"
#include "message.hpp"
#include "language.hpp"
#include "processname.hpp"
#include "configdb.hpp"

const wchar_t EXCLUDEMASKSEPARATOR=L'|';

bool filemasks::Set(const string& masks, DWORD Flags)
{
	bool Result = false;

	if (!masks.IsEmpty())
	{
		Free();
		wchar_t_ptr MasksStr(masks.GetLength()+1);

		wcscpy(MasksStr.get(), masks);
		wchar_t *pExclude = FindExcludeChar(MasksStr.get());

		Result = true;

		if (pExclude)
		{
			*pExclude=0;
			++pExclude;

			if (*pExclude!=L'/' && wcschr(pExclude, EXCLUDEMASKSEPARATOR))
				Result = false;
		}

		if (Result)
		{
			Result = Include.Set(MasksStr[0]? MasksStr.get() : L"*");

			if (Result && pExclude)
				Result = Exclude.Set(pExclude);
		}

		if (!Result)
		{
			if (!(Flags & FMF_SILENT))
			{
				ErrorMessage();
			}
			Free();
		}

	}

	return Result;
}

void filemasks::Free()
{
	Include.Free();
	Exclude.Free();
}

// Путь к файлу в FileName НЕ игнорируется

bool filemasks::Compare(const string& FileName) const
{
	return (Include.Compare(FileName) && !Exclude.Compare(FileName));
}

bool filemasks::IsEmpty() const
{
	return (Include.IsEmpty() && Exclude.IsEmpty());
}

void filemasks::ErrorMessage() const
{
	Message(MSG_WARNING, 1, MSG(MWarning), MSG(MIncorrectMask), MSG(MOk));
}

wchar_t* filemasks::FindExcludeChar(wchar_t* masks) const
{
	wchar_t* pExclude = masks;

	if (*pExclude == L'/')
	{
		pExclude++;

		while (*pExclude && (*pExclude != L'/' || *(pExclude-1) == L'\\'))
			pExclude++;

		while (*pExclude && *pExclude != EXCLUDEMASKSEPARATOR)
			pExclude++;

		if (*pExclude != EXCLUDEMASKSEPARATOR)
			pExclude = nullptr;
	}
	else
	{
		pExclude = wcschr(masks,EXCLUDEMASKSEPARATOR);
	}

	return pExclude;
}

bool filemasks::masks::Set(const string& masks)
{
	Free();

	string expmasks(masks);
	std::list<string> UsedGroups;
	size_t LBPos, RBPos;

	while(expmasks.Pos(LBPos, L'<') && expmasks.Pos(RBPos, L'>', LBPos))
	{
		string MaskGroupNameWB = expmasks.SubStr(LBPos, RBPos-LBPos+1);
		string MaskGroupName = expmasks.SubStr(LBPos+1, RBPos-LBPos-1);
		string MaskGroupValue;
		if (std::find(UsedGroups.cbegin(), UsedGroups.cend(), MaskGroupName) == UsedGroups.cend())
		{
			Global->Db->GeneralCfg()->GetValue(L"Masks", MaskGroupName, MaskGroupValue, L"");
			ReplaceStrings(expmasks, MaskGroupNameWB, MaskGroupValue);
			UsedGroups.emplace_back(MaskGroupName);
		}
	}

	size_t pos;
	const wchar_t* PathExtName = L"%PATHEXT%";
	if (expmasks.PosI(pos, PathExtName))
	{
		string strSysPathExt;
		if (apiGetEnvironmentVariable(L"PATHEXT" ,strSysPathExt))
		{
			auto MaskList(StringToList(strSysPathExt, STLF_UNIQUE));
			if (!MaskList.empty())
			{
				string strFarPathExt;
				std::for_each(CONST_RANGE(MaskList, i)
				{
					strFarPathExt.Append('*').Append(i).Append(',');
				});
				strFarPathExt.SetLength(strFarPathExt.GetLength()-1);
				ReplaceStrings(expmasks, PathExtName, strFarPathExt, -1, true);
			}
		}
	}

	bRE = expmasks.At(0) == L'/';

	if (bRE)
	{
		re.reset(new RegExp);

		if (re && re->Compile(expmasks, OP_PERLSTYLE|OP_OPTIMIZE))
		{
			n = re->GetBracketsCount();
			m.reset(n);

			if (!m)
			{
				n = 0;
				return false;
			}

			return true;
		}

		return false;
	}
	else
	{
		Masks = StringToList(expmasks, STLF_PACKASTERISKS|STLF_PROCESSBRACKETS|STLF_SORT|STLF_UNIQUE);
		return !Masks.empty();
	}
}

void filemasks::masks::Free()
{
	Masks.clear();

	re.reset();
	m.reset();

	n = 0;
	bRE = false;
}

// Путь к файлу в FileName НЕ игнорируется

bool filemasks::masks::Compare(const string& FileName) const
{
	if (bRE)
	{
		intptr_t i = n;
		size_t len = FileName.GetLength();
		bool ret = re->Search(FileName,FileName+len,m.get(),i) ? TRUE : FALSE;

		//Освободим память если большая строка, чтоб не накапливалось.
		if (len > 1024)
			re->CleanStack();

		return ret;
	}
	else return std::find_if(CONST_RANGE(Masks, i)
	{
		return CmpName(i, FileName, false) != 0;
	}) != Masks.cend();
}

bool filemasks::masks::IsEmpty() const
{
	return bRE? !n : Masks.empty();
}
