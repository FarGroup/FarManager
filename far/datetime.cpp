﻿/*
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

#include "datetime.hpp"

#include "config.hpp"
#include "strmix.hpp"
#include "global.hpp"
#include "imports.hpp"
#include "locale.hpp"
#include "encoding.hpp"

#include "common/enum_tokens.hpp"

#include "format.hpp"

DWORD ConvertYearToFull(DWORD ShortYear)
{
	DWORD UpperBoundary = 0;
	if(!GetCalendarInfo(LOCALE_USER_DEFAULT, CAL_GREGORIAN, CAL_ITWODIGITYEARMAX|CAL_RETURN_NUMBER, nullptr, 0, &UpperBoundary))
	{
		UpperBoundary = 2029; // Magic, current default value.
	}
	return (UpperBoundary/100-(ShortYear<UpperBoundary%100?0:1))*100+ShortYear;
}

static void st_time(string &strDest, const tm *tmPtr, const locale_names& Names, const wchar_t chr)
{
	const auto DateSeparator = locale.date_separator();

	if (chr==L'v')
	{
		strDest = format(L"{0:2}-{1:3:3}-{2:4}",
			tmPtr->tm_mday,
			Names.Months[tmPtr->tm_mon].Short,
			tmPtr->tm_year+1900);

		inplace::upper(strDest, 3, 3);
	}
	else
	{
		const auto& GetFormat = []
		{
			switch (locale.date_format())
			{
			case 0: return L"{2:02}{0}{1:02}{0}{3:4}";
			case 1: return L"{1:02}{0}{2:02}{0}{3:4}";
			default: return L"{3:4}{0}{2:02}{0}{1:02}";
			}
		};

		strDest = format(GetFormat(), DateSeparator, tmPtr->tm_mday, tmPtr->tm_mon + 1, tmPtr->tm_year + 1900);
	}
}

// weeknumber --- figure how many weeks into the year
static int weeknumber(const tm *timeptr,const int firstweekday)
{
	int wday=timeptr->tm_wday;

	if (firstweekday==1)
	{
		if (!wday) // sunday
			wday=6;
		else
			wday--;
	}

	int ret=((timeptr->tm_yday+7-wday)/7);

	if (ret<0)
		ret=0;

	return ret;
}

// isleap --- is a year a leap year?
static int isleap(const int year)
{
	return (!(year%4) && (year%100)) || !(year%400);
}

static int iso8601wknum(const tm *timeptr)
{
	/*
	 * From 1003.2:
	 *  If the week (Monday to Sunday) containing January 1
	 *  has four or more days in the new year, then it is week 1;
	 *  otherwise it is the highest numbered week of the previous
	 *  year (52 or 53), and the next week is week 1.
	 *
	 * ADR: This means if Jan 1 was Monday through Thursday,
	 *  it was week 1, otherwise week 52 or 53.
	 *
	 * XPG4 erroneously included POSIX.2 rationale text in the
	 * main body of the standard. Thus it requires week 53.
	 */
	// get week number, Monday as first day of the week
	int weeknum=weeknumber(timeptr,1);
	/*
	 * With thanks and tip of the hatlo to tml@tik.vtt.fi
	 *
	 * What day of the week does January 1 fall on?
	 * We know that
	 *  (timeptr->tm_yday - jan1.tm_yday) MOD 7 ==
	 *      (timeptr->tm_wday - jan1.tm_wday) MOD 7
	 * and that
	 *  jan1.tm_yday == 0
	 * and that
	 *  timeptr->tm_wday MOD 7 == timeptr->tm_wday
	 * from which it follows that. . .
	 */
	int jan1day=timeptr->tm_wday-(timeptr->tm_yday%7);

	if (jan1day<0)
		jan1day+=7;

	/*
	 * If Jan 1 was a Monday through Thursday, it was in
	 * week 1.  Otherwise it was last year's highest week, which is
	 * this year's week 0.
	 *
	 * What does that mean?
	 * If Jan 1 was Monday, the week number is exactly right, it can
	 *  never be 0.
	 * If it was Tuesday through Thursday, the weeknumber is one
	 *  less than it should be, so we add one.
	 * Otherwise, Friday, Saturday or Sunday, the week number is
	 * OK, but if it is 0, it needs to be 52 or 53.
	 */
	switch (jan1day)
	{
		case 1: // Monday
			break;
		case 2: // Tuesday
		case 3: // Wednesday
		case 4: // Thursday
			weeknum++;
			break;
		case 5: // Friday
		case 6: // Saturday
		case 0: // Sunday

			if (!weeknum)
			{
#ifdef USE_BROKEN_XPG4
				/* XPG4 (as of March 1994) says 53 unconditionally */
				weeknum = 53;
#else
				// get week number of last week of last year
				// 12/31 last year
				tm dec31ly=*timeptr;
				dec31ly.tm_year--;
				dec31ly.tm_mon=11;
				dec31ly.tm_mday=31;
				dec31ly.tm_wday=!jan1day?6:jan1day-1;
				dec31ly.tm_yday=364+isleap(dec31ly.tm_year+1900);
				weeknum=iso8601wknum(&dec31ly);
#endif
			}

			break;
	}

	if (timeptr->tm_mon==11)
	{
		/*
		 * The last week of the year
		 * can be in week 1 of next year.
		 * Sigh.
		 *
		 * This can only happen if
		 *  M   T  W
		 *  29  30 31
		 *  30  31
		 *  31
		 */
		if ((timeptr->tm_wday==1&&(timeptr->tm_mday>=29&&timeptr->tm_mday<=31))||
		        (timeptr->tm_wday==2&&(timeptr->tm_mday==30||timeptr->tm_mday==31))||
		        (timeptr->tm_wday==3&&timeptr->tm_mday==31))
			weeknum=1;
	}

	return weeknum;
}

