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

#include "headers.hpp"
#pragma hdrstop

#include "datetime.hpp"
#include "config.hpp"
#include "strmix.hpp"
#include "global.hpp"
#include "imports.hpp"
#include "locale.hpp"

class locale_cache
{
public:
	locale_cache():
		m_Valid()
	{
		init();
	}

	void Invalidate() const
	{
		m_Valid = false;
	}

	struct names
	{
		struct name
		{
			string Full;
			string Short;
		};

		name Months[12];
		name Weekdays[7];
	};

	int DateFormat() const { init(); return m_DateFormat; }
	wchar_t DateSeparator() const { init(); return m_DateSeparator; }
	wchar_t TimeSeparator() const { init(); return m_TimeSeparator; }
	wchar_t DecimalSeparator() const { init(); return m_DecimalSeparator; }
	const names& LocalNames() const { init(); return m_LocalNames; }
	const names& EnglishNames() const { init(); return m_EnglishNames; }
	const names& Names(bool Local) const { init(); return Local? m_LocalNames : m_EnglishNames; }

private:
	void init() const
	{
		if (m_Valid)
		{
			return;
		}

		const auto InitNames = [](int Language, names& Names)
		{
			// LOCALE_S[ABBREV]DAYNAME<1-7> indexes start from Monday, remap to Sunday to make them compatible with tm::tm_wday
			static const LCTYPE DayIndexes[] = { LOCALE_SDAYNAME7, LOCALE_SDAYNAME1, LOCALE_SDAYNAME2, LOCALE_SDAYNAME3, LOCALE_SDAYNAME4, LOCALE_SDAYNAME5, LOCALE_SDAYNAME6 };
			static const LCTYPE ShortDayIndexes[] = { LOCALE_SABBREVDAYNAME7, LOCALE_SABBREVDAYNAME1, LOCALE_SABBREVDAYNAME2, LOCALE_SABBREVDAYNAME3, LOCALE_SABBREVDAYNAME4, LOCALE_SABBREVDAYNAME5, LOCALE_SABBREVDAYNAME6 };

			const LCID CurLCID = MAKELCID(MAKELANGID(Language, SUBLANG_DEFAULT), SORT_DEFAULT);

			for_each_cnt(RANGE(Names.Months, i, size_t index)
			{
				i.Full = locale::GetValue(CurLCID, LCTYPE(LOCALE_SMONTHNAME1 + index));
				i.Short = locale::GetValue(CurLCID, LCTYPE(LOCALE_SABBREVMONTHNAME1 + index));
			});

			for_each_cnt(RANGE(Names.Weekdays, i, size_t index)
			{
				i.Full = locale::GetValue(CurLCID, DayIndexes[index]);
				i.Short = locale::GetValue(CurLCID, ShortDayIndexes[index]);
			});
		};

		InitNames(LANG_NEUTRAL, m_LocalNames);
		InitNames(LANG_ENGLISH, m_EnglishNames);

		m_DateFormat = locale::GetDateFormat();
		m_DateSeparator = locale::GetDateSeparator();
		m_TimeSeparator = locale::GetTimeSeparator();
		m_DecimalSeparator = locale::GetDecimalSeparator();

		m_Valid = true;
	}

	mutable names m_LocalNames;
	mutable names m_EnglishNames;
	mutable int m_DateFormat;
	mutable wchar_t m_DateSeparator;
	mutable wchar_t m_TimeSeparator;
	mutable wchar_t m_DecimalSeparator;

	mutable bool m_Valid;
};

locale_cache& LocaleCache()
{
	static locale_cache sCache;
	return sCache;
}

void OnIntlSettingsChange()
{
	LocaleCache().Invalidate();
}

DWORD ConvertYearToFull(DWORD ShortYear)
{
	DWORD UpperBoundary = 0;
	if(!GetCalendarInfo(LOCALE_USER_DEFAULT, CAL_GREGORIAN, CAL_ITWODIGITYEARMAX|CAL_RETURN_NUMBER, nullptr, 0, &UpperBoundary))
	{
		UpperBoundary = 2029; // Magic, current default value.
	}
	return (UpperBoundary/100-(ShortYear<UpperBoundary%100?0:1))*100+ShortYear;
}

