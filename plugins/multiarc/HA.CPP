#include <windows.h>
#include <string.h>
#include <dos.h>
#include "plugin.hpp"
#include "fmt.hpp"

#if defined(__BORLANDC__)
  #pragma option -a1
#elif defined(__GNUC__) || (defined(__WATCOMC__) && (__WATCOMC__ < 1100)) || defined(__LCC__)
  #pragma pack(1)
  #if defined(__LCC__)
    #define _export __declspec(dllexport)
  #endif
#else
  #pragma pack(push,1)
  #if _MSC_VER
    #define _export
  #endif
#endif

#if _MSC_VER
struct dosdate_t {
   unsigned char day;       /* 1-31 */
   unsigned char month;     /* 1-12 */
   unsigned int  year;      /* 1980 - 2099 */
   unsigned char dayofweek; /* 0 - 6 (0=Sunday) */
};
struct time {
   unsigned char ti_min;      /* minutes */
   unsigned char ti_hour;     /* hours */
   unsigned char ti_hund;     /* hundredths of seconds */
   unsigned char ti_sec;      /* seconds */
};
#endif

void UnixToDos(long time, struct dosdate_t *d, struct time *t);

static HANDLE ArcHandle;
static DWORD NextPosition,FileSize;
static int timezone=0;

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
  d->day = time;
}


BOOL WINAPI _export IsArchive(const char *Name,const unsigned char *Data,int DataSize)
{
  if (DataSize<26 || Data[0]!='H' || Data[1]!='A' || Data[3]>32)
    return(FALSE);
  int Type=Data[4] & 0xf;
  if (Type>2 && Type<14 || Data[4]>0x2f)
    return(FALSE);
  return(TRUE);
}


BOOL WINAPI _export OpenArchive(const char *Name,int *Type)
{
  ArcHandle=CreateFile(Name,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,
                       NULL,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,NULL);
  if (ArcHandle==INVALID_HANDLE_VALUE)
    return(FALSE);

  *Type=0;

  FileSize=GetFileSize(ArcHandle,NULL);

  TIME_ZONE_INFORMATION tzi;
  if (GetTimeZoneInformation(&tzi)!=0xFFFFFFFF)
    timezone=-tzi.Bias*60;

  NextPosition=4;
  return(TRUE);
}


int WINAPI _export GetArcItem(struct PluginPanelItem *Item,struct ArcItemInfo *Info)
{
  struct HaHeader
  {
    BYTE Type;
    DWORD PackSize;
    DWORD UnpSize;
    DWORD CRC;
    DWORD FileTime;
  } Header;
  DWORD ReadSize;
  NextPosition=SetFilePointer(ArcHandle,NextPosition,NULL,FILE_BEGIN);
  if (NextPosition==0xFFFFFFFF)
    return(GETARC_READERROR);
  if (NextPosition>FileSize)
    return(GETARC_UNEXPEOF);
  if (!ReadFile(ArcHandle,&Header,sizeof(Header),&ReadSize,NULL))
    return(GETARC_READERROR);
  if (ReadSize==0)
    return(GETARC_EOF);
  char Path[3*NM],Name[NM];
  if (!ReadFile(ArcHandle,Path,sizeof(Path),&ReadSize,NULL) || ReadSize==0)
    return(GETARC_READERROR);
  Path[NM-1]=0;
  int PathLength=strlen(Path)+1;
  strncpy(Name,Path+PathLength,sizeof(Name)-1);
  Name[sizeof(Name)-1]=0;
  int Length=PathLength+strlen(Name)+1;
  DWORD PrevPosition=NextPosition;
  NextPosition+=sizeof(Header)+Length+Path[Length]+1+Header.PackSize;
  if (PrevPosition>=NextPosition)
    return(GETARC_BROKEN);
  char *EndSym=strrchr(Path,255);
  if (EndSym!=NULL)
    *EndSym=0;
  if (*Path)
    strcat(Path,"\\");
  strcat(Path,Name);
  for (int I=0;Path[I]!=0;I++)
    if ((unsigned char)Path[I]==0xff)
      Path[I]='\\';
  strncpy(Item->FindData.cFileName,Path,sizeof(Item->FindData.cFileName)-1);
  Item->FindData.cFileName[sizeof(Item->FindData.cFileName)-1]=0;
  Item->FindData.dwFileAttributes=(Header.Type & 0xf)==0xe ? FILE_ATTRIBUTE_DIRECTORY:0;
  Item->CRC32=Header.CRC;
  struct dosdate_t dt;
  struct time tm;
  UnixToDos(Header.FileTime,&dt,&tm);
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
  LocalFileTimeToFileTime(&lft,&Item->FindData.ftLastWriteTime);
  Item->FindData.nFileSizeLow=Header.UnpSize;
  Item->FindData.nFileSizeHigh=0;
  Item->PackSize=Header.PackSize;
  return(GETARC_SUCCESS);
}


BOOL WINAPI _export CloseArchive(struct ArcInfo *Info)
{
  return(CloseHandle(ArcHandle));
}

BOOL WINAPI _export GetFormatName(int Type,char *FormatName,char *DefaultExt)
{
  if (Type==0)
  {
    strcpy(FormatName,"HA");
    strcpy(DefaultExt,"ha");
    return(TRUE);
  }
  return(FALSE);
}


BOOL WINAPI _export GetDefaultCommands(int Type,int Command,char *Dest)
{
  if (Type==0)
  {
    static char *Commands[]={
    /*Extract               */"ha xay %%a %%FMQ",
    /*Extract without paths */"ha eay %%a %%FMQ",
    /*Test                  */"ha t %%a %%FMQ",
    /*Delete                */"ha d %%a %%FMQ",
    /*Comment archive       */"",
    /*Comment files         */"",
    /*Convert to SFX        */"",
    /*Lock archive          */"",
    /*Protect archive       */"",
    /*Recover archive       */"",
    /*Add files             */"ha as2{%%S} %%a %%FQ",
    /*Move files            */"ha asm2{%%S} %%a %%FQ",
    /*Add files and folders */"ha asr2{%%S} %%a %%FMQ",
    /*Move files and folders*/"ha asmr2{%%S} %%a %%FMQ",
    /*"All files" mask      */"*.*"
    };
    if (Command<sizeof(Commands)/sizeof(Commands[0]))
    {
      strcpy(Dest,Commands[Command]);
      return(TRUE);
    }
  }
  return(FALSE);
}
