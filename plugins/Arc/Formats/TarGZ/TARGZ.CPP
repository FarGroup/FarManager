#include <windows.h>
#include <string.h>
#include <dos.h>
#include "..\\fmt.hpp"
#include "d:\lang\bc5\far\plugin.hpp"

void UnixToDos(long time, struct date *d, struct time *t);
int IsTarHeader(const unsigned char *Data,int DataSize);
DWORD GetOctal(char *Str);

enum {TAR_FORMAT,GZ_FORMAT,Z_FORMAT,BZ_FORMAT};

HANDLE ArcHandle;
DWORD NextPosition,FileSize;
int timezone=0;
int ArcType;
char ZipName[NM];

BOOL WINAPI _export IsArchive(char *Name,const unsigned char *Data,int DataSize)
{
  if (IsTarHeader(Data,DataSize))
  {
    ArcType=TAR_FORMAT;
    return(TRUE);
  }
  if (DataSize<2)
    return(FALSE);
  if (Data[0]==0x1f && Data[1]==0x8b)
    ArcType=GZ_FORMAT;
  else
    if (Data[0]==0x1f && Data[1]==0x9d)
      ArcType=Z_FORMAT;
    else
      if (Data[0]=='B' && Data[1]=='Z')
        ArcType=BZ_FORMAT;
      else
        return(FALSE);
  char *NamePtr=strrchr(Name,'\\');
  NamePtr=(NamePtr==NULL) ? Name:NamePtr+1;
  strcpy(ZipName,NamePtr);
  char *Dot=strrchr(NamePtr,'.');
  if (Dot!=NULL)
  {
    Dot++;
    if (stricmp(Dot,"tgz")==0 || stricmp(Dot,"taz")==0)
      strcpy(&ZipName[Dot-NamePtr],"tar");
    else
      ZipName[Dot-NamePtr-1]=0;
  }
  return(TRUE);
}


BOOL WINAPI _export OpenArchive(char *Name,int *Type)
{
  ArcHandle=CreateFile(Name,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,
                       NULL,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,NULL);
  if (ArcHandle==INVALID_HANDLE_VALUE)
    return(FALSE);
  *Type=ArcType;

  FileSize=GetFileSize(ArcHandle,NULL);

  TIME_ZONE_INFORMATION tzi;
  if (GetTimeZoneInformation(&tzi)!=0xFFFFFFFF)
    timezone=tzi.Bias*60;

  NextPosition=0;
  return(TRUE);
}


