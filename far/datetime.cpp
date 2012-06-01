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
#include "language.hpp"
#include "config.hpp"
#include "strmix.hpp"

#define range(low,item,hi) Max(low,Min(item,hi))

string AMonth[2][12],AWeekday[2][7],Month[2][12],Weekday[2][7];

int CurLang=-1,WeekFirst=0;

DWORD ConvertYearToFull(DWORD ShortYear)
{
	DWORD UpperBoundary = 0;
	if(!GetCalendarInfo(LOCALE_USER_DEFAULT, CAL_GREGORIAN, CAL_ITWODIGITYEARMAX|CAL_RETURN_NUMBER, nullptr, 0, &UpperBoundary))
	{
		UpperBoundary = 2029; // Magic, current default value.
	}
	return (UpperBoundary/100-(ShortYear<UpperBoundary%100?0:1))*100+ShortYear;
}

int GetDateFormat()
{
	int Result = 0;
	GetLocaleInfo(LOCALE_USER_DEFAULT,LOCALE_IDATE|LOCALE_RETURN_NUMBER,reinterpret_cast<LPWSTR>(&Result),sizeof(Result)/sizeof(WCHAR));
	return Result;
}

wchar_t GetDateSeparator()
{
	wchar_t Info[100];
	GetLocaleInfo(LOCALE_USER_DEFAULT,LOCALE_SDATE,Info,ARRAYSIZE(Info));
	return *Info;
}

wchar_t GetTimeSeparator()
{
	wchar_t Info[100];
	GetLocaleInfo(LOCALE_USER_DEFAULT,LOCALE_STIME,Info,ARRAYSIZE(Info));
	return *Info;
}

void PrepareStrFTime()
{
	DWORD Loc[]={LANG_ENGLISH,LANG_NEUTRAL},ID;

	GetLocaleInfo(LOCALE_USER_DEFAULT,LOCALE_IFIRSTDAYOFWEEK|LOCALE_RETURN_NUMBER,reinterpret_cast<LPWSTR>(&WeekFirst),sizeof(WeekFirst)/sizeof(WCHAR));

	for (int i=0; i<2; i++)
	{
		LCID CurLCID=MAKELCID(MAKELANGID(Loc[i],SUBLANG_DEFAULT),SORT_DEFAULT);

		for (ID=LOCALE_SMONTHNAME1; ID<=LOCALE_SMONTHNAME12; ID++)
		{
			int size=GetLocaleInfo(CurLCID,ID,nullptr,0);
			LPWSTR lpwszTemp=Month[i][ID-LOCALE_SMONTHNAME1].GetBuffer(size);
			GetLocaleInfo(CurLCID,ID,lpwszTemp,size);
			*lpwszTemp=Upper(*lpwszTemp);
			Month[i][ID-LOCALE_SMONTHNAME1].ReleaseBuffer();
		}

		for (ID=LOCALE_SABBREVMONTHNAME1; ID<=LOCALE_SABBREVMONTHNAME12; ID++)
		{
			int size=GetLocaleInfo(CurLCID,ID,nullptr,0);
			LPWSTR lpwszTemp=AMonth[i][ID-LOCALE_SABBREVMONTHNAME1].GetBuffer(size);
			GetLocaleInfo(CurLCID,ID,lpwszTemp,size);
			*lpwszTemp=Upper(*lpwszTemp);
			AMonth[i][ID-LOCALE_SABBREVMONTHNAME1].ReleaseBuffer();
		}

		for (ID=LOCALE_SDAYNAME1; ID<=LOCALE_SDAYNAME7; ID++)
		{
			int size=GetLocaleInfo(CurLCID,ID,nullptr,0);
			LPWSTR lpwszTemp=Weekday[i][ID-LOCALE_SDAYNAME1].GetBuffer(size);
			GetLocaleInfo(CurLCID,ID,lpwszTemp,size);
			*lpwszTemp=Upper(*lpwszTemp);
			Weekday[i][ID-LOCALE_SDAYNAME1].ReleaseBuffer();
		}

		for (ID=LOCALE_SABBREVDAYNAME1; ID<=LOCALE_SABBREVDAYNAME7; ID++)
		{
			int size=GetLocaleInfo(CurLCID,ID,nullptr,0);
			LPWSTR lpwszTemp=AWeekday[i][ID-LOCALE_SABBREVDAYNAME1].GetBuffer(size);
			GetLocaleInfo(CurLCID,ID,lpwszTemp,size);
			*lpwszTemp=Upper(*lpwszTemp);
			AWeekday[i][ID-LOCALE_SABBREVDAYNAME1].ReleaseBuffer();
		}
	}

	CurLang=0;
}

