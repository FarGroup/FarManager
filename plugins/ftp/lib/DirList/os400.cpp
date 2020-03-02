#include <all_far.h>
#pragma hdrstop

#include "Int.h"

/*          1         2         3         4         5         6         7
  01234567890123456789012345678901234567890123456789012345678901234567890123456789
  06/27/06 19:12:53
  12/31/69 19:00:00
  07/10/06 16:21:25
  12/31/69 19:00:00
  07/05/06 06:08:14
  07/05/06 06:08:14
*/
BOOL parse_os400_date_time(LPSTR& line, Time_t& decoded)
{
	if(!line[0] ||
	        line[2] != '/' || line[5] != '/' ||
	        line[8] != ' ' ||
	        line[11] != ':' || line[14] != ':')
		return FALSE;

	SYSTEMTIME st;
	GetSystemTime(&st);
	st.wMilliseconds = 0;
//mon
	TwoDigits(line, st.wMonth);

	if(st.wMonth == MAX_WORD)
		return FALSE;

	line += 3;
//mday
	TwoDigits(line, st.wDay);

	if(st.wDay == MAX_WORD)
		return FALSE;

	line += 3;
//year
	TwoDigits(line, st.wYear);

	if(st.wYear == MAX_WORD)
		return FALSE;

	if(st.wYear < 50)
		st.wYear += 100;

	st.wYear += 1900;
	line += 3;
//Time
	TwoDigits(line+0, st.wHour);
	TwoDigits(line+3, st.wMinute);
	TwoDigits(line+6, st.wSecond);

	if(st.wHour   == MAX_WORD ||
	        st.wMinute == MAX_WORD ||
	        st.wSecond == MAX_WORD)
		return FALSE;

	line += 8;
	st.wDayOfWeek = 0;
	return SystemTimeToFileTime(&st, decoded);
}

/*
ROOT         21147648 05/29/08 10:16:58 *FILE      AMSAVF.SAVF
ROOT           217088 04/14/08 11:41:28 *FILE      CYN_COLLEC.FILE
ROOT                                    *MEM       CYN_COLLEC.FILE/SH.MBR
ROOT            16384 04/08/08 12:17:34 *FILE      EVFEVENT.FILE
ROOT           954368 04/08/08 11:48:21 *FILE      LIBCCLOG.SAVF
ROOT           163840 04/08/08 11:48:21 *FILE      LIBCFFDC.SAVF
ROOT           503808 04/08/08 11:48:21 *FILE      LIBMSG23.SAVF
QPGMR          143360 08/05/06 18:09:14 *FILE      QAAPFILE.FILE
QPGMR                                   *MEM       QAAPFILE.FILE/QAAPF1X1.MBR
QPGMR                                   *MEM       QAAPFILE.FILE/QAAPF1X2.MBR
QPGMR                                   *MEM       QAAPFILE.FILE/QAAPF2X2.MBR
QSYS           131072 06/27/06 19:12:53 *DIR       QOpenSys/
QDOC            69120 12/31/69 19:00:00 *FLR       QDLS/
QSYS         13778944 07/10/06 16:21:25 *LIB       QSYS.LIB/
QDFTOWN          4096 12/31/69 19:00:00 *DDIR      QOPT/
QSYS             2272 07/05/06 06:08:14 *DDIR      QFileSvr.400/
QDFTOWN          1200 07/05/06 06:08:14 *DDIR      QNTC/
QSYS            40960 06/27/06 19:10:15 *DIR       dev/
QSYS             8192 06/27/06 16:29:44 *DIR       home/
QSYS            12288 07/07/06 23:24:53 *DIR       tmp/
MUELLERJ      1282232 01/18/06 11:40:23 *STMF      PARCELST.TXT
MUELLERJ           58 01/03/06 11:29:17 *STMF      UNTITLED.TXT
*/
BOOL net_parse_ls_line(char *line, NET_FileEntryInfo* entry_info)
{
	char *e;
	int   len;
	static Time_t savedate;
//Owner
	e = SkipNSpace(line);
	CHECK((*e == 0), FALSE)
	len = Min((int)ARRAYSIZE(entry_info->FTPOwner)-1, (int)(e-line));
	StrCpy(entry_info->FTPOwner, line, len+1);
	line = SkipSpace(e);

	if(line[0] != '*')
	{
		//Size
		e = SkipNSpace(line);
		CHECK((*e == 0), FALSE)
		*e = 0;
		entry_info->size = AtoI(line, (__int64)-1);
		*e = ' ';
		CHECK((entry_info->size == (__int64)-1), FALSE)
		//Date
		line = SkipSpace(e);
		CHECK((!parse_os400_date_time(line,entry_info->date)), FALSE)
		//save date of *FILE for *MEM members
		savedate = entry_info->date;
		line = SkipSpace(line);
		CHECK((*line == 0), FALSE)
	}
	else
	{
		entry_info->size = 0;
		entry_info->date = savedate;
	}

	line = SkipNSpace(line);
//File name
	line = SkipSpace(line);
	CHECK((*line == 0), FALSE)
	StrCpy(entry_info->FindData.cFileName, line, ARRAYSIZE(entry_info->FindData.cFileName));
	len = static_cast<int>(strlen(entry_info->FindData.cFileName));

	if(entry_info->FindData.cFileName[len-1] == '/')
	{
		entry_info->FindData.cFileName[len-1] = 0;
		entry_info->FileType = NET_DIRECTORY;
	}

	return TRUE;
}

BOOL WINAPI idPRParceOS400(const FTPServerInfo* Server, FTPFileInfo* p, char *entry, int entry_len)
{
	NET_FileEntryInfo entry_info;

	if(!net_parse_ls_line(entry, &entry_info))
		return FALSE;

	return ConvertEntry(&entry_info,p);
}
