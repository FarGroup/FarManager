#include <all_far.h>
#pragma hdrstop

#include "p_Int.h"

//--- 4.06.2003 by Oleg Hohloff

// 0         1         2
// 0123456789012345678901234
// Tue May 31 06:22:00 1994

BOOL net_parse_pctcp_date_time( char *datestr, Time_t& decoded )
  {
     SYSTEMTIME st;
     GetSystemTime(&st);
     st.wMilliseconds = 0;

     CHECK( (datestr[3]!=' ' || datestr[7]!=' ' || datestr[10]!=' ' || datestr[19]!=' '), FALSE )
     CHECK( (datestr[13] != ':' || datestr[16] != ':'), FALSE )

     // �����
     datestr[7] = 0;
     st.wMonth = NET_MonthNo( datestr+4 );
     datestr[7] = ' ';
     CHECK( (st.wMonth > 12), FALSE )

     // ����
     if ( !NET_IS_DIGIT(datestr[8]) || !NET_IS_DIGIT(datestr[9]) ) return FALSE;
     st.wDay = ((datestr[8]-'0')*10) + (datestr[9]-'0');
     if (st.wDay==0 || st.wDay>31) return FALSE;

     // ���
     st.wYear = AtoI(datestr+20,MAX_WORD);
     if ( st.wYear == MAX_WORD ) return FALSE;

     // �६�
     st.wHour   = ((datestr[11]-'0')*10) + (datestr[12]-'0');
     st.wMinute = ((datestr[14]-'0')*10) + (datestr[15]-'0');
     st.wSecond = ((datestr[17]-'0')*10) + (datestr[18]-'0');

     //
     st.wDayOfWeek = 0;

     if ( !SystemTimeToFileTime( &st, decoded ) ) {
       Log(( "!time: %d-%d-%d %d:%d:%d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond ));
     }

 return TRUE;
}

/* PC/TCP ftpsrv.exe
 * looks like:
 *
 *            1         2         3         4         5         6
 *  0123456789012345678901234567890123456789012345678901234567890
 *       40774              IO.SYS   Tue May 31 06:22:00 1994
 *       38138           MSDOS.SYS   Tue May 31 06:22:00 1994
 *       54645         COMMAND.COM   Tue May 31 06:22:00 1994
 *  <dir>                     UTIL   Thu Feb 20 09:55:02 2003
 */
BOOL DECLSPEC idPRParcePCTCP( const PFTPServerInfo Server, PFTPFileInfo p, char *entry, int entry_len )
  {  NET_FileEntryInfo  entry_info;
     char              *e;

    CHECK( (entry_len<56 || entry[10]!=' ' || entry[30]!=' ' || entry[32]!=' '), FALSE )

    // ���ᨬ ����-�६�
    CHECK( (!net_parse_pctcp_date_time(entry+33, entry_info.date )), FALSE )

    // ���
    entry[30] = 0;
    e = SkipSpace(entry+11);
    CHECK( (e[0]==0), FALSE )
    StrCpy(entry_info.FindData.cFileName, e, sizeof(entry_info.FindData.cFileName));

    // ࠧ��� ��� �ਧ��� ��४���
    if( StrCmp(entry, "<dir> ",6,FALSE) == 0 ) {
      entry_info.FileType = NET_DIRECTORY;
    } else {
      e = SkipSpace(entry);
      entry_info.size = AtoI(e, (__int64)-1);
      CHECK( (entry_info.size == -1), FALSE )
    }

 return ConvertEntry( &entry_info,p );
}

BOOL DECLSPEC idDirParcePCTCP( const PFTPServerInfo Server, CONSTSTR Line, char *CurDir, size_t CurDirSize )
  {
    // �᫨ ��ப� ��稭����� � 'Current Working Directory is ',
    // � �����㥬 ⥪�� ��᫥ �⮩ ��ப�

    if (memcmp(Line, PCTCP_PWD_Title, PCTCP_PWD_TITLE_LEN)==0) {
      StrCpy(CurDir, Line+PCTCP_PWD_TITLE_LEN, CurDirSize);
      return TRUE;
    }

 return FALSE;
}
