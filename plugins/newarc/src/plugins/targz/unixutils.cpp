#include <Rtl.Base.h>
#include <FarPluginBase.hpp>
#include <stdlib.h>
#include "unixutils.hpp"

static int timezone=0; //FIXME

void UnixToDos(long time, struct dosdate_t *d, struct time *t)
{
  static char Days[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
  time -= 24L * 60L * 60L * 3652L + timezone;
  t->ti_hund = 0;
  t->ti_sec = time % 60;
  time /= 60;
  t->ti_min = time % 60;
  time /= 60;
  d->year = 1980 + (int)((time / (1461L * 24L)) << 2);
  time %= 1461L * 24L;
  if (time >= 366 * 24)
  {
    time -= 366 * 24;
    d->year++;
    d->year += (int)(time / (365 * 24));
    time %= 365 * 24;
  }
  t->ti_hour = time % 24;
  time /= 24;
  time++;
  if ((d->year & 3) == 0)
  {
    if (time > 60)
      time--;
    else
      if (time == 60)
      {
        d->month = 2;
        d->day = 29;
        return;
      }
  }
  for (d->month = 0; Days[d->month] < time; d->month++)
    time -= Days[d->month];
  d->month++;
  d->day = (unsigned char) time;
}

void UnixTimeToFileTime(DWORD UnixTime,FILETIME *FileTime)
{
  struct dosdate_t dt;
  struct time tm;
  UnixToDos(UnixTime,&dt,&tm);
  SYSTEMTIME st;
  st.wYear=dt.year;
  st.wMonth=dt.month;
  st.wDay=dt.day;
  st.wHour=tm.ti_hour;
  st.wMinute=tm.ti_min;
  st.wSecond=tm.ti_sec;
  st.wMilliseconds=tm.ti_hund*10;
  FILETIME lft;
  SystemTimeToFileTime(&st,&lft);
  LocalFileTimeToFileTime(&lft,FileTime);
}

char *AdjustTARFileName(char *FileName)
{
  char *EndPos = FileName;
  while( *EndPos )
  {
    if( *EndPos == '/' )
      *EndPos = '\\';
    EndPos++;
  }
  return FileName;
}

DWORD GetOctal(const char *Str)
{
  char *endptr;
  return(strtoul(Str,&endptr,8));
}

DWORD GetHex(const char *Str)
{
  char *endptr;
  return(strtoul(Str,&endptr,16));
}