static void st_time(string &strDest, const tm *tmPtr, const locale_cache::names& Names, const wchar_t chr)
{
	const auto DateSeparator = LocaleCache().DateSeparator();

	if (chr==L'v')
	{
		strDest = str_printf(L"%2d-%3.3s-%4d",
			tmPtr->tm_mday,
			Names.Months[tmPtr->tm_mon].Short.data(),
			tmPtr->tm_year+1900);

		InplaceUpper(strDest, 3, 3);
	}
	else
		switch (LocaleCache().DateFormat())
		{
			case 0:
				strDest = str_printf(L"%02d%c%02d%c%4d",
				                   tmPtr->tm_mon+1,
				                   DateSeparator,
				                   tmPtr->tm_mday,
				                   DateSeparator,
				                   tmPtr->tm_year+1900);
				break;
			case 1:
				strDest = str_printf(L"%02d%c%02d%c%4d",
				                   tmPtr->tm_mday,
				                   DateSeparator,
				                   tmPtr->tm_mon+1,
				                   DateSeparator,
				                   tmPtr->tm_year+1900);
				break;
			default:
				strDest = str_printf(L"%4d%c%02d%c%02d",
				                   tmPtr->tm_year+1900,
				                   DateSeparator,
				                   tmPtr->tm_mon+1,
				                   DateSeparator,
				                   tmPtr->tm_mday);
				break;
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
					strBuf = LocaleCache().Names(IsLocal).Weekdays[t->tm_wday].Short;
					break;
					// Полное имя дня недели
					// full weekday name
				case L'A':
					strBuf = LocaleCache().Names(IsLocal).Weekdays[t->tm_wday].Full;
					break;
					// Краткое имя месяца (Jan,Feb,...)
					// abbreviated month name
				case L'h':
				case L'b':
					strBuf = LocaleCache().Names(IsLocal).Months[t->tm_mon].Short;
					break;
					// Полное имя месяца
					// full month name
				case L'B':
					strBuf = LocaleCache().Names(IsLocal).Months[t->tm_mon].Full;
					break;
					//Дата и время в формате WDay Mnt  Day HH:MM:SS yyyy
					//appropriate date and time representation
				case L'c':
					// Thu Oct 07 12:37:32 1999
					strBuf = str_printf(L"%s %s %02d %02d:%02d:%02d %4d",
						LocaleCache().Names(IsLocal).Weekdays[t->tm_wday].Short.data(),
						LocaleCache().Names(IsLocal).Months[t->tm_mon].Short.data(),
						t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec, t->tm_year + 1900);
					break;
					// Столетие как десятичное число (00 - 99). Например, 1992 => 19
				case L'C':
					strBuf = str_printf(L"%02d",(t->tm_year+1900)/100);
					break;
					// day of month, blank padded
				case L'e':
					// Две цифры дня месяца (01 - 31)
					// day of the month, 01 - 31
				case L'd':
					strBuf = str_printf(*Format==L'e'?L"%2d":L"%02d",t->tm_mday);
					break;
					// hour, 24-hour clock, blank pad
				case L'k':
					// Две цифры часа (00 - 23)
					// hour, 24-hour clock, 00 - 23
				case L'H':
					strBuf = str_printf(*Format==L'k'?L"%2d":L"%02d",t->tm_hour);
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

					strBuf = str_printf(*Format==L'l'?L"%2d":L"%02d",I);
					break;
				}
				// Три цифры дня в году (001 - 366)
				// day of the year, 001 - 366
				case L'j':
					strBuf = str_printf(L"%03d",t->tm_yday+1);
					break;
					// Две цифры месяца, как десятичное число (1 - 12)
					// month, 01 - 12
				case L'm':
				{
					// %mh - Hex month digit
					// %m0 - ведущий 0
					const wchar_t *fmt=Format[1]==L'h'?L"%X":Format[1]==L'0'?L"%02d":L"%d";

					if (fmt[1]!=L'd')
						Format++;

					strBuf = str_printf(fmt,t->tm_mon+1);
					break;
				}
				// Две цифры минут (00 - 59)
				// minute, 00 - 59
				case L'M':
					strBuf = str_printf(L"%02d",t->tm_min);
					break;
					// AM или PM
					// am or pm based on 12-hour clock
				case L'p':
					strBuf=(t->tm_hour/12)?L"PM":L"AM";
					break;
					// Две цифры секунд (00 - 59)
					// second, 00 - 59
				case L'S':
					strBuf = str_printf(L"%02d",t->tm_sec);
					break;
					// День недели где 0 - Воскресенье (Sunday) (0 - 6)
					// weekday, Sunday == 0, 0 - 6
				case L'w':
					strBuf = std::to_wstring(t->tm_wday);
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

					strBuf = str_printf(L"%02d",(t->tm_yday+I-(*Format==L'W'))/7);
					break;
				}
				// date as dd-bbb-YYYY
				case L'v':
					// Дата в формате mm.dd.yyyy
					// appropriate date representation
				case L'D':
				case L'x':
					st_time(strBuf, t, LocaleCache().Names(IsLocal), *Format);
					break;
					// Время в формате HH:MM:SS
					// appropriate time representation
				case L'T':
				case L'X':
				{
					const auto TimeSeparator = LocaleCache().TimeSeparator();
					strBuf = str_printf(L"%02d%c%02d%c%02d",t->tm_hour,TimeSeparator,t->tm_min,TimeSeparator,t->tm_sec);
					break;
				}
				// Две цифры года без столетия (00 to 99)
				// year without a century, 00 - 99
				case L'y':
					strBuf = str_printf(L"%02d",t->tm_year%100);
					break;
					// Год со столетием (19yy-20yy)
					// year with century
				case L'Y':
					strBuf = std::to_wstring(1900+t->tm_year);
					break;
					// Имя часового пояса или пусто, если часовой пояс не задан
				case L'Z':
					strBuf = str_printf(L"%+03d%02d",-(_timezone/3600),-(_timezone/60)%60);
					//Ptr = _tzname[ t->tm_isdst ];
					break;
					// same as \n
				case L'n':
					strBuf=L"\n";
					break;
					// same as \t
				case L't':
					strBuf=L"\t";
					break;
				case L'%':
					strBuf=L"%";
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
					strBuf = str_printf(L"%02d",iso8601wknum(t));
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
	time_t secs_now;
	_tzset();
	time(&secs_now);
	const auto time_now = localtime(&secs_now);

	if (!Format || !*Format)
		Format = Global->Opt->Macro.strDateFormat.data();

	return StrFTime(Format, time_now);
}