string StrFTime(const wchar_t* Format, const tm* t)
{
	bool IsLocal = false;

	string strDest;

	for (; *Format; ++Format)
	{
		if (*Format!=L'%')
		{
			const wchar_t Text[]={*Format,0};
			strDest+=Text;
		}
#if 1
		else
		{
			string strBuf;

			switch (*++Format)
			{
				case L'L':
					IsLocal = !IsLocal;
					continue;
					// Краткое имя дня недели (Sun,Mon,Tue,Wed,Thu,Fri,Sat)
					// abbreviated weekday name
				case L'a':
					strBuf = locale.Names(IsLocal).Weekdays[t->tm_wday].Short;
					break;
					// Полное имя дня недели
					// full weekday name
				case L'A':
					strBuf = locale.Names(IsLocal).Weekdays[t->tm_wday].Full;
					break;
					// Краткое имя месяца (Jan,Feb,...)
					// abbreviated month name
				case L'h':
				case L'b':
					strBuf = locale.Names(IsLocal).Months[t->tm_mon].Short;
					break;
					// Полное имя месяца
					// full month name
				case L'B':
					strBuf = locale.Names(IsLocal).Months[t->tm_mon].Full;
					break;
					//Дата и время в формате WDay Mnt  Day HH:MM:SS yyyy
					//appropriate date and time representation
				case L'c':
					// Thu Oct 07 12:37:32 1999
					strBuf = format(L"{0} {1} {2:02} {3:02}:{4:02}:{5:02} {6:4}",
						locale.Names(IsLocal).Weekdays[t->tm_wday].Short,
						locale.Names(IsLocal).Months[t->tm_mon].Short,
						t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec, t->tm_year + 1900);
					break;
					// Столетие как десятичное число (00 - 99). Например, 1992 => 19
				case L'C':
					strBuf = format(L"{0:02}", (t->tm_year + 1900) / 100);
					break;
					// day of month, blank padded
				case L'e':
					// Две цифры дня месяца (01 - 31)
					// day of the month, 01 - 31
				case L'd':
					strBuf = format(*Format == L'e'? L"{0:2}" : L"{0:02}", t->tm_mday);
					break;
					// hour, 24-hour clock, blank pad
				case L'k':
					// Две цифры часа (00 - 23)
					// hour, 24-hour clock, 00 - 23
				case L'H':
					strBuf = format(*Format == L'k'? L"{0:2}" : L"{0:02}", t->tm_hour);
					break;
					// hour, 12-hour clock, 1 - 12, blank pad
				case L'l':
					// Две цифры часа (01 - 12)
					// hour, 12-hour clock, 01 - 12
				case L'I':
				{
					int I=t->tm_hour%12;

					if (!I)
						I=12;

					strBuf = format(*Format == L'l'? L"{0:2}" : L"{0:02}", I);
					break;
				}
				// Три цифры дня в году (001 - 366)
				// day of the year, 001 - 366
				case L'j':
					strBuf = format(L"{0:03}", t->tm_yday+1);
					break;
					// Две цифры месяца, как десятичное число (1 - 12)
					// month, 01 - 12
				case L'm':
				{
					// %mh - Hex month digit
					// %m0 - ведущий 0
					const auto fmt = Format[1] == L'h'? L"{0:X}" : Format[1] == L'0'? L"{0:02}" : L"{0}";

					if (Format[1] == L'h' || Format[1] == L'0')
						Format++;

					strBuf = format(fmt, t->tm_mon + 1);
					break;
				}
				// Две цифры минут (00 - 59)
				// minute, 00 - 59
				case L'M':
					strBuf = format(L"{0:02}", t->tm_min);
					break;
					// AM или PM
					// am or pm based on 12-hour clock
				case L'p':
					strBuf = t->tm_hour / 12? L"PM"s : L"AM"s;
					break;
					// Две цифры секунд (00 - 59)
					// second, 00 - 59
				case L'S':
					strBuf = format(L"{0:02}", t->tm_sec);
					break;
					// День недели где 0 - Воскресенье (Sunday) (0 - 6)
					// weekday, Sunday == 0, 0 - 6
				case L'w':
					strBuf = str(t->tm_wday);
					break;
					// Две цифры номера недели, где Воскресенье (Sunday)
					//   является первым днем недели (00 - 53)
					// week of year, Sunday is first day of week
				case L'U':
					// Две цифры номера недели, где Понедельник (Monday)
					//    является первым днем недели (00 - 53)
					// week of year, Monday is first day of week
				case L'W':
				{
					int I=t->tm_wday-(t->tm_yday%7);

					//I = (chr == 'W'?(!WeekFirst?((t->tm_wday+6)%7):(t->tm_wday? t->tm_wday-1:6)):(t->tm_wday)) - (t->tm_yday % 7);
					if (I<0)
						I+=7;

					strBuf = format(L"{0:02}", (t->tm_yday + I - (*Format == L'W')) / 7);
					break;
				}
				// date as dd-bbb-YYYY
				case L'v':
					// Дата в формате mm.dd.yyyy
					// appropriate date representation
				case L'D':
				case L'x':
					st_time(strBuf, t, locale.Names(IsLocal), *Format);
					break;
					// Время в формате HH:MM:SS
					// appropriate time representation
				case L'T':
				case L'X':
					strBuf = format(L"{1:02}{0}{2:02}{0}{3:02}", locale.time_separator(), t->tm_hour, t->tm_min, t->tm_sec);
					break;

				// Две цифры года без столетия (00 to 99)
				// year without a century, 00 - 99
				case L'y':
					strBuf = format(L"{0:02}", t->tm_year % 100);
					break;
					// Год со столетием (19yy-20yy)
					// year with century
				case L'Y':
					strBuf = str(1900+t->tm_year);
					break;
					// ISO 8601 offset from UTC in timezone
				case L'z':
					{
						using namespace std::chrono;
						const auto Offset = split_duration<hours, minutes>(-seconds(_timezone + (t->tm_isdst? _dstbias : 0)));
						strBuf = format(L"{0:+05}", Offset.get<hours>().count() * 100 + Offset.get<minutes>().count());
					}
					break;
					// Timezone name or abbreviation
				case L'Z':
					strBuf = encoding::ansi::get_chars(_tzname[t->tm_isdst]);
					break;
					// same as \n
				case L'n':
					strBuf = L'\n';
					break;
					// same as \t
				case L't':
					strBuf = L'\t';
					break;
				case L'%':
					strBuf = L'%';
					break;
					// time as %I:%M:%S %p
				case L'r':
					strBuf = StrFTime(L"%I:%M:%S %p", t);
					break;
					// time as %H:%M
				case L'R':
					strBuf = StrFTime(L"%H:%M", t);
					break;
					// week of year according ISO 8601
				case L'V':
					strBuf = format(L"{0:02}", iso8601wknum(t));
					break;
			}

			strDest+=strBuf;
		}
#endif
	}

	if (*Format)
		strDest.clear();

	return strDest;
}

