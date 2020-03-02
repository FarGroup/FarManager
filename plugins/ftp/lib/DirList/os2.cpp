#include <all_far.h>
#pragma hdrstop

#include "Int.h"

/* a dos date/time string looks like this
 *           1         2         3         4         5
 * 012345678901234567890123456789012345678901234567890
 * 04-06-95   02:03
 * 07-13-102   21:39
 */
BOOL net_parse_os_date_time(LPCSTR datestr, Time_t& decoded)
{
	if(datestr[2] != '-' || datestr[5] != '-')
		return FALSE;

	SYSTEMTIME st;
	GetSystemTime(&st);
	st.wMilliseconds = 0;
	BOOL tmOff = !NET_IS_SPACE(datestr[8]);

	if(datestr[13+tmOff] != ':')
		return FALSE;

	if(datestr[0] == ' ')
		st.wMonth = datestr[1]-'0';
	else
		st.wMonth = (datestr[0]-'0')*10 + (datestr[1]-'0');

	st.wDay = ((datestr[3]-'0')*10) + (datestr[4]-'0');
	st.wYear = ((datestr[6]-'0')*10) + (datestr[7]-'0');

	if(!NET_IS_SPACE(datestr[8]))
		st.wYear = st.wYear*10 + (datestr[8]-'0');

	if(st.wYear < 50)
		st.wYear += 100;

	st.wYear += 1900;
	st.wHour      = ((datestr[11+tmOff]-'0')*10) + (datestr[12+tmOff]-'0');
	st.wMinute    = ((datestr[14+tmOff]-'0')*10) + (datestr[15+tmOff]-'0');
	st.wDayOfWeek = 0;
	st.wSecond    = 0;
	return SystemTimeToFileTime(&st, decoded);
}

/*
 *            1         2         3         4         5
 *  012345678901234567890123456789012345678901234567890123456789
                  8807      A         11-23-102   13:37  22BAL.GOM
                  8807      A         11-24-102   10:05  23BAL.GOM
                   512           DIR   07-22-99   16:41  Р1
                  1024      R    DIR   11-16-98   17:24  h
                513910      A         01-07-100   13:44  wined241.zip
                513910      A         01-07-100   13:44  wined241.zip
                513910      A         01-07-100   13:44  wined241.zip
                513910      A         01-07-100   13:44  wined241.zip

                  3259      A          04-07-00   11:23  24250C2E.PKT
               1221444      A          04-04-00   08:01  bcb4up2.exe
    >> обычные файлы с Arhive атрибутом

                293483     RA          04-02-00   08:55  Mablag.rar
    >> read-only + arhive

                     2                 04-07-00   15:14  no-attr
    >> файл без отрибутов

                     6      R          04-07-00   15:10  readonly
    >> файл просто с read-only

                     6    RSA          04-07-00   15:12  ro-a-sys
    >> read-only + system + archive

                 87264      A          03-25-00   07:48  SP Dialer.rar
    >> файл с пробелом в имени

                     0      S          04-07-00   15:09  system
    >> просто system атрибут
*/
BOOL WINAPI idPRParceOS2(const FTPServerInfo* Server, FTPFileInfo* p, char *entry, int entry_len)
{
	NET_FileEntryInfo entry_info;
	char             *start,*m;

	if(entry_len < 54)
		return FALSE;

	//Date + time
	if(!net_parse_os_date_time(entry + 34 + (entry[34] == ' '), entry_info.date))
		return FALSE;

	//Name
	StrCpy(entry_info.FindData.cFileName, entry+53, ARRAYSIZE(entry_info.FindData.cFileName));
	//Size
	start = SkipSpace(entry);
	m     = SkipNSpace(start);
	*m = 0;
	entry_info.size = AtoI(start, (__int64)-1);
	*m = ' ';

	if(entry_info.size == (__int64)-1)
		entry_info.size = 0;

	//Attributes
	start = entry;
	m     = entry + 24;

	for(; m > start && !NET_IS_SPACE(*m); m--)
		switch(NET_TO_UPPER(*m))
		{
			case 'R': SET_FLAG(entry_info.FindData.dwFileAttributes, FILE_ATTRIBUTE_READONLY); break;
			case 'A': SET_FLAG(entry_info.FindData.dwFileAttributes, FILE_ATTRIBUTE_ARCHIVE); break;
			case 'S': SET_FLAG(entry_info.FindData.dwFileAttributes, FILE_ATTRIBUTE_SYSTEM); break;
			case 'H': SET_FLAG(entry_info.FindData.dwFileAttributes, FILE_ATTRIBUTE_HIDDEN); break;
			default: Log(("UnknownAttr: \'%c\' [%s]", *m, entry));
		}

	if(m == start)
		return FALSE;

	//Directory
	if(StrNCmpI(entry+29,"DIR",3) == 0)
	{
		SET_FLAG(entry_info.FindData.dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY);
		entry_info.size = 0;
	}

	return ConvertEntry(&entry_info,p);
}
