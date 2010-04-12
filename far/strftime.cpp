/*
strftime.cpp

Функция StrFTime

*/

#include "headers.hpp"
#pragma hdrstop

#include "fn.hpp"
#include "global.hpp"
#include "lang.hpp"

#define range(low, item, hi)    Max(low, Min(item, hi))

//extern char  *const _tzname[2];
static const char Days[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
static int YDays[12] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };
static char AMonth[2][12][16];
static char AWeekday[2][7][16];
static char Month[2][12][32];
static char Weekday[2][7][32];
static char *AmPm[2] = { "AM", "PM" };
static int   CurLang=-1;
static char  Word[80];
static int  WeekFirst=0;

void PrepareStrFTime(void)
{
	DWORD Loc[2]={LANG_ENGLISH,LANG_NEUTRAL}, ID;
	char TempBuf[100];
	int I;
	GetLocaleInfo(LOCALE_USER_DEFAULT,LOCALE_IFIRSTDAYOFWEEK,TempBuf,sizeof(TempBuf));
	WeekFirst=atoi(TempBuf);

	for (I=0; I < 2; ++I)
	{
		LCID CurLCID=MAKELCID(MAKELANGID(Loc[I],SUBLANG_DEFAULT),SORT_DEFAULT);

		for (ID=LOCALE_SMONTHNAME1; ID <= LOCALE_SMONTHNAME12; ++ID)
		{
			GetLocaleInfo(CurLCID,ID,TempBuf,sizeof(TempBuf));
			FAR_CharToOem(TempBuf,Month[I][ID-LOCALE_SMONTHNAME1]);
			*Month[I][ID-LOCALE_SMONTHNAME1]=LocalUpper(*Month[I][ID-LOCALE_SMONTHNAME1]);
		}

		for (ID=LOCALE_SABBREVMONTHNAME1; ID <= LOCALE_SABBREVMONTHNAME12; ++ID)
		{
			GetLocaleInfo(CurLCID,ID,TempBuf,sizeof(TempBuf));
			FAR_CharToOem(TempBuf,AMonth[I][ID-LOCALE_SABBREVMONTHNAME1]);
			*AMonth[I][ID-LOCALE_SABBREVMONTHNAME1]=LocalUpper(*AMonth[I][ID-LOCALE_SABBREVMONTHNAME1]);
		}

		for (ID=LOCALE_SDAYNAME1; ID <= LOCALE_SDAYNAME7; ++ID)
		{
			GetLocaleInfo(CurLCID,ID,TempBuf,sizeof(TempBuf));
			FAR_CharToOem(TempBuf,Weekday[I][ID-LOCALE_SDAYNAME1]);
			*Weekday[I][ID-LOCALE_SDAYNAME1]=LocalUpper(*Weekday[I][ID-LOCALE_SDAYNAME1]);
		}

		for (ID=LOCALE_SABBREVDAYNAME1; ID <= LOCALE_SABBREVDAYNAME7; ++ID)
		{
			GetLocaleInfo(CurLCID,ID,TempBuf,sizeof(TempBuf));
			FAR_CharToOem(TempBuf,AWeekday[I][ID-LOCALE_SABBREVDAYNAME1]);
			*AWeekday[I][ID-LOCALE_SABBREVDAYNAME1]=LocalUpper(*AWeekday[I][ID-LOCALE_SABBREVDAYNAME1]);
		}
	}

	CurLang=0;
}

static int atime(char *dest,const struct tm *tmPtr)
{
	// Thu Oct  7 12:37:32 1999
	return sprintf(dest, "%s %s %02d %02d:%02d:%02d %4d",
	               AWeekday[CurLang][!WeekFirst?((tmPtr->tm_wday+6)%7):(tmPtr->tm_wday == 0?6:tmPtr->tm_wday-1)],
	               AMonth[CurLang][tmPtr->tm_mon],
	               tmPtr->tm_mday,
	               tmPtr->tm_hour,
	               tmPtr->tm_min,
	               tmPtr->tm_sec,
	               tmPtr->tm_year + 1900
	              );
}