int WINAPI _export GetArcItem(struct PluginPanelItem *Item,struct ArcItemInfo *Info)
{
  DWORD ReadSize;
  if (ArcType!=TAR_FORMAT)
    if (*ZipName)
    {
      if (ArcType==BZ_FORMAT)
      {
        Item->PackSize=Item->FindData.nFileSizeLow=FileSize;
        strcpy(Item->FindData.cFileName,ZipName);
        *ZipName=0;
        return(GETARC_SUCCESS);
      }
      struct GZHeader
      {
        BYTE Mark[2];
        BYTE Method;
        BYTE Flags;
        DWORD FileTime;
        BYTE ExtraFlags;
        BYTE HostOS;
      } Header;
      if (!ReadFile(ArcHandle,&Header,sizeof(Header),&ReadSize,NULL))
        return(GETARC_READERROR);
      Item->PackSize=FileSize;
      if (ArcType==Z_FORMAT)
      {
        strcpy(Item->FindData.cFileName,ZipName);
        *ZipName=0;
        Item->FindData.nFileSizeLow=FileSize;
        return(GETARC_SUCCESS);
      }
      if (Header.Flags & 2)
        SetFilePointer(ArcHandle,2,NULL,FILE_CURRENT);
      if (Header.Flags & 4)
      {
        DWORD ExtraLength;
        if (!ReadFile(ArcHandle,&ExtraLength,sizeof(ExtraLength),&ReadSize,NULL))
          return(GETARC_READERROR);
        SetFilePointer(ArcHandle,ExtraLength,NULL,FILE_CURRENT);
      }
      if (Header.Flags & 8)
        if (!ReadFile(ArcHandle,Item->FindData.cFileName,sizeof(Item->FindData.cFileName),&ReadSize,NULL))
          return(GETARC_READERROR);
      if (*Item->FindData.cFileName==0)
        strcpy(Item->FindData.cFileName,ZipName);
      *ZipName=0;
      struct date dt;
      struct time tm;
      UnixToDos(Header.FileTime,&dt,&tm);
      SYSTEMTIME st;
      st.wYear=dt.da_year;
      st.wMonth=dt.da_mon;
      st.wDay=dt.da_day;
      st.wHour=tm.ti_hour;
      st.wMinute=tm.ti_min;
      st.wSecond=tm.ti_sec;
      st.wMilliseconds=tm.ti_hund*10;
      FILETIME lft;
      SystemTimeToFileTime(&st,&lft);
      LocalFileTimeToFileTime(&lft,&Item->FindData.ftLastWriteTime);
      Info->Comment=(Header.Flags & 16)!=0;
      Info->Encrypted=(Header.Flags & 32)!=0;
      SetFilePointer(ArcHandle,-4,NULL,FILE_END);
      if (!ReadFile(ArcHandle,&Item->FindData.nFileSizeLow,sizeof(Item->FindData.nFileSizeLow),&ReadSize,NULL))
        return(GETARC_READERROR);
      return(GETARC_SUCCESS);
    }
    else
      return(GETARC_EOF);
  struct TarHeader
  {
    char name[100];
    char mode[8];
    char userid[8];
    char groupid[8];
    char filesize[12];
    char timestamp[12];
    char checksum[8];
    char linkflag;
    char linkname[100];
    char unused_chars[255];
  } Header;
  NextPosition=SetFilePointer(ArcHandle,NextPosition,NULL,FILE_BEGIN);
  if (NextPosition==0xFFFFFFFF)
    return(GETARC_READERROR);
  if (NextPosition>FileSize)
    return(GETARC_UNEXPEOF);
  if (!ReadFile(ArcHandle,&Header,sizeof(Header),&ReadSize,NULL))
    return(GETARC_READERROR);
  if (ReadSize==0 || *Header.name==0)
    return(GETARC_EOF);
  strncpy(Item->FindData.cFileName,Header.name,sizeof(Item->FindData.cFileName));
  Item->FindData.dwFileAttributes=(GetOctal(Header.mode) & 0x4000) || ((Header.linkflag-'0') & 4) ? FILE_ATTRIBUTE_DIRECTORY:0;
  struct date dt;
  struct time tm;
  UnixToDos(GetOctal(Header.timestamp),&dt,&tm);
  SYSTEMTIME st;
  st.wYear=dt.da_year;
  st.wMonth=dt.da_mon;
  st.wDay=dt.da_day;
  st.wHour=tm.ti_hour;
  st.wMinute=tm.ti_min;
  st.wSecond=tm.ti_sec;
  st.wMilliseconds=tm.ti_hund*10;
  FILETIME lft;
  SystemTimeToFileTime(&st,&lft);
  LocalFileTimeToFileTime(&lft,&Item->FindData.ftLastWriteTime);
  Item->PackSize=Item->FindData.nFileSizeLow=GetOctal(Header.filesize);
  Item->FindData.nFileSizeHigh=0;
  DWORD PrevPosition=NextPosition;
  NextPosition+=sizeof(Header)+Item->PackSize;
  if (NextPosition & 511)
    NextPosition+=512-(NextPosition & 511);
  if (PrevPosition>=NextPosition)
    return(GETARC_BROKEN);
  return(GETARC_SUCCESS);
}


BOOL WINAPI _export CloseArchive(struct ArcInfo *Info)
{
  return(CloseHandle(ArcHandle));
}


BOOL WINAPI _export GetFormatName(int Type,char *FormatName,char *DefaultExt)
{
  switch(Type)
  {
    case TAR_FORMAT:
      strcpy(FormatName,"TAR");
      strcpy(DefaultExt,"TAR");
      return(TRUE);
    case GZ_FORMAT:
      strcpy(FormatName,"GZip");
      strcpy(DefaultExt,"GZ");
      return(TRUE);
    case Z_FORMAT:
      strcpy(FormatName,"Z(Unix)");
      strcpy(DefaultExt,"Z");
      return(TRUE);
    case BZ_FORMAT:
      strcpy(FormatName,"BZip");
      strcpy(DefaultExt,"BZ2");
      return(TRUE);
  }
  return(FALSE);
}


