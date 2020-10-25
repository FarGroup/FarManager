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
#include "global.hpp"

// Platform:

// Common:
#include "common/algorithm.hpp"
#include "common/string_utils.hpp"
#include "common/view/enumerate.hpp"

// External:

//----------------------------------------------------------------------------

NIFTY_DEFINE(detail::locale, locale);

namespace detail
{
	date_type locale::date_format() const
	{
		refresh();
		return m_DateFormat;
	}

	int locale::digits_grouping() const
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
		m_Valid = false;
	}

	void locale::refresh() const
	{
		if (m_Valid)
			return;

		{
			int DateFormat;
			if (!os::get_locale_value(LOCALE_USER_DEFAULT, LOCALE_IDATE, DateFormat))
				DateFormat = 2;

			switch (DateFormat)
			{
			case 0:
				m_DateFormat = date_type::mdy;
				break;

			case 1:
				m_DateFormat = date_type::dmy;
				break;

			case 2:
			default:
				m_DateFormat = date_type::ymd;
				break;
			}
		}

		{
			string Grouping;
			if (os::get_locale_value(LOCALE_USER_DEFAULT, LOCALE_SGROUPING, Grouping))
			{
				m_DigitsGrouping = 0;
				for (const auto i: Grouping)
				{
					if (in_closed_range(L'1', i, L'9'))
						m_DigitsGrouping = m_DigitsGrouping * 10 + i - L'0';
				}

				if (!ends_with(Grouping, L";0"sv))
					m_DigitsGrouping *= 10;
			}
			else
			{
				m_DigitsGrouping = 3;
			}
		}

		{
			string Value;
			if (os::get_locale_value(LOCALE_USER_DEFAULT, LOCALE_SSHORTDATE, Value))
			{
				size_t pos = 0;
				const auto Weekday = L"ddd"sv;
				const auto dMyg = L"dMyg"sv;
				if (starts_with(Value, Weekday))
				{
					pos = Value.find_first_not_of(L'd', Weekday.size());
					// skip separators
					pos = Value.find_first_of(dMyg, pos);
				}

				// find separator
				pos = Value.find_first_not_of(dMyg, pos);
				if (pos != Value.npos)
					m_DateSeparator = Value[pos];
			}
			else
			{
				m_DateSeparator = os::get_locale_value(LOCALE_USER_DEFAULT, LOCALE_SDATE, Value) && !Value.empty()? Value.front() : L'/';
			}
		}

		{
			string Value;
			m_TimeSeparator = os::get_locale_value(LOCALE_USER_DEFAULT, LOCALE_STIME, Value) && !Value.empty()? Value.front() : L':';
		}

		{
			if (Global && Global->Opt && Global->Opt->FormatNumberSeparators.size() > 1)
			{
				m_DecimalSeparator = Global->Opt->FormatNumberSeparators[1];
			}
			else
			{
				string Value;
				m_DecimalSeparator = os::get_locale_value(LOCALE_USER_DEFAULT, LOCALE_SDECIMAL, Value) && !Value.empty()? Value.front() : L'.';
			}
		}

		{
			if (Global && Global->Opt && !Global->Opt->FormatNumberSeparators.empty())
			{
				m_ThousandSeparator = Global->Opt->FormatNumberSeparators[0];
			}
			else
			{
				string Value;
				m_ThousandSeparator = os::get_locale_value(LOCALE_USER_DEFAULT, LOCALE_STHOUSAND, Value) && !Value.empty()? Value.front() : L',';
			}
		}

		{
			const auto InitNames = [](int Language, locale_names& Names)
			{
				// LOCALE_S[ABBREV]DAYNAME<1-7> indexes start from Monday, remap to Sunday to make them compatible with tm::tm_wday
				static const LCTYPE DayIndexes[] = { LOCALE_SDAYNAME7, LOCALE_SDAYNAME1, LOCALE_SDAYNAME2, LOCALE_SDAYNAME3, LOCALE_SDAYNAME4, LOCALE_SDAYNAME5, LOCALE_SDAYNAME6 };
				static const LCTYPE ShortDayIndexes[] = { LOCALE_SABBREVDAYNAME7, LOCALE_SABBREVDAYNAME1, LOCALE_SABBREVDAYNAME2, LOCALE_SABBREVDAYNAME3, LOCALE_SABBREVDAYNAME4, LOCALE_SABBREVDAYNAME5, LOCALE_SABBREVDAYNAME6 };

				const LCID CurLCID = MAKELCID(MAKELANGID(Language, SUBLANG_DEFAULT), SORT_DEFAULT);

				for (const auto& [i, index]: enumerate(Names.Months))
				{
					// BUGBUG check result
					(void)os::get_locale_value(CurLCID, LCTYPE(LOCALE_SMONTHNAME1 + index), i.Full);
					// BUGBUG check result
					(void)os::get_locale_value(CurLCID, LCTYPE(LOCALE_SABBREVMONTHNAME1 + index), i.Short);
				}

				for (const auto& [i, index]: enumerate(Names.Weekdays))
				{
					// BUGBUG check result
					(void)os::get_locale_value(CurLCID, DayIndexes[index], i.Full);
					// BUGBUG check result
					(void)os::get_locale_value(CurLCID, ShortDayIndexes[index], i.Short);
				}
			};

			InitNames(LANG_NEUTRAL, m_LocalNames);
			InitNames(LANG_ENGLISH, m_EnglishNames);
		}

		m_Valid = true;
	}
}
