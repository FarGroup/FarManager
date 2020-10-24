/*
datetime.cpp

Функции для работы с датой и временем
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
#include "datetime.hpp"

// Internal:
#include "config.hpp"
#include "strmix.hpp"
#include "global.hpp"
#include "imports.hpp"
#include "locale.hpp"
#include "encoding.hpp"

// Platform:

// Common:
#include "common/chrono.hpp"
#include "common/from_string.hpp"

// External:
#include "format.hpp"

//----------------------------------------------------------------------------

static unsigned full_year(unsigned const Year)
{
	if (Year >= 100)
		return Year;

	DWORD TwoDigitYearMax = 0;
	if(!GetCalendarInfo(LOCALE_USER_DEFAULT, CAL_GREGORIAN, CAL_ITWODIGITYEARMAX|CAL_RETURN_NUMBER, nullptr, 0, &TwoDigitYearMax))
	{
		// Current default value (as of 4 Dec 2019 / Windows 10)
		TwoDigitYearMax = 2049;
	}

	return (TwoDigitYearMax / 100 - (Year > TwoDigitYearMax % 100? 1 : 0)) * 100 + Year;
}

static string st_time(const tm* tmPtr, const locale_names& Names, bool const is_dd_mmm_yyyy)
{
	const auto DateSeparator = locale.date_separator();

	if (is_dd_mmm_yyyy)
	{
		return format(FSTR(L"{0:2}-{1:3.3}-{2:4}"),
			tmPtr->tm_mday,
			upper(Names.Months[tmPtr->tm_mon].Short),
			tmPtr->tm_year + 1900);
	}

	const auto Format = [&](const auto FormatString)
	{
		return format(FormatString, DateSeparator, tmPtr->tm_mday, tmPtr->tm_mon + 1, tmPtr->tm_year + 1900);
	};

	switch(locale.date_format())
	{
	default:
	case date_type::ymd: return Format(FSTR(L"{3:4}{0}{2:02}{0}{1:02}"));
	case date_type::dmy: return Format(FSTR(L"{1:02}{0}{2:02}{0}{3:4}"));
	case date_type::mdy: return Format(FSTR(L"{2:02}{0}{1:02}{0}{3:4}"));
	}
}

struct time_zone_information
{
	string Name;
	std::chrono::minutes Offset;
};

static std::optional<time_zone_information> time_zone()
{
	using namespace std::chrono;

	TIME_ZONE_INFORMATION Tz;
	switch (GetTimeZoneInformation(&Tz))
	{
	case TIME_ZONE_ID_UNKNOWN:
	case TIME_ZONE_ID_STANDARD:
		return { { Tz.StandardName, -minutes(Tz.Bias + Tz.StandardBias) } };

	case TIME_ZONE_ID_DAYLIGHT:
		return { { Tz.DaylightName, -minutes(Tz.Bias + Tz.DaylightBias) } };

	case TIME_ZONE_ID_INVALID:
	default:
		return {};
	}
}

static string StrFTime(string_view const Format, const tm* Time)
{
	bool IsLocal = false;

	string Result;

	for (auto Iterator = Format.cbegin(); Iterator != Format.cend(); ++Iterator)
	{
		if (*Iterator != L'%')
		{
			Result.push_back(*Iterator);
			continue;
		}

		++Iterator;
		if (Iterator == Format.cend())
			break;

		switch (*Iterator)
		{
		case L'%':
			Result.push_back(L'%');
			break;

		case L'L':
			IsLocal = !IsLocal;
			continue;

		// Краткое имя дня недели (Sun,Mon,Tue,Wed,Thu,Fri,Sat)
		// abbreviated weekday name
		case L'a':
			Result += locale.Names(IsLocal).Weekdays[Time->tm_wday].Short;
			break;

		// Полное имя дня недели
		// full weekday name
		case L'A':
			Result += locale.Names(IsLocal).Weekdays[Time->tm_wday].Full;
			break;

		// Краткое имя месяца (Jan,Feb,...)
		// abbreviated month name
		case L'h':
		case L'b':
			Result += locale.Names(IsLocal).Months[Time->tm_mon].Short;
			break;

		// Полное имя месяца
		// full month name
		case L'B':
			Result += locale.Names(IsLocal).Months[Time->tm_mon].Full;
			break;

		//Дата и время в формате WDay Mnt  Day HH:MM:SS yyyy
		//appropriate date and time representation
		case L'c':
			// Thu Oct 07 12:37:32 1999
			format_to(Result, FSTR(L"{0} {1} {2:02} {3:02}:{4:02}:{5:02} {6:4}"),
				locale.Names(IsLocal).Weekdays[Time->tm_wday].Short,
				locale.Names(IsLocal).Months[Time->tm_mon].Short,
				Time->tm_mday, Time->tm_hour, Time->tm_min, Time->tm_sec, Time->tm_year + 1900);
			break;

		// Столетие как десятичное число (00 - 99). Например, 1992 => 19
		case L'C':
			format_to(Result, FSTR(L"{0:02}"), (Time->tm_year + 1900) / 100);
			break;

		// day of month, blank padded
		case L'e':
		// Две цифры дня месяца (01 - 31)
		// day of the month, 01 - 31
		case L'd':
			Result += *Iterator == L'e'?
				format(FSTR(L"{0:2}"), Time->tm_mday) :
				format(FSTR(L"{0:02}"), Time->tm_mday);
			break;

		// hour, 24-hour clock, blank pad
		case L'k':
		// Две цифры часа (00 - 23)
		// hour, 24-hour clock, 00 - 23
		case L'H':
			Result += *Iterator == L'k'?
				format(FSTR(L"{0:2}"), Time->tm_hour) :
				format(FSTR(L"{0:02}"), Time->tm_hour);
			break;

		// hour, 12-hour clock, 1 - 12, blank pad
		case L'l':
		// Две цифры часа (01 - 12)
		// hour, 12-hour clock, 01 - 12
		case L'I':
		{
			int I = Time->tm_hour % 12;

			if (!I)
				I=12;

			Result += *Iterator == L'l'?
				format(FSTR(L"{0:2}"), I) :
				format(FSTR(L"{0:02}"), I);
			break;
		}

		// Три цифры дня в году (001 - 366)
		// day of the year, 001 - 366
		case L'j':
			format_to(Result, FSTR(L"{0:03}"), Time->tm_yday+1);
			break;

		// Две цифры месяца, как десятичное число (1 - 12)
		// month, 01 - 12
		case L'm':
			++Iterator;
			if (Iterator == Format.cend())
				break;

			switch(*Iterator)
			{
			// %mh - Hex month digit
			case L'h':
				format_to(Result, FSTR(L"{0:X}"), Time->tm_mon + 1);
				break;

			// %m0 - ведущий 0
			case L'0':
				format_to(Result, FSTR(L"{0:02}"), Time->tm_mon + 1);
				break;

			default:
				--Iterator;
				format_to(Result, FSTR(L"{0}"), Time->tm_mon + 1);
				break;
			}
			break;

		// Две цифры минут (00 - 59)
		// minute, 00 - 59
		case L'M':
			format_to(Result, FSTR(L"{0:02}"), Time->tm_min);
			break;

		// AM или PM
		// am or pm based on 12-hour clock
		case L'p':
			Result += Time->tm_hour / 12? L"PM"sv : L"AM"sv;
			break;

		// Две цифры секунд (00 - 59)
		// second, 00 - 59
		case L'S':
			format_to(Result, FSTR(L"{0:02}"), Time->tm_sec);
			break;

		// День недели где 0 - Воскресенье (Sunday) (0 - 6)
		// weekday, Sunday == 0, 0 - 6
		case L'w':
			Result.push_back(L'0' + Time->tm_wday);
			break;

		// Две цифры номера недели, где Воскресенье (Sunday) является первым днем недели (00 - 53)
		// week of year, Sunday is first day of week
		case L'U':
		// Две цифры номера недели, где Понедельник (Monday) является первым днем недели (00 - 53)
		// week of year, Monday is first day of week
		case L'W':
		{
			int I = Time->tm_wday - (Time->tm_yday % 7);

			//I = (chr == 'W'?(!WeekFirst?((t->tm_wday+6)%7):(t->tm_wday? t->tm_wday-1:6)):(t->tm_wday)) - (t->tm_yday % 7);
			if (I<0)
				I+=7;

			format_to(Result, FSTR(L"{0:02}"), (Time->tm_yday + I - (*Iterator == L'W')) / 7);
			break;
		}

		// date as dd-bbb-YYYY
		case L'v':
		// Дата в формате mm.dd.yyyy
		// appropriate date representation
		case L'D':
		case L'x':
			Result += st_time(Time, locale.Names(IsLocal), *Iterator == L'v');
			break;

		// Время в формате HH:MM:SS
		// appropriate time representation
		case L'T':
		case L'X':
			format_to(Result, FSTR(L"{1:02}{0}{2:02}{0}{3:02}"), locale.time_separator(), Time->tm_hour, Time->tm_min, Time->tm_sec);
			break;

		// Две цифры года без столетия (00 to 99)
		// year without a century, 00 - 99
		case L'y':
			format_to(Result, FSTR(L"{0:02}"), Time->tm_year % 100);
			break;

		// Год со столетием (19yy-20yy)
		// year with century
		case L'Y':
			Result += str(1900 + Time->tm_year);
			break;

		// ISO 8601 offset from UTC in timezone
		case L'z':
			{
				const auto HHMM = []
				{
					const auto Tz = time_zone();
					if (!Tz)
						return 0h / 1h;

					using namespace std::chrono;
					const auto Offset = split_duration<hours, minutes>(Tz->Offset);
					return Offset.get<hours>() / 1h * 100 + Offset.get<minutes>() / 1min;
				}();

				format_to(Result, FSTR(L"{0:+05}"), HHMM);
			}
			break;

		// Timezone name or abbreviation
		case L'Z':
			if (const auto Tz = time_zone())
			{
				Result += Tz->Name;
			}
			break;

		// same as \n
		case L'n':
			Result.push_back(L'\n');
			break;

		// same as \t
		case L't':
			Result.push_back(L'\t');
			break;

		// time as %I:%M:%S %p
		case L'r':
			Result += StrFTime(L"%I:%M:%S %p"sv, Time);
			break;

		// time as %H:%M
		case L'R':
			Result += StrFTime(L"%H:%M"sv, Time);
			break;

		// week of year according to ISO 8601
		case L'V':
			{
				// [01,53]
				wchar_t Buffer[3];
				std::wcsftime(Buffer, std::size(Buffer), L"%V", Time);
				Result += Buffer;
			}
			break;
		}

		if (Iterator == Format.cend())
			break;
	}

	return Result;
}

string MkStrFTime(string_view const Format)
{
	const auto Time = os::chrono::nt_clock::to_time_t(os::chrono::nt_clock::now());

	_tzset();
	return StrFTime(Format.empty()? Global->Opt->Macro.strDateFormat : Format, std::localtime(&Time));
}

static void ParseTimeComponents(string_view const Src, span<const std::pair<size_t, size_t>> const Ranges, span<time_component> const Dst, time_component const Default)
{
	assert(Dst.size() == Ranges.size());
	std::transform(ALL_CONST_RANGE(Ranges), Dst.begin(), [Src, Default](const auto& i)
	{
		const auto Part = trim(Src.substr(i.first, i.second));
		return Part.empty()? Default : from_string<time_component>(Part);
	});
}

namespace
{
	template<size_t Size>
	using dt_ranges = std::array<std::pair<size_t, size_t>, Size>;

	using date_ranges = dt_ranges<3>;
	using time_ranges = dt_ranges<4>;
}

// HH:MM:SS.XXXXXXX
// 0123456789012345
// ^  ^  ^  ^
// 12 12 12 1234567
static constexpr time_ranges TimeRanges{ { {0, 2}, { 3, 2 }, { 6, 2 }, { 9, 7 } } };

static date_ranges get_date_ranges(date_type const DateFormat)
{
	switch (DateFormat)
	{
	default:
	case date_type::ymd:
		// YYYYY/MM/DD
		// 01234567890
		// ^     ^  ^
		// 12345 12 12
		return { { { 0, 5 }, { 6, 2 }, { 9, 2 } } };

	case date_type::dmy:
	case date_type::mdy:
		// DD/MM/YYYYY
		// MM/DD/YYYYY
		// 01234567890
		// ^  ^  ^
		// 12 12 12345
		return {{ { 0, 2 }, { 3, 2 }, { 6, 5 } }};
	}
}

enum date_indices { i_day, i_month, i_year };

static std::array<date_indices, 3> get_date_indices(date_type const DateFormat)
{
	switch (DateFormat)
	{
	default:
	case date_type::ymd: return { i_year, i_month, i_day };
	case date_type::dmy: return { i_day, i_month, i_year };
	case date_type::mdy: return { i_month, i_day, i_year };
	}
}

detailed_time_point parse_detailed_time_point(string_view Date, string_view Time, int DateFormat)
{
	assert(Date.size() == L"YYYYY/MM/DD"sv.size());
	time_component DateN[3];
	const auto DateRanges = get_date_ranges(static_cast<date_type>(DateFormat));
	ParseTimeComponents(Date, DateRanges, DateN, time_none);

	assert(Time.size() == L"HH:MM:SS.XXXXXXX"sv.size());
	time_component TimeN[4];
	ParseTimeComponents(Time, TimeRanges, TimeN, time_none);

	enum time_indices { i_hour, i_minute, i_second, i_tick };

	const auto Indices = get_date_indices(static_cast<date_type>(DateFormat));

	return
	{
		full_year(DateN[Indices[2]]),
		DateN[Indices[1]],
		DateN[Indices[0]],
		TimeN[i_hour],
		TimeN[i_minute],
		TimeN[i_second],
		TimeN[i_tick]
	};
}

os::chrono::time_point ParseTimePoint(string_view const Date, string_view const Time, int const DateFormat)
{
	const auto Point = parse_detailed_time_point(Date, Time, DateFormat);

	if (Point.Year == time_none || Point.Month == time_none || Point.Day == time_none)
	{
		// Year / Month / Day can't have reasonable defaults
		return {};
	}

	const auto Default = [](unsigned const Value)
	{
		// Everything else can
		return Value == time_none? 0 : Value;
	};

	SYSTEMTIME st{};

	const auto Milliseconds = Point.Hectonanosecond == time_none? time_none : os::chrono::hectonanoseconds(Point.Hectonanosecond) / 1ms;

	st.wYear         = Point.Year;
	st.wMonth        = Point.Month;
	st.wDay          = Point.Day;
	st.wHour         = Default(Point.Hour);
	st.wMinute       = Default(Point.Minute);
	st.wSecond       = Default(Point.Second);
	st.wMilliseconds = Default(Milliseconds);

	os::chrono::time_point TimePoint;
	if (!local_to_utc(st, TimePoint))
		return {};

	return TimePoint + os::chrono::hectonanoseconds(Default(Point.Hectonanosecond)) % 1ms;
}

os::chrono::duration ParseDuration(string_view const Date, string_view const Time)
{
	time_component DateN[1];
	const std::pair<size_t, size_t> DateRange[]{ { 0, Date.size() } };
	ParseTimeComponents(Date, DateRange, DateN, 0);

	time_component TimeN[4];
	ParseTimeComponents(Time, TimeRanges, TimeN, 0);

	using namespace std::chrono;
	using namespace chrono;

	return days(DateN[0]) + hours(TimeN[0]) + minutes(TimeN[1]) + seconds(TimeN[2]) + os::chrono::hectonanoseconds(TimeN[3]);
}

void ConvertDate(os::chrono::time_point const Point, string& strDateText, string& strTimeText, int const TimeLength, int const FullYear, bool const Brief, bool const TextMonth)
{
	if (Point == os::chrono::time_point{})
	{
		strDateText.clear();
		strTimeText.clear();
		return;
	}

	const auto DateFormat = locale.date_format();
	const auto DateSeparator = locale.date_separator();
	const auto TimeSeparator = locale.time_separator();
	const auto DecimalSeparator = locale.decimal_separator();

	const auto CurDateFormat = Brief && DateFormat == date_type::ymd? date_type::mdy : DateFormat;

	SYSTEMTIME st;
	if (!utc_to_local(Point, st))
		return;

	auto Letter = L""sv;

	if (TimeLength == 6)
	{
		Letter = st.wHour < 12 ? L"a"sv : L"p"sv;

		if (st.wHour > 12)
			st.wHour -= 12;

		if (!st.wHour)
			st.wHour = 12;
	}

	if (TimeLength < 7)
	{
		strTimeText = format(FSTR(L"{0:02}{1}{2:02}{3}"), st.wHour, TimeSeparator, st.wMinute, Letter);
	}
	else
	{
		strTimeText = cut_right(
			format(
				FSTR(L"{0:02}{1}{2:02}{1}{3:02}{4}{5:07}"),
				st.wHour,
				TimeSeparator,
				st.wMinute,
				st.wSecond,
				DecimalSeparator,
				(std::chrono::milliseconds(st.wMilliseconds) + Point.time_since_epoch() % 1ms) / 1_hns
			),
			TimeLength);
	}

	const auto Year = FullYear? st.wYear : st.wYear % 100;

	if (TextMonth)
	{
		const auto Format = [&](const auto FormatString)
		{
			strDateText = format(FormatString, st.wDay, locale.LocalNames().Months[st.wMonth - 1].Short, Year);
		};

		switch (CurDateFormat)
		{
		default:
		case date_type::ymd: Format(FSTR(L"{2:02} {1:3.3} {0:2}")); break;
		case date_type::dmy: Format(FSTR(L"{0:2} {1:3.3} {2:02}")); break;
		case date_type::mdy: Format(FSTR(L"{1:3.3} {0:2} {2:02}")); break;
		}
	}
	else
	{
		int p1, p2, p3;
		int w1 = 2, w2 = 2, w3 = 2;
		wchar_t f1 = L'0', f2 = L'0', f3 = FullYear == 2? L' ' : L'0';

		switch (CurDateFormat)
		{
		default:
		case date_type::ymd:
			p1 = Year;
			w1 = FullYear == 2? 5 : 2;
			using std::swap;
			swap(f1, f3);
			p2 = st.wMonth;
			p3 = st.wDay;
			break;

		case date_type::dmy:
			p1 = st.wDay;
			p2 = st.wMonth;
			p3 = Year;
			break;

		case date_type::mdy:
			p1 = st.wMonth;
			p2 = st.wDay;
			p3 = Year;
			break;
		}

		// Library doesn't support dynamic fill currently. TODO: fix it?
		wchar_t Format[] = L"{0: >{1}}{6}{2: >{3}}{6}{4: >{5}}";
		Format[3] = f1;
		Format[15] = f2;
		Format[27] = f3;
		strDateText = format(Format, p1, w1, p2, w2, p3, w3, DateSeparator);
	}

	if (Brief)
	{
		strDateText.resize(TextMonth? 6 : 5);

		SYSTEMTIME Now;
		if (utc_to_local(os::chrono::nt_clock::now(), Now) && Now.wYear != st.wYear)
			strTimeText = format(FSTR(L"{0:5}"), st.wYear);
	}
}

std::tuple<string, string> ConvertDuration(os::chrono::duration Duration)
{
	using namespace std::chrono;
	using namespace chrono;

	const auto Result = split_duration<days, hours, minutes, seconds, os::chrono::hectonanoseconds>(Duration);

	return
	{
		str(Result.get<days>() / 1_d),
		format(FSTR(L"{0:02}{4}{1:02}{4}{2:02}{5}{3:07}"),
			Result.get<hours>() / 1h,
			Result.get<minutes>() / 1min,
			Result.get<seconds>() / 1s,
			Result.get<os::chrono::hectonanoseconds>() / 1_hns,
			locale.time_separator(),
			locale.decimal_separator()
		)
	};
}

string ConvertDurationToHMS(os::chrono::duration Duration)
{
	using namespace std::chrono;

	const auto Result = split_duration<hours, minutes, seconds>(Duration);

	return format(FSTR(L"{0:02}{3}{1:02}{3}{2:02}"),
		Result.get<hours>() / 1h,
		Result.get<minutes>() / 1min,
		Result.get<seconds>() / 1s,
		locale.time_separator()
	);
}

bool utc_to_local(os::chrono::time_point UtcTime, SYSTEMTIME& LocalTime)
{
	const auto FileTime = os::chrono::nt_clock::to_filetime(UtcTime);
	SYSTEMTIME SystemTime;
	return FileTimeToSystemTime(&FileTime, &SystemTime) && SystemTimeToTzSpecificLocalTime(nullptr, &SystemTime, &LocalTime);
}

static bool local_to_utc(const SYSTEMTIME &lst, SYSTEMTIME &ust)
{
	if (imports.TzSpecificLocalTimeToSystemTime && imports.TzSpecificLocalTimeToSystemTime(nullptr, &lst, &ust))
		return true;

	TIME_ZONE_INFORMATION Tz;
	if (GetTimeZoneInformation(&Tz) != TIME_ZONE_ID_INVALID)
	{
		Tz.Bias = -Tz.Bias;
		Tz.StandardBias = -Tz.StandardBias;
		Tz.DaylightBias = -Tz.DaylightBias;
		if (SystemTimeToTzSpecificLocalTime(&Tz, &lst, &ust))
			return true;
	}

	std::tm ltm
	{
		lst.wSecond,
		lst.wMinute,
		lst.wHour,
		lst.wDay,
		lst.wMonth - 1,
		lst.wYear - 1900,
		lst.wDayOfWeek,
		-1,
		-1
	};

	if (const auto gtim = std::mktime(&ltm); gtim != static_cast<time_t>(-1))
	{
		if (const auto ptm = std::gmtime(&gtim))
		{
			ust.wYear = ptm->tm_year + 1900;
			ust.wMonth = ptm->tm_mon + 1;
			ust.wDay = ptm->tm_mday;
			ust.wHour = ptm->tm_hour;
			ust.wMinute = ptm->tm_min;
			ust.wSecond = ptm->tm_sec;
			ust.wDayOfWeek = ptm->tm_wday;
			ust.wMilliseconds = lst.wMilliseconds;
			return true;
		}
	}

	FILETIME lft, uft;
	return SystemTimeToFileTime(&lst, &lft) && LocalFileTimeToFileTime(&lft, &uft) && FileTimeToSystemTime(&uft, &ust);
}

bool local_to_utc(const SYSTEMTIME& LocalTime, os::chrono::time_point& UtcTime)
{
	SYSTEMTIME SystemUtcTime;
	if (!local_to_utc(LocalTime, SystemUtcTime))
		return false;

	FILETIME FileUtcTime;
	if (!SystemTimeToFileTime(&SystemUtcTime, &FileUtcTime))
		return false;

	UtcTime = os::chrono::nt_clock::from_filetime(FileUtcTime);
	return true;
}

time_check::time_check(mode Mode) noexcept:
	time_check(Mode, GetRedrawTimeout())
{
}

time_check::time_check(mode Mode, clock_type::duration Interval) noexcept:
	m_Begin(Mode == mode::delayed? clock_type::now() : clock_type::now() - Interval),
	m_Interval(Interval)
{
}

void time_check::reset(clock_type::time_point Value) const noexcept
{
	m_Begin = Value;
}

bool time_check::is_time() const noexcept
{
	return clock_type::now() - m_Begin > m_Interval;
}

time_check::operator bool() const noexcept
{
	const auto Current = clock_type::now();
	if (m_Interval != 0s && Current - m_Begin > m_Interval)
	{
		reset(Current);
		return true;
	}
	return false;
}

#ifdef ENABLE_TESTS

#include "testing.hpp"

TEST_CASE("datetime.parse.duration")
{
	static const struct
	{
		string_view Date, Time;
		os::chrono::duration Duration;
	}
	Tests[]
	{
		{ {},          L"  :  :  .       "sv,              0_hns, },
		{ {},          L"  :  :  .      1"sv,              1_hns, },
		{ {},          L"  :  :  . 12    "sv,              12_hns, },
		{ {},          L"  :42:  .       "sv,              42min, },
		{ {},          L"33:  :  .       "sv,              33h, },
		{ L"1"sv,      L"  :  :  .       "sv,              1_d, },
		{ L"2"sv,      L"26:  :  .       "sv,              3_d + 2h, },
		{ L"3"sv,      L"  :42:  .       "sv,              3_d + 42min, },
		{ L"512"sv,    L"12:34:56.7890123"sv,              512_d + 12h + 34min + 56s + 789ms + 123_hns, },
	};

	for (const auto& i: Tests)
	{
		REQUIRE(ParseDuration(i.Date, i.Time) == i.Duration);
		const auto& [Date, Time] = ConvertDuration(i.Duration);
		REQUIRE(ParseDuration(Date, Time) == i.Duration);
	}
}

TEST_CASE("datetime.parse.timepoint")
{
	const auto tn = time_none;

	static const struct
	{
		string_view Date, Time;
		int DateFormat;
		detailed_time_point TimePoint;
	}
	Tests[]
	{
		{ L"     /  /  "sv, L"  :  :  .       "sv, 2, { tn,   tn, tn, tn, tn, tn, tn      }, },
		{ L"  /  /     "sv, L"  :  :  .       "sv, 1, { tn,   tn, tn, tn, tn, tn, tn      }, },
		{ L"  /  /     "sv, L"  :  :  .       "sv, 0, { tn,   tn, tn, tn, tn, tn, tn      }, },
		{ L" 1234/56/78"sv, L"  :55:  .     44"sv, 2, { 1234, 56, 78, tn, 55, tn, 44      }, },
		{ L"12/34/5678 "sv, L"  :  :  .      1"sv, 1, { 5678, 34, 12, tn, tn, tn, 1       }, },
		{ L"12/34/5678 "sv, L"  :  :  . 12    "sv, 0, { 5678, 12, 34, tn, tn, tn, 12,     }, },
		{ L"3 / 5/7    "sv, L" 0:0 : 0.   0   "sv, 1, { 2007,  5,  3,  0,  0,  0, 0       }, },
		{ L"01/02/ 84  "sv, L"44:55:66.1234567"sv, 1, { 1984,  2,  1, 44, 55, 66, 1234567 }, },
		{ L" 8765/43/21"sv, L"11:22:33.4567890"sv, 2, { 8765, 43, 21, 11, 22, 33, 4567890 }, },
	};

	for (const auto& i: Tests)
	{
		const auto Result = parse_detailed_time_point(i.Date, i.Time, i.DateFormat);

		REQUIRE(Result.Year == i.TimePoint.Year);
		REQUIRE(Result.Month == i.TimePoint.Month);
		REQUIRE(Result.Day == i.TimePoint.Day);
		REQUIRE(Result.Hour == i.TimePoint.Hour);
		REQUIRE(Result.Minute == i.TimePoint.Minute);
		REQUIRE(Result.Second == i.TimePoint.Second);
		REQUIRE(Result.Hectonanosecond == i.TimePoint.Hectonanosecond);
	}
}

TEST_CASE("datetime.ConvertDuration")
{
	static const struct
	{
		os::chrono::duration Duration;
		string_view Days, Timestamp, HMS;
	}
	Tests[]
	{
		{ 0s,                                         L"0"sv, L"00:00:00.0000000"sv, L"00:00:00"sv,  },
		{ 7_d,                                        L"7"sv, L"00:00:00.0000000"sv, L"168:00:00"sv, },
		{ 2_d +  7h + 13min +  47s +   7654321_hns,   L"2"sv, L"07:13:47.7654321"sv, L"55:13:47"sv,  },
		{ 3_d + 25h + 81min + 120s + 123456789_hns,   L"4"sv, L"02:23:12.3456789"sv, L"98:23:12"sv,  },
	};

	for (const auto& i: Tests)
	{
		const auto [Days, Timestamp] = ConvertDuration(i.Duration);
		REQUIRE(i.Days == Days);
		// Time & decimal separators are locale-specific, so let's compare numbers only
		REQUIRE(std::equal(ALL_CONST_RANGE(i.Timestamp), ALL_CONST_RANGE(Timestamp), [](wchar_t const a, wchar_t const b)
		{
			return a == b || !std::iswdigit(a);
		}));
		const auto HMS = ConvertDurationToHMS(i.Duration);
		REQUIRE(i.HMS == HMS);
	}
}
#endif