static int atime(string &strDest,const tm *tmPtr)
{
	// Thu Oct 07 12:37:32 1999
	return strDest.Format(L"%s %s %02d %02d:%02d:%02d %4d",
	                      AWeekday[CurLang][!WeekFirst?((tmPtr->tm_wday+6)%7):(!(tmPtr->tm_wday)?6:tmPtr->tm_wday-1)].CPtr(),
	                      AMonth[CurLang][tmPtr->tm_mon].CPtr(),
	                      tmPtr->tm_mday,
	                      tmPtr->tm_hour,
	                      tmPtr->tm_min,
	                      tmPtr->tm_sec,
	                      tmPtr->tm_year+1900);
}

static int st_time(string &strDest,const tm *tmPtr,const wchar_t chr)
{
	int res;
	wchar_t DateSeparator=GetDateSeparator();

	if (chr==L'v')
	{
		res=strDest.Format(L"%2d-%3.3s-%4d",range(1,tmPtr->tm_mday,31),AMonth[CurLang][range(0, tmPtr->tm_mon,11)].CPtr(),tmPtr->tm_year+1900);
		strDest.Upper(3,3);
	}
	else
		switch (GetDateFormat())
		{
			case 0:
				res=strDest.Format(L"%02d%c%02d%c%4d",
				                   tmPtr->tm_mon+1,
				                   DateSeparator,
				                   tmPtr->tm_mday,
				                   DateSeparator,
				                   tmPtr->tm_year+1900);
				break;
			case 1:
				res=strDest.Format(L"%02d%c%02d%c%4d",
				                   tmPtr->tm_mday,
				                   DateSeparator,
				                   tmPtr->tm_mon+1,
				                   DateSeparator,
				                   tmPtr->tm_year+1900);
				break;
			default:
				res=strDest.Format(L"%4d%c%02d%c%02d",
				                   tmPtr->tm_year+1900,
				                   DateSeparator,
				                   tmPtr->tm_mon+1,
				                   DateSeparator,
				                   tmPtr->tm_mday);
				break;
		}

	return res;
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
	return ((!(year%4)&&(year%100))||!(year%400));
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

size_t StrFTime(string &strDest, const wchar_t *Format,const tm *t)
{
	if (CurLang==-1 && Lang.IsLanguageLoaded())
		PrepareStrFTime();

	// меняем язык.
	CurLang=0;
	size_t Len;

	for (Len=1; *Format; Format++)
	{
		if (*Format!=L'%')
		{
			Len++;
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
					CurLang=!CurLang;
					continue;
					// Краткое имя дня недели (Sun,Mon,Tue,Wed,Thu,Fri,Sat)
					// abbreviated weekday name
				case L'a':
					strBuf=AWeekday[CurLang][!WeekFirst?((t->tm_wday+6)%7):(!t->tm_wday?6:t->tm_wday-1)];
					break;
					// Полное имя дня недели
					// full weekday name
				case L'A':
					strBuf=Weekday[CurLang][!WeekFirst?((t->tm_wday+6)%7):(!t->tm_wday?6:t->tm_wday-1)];
					break;
					// Краткое имя месяца (Jan,Feb,...)
					// abbreviated month name
				case L'h':
				case L'b':
					strBuf=AMonth[CurLang][t->tm_mon];
					break;
					// Полное имя месяца
					// full month name
				case L'B':
					strBuf=Month[CurLang][t->tm_mon];
					break;
					//Дата и время в формате WDay Mnt  Day HH:MM:SS yyyy
					//appropriate date and time representation
				case L'c':
					atime(strBuf,t);
					break;
					// Столетие как десятичное число (00 - 99). Например, 1992 => 19
				case L'C':
					strBuf.Format(L"%02d",(t->tm_year+1900)/100);
					break;
					// day of month, blank padded
				case L'e':
					// Две цифры дня месяца (01 - 31)
					// day of the month, 01 - 31
				case L'd':
					strBuf.Format(*Format==L'e'?L"%2d":L"%02d",t->tm_mday);
					break;
					// hour, 24-hour clock, blank pad
				case L'k':
					// Две цифры часа (00 - 23)
					// hour, 24-hour clock, 00 - 23
				case L'H':
					strBuf.Format(*Format==L'k'?L"%2d":L"%02d",t->tm_hour);
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

					strBuf.Format(*Format==L'l'?L"%2d":L"%02d",I);
					break;
				}
				// Три цифры дня в году (001 - 366)
				// day of the year, 001 - 366
				case L'j':
					strBuf.Format(L"%03d",t->tm_yday+1);
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

					strBuf.Format(fmt,t->tm_mon+1);
					break;
				}
				// Две цифры минут (00 - 59)
				// minute, 00 - 59
				case L'M':
					strBuf.Format(L"%02d",t->tm_min);
					break;
					// AM или PM
					// am or pm based on 12-hour clock
				case L'p':
					strBuf=(t->tm_hour/12)?L"PM":L"AM";
					break;
					// Две цифры секунд (00 - 59)
					// second, 00 - 59
				case L'S':
					strBuf.Format(L"%02d",t->tm_sec);
					break;
					// День недели где 0 - Воскресенье (Sunday) (0 - 6)
					// weekday, Sunday == 0, 0 - 6
				case L'w':
					strBuf.Format(L"%d",t->tm_wday);
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

					strBuf.Format(L"%02d",(t->tm_yday+I-(*Format==L'W'))/7);
					break;
				}
				// date as dd-bbb-YYYY
				case L'v':
					// Дата в формате mm.dd.yyyy
					// appropriate date representation
				case L'D':
				case L'x':
					st_time(strBuf,t,*Format);
					break;
					// Время в формате HH:MM:SS
					// appropriate time representation
				case L'T':
				case L'X':
				{
					wchar_t TimeSeparator=GetTimeSeparator();
					strBuf.Format(L"%02d%c%02d%c%02d",t->tm_hour,TimeSeparator,t->tm_min,TimeSeparator,t->tm_sec);
					break;
				}
				// Две цифры года без столетия (00 to 99)
				// year without a century, 00 - 99
				case L'y':
					strBuf.Format(L"%02d",t->tm_year%100);
					break;
					// Год со столетием (19yy-20yy)
					// year with century
				case L'Y':
					strBuf.Format(L"%d",1900+t->tm_year);
					break;
					// Имя часового пояса или пусто, если часовой пояс не задан
				case L'Z':
					strBuf.Format(L"%+03d%02d",-(_timezone/3600),-(_timezone/60)%60);
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
					StrFTime(strBuf,L"%I:%M:%S %p",t);
					break;
					// time as %H:%M
				case L'R':
					StrFTime(strBuf,L"%H:%M",t);
					break;
					// week of year according ISO 8601
				case L'V':
					strBuf.Format(L"%02d",iso8601wknum(t));
					break;
			}

			strDest+=strBuf;
			Len+=strBuf.GetLength();
		}
