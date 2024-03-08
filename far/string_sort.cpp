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
#include "imports.hpp"

// Platform:

// Common:

// External:

//----------------------------------------------------------------------------

static std::strong_ordering per_char_compare(const string_view Str1, const string_view Str2, const auto Comparer)
{
	// VS2019 bug - 'classic' CTAD breaks the compilation here
	auto Iterator = std::pair(Str1.cbegin(), Str2.cbegin());
	const auto End = std::pair(Str1.cend(), Str2.cend());

	while (Iterator.first != End.first && Iterator.second != End.second)
	{
		if (const auto Result = Comparer(Iterator.first, End.first, Iterator.second, End.second); !std::is_eq(Result))
			return Result;
	}

	if (Iterator == End)
	{
		const auto Size1 = Iterator.first - Str1.cbegin();
		const auto Size2 = Iterator.second - Str2.cbegin();

		return Size1 <=> Size2;
	}

	return Iterator.first == End.first? std::strong_ordering::less : std::strong_ordering::greater;
}

struct numeric_comparer
{
	auto operator()(string_view::const_iterator& It1, string_view::const_iterator const End1, string_view::const_iterator& It2, string_view::const_iterator const End2) const
	{
		const auto IsZero = [](wchar_t Char) { return Char == L'0'; };

		const auto Begin1 = It1;
		const auto Begin2 = It2;

		It1 = std::find_if_not(It1, End1, IsZero);
		It2 = std::find_if_not(It2, End2, IsZero);

		// 00 is less than 0 in Windows numeric sort implementation
		const auto ZerosResult = It2 - Begin2 <=> It1 - Begin1;

		if (It1 == End1 && It2 == End2)
			return ZerosResult;

		auto Result = std::strong_ordering::equal;
		while (It1 != End1 && It2 != End2 && std::iswdigit(*It1) && std::iswdigit(*It2))
		{
			if (std::is_eq(Result) && *It1 != *It2)
				Result = *It1 <=> *It2;

			++It1;
			++It2;
		}

		const auto EndOrNonDigit1 = It1 == End1 || !std::iswdigit(*It1);
		const auto EndOrNonDigit2 = It2 == End2 || !std::iswdigit(*It2);

		if (EndOrNonDigit1 && EndOrNonDigit2)
			return std::is_eq(Result)? ZerosResult : Result;

		return EndOrNonDigit1? std::strong_ordering::less : std::strong_ordering::greater;
	}
};

struct ordinal_comparer
{
	auto operator()(wchar_t const Char1, wchar_t const Char2) const
	{
		return Char1 <=> Char2;
	}

	static constexpr bool ignore_case = false;
};

struct ordinal_comparer_icase
{
	auto operator()(wchar_t const Char1, wchar_t const Char2) const
	{
		if (Char1 == Char2)
			return std::strong_ordering::equal;

		return ordinal_comparer{}(upper(Char1), upper(Char2));
	}

	static constexpr bool ignore_case = true;
};

static auto windows_to_std(int const Value)
{
	switch (Value)
	{
	case CSTR_LESS_THAN:     return std::strong_ordering::less;
	case CSTR_EQUAL:         return std::strong_ordering::equal;
	case CSTR_GREATER_THAN:  return std::strong_ordering::greater;
	default:
		std::unreachable();
	}
}

template<typename Comparer>
static auto compare_ordinal_t(const string_view Str1, const string_view Str2)
{
	if (imports.CompareStringOrdinal)
	{
		if (const auto Result = imports.CompareStringOrdinal(Str1.data(), static_cast<int>(Str1.size()), Str2.data(), static_cast<int>(Str2.size()), Comparer::ignore_case))
			return windows_to_std(Result);
	}

	return per_char_compare(Str1, Str2, [](string_view::const_iterator& It1, string_view::const_iterator, string_view::const_iterator& It2, string_view::const_iterator)
	{
		return Comparer{}(*It1++, *It2++);
	});
}

static auto compare_ordinal(const string_view Str1, const string_view Str2)
{
	return compare_ordinal_t<ordinal_comparer>(Str1, Str2);
}

static auto compare_ordinal_icase(const string_view Str1, const string_view Str2)
{
	return compare_ordinal_t<ordinal_comparer_icase>(Str1, Str2);
}

template<typename Comparer>
static auto compare_numeric_t(const string_view Str1, const string_view Str2)
{
	return per_char_compare(Str1, Str2, [](string_view::const_iterator& It1, string_view::const_iterator const End1, string_view::const_iterator& It2, string_view::const_iterator const End2)
	{
		return std::iswdigit(*It1) && std::iswdigit(*It2)?
			numeric_comparer{}(It1, End1, It2, End2) :
			Comparer{}(*It1++, *It2++);
	});
}

