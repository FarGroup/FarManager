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
	CharUpperBuff(&Char, 1);
	return Char;
}

wchar_t lower(wchar_t Char)
{
	CharLowerBuff(&Char, 1);
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

void inplace::upper(wchar_t* Str)
{
	upper({ Str, wcslen(Str) });
}

void inplace::lower(wchar_t* Str)
{
	lower({ Str, wcslen(Str) });
}

void inplace::upper(string& Str, size_t Pos, size_t Count)
{
	upper({ &Str[Pos], Count == string::npos? Str.size() - Pos : Count });
}

void inplace::lower(string& Str, size_t Pos, size_t Count)
{
	lower({ &Str[Pos], Count == string::npos? Str.size() - Pos : Count });
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
	return upper(Chr1) == upper(Chr2);
}

bool equal_icase_t::operator()(const string_view Str1, const string_view Str2) const
{
	return equal_icase(Str1, Str2);
}

bool equal_icase(const string_view Str1, const string_view Str2)
{
	return std::equal(ALL_CONST_RANGE(Str1), ALL_CONST_RANGE(Str2), equal_icase_t{});
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
#endif