void GetFileDateAndTime(const string& Src, LPWORD Dst, size_t Count, wchar_t Separator)
{
	const wchar_t Separators[] = { Separator, 0 };
	const auto Components = split<std::vector<string>>(Src, STLF_ALLOWEMPTY, Separators);
	assert(Components.size() == Count);
	std::transform(ALL_CONST_RANGE(Components), Dst, [](const auto& i) { return i.empty()? -1 : std::stoul(i); });
}

void StrToDateTime(const string& CDate, const string& CTime, FILETIME &ft, int DateFormat, wchar_t DateSeparator, wchar_t TimeSeparator, bool bRelative)
{
	WORD DateN[3]={},TimeN[4]={};
	SYSTEMTIME st={};
	// Преобразуем введённые пользователем дату и время
	GetFileDateAndTime(CDate, DateN, std::size(DateN), DateSeparator);
	GetFileDateAndTime(CTime, TimeN, std::size(TimeN), TimeSeparator);

	if (!bRelative)
	{
		if (DateN[0]==(WORD)-1||DateN[1]==(WORD)-1||DateN[2]==(WORD)-1)
		{
			// Пользователь оставил дату пустой, значит обнулим дату и время.
			ft = {};
			return;
		}

		// "Оформим"
		switch (DateFormat)
		{
			case 0:
				st.wMonth=DateN[0]!=(WORD)-1?DateN[0]:0;
				st.wDay  =DateN[1]!=(WORD)-1?DateN[1]:0;
				st.wYear =DateN[2]!=(WORD)-1?DateN[2]:0;
				break;
			case 1:
				st.wDay  =DateN[0]!=(WORD)-1?DateN[0]:0;
				st.wMonth=DateN[1]!=(WORD)-1?DateN[1]:0;
				st.wYear =DateN[2]!=(WORD)-1?DateN[2]:0;
				break;
			default:
				st.wYear =DateN[0]!=(WORD)-1?DateN[0]:0;
				st.wMonth=DateN[1]!=(WORD)-1?DateN[1]:0;
				st.wDay  =DateN[2]!=(WORD)-1?DateN[2]:0;
				break;
		}

		if (st.wYear<100)
		{
			st.wYear = static_cast<WORD>(ConvertYearToFull(st.wYear));
		}
	}
	else
	{
		st.wDay = DateN[0]!=(WORD)-1?DateN[0]:0;
	}

	st.wHour   = TimeN[0]!=(WORD)-1?(TimeN[0]):0;
	st.wMinute = TimeN[1]!=(WORD)-1?(TimeN[1]):0;
	st.wSecond = TimeN[2]!=(WORD)-1?(TimeN[2]):0;
	st.wMilliseconds = TimeN[3]!=(WORD)-1?(TimeN[3]):0;

	// преобразование в "удобоваримый" формат
	if (bRelative)
	{
		unsigned long long time;
		time = st.wMilliseconds;
		time += (UINT64)st.wSecond * 1000;
		time += (UINT64)st.wMinute * 1000 * 60;
		time += (UINT64)st.wHour   * 1000 * 60 * 60;
		time += (UINT64)st.wDay    * 1000 * 60 * 60 * 24;
		time *= 10000;
		ft = UI64ToFileTime(time);
	}
	else
	{
		Local2Utc(st, ft);
	}
}

