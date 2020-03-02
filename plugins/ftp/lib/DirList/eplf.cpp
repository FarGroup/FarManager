#include <all_far.h>
#pragma hdrstop

#include "Int.h"

/*
 * EPLF: see http://pobox.com/~djb/proto/eplf.txt
 * "+i8388621.29609,m824255902,/,\tdev"
 * "+i8388621.44468,m839956783,r,s10376,\tRFCEPLF"
 *
 * A plus, a bunch of comma-separated facts, a tab,
 * and then the name.  Facts include m for mdtm (as
 * seconds since the Unix epoch), s for size, r for
 * (readable) file, and / for (listable) directory.
 */
BOOL WINAPI idPRParceEPLF(const FTPServerInfo* Server, FTPFileInfo* p, char *entry, int entry_len)
{
	char *tab;
	char *begin, *end;
	NET_FileEntryInfo entry_info;

	if(entry[0] != '+' ||
	        strchr(entry,' ') ||
	        (tab = strchr(entry, '\t')) == NULL)
		return FALSE;

	for(begin = &entry[1]; begin < tab; begin = end+1)
	{
		end = strchr(begin, ',');

		if(!end || end > tab)
			break;

		switch(*begin)
		{
			case 'm':
			{
				time_t tt = AtoI(begin+1,(time_t)0);

				if(!tt) { entry_info.date = 0; break; }

				struct tm *t = localtime(&tt);

				if(!t) { entry_info.date = 0; break; }

				SYSTEMTIME st;
				st.wYear       = t->tm_year+1900;
				st.wMonth      = t->tm_mon+1;
				st.wDayOfWeek  = 0;
				st.wDay        = t->tm_mday;
				st.wHour       = t->tm_hour;
				st.wMinute     = t->tm_min;
				st.wSecond     = t->tm_sec;
				st.wMilliseconds = 0;

				if(!SystemTimeToFileTime(&st, entry_info.date))
					entry_info.date = 0;
			}
			break;
			case 's':  entry_info.size = AtoI(begin+1,(__int64)-1);

				if(entry_info.size == (__int64)-1)
					return FALSE;

				break;
			case 'r':  entry_info.FileType = NET_FILE_TYPE; break;
			case '/':  entry_info.FileType = NET_DIRECTORY; break;
			default:
				break;
		}
	}

	StrCpy(entry_info.FindData.cFileName, tab+1, ARRAYSIZE(entry_info.FindData.cFileName));
	return ConvertEntry(&entry_info,p);
}