string MkStrFTime(const wchar_t *Format)
{
	const auto Time = os::chrono::nt_clock::to_time_t(os::chrono::nt_clock::now());

	if (!Format || !*Format)
		Format = Global->Opt->Macro.strDateFormat.c_str();

	_tzset();
	return StrFTime(Format, std::localtime(&Time));
}

void ParseDateComponents(string_view const Src, range<const std::pair<size_t, size_t>*> const Ranges, range<WORD*> const Dst, WORD const Default)
{
	assert(Dst.size() == Ranges.size());
	std::transform(ALL_CONST_RANGE(Ranges), Dst.begin(), [Src, Default](const auto& i)
	{
		const auto Part = trim(Src.substr(i.first, i.second));
		return Part.empty()? Default : std::stoul(string(Part));
	});
}

os::chrono::time_point ParseDate(const string& Date, const string& Time, int DateFormat, const date_ranges& DateRanges, const time_ranges& TimeRanges)
{
	WORD DateN[3];
	ParseDateComponents(Date, make_span(DateRanges), make_span(DateN));
	WORD TimeN[4];
	ParseDateComponents(Time, make_span(TimeRanges), make_span(TimeN), 0);

	if (DateN[0] == date_none || DateN[1] == date_none || DateN[2] == date_none)
	{
		// Пользователь оставил дату пустой, значит обнулим дату и время.
		return {};
	}

	SYSTEMTIME st{};

	switch (DateFormat)
	{
	case 0:
		st.wMonth = DateN[0];
		st.wDay   = DateN[1];
		st.wYear  = DateN[2];
		break;

	case 1:
		st.wDay   = DateN[0];
		st.wMonth = DateN[1];
		st.wYear  = DateN[2];
		break;

	default:
		st.wYear  = DateN[0];
		st.wMonth = DateN[1];
		st.wDay   = DateN[2];
		break;
	}

	if (st.wYear < 100)
	{
		st.wYear = static_cast<WORD>(ConvertYearToFull(st.wYear));
	}

	st.wHour         = TimeN[0];
	st.wMinute       = TimeN[1];
	st.wSecond       = TimeN[2];
	st.wMilliseconds = TimeN[3];

	os::chrono::time_point Point;
	Local2Utc(st, Point);
	return Point;
}