void ConvertDate(const FILETIME &ft,string &strDateText, string &strTimeText,int TimeLength,
                 int Brief,int TextMonth,int FullYear)
{
	const auto DateFormat = LocaleCache().DateFormat();
	const auto DateSeparator = LocaleCache().DateSeparator();
	const auto TimeSeparator = LocaleCache().TimeSeparator();
	const auto DecimalSeparator = LocaleCache().DecimalSeparator();

	int CurDateFormat=DateFormat;

	if (Brief && CurDateFormat==2)
		CurDateFormat=0;

	if (!ft.dwHighDateTime)
	{
		strDateText.clear();
		strTimeText.clear();
		return;
	}

	SYSTEMTIME st;
	Utc2Local(ft, st);
	//if ( !strTimeText.empty() )
	{
		const wchar_t *Letter=L"";

		if (TimeLength==6)
		{
			Letter=(st.wHour<12) ? L"a":L"p";

			if (st.wHour>12)
				st.wHour-=12;

			if (!st.wHour)
				st.wHour=12;
		}

		if (TimeLength<7)
			strTimeText = str_printf(L"%02d%c%02d%s",st.wHour,TimeSeparator,st.wMinute,Letter);
		else
		{
			const auto strFullTime = str_printf(L"%02d%c%02d%c%02d%c%03d",
				st.wHour, TimeSeparator, st.wMinute, TimeSeparator, st.wSecond, DecimalSeparator, st.wMilliseconds);
			strTimeText = str_printf(L"%.*s",TimeLength, strFullTime.data());
		}
	}
	//if ( !strDateText.empty() )
	{
		int Year=st.wYear;

		if (!FullYear)
			Year%=100;

		if (TextMonth)
		{
			const auto Mnth = LocaleCache().LocalNames().Months[st.wMonth - 1].Short.data();

			switch (CurDateFormat)
			{
				case 0:
					strDateText = str_printf(L"%3.3s %2d %02d",Mnth,st.wDay,Year);
					break;
				case 1:
					strDateText = str_printf(L"%2d %3.3s %02d",st.wDay,Mnth,Year);
					break;
				default:
					strDateText = str_printf(L"%02d %3.3s %2d",Year,Mnth,st.wDay);
					break;
			}
		}
		else
		{
			int p1,p2,p3=Year;
			int w1=2, w2=2, w3=2;
			wchar_t f1=L'0', f2=L'0', f3=FullYear==2?L' ':L'0';
			switch (CurDateFormat)
			{
				case 0:
					p1=st.wMonth;
					p2=st.wDay;
					break;
				case 1:
					p1=st.wDay;
					p2=st.wMonth;
					break;
				default:
					p1=Year;
					w1=FullYear==2?5:2;
					using std::swap;
					swap(f1, f3);
					p2=st.wMonth;
					p3=st.wDay;
					break;
			}
			strDateText = FormatString()<<fmt::FillChar(f1)<<fmt::MinWidth(w1)<<p1<<DateSeparator<<fmt::FillChar(f2)<<fmt::MinWidth(w2)<<p2<<DateSeparator<<fmt::FillChar(f3)<<fmt::MinWidth(w3)<<p3;
		}
	}

	if (Brief)
	{
		strDateText.resize(TextMonth ? 6 : 5);

		if (get_local_time().wYear != st.wYear)
			strTimeText = str_printf(L"%5d",st.wYear);
	}
}

