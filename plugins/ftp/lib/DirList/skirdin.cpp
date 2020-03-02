#include <all_far.h>
#pragma hdrstop

#include "Int.h"

/*
 *            1         2         3         4         5
 *  012345678901234567890123456789012345678901234567890123456789
    15.54:21 16.04.2003  FF.FF:FF FF.FF.FFFF  0000000001 TTSCOF02
  Ignore
    00.00:00 00.00.0000
    FF.FF:FF FF.FF.FFFF
  Datetime:
    20.18:04 26.11.2002
*/
BOOL net_parse_full_date_time(char *datestr, Time_t& decoded)
{
	static const char FF[]="FF.FF:FF FF.FF.FFFF",
	                       ZZ[]="00.00:00 00.00.0000";
	SYSTEMTIME st;

	if(datestr[2] != '.' || datestr[5] != ':' ||
	        datestr[11] != '.' || datestr[14] != '.')
		return FALSE;

	GetSystemTime(&st);
	st.wMilliseconds = 0;

//Ignore FF and 00
//
	if(StrCmp(datestr,FF,19) == 0 ||
	        StrCmp(datestr,ZZ,19) == 0)
	{
		decoded = NOT_TIME;
		return TRUE;
	}

	datestr[ 2] = 0;
	datestr[ 5] = 0;
	datestr[ 8] = 0;
	datestr[11] = 0;
	datestr[14] = 0;

//Time
	if(!TwoDigits(datestr+0,st.wHour) ||
	        !TwoDigits(datestr+3,st.wMinute) ||
	        !TwoDigits(datestr+6,st.wSecond))
		return FALSE;

//Date
	if(!TwoDigits(datestr+ 9,st.wDay) ||
	        !TwoDigits(datestr+12,st.wMonth) ||
	        (st.wYear=AtoI(datestr+15,MAX_WORD)) == MAX_WORD)
		return FALSE;

	st.wDayOfWeek  = 0;
	return SystemTimeToFileTime(&st, decoded);
}
/*
 *            1         2         3         4         5
 *  012345678901234567890123456789012345678901234567890123456789
    00.00:00 00.00.0000  00.00:00 00.00.0000  0000000000 SECTORS.BAK
    00.00:00 00.00.0000  00.00:00 00.00.0000  0000000410 GSMBIL.DIR
    00.00:00 00.00.0000  00.00:00 00.00.0000  0000000410 VIDAST.DIR
    22.13:45 18.11.2002  20.18:04 26.11.2002  0000000014 TTTCOF00.IMG
    20.18:04 26.11.2002  02.40:31 07.11.2002  0000000018 TTSCOF00.VDS
    15.54:21 16.04.2003  FF.FF:FF FF.FF.FFFF  0000000001 TTSCOF02

   ! Need to delete trailing spaces
   ! Size is in blocks by 512 bytes each
*/
BOOL WINAPI idPRParceSkirdin(const FTPServerInfo* Server, FTPFileInfo* p, char *entry, int entry_len)
{
	NET_FileEntryInfo entry_info;

	if(entry_len < 53 || NET_IS_SPACE(entry[53]) ||
	        !NET_IS_SPACE(entry[19]) ||
	        !NET_IS_SPACE(entry[40]) ||
	        !NET_IS_SPACE(entry[52]))
		return FALSE;

	entry[19] = 0;
	entry[40] = 0;
	entry[52] = 0;

//Time + size
	if(!net_parse_full_date_time(entry+0,  entry_info.date) ||
	        !net_parse_full_date_time(entry+21, entry_info.acc_date) ||
	        (entry_info.size=AtoI(entry+42,(__int64)-1)) == -1)
		return FALSE;

	entry_info.size *= 512;

//Del trailing spaces
	for(entry_len--;                       //Skips trail zero
	        NET_IS_SPACE(entry[entry_len]);
	        entry_len--);

	entry[++entry_len] = 0;
//File name
	entry += 53;
	size_t len = strlen(entry);

	if(len > 4)
	{
		char *e = entry + len - 4;

		if(StrCmp(e, ".DIR") == 0)
		{
			*e = 0;
			entry_info.FileType = NET_DIRECTORY;
		}
	}

	StrCpy(entry_info.FindData.cFileName, entry, ARRAYSIZE(entry_info.FindData.cFileName));
	return ConvertEntry(&entry_info,p);
}

/*
 ftp> cd MA_5_3_0
 250 DW0-/MA_5_3_0/ is current directory
         ^^^^^^^^^^ folder Name!!!

 *            1         2         3         4         5
 *  012345678901234567890123456789012345678901234567890123456789
    DW0-/MA_5_3_0/ is current directory
*/
BOOL WINAPI idDirParceSkirdin(const FTPServerInfo* Server, LPCSTR Line, char *CurDir, size_t CurDirSize)
{
	char *Ptr;

	if(StrNCmp(Line+4,"DW0-/",5) != 0)
		return FALSE;

	StrCpy(CurDir, Line+4, (int)CurDirSize);
	Ptr=CurDir;

	while(!isspace(*Ptr))
		Ptr++;

	if(Ptr > CurDir+1)
		Ptr--;

	*Ptr=0;
	return TRUE;
}
