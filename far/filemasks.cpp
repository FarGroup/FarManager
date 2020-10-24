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

// BUGBUG
#include "platform.headers.hpp"

// Self:
#include "filemasks.hpp"

// Internal:
#include "message.hpp"
#include "lang.hpp"
#include "processname.hpp"
#include "configdb.hpp"
#include "RegExp.hpp"
#include "stddlg.hpp"
#include "strmix.hpp"
#include "string_utils.hpp"
#include "plugin.hpp"

// Platform:
#include "platform.env.hpp"

// Common:
#include "common/enum_tokens.hpp"

// External:

//----------------------------------------------------------------------------

static const wchar_t ExcludeMaskSeparator = L'|';
static const wchar_t RE_start = L'/', RE_end = L'/';

static auto extract_impl(string_view& Str, size_t const Size)
{
	const auto Token = Str.substr(0, Size);
	Str.remove_prefix(Size);
	return Token;
}

static auto extract_separators(string_view& Str)
{
	const auto SeparatorsSize = std::find_if_not(ALL_CONST_RANGE(Str), [](wchar_t c) { return c == L' ' || c == L',' || c == L';'; }) - Str.cbegin();
	return extract_impl(Str, SeparatorsSize);
}

static auto extract_masks(string_view& Str)
{
	const auto MasksSize = std::find_if(ALL_CONST_RANGE(Str), [](wchar_t c) { return c == RE_start || c == ExcludeMaskSeparator; }) - Str.cbegin();
	return extract_impl(Str, MasksSize);
}

static auto extract_re(string_view& Str)
{
	if (!starts_with(Str, RE_start))
		return extract_impl(Str, 0);

	auto Iterator = Str.cbegin() + 1;

	while (Iterator != Str.cend() && (*Iterator != RE_end || *(Iterator - 1) == L'\\'))
		++Iterator;

	if (Iterator != Str.cend() && *Iterator == RE_end)
		++Iterator;

	// options
	const auto Size = std::find_if_not(Iterator, Str.cend(), is_alpha) - Str.cbegin();
	return extract_impl(Str, Size);
}

class filemasks::masks
{
public:
	bool assign(string&& Masks, DWORD Flags);
	bool operator==(string_view FileName) const;
	bool empty() const;

private:
	struct regex_data
	{
		RegExp Regex;
		mutable std::vector<RegExpMatch> Match;
	};

	std::variant<std::vector<string>, regex_data> m_Masks;
};

filemasks::filemasks() = default;
filemasks::~filemasks() = default;
filemasks::filemasks(filemasks&&) noexcept = default;
filemasks& filemasks::operator=(filemasks&&) noexcept = default;

