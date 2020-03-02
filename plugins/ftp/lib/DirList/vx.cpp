#include <all_far.h>
#pragma hdrstop

#include "Int.h"

/*          1         2         3         4         5         6         7
  01234567890123456789012345678901234567890123456789012345678901234567890123456789
  Aug-19-1994  12:00:00
  Aug-19-1994  12:00:00
  Aug-19-1994  12:00:00
  May-05-2003  14:20:44
  May-05-2003  14:20:58
*/
BOOL parse_vxdos_date_time( LPSTR& line, Time_t& decoded )
  {
    if ( !line[0] ||
         line[3] != '-' || line[6] != '-' ||
         line[11] != ' ' ||
         line[15] != ':' || line[18] != ':' )
      return FALSE;

    SYSTEMTIME st;
    GetSystemTime(&st);
    st.wMilliseconds = 0;

//mon
    st.wMonth = NET_MonthNo( line );
    if ( st.wMonth == MAX_WORD )
      return FALSE;
    line += 4;

//mday
    TwoDigits( line, st.wDay );
    if ( st.wDay == MAX_WORD )
      return FALSE;
    line += 3;

//year
    line[11] = 0;
    st.wYear = AtoI( line, MAX_WORD );
    if ( st.wYear == MAX_WORD )
      return FALSE;
    line += 6;

//Time
    TwoDigits( line+0, st.wHour );
    TwoDigits( line+3, st.wMinute );
    TwoDigits( line+6, st.wSecond );
    if ( st.wHour   == MAX_WORD ||
         st.wMinute == MAX_WORD ||
         st.wSecond == MAX_WORD )
      return FALSE;

    st.wDayOfWeek = 0;

 return SystemTimeToFileTime( &st, decoded );
}

/*          1         2         3         4         5         6         7
  01234567890123456789012345678901234567890123456789012345678901234567890123456789
    size          date       time       name
  --------       ------     ------    --------
     40774    Aug-19-1994  12:00:00   IO.SYS
     38138    Aug-19-1994  12:00:00   MSDOS.SYS
     54869    Aug-19-1994  12:00:00   COMMAND.COM
       512    May-05-2003  14:20:44   BOOT              <DIR>
       512    May-05-2003  14:20:58   SHELLS            <DIR>
        29    May-05-2003  14:24:14   AUTOEXEC.BAT
       915    May-05-2003  14:26:18   CONFIG.SYS
     29336    Aug-19-1994  12:00:00   FDISK.EXE
*/

static LPCSTR vx_skips[] = {
  "  size          date  ",
  "--------       ------  ",
  NULL };

BOOL WINAPI idPRParceVX_DOS( const FTPServerInfo* Server, FTPFileInfo* p, char *entry, int entry_len )
  {  NET_FileEntryInfo ei;

     if ( entry_len < 36 )
       return FALSE;

     if ( StartsWith( entry,vx_skips ) ) {
       p->FileType = NET_SKIP;
       return TRUE;
     }

     char *e;

//size
     entry = SkipSpace(entry);
     e     = SkipNSpace(entry);
     *e = 0;
     ei.size = AtoI( entry, (__int64)-1 );
     if ( ei.size == (__int64)-1 )
       return FALSE;

//time
     entry = SkipSpace( e+1 );
     e     = SkipNSpace(SkipSpace(SkipNSpace(entry)));
     if ( !parse_vxdos_date_time(entry,ei.date) )
       return FALSE;

//Name
     entry = SkipSpace( e );
     StrCpy( ei.FindData.cFileName, entry, ARRAYSIZE(ei.FindData.cFileName) );
     XP_StripLine( ei.FindData.cFileName );

     if ( (e=strstr( ei.FindData.cFileName,"<DIR>" )) != NULL ) {
       ei.FileType = NET_DIRECTORY;
       *e = '\0';
       XP_StripLine( ei.FindData.cFileName );
     }

 return ConvertEntry( &ei,p );
}