BOOL WINAPI _export GetDefaultCommands(int Type,int Command,char *Dest)
{
  switch(Type)
  {
    case TAR_FORMAT:
    {
      static char *Commands[]={
        "tar --force-local -xf %%A %%FSQ32768",
        "%comspec% /c tar --force-local -O -xf %%A %%fSQ > %%fWQ",
        "",
        "tar --delete --force-local -f %%A %%FSQ32768",
        "",
        "",
        "",
        "",
        "",
        "",
        "tar --force-local -rf %%A %%FSQ32768",
        "tar --force-local --remove-files -rf %%A %%FSQ32768",
        "tar --force-local -rf %%A %%FSQ32768",
        "tar --force-local --remove-files -rf %%A %%FSQ32768",
        "*"
      };
      if (Command<sizeof(Commands)/sizeof(Commands[0]))
      {
        strcpy(Dest,Commands[Command]);
        return(TRUE);
      }
      break;
    }
    case GZ_FORMAT:
    {
      static char *Commands[]={
        "%COMSPEC% /c gzip -cd %%A >%%fQ",
        "%COMSPEC% /c gzip -cd %%A >%%fQ",
        "gzip -t %%A",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "%COMSPEC% /c gzip -c %%fQ>%%A",
        "gzip %%fQ",
        "%COMSPEC% /c gzip -c %%fQ>%%A",
        "gzip %%fQ",
        "*"
      };
      if (Command<sizeof(Commands)/sizeof(Commands[0]))
      {
        strcpy(Dest,Commands[Command]);
        return(TRUE);
      }
      break;
    }
    case Z_FORMAT:
    {
      static char *Commands[]={
        "%COMSPEC% /c gzip -cd %%A >%%fQ",
        "%COMSPEC% /c gzip -cd %%A >%%fQ",
        "gzip -t %%A",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "*"
      };
      if (Command<sizeof(Commands)/sizeof(Commands[0]))
      {
        strcpy(Dest,Commands[Command]);
        return(TRUE);
      }
      break;
    }
    case BZ_FORMAT:
    {
      static char *Commands[]={
        "%COMSPEC% /c bzip2 -cd %%A >%%fQ",
        "%COMSPEC% /c bzip2 -cd %%A >%%fQ",
        "%COMSPEC% /c bzip2 -cd %%A >nul",
        "",
        "",
        "",
        "",
        "",
        "",
        "",
        "%COMSPEC% /c bzip2 -c %%fQ >%%A",
        "bzip2 %%fQ",
        "%COMSPEC% /c bzip2 -c %%fQ >%%A",
        "bzip2 %%fQ",
        "*"
      };
      if (Command<sizeof(Commands)/sizeof(Commands[0]))
      {
        strcpy(Dest,Commands[Command]);
        return(TRUE);
      }
      break;
    }
  }
  return(FALSE);
}


void UnixToDos(long time, struct date *d, struct time *t)
{
  static char Days[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
  time -= 24L * 60L * 60L * 3652L + timezone;
  t->ti_hund = 0;
  t->ti_sec = time % 60;
  time /= 60;
  t->ti_min = time % 60;
  time /= 60;
  d->da_year = 1980 + (int)((time / (1461L * 24L)) << 2);
  time %= 1461L * 24L;
  if (time >= 366 * 24)
  {
    time -= 366 * 24;
    d->da_year++;
    d->da_year += (int)(time / (365 * 24));
    time %= 365 * 24;
  }
  t->ti_hour = time % 24;
  time /= 24;
  time++;
  if ((d->da_year & 3) == 0)
  {
    if (time > 60)
      time--;
    else
      if (time == 60)
      {
        d->da_mon = 2;
        d->da_day = 29;
        return;
      }
  }
  for (d->da_mon = 0; Days[d->da_mon] < time; d->da_mon++)
    time -= Days[d->da_mon];
  d->da_mon++;
  d->da_day = time;
}


int IsTarHeader(const unsigned char *Data,int DataSize)
{
  struct TarHeader
  {
    char name[100];
    char mode[8];
    char userid[8];
    char groupid[8];
    char filesize[12];
    char timestamp[12];
    char checksum[8];
    char linkflag;
    char linkname[100];
    char unused_chars[255];
  } *Header;
  if (DataSize<sizeof(TarHeader))
    return(FALSE);
  Header=(struct TarHeader *)Data;
  for (int I=0;Header->name[I];I++)
    if (I==sizeof(Header->name) || Header->name[I]<' ')
      return(FALSE);
  for (int I=0;I<&Header->linkflag-Header->mode;I++)
    if (Header->mode[I]>'7' || Header->mode[I]<'0' && Header->mode[I]!=0 &&
        Header->mode[I]!=' ')
      return(FALSE);
  DWORD Seconds=GetOctal(Header->timestamp);
  if (Seconds<300000000 || Seconds>1500000000)
    return(FALSE);
  return(TRUE);
}


DWORD GetOctal(char *Str)
{
  char *endptr;
  return(strtoul(Str,&endptr,8));
}