static auto compare_ordinal_numeric(const string_view Str1, const string_view Str2)
{
	return compare_numeric_t<ordinal_comparer>(Str1, Str2);
}

static auto compare_ordinal_numeric_icase(const string_view Str1, const string_view Str2)
{
	return compare_numeric_t<ordinal_comparer_icase>(Str1, Str2);
}

static const auto& create_alt_sort_table()
{
	static_assert(sizeof(wchar_t) == 2, "4 GB for a sort table is too much, rewrite it.");
	static const auto TableSize = std::numeric_limits<wchar_t>::max() + 1;
	static wchar_t alt_sort_table[TableSize];

	const auto Iota = std::views::iota(0, TableSize);
	std::vector<wchar_t> chars(ALL_CONST_RANGE(Iota));
	std::ranges::sort(chars | std::views::drop(1), [](wchar_t a, wchar_t b)
	{
		if (const auto Result = CompareString(LOCALE_INVARIANT, 0, &a, 1, &b, 1))
			return std::is_lt(windows_to_std(Result));

		return a < b;
	});

	int u_beg = 0, u_end = 0xffff;
	for (const auto ic: std::views::iota(0, TableSize))
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
	for (const auto ic: std::views::iota(u_beg, u_end + 1)) // uppercase first
	{
		if (is_upper(chars[ic]))
			alt_sort_table[chars[ic]] = static_cast<wchar_t>(cc++);
	}

	for (const auto ic: std::views::iota(u_beg, u_end + 1)) // then not uppercase
	{
		if (!is_upper(chars[ic]))
			alt_sort_table[chars[ic]] = static_cast<wchar_t>(cc++);
	}
	assert(cc == u_end+1);
	return alt_sort_table;
}

struct invariant_comparer
{
	auto operator()(wchar_t const Char1, wchar_t const Char2) const
	{
		if (Char1 == Char2)
			return std::strong_ordering::equal;

		static const auto& SortTable = create_alt_sort_table();
		return SortTable[Char1] <=> SortTable[Char2];
	}
};

struct invariant_comparer_icase
{
	auto operator()(wchar_t const Char1, wchar_t const Char2) const
	{
		if (Char1 == Char2)
			return std::strong_ordering::equal;

		return invariant_comparer{}(upper(Char1), upper(Char2));
	}
};

static auto compare_invariant(const string_view Str1, const string_view Str2)
{
	return per_char_compare(Str1, Str2, [&](string_view::const_iterator& It1, string_view::const_iterator, string_view::const_iterator& It2, string_view::const_iterator)
	{
		return invariant_comparer{}(*It1++, *It2++);
	});
}

static auto compare_invariant_icase(const string_view Str1, const string_view Str2)
{
	return per_char_compare(Str1, Str2, [&](string_view::const_iterator& It1, string_view::const_iterator, string_view::const_iterator& It2, string_view::const_iterator)
	{
		return invariant_comparer{}(upper(*It1++), upper(*It2++));
	});
}

static auto compare_invariant_numeric(const string_view Str1, const string_view Str2)
{
	return compare_numeric_t<invariant_comparer>(Str1, Str2);
}

static auto compare_invariant_numeric_icase(const string_view Str1, const string_view Str2)
{
	return compare_numeric_t<invariant_comparer_icase>(Str1, Str2);
}

