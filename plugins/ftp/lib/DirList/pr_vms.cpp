#include <all_far.h>
#pragma hdrstop

#include "p_Int.h"

/*          1         2         3         4         5         6         7
  01234567890123456789012345678901234567890123456789012345678901234567890123456789
  13-MAY-1996 22:55:36
   2-NOV-2000 13:55:58
   2-NOV-2000 13:56:25
  13-MAY-1996 22:56:37
*/
BOOL net_parse_vms_date_time(LPSTR& line, Time_t& decoded)
{
	SYSTEMTIME st;
	GetSystemTime(&st);
	st.wMilliseconds = 0;

	if(line[2] != '-' || line[6] != '-')
		return FALSE;

//mday
	if(line[0] == ' ')
		st.wDay = (line[1]-'0');
	else
		st.wDay = (line[0]-'0')*10 + (line[1]-'0');

//mon
	st.wMonth = NET_MonthNo(line+3);

	if(st.wMonth == MAX_WORD)
		return FALSE;

//year
	if(line[9] == ' ')
	{
		//Two digits
		st.wYear = ((line[6]-'0')*10) + (line[7]-'0');

		if(st.wYear < 50)
			st.wYear += 100;

		st.wYear += 1900;
		line += 10;
	}
	else
	{
		//Four digits
		if(line[11] != ' ')
			return FALSE;

		line[11] = 0;
		st.wYear = AtoI(line+7, MAX_WORD);

		if(st.wYear == MAX_WORD)
			return FALSE;

		line += 12;
	}

//Time
	st.wDayOfWeek = 0;
	st.wSecond    = 0;

	if(line[2] == ':')
	{
		//Hour
		st.wHour   = ((line[0]-'0')*10) + (line[1]-'0');
		//Min
		st.wMinute = ((line[3]-'0')*10) + (line[4]-'0');
		line += 5;

		//sec
		if(*line == ':')
		{
			st.wSecond = ((line[1]-'0')*10) + (line[2]-'0');
			line += 3;
		}
	}

	return SystemTimeToFileTime(&st, decoded);
}

/*          1         2         3         4         5         6         7
  01234567890123456789012345678901234567890123456789012345678901234567890123456789
  ANKETA.TXT;1             4/6          13-MAY-1996 22:55:36  [10545,1]              (RWED,RWED,RE,RE)
  EA1995.DIR;1             1/3           2-NOV-2000 13:55:58  [10545,1]              (RWED,RWED,RE,RE)
  EA1996.DIR;1             1/3           2-NOV-2000 13:56:25  [10545,1]              (RWED,RWED,RE,RE)
  EA_INET.TXT;1            1/6          13-MAY-1996 22:56:37  [10545,1]              (RWED,RWED,RE,RE)
*/

BOOL net_parse_vms_dir_entry(char *line, NET_FileEntryInfo* entry_info)
{
	char *e,*tmp;

	if((e=strchr(line, ';')) == NULL)
		return FALSE;

//Filename
	StrCpy(entry_info->FindData.cFileName, line, (int)(e-line+1));
	line = SkipSpace(SkipNSpace(e));
//Size
	e = SkipNSpace(line);

	if(!e[0]) return FALSE;

	*e = 0;
	tmp = e+1;

	if((e=strchr(line,'/')) != NULL)
	{
		*e = 0;
		entry_info->size = AtoI(line,-1);

		if(entry_info->size == -1)
			return FALSE;

		int ialloc = AtoI(e+1, -1);

		if(ialloc == -1)
			return FALSE;

		/* Check if used is in blocks or bytes */
		if(entry_info->size <= ialloc)
			entry_info->size *= 512;
	}
	else
	{
		entry_info->size = AtoI(line, -1);

		if(entry_info->size == -1)
			return FALSE;
	}

	line = SkipSpace(tmp);
//Date\time
	e = strchr(line,'-');

	if(!e ||
	        !NET_IS_DIGIT(*(e-1)) ||
	        !NET_IS_ALPHA(*(e+1)))
		return FALSE;

	line = e-2;

	if(!net_parse_vms_date_time(line, entry_info->date))
		return FALSE;

	line = SkipSpace(line);

//Owner
	if(*line == '[')
	{
		line++;
		e = SkipNX(line,']');
		StrCpy(entry_info->FTPOwner, line, Min((int)sizeof(entry_info->FTPOwner), (int)(e-line+1)));
		line = SkipSpace(e+1);
	}

//Attr
	int len = strLen(entry_info->FindData.cFileName);

	if(len > 4)
	{
		e = entry_info->FindData.cFileName + len - 4;

		if(StrCmp(e, ".DIR") == 0)
		{
			*e = 0;
			entry_info->FileType = NET_DIRECTORY;
		}
	}

	//??

//Lowercase name
	for(e = entry_info->FindData.cFileName; *e; e++)
		*e = NET_TO_LOWER(*e);

	return TRUE;
}

BOOL WINAPI idPRParceVMS(const FTPServerInfo* Server, FTPFileInfo* p, char *entry, int entry_len)
{
	NET_FileEntryInfo entry_info;

	if(!net_parse_vms_dir_entry(entry, &entry_info))
		return FALSE;

	/** Trim off VMS directory extensions **/
	int len = strLen(entry_info.FindData.cFileName);

	if((len > 4) && StrCmp(&entry_info.FindData.cFileName[len-4], ".dir") == 0)
	{
		entry_info.FindData.cFileName[len-4] = '\0';
		entry_info.FileType = NET_DIRECTORY;
	}

	return ConvertEntry(&entry_info,p);
}

BOOL WINAPI idDirPRParceVMS(const FTPServerInfo* Server, LPCSTR Line, char *CurDir, size_t CurDirSize)
{
	return FALSE;
}
