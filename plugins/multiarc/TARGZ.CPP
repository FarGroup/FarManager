/*
  TARGZ.CPP

  Second-level plugin module for FAR Manager 1.70 and MultiArc plugin

  Copyright (c) 1996-2000 Eugene Roshal
  Copyrigth (c) 2000-2002 FAR group
*/
/* Revision: 1.15 27.04.2002 $ */

//#define USE_TAR_H

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

#ifdef _MSC_VER
#pragma comment(linker, "-subsystem:console")
#pragma comment(linker, "/ignore:4078")
#pragma comment(linker, "-merge:.rdata=.text")
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

#if defined(USE_TAR_H)
#include "tar.h"

#else

struct posix_header
{                       /* byte offset */
  char name[100];       /*   0 */
  char mode[8];         /* 100 */
  char userid[8];       /* 108 */
  char groupid[8];      /* 116 */
  char size[12];        /* 124 */
  char mtime[12];       /* 136 */
  char chksum[8];       /* 148 */
  char typeflag;        /* 156 */
  char linkname[100];   /* 157 */
  char magic[6];        /* 257 */
  char version[2];      /* 263 */
  char uname[32];       /* 265 */
  char gname[32];       /* 297 */
  char devmajor[8];     /* 329 */
  char devminor[8];     /* 337 */
  char prefix[155];     /* 345 */
                        /* 500 */
};

#define TMAGIC   "ustar"	/* ustar and a null */
#define TMAGLEN  6

#define BLOCKSIZE 512
typedef union block {
  char  buffer[BLOCKSIZE];
  struct posix_header header;
} TARHeader;

/* Identifies the *next* file on the tape as having a long linkname.  */
#define GNUTYPE_LONGLINK 'K'

/* Identifies the *next* file on the tape as having a long name.  */
#define GNUTYPE_LONGNAME 'L'

/* Values used in typeflag field.  */
#define REGTYPE   '0'    /* regular file */
#define AREGTYPE '\0'    /* regular file */
#define LNKTYPE  '1'    /* link */
#define SYMTYPE  '2'    /* reserved */
#define CHRTYPE  '3'    /* character special */
#define BLKTYPE  '4'    /* block special */
#define DIRTYPE  '5'    /* directory */
#define FIFOTYPE '6'    /* FIFO special */
#define CONTTYPE '7'    /* reserved */

#endif

void UnixToDos(long time, struct dosdate_t *d, struct time *t);
int IsTarHeader(const unsigned char *Data,int DataSize);
DWORD GetOctal(const char *Str);
int GetArcItemGZIP(struct PluginPanelItem *Item,struct ArcItemInfo *Info);
int GetArcItemTAR(struct PluginPanelItem *Item,struct ArcItemInfo *Info);
void UnixTimeToFileTime(DWORD UnixTime,FILETIME *LocalFileTime);
char *AdjustTARFileName(char *FileName);

enum {TAR_FORMAT,GZ_FORMAT,Z_FORMAT,BZ_FORMAT};

HANDLE ArcHandle;
DWORD NextPosition,SFXSize,FileSize;
int timezone=0;
int ArcType;
char ZipName[NM];

BOOL WINAPI _export IsArchive(const char *Name,const unsigned char *Data,int DataSize)
{
  SFXSize=0;
  if (IsTarHeader(Data,DataSize))
  {
    ArcType=TAR_FORMAT;
    return(TRUE);
  }

  if (DataSize<2)
    return(FALSE);

  if (Data[0]==0x1f && Data[1]==0x8b)
  {
    ArcType=GZ_FORMAT;
  }
  else if (Data[0]==0x1f && Data[1]==0x9d)
    ArcType=Z_FORMAT;
  else if (Data[0]=='B' && Data[1]=='Z')
    ArcType=BZ_FORMAT;
  else
    return(FALSE);

  const char *NamePtr=(const char *)strrchr((char*)Name,'\\');
  NamePtr=(NamePtr==NULL) ? Name:NamePtr+1;
  strcpy(ZipName,NamePtr);
  const char *Dot=(const char *)strrchr((char*)NamePtr,'.');

  if (Dot!=NULL)
  {
    Dot++;
    if (strcmpi(Dot,"tgz")==0 || strcmpi(Dot,"taz")==0)
      strcpy(&ZipName[Dot-NamePtr],"tar");
    else
      ZipName[Dot-NamePtr-1]=0;
  }

  return(TRUE);
}


