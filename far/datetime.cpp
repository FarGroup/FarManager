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
#include "global.hpp"
#include "locale.hpp"

// Platform:

// Common:
#include "common/chrono.hpp"
#include "common/from_string.hpp"
#include "common/view/zip.hpp"

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

static string st_time(const tm& Time, const locale_names& Names, bool const is_dd_mmm_yyyy)
{
	const auto DateSeparator = locale.date_separator();

	if (is_dd_mmm_yyyy)
	{
		return far::format(L"{:2}-{:3.3}-{:4}"sv,
			Time.tm_mday,
			upper(Names.Months[Time.tm_mon].Short),
			Time.tm_year + 1900);
	}

	const auto Format = [&](const auto FormatString)
	{
		return far::format(FormatString, DateSeparator, Time.tm_mday, Time.tm_mon + 1, Time.tm_year + 1900);
	};

	switch(locale.date_format())
	{
	default:
	case date_type::ymd: return Format(FSTR(L"{3:4}{0}{2:02}{0}{1:02}"sv));
	case date_type::dmy: return Format(FSTR(L"{1:02}{0}{2:02}{0}{3:4}"sv));
	case date_type::mdy: return Format(FSTR(L"{2:02}{0}{1:02}{0}{3:4}"sv));
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

static string StrFTime(string_view const Format, const tm& Time)
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
			Result += locale.Names(IsLocal).Weekdays[Time.tm_wday].Short;
			break;

		// Полное имя дня недели
		// full weekday name
		case L'A':
			Result += locale.Names(IsLocal).Weekdays[Time.tm_wday].Full;
			break;

		// Краткое имя месяца (Jan,Feb,...)
		// abbreviated month name
		case L'h':
		case L'b':
			Result += locale.Names(IsLocal).Months[Time.tm_mon].Short;
			break;

		// Полное имя месяца
		// full month name
		case L'B':
			Result += locale.Names(IsLocal).Months[Time.tm_mon].Full;
			break;

		//Дата и время в формате WDay Mnt  Day HH:MM:SS yyyy
		//appropriate date and time representation
		case L'c':
			// Thu Oct 07 12:37:32 1999
			far::format_to(Result, L"{} {} {:02} {:02}:{:02}:{:02} {:4}"sv,
				locale.Names(IsLocal).Weekdays[Time.tm_wday].Short,
				locale.Names(IsLocal).Months[Time.tm_mon].Short,
				Time.tm_mday, Time.tm_hour, Time.tm_min, Time.tm_sec, Time.tm_year + 1900);
			break;

		// Столетие как десятичное число (00 - 99). Например, 1992 => 19
		case L'C':
			far::format_to(Result, L"{:02}"sv, (Time.tm_year + 1900) / 100);
			break;

		// day of month, blank padded
		case L'e':
		// Две цифры дня месяца (01 - 31)
		// day of the month, 01 - 31
		case L'd':
			Result += *Iterator == L'e'?
				far::format(L"{:2}"sv, Time.tm_mday) :
				far::format(L"{:02}"sv, Time.tm_mday);
			break;

		// hour, 24-hour clock, blank pad
		case L'k':
		// Две цифры часа (00 - 23)
		// hour, 24-hour clock, 00 - 23
		case L'H':
			Result += *Iterator == L'k'?
				far::format(L"{:2}"sv, Time.tm_hour) :
				far::format(L"{:02}"sv, Time.tm_hour);
			break;

		// hour, 12-hour clock, 1 - 12, blank pad
		case L'l':
		// Две цифры часа (01 - 12)
		// hour, 12-hour clock, 01 - 12
		case L'I':
		{
			int I = Time.tm_hour % 12;

			if (!I)
				I=12;

			Result += *Iterator == L'l'?
				far::format(L"{:2}"sv, I) :
				far::format(L"{:02}"sv, I);
			break;
		}

		// Три цифры дня в году (001 - 366)
		// day of the year, 001 - 366
		case L'j':
			far::format_to(Result, L"{:03}"sv, Time.tm_yday+1);
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
				far::format_to(Result, L"{:X}"sv, Time.tm_mon + 1);
				break;

			// %m0 - ведущий 0
			case L'0':
				far::format_to(Result, L"{:02}"sv, Time.tm_mon + 1);
				break;

			default:
				--Iterator;
				far::format_to(Result, L"{}"sv, Time.tm_mon + 1);
				break;
			}
			break;

		// Две цифры минут (00 - 59)
		// minute, 00 - 59
		case L'M':
			far::format_to(Result, L"{:02}"sv, Time.tm_min);
			break;

		// AM или PM
		// am or pm based on 12-hour clock
		case L'p':
			Result += Time.tm_hour / 12? L"PM"sv : L"AM"sv;
			break;

		// Две цифры секунд (00 - 59)
		// second, 00 - 59
		case L'S':
			far::format_to(Result, L"{:02}"sv, Time.tm_sec);
			break;

		// День недели где 0 - Воскресенье (Sunday) (0 - 6)
		// weekday, Sunday == 0, 0 - 6
		case L'w':
			Result.push_back(L'0' + Time.tm_wday);
			break;

		// Две цифры номера недели, где Воскресенье (Sunday) является первым днем недели (00 - 53)
		// week of year, Sunday is first day of week
		case L'U':
		// Две цифры номера недели, где Понедельник (Monday) является первым днем недели (00 - 53)
		// week of year, Monday is first day of week
		case L'W':
		{
			int I = Time.tm_wday - (Time.tm_yday % 7);

			//I = (chr == 'W'?(!WeekFirst?((t->tm_wday+6)%7):(t->tm_wday? t->tm_wday-1:6)):(t->tm_wday)) - (t->tm_yday % 7);
			if (I<0)
				I+=7;

			far::format_to(Result, L"{:02}"sv, (Time.tm_yday + I - (*Iterator == L'W')) / 7);
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
			far::format_to(Result, L"{1:02}{0}{2:02}{0}{3:02}"sv, locale.time_separator(), Time.tm_hour, Time.tm_min, Time.tm_sec);
			break;

		// Две цифры года без столетия (00 to 99)
		// year without a century, 00 - 99
		case L'y':
			far::format_to(Result, L"{:02}"sv, Time.tm_year % 100);
			break;

		// Год со столетием (19yy-20yy)
		// year with century
		case L'Y':
			Result += str(1900 + Time.tm_year);
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

				far::format_to(Result, L"{:+05}"sv, HHMM);
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
				std::wcsftime(Buffer, std::size(Buffer), L"%V", &Time);
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
	return StrFTime(Format.empty()? Global->Opt->Macro.strDateFormat : Format, *std::localtime(&Time));
}

static void ParseTimeComponents(string_view const Src, std::span<const std::pair<size_t, size_t>> const Ranges, std::span<time_component> const Dst, time_component const Default)
{
	assert(Dst.size() == Ranges.size());
	std::ranges::transform(Ranges, Dst.begin(), [Src, Default](const auto& i)
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

os::chrono::time parse_time(string_view Date, string_view Time, int DateFormat)
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
		static_cast<uint16_t>(full_year(DateN[Indices[2]])),
		static_cast<uint8_t>(DateN[Indices[1]]),
		static_cast<uint8_t>(DateN[Indices[0]]),
		static_cast<uint8_t>(TimeN[i_hour]),
		static_cast<uint8_t>(TimeN[i_minute]),
		static_cast<uint8_t>(TimeN[i_second]),
		TimeN[i_tick]
	};
}

os::chrono::time_point ParseTimePoint(string_view const Date, string_view const Time, int const DateFormat)
{
	const auto ParsedTime = parse_time(Date, Time, DateFormat);

	if (is_time_none(ParsedTime.Year) || is_time_none(ParsedTime.Month) || is_time_none(ParsedTime.Day))
	{
		// Year / Month / Day can't have reasonable defaults
		return {};
	}

	const auto Default = [](unsigned const Value)
	{
		// Everything else can
		return is_time_none(Value)? 0 : Value;
	};

	os::chrono::local_time LocalTime{ ParsedTime };

	LocalTime.Hours            = Default(ParsedTime.Hours);
	LocalTime.Minutes          = Default(ParsedTime.Minutes);
	LocalTime.Seconds          = Default(ParsedTime.Seconds);
	LocalTime.Hectonanoseconds = Default(ParsedTime.Hectonanoseconds);

	os::chrono::time_point TimePoint;
	if (!local_to_utc(LocalTime, TimePoint))
		return {};

	return TimePoint;
}

os::chrono::duration ParseDuration(string_view const Date, string_view const Time)
{
	time_component DateN[1];
	const std::pair<size_t, size_t> DateRange[]{ { 0, Date.size() } };
	ParseTimeComponents(Date, DateRange, DateN, 0);

	time_component TimeN[4];
	ParseTimeComponents(Time, TimeRanges, TimeN, 0);

	using namespace std::chrono;

	return days(DateN[0]) + hours(TimeN[0]) + minutes(TimeN[1]) + seconds(TimeN[2]) + os::chrono::hectonanoseconds(TimeN[3]);
}

std::tuple<string, string> time_point_to_string(os::chrono::time_point const Point, int const TimeLength, int const FullYear, bool const Brief, bool const TextMonth)
{
	if (Point == os::chrono::time_point{})
	{
		return {};
	}

	const auto DateFormat = locale.date_format();
	const auto DateSeparator = locale.date_separator();
	const auto TimeSeparator = locale.time_separator();
	const auto DecimalSeparator = locale.decimal_separator();

	const auto CurDateFormat = Brief && DateFormat == date_type::ymd? date_type::mdy : DateFormat;

	os::chrono::local_time LocalTime;
	if (!utc_to_local(Point, LocalTime))
		return {};

	auto Letter = L""sv;

	if (TimeLength == 6)
	{
		Letter = LocalTime.Hours < 12 ? L"a"sv : L"p"sv;

		if (LocalTime.Hours > 12)
			LocalTime.Hours -= 12;

		if (!LocalTime.Hours)
			LocalTime.Hours = 12;
	}

	auto TimeText = TimeLength < 7?
		far::format(L"{:02}{}{:02}{}"sv, LocalTime.Hours, TimeSeparator, LocalTime.Minutes, Letter) :
		cut_right(
			far::format(
				L"{0:02}{1}{2:02}{1}{3:02}{4}{5:07}"sv,
				LocalTime.Hours,
				TimeSeparator,
				LocalTime.Minutes,
				LocalTime.Seconds,
				DecimalSeparator,
				LocalTime.Hectonanoseconds
			),
			TimeLength
		);

	const auto Year = FullYear? LocalTime.Year : LocalTime.Year % 100;

	string DateText;

	if (TextMonth)
	{
		const auto Format = [&](const auto FormatString)
		{
			DateText = far::format(FormatString, LocalTime.Day, locale.LocalNames().Months[LocalTime.Month - 1].Short, Year);
		};

		switch (CurDateFormat)
		{
		default:
		case date_type::ymd: Format(FSTR(L"{2:02} {1:3.3} {0:2}"sv)); break;
		case date_type::dmy: Format(FSTR(L"{0:2} {1:3.3} {2:02}"sv)); break;
		case date_type::mdy: Format(FSTR(L"{1:3.3} {0:2} {2:02}"sv)); break;
		}
	}
	else
	{
		int p1, p2, p3;
		int w1 = 2, w3 = 2;
		wchar_t f1 = L'0', f3 = FullYear == 2? L' ' : L'0';

		switch (CurDateFormat)
		{
		default:
		case date_type::ymd:
			p1 = Year;
			w1 = FullYear == 2? 5 : 2;
			std::ranges::swap(f1, f3);
			p2 = LocalTime.Month;
			p3 = LocalTime.Day;
			break;

		case date_type::dmy:
			p1 = LocalTime.Day;
			p2 = LocalTime.Month;
			p3 = Year;
			break;

		case date_type::mdy:
			p1 = LocalTime.Month;
			p2 = LocalTime.Day;
			p3 = Year;
			break;
		}

		// Library doesn't support dynamic fill currently. TODO: fix it?
		wchar_t Format[] = L"{0: >{1}}{6}{2:0>{3}}{6}{4: >{5}}";
		Format[3] = f1;
		Format[27] = f3;
		DateText = far::vformat(Format, p1, w1, p2, 2, p3, w3, DateSeparator);
	}

	if (Brief)
	{
		DateText.resize(TextMonth? 6 : 5);

		os::chrono::local_time Now;
		if (utc_to_local(os::chrono::nt_clock::now(), Now) && Now.Year != LocalTime.Year)
			TimeText = far::format(L"{:5}"sv, LocalTime.Year);
	}

	return { std::move(DateText), std::move(TimeText) };
}

template<typename T> requires (T::period::num < T::period::den && T::period::num == 1 && T::period::den % 10 == 0)
static constexpr auto decimal_duration_width()
{
	size_t Result = 0;

	for (auto i = T::period::den; i != 1; i /= 10)
		++Result;

	return Result;
}

std::tuple<string, string> duration_to_string(os::chrono::duration Duration)
{
	using namespace std::chrono;
	using namespace os::chrono;

	const auto Parts = split_duration<days, hours, minutes, seconds, hectonanoseconds>(Duration);

	return
	{
		str(Parts.get<days>() / 1_d),
		far::format(L"{0:02}{4}{1:02}{4}{2:02}{5}{3:0{6}}"sv,
			Parts.get<hours>() / 1h,
			Parts.get<minutes>() / 1min,
			Parts.get<seconds>() / 1s,
			Parts.get<hectonanoseconds>() / 1_hns,
			locale.time_separator(),
			locale.decimal_separator(),
			decimal_duration_width<hectonanoseconds>()
		)
	};
}

string duration_to_string_hms(os::chrono::duration Duration)
{
	using namespace std::chrono;

	const auto Parts = split_duration<hours, minutes, seconds>(Duration);

	return far::format(L"{0:02}{3}{1:02}{3}{2:02}"sv,
		Parts.get<hours>() / 1h,
		Parts.get<minutes>() / 1min,
		Parts.get<seconds>() / 1s,
		locale.time_separator()
	);
}

string duration_to_string_hr(os::chrono::duration Duration)
{
	using namespace std::chrono;
	using namespace os::chrono;

	const auto Parts = split_duration<days, hours, minutes, seconds, hectonanoseconds>(Duration);

	const long long Values[]
	{
		Parts.get<days>() / 1_d,
		Parts.get<hours>() / 1h,
		Parts.get<minutes>() / 1min,
	};

	string Result;

	for (const auto& [v, s]: zip(Values, L"dhm"sv))
	{
		if (v)
			far::format_to(Result, L"{}{} "sv, v, s);
	}

	const auto Seconds = Parts.get<seconds>() / 1s;
	const auto Decimals = Parts.get<hectonanoseconds>() / 1_hns;

	if (Seconds || Decimals || Result.empty())
	{
		far::format_to(Result, L"{}"sv, Seconds);

		if (Decimals)
			far::format_to(Result, L".{:0{}}"sv, Decimals, decimal_duration_width<hectonanoseconds>());

		Result += L's';
	}
	else
	{
		Result.pop_back();
	}

	return Result;
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
	if (Current - m_Begin > m_Interval)
	{
		reset(Current);
		return true;
	}
	return false;
}

std::pair<string, string> format_datetime(os::chrono::time const Time)
{
	return
	{
		far::format(
			L"{:04}-{:02}-{:02}"sv,
			Time.Year,
			Time.Month,
			Time.Day
		),
		far::format(
			L"{:02}:{:02}:{:02}.{:03}"sv,
			Time.Hours,
			Time.Minutes,
			Time.Seconds,
			Time.Hectonanoseconds / (1ms / 1_hns)
		)
	};
}

static std::chrono::milliseconds till_next_unit(std::chrono::seconds const Unit)
{
	const auto Now = os::chrono::nt_clock::now().time_since_epoch();
	return ((Now / Unit + 1) * Unit - Now) / 1ms * 1ms;
}

std::chrono::milliseconds till_next_second()
{
	return till_next_unit(1s);
}

std::chrono::milliseconds till_next_minute()
{
	return till_next_unit(1min);
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
		const auto& [Date, Time] = duration_to_string(i.Duration);
		REQUIRE(ParseDuration(Date, Time) == i.Duration);
	}
}

