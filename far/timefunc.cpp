/*
timefunc.cpp

функции работы с датой/временем.

*/

#include "headers.hpp"
#pragma hdrstop

#include "lang.hpp"
#include "fn.hpp"


DWORD NTTimeToDos(FILETIME *ft)
{
	WORD DosDate,DosTime;
	FILETIME ct;
	FileTimeToLocalFileTime(ft,&ct);
	FileTimeToDosDateTime(&ct,&DosDate,&DosTime);
	return(((DWORD)DosDate<<16)|DosTime);
}


// В Dst получить числа для даты времени
BOOL GetFileDateAndTime(const char *Src,unsigned *Dst,int Separator)
{
	Dst[0]=Dst[1]=Dst[2]=(unsigned)-1;

	if (Src && *Src)
	{
		char Temp[32], Digit[16],*PtrDigit;
		xstrncpy(Temp,Src,sizeof(Temp)-1);
		const char *Ptr=Temp;
		int I=0;

		while ((Ptr=GetCommaWord(Ptr,Digit,Separator)) != NULL)
		{
			PtrDigit=Digit;

			while (*PtrDigit && !isdigit(*PtrDigit))
				PtrDigit++;

			if (*PtrDigit)
				Dst[I]=atoi(PtrDigit);

			if (++I > 2) //не должно быть больше трёх чисел
				break;
		}

		return TRUE;
	}

	return FALSE;
}

BOOL StrToDateTime(const char *CDate,const char *CTime,FILETIME &ft, int DateFormat, int DateSeparator, int TimeSeparator, bool bRelative)
{
	unsigned DateN[3],TimeN[3];
	SYSTEMTIME st;
	FILETIME lft;
	// Преобразуем введённые пользователем дату и время
	GetFileDateAndTime(CDate,DateN,DateSeparator);
	GetFileDateAndTime(CTime,TimeN,TimeSeparator);

	if (!bRelative)
	{
		if (DateN[0] == -1 || DateN[1] == -1 || DateN[2] == -1)
		{
			// Пользователь оставил дату пустой, значит обнулим дату и время.
			memset(&ft,0,sizeof(ft));
			return FALSE;
		}

		memset(&st,0,sizeof(st));

		// "Оформим"
		switch (DateFormat)
		{
			case 0:
				st.wMonth=DateN[0]!=(unsigned)-1?DateN[0]:0;
				st.wDay  =DateN[1]!=(unsigned)-1?DateN[1]:0;
				st.wYear =DateN[2]!=(unsigned)-1?DateN[2]:0;
				break;
			case 1:
				st.wDay  =DateN[0]!=(unsigned)-1?DateN[0]:0;
				st.wMonth=DateN[1]!=(unsigned)-1?DateN[1]:0;
				st.wYear =DateN[2]!=(unsigned)-1?DateN[2]:0;
				break;
			default:
				st.wYear =DateN[0]!=(unsigned)-1?DateN[0]:0;
				st.wMonth=DateN[1]!=(unsigned)-1?DateN[1]:0;
				st.wDay  =DateN[2]!=(unsigned)-1?DateN[2]:0;
				break;
		}

		if (st.wYear<100)
			if (st.wYear<80)
				st.wYear+=2000;
			else
				st.wYear+=1900;
	}
	else
		st.wDay = DateN[0]!=(unsigned)-1?DateN[0]:0;

	st.wHour   = TimeN[0]!=(unsigned)-1?(TimeN[0]):0;
	st.wMinute = TimeN[1]!=(unsigned)-1?(TimeN[1]):0;
	st.wSecond = TimeN[2]!=(unsigned)-1?(TimeN[2]):0;

	// преобразование в "удобоваримый" формат
	if (bRelative)
	{
		ULARGE_INTEGER time;
		time.QuadPart  = (unsigned __int64)st.wSecond * _ui64(10000000);
		time.QuadPart += (unsigned __int64)st.wMinute * _ui64(10000000) * _ui64(60);
		time.QuadPart += (unsigned __int64)st.wHour   * _ui64(10000000) * _ui64(60) * _ui64(60);
		time.QuadPart += (unsigned __int64)st.wDay    * _ui64(10000000) * _ui64(60) * _ui64(60) * _ui64(24);
		ft.dwLowDateTime  = time.u.LowPart;
		ft.dwHighDateTime = time.u.HighPart;
	}
	else
	{
		SystemTimeToFileTime(&st,&lft);
		LocalFileTimeToFileTime(&lft,&ft);
	}

	return TRUE;
}