BOOL WINAPI _export OpenArchive(const char *Name,int *Type)
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

DWORD WINAPI _export GetSFXPos(void)
{
  return SFXSize;
}

int WINAPI _export GetArcItem(struct PluginPanelItem *Item,struct ArcItemInfo *Info)
{
  if (ArcType!=TAR_FORMAT)
  {
    if (*ZipName)
    {
      if (ArcType==BZ_FORMAT)
      {
        Item->PackSize=Item->FindData.nFileSizeLow=FileSize;
        strcpy(Item->FindData.cFileName,ZipName);
        *ZipName=0;
        return(GETARC_SUCCESS);
      }
      return GetArcItemGZIP(Item,Info);
    }
    else
      return(GETARC_EOF);
  }
  return GetArcItemTAR(Item,Info);
}


int GetArcItemGZIP(struct PluginPanelItem *Item,struct ArcItemInfo *Info)
{
  struct GZHeader
  {
    BYTE Mark[2];
    BYTE Method;
    BYTE Flags;
    DWORD FileTime;
    BYTE ExtraFlags;
    BYTE HostOS;
  } Header;
  DWORD ReadSize;
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
  UnixTimeToFileTime(Header.FileTime,&Item->FindData.ftLastWriteTime);
  Info->Comment=(Header.Flags & 16)!=0;
  Info->Encrypted=(Header.Flags & 32)!=0;
  SetFilePointer(ArcHandle,-4,NULL,FILE_END);
  if (!ReadFile(ArcHandle,&Item->FindData.nFileSizeLow,sizeof(Item->FindData.nFileSizeLow),&ReadSize,NULL))
    return(GETARC_READERROR);
  return(GETARC_SUCCESS);
}


int GetArcItemTAR(struct PluginPanelItem *Item,struct ArcItemInfo *Info)
{
  TARHeader TAR_hdr;
  DWORD ReadSize;
  BOOL SkipItem=FALSE;
  char *LongName = NULL;
  do
  {
    NextPosition=SetFilePointer(ArcHandle,NextPosition,NULL,FILE_BEGIN);

    if (NextPosition==0xFFFFFFFF)
      return(GETARC_READERROR);

    if (NextPosition>FileSize)
      return(GETARC_UNEXPEOF);

    if (!ReadFile(ArcHandle,&TAR_hdr,sizeof(TAR_hdr),&ReadSize,NULL))
      return(GETARC_READERROR);

    if (ReadSize==0 || *TAR_hdr.header.name==0)
      return(GETARC_EOF);

    if (TAR_hdr.header.typeflag == GNUTYPE_LONGLINK || TAR_hdr.header.typeflag == GNUTYPE_LONGNAME)
      SkipItem=TRUE;
    else
    {
      SkipItem=FALSE;
      char *EndPos;
      if (LongName != NULL)
        EndPos = AdjustTARFileName (LongName);
      else
        EndPos = AdjustTARFileName (TAR_hdr.header.name);
      strncpy(Item->FindData.cFileName,EndPos,sizeof(Item->FindData.cFileName)-1);
      Item->FindData.cFileName[sizeof(Item->FindData.cFileName)-1]=0;
      Item->FindData.dwFileAttributes=(GetOctal(TAR_hdr.header.mode) & 0x4000) || ((TAR_hdr.header.typeflag-'0') & 4) ? FILE_ATTRIBUTE_DIRECTORY:0;
      UnixTimeToFileTime(GetOctal(TAR_hdr.header.mtime),&Item->FindData.ftLastWriteTime);
      Item->FindData.nFileSizeHigh=0;
    }
    Item->PackSize=Item->FindData.nFileSizeLow=GetOctal(TAR_hdr.header.size);

    DWORD PrevPosition=NextPosition;
    // for LNKTYPE - only sizeof(TAR_hdr)
    NextPosition+=sizeof(TAR_hdr)+(TAR_hdr.header.typeflag == LNKTYPE?0:Item->PackSize);

    if (NextPosition & 511)
      NextPosition+=512-(NextPosition & 511);

    if (PrevPosition>=NextPosition)
      return(GETARC_BROKEN);

    if (TAR_hdr.header.typeflag == GNUTYPE_LONGNAME)
    {
      SetFilePointer (ArcHandle,PrevPosition+sizeof(TAR_hdr),NULL,FILE_BEGIN);
      // we can't have two LONGNAME records in a row without a file between them
      if (LongName != NULL)
        return GETARC_BROKEN;
      LongName = (char *)HeapAlloc(GetProcessHeap(), 0, Item->PackSize);
      DWORD BytesRead;
      ReadFile(ArcHandle,LongName,Item->PackSize,&BytesRead,NULL);
      if (BytesRead != Item->PackSize)
      {
        HeapFree (GetProcessHeap(),0,LongName);
        return GETARC_BROKEN;
      }
    }
  } while (SkipItem);

  if (LongName)
    HeapFree (GetProcessHeap(), 0, LongName);
  return(GETARC_SUCCESS);
}