TEST_CASE("datetime.parse.timepoint")
{
	constexpr auto tn = time_none;

	static const struct
	{
		string_view Date, Time;
		int DateFormat;
		os::chrono::time TimePoint;
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
		REQUIRE(i.TimePoint == parse_time(i.Date, i.Time, i.DateFormat));
	}
}

TEST_CASE("datetime.decimal_duration_width")
{
	using namespace std::chrono;
	using namespace os::chrono;

	using bad_duration_1 = std::chrono::duration<int, std::ratio<2, 3>>;
	using bad_duration_2 = std::chrono::duration<int, std::ratio<1, 3>>;

	STATIC_REQUIRE_ERROR(bad_duration_1, decimal_duration_width<TestType>());
	STATIC_REQUIRE_ERROR(bad_duration_2, decimal_duration_width<TestType>());
	STATIC_REQUIRE_ERROR(minutes, decimal_duration_width<TestType>());
	STATIC_REQUIRE_ERROR(seconds, decimal_duration_width<TestType>());

	STATIC_REQUIRE(decimal_duration_width<milliseconds>() == 3);
	STATIC_REQUIRE(decimal_duration_width<microseconds>() == 6);
	STATIC_REQUIRE(decimal_duration_width<hectonanoseconds>() == 9 - 2);
	STATIC_REQUIRE(decimal_duration_width<nanoseconds>() == 9);
}