#endif
	}

	if (*Format)
		return 0;

	return Len-1;
}

size_t MkStrFTime(string &strDest, const wchar_t *Fmt)
{
	tm *time_now;
	time_t secs_now;
	_tzset();
	time(&secs_now);
	time_now=localtime(&secs_now);

	if (!Fmt||!*Fmt)
		Fmt=Opt.Macro.strDateFormat;

	return StrFTime(strDest,Fmt,time_now);
}

void GetFileDateAndTime(const wchar_t *Src,LPWORD Dst,size_t Count,int Separator)
{
	for (size_t i=0; i<Count; i++)
	{
		Dst[i]=(WORD)-1;
	}

	string strDigit;
	const wchar_t *Ptr=Src;

	for (size_t i=0; i<Count; i++)
	{
		Ptr=GetCommaWord(Ptr,strDigit,Separator);

		if (Ptr)
		{
			const wchar_t *PtrDigit=strDigit;

			while (*PtrDigit&&!iswdigit(*PtrDigit))
			{
				PtrDigit++;
			}

			if (*PtrDigit)
			{
				Dst[i]=static_cast<WORD>(_wtoi(PtrDigit));
			}
		}
		else
		{
			break;
		}
	}
}

void StrToDateTime(const wchar_t *CDate, const wchar_t *CTime, FILETIME &ft, int DateFormat, wchar_t DateSeparator, wchar_t TimeSeparator, bool bRelative)
{
	WORD DateN[3]={},TimeN[4]={};
	SYSTEMTIME st={};
	// Преобразуем введённые пользователем дату и время
	GetFileDateAndTime(CDate,DateN,ARRAYSIZE(DateN),DateSeparator);
	GetFileDateAndTime(CTime,TimeN,ARRAYSIZE(TimeN),TimeSeparator);

	if (!bRelative)
	{
		if (DateN[0]==(WORD)-1||DateN[1]==(WORD)-1||DateN[2]==(WORD)-1)
		{
			// Пользователь оставил дату пустой, значит обнулим дату и время.
			ClearStruct(ft);
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
		ULARGE_INTEGER time;
		time.QuadPart = st.wMilliseconds;
		time.QuadPart += (UINT64)st.wSecond * 1000;
		time.QuadPart += (UINT64)st.wMinute * 1000 * 60;
		time.QuadPart += (UINT64)st.wHour   * 1000 * 60 * 60;
		time.QuadPart += (UINT64)st.wDay    * 1000 * 60 * 60 * 24;
		time.QuadPart *= 10000;
		ft.dwLowDateTime  = time.LowPart;
		ft.dwHighDateTime = time.HighPart;
	}
	else
	{
		FILETIME lft={};

		if (SystemTimeToFileTime(&st,&lft))
		{
			LocalFileTimeToFileTime(&lft,&ft);
		}
	}
}

void ConvertDate(const FILETIME &ft,string &strDateText, string &strTimeText,int TimeLength,
                 int Brief,int TextMonth,int FullYear,int DynInit)
{
	static int WDateFormat;
	static wchar_t WDateSeparator,WTimeSeparator,WDecimalSeparator;
	static bool Init=false;
	static SYSTEMTIME lt;
	int DateFormat;
	wchar_t DateSeparator,TimeSeparator,DecimalSeparator;

	if (!Init)
	{
		WDateFormat=GetDateFormat();
		WDateSeparator=GetDateSeparator();
		WTimeSeparator=GetTimeSeparator();
		WDecimalSeparator=GetDecimalSeparator();
		GetLocalTime(&lt);
		Init=true;
	}

	DateFormat=DynInit?GetDateFormat():WDateFormat;
	DateSeparator=DynInit?GetDateSeparator():WDateSeparator;
	TimeSeparator=DynInit?GetTimeSeparator():WTimeSeparator;
	DecimalSeparator=DynInit?GetDecimalSeparator():WDecimalSeparator;
	int CurDateFormat=DateFormat;

	if (Brief && CurDateFormat==2)
		CurDateFormat=0;

	SYSTEMTIME st;
	FILETIME ct;

	if (!ft.dwHighDateTime)
	{
		strDateText.Clear();
		strTimeText.Clear();
		return;
	}

	FileTimeToLocalFileTime(&ft,&ct);
	FileTimeToSystemTime(&ct,&st);
	//if ( !strTimeText.IsEmpty() )
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
			strTimeText.Format(L"%02d%c%02d%s",st.wHour,TimeSeparator,st.wMinute,Letter);
		else
		{
			string strFullTime;
			strFullTime.Format(L"%02d%c%02d%c%02d%c%03d",st.wHour,TimeSeparator,
			                   st.wMinute,TimeSeparator,st.wSecond,DecimalSeparator,st.wMilliseconds);
			strTimeText.Format(L"%.*s",TimeLength, strFullTime.CPtr());
		}
	}
	//if ( !strDateText.IsEmpty() )
	{
		int Year=st.wYear;

		if (!FullYear)
			Year%=100;

		if (TextMonth)
		{
			const wchar_t *Mnth=MSG(MMonthJan+(st.wMonth-1));

			switch (CurDateFormat)
			{
				case 0:
					strDateText.Format(L"%3.3s %2d %02d",Mnth,st.wDay,Year);
					break;
				case 1:
					strDateText.Format(L"%2d %3.3s %02d",st.wDay,Mnth,Year);
					break;
				default:
					strDateText.Format(L"%02d %3.3s %2d",Year,Mnth,st.wDay);
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
					f3=f1;
					f1=L' ';
					p2=st.wMonth;
					p3=st.wDay;
					break;
			}
			strDateText = FormatString()<<fmt::FillChar(f1)<<fmt::MinWidth(w1)<<p1<<DateSeparator<<fmt::FillChar(f2)<<fmt::MinWidth(w2)<<p2<<DateSeparator<<fmt::FillChar(f3)<<fmt::MinWidth(w3)<<p3;
		}
	}

	if (Brief)
	{
		strDateText.SetLength(TextMonth ? 6 : 5);

		if (lt.wYear!=st.wYear)
			strTimeText.Format(L"%5d",st.wYear);
	}
}

void ConvertRelativeDate(const FILETIME &ft,string &strDaysText,string &strTimeText)
{
	ULARGE_INTEGER time={ft.dwLowDateTime,ft.dwHighDateTime};

	UINT64 ms = (time.QuadPart/=10000)%1000;
	UINT64 s = (time.QuadPart/=1000)%60;
	UINT64 m = (time.QuadPart/=60)%60;
	UINT64 h = (time.QuadPart/=60)%24;
	UINT64 d = time.QuadPart/=24;

	strDaysText = FormatString()<<d;
	strTimeText = FormatString()<<fmt::MinWidth(2)<<fmt::FillChar(L'0')<<h<<GetTimeSeparator()<<fmt::MinWidth(2)<<fmt::FillChar(L'0')<<m<<GetTimeSeparator()<<fmt::MinWidth(2)<<fmt::FillChar(L'0')<<s<<GetDecimalSeparator()<<fmt::MinWidth(3)<<fmt::FillChar(L'0')<<ms;
}
