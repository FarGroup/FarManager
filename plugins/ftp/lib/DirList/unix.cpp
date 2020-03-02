#include <all_far.h>
#pragma hdrstop

#include "Int.h"

#define MIN_DATETIME_STRING (3+4) //The minimal date-time string is "F  2000"

/*
 *--Full format

            1         2         3         4         5         6         7
  01234567890123456789012345678901234567890123456789012345678901234567890123456789
  Sep  1  1990   - start with ' '
  Sep 11 11:59
  Sep 11 01:59   - start with 0
  Sep 11  1:59   - start with ' '
  Dec 12 1989
  FCv 23 1990

 *--Short format:

            1         2         3         4         5         6         7
  01234567890123456789012345678901234567890123456789012345678901234567890123456789
  f 01:07   - time
  f 01:7    - minutes with one digit
  F 15:43
  f  2002   - only year

 *--Expanded format:

            1         2         3         4         5         6         7
  01234567890123456789012345678901234567890123456789012345678901234567890123456789
 *2005-06-20 14:22
 *2005-07-08 19:21
 *2004-10-14 14:14
 *2004-10-14 14:14
*/
BOOL net_convert_unix_date(LPSTR& datestr, Time_t& decoded)
{
	SYSTEMTIME st;
	GetSystemTime(&st);
	st.wMilliseconds = 0;
	st.wSecond       = 0;
	st.wDayOfWeek    = 0;
	char *bcol = datestr;         /* Column begin */
	char *ecol;                   /* Column end */
	//Expanded format (DDDD-)
	if(NET_IS_DIGIT(bcol[0]) && NET_IS_DIGIT(bcol[1]) && NET_IS_DIGIT(bcol[2]) && NET_IS_DIGIT(bcol[3]) &&
	        bcol[4] == '-')
	{
#define CVT( nm, start, end )              bcol[end] = 0;                       \
	st.nm = atoi(bcol+start);   \
	CHECK( (st.nm == MAX_WORD), FALSE )
		CVT(wYear,   0,  4)
		CVT(wMonth,  5,  7)
		CVT(wDay,    8, 10)
		CVT(wHour,  11, 13)
		CVT(wMinute,14, 16)
#undef CVT
		datestr = bcol + 17;
		return SystemTimeToFileTime(&st, decoded);
	}

	//Month+day or short format
	// (ecol must be set to char after decoded part)
	if(NET_TO_UPPER(bcol[0]) == 'F' &&
	        NET_IS_SPACE(bcol[1]))
	{
		//Short format - ignore month and day
		ecol = bcol + 2;
	}
	else
	{
		//Month
		if(NET_IS_DIGIT(bcol[0]) && NET_IS_DIGIT(bcol[1]) && NET_IS_SPACE(bcol[2]))
			st.wMonth = AtoI(bcol,MAX_WORD);
		else
			st.wMonth = NET_MonthNo(datestr);

		CHECK((st.wMonth == MAX_WORD), FALSE)
		bcol = SkipSpace(SkipNSpace(bcol));
		CHECK((*bcol == 0), FALSE)
		//Day
		ecol = SkipNSpace(bcol);

		if(*ecol != ' ')
			return FALSE;

		*ecol = 0;
		st.wDay = AtoI(bcol,MAX_WORD);
		*ecol = ' ';
		CHECK((st.wDay == MAX_WORD), FALSE)
	}

	//Year or time
	ecol = SkipSpace(ecol);
	bcol = ecol;

	if(bcol[2] != ':' && bcol[1] != ':')
	{
		//Four digits year
		ecol = SkipDigit(bcol);
		CHECK((ecol == bcol), FALSE)
		*ecol = 0;
		st.wYear = AtoI(bcol,MAX_WORD);
		ecol++;
		CHECK((st.wYear == MAX_WORD), FALSE)

		//Only first three digits of year with cut last digit
		if(st.wYear > 190 && st.wYear < 300)
		{
			st.wYear *= 10;
		}

		st.wSecond = 0;
		st.wMinute = 0;
		st.wHour   = 0;
	}
	else
	{
		// Time
		/* If the time is given as hh:mm, then the file is less than 1 year
		 *  old, but we might shift calandar year. This is avoided by checking
		 *  if the date parsed is future or not.
		 */
		if(bcol[1] == ':')
		{
			bcol--;
			*bcol = '0';
		}

		CHECK((!TwoDigits(bcol,st.wHour)), FALSE)
		//Time minutes may be specified by single digit
		//In this case next character is space
		CHECK((!NET_IS_DIGIT(bcol[3])), FALSE)

		if(bcol[4] == ' ')
		{
			st.wMinute = (0 + (bcol[3]-'0')) * 10;
			ecol = bcol + 5;
		}
		else
		{
			CHECK((!TwoDigits(bcol+3,st.wMinute)), FALSE)
			ecol = bcol + 6;
		}
	}

	datestr = ecol;

	if(SystemTimeToFileTime(&st, decoded))
		return TRUE;

	CHECK((st.wDay < 31), FALSE)
	st.wDay = 30;
	return SystemTimeToFileTime(&st, decoded);
}