bool filemasks::assign(string_view Str, DWORD const Flags)
{
	if (Str.empty())
		return false;

	bool Result = false;

	clear();

	string ExpandedGroups(Str);
	std::unordered_set<string> UsedGroups;
	size_t LBPos, RBPos;
	string MaskGroupValue;

	while ((LBPos = ExpandedGroups.find(L'<')) != string::npos && (RBPos = ExpandedGroups.find(L'>', LBPos)) != string::npos)
	{
		const auto MaskGroupNameWithBrackets = string_view(ExpandedGroups).substr(LBPos, RBPos - LBPos + 1);
		string MaskGroupName(MaskGroupNameWithBrackets.substr(1, MaskGroupNameWithBrackets.size() - 2));

		if (contains(UsedGroups, MaskGroupName))
		{
			MaskGroupValue.clear();
		}
		else
		{
			MaskGroupValue = ConfigProvider().GeneralCfg()->GetValue<string>(L"Masks"sv, MaskGroupName);
			UsedGroups.emplace(std::move(MaskGroupName));
		}
		replace(ExpandedGroups, MaskGroupNameWithBrackets, MaskGroupValue);
	}

	Str = ExpandedGroups;

	if (!Str.empty())
	{
		string SimpleMasksInclude, SimpleMasksExclude;

		auto DestContainer = &Include;
		auto DestString = &SimpleMasksInclude;

		Result = true;

		while (!Str.empty())
		{
			extract_separators(Str);
			const auto Re = extract_re(Str);
			if (!Re.empty())
			{
				masks m;
				Result = m.assign(string(Re), Flags);
				if (Result)
				{
					DestContainer->push_back(std::move(m));
				}
				else
				{
					break;
				}
			}

			extract_separators(Str);
			const auto Masks = extract_masks(Str);
			if (!Masks.empty())
			{
				DestString->append(Masks);
			}

			if (starts_with(Str, ExcludeMaskSeparator))
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

				Str.remove_prefix(1);
			}
		}

		if (Result && !SimpleMasksInclude.empty())
		{
			masks m;
			Result = m.assign(std::move(SimpleMasksInclude), Flags);
			if (Result)
				Include.push_back(std::move(m));
		}

		if (Result && !SimpleMasksExclude.empty())
		{
			masks m;
			Result = m.assign(std::move(SimpleMasksExclude), Flags);
			if (Result)
				Exclude.push_back(std::move(m));
		}

		if (Result && Include.empty() && !Exclude.empty())
		{
			masks m;
			Result = m.assign(L"*"s, Flags);
			if (Result)
				Include.push_back(std::move(m));
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

bool filemasks::check(const string_view Name) const
{
	return contains(Include, Name) && !contains(Exclude, Name);
}

bool filemasks::empty() const
{
	return std::all_of(ALL_CONST_RANGE(Include), LIFT_MF(empty)) &&
	       std::all_of(ALL_CONST_RANGE(Exclude), LIFT_MF(empty));
}

void filemasks::ErrorMessage()
{
	Message(MSG_WARNING,
		msg(lng::MWarning),
		{
			msg(lng::MIncorrectMask)
		},
		{ lng::MOk });
}

static void add_pathext(string& Masks)
{
	static const auto PathExtName = L"%PATHEXT%"sv;
	if (contains_icase(Masks, PathExtName))
	{
		string FarPathExt;
		for (const auto& i: enum_tokens_with_quotes(os::env::get(L"PATHEXT"sv), L";"sv))
		{
			if (i.empty())
				continue;

			append(FarPathExt, L'*', i, L',');
		}

		if (!FarPathExt.empty())
			FarPathExt.pop_back();

		replace_icase(Masks, PathExtName, FarPathExt);
	}
}

class with_brackets
{
public:
	void reset()
	{
		m_InBrackets = false;
	}

	bool active(wchar_t i)
	{
		if (!m_InBrackets && i == L'[')
		{
			m_InBrackets = true;
			return true;
		}

		if (m_InBrackets)
		{
			if (i == L']')
				m_InBrackets = false;
			return true;
		}

		return false;
	}

private:
	bool m_InBrackets{};
};

bool filemasks::masks::assign(string&& Masks, DWORD Flags)
{
	if (Masks[0] != RE_start)
	{
		auto& MasksData = m_Masks.emplace<0>();

		add_pathext(Masks);

		for (const auto& Mask: enum_tokens_with_quotes_t<with_brackets, with_trim>(Masks, L",;"sv))
		{
			if (Mask.empty())
				continue;

			if (Mask == L"*.*"sv)
			{
				MasksData.emplace_back(1, L'*');
			}
			else if (contains(Mask, L"**"sv))
			{
				string NewMask(Mask);
				remove_duplicates(NewMask, L'*');
				MasksData.emplace_back(std::move(NewMask));
			}
			else
			{
				MasksData.emplace_back(Mask);
			}
		}

		return !MasksData.empty();
	}

	auto& RegexData = m_Masks.emplace<1>();

	if (!RegexData.Regex.Compile(Masks, OP_PERLSTYLE | OP_OPTIMIZE))
	{
		if (!(Flags & FMF_SILENT))
		{
			ReCompileErrorMessage(RegexData.Regex, Masks);
		}
		return false;
	}

	RegexData.Match.resize(RegexData.Regex.GetBracketsCount());
	return true;
}

// Путь к файлу в FileName НЕ игнорируется

bool filemasks::masks::operator==(const string_view FileName) const
{
	return std::visit(overload
	{
		[&](const std::vector<string>& Data)
		{
			return std::any_of(CONST_RANGE(Data, i) { return CmpName(i, FileName, false); });
		},
		[&](const regex_data& Data)
		{
			intptr_t i = Data.Match.size();
			return Data.Regex.Search(FileName, Data.Match.data(), i) != 0; // BUGBUG
		}
	}, m_Masks);
}

bool filemasks::masks::empty() const
{
	return std::visit(overload
	{
		[](const std::vector<string>& Data)
		{
			return Data.empty();
		},
		[](const regex_data& Data)
		{
			return Data.Match.empty();
		}
	}, m_Masks);
}

#ifdef ENABLE_TESTS

#include "testing.hpp"

TEST_CASE("masks")
{
	static const struct
	{
		string_view Mask, Test;
		bool Match;
	}
	Tests[]
	{
		{ L".."sv,          {},                  false },
		{ L".."sv,          L"."sv,              false },
		{ L".."sv,          L".."sv,             true  },
		{ L"*.ext"sv,       L"file.ext"sv,       true  },
		{ L"*.ex*"sv,       L"file.ext"sv,       true  },
		{ L"*.e?t"sv,       L"file.est"sv,       true  },
		{ L"*.ext"sv,       L"file.bin"sv,       false },
		{ L"file.*"sv,      L"file"sv,           true  },
		{ L"file.*"sv,      L"file.."sv,         true  },
		{ L"file.*"sv,      L"file.bin"sv,       true  },
		{ L"file.*"sv,      L"file..bin"sv,      true  },
		{ L"file.*|*b*"sv,  L"file.bin"sv,       false },
	};

	filemasks Masks;

	for (const auto& i: Tests)
	{
		REQUIRE(Masks.assign(i.Mask, FMF_SILENT));
		REQUIRE(i.Match == Masks.check(i.Test));
	}
}
#endif