os::chrono::duration ParseDuration(const string& Date, const string& Time, int DateFormat, const time_ranges& TimeRanges)
{
	WORD DateN[1];
	const std::pair<size_t, size_t> DateRange[]{ { 0, Date.size() } };
	ParseDateComponents(Date, make_span(DateRange), make_span(DateN));

	WORD TimeN[4];
	ParseDateComponents(Time, make_span(TimeRanges), make_span(TimeN));

	using namespace std::chrono;
	using namespace chrono;
	return days(DateN[0]) + hours(TimeN[0]) + minutes(TimeN[1]) + seconds(TimeN[2]) + milliseconds(TimeN[3]);
}


void ConvertDate(os::chrono::time_point Point, string& strDateText, string& strTimeText, int TimeLength, int Brief, int TextMonth, int FullYear)
{
	// Epoch => empty
	if (!Point.time_since_epoch().count())
	{
		strDateText.clear();
		strTimeText.clear();
		return;
	}

	const auto DateFormat = locale.date_format();
	const auto DateSeparator = locale.date_separator();
	const auto TimeSeparator = locale.time_separator();
	const auto DecimalSeparator = locale.decimal_separator();

	const auto CurDateFormat = Brief && DateFormat == 2? 0 : DateFormat;

	SYSTEMTIME st;
	if (!Utc2Local(Point, st))
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
		strTimeText = format(L"{0:02}{1}{2:02}{3}", st.wHour, TimeSeparator, st.wMinute, Letter);
	}
	else
	{
		strTimeText = cut_right(format(L"{0:02}{1}{2:02}{1}{3:02}{4}{5:03}",
			st.wHour, TimeSeparator, st.wMinute, st.wSecond, DecimalSeparator, st.wMilliseconds), TimeLength);
	}

	const auto Year = FullYear? st.wYear : st.wYear % 100;

	if (TextMonth)
	{
		const auto GetFormat = [CurDateFormat]
		{
			switch (CurDateFormat)
			{
			case 0:  return L"{1:3.3} {0:2} {2:02}";
			case 1:  return L"{0:2} {1:3.3} {2:02}";
			default: return L"{2:02} {1:3.3} {0:2}";
			}
		};

		strDateText = format(GetFormat(), st.wDay, locale.LocalNames().Months[st.wMonth - 1].Short, Year);
	}
	else
	{
		int p1, p2, p3 = Year;
		int w1 = 2, w2 = 2, w3 = 2;
		wchar_t f1 = L'0', f2 = L'0', f3 = FullYear == 2? L' ' : L'0';
		switch (CurDateFormat)
		{
		case 0:
			p1 = st.wMonth;
			p2 = st.wDay;
			break;
		case 1:
			p1 = st.wDay;
			p2 = st.wMonth;
			break;
		default:
			p1 = Year;
			w1 = FullYear == 2?5:2;
			using std::swap;
			swap(f1, f3);
			p2 = st.wMonth;
			p3 = st.wDay;
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

		if (get_local_time().wYear != st.wYear)
			strTimeText = format(L"{0:5}", st.wYear);
	}
}