TEST_CASE("datetime.duration_to_string")
{
	static const struct
	{
		os::chrono::duration Duration;
		string_view Days, Timestamp, HMS, HR;
	}
	Tests[]
	{
		{ 0s,                                         L"0"sv, L"00:00:00.0000000"sv, L"00:00:00"sv,  L"0s"sv },
		{ 5min,                                       L"0"sv, L"00:05:00.0000000"sv, L"00:05:00"sv,  L"5m"sv },
		{ 1_d + 1_hns,                                L"1"sv, L"00:00:00.0000001"sv, L"24:00:00"sv,  L"1d 0.0000001s"sv },
		{ 7_d,                                        L"7"sv, L"00:00:00.0000000"sv, L"168:00:00"sv, L"7d"sv },
		{ 2_d +  7h + 13min +  47s +   7654321_hns,   L"2"sv, L"07:13:47.7654321"sv, L"55:13:47"sv,  L"2d 7h 13m 47.7654321s"sv },
		{ 3_d + 25h + 81min + 120s + 123456789_hns,   L"4"sv, L"02:23:12.3456789"sv, L"98:23:12"sv,  L"4d 2h 23m 12.3456789s"sv },
	};

	for (const auto& i: Tests)
	{
		const auto [Days, Timestamp] = duration_to_string(i.Duration);
		REQUIRE(i.Days == Days);
		REQUIRE(i.Timestamp == Timestamp);
		REQUIRE(i.HMS == duration_to_string_hms(i.Duration));
		REQUIRE(i.HR == duration_to_string_hr(i.Duration));
	}
}

TEST_CASE("datetime.format_datetime")
{
	static const struct
	{
		os::chrono::time SystemTime;
		string_view Date, Time;
	}
	Tests[]
	{
		{ {                                    },  L"0000-00-00"sv, L"00:00:00.000"sv },
		{ {     1,  1,  1,  1,  1,  1,   10000 },  L"0001-01-01"sv, L"01:01:01.001"sv },
		{ {  2023,  9, 18, 22, 37, 14, 1230000 },  L"2023-09-18"sv, L"22:37:14.123"sv },
		{ { 30827, 12, 31, 23, 59, 59, 9990000 }, L"30827-12-31"sv, L"23:59:59.999"sv },
	};

	for (const auto& i: Tests)
	{
		const auto [Date, Time] = format_datetime(i.SystemTime);
		REQUIRE(i.Date == Date);
		REQUIRE(i.Time == Time);
	}
}
#endif