static auto compare_natural_base(const string_view Str1, const string_view Str2, const bool Numeric, const bool CaseSensitive)
{
	static const auto Windows7OrGreater = IsWindows7OrGreater();
	static const auto WindowsVistaOrGreater = Windows7OrGreater || IsWindowsVistaOrGreater();

	static const DWORD CaseFlags[][2]
	{
		{ NORM_IGNORECASE,       0 },
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
		return windows_to_std(Result);

	static const decltype(&string_sort::compare) FallbackComparers[2][2]
	{
		{ compare_invariant_icase,         compare_invariant },
		{ compare_invariant_numeric_icase, compare_invariant_numeric },
	};

	return FallbackComparers[Numeric][CaseSensitive](Str1, Str2);
}

static auto compare_natural(const string_view Str1, const string_view Str2)
{
	return compare_natural_base(Str1, Str2, false, true);
}

static auto compare_natural_icase(const string_view Str1, const string_view Str2)
{
	return compare_natural_base(Str1, Str2, false, false);
}

static auto compare_natural_numeric(const string_view Str1, const string_view Str2)
{
	return compare_natural_base(Str1, Str2, true, true);
}

static auto compare_natural_numeric_icase(const string_view Str1, const string_view Str2)
{
	return compare_natural_base(Str1, Str2, true, false);
}

static auto DefaultComparer = IsWindows7OrGreater()? compare_natural_numeric_icase : compare_natural_icase;

std::strong_ordering string_sort::compare(const string_view Str1, const string_view Str2)
{
	return DefaultComparer(Str1, Str2);
}

void string_sort::adjust_comparer(size_t const Collation, bool const CaseSensitive, bool const DigitsAsNumbers)
{
	static const decltype(&compare) Comparers[][2][2]
	{
		{
			{ compare_ordinal_icase,           compare_ordinal },
			{ compare_ordinal_numeric_icase,   compare_ordinal_numeric },
		},
		{
			{ compare_invariant_icase,         compare_invariant },
			{ compare_invariant_numeric_icase, compare_invariant_numeric },
		},
		{
			{ compare_natural_icase,           compare_natural },
			{ compare_natural_numeric_icase,   compare_natural_numeric },
		},
	};

	const auto CollationIdex = std::clamp(0uz, Collation, std::size(Comparers) - 1uz);

	DefaultComparer = Comparers[CollationIdex][DigitsAsNumbers][CaseSensitive];
}

bool string_sort::less_icase_t::operator()(string_view Str1, string_view Str2) const
{
	return std::is_lt(compare_ordinal_icase(Str1, Str2));
}

std::strong_ordering string_sort::keyhole::compare_ordinal_icase(string_view const Str1, string_view const Str2)
{
	return ::compare_ordinal_icase(Str1, Str2);
}

std::strong_ordering string_sort::keyhole::compare_ordinal_numeric(string_view const Str1, string_view const Str2)
{
	return ::compare_ordinal_numeric(Str1, Str2);
}

#ifdef ENABLE_TESTS

#include "testing.hpp"

TEST_CASE("strings.sorting")
{
	constexpr auto
		lt = std::strong_ordering::less,
		eq = std::strong_ordering::equal,
		gt = std::strong_ordering::greater;

	static const struct
	{
		string_view Str1, Str2;
		std::strong_ordering CaseResult, IcaseResult;
	}
	Tests[]
	{
		{ {},          {},                eq, eq, },
		{ {},          L"a"sv,            lt, lt, },
		{ L"a"sv,      L"a"sv,            eq, eq, },
		{ L"a"sv,      L"A"sv,            gt, eq, },

		{ L"0"sv,      L"1"sv,            lt, lt, },
		{ L"0"sv,      L"00"sv,           gt, gt, },
		{ L"1"sv,      L"00"sv,           gt, gt, },
		{ L"10"sv,     L"1"sv,            gt, gt, },
		{ L"10"sv,     L"2"sv,            gt, gt, },
		{ L"10"sv,     L"0100"sv,         lt, lt, },
		{ L"1"sv,      L"001"sv,          gt, gt, },

		{ L"10a"sv,    L"2b"sv,           gt, gt, },
		{ L"10a"sv,    L"0100b"sv,        lt, lt, },
		{ L"a1a"sv,    L"a001a"sv,        gt, gt, },
		{ L"a1b2c"sv,  L"a1b2c"sv,        eq, eq, },
		{ L"a01b2c"sv, L"a1b002c"sv,      lt, lt, },
		{ L"a01b3c"sv, L"a1b002"sv,       lt, lt, },

		{ L"10"sv,     L"01"sv,           gt, gt, },
		{ L"01"sv,     L"01"sv,           eq, eq, },

		{ L"A1"sv,     L"a2"sv,           lt, lt, },
		{ L"a1"sv,     L"A2"sv,           gt, lt, },
	};

	const auto invert = [&](std::strong_ordering const Result)
	{
		return Result == lt? gt : Result == gt? lt : eq;
	};

	for (const auto& i: Tests)
	{
		REQUIRE(compare_invariant_numeric(i.Str1, i.Str2) == i.CaseResult);
		REQUIRE(compare_invariant_numeric_icase(i.Str1, i.Str2) == i.IcaseResult);

		REQUIRE(invert(compare_invariant_numeric(i.Str2, i.Str1)) == i.CaseResult);
		REQUIRE(invert(compare_invariant_numeric_icase(i.Str2, i.Str1)) == i.IcaseResult);
	}
}
#endif