void ConvertDuration(os::chrono::duration Duration, string& strDaysText, string& strTimeText)
{
	using namespace std::chrono;
	using namespace chrono;

	const auto Result = split_duration<days, hours, minutes, seconds, milliseconds>(Duration);

	strDaysText = str(Result.get<days>().count());
	strTimeText = format(L"{0:02}{4}{1:02}{4}{2:02}{5}{3:03}",
		Result.get<hours>().count(),
		Result.get<minutes>().count(),
		Result.get<seconds>().count(),
		Result.get<milliseconds>().count(),
		locale.time_separator(),
		locale.decimal_separator());
}

bool Utc2Local(os::chrono::time_point UtcTime, SYSTEMTIME& LocalTime)
{
	const auto FileTime = os::chrono::nt_clock::to_filetime(UtcTime);
	SYSTEMTIME SystemTime;
	return FileTimeToSystemTime(&FileTime, &SystemTime) && SystemTimeToTzSpecificLocalTime(nullptr, &SystemTime, &LocalTime);
}

static bool local_to_utc(const SYSTEMTIME &lst, SYSTEMTIME &ust)
{
	if (imports.TzSpecificLocalTimeToSystemTime)
	{
		return imports.TzSpecificLocalTimeToSystemTime(nullptr, &lst, &ust) != FALSE;
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

	const auto gtim = mktime(&ltm);
	if (gtim == static_cast<time_t>(-1))
		return false;

	if (const auto ptm = gmtime(&gtim))
	{
		ust.wYear   = ptm->tm_year + 1900;
		ust.wMonth  = ptm->tm_mon + 1;
		ust.wDay    = ptm->tm_mday;
		ust.wHour   = ptm->tm_hour;
		ust.wMinute = ptm->tm_min;
		ust.wSecond = ptm->tm_sec;
		ust.wDayOfWeek = ptm->tm_wday;
		ust.wMilliseconds = lst.wMilliseconds;
		return true;
	}

	FILETIME lft, uft;
	return SystemTimeToFileTime(&lst, &lft) && LocalFileTimeToFileTime(&lft, &uft) && FileTimeToSystemTime(&uft, &ust);
}

bool Local2Utc(const SYSTEMTIME& LocalTime, os::chrono::time_point& UtcTime)
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
