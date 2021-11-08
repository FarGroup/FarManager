/*
string_utils.cpp

Сравнение без учета регистра, преобразование регистра
*/
/*
Copyright © 2017 Far Group
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
#include "string_utils.hpp"

// Internal:

// Platform:

// Common:
#include "common/preprocessor.hpp"
#include "common/string_utils.hpp"
#include "common/utility.hpp"
#include "common/view/zip.hpp"

// External:

//----------------------------------------------------------------------------

string_view GetSpaces()
{
	return L" \t"sv;
}

string_view GetEols()
{
	return L"\r\n"sv;
}

bool is_alpha(wchar_t Char)
{
	return IsCharAlpha(Char) != FALSE;
}

bool is_alphanumeric(wchar_t Char)
{
	return IsCharAlphaNumeric(Char) != FALSE;
}

bool is_upper(wchar_t Char)
{
	return IsCharUpper(Char) != FALSE;
}

bool is_lower(wchar_t Char)
{
	return IsCharLower(Char) != FALSE;
}

wchar_t upper(wchar_t Char)
{
	inplace::upper(Char);
	return Char;
}

wchar_t lower(wchar_t Char)
{
	inplace::lower(Char);
	return Char;
}

void inplace::upper(span<wchar_t> const Str)
{
	CharUpperBuff(Str.data(), static_cast<DWORD>(Str.size()));
}

void inplace::lower(span<wchar_t> const Str)
{
	CharLowerBuff(Str.data(), static_cast<DWORD>(Str.size()));
}

void inplace::upper(wchar_t& Char)
{
	upper({ &Char, 1 });
}

void inplace::lower(wchar_t& Char)
{
	lower({ &Char, 1 });
}

void inplace::upper(wchar_t* Str)
{
	upper({ Str, std::wcslen(Str) });
}

void inplace::lower(wchar_t* Str)
{
	lower({ Str, std::wcslen(Str) });
}

void inplace::upper(string& Str, size_t Pos, size_t Count)
{
	upper({ &Str[Pos], Count == string::npos? Str.size() - Pos : Count });
}

void inplace::lower(string& Str, size_t Pos, size_t Count)
{
	lower({ &Str[Pos], Count == string::npos? Str.size() - Pos : Count });
}

static void fold(string_view const From, string& To, DWORD const Flags)
{
	for (;;)
	{
		if (const auto Result = FoldString(Flags, From.data(), static_cast<int>(From.size()), To.data(), static_cast<int>(To.size())))
		{
			To.resize(Result);
			return;
		}

		if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
		{
			resize_exp_noshrink(To);
			continue;
		}

		To = From;
		return;
	}
}

string upper(string Str)
{
	inplace::upper(Str, 0, string::npos);
	return Str;
}

string lower(string Str)
{
	inplace::lower(Str, 0, string::npos);
	return Str;
}

string upper(string_view const Str)
{
	return upper(string(Str));
}

string lower(string_view const Str)
{
	return lower(string(Str));
}

size_t hash_icase_t::operator()(wchar_t const Char) const
{
	return make_hash(upper(Char));
}

size_t hash_icase_t::operator()(string_view const Str) const
{
	return make_hash(upper(Str));
}

bool equal_icase_t::operator()(wchar_t Chr1, wchar_t Chr2) const
{
	return Chr1 == Chr2 || upper(Chr1) == upper(Chr2);
}

bool equal_icase_t::operator()(const string_view Str1, const string_view Str2) const
{
	return equal_icase(Str1, Str2);
}

bool equal_icase(const string_view Str1, const string_view Str2)
{
	return Str1 == Str2 || std::equal(ALL_CONST_RANGE(Str1), ALL_CONST_RANGE(Str2), equal_icase_t{});
}

bool starts_with_icase(const string_view Str, const string_view Prefix)
{
	return Str.size() >= Prefix.size() && equal_icase(Str.substr(0, Prefix.size()), Prefix);
}

bool ends_with_icase(const string_view Str, const string_view Suffix)
{
	return Str.size() >= Suffix.size() && equal_icase(Str.substr(Str.size() - Suffix.size()), Suffix);
}

size_t find_icase(string_view const Str, string_view const What, size_t Pos)
{
	if (Pos >= Str.size())
		return Str.npos;

	const auto It = std::search(Str.cbegin() + Pos, Str.cend(), ALL_CONST_RANGE(What), equal_icase_t{});
	return It == Str.cend()? Str.npos : It - Str.cbegin();
}

size_t find_icase(string_view const Str, wchar_t const What, size_t Pos)
{
	if (Pos >= Str.size())
		return Str.npos;

	const auto It = std::find_if(Str.cbegin() + Pos, Str.cend(), [&](wchar_t const Char) { return equal_icase_t{}(What, Char); });
	return It == Str.cend() ? Str.npos : It - Str.cbegin();
}

bool contains_icase(const string_view Str, const string_view What)
{
	return find_icase(Str, What) != Str.npos;
}

bool contains_icase(const string_view Str, wchar_t const What)
{
	return find_icase(Str, What) != Str.npos;
}

exact_searcher::exact_searcher(string_view const Needle, bool const CanReverse):
	m_Searcher(ALL_CONST_RANGE(Needle)),
	m_NeedleSize(Needle.size())
{
	if (CanReverse)
		m_ReverseSearcher.emplace(ALL_CONST_REVERSE_RANGE(Needle));
}

std::optional<std::pair<size_t, size_t>> exact_searcher::find_in(string_view const Haystack, bool const Reverse) const
{
	if (Reverse)
	{
		assert(m_ReverseSearcher);
		const auto ReverseIterator = std::search(ALL_CONST_REVERSE_RANGE(Haystack), *m_ReverseSearcher);
		const auto Offset = Haystack.crend() - ReverseIterator;
		if (!Offset)
			return {};

		return { { Offset - m_NeedleSize, m_NeedleSize } };
	}

	const auto Iterator = std::search(ALL_CONST_RANGE(Haystack), m_Searcher);
	if (Iterator == Haystack.cend())
		return {};

	return { { Iterator - Haystack.cbegin(), m_NeedleSize } };
}

bool exact_searcher::contains_in(string_view const Haystack) const
{
	return find_in(Haystack).has_value();
}

static void normalize_for_search(string_view const Str, string& Result, string& Intermediate, std::vector<WORD>& Types)
{
	if (Str.empty())
	{
		Result.clear();
		return;
	}

	// This retarded function can't do both in one go :(
	resize_exp_noshrink(Intermediate, Str.size());
	fold(Str, Intermediate, MAP_EXPAND_LIGATURES);

	resize_exp_noshrink(Result, Intermediate.size());

	// For some insane reason trailing diacritics are not decomposed in old OS
	Intermediate.push_back(0);

	fold(Intermediate, Result, MAP_COMPOSITE | MAP_FOLDCZONE | MAP_FOLDDIGITS);

	if (!Result.back())
		Result.pop_back();

	resize_exp_noshrink(Types, Result.size());
	if (!GetStringTypeW(CT_CTYPE3, Result.data(), static_cast<int>(Result.size()), Types.data()))
	{
		Result = Str;
		return;
	}

	zip const Zip(Result, Types);
	const auto End = std::remove_if(ALL_RANGE(Zip), [](const auto& i)
	{
		return
			!flags::check_any(std::get<1>(i), C3_ALPHA | C3_LEXICAL) &&
			flags::check_any(std::get<1>(i), C3_NONSPACING | C3_DIACRITIC | C3_VOWELMARK);
	});

	Result.resize(End - Zip.begin());
}

fuzzy_searcher::fuzzy_searcher(string_view const Needle, bool const CanReverse):
	m_Searcher(((void)normalize_for_search(Needle, m_Needle, m_Intermediate, m_Types), (void)inplace::upper(m_Needle), m_Needle), CanReverse)
{
}

std::optional<std::pair<size_t, size_t>> fuzzy_searcher::find_in(string_view const Haystack, bool const Reverse) const
{
	const auto Result = find_in_uncorrected(Haystack, Reverse);
	if (!Result)
		return {};

	// We have the position in the transformed haystack, but this is not what we want.
	size_t TransformedSize{};
	std::optional<size_t> CorrectedOffset;

	for (const auto& i: irange(Haystack.size()))
	{
		normalize_for_search(Haystack.substr(i, 1), m_HayStack, m_Intermediate, m_Types);
		TransformedSize += m_HayStack.size();

		if (!CorrectedOffset && TransformedSize > Result->first)
			CorrectedOffset = i;

		if (CorrectedOffset && TransformedSize >= Result->first + Result->second)
			return { { *CorrectedOffset, i - *CorrectedOffset + 1 } };
	}

	return Result;
}

bool fuzzy_searcher::contains_in(string_view Haystack) const
{
	return find_in_uncorrected(Haystack).has_value();
}

std::optional<std::pair<size_t, size_t>> fuzzy_searcher::find_in_uncorrected(string_view Haystack, bool Reverse) const
{
	normalize_for_search(Haystack, m_HayStack, m_Intermediate, m_Types);
	inplace::upper(m_HayStack);
	return m_Searcher.find_in(m_HayStack, Reverse);
}


#ifdef ENABLE_TESTS

#include "testing.hpp"

TEST_CASE("string.spaces")
{
	for (const auto& i: GetSpaces())
	{
		REQUIRE(std::iswblank(i));
	}
}

TEST_CASE("string.eols")
{
	for (const auto& i: GetEols())
	{
		REQUIRE(IsEol(i));
	}
}

TEST_CASE("string.traits")
{
	REQUIRE(is_alpha(L'A'));
	REQUIRE(!is_alpha(L'1'));

	REQUIRE(is_alphanumeric(L'0'));
	REQUIRE(!is_alphanumeric(L'?'));

	REQUIRE(is_upper(L'A'));
	REQUIRE(!is_upper(L'a'));

	REQUIRE(is_lower(L'a'));
	REQUIRE(!is_lower(L'A'));

	REQUIRE(!is_upper(L'1'));
	REQUIRE(!is_lower(L'1'));
}

TEST_CASE("string.case")
{
	REQUIRE(upper(L'a') == L'A');
	REQUIRE(upper(L'A') == L'A');

	REQUIRE(upper(L"foo"sv) == L"FOO"sv);
	REQUIRE(upper(L"FOO"sv) == L"FOO"sv);

	REQUIRE(lower(L'A') == L'a');
	REQUIRE(lower(L'a') == L'a');

	REQUIRE(lower(L"FOO"sv) == L"foo"sv);
	REQUIRE(lower(L"foo"sv) == L"foo"sv);
}

TEST_CASE("string.utils")
{
	for (const auto& i: GetSpaces())
	{
		REQUIRE(std::isblank(i));
	}

	for (const auto& i: GetEols())
	{
		REQUIRE(IsEol(i));
	}
}

TEST_CASE("string.utils.hash")
{
	const hash_icase_t hash;
	REQUIRE(hash(L'A') == hash(L'a'));
	REQUIRE(hash(L'A') != hash(L'B'));
	REQUIRE(hash(L"fooBAR"sv) == hash(L"FOObar"sv));
	REQUIRE(hash(L"fooBAR"sv) != hash(L"Banana"sv));
}

TEST_CASE("string.utils.icase")
{
	const auto npos = string_view::npos;

	static const struct
	{
		string_view Str, Token;
		size_t Pos;
	}
	Tests[]
	{
		{ {},                    {},                npos, },
		{ {},                    L"abc"sv,          npos, },
		{ L"foobar"sv,           {},                0,    },
		{ L"foobar"sv,           L"FOOBAR"sv,       0,    },
		{ L"foobar"sv,           L"foobar1"sv,      npos, },
		{ L"foobar"sv,           L"foo"sv,          0,    },
		{ L"foobar"sv,           L"FOO"sv,          0,    },
		{ L"foobar"sv,           L"OoB"sv,          1,    },
		{ L"foobar"sv,           L"BaR"sv,          3,    },
	};

	for (const auto& i: Tests)
	{
		REQUIRE(find_icase(i.Str, i.Token) == i.Pos);
		REQUIRE(contains_icase(i.Str, i.Token) == (i.Pos != npos));
	}
}

TEMPLATE_TEST_CASE("exact_searcher", "", exact_searcher, fuzzy_searcher)
{
	static const struct
	{
		string_view Needle, GoodHaystack, BadHaystack;
		std::optional<std::pair<size_t, size_t>> FirstPos, LastPos;
	}
	Tests[]
	{
		{ {},           {},                {},          {},          {},         },
		{ L"1"sv,       L"1"sv,            L""sv,       {{ 0, 1 }},  {{ 0, 1 }}, },
		{ L"11"sv,      L"111"sv,          L"1"sv,      {{ 0, 2 }},  {{ 1, 2 }}, },
		{ L"la"sv,      L"lalala"sv,       L"kekeke"sv, {{ 0, 2 }},  {{ 4, 2 }}, },
		{ L"ll"sv,      L"Hullaballoo"sv,  L"lol"sv,    {{ 2, 2 }},  {{ 7, 2 }}, },
	};

	for (const auto& i: Tests)
	{
		exact_searcher const Searcher(i.Needle);
		REQUIRE(Searcher.find_in(i.GoodHaystack, false) == i.FirstPos);
		REQUIRE(Searcher.find_in(i.GoodHaystack, true) == i.LastPos);

		REQUIRE(!Searcher.find_in(i.BadHaystack, false));
		REQUIRE(!Searcher.find_in(i.BadHaystack, true));
	}
}

TEST_CASE("normalize_for_search")
{
	static const struct
	{
		string_view Src, Normalized;
	}
	Tests[]
	{
		{ {},                {},               },
		{ L"\0\0\0"sv,       L"\0\0\0"sv,      },
		{ L"їёй"sv,          L"іеи"sv,         },
		{ L"Æﬁ"sv,           L"AEfi"sv,        },
		{ L"Ĝŕžèģōŗż"sv,     L"Grzegorz"sv,    },
		{ L"𝔇𝔬𝔯𝔦𝔪𝔢"sv,       L"Dorime"sv,      },
		{ L"⓪①②③④⑤"sv,       L"012345"sv,      },
		{ L"⅓/¼"sv,          L"1⁄3/1⁄4"sv,     },
		{ L"ßß"sv,           L"ssss"sv,        },
		{ L"ざじず"sv,       L"さしす"sv,      },
	};

	string Normalized, Intermediate;
	std::vector<WORD> Types;

	for (const auto& i: Tests)
	{
		normalize_for_search(i.Src, Normalized, Intermediate, Types);
		REQUIRE(Normalized == i.Normalized);
	}
}
#endif
