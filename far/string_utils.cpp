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

string_view GetBlanks()
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

void inplace::upper(std::span<wchar_t> const Str)
{
	CharUpperBuff(Str.data(), static_cast<DWORD>(Str.size()));
}

void inplace::lower(std::span<wchar_t> const Str)
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
			resize_exp(To);
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

size_t string_comparer_icase::operator()(wchar_t const Char) const
{
	return make_hash(upper(Char));
}

size_t string_comparer_icase::operator()(string_view const Str) const
{
	return make_hash(upper(Str));
}

bool string_comparer_icase::operator()(wchar_t Chr1, wchar_t Chr2) const
{
	return Chr1 == Chr2 || upper(Chr1) == upper(Chr2);
}

bool string_comparer_icase::operator()(const string_view Str1, const string_view Str2) const
{
	return equal_icase(Str1, Str2);
}

bool equal_icase(const string_view Str1, const string_view Str2)
{
	return Str1 == Str2 || std::ranges::equal(Str1, Str2, string_comparer_icase{});
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

	const auto Where = Str.substr(Pos);
	const auto Found = std::ranges::search(Where, What, string_comparer_icase{});
	return Found.begin() == Where.cend()? Str.npos : Pos + Found.begin() - Where.cbegin();
}

size_t find_icase(string_view const Str, wchar_t const What, size_t Pos)
{
	if (Pos >= Str.size())
		return Str.npos;

	const auto It = std::find_if(Str.cbegin() + Pos, Str.cend(), [&](wchar_t const Char) { return string_comparer_icase{}(What, Char); });
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
	m_Needle(Needle),
	m_Searcher(ALL_CONST_RANGE(m_Needle))
{
	if (CanReverse)
		m_ReverseSearcher.emplace(ALL_CONST_REVERSE_RANGE(m_Needle));
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

		return { { Offset - m_Needle.size(), m_Needle.size() } };
	}

	const auto Iterator = std::search(ALL_CONST_RANGE(Haystack), m_Searcher);
	if (Iterator == Haystack.cend())
		return {};

	return { { Iterator - Haystack.cbegin(), m_Needle.size() } };
}

icase_searcher::icase_searcher(string_view const Needle, bool const CanReverse):
	m_Searcher(upper(Needle), CanReverse)
{
}

std::optional<std::pair<size_t, size_t>> icase_searcher::find_in(string_view const Haystack, bool const Reverse) const
{
	// Reuse capacity
	m_HayStack = Haystack;
	inplace::upper(m_HayStack);
	return m_Searcher.find_in(m_HayStack, Reverse);
}

string_view detail::fuzzy_searcher_impl::normalize(string_view const Str)
{
	if (Str.empty())
	{
		m_Result.clear();
		return m_Result;
	}

	// This retarded function can't do both in one go :(
	resize_exp(m_Intermediate, Str.size());
	fold(Str, m_Intermediate, MAP_EXPAND_LIGATURES);

	resize_exp(m_Result, m_Intermediate.size());

	// For some insane reason trailing diacritics are not decomposed in old OS
	m_Intermediate.push_back(0);

	fold(m_Intermediate, m_Result, MAP_COMPOSITE | MAP_FOLDCZONE | MAP_FOLDDIGITS);

	if (!m_Result.back())
		m_Result.pop_back();

	resize_exp(m_Types, m_Result.size());
	if (!GetStringTypeW(CT_CTYPE3, m_Result.data(), static_cast<int>(m_Result.size()), m_Types.data()))
	{
		m_Result = Str;
		return m_Result;
	}

	zip const Zip(m_Result, m_Types);
	const auto Removed = std::ranges::remove_if(Zip, [](const auto& i)
	{
		return
			!flags::check_any(std::get<1>(i), C3_ALPHA | C3_LEXICAL) &&
			flags::check_any(std::get<1>(i), C3_NONSPACING | C3_DIACRITIC | C3_VOWELMARK);
	});

	m_Result.resize(m_Result.size() - Removed.size());
	return m_Result;
}

std::optional<std::pair<size_t, size_t>> detail::fuzzy_searcher_impl::find_in(const i_searcher& searcher, string_view const Haystack, bool const Reverse)
{
	const auto Result = searcher.find_in(normalize(Haystack), Reverse);
	if (!Result)
		return {};

	// We have the position in the transformed haystack, but this is not what we want.
	size_t TransformedSize{};
	std::optional<size_t> CorrectedOffset;

	for (const auto i: std::views::iota(0uz, Haystack.size()))
	{
		TransformedSize += normalize(Haystack.substr(i, 1)).size();

		if (!CorrectedOffset && TransformedSize > Result->first)
			CorrectedOffset = i;

		if (CorrectedOffset && TransformedSize >= Result->first + Result->second)
			return { { *CorrectedOffset, i - *CorrectedOffset + 1 } };
	}

	return Result;
}

#ifdef ENABLE_TESTS

#include "testing.hpp"

TEST_CASE("string.blanks")
{
	for (const auto& i: GetBlanks())
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

TEST_CASE("string.utils.hash_icase")
{
	const string_comparer_icase hash;
	REQUIRE(hash(L'A') == hash(L'a'));
	REQUIRE(hash(L'A') != hash(L'B'));
	REQUIRE(hash(L"fooBAR"sv) == hash(L"FOObar"sv));
	REQUIRE(hash(L"fooBAR"sv) != hash(L"Banana"sv));
}

TEST_CASE("string_utils.generic_lookup_icase")
{
	const unordered_string_map_icase<int> Map
	{
		{ L"ABC"s, 123 },
	};

	REQUIRE(Map.find(L"AbC"sv) != Map.cend());
	REQUIRE(Map.find(L"aBc") != Map.cend());
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

TEMPLATE_TEST_CASE("searcher.ascii", "", exact_searcher, icase_searcher, fuzzy_ic_searcher, fuzzy_cs_searcher)
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
		TestType const Searcher(i.Needle);
		REQUIRE(Searcher.find_in(i.GoodHaystack, false) == i.FirstPos);
		REQUIRE(Searcher.find_in(i.GoodHaystack, true) == i.LastPos);

		REQUIRE(!Searcher.find_in(i.BadHaystack, false));
		REQUIRE(!Searcher.find_in(i.BadHaystack, true));
	}
}

TEST_CASE("fuzzy.normalize")
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

	detail::fuzzy_searcher_impl SearcherImpl;

	for (const auto& i: Tests)
	{
		REQUIRE(SearcherImpl.normalize(i.Src) == i.Normalized);
	}
}
#endif