BOOL WINAPI _export CloseArchive(struct ArcInfo *Info)
{
  return(CloseHandle(ArcHandle));
}


BOOL WINAPI _export GetFormatName(int Type,char *FormatName,char *DefaultExt)
{
  static const char * const FmtAndExt[4][2]={
    {"TAR","tar"},
    {"GZip","gz"},
    {"Z(Unix)","z"},
    {"BZip","bz2"},
  };
  switch(Type)
  {
    case TAR_FORMAT:
    case GZ_FORMAT:
    case Z_FORMAT:
    case BZ_FORMAT:
      strcpy(FormatName,FmtAndExt[Type][0]);
      strcpy(DefaultExt,FmtAndExt[Type][1]);
      return(TRUE);
  }
  return(FALSE);
}


BOOL WINAPI _export GetDefaultCommands(int Type,int Command,char *Dest)
{
   static const char * Commands[4][15]=
   {
     { // TAR_FORMAT
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
     },

     { // GZ_FORMAT
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
       "%COMSPEC% /c gzip -c %%fQ >%%A",
       "gzip %%fQ",
       "%COMSPEC% /c gzip -c %%fQ >%%A",
       "gzip %%fQ",
       "*"
     },

     { // Z_FORMAT
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
     },

     { // BZ_FORMAT
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
     },
   };
   if (Type >= TAR_FORMAT && Type <= BZ_FORMAT && Command < sizeof(Commands[Type])/sizeof(Commands[Type][0]))
   {
     strcpy(Dest,Commands[Type][Command]);
     return(TRUE);
   }
  return(FALSE);
}


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


int IsTarHeader(const BYTE *Data,int DataSize)
{
  int I;
  struct posix_header *Header;
  if (DataSize<sizeof(struct posix_header))
    return(FALSE);

  Header=(struct posix_header *)Data;

  if (!strncmp (Header->magic, TMAGIC, TMAGLEN - 1))
  {
    /* If we have what looks like a real ustar archive, we must
    check the version number.  We only understand version 00.  */
    if (Header->magic[TMAGLEN] == '\0' && (Header->version[0] != '0' || Header->version[1] != '0'))
      return (FALSE);
  }

  for (I=0;Header->name[I];I++)
    if (I==sizeof(Header->name) || Header->name[I]<' ')
      return(FALSE);

  for (I=0;I<&Header->typeflag-Header->mode;I++)
    if (Header->mode[I]>'7' || Header->mode[I]<'0' && Header->mode[I]!=0 &&
        Header->mode[I]!=' ')
      return(FALSE);

  if(strcmp(Header->name,"././@LongLink"))
  {
    DWORD Seconds=GetOctal(Header->mtime);
    if (Seconds<300000000 || Seconds>1500000000)
      return(FALSE);
  }
  return(TRUE);
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
