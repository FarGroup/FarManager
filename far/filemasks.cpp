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
#include "stddlg.hpp"
#include "strmix.hpp"
#include "local.hpp"

static const wchar_t ExcludeMaskSeparator = L'|';
static const wchar_t RE_start = L'/', RE_end = L'/';

static auto SkipSeparators(const string::const_iterator& Iterator, const string::const_iterator& End)
{
	return std::find_if_not(Iterator, End, [](wchar_t c) { return c == L' ' || c == L',' || c == L';'; });
}

static auto SkipMasks(const string::const_iterator& Iterator, const string::const_iterator& End)
{
	return std::find_if(Iterator, End, [](wchar_t c) { return c == RE_start || c == ExcludeMaskSeparator; });
}

static auto SkipRE(string::const_iterator Iterator, const string::const_iterator& End)
{
	if (*Iterator != RE_start)
	{
		return Iterator;
	}

	++Iterator;

	while (Iterator != End && (*Iterator != RE_end || *(Iterator - 1) == L'\\'))
		++Iterator;

	if (Iterator != End && *Iterator == RE_end)
		++Iterator;
	// options
	return std::find_if_not(Iterator, End, IsAlpha);
}

class filemasks::masks
{
public:
	NONCOPYABLE(masks);
	TRIVIALLY_MOVABLE(masks);

	masks() = default;

	bool assign(const string& Masks, DWORD Flags);
	bool operator ==(const string& Name) const;
	bool empty() const;

private:
	std::vector<string> Masks;
	std::unique_ptr<RegExp> re;
	std::vector<RegExpMatch> m;
};

filemasks::filemasks() = default;
filemasks::~filemasks() = default;
filemasks::filemasks(filemasks&&) = default;
filemasks& filemasks::operator=(filemasks&&) = default;

bool filemasks::Set(const string& Masks, DWORD Flags)
{
	if (Masks.empty())
		return false;

	bool Result = false;

	clear();

	auto ExpMasks = Masks;
	std::unordered_set<string> UsedGroups;
	size_t LBPos, RBPos;

	while ((LBPos = ExpMasks.find(L'<')) != string::npos && (RBPos = ExpMasks.find(L'>', LBPos)) != string::npos)
	{
		string MaskGroupNameWB = ExpMasks.substr(LBPos, RBPos - LBPos + 1);
		string MaskGroupName = ExpMasks.substr(LBPos + 1, RBPos - LBPos - 1);
		string MaskGroupValue;
		if (!UsedGroups.count(MaskGroupName))
		{
			ConfigProvider().GeneralCfg()->GetValue(L"Masks", MaskGroupName, MaskGroupValue, L"");
			ReplaceStrings(ExpMasks, MaskGroupNameWB, MaskGroupValue);
			UsedGroups.emplace(MaskGroupName);
		}
		else
		{
			ReplaceStrings(ExpMasks, MaskGroupNameWB, L"");
		}
	}

	if (!ExpMasks.empty())
	{
		auto ptr = ExpMasks.cbegin();
		const auto End = ExpMasks.cend();

		string SimpleMasksInclude, SimpleMasksExclude;

		auto DestContainer = &Include;
		auto DestString = &SimpleMasksInclude;

		Result = true;

		while (ptr != End)
		{
			ptr = SkipSeparators(ptr, End);
			auto nextpos = SkipRE(ptr, End);
			if (nextpos != ptr)
			{
				filemasks::masks m;
				Result = m.assign(string(ptr, nextpos), Flags);
				if (Result)
				{
					DestContainer->emplace_back(std::move(m));
					ptr = nextpos;
				}
				else
				{
					break;
				}
			}
			ptr = SkipSeparators(ptr, End);
			nextpos = SkipMasks(ptr, End);
			if (nextpos != ptr)
			{
				*DestString += string(ptr, nextpos);
				ptr = nextpos;
			}
			if (ptr != End && *ptr == ExcludeMaskSeparator)
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
			Result = m.assign(SimpleMasksInclude, Flags);
			if (Result)
				Include.emplace_back(std::move(m));
		}

		if (Result && !SimpleMasksExclude.empty())
		{
			filemasks::masks m;
			Result = m.assign(SimpleMasksExclude, Flags);
			if (Result)
				Exclude.emplace_back(std::move(m));
		}

		if (Result && Include.empty() && !Exclude.empty())
		{
			filemasks::masks m;
			Result = m.assign(L"*", Flags);
			if (Result)
				Include.emplace_back(std::move(m));
		}

		Result = !empty();
	}

	if (!Result)
	{
		if (!(Flags & FMF_SILENT))
		{
			ErrorMessage();
		}
		clear();
	}

	return Result;
}

void filemasks::clear()
{
	Include.clear();
	Exclude.clear();
}

// Путь к файлу в FileName НЕ игнорируется

bool filemasks::Compare(const string& FileName) const
{
	return contains(Include, FileName) && !contains(Exclude, FileName);
}

bool filemasks::empty() const
{
	return std::all_of(ALL_CONST_RANGE(Include), std::mem_fn(&masks::empty)) &&
	       std::all_of(ALL_CONST_RANGE(Exclude), std::mem_fn(&masks::empty));
}

void filemasks::ErrorMessage()
{
	Message(MSG_WARNING, 1, MSG(lng::MWarning), MSG(lng::MIncorrectMask), MSG(lng::MOk));
}

bool filemasks::masks::assign(const string& masks, DWORD Flags)
{
	Masks.clear();
	re.reset();
	m.clear();

	string expmasks(masks);

	static const string PathExtName = L"%PATHEXT%";
	if (StrStrI(expmasks, PathExtName) != expmasks.cend())
	{
		const auto strSysPathExt(os::env::get_variable(L"PATHEXT"));
		if (!strSysPathExt.empty())
		{
			const auto MaskList = split<std::vector<string>>(strSysPathExt, STLF_UNIQUE);
			if (!MaskList.empty())
			{
				string strFarPathExt;
				std::for_each(CONST_RANGE(MaskList, i)
				{
					append(strFarPathExt, L'*', i, ',');
				});
				strFarPathExt.pop_back();
				ReplaceStrings(expmasks, PathExtName, strFarPathExt, true);
			}
		}
	}

	if (expmasks[0] != RE_start)
	{
		Masks = split<std::vector<string>>(expmasks, STLF_PACKASTERISKS | STLF_PROCESSBRACKETS | STLF_SORT | STLF_UNIQUE);
		return !Masks.empty();
	}

	re = std::make_unique<RegExp>();

	if (!re->Compile(expmasks.data(), OP_PERLSTYLE | OP_OPTIMIZE))
	{
		if (!(Flags & FMF_SILENT))
		{
			ReCompileErrorMessage(*re, expmasks);
		}
		return false;
	}
	m.resize(re->GetBracketsCount());
	return true;
}

// Путь к файлу в FileName НЕ игнорируется

bool filemasks::masks::operator ==(const string& FileName) const
{
	if (!re)
	{
		return std::any_of(CONST_RANGE(Masks, i) { return CmpName(i.data(), FileName.data(), false); });
	}

	intptr_t i = m.size();
	return re->Search(FileName.data(), FileName.data() + FileName.size(), const_cast<RegExpMatch *>(m.data()), i) != 0; // BUGBUG
}

bool filemasks::masks::empty() const
{
	return re? m.empty() : Masks.empty();
}
