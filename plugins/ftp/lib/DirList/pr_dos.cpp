#include <all_far.h>
#pragma hdrstop

#include "p_Int.h"

/* a dos date/time string looks like this
 * 04-06-95  02:03PM
 * 07-13-95  11:39AM
 */
BOOL net_parse_dos_date_time( CONSTSTR datestr, Time_t& decoded )
  {
    SYSTEMTIME st;
    GetSystemTime(&st);
    st.wMilliseconds = 0;

    CHECK( (datestr[2] != '-' || datestr[5] != '-' || datestr[8] != ' '), FALSE )
    CHECK( (datestr[12] != ':'), FALSE )

    if ( datestr[0] == ' ' )
      st.wMonth = (datestr[1]-'0');
     else
      st.wMonth = (datestr[0]-'0')*10 + (datestr[1]-'0');

    st.wDay = ((datestr[3]-'0')*10) + (datestr[4]-'0');

    st.wYear = ((datestr[6]-'0')*10) + (datestr[7]-'0');
    if ( st.wYear < 50 )
      st.wYear += 100;
    st.wYear += 1900;

    st.wHour   = ((datestr[10]-'0')*10) + (datestr[11]-'0');
    st.wMinute = ((datestr[13]-'0')*10) + (datestr[14]-'0');

    if(datestr[15] == 'P')
      st.wHour += 12;

    st.wDayOfWeek = 0;
    st.wSecond    = 0;

    if ( st.wHour >= 24 && st.wMinute )
      st.wHour -= 12;

    if ( !SystemTimeToFileTime( &st, decoded ) ) {
      Log(( "!time: %d-%d-%d %d:%d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute ));
    }
 return TRUE;
}

/* windows NT DOS dir syntax.
 * looks like:
 *            1         2         3         4         5
 *  012345678901234567890123456789012345678901234567890
 *  06-29-95  03:05PM       <DIR>          muntemp
 *  05-02-95  10:03AM               961590 naxp11e.zip
 *
 *  The date time directory indicator and FindData.cFileName
 *  are always in a fixed position.  The file
 *  size always ends at position 37.
 */
BOOL DECLSPEC idPRParceDos( const PFTPServerInfo Server, PFTPFileInfo p, char *entry, int entry_len )
  {  NET_FileEntryInfo  entry_info;
     char              *e;

     CHECK( (entry_len < 39 || entry[17] != ' '), FALSE )

     entry[17] = 0;
     CHECK( (!net_parse_dos_date_time(entry, entry_info.date )), FALSE )

     // <DIR> | digits
     e = SkipSpace(entry+18);

     if( StrCmp(e, "<DIR> ",5,FALSE) == 0 )
       entry_info.FileType = NET_DIRECTORY;
      else {
       SkipNSpace(e)[0] = 0;
       entry_info.size = AtoI( e,(__int64)-1 );
       CHECK( (entry_info.size == -1), FALSE )
     }

     StrCpy( entry_info.FindData.cFileName, entry+39, sizeof(entry_info.FindData.cFileName)  );

 return ConvertEntry( &entry_info,p );
}
