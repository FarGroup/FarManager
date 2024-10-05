/*
locale.cpp

*/
/*
Copyright © 2014 Far Group
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
#include "locale.hpp"

// Internal:
#include "config.hpp"
#include "console.hpp"
#include "global.hpp"
#include "encoding.hpp"
#include "log.hpp"

// Platform:
#include "platform.hpp"

// Common:
#include "common/algorithm.hpp"
#include "common/string_utils.hpp"
#include "common/view/enumerate.hpp"

// External:

//----------------------------------------------------------------------------

NIFTY_DEFINE(detail::locale, locale);

static auto is_cjk_codepage(uintptr_t const Codepage)
{
	enum: unsigned
	{
		chinese_s     = 936,
		chinese_t     = 950,
		japanese      = 932,
		korean_hangul = 949,
		korean_johab  = 1361,
	};

	return any_of(Codepage, chinese_s, chinese_t, japanese, korean_hangul, korean_johab);
}

static auto get_is_cjk()
{
	return
		any_of(extract_integer<BYTE, 0>(GetUserDefaultLCID()), LANG_CHINESE, LANG_JAPANESE, LANG_KOREAN) ||
		is_cjk_codepage(encoding::codepage::oem()) ||
		is_cjk_codepage(encoding::codepage::ansi()) ||
		is_cjk_codepage(console.GetOutputCodepage());
}

static auto get_date_format()
{
	int DateFormat;
	if (!os::get_locale_value(LOCALE_USER_DEFAULT, LOCALE_IDATE, DateFormat))
	{
		LOGWARNING(L"get_locale_value(LOCALE_IDATE): {}"sv, os::last_error());
		DateFormat = 2;
	}

	switch (DateFormat)
	{
	case 0:  return date_type::mdy;
	case 1:  return date_type::dmy;
	default: return date_type::ymd;
	}
}

static auto get_digits_grouping()
{
	string Grouping;
	if (!os::get_locale_value(LOCALE_USER_DEFAULT, LOCALE_SGROUPING, Grouping))
	{
		LOGWARNING(L"get_locale_value(LOCALE_SGROUPING): {}"sv, os::last_error());
		return 3;
	}

	int DigitsGrouping = 0;
	for (const auto i: Grouping)
	{
		if (in_closed_range(L'1', i, L'9'))
			DigitsGrouping = DigitsGrouping * 10 + i - L'0';
	}

	if (!Grouping.ends_with(L";0"sv))
		DigitsGrouping *= 10;

	return DigitsGrouping;
}

static auto get_date_separator()
{
	const auto KnownSeparators = L"/-."sv;

	if (string Value; os::get_locale_value(LOCALE_USER_DEFAULT, LOCALE_SSHORTDATE, Value))
	{
		size_t StartPos = 0;
		const auto Weekday = L"ddd"sv;
		const auto dMyg = L"dMyg"sv;

		// Skip day of week, if any
		if (Value.starts_with(Weekday))
		{
			StartPos = Value.find_first_not_of(L'd', Weekday.size());
			// skip separators
			StartPos = Value.find_first_of(dMyg, StartPos);
		}

		// Find a preferable separator
		if (const auto SeparatorPos = Value.find_first_of(KnownSeparators, StartPos); SeparatorPos != Value.npos)
			return Value[SeparatorPos];

		// Find any other separator
		if (const auto SeparatorPos = Value.find_first_not_of(dMyg, StartPos); SeparatorPos != Value.npos)
			return Value[SeparatorPos];
	}
	else
	{
		LOGDEBUG(L"get_locale_value(LOCALE_SSHORTDATE): {}"sv, os::last_error());
	}

	if (string Value; os::get_locale_value(LOCALE_USER_DEFAULT, LOCALE_SDATE, Value))
	{
		return Value.empty()? KnownSeparators.front() : Value.front();
	}
	else
	{
		LOGWARNING(L"get_locale_value(LOCALE_SDATE): {}"sv, os::last_error());
	}

	return KnownSeparators.front();
}

static auto get_time_separator()
{
	string Value;
	if (!os::get_locale_value(LOCALE_USER_DEFAULT, LOCALE_STIME, Value))
	{
		LOGWARNING(L"get_locale_value(LOCALE_STIME): {}"sv, os::last_error());
	}

	return Value.empty()? L':' : Value.front();
}

static auto get_decimal_separator()
{
	if (Global && Global->Opt && Global->Opt->FormatNumberSeparators.size() > 1)
	{
		return Global->Opt->FormatNumberSeparators[1];
	}

	string Value;
	if (!os::get_locale_value(LOCALE_USER_DEFAULT, LOCALE_SDECIMAL, Value))
	{
		LOGWARNING(L"get_locale_value(LOCALE_SDECIMAL): {}"sv, os::last_error());
	}

	return Value.empty()? L'.' : Value.front();
}

static auto get_thousand_separator()
{
	if (Global && Global->Opt && !Global->Opt->FormatNumberSeparators.empty())
	{
		return Global->Opt->FormatNumberSeparators[0];
	}

	string Value;
	if (!os::get_locale_value(LOCALE_USER_DEFAULT, LOCALE_STHOUSAND, Value))
	{
		LOGWARNING(L"get_locale_value(LOCALE_STHOUSAND): {}"sv, os::last_error());
	}

	return Value.empty()? L',' : Value.front();
}

static auto get_month_day_names(int const Language)
{
	locale_names Names;

	const LCID CurLCID = MAKELCID(MAKELANGID(Language, SUBLANG_DEFAULT), SORT_DEFAULT);

	struct init
	{
		LCTYPE Index, AbbrIndex;
		string_view Default;
	};

	static const init MonthInit[]
	{
#define MONTHNAME(n) LOCALE_SMONTHNAME ## n, LOCALE_SABBREVMONTHNAME ## n
		{ MONTHNAME(1),  L"January"sv },
		{ MONTHNAME(2),  L"February"sv },
		{ MONTHNAME(3),  L"March"sv },
		{ MONTHNAME(4),  L"April"sv },
		{ MONTHNAME(5),  L"May"sv },
		{ MONTHNAME(6),  L"June"sv },
		{ MONTHNAME(7),  L"July"sv },
		{ MONTHNAME(8),  L"August"sv },
		{ MONTHNAME(9),  L"September"sv },
		{ MONTHNAME(10), L"October"sv },
		{ MONTHNAME(11), L"November"sv },
		{ MONTHNAME(12), L"December"sv },
#undef MONTHNAME
	};

	for (const auto& [Init, Dest]: zip(MonthInit, Names.Months))
	{
		if (!os::get_locale_value(CurLCID, Init.Index, Dest.Full))
		{
			Dest.Full = Init.Default;
			LOGWARNING(L"get_locale_value(LOCALE_SMONTHNAME{}): {}"sv, Init.Index, os::last_error());
		}

		if (!os::get_locale_value(CurLCID, Init.AbbrIndex, Dest.Short))
		{
			Dest.Full = Init.Default.substr(0, 3);
			LOGWARNING(L"get_locale_value(LOCALE_SABBREVMONTHNAME{}): {}"sv, Init.AbbrIndex, os::last_error());
		}
	}

	// LOCALE_S[ABBREV]DAYNAME<1-7> indexes start from Monday, remap to Sunday to make them compatible with tm::tm_wday
	static const init DayInit[]
	{
#define DAYNAME(n) LOCALE_SDAYNAME ## n, LOCALE_SABBREVDAYNAME ## n
		{ DAYNAME(7), L"Sunday"sv },
		{ DAYNAME(1), L"Monday"sv },
		{ DAYNAME(2), L"Tuesday"sv },
		{ DAYNAME(3), L"Wednesday"sv },
		{ DAYNAME(4), L"Thursday"sv },
		{ DAYNAME(5), L"Friday"sv },
		{ DAYNAME(6), L"Saturday"sv },
#undef DAYNAME
	};

	for (const auto& [Init, Dest]: zip(DayInit, Names.Weekdays))
	{
		if (!os::get_locale_value(CurLCID, Init.Index, Dest.Full))
		{
			Dest.Full = Init.Default;
			LOGWARNING(L"get_locale_value({}): {}"sv, Init.Index, os::last_error());
		}

		if (!os::get_locale_value(CurLCID, Init.AbbrIndex, Dest.Short))
		{
			Dest.Full = Init.Default.substr(0, 3);
			LOGWARNING(L"get_locale_value({}): {}"sv, Init.AbbrIndex, os::last_error());
		}
	}

	return Names;
}

namespace detail
{
	locale::locale(bool const Invariant):
		m_IsInvariant(Invariant)
	{
		if (m_IsInvariant)
		{
			m_IsCJK = false;
			m_DateFormat = date_type::mdy;
			m_DigitsGrouping = 3;
			m_DateSeparator = L'/';
			m_TimeSeparator = L':';
			m_DecimalSeparator = L'.';
			m_ThousandSeparator = L',';

			m_LocalNames = m_EnglishNames = get_month_day_names(LANG_ENGLISH);

			m_Valid = true;
		}
	}

	bool locale::is_invariant() const
	{
		return m_IsInvariant;
	}

	bool locale::is_cjk() const
	{
		refresh();
		return m_IsCJK;
	}

	date_type locale::date_format() const
	{
		refresh();
		return m_DateFormat;
	}

	unsigned locale::digits_grouping() const
	{
		refresh();
		return m_DigitsGrouping;
	}

	wchar_t locale::date_separator() const
	{
		refresh();
		return m_DateSeparator;
	}

	wchar_t locale::time_separator() const
	{
		refresh();
		return m_TimeSeparator;
	}

	wchar_t locale::decimal_separator() const
	{
		refresh();
		return m_DecimalSeparator;
	}

	wchar_t locale::thousand_separator() const
	{
		refresh();
		return m_ThousandSeparator;
	}

	const locale_names& locale::LocalNames() const
	{
		refresh();
		return m_LocalNames;
	}

	const locale_names& locale::EnglishNames() const
	{
		refresh();
		return m_EnglishNames;
	}

	const locale_names& locale::Names(bool const Local) const
	{
		refresh();
		return Local? m_LocalNames : m_EnglishNames;
	}

	void locale::invalidate()
	{
		if (m_IsInvariant)
			return;

		m_Valid = false;

		if (Global)
			Global->CurrentTime.update(true);

		LOGINFO(L"Locale cache invalidated"sv);
	}

	void locale::refresh() const
	{
		if (m_Valid || m_IsInvariant)
			return;

		m_IsCJK = get_is_cjk();
		m_DateFormat = get_date_format();
		m_DigitsGrouping = get_digits_grouping();
		m_DateSeparator = get_date_separator();
		m_TimeSeparator = get_time_separator();
		m_DecimalSeparator = get_decimal_separator();
		m_ThousandSeparator = get_thousand_separator();

		m_LocalNames = get_month_day_names(LANG_NEUTRAL);
		m_EnglishNames = get_month_day_names(LANG_ENGLISH);

		m_Valid = true;
	}
}

detail::locale const& invariant_locale()
{
	static const detail::locale s_InvariantLocale(true);
	return s_InvariantLocale;
}