int ReadFileTime(int Type,const char *Name,DWORD FileAttr,FILETIME *FileTime,char *OSrcDate,char *OSrcTime)
{
	FILETIME ft, oft;
	SYSTEMTIME st, ost;
	unsigned DateN[3],TimeN[3];
	int DigitCount;
	int /*SetTime,*/GetTime;
	FILETIME *OriginalFileTime=0, OFTModify, OFTCreate, OFTLast;
	// ****** ОБРАБОТКА ДАТЫ ******** //
	GetFileDateAndTime(OSrcDate,DateN,GetDateSeparator());
	// ****** ОБРАБОТКА ВРЕМЕНИ ******** //
	GetFileDateAndTime(OSrcTime,TimeN,GetTimeSeparator());

	// исключаем лишние телодвижения
	if (DateN[0] == -1 || DateN[1] == -1 || DateN[2] == -1 ||
	        TimeN[0] == -1 || TimeN[1] == -1 || TimeN[2] == -1)
	{
		// получаем инфу про оригинальную дату и время файла.
		HANDLE hFile=FAR_CreateFile(Name,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,
		                            NULL,OPEN_EXISTING,
		                            (FileAttr & FA_DIREC) ? FILE_FLAG_BACKUP_SEMANTICS:0,NULL);

		if (hFile==INVALID_HANDLE_VALUE)
			return(FALSE);

		GetTime=GetFileTime(hFile,&OFTCreate,&OFTLast,&OFTModify);
		CloseHandle(hFile);

		if (!GetTime)
			return(FALSE);

		switch (Type)
		{
			case 0: // Modif
				OriginalFileTime=&OFTModify;
				break;
			case 1: // Creat
				OriginalFileTime=&OFTCreate;
				break;
			case 2: // Last
				OriginalFileTime=&OFTLast;
				break;
		}

		// конвертнем в локальное время.
		FileTimeToLocalFileTime(OriginalFileTime,&oft);
		FileTimeToSystemTime(&oft,&ost);
		DigitCount=TRUE;
	}
	else
		DigitCount=FALSE;

	memset(&st,0,sizeof(st));

	// "Оформим"
	switch (GetDateFormat())
	{
		case 0:
			st.wMonth=DateN[0]!=(unsigned)-1?DateN[0]:ost.wMonth;
			st.wDay  =DateN[1]!=(unsigned)-1?DateN[1]:ost.wDay;
			st.wYear =DateN[2]!=(unsigned)-1?DateN[2]:ost.wYear;
			break;
		case 1:
			st.wDay  =DateN[0]!=(unsigned)-1?DateN[0]:ost.wDay;
			st.wMonth=DateN[1]!=(unsigned)-1?DateN[1]:ost.wMonth;
			st.wYear =DateN[2]!=(unsigned)-1?DateN[2]:ost.wYear;
			break;
		default:
			st.wYear =DateN[0]!=(unsigned)-1?DateN[0]:ost.wYear;
			st.wMonth=DateN[1]!=(unsigned)-1?DateN[1]:ost.wMonth;
			st.wDay  =DateN[2]!=(unsigned)-1?DateN[2]:ost.wDay;
			break;
	}

	st.wHour   = TimeN[0]!=(unsigned)-1? (TimeN[0]):ost.wHour;
	st.wMinute = TimeN[1]!=(unsigned)-1? (TimeN[1]):ost.wMinute;
	st.wSecond = TimeN[2]!=(unsigned)-1? (TimeN[2]):ost.wSecond;

	if (st.wYear<100)
		if (st.wYear<80)
			st.wYear+=2000;
		else
			st.wYear+=1900;

	if (TimeN[0]==(unsigned)-1 && TimeN[1]==(unsigned)-1 && TimeN[2]==(unsigned)-1)
	{
		st.wMilliseconds=ost.wMilliseconds;
		// для правильности выставления wDayOfWeek
		//SystemTimeToFileTime(&st,&ft);
		//FileTimeToSystemTime(&ft,&st);
	}

	// преобразование в "удобоваримый" формат
	SystemTimeToFileTime(&st,&ft);
	LocalFileTimeToFileTime(&ft,FileTime);

	if (DigitCount)
		return (CompareFileTime(FileTime,OriginalFileTime) == 0)?FALSE:TRUE;

	return TRUE;
}