/* nlinks
 *      group
 *               owner      size  date|time
 * ---------------------------------------------------------
 * 2    montulli eng          512 Nov  8 23:23 CVS
 * 1    montulli eng         2244 Nov  8 23:23 Imakefile
 * root montulli eng        14615 Nov  9 17:03 Makefile
 *      root     root           6,  10 Jun 17 17:45 con10
 *      root     root           6,   2 Jun 17 17:45 con2
*/
BOOL net_parse_ls_line(char *line, NET_FileEntryInfo* entry_info, BOOL nLinks)
{
	char *e;
	int   len;

//Skip nlinks
	if(nLinks)
		line = SkipNSpace(SkipSpace(line));

	line = SkipSpace(line);
	CHECK((*line == 0), FALSE)
//Owner
	e = SkipNSpace(line);
	CHECK((*e == 0), FALSE)
	len = Min((int)ARRAYSIZE(entry_info->FTPOwner)-1, (int)(e-line));
	StrCpy(entry_info->FTPOwner, line, len+1);
//Delimiter
	entry_info->FTPOwner[len++] = ':';
//Group
	line = SkipSpace(e);

	if(line[0] == '@' && line[1] == ' ')
		line = SkipSpace(line+1);

	e    = SkipNSpace(line);
	CHECK((*e == 0), FALSE)
	StrCpy(entry_info->FTPOwner+len,
	       line,
	       Min((int)ARRAYSIZE(entry_info->FTPOwner) - len, (int)(e-line+1)));
//Size
	line = SkipSpace(e);

	if(line[0] == '@' && line[1] == ' ')
		line = SkipSpace(line+1);

	e = SkipNSpace(line);
	//Check`n`Skip trailing ','
	e--;

	if(*e == ',')
	{
		line = SkipSpace(e+1);
		e = SkipNSpace(line);
	}
	else
		e++;

	CHECK((*e == 0), FALSE)
	*e = 0;
	entry_info->size = AtoI(line, (__int64)-1);
	*e = ' ';
	CHECK((entry_info->size == (__int64)-1), FALSE)
//Date
	line = SkipSpace(e);
	CHECK((!net_convert_unix_date(line,entry_info->date)), FALSE)

//File name
	if(*line)
	{
		FTPHostPlugin* host = FTP_Info->GetHostOpt();

		if(!host || !host->UseStartSpaces)
			line = SkipSpace(line);

		StrCpy(entry_info->FindData.cFileName, line, ARRAYSIZE(entry_info->FindData.cFileName));
	}
	else
	{
		entry_info->FindData.cFileName[0] = ' ';
		entry_info->FindData.cFileName[1] = 0;
	}

	return TRUE;
}

/*
 * ls -l listing:
 *
 * drwxr-xr-x    2 montulli eng          512 Nov  8 23:23 CVS
 * -rw-r--r--    1 montulli eng         2244 Nov  8 23:23 Imakefile
 * -rw-r--r--    1 montulli eng        14615 Nov  9 17:03 Makefile
 *
 * ls -s listing:
 *
 * 69792668804186112 drwxrw-rw- 1 root  root    0 Nov 15 14:01 .
 * 69792668804186112 drwxrw-rw- 1 root  root    0 Nov 15 14:01 ..
 * 69792668804186113 dr-x-w---- 1 root  root  512 Dec  1 02:16 Archives
 *
 * Full dates:
 *
 * -rw-r--r--  1 panfilov users 139264 2005-06-20 14:22 AddTransToStacks.doc
 * -rw-------  1 panfilov users    535 2005-07-08 19:21 .bash_history
 * -rw-r--r--  1 panfilov users    703 2004-10-14 14:14 .bash_profile
 * -rw-r--r--  1 panfilov users   1290 2004-10-14 14:14 .bashrc
*/
BOOL WINAPI idPRParceUnix(const FTPServerInfo* Server, FTPFileInfo* p, char *entry, int entry_len)
{
	NET_FileEntryInfo entry_info;
	BOOL              remove_size = FALSE;
	char              first;
	int               off = 0;
	CHECK((!is_unix_start(entry, entry_len,&off)), FALSE)
	entry += off;
	entry_len -= off;
	first = NET_TO_UPPER(*entry);
	StrCpy(entry_info.UnixMode, entry, ARRAYSIZE(entry_info.UnixMode));
	entry += 10;
	entry_len-=10;

//Skip ACL
	if(*entry == '+')
	{
		entry++;
		entry_len--;
	}

//D Dir
	if(first == 'D')
	{
		/* it's a directory */
		entry_info.FileType = NET_DIRECTORY;
		remove_size          = TRUE; /* size is not useful */
	}
	else

//C Char-device
//B Block-device
		if(first == 'C' || first == 'B')
		{
			SET_FLAG(entry_info.FindData.dwFileAttributes, FILE_ATTRIBUTE_SYSTEM);
		}
		else

//N ?
//S Socket
//P Pipe
			if(first == 'N' || first == 'S' || first == 'P')
			{
				SET_FLAG(entry_info.FindData.dwFileAttributes, FILE_ATTRIBUTE_HIDDEN);
			}
			else

//L Link
				if(first == 'L')
				{
					/* it's a symbolic link, does the user care about
					 * knowing if it is symbolic?  I think so since
					 * it might be a directory
					 */
					entry_info.FileType = NET_SYM_LINK;
					remove_size          = TRUE; /* size is not useful */
					int i;

					/* strip off " -> pathname" */
					for(i = entry_len - 1;
					        (i > 3) && (!NET_IS_SPACE(entry[i]) || (entry[i-1] != '>') || (entry[i-2] != '-') || (entry[i-3] != ' '));
					        i--) ; /* null body */

					if(i > 3)
					{
						entry[i-3] = '\0';
						StrCpy(entry_info.Link, entry + i + 1, ARRAYSIZE(entry_info.Link));
						i = static_cast<int>(strlen(entry_info.Link));

						if(i)
						{
							i--;

							if(entry_info.Link[i] == '/')
								entry_info.FileType = NET_SYM_LINK_TO_DIR;
							else
								;
						}
					}
				} /* link */

	if(!net_parse_ls_line(entry, &entry_info, TRUE) &&
	        !net_parse_ls_line(entry, &entry_info, FALSE))
		return FALSE;

	if(remove_size)
		entry_info.size = 0;

	return ConvertEntry(&entry_info,p);
}
