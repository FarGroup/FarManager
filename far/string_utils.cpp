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

#include "headers.hpp"
#pragma hdrstop

#include "string_utils.hpp"

const wchar_t WIN_EOL_fmt[]      = L"\r\n";   // <CR><LF>     // same as DOS
const wchar_t UNIX_EOL_fmt[]     = L"\n";     // <LF>         //
const wchar_t OLD_MAC_EOL_fmt[]  = L"\r";     // <CR>         // modern is Unix <LF>
const wchar_t BAD_WIN_EOL_fmt[]  = L"\r\r\n"; // <CR><CR><LF> // result of <CR><LF> text mode conversion

const string& GetSpaces()
{
	// TODO: test for consistency with IsSpace()
	static const auto Spaces = L" \t"s;
	return Spaces;
}

const string& GetEols()
{
	// TODO: test for consistency with IsEol()
	static const auto Eols = L"\r\n"s;
	return Eols;
}

const string& GetSpacesAndEols()
{
	static const auto SpacesOrEols = GetSpaces() + GetEols();
	return SpacesOrEols;
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

void upper(wchar_t *Str, size_t Size)
{
	CharUpperBuff(Str, static_cast<DWORD>(Size));
}

void lower(wchar_t *Str, size_t Size)
{
	CharLowerBuff(Str, static_cast<DWORD>(Size));
}

void upper(wchar_t* Str)
{
	upper(Str, wcslen(Str));
}

void lower(wchar_t* Str)
{
	lower(Str, wcslen(Str));
}

string& upper(string& Str, size_t Pos, size_t Count)
{
	upper(&Str[0] + Pos, Count == string::npos? Str.size() - Pos : Str.size());
	return Str;
}

string& lower(string& Str, size_t Pos, size_t Count)
{
	lower(&Str[0] + Pos, Count == string::npos ? Str.size() - Pos : Str.size());
	return Str;
}

string upper_copy(string Str, size_t Pos, size_t Count)
{
	return upper(Str, Pos, Count);
}

string lower_copy(string Str, size_t Pos, size_t Count)
{
	return lower(Str, Pos, Count);
}

bool equal_to_icase::operator()(wchar_t c1, wchar_t c2) const
{
	return upper(c1) == upper(c2);
}

bool equal(const string_view& Str1, const string_view& Str2)
{
	return Str1 == Str2;
}

bool equal_icase(const string_view& Str1, const string_view& Str2)
{
	return std::equal(ALL_CONST_RANGE(Str1), ALL_CONST_RANGE(Str2), equal_to_icase{});
}

bool starts_with(const string_view& Str, const string_view& Prefix)
{
	return Str.size() >= Prefix.size() && equal(string_view(Str.data(), Prefix.size()), Prefix);
}

bool starts_with_icase(const string_view& Str, const string_view& Prefix)
{
	return Str.size() >= Prefix.size() && equal_icase(string_view(Str.data(), Prefix.size()), Prefix);
}

bool ends_with(const string_view& Str, const string_view& Suffix)
{
	return Str.size() >= Suffix.size() && equal(string_view(Str.data() + Str.size() - Suffix.size(), Suffix.size()), Suffix);
}

bool ends_with_icase(const string_view& Str, const string_view& Suffix)
{
	return Str.size() >= Suffix.size() && equal_icase(string_view(Str.data() + Str.size() - Suffix.size(), Suffix.size()), Suffix);
}

bool contains(const string_view& Str, const string_view& Token)
{
	return std::search(ALL_CONST_RANGE(Str), ALL_CONST_RANGE(Token)) != Str.cend();
}

bool contains_icase(const string_view& Str, const string_view& Token)
{
	return std::search(ALL_CONST_RANGE(Str), ALL_CONST_RANGE(Token), equal_to_icase{}) != Str.cend();
}

int StrCmp(const wchar_t *s1, const wchar_t *s2)
{
	return CompareString(0, SORT_STRINGSORT, s1, -1, s2, -1) - 2;
}

int StrCmpI(const wchar_t *s1, const wchar_t *s2)
{
	return CompareString(0, SORT_STRINGSORT | NORM_IGNORECASE, s1, -1, s2, -1) - 2;
}

// deprecated, for pluginapi::apiStrCmpNI only
int StrCmpNI(const wchar_t *s1, const wchar_t *s2, size_t n)
{
	return CompareString(0, NORM_STOP_ON_NULL | SORT_STRINGSORT | NORM_IGNORECASE, s1, static_cast<int>(n), s2, static_cast<int>(n)) - 2;
}

int StrCmp(const string_view& Str1, const string_view& Str2)
{
	return CompareString(0, SORT_STRINGSORT, Str1.data(), static_cast<int>(Str1.size()), Str2.data(), static_cast<int>(Str2.size())) - 2;
}

int StrCmpI(const string_view& Str1, const string_view& Str2)
{
	return CompareString(0, SORT_STRINGSORT | NORM_IGNORECASE, Str1.data(), static_cast<int>(Str1.size()), Str2.data(), static_cast<int>(Str2.size())) - 2;
}

static int per_char_compare(const string_view& Str1, const string_view& Str2, const std::function<int(const wchar_t*&, const wchar_t*, const wchar_t*&, const wchar_t*)>& Comparer)
{
	auto Iterator = std::make_pair(Str1.cbegin(), Str2.cbegin());
	const auto End = std::make_pair(Str1.cend(), Str2.cend());

	while (Iterator.first != End.first && Iterator.second != End.second)
	{
		if (const auto Result = Comparer(Iterator.first, End.first, Iterator.second, End.second))
			return Result < 0? -1 : 1;
	}

	if (Iterator == End)
	{
		const auto Size1 = Iterator.first - Str1.cbegin();
		const auto Size2 = Iterator.second - Str2.cbegin();

		if (Size1 == Size2)
			return 0;

		return Size1 < Size2? -1 : 1;
	}

	return Iterator.first == End.first? -1 : 1;
}

static const auto create_alt_sort_table()
{
	static_assert(sizeof(wchar_t) == 2, "4 GB for a sort table is too much, rewrite it.");
	static const auto TableSize = std::numeric_limits<wchar_t>::max() + 1;
	static wchar_t alt_sort_table[TableSize];
	std::vector<wchar_t> chars(TableSize);
	std::iota(ALL_RANGE(chars), 0);
	std::sort(chars.begin() + 1, chars.end(), [](wchar_t a, wchar_t b) { return StrCmp(string_view(&a, 1), string_view(&b, 1)) < 0; });

	int u_beg = 0, u_end = 0xffff;
	for (int ic=0; ic < 0x10000; ++ic)
	{
		if (chars[ic] == L'a')
		{
			u_beg = ic;
			break;
		}
		alt_sort_table[chars[ic]] = static_cast<wchar_t>(ic);
	}

	for (int ic=0xffff; ic > u_beg; --ic)
	{
		if (is_upper(chars[ic]))
		{
			u_end = ic;
			break;
		}
		alt_sort_table[chars[ic]] = static_cast<wchar_t>(ic);
	}
	assert(u_beg > 0 && u_beg < u_end && u_end < 0xffff);

	int cc = u_beg;
	for (int ic=u_beg; ic <= u_end; ++ic) // uppercase first
	{
		if (is_upper(chars[ic]))
			alt_sort_table[chars[ic]] = static_cast<wchar_t>(cc++);
	}
	for (int ic=u_beg; ic <= u_end; ++ic) // than not uppercase
	{
		if (!is_upper(chars[ic]))
			alt_sort_table[chars[ic]] = static_cast<wchar_t>(cc++);
	}
	assert(cc == u_end+1);
	return alt_sort_table;
}

int StrCmpC(const string_view& Str1, const string_view& Str2)
{
	static const auto alt_sort_table = create_alt_sort_table();

	return per_char_compare(Str1, Str2, [](const wchar_t*& It1, const wchar_t*, const wchar_t*& It2, const wchar_t*)
	{
		const auto Result = static_cast<int>(alt_sort_table[*It1]) - static_cast<int>(alt_sort_table[*It2]);
		++It1;
		++It2;
		return Result < 0? -1 : Result > 0? 1 : 0;
	});
}

static int NumStrCmp_base(const string_view& Str1, const string_view Str2, int(*Comparer)(const string_view&, const string_view&))
{
	return per_char_compare(Str1, Str2, [&](const wchar_t*& It1, const wchar_t* End1, const wchar_t*& It2, const wchar_t* End2)
	{
		if (std::iswdigit(*It1) && std::iswdigit(*It2))
		{
			auto NotZero = [](wchar_t Char) { return Char != L'0'; };

			It1 = std::find_if(It1, End1, NotZero);
			It2 = std::find_if(It2, End2, NotZero);

			if (It1 == End1 && It2 == End2)
				return 0;

			// compare numbers
			int Result = 0;
			while (It1 != End1 && It2 != End2 && std::iswdigit(*It1) && std::iswdigit(*It2))
			{
				if (!Result && *It1 != *It2)
					Result = *It1 < *It2? -1 : 1;

				++It1;
				++It2;
			}

			const auto EndOrNonDigit1 = It1 == End1 || !std::iswdigit(*It1);
			const auto EndOrNonDigit2 = It2 == End2 || !std::iswdigit(*It2);

			if (EndOrNonDigit1 && EndOrNonDigit2)
				return Result;

			return EndOrNonDigit1? -1 : 1;
		}

		return Comparer(string_view(It1++, 1), string_view(It2++, 1));
	});
}

int NumStrCmp(const string_view& Str1, const string_view& Str2)
{
	return NumStrCmp_base(Str1, Str2, StrCmp);
}

int NumStrCmpI(const string_view& Str1, const string_view& Str2)
{
	return NumStrCmp_base(Str1, Str2, StrCmpI);
}

int NumStrCmpC(const string_view& Str1, const string_view& Str2)
{
	return NumStrCmp_base(Str1, Str2, StrCmpC);
}

str_comparer get_comparer(bool Numeric, bool CaseSensitive)
{
	using comparer_type = int(*)(const string_view&, const string_view&);
	static const comparer_type Comparers[][2] =
	{
		{ StrCmpI, StrCmpC },
		{ NumStrCmpI, NumStrCmpC },
	};

	return Comparers[Numeric? 1 : 0][CaseSensitive? 1 : 0];
}

SELF_TEST(
	assert(NumStrCmp(L"", L"") == 0);
	assert(NumStrCmp(L"", L"a") < 0);
	assert(NumStrCmp(L"a", L"a") == 0);

	assert(NumStrCmp(L"0", L"1") < 0);
	assert(NumStrCmp(L"0", L"00") < 0);
	assert(NumStrCmp(L"1", L"00") > 0);
	assert(NumStrCmp(L"10", L"1") > 0);
	assert(NumStrCmp(L"10", L"2") > 0);
	assert(NumStrCmp(L"10", L"0100") < 0);
	assert(NumStrCmp(L"1", L"001") < 0);

	assert(NumStrCmp(L"10a", L"2b") > 0);
	assert(NumStrCmp(L"10a", L"0100b") < 0);
	assert(NumStrCmp(L"a1a", L"a001a") < 0);
	assert(NumStrCmp(L"a1b2c", L"a1b2c") == 0);
	assert(NumStrCmp(L"a01b2c", L"a1b002c") < 0);
	assert(NumStrCmp(L"a01b3c", L"a1b002") > 0);

	assert(NumStrCmp(L"10", L"01") > 0);
	assert(!NumStrCmp(L"01", L"01"));

	assert(NumStrCmp(L"A1", L"a2") > 0);
	assert(NumStrCmpI(L"A1", L"a2") < 0);
)
