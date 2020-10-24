/*
string_sort.cpp

*/
/*
Copyright © 2018 Far Group
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
#include "string_sort.hpp"

// Internal:
#include "string_utils.hpp"
#include "config.hpp"
#include "global.hpp"
#include "imports.hpp"

// Platform:

// Common:

// External:

//----------------------------------------------------------------------------

template<typename comparer>
static int per_char_compare(const string_view Str1, const string_view Str2, const comparer Comparer)
{
	// VS2019 bug - 'classic' CTAD breaks the compilation here
	auto Iterator = std::pair(Str1.cbegin(), Str2.cbegin());
	const auto End = std::pair(Str1.cend(), Str2.cend());

	while (Iterator.first != End.first && Iterator.second != End.second)
	{
		if (const auto Result = Comparer(Iterator.first, End.first, Iterator.second, End.second))
			return Result;
	}

	if (Iterator == End)
	{
		const auto Size1 = Iterator.first - Str1.cbegin();
		const auto Size2 = Iterator.second - Str2.cbegin();

		return Size1 - Size2;
	}

	return Iterator.first == End.first? -1 : 1;
}

static int number_comparer(string_view::const_iterator& It1, string_view::const_iterator const End1, string_view::const_iterator& It2, string_view::const_iterator const End2)
{
	const auto IsZero = [](wchar_t Char) { return Char == L'0'; };

	const auto Begin1 = It1;
	const auto Begin2 = It2;

	It1 = std::find_if_not(It1, End1, IsZero);
	It2 = std::find_if_not(It2, End2, IsZero);

	// 00 is less than 0 in Windows numeric sort implementation
	const auto ZerosResult = static_cast<int>((It2 - Begin2) - (It1 - Begin1));

	if (It1 == End1 && It2 == End2)
		return ZerosResult;

	int Result = 0;
	while (It1 != End1 && It2 != End2 && std::iswdigit(*It1) && std::iswdigit(*It2))
	{
		if (!Result && *It1 != *It2)
			Result = *It1 - *It2;

		++It1;
		++It2;
	}

	const auto EndOrNonDigit1 = It1 == End1 || !std::iswdigit(*It1);
	const auto EndOrNonDigit2 = It2 == End2 || !std::iswdigit(*It2);

	if (EndOrNonDigit1 && EndOrNonDigit2)
		return Result? Result : ZerosResult;

	return EndOrNonDigit1? -1 : 1;
}

static int ordinal_comparer(wchar_t Char1, wchar_t Char2)
{
	return static_cast<int>(Char1) - static_cast<int>(Char2);
}

static int compare_ordinal(const string_view Str1, const string_view Str2)
{
	if (imports.CompareStringOrdinal)
	{
		if (const auto Result = imports.CompareStringOrdinal(Str1.data(), static_cast<int>(Str1.size()), Str2.data(), static_cast<int>(Str2.size()), FALSE))
			return Result - 2;
	}

	return per_char_compare(Str1, Str2, [](string_view::const_iterator& It1, string_view::const_iterator, string_view::const_iterator& It2, string_view::const_iterator)
	{
		return ordinal_comparer(*It1++, *It2++);
	});
}

static int compare_ordinal_icase(const string_view Str1, const string_view Str2)
{
	if (imports.CompareStringOrdinal)
	{
		if (const auto Result = imports.CompareStringOrdinal(Str1.data(), static_cast<int>(Str1.size()), Str2.data(), static_cast<int>(Str2.size()), TRUE))
			return Result - 2;
	}

	return per_char_compare(Str1, Str2, [](string_view::const_iterator& It1, string_view::const_iterator, string_view::const_iterator& It2, string_view::const_iterator)
	{
		return ordinal_comparer(upper(*It1++), upper(*It2++));
	});
}

static int compare_ordinal_numeric(const string_view Str1, const string_view Str2)
{
	return per_char_compare(Str1, Str2, [](string_view::const_iterator& It1, string_view::const_iterator const End1, string_view::const_iterator& It2, string_view::const_iterator const End2)
	{
		return std::iswdigit(*It1) && std::iswdigit(*It2) ?
			number_comparer(It1, End1, It2, End2) :
			ordinal_comparer(*It1++, *It2++);
	});
}

static int compare_ordinal_numeric_icase(const string_view Str1, const string_view Str2)
{
	return per_char_compare(Str1, Str2, [](string_view::const_iterator& It1, string_view::const_iterator const End1, string_view::const_iterator& It2, string_view::const_iterator const End2)
	{
		return std::iswdigit(*It1) && std::iswdigit(*It2) ?
			number_comparer(It1, End1, It2, End2) :
			ordinal_comparer(upper(*It1++), upper(*It2++));
	});
}

static auto create_alt_sort_table()
{
	static_assert(sizeof(wchar_t) == 2, "4 GB for a sort table is too much, rewrite it.");
	static const auto TableSize = std::numeric_limits<wchar_t>::max() + 1;
	static wchar_t alt_sort_table[TableSize];
	std::vector<wchar_t> chars(TableSize);
	std::iota(ALL_RANGE(chars), 0);
	std::sort(chars.begin() + 1, chars.end(), [](wchar_t a, wchar_t b)
	{
		if (const auto Result = CompareString(LOCALE_INVARIANT, 0, &a, 1, &b, 1))
			return Result - 2 < 0;

		return static_cast<int>(a) < static_cast<int>(b);
	});

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

static int invariant_comparer(wchar_t Char1, wchar_t Char2)
{
	static const auto SortTable = create_alt_sort_table();
	return SortTable[Char1] - SortTable[Char2];
}

static int compare_invariant(const string_view Str1, const string_view Str2)
{
	return per_char_compare(Str1, Str2, [&](string_view::const_iterator& It1, string_view::const_iterator, string_view::const_iterator& It2, string_view::const_iterator)
	{
		return invariant_comparer(*It1++, *It2++);
	});
}

static int compare_invariant_icase(const string_view Str1, const string_view Str2)
{
	return per_char_compare(Str1, Str2, [&](string_view::const_iterator& It1, string_view::const_iterator, string_view::const_iterator& It2, string_view::const_iterator)
	{
		return invariant_comparer(upper(*It1++), upper(*It2++));
	});
}

static int compare_invariant_numeric(const string_view Str1, const string_view Str2)
{
	return per_char_compare(Str1, Str2, [&](string_view::const_iterator& It1, string_view::const_iterator const End1, string_view::const_iterator& It2, string_view::const_iterator const End2)
	{
		return std::iswdigit(*It1) && std::iswdigit(*It2)?
			number_comparer(It1, End1, It2, End2) :
			invariant_comparer(*It1++, *It2++);
	});
}

static int compare_invariant_numeric_icase(const string_view Str1, const string_view Str2)
{
	return per_char_compare(Str1, Str2, [&](string_view::const_iterator& It1, string_view::const_iterator const End1, string_view::const_iterator& It2, string_view::const_iterator const End2)
	{
		return std::iswdigit(*It1) && std::iswdigit(*It2)?
			number_comparer(It1, End1, It2, End2) :
			invariant_comparer(upper(*It1++), upper(*It2++));
	});
}

static int compare_natural_base(const string_view Str1, const string_view Str2, const bool Numeric, const bool CaseSensitive)
{
	static const auto Windows7OrGreater = IsWindows7OrGreater();
	static const auto WindowsVistaOrGreater = Windows7OrGreater || IsWindowsVistaOrGreater();

	static const DWORD CaseFlags[][2] =
	{
		{ NORM_IGNORECASE, 0 },
		{ LINGUISTIC_IGNORECASE, NORM_LINGUISTIC_CASING }
	};

	auto Flags = SORT_STRINGSORT | CaseFlags[WindowsVistaOrGreater][CaseSensitive];

	if (Numeric)
	{
		if (Windows7OrGreater)
		{
			Flags |= SORT_DIGITSASNUMBERS;
		}
		else
		{
			// BUGBUG can we implement a linguistic numeric sort manually?
			return (CaseSensitive? compare_invariant_numeric : compare_invariant_numeric_icase)(Str1, Str2);
		}
	}

	if (const auto Result = CompareString(LOCALE_USER_DEFAULT, Flags, Str1.data(), static_cast<int>(Str1.size()), Str2.data(), static_cast<int>(Str2.size())))
		return Result - 2;

	static const decltype(&string_sort::compare) FallbackComparers[2][2] =
	{
		{ compare_invariant_icase, compare_invariant },
		{ compare_invariant_numeric_icase, compare_invariant_numeric },
	};

	return FallbackComparers[Numeric][CaseSensitive](Str1, Str2);
}

static int compare_natural(const string_view Str1, const string_view Str2)
{
	return compare_natural_base(Str1, Str2, false, true);
}

static int compare_natural_icase(const string_view Str1, const string_view Str2)
{
	return compare_natural_base(Str1, Str2, false, false);
}

static int compare_natural_numeric(const string_view Str1, const string_view Str2)
{
	return compare_natural_base(Str1, Str2, true, true);
}

static int compare_natural_numeric_icase(const string_view Str1, const string_view Str2)
{
	return compare_natural_base(Str1, Str2, true, false);
}

static auto DefaultComparer = IsWindows7OrGreater()? compare_natural_numeric_icase : compare_natural_icase;

int string_sort::compare(const string_view Str1, const string_view Str2)
{
	return DefaultComparer(Str1, Str2);
}

void string_sort::adjust_comparer()
{
	static const decltype(&compare) Comparers[][2][2] =
	{
		{
			{ compare_ordinal_icase, compare_ordinal },
			{ compare_ordinal_numeric_icase, compare_ordinal_numeric },
		},
		{
			{ compare_invariant_icase, compare_invariant },
			{ compare_invariant_numeric_icase, compare_invariant_numeric },
		},
		{
			{ compare_natural_icase, compare_natural },
			{ compare_natural_numeric_icase, compare_natural_numeric },
		},
	};

	const auto CollationIdex = std::clamp<size_t>(0, Global->Opt->Sort.Collation, std::size(Comparers) - 1);

	DefaultComparer = Comparers[CollationIdex][Global->Opt->Sort.DigitsAsNumbers][Global->Opt->Sort.CaseSensitive];
}

int string_sort::keyhole::compare_ordinal_numeric(string_view const Str1, string_view const Str2)
{
	return ::compare_ordinal_numeric(Str1, Str2);
}

#ifdef ENABLE_TESTS

#include "testing.hpp"

TEST_CASE("strings.sorting")
{
	static const struct
	{
		string_view Str1, Str2;
		int CaseResult;
		int IcaseResult;
	}
	Tests[]
	{
		{ {},          {},                 0,  0, },
		{ {},          L"a"sv,            -1, -1, },
		{ L"a"sv,      L"a"sv,             0,  0, },
		{ L"a"sv,      L"A"sv,             1,  0, },

		{ L"0"sv,      L"1"sv,            -1, -1, },
		{ L"0"sv,      L"00"sv,            1,  1, },
		{ L"1"sv,      L"00"sv,            1,  1, },
		{ L"10"sv,     L"1"sv,             1,  1, },
		{ L"10"sv,     L"2"sv,             1,  1, },
		{ L"10"sv,     L"0100"sv,         -1, -1, },
		{ L"1"sv,      L"001"sv,           1,  1, },

		{ L"10a"sv,    L"2b"sv,            1,  1, },
		{ L"10a"sv,    L"0100b"sv,        -1, -1, },
		{ L"a1a"sv,    L"a001a"sv,         1,  1, },
		{ L"a1b2c"sv,  L"a1b2c"sv,         0,  0, },
		{ L"a01b2c"sv, L"a1b002c"sv,      -1, -1, },
		{ L"a01b3c"sv, L"a1b002"sv,       -1, -1, },

		{ L"10"sv,     L"01"sv,            1,  1, },
		{ L"01"sv,     L"01"sv,            0,  0, },

		{ L"A1"sv,     L"a2"sv,           -1, -1, },
		{ L"a1"sv,     L"A2"sv,            1, -1, },
	};

	const auto normalise = [](int const Result)
	{
		return Result < 0? -1 : Result > 0? 1 : 0;
	};

	const auto invert = [](int const Result)
	{
		return Result < 0? 1 : Result > 0 ? -1 : 0;
	};

	for (const auto& i: Tests)
	{
		REQUIRE(normalise(compare_invariant_numeric(i.Str1, i.Str2)) == i.CaseResult);
		REQUIRE(normalise(compare_invariant_numeric_icase(i.Str1, i.Str2)) == i.IcaseResult);

		REQUIRE(invert(compare_invariant_numeric(i.Str2, i.Str1)) == i.CaseResult);
		REQUIRE(invert(compare_invariant_numeric_icase(i.Str2, i.Str1)) == i.IcaseResult);
	}
}
#endif