static int st_time(char *dest,const struct tm *tmPtr,char chr)
{
	int res,i;
	int DateSeparator=GetDateSeparator();

	if (chr == 'v')
	{
		res=sprintf(dest, "%2d-%3.3s-%4d",
		            range(1, tmPtr->tm_mday, 31),
		            AMonth[CurLang][range(0, tmPtr->tm_mon, 11)],
		            tmPtr->tm_year + 1900);

		for (i = 3; i < 6; i++)
			if (LocalIslower(dest[i]))
				dest[i] = LocalUpper(dest[i]);
	}
	else
		switch (GetDateFormat())
		{
			case 0:
				res=sprintf(dest, "%02d%c%02d%c%4d",
				            tmPtr->tm_mon+1,DateSeparator,
				            tmPtr->tm_mday,DateSeparator,
				            tmPtr->tm_year + 1900);
				break;
			case 1:
				res=sprintf(dest, "%02d%c%02d%c%4d",
				            tmPtr->tm_mday,DateSeparator,
				            tmPtr->tm_mon+1,DateSeparator,
				            tmPtr->tm_year + 1900);
				break;
			default:
				res=sprintf(dest, "%4d%c%02d%c%02d",
				            tmPtr->tm_year + 1900,DateSeparator,
				            tmPtr->tm_mon+1,DateSeparator,
				            tmPtr->tm_mday);
				break;
		}

	return res;
}


/* weeknumber --- figure how many weeks into the year */
static int weeknumber(const struct tm *timeptr, int firstweekday)
{
	int wday = timeptr->tm_wday;
	int ret;

	if (firstweekday == 1)
	{
		if (wday == 0)  /* sunday */
			wday = 6;
		else
			wday--;
	}

	ret = ((timeptr->tm_yday + 7 - wday) / 7);

	if (ret < 0)
		ret = 0;

	return ret;
}

/* isleap --- is a year a leap year? */
static int isleap(int year)
{
	return ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0);
}

static int iso8601wknum(const struct tm *timeptr)
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
	int weeknum, jan1day;
	/* get week number, Monday as first day of the week */
	weeknum = weeknumber(timeptr, 1);
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
	jan1day = timeptr->tm_wday - (timeptr->tm_yday % 7);

	if (jan1day < 0)
		jan1day += 7;

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
		case 1:     /* Monday */
			break;
		case 2:     /* Tuesday */
		case 3:     /* Wednesday */
		case 4:     /* Thursday */
			weeknum++;
			break;
		case 5:     /* Friday */
		case 6:     /* Saturday */
		case 0:     /* Sunday */

			if (weeknum == 0)
			{
#ifdef USE_BROKEN_XPG4
				/* XPG4 (as of March 1994) says 53 unconditionally */
				weeknum = 53;
#else
				/* get week number of last week of last year */
				struct tm dec31ly;  /* 12/31 last year */
				dec31ly = *timeptr;
				dec31ly.tm_year--;
				dec31ly.tm_mon = 11;
				dec31ly.tm_mday = 31;
				dec31ly.tm_wday = (jan1day == 0) ? 6 : jan1day - 1;
				dec31ly.tm_yday = 364 + isleap(dec31ly.tm_year + 1900);
				weeknum = iso8601wknum(& dec31ly);
#endif
			}

			break;
	}

	if (timeptr->tm_mon == 11)
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
		int wday, mday;
		wday = timeptr->tm_wday;
		mday = timeptr->tm_mday;

		if ((wday == 1 && (mday >= 29 && mday <= 31))
		        || (wday == 2 && (mday == 30 || mday == 31))
		        || (wday == 3 &&  mday == 31))
			weeknum = 1;
	}

	return weeknum;
}

int MkStrFTime(char *Dest,int DestSize,const char *Fmt)
{
	struct tm *time_now;
	time_t secs_now;
	char Fmt0[NM];
	tzset();
	time(&secs_now);
	time_now = localtime(&secs_now);
	xstrncpy(Fmt0,!Fmt || !Fmt[0]?Opt.DateFormat:Fmt,sizeof(Fmt0));
	return StrFTime(Dest, DestSize, Fmt0, time_now);
}


