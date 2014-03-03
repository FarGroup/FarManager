/*
filemasks.cpp

Класс для работы с масками файлов (учитывается наличие масок исключения).
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
#include "RegExp.hpp"

const wchar_t ExcludeMaskSeparator = L'|';
const wchar_t RE_start = L'/', RE_end = L'/';

static inline const wchar_t* SkipSeparators(const wchar_t* masks)
{
	while(*masks == L' ' || *masks == L',' || *masks == L';')
		++masks;
	return masks;
}

static inline const wchar_t* SkipMasks(const wchar_t* masks)
{
	while(*masks && *masks != RE_start && *masks != ExcludeMaskSeparator)
		masks++;
	return masks;
}

static inline const wchar_t* SkipRE(const wchar_t* masks)
{
	if (*masks == RE_start)
	{
		masks++;

		while (*masks && (*masks != RE_end || *(masks-1) == L'\\'))
			masks++;

		if (*masks == RE_end)
			masks++;
		// options
		while (IsAlpha(*masks))
			masks++;
	}
	return masks;
}

filemasks::masks::~masks()
{
}

bool filemasks::Set(const string& masks, DWORD Flags)
{
	bool Result = false;

	if (!masks.empty())
	{
		Free();

		string expmasks(masks);
		std::list<string> UsedGroups;
		size_t LBPos, RBPos;

		while((LBPos = expmasks.find(L'<')) != string::npos && (RBPos = expmasks.find(L'>', LBPos)) != string::npos)
		{
			string MaskGroupNameWB = expmasks.substr(LBPos, RBPos-LBPos+1);
			string MaskGroupName = expmasks.substr(LBPos+1, RBPos-LBPos-1);
			string MaskGroupValue;
			if (std::find(ALL_CONST_RANGE(UsedGroups), MaskGroupName) == UsedGroups.cend())
			{
				Global->Db->GeneralCfg()->GetValue(L"Masks", MaskGroupName, MaskGroupValue, L"");
				ReplaceStrings(expmasks, MaskGroupNameWB, MaskGroupValue);
				UsedGroups.emplace_back(MaskGroupName);
			}
			else
			{
				ReplaceStrings(expmasks, MaskGroupNameWB, L"");	
			}
		}

		if (!expmasks.empty())
		{
			const wchar_t* ptr = expmasks.data();

			string SimpleMasksInclude, SimpleMasksExclude;

			auto DestContainer = &Include;
			auto DestString = &SimpleMasksInclude;

			Result = true;

			while(*ptr)
			{
				ptr = SkipSeparators(ptr);
				auto nextpos = SkipRE(ptr);
				if (nextpos != ptr)
				{
					filemasks::masks m;
					Result = m.Set(string(ptr, nextpos-ptr));
					if (Result)
					{
						DestContainer->emplace_back(std::move(m));
						ptr = nextpos;
					}
					else
					{
						break;
					}
					ptr = nextpos;
				}
				ptr = SkipSeparators(ptr);
				nextpos = SkipMasks(ptr);
				if (nextpos != ptr)
				{
					*DestString += string(ptr, nextpos-ptr);
					ptr = nextpos;
				}
				if (*ptr == ExcludeMaskSeparator)
				{
					if (DestContainer != &Exclude)
					{
						DestContainer = &Exclude;
						DestString = &SimpleMasksExclude;
					}
					else
					{
						break;
					}
					++ptr;
				}
			}

			if (Result && !SimpleMasksInclude.empty())
			{
				filemasks::masks m;
				Result = m.Set(SimpleMasksInclude);
				if (Result)
					Include.emplace_back(std::move(m));
			}

			if (Result && !SimpleMasksExclude.empty())
			{
				filemasks::masks m;
				Result = m.Set(SimpleMasksExclude);
				if (Result)
					Exclude.emplace_back(std::move(m));
			}

			if (Result && Include.empty() && !Exclude.empty())
			{
				Include.emplace_back(VALUE_TYPE(Include)());
				Result = Include.back().Set(L"*");
			}

			Result = !empty();
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
	Include.clear();
	Exclude.clear();
}

// Путь к файлу в FileName НЕ игнорируется

bool filemasks::Compare(const string& FileName) const
{
	return std::find(Include.cbegin(), Include.cend(), FileName) != Include.cend() &&
	       std::find(Exclude.cbegin(), Exclude.cend(), FileName) == Exclude.cend();
}

bool filemasks::empty() const
{
	return std::all_of(ALL_CONST_RANGE(Include), std::mem_fn(&masks::empty)) &&
	       std::all_of(ALL_CONST_RANGE(Exclude), std::mem_fn(&masks::empty));
}

void filemasks::ErrorMessage()
{
	Message(MSG_WARNING, 1, MSG(MWarning), MSG(MIncorrectMask), MSG(MOk));
}

bool filemasks::masks::Set(const string& masks)
{
	Free();

	string expmasks(masks);

	const wchar_t* PathExtName = L"%PATHEXT%";
	if (StrStrI(expmasks.data(), PathExtName))
	{
		string strSysPathExt;
		if (api::GetEnvironmentVariable(L"PATHEXT" ,strSysPathExt))
		{
			auto MaskList(StringToList(strSysPathExt, STLF_UNIQUE));
			if (!MaskList.empty())
			{
				string strFarPathExt;
				std::for_each(CONST_RANGE(MaskList, i)
				{
					strFarPathExt.append(1, '*').append(i).append(1, ',');
				});
				strFarPathExt.pop_back();
				ReplaceStrings(expmasks, PathExtName, strFarPathExt, -1, true);
			}
		}
	}

	bRE = expmasks[0] == RE_start;

	if (bRE)
	{
		re = std::make_unique<RegExp>();

		if (re && re->Compile(expmasks.data(), OP_PERLSTYLE|OP_OPTIMIZE))
		{
			m.resize(re->GetBracketsCount());
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
	m.clear();
	bRE = false;
}

// Путь к файлу в FileName НЕ игнорируется

bool filemasks::masks::operator ==(const string& FileName) const
{
	if (bRE)
	{
		intptr_t i = m.size();
		size_t len = FileName.size();
		bool ret = re->Search(FileName.data(), FileName.data() + len, const_cast<RegExpMatch *>(m.data()), i) != 0; // BUGBUG

		//Освободим память если большая строка, чтоб не накапливалось.
		if (len > 1024)
			re->CleanStack();

		return ret;
	}
	else
	{
		return std::any_of(CONST_RANGE(Masks, i) {return CmpName(i.data(), FileName.data(), false) != 0;});
	}
}

bool filemasks::masks::empty() const
{
	return bRE? m.empty() : Masks.empty();
}