void ConvertRelativeDate(const FILETIME &ft,string &strDaysText,string &strTimeText)
{
	auto time = FileTimeToUI64(ft);

	UINT64 ms = (time/=10000)%1000;
	UINT64 s = (time/=1000)%60;
	UINT64 m = (time/=60)%60;
	UINT64 h = (time/=60)%24;
	UINT64 d = time/=24;

	strDaysText = std::to_wstring(d);
	strTimeText = FormatString() << fmt::MinWidth(2) << fmt::FillChar(L'0') << h << LocaleCache().TimeSeparator() << fmt::MinWidth(2) << fmt::FillChar(L'0') << m << LocaleCache().TimeSeparator() << fmt::MinWidth(2) << fmt::FillChar(L'0') << s << LocaleCache().DecimalSeparator() << fmt::MinWidth(3) << fmt::FillChar(L'0') << ms;
}

bool Utc2Local(const FILETIME &ft, SYSTEMTIME &lst)
{
	SYSTEMTIME st;
	return FileTimeToSystemTime(&ft, &st) && SystemTimeToTzSpecificLocalTime(nullptr, &st, &lst);
}

bool Utc2Local(SYSTEMTIME &st, FILETIME &lft)
{
	SYSTEMTIME lst;
	return SystemTimeToTzSpecificLocalTime(nullptr, &st, &lst) && SystemTimeToFileTime(&lst, &lft);
}

static bool local_to_utc(const SYSTEMTIME &lst, SYSTEMTIME &ust)
{
	if (Imports().TzSpecificLocalTimeToSystemTime)
	{
		return Imports().TzSpecificLocalTimeToSystemTime(nullptr, &lst, &ust) != FALSE;
	}

	tm ltm;
	ltm.tm_year = lst.wYear - 1900;
	ltm.tm_mon  = lst.wMonth - 1;
	ltm.tm_mday = lst.wDay;
	ltm.tm_hour = lst.wHour;
	ltm.tm_min  = lst.wMinute;
	ltm.tm_sec  = lst.wSecond;
	ltm.tm_wday = lst.wDayOfWeek;
	ltm.tm_yday = -1;
	ltm.tm_isdst = -1;

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

bool Local2Utc(const FILETIME &lft, SYSTEMTIME &st)
{
	SYSTEMTIME lst;
	return FileTimeToSystemTime(&lft, &lst) && local_to_utc(lst, st);
}

bool Local2Utc(const SYSTEMTIME &lst, FILETIME &ft)
{
	SYSTEMTIME st;
	return local_to_utc(lst, st) && SystemTimeToFileTime(&st, &ft);
}