void ConvertDate(const FILETIME &ft,char *DateText,char *TimeText,int TimeLength,
                 int Brief,int TextMonth,int FullYear,int DynInit)
{
	static int WDateFormat,WDateSeparator,WTimeSeparator;
	static int Init=FALSE;
	static SYSTEMTIME lt;
	int DateFormat,DateSeparator,TimeSeparator;

	if (!Init)
	{
		WDateFormat=GetDateFormat();
		WDateSeparator=GetDateSeparator();
		WTimeSeparator=GetTimeSeparator();
		GetLocalTime(&lt);
		Init=TRUE;
	}

	DateFormat=DynInit?GetDateFormat():WDateFormat;
	DateSeparator=DynInit?GetDateSeparator():WDateSeparator;
	TimeSeparator=DynInit?GetTimeSeparator():WTimeSeparator;
	int CurDateFormat=DateFormat;

	if (Brief && CurDateFormat==2)
		CurDateFormat=0;

	SYSTEMTIME st;
	FILETIME ct;

	if (ft.dwHighDateTime==0)
	{
		if (DateText!=NULL)
			*DateText=0;

		if (TimeText!=NULL)
			*TimeText=0;

		return;
	}

	FileTimeToLocalFileTime(&ft,&ct);
	FileTimeToSystemTime(&ct,&st);

	if (TimeText!=NULL)
	{
		const char *Letter="";

		if (TimeLength==6)
		{
			Letter=(st.wHour<12) ? "a":"p";

			if (st.wHour>12)
				st.wHour-=12;

			if (st.wHour==0)
				st.wHour=12;
		}

		if (TimeLength<7)
			sprintf(TimeText,"%02d%c%02d%s",st.wHour,TimeSeparator,st.wMinute,Letter);
		else
		{
			char FullTime[100];
			sprintf(FullTime,"%02d%c%02d%c%02d.%03d",st.wHour,TimeSeparator,
			        st.wMinute,TimeSeparator,st.wSecond,st.wMilliseconds);
			sprintf(TimeText,"%.*s",TimeLength,FullTime);
		}
	}

	if (DateText!=NULL)
	{
		int Year=st.wYear;

		if (!FullYear)
			Year%=100;

		if (TextMonth)
		{
			char *Month=MSG(MMonthJan+st.wMonth-1);

			switch (CurDateFormat)
			{
				case 0:
					sprintf(DateText,"%3.3s %2d %02d",Month,st.wDay,Year);
					break;
				case 1:
					sprintf(DateText,"%2d %3.3s %02d",st.wDay,Month,Year);
					break;
				default:
					sprintf(DateText,"%02d %3.3s %2d",Year,Month,st.wDay);
					break;
			}
		}
		else
		{
			int p1,p2,p3=Year;

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
					p2=st.wMonth;
					p3=st.wDay;
					break;
			}

			sprintf(DateText,"%02d%c%02d%c%02d",p1,DateSeparator,p2,DateSeparator,p3);
		}
	}

	if (Brief)
	{
		DateText[TextMonth ? 6:5]=0;

		if (lt.wYear!=st.wYear)
			sprintf(TimeText,"%5d",st.wYear);
	}
}

void ConvertRelativeDate(const FILETIME &ft,char *DaysText,char *TimeText)
{
	WORD d,h,m,s;
	ULARGE_INTEGER time;
	time.u.LowPart  = ft.dwLowDateTime;
	time.u.HighPart = ft.dwHighDateTime;

	if (time.QuadPart == _ui64(0))
	{
		*DaysText=0;
		*TimeText=0;
		return;
	}

	d = (WORD)(time.QuadPart / (_ui64(10000000) * _ui64(60) * _ui64(60) * _ui64(24)));
	time.QuadPart = time.QuadPart - ((unsigned __int64)d * _ui64(10000000) * _ui64(60) * _ui64(60) * _ui64(24));
	h = (WORD)(time.QuadPart / (_ui64(10000000) * _ui64(60) * _ui64(60)));
	time.QuadPart = time.QuadPart - ((unsigned __int64)h * _ui64(10000000) * _ui64(60) * _ui64(60));
	m = (WORD)(time.QuadPart / (_ui64(10000000) * _ui64(60)));
	time.QuadPart = time.QuadPart - ((unsigned __int64)m * _ui64(10000000) * _ui64(60));
	s = (WORD)(time.QuadPart / _ui64(10000000));
	itoa(d, DaysText, 10);
	sprintf(TimeText, "%02d%c%02d%c%02d", h, GetTimeSeparator(), m, GetTimeSeparator(), s);
}

int GetDateFormat()
{
	char Info[100];
	GetLocaleInfo(LOCALE_USER_DEFAULT,LOCALE_IDATE,Info,sizeof(Info));
	return(atoi(Info));
}


int GetDateSeparator()
{
	char Info[100];
	GetLocaleInfo(LOCALE_USER_DEFAULT,LOCALE_SDATE,Info,sizeof(Info));
	return(*Info);
}


int GetTimeSeparator()
{
	char Info[100];
	GetLocaleInfo(LOCALE_USER_DEFAULT,LOCALE_STIME,Info,sizeof(Info));
	return(*Info);
}