/*
Вызов:
struct tm *time_now;
time_t secs_now;
char str[80];

tzset();
time(&secs_now);
time_now = localtime(&secs_now);
StrFTime(str, 80,
            "It is %M minutes after %I o'clock (%Z)  %A, %B %d 19%y",
            time_now);


$Date(%d-%a-%Y)
*/
int WINAPI StrFTime(char *Dest, size_t MaxSize, const char *Format,const struct tm *t)
{
	char Buf[32];
	char chr;
	char *Ptr = Buf;
	size_t I;
	size_t Len;

	if (!Dest || !MaxSize)
		return 0;

	if (CurLang == -1 && LanguageLoaded)
		PrepareStrFTime();

	int TimeSeparator=GetTimeSeparator();
	// меняем язык.
	CurLang=0;

	for (Len = 1; Len < MaxSize && *Format; ++Format, Ptr = Buf)
	{
		if (*Format != '%')  { ++Len; *Dest++ = *Format; }
		else
		{
			Buf[1]=Buf[0]='0'; Buf[2]='\0';
			chr=*++Format;

			switch (chr)
			{
				case 'L':
					CurLang=CurLang?0:1;
					continue;
					// Краткое имя дня недели (Sun,Mon,Tue,Wed,Thu,Fri,Sat)
					// abbreviated weekday name
				case 'a':
					Ptr = AWeekday[CurLang][!WeekFirst?((t->tm_wday+6)%7):(t->tm_wday == 0?6:t->tm_wday-1)];
					break;
					// Полное имя дня недели
					// full weekday name
				case 'A':
					Ptr = Weekday[CurLang][!WeekFirst?((t->tm_wday+6)%7):(t->tm_wday == 0?6:t->tm_wday-1)];;
					break;
					// Краткое имя месяца (Jan,Feb,...)
					// abbreviated month name
				case 'h':
				case 'b': Ptr = AMonth[CurLang][t->tm_mon];    break;
					// Полное имя месяца
					// full month name
				case 'B': Ptr = Month[CurLang][t->tm_mon];    break;
					// Дата и время в формате WDay Mnt  Day HH:MM:SS yyyy
					// appropriate date and time representation
				case 'c': atime(Ptr, t);                   break;
					// Столетие как десятичное число (00 - 99). Например, 1992 => 19
					//
				case 'C':
					I = (t->tm_year+1900) / 100;
					ultoa((long)(I&0xffff), Buf + (I < 10),10);
					break;
					// day of month, blank padded
				case 'e': Buf[0]=' ';
					// Две цифры дня месяца (01 - 31)
					// day of the month, 01 - 31
				case 'd':
					ultoa((long)(t->tm_mday&0xffff),Buf+(t->tm_mday < 10),10);
					break;
					// hour, 24-hour clock, blank pad
				case 'k': Buf[0]=' ';
					// Две цифры часа (00 - 23)
					// hour, 24-hour clock, 00 - 23
				case 'H': ultoa((long)(t->tm_hour&0xffff),Buf+(t->tm_hour < 10),10); break;
					// hour, 12-hour clock, 1 - 12, blank pad
				case 'l': Buf[0]=' ';
					// Две цифры часа (01 - 12)
					// hour, 12-hour clock, 01 - 12
				case 'I':
					I = t->tm_hour % 12;

					if (I == 0) I = 12;

					ultoa((long)(I&0xffff),Buf + (I < 10),10);
					break;
					// Три цифры дня в году (001 - 366)
					// day of the year, 001 - 366
				case 'j':
				{
					I = t->tm_yday + 1;
					ultoa((long)(I&0xffff), Buf + (I < 10) + (I < 100),10);
					break;
				}
				// Две цифры месяца, как десятичное число (1 - 12)
				// month, 01 - 12
				case 'm':
				{
					int Hex=Format[1] == 'h'; // %mh - Hex month digit
					int N=Format[1] == '0'; // %m0 - ведущий 0
					I = t->tm_mon + 1;
					ultoa((long)(I&0xffff), Buf + (I < 10 && !Hex && N?1:0),Hex?16:10);

					if (Hex)
						Buf[0]=toupper(Buf[0]);

					if (Hex || N)
						++Format;

					break;
				}
				// Две цифры минут (00 - 59)
				// minute, 00 - 59
				case 'M':
					ultoa((long)(t->tm_min&0xffff), Buf + (t->tm_min < 10),10);
					break;
					// AM или PM
					// am or pm based on 12-hour clock
				case 'p':
					Ptr = AmPm[ t->tm_hour / 12 ];
					break;
					// Две цифры секунд (00 - 59)
					// second, 00 - 60
				case 'S':
					ultoa((long)(t->tm_sec&0xffff), Buf + (t->tm_sec < 10),10);
					break;
					// День недели где 0 - Воскресенье (Sunday) (0 - 6)
					// weekday, Sunday == 0, 0 - 6
				case 'w':
					ultoa((long)(t->tm_wday&0xffff), Buf ,10);
					break;
					// Две цифры номера недели, где Воскресенье (Sunday)
					//   является первым днем недели (00 - 53)
					// week of year, Sunday is first day of week
				case 'U':
					// Две цифры номера недели, где Понедельник (Monday)
					//    является первым днем недели (00 - 53)
					// week of year, Monday is first day of week
				case 'W':
					I = t->tm_wday - (t->tm_yday % 7);

					//I = (chr == 'W'?(!WeekFirst?((t->tm_wday+6)%7):(t->tm_wday == 0?6:t->tm_wday-1)):(t->tm_wday)) - (t->tm_yday % 7);
					if (I < 0)
						I += 7;

					I = (t->tm_yday + I - (chr == 'W'?1:0)) / 7;
					ultoa((long)(I&0xffff), Buf + (I < 10),10);
					break;
					// date as dd-bbb-YYYY
				case 'v':
					// Дата в формате mm.dd.yyyy
					// appropriate date representation
				case 'D':
				case 'x': st_time(Ptr, t, chr); break;
					// Время в формате HH:MM:SS
					// appropriate time representation
				case 'T':
				case 'X':
					sprintf(Ptr,"%02d%c%02d%c%02d",t->tm_hour,TimeSeparator,t->tm_min,TimeSeparator,t->tm_sec);
					break;
					// Две цифры года без столетия (00 to 99)
					// year without a century, 00 - 99
				case 'y':
					I = t->tm_year % 100;
					ultoa((long)(I&0xffff), Buf + (I < 10),10);
					break;
					// Год со столетием (19yy-20yy)
					// year with century
				case 'Y':
					ultoa((unsigned long)((1900 + t->tm_year)), Buf,10);
					break;
					// time zone offset east of GMT e.g. -0600
					//case 'z':
					//sprintf(Ptr,"%d %d",WeekFirst,t->tm_wday);
					//break;
					//case 'z':
//          sprintf(Ptr,"%+03d%02d",-(_timezone/3600),-(_timezone/60)%60);
//          //Ptr = _tzname[ t->tm_isdst ];
//          break;
					// Имя часового пояса или пусто, если часовой пояс не задан
				case 'Z':
				{
					sprintf(Buf,"%+03d%02d",-(_timezone/3600),-(_timezone/60)%60);
					//Ptr = _tzname[ t->tm_isdst ];
					break;
				}
				case 'n':  /* same as \n */
					Buf[0] = '\n';
					Buf[1] = '\0';
					break;
				case 't':  /* same as \t */
					Buf[0] = '\t';
					Buf[1] = '\0';
					break;
				case '%':
					Ptr = "%"; break;
					// time as %I:%M:%S %p
				case 'r':
					StrFTime(Buf, sizeof(Buf), "%I:%M:%S %p", t); break;
					// time as %H:%M
				case 'R':
					StrFTime(Buf, sizeof(Buf), "%H:%M", t); break;
				case 'V':  /* week of year according ISO 8601 */
					sprintf(Buf, "%02d", iso8601wknum(t));
					break;
			}

			I=(strlen(Ptr) < MaxSize-Len?strlen(Ptr):MaxSize-Len);
			xstrncpy(Dest, Ptr, I);
			Len += I;
			Dest += I;
			Ptr = Buf;
		}
	}

	*Dest = '\0';

	if (*Format)
		return(0);

	return (int)(Len-1);
} /* StrFTime */
