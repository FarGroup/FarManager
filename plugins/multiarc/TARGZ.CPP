/*
  TARGZ.CPP

  Second-level plugin module for FAR Manager 1.70 and MultiArc plugin

  Copyright (c) 1996-2000 Eugene Roshal
  Copyrigth (c) 2000-2002 FAR group
*/
/* Revision: 1.24 30.10.2002 $ */

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

#if defined(USE_TAR_H)
#include "tar.h"

#else

struct posix_header
{                               /* byte offset */
  char name[100];               /*   0 = 0x000 */
  char mode[8];                 /* 100 = 0x064 */
  char uid[8];                  /* 108 = 0x06C */
  char gid[8];                  /* 116 = 0x074 */
  char size[12];                /* 124 = 0x07C */
  char mtime[12];               /* 136 = 0x088 */
  char chksum[8];               /* 148 = 0x094 */
  char typeflag;                /* 156 = 0x09C */
  char linkname[100];           /* 157 = 0x09D */
  char magic[6];                /* 257 = 0x101 */
  char version[2];              /* 263 = 0x107 */
  char uname[32];               /* 265 = 0x109 */
  char gname[32];               /* 297 = 0x129 */
  char devmajor[8];             /* 329 = 0x149 */
  char devminor[8];             /* 337 = 0x151 */
  char prefix[155];             /* 345 = 0x159 */
                                /* 500 = 0x1F4 */
};

#define TMAGIC   "ustar"	// ustar and a null
#define TMAGLEN  6
#define TVERSION "00"		// 00 and no null
#define TVERSLEN 2

/* OLDGNU_MAGIC uses both magic and version fields, which are contiguous.
   Found in an archive, it indicates an old GNU header format, which will be
   hopefully become obsolescent.  With OLDGNU_MAGIC, uname and gname are
   valid, though the header is not truly POSIX conforming.  */
#define OLDGNU_MAGIC "ustar  "	/* 7 chars and a null */


enum archive_format
{
  DEFAULT_FORMAT,		/* format to be decided later */
  V7_FORMAT,			/* old V7 tar format */
  OLDGNU_FORMAT,		/* GNU format as per before tar 1.12 */
  POSIX_FORMAT,			/* restricted, pure POSIX format */
  GNU_FORMAT			/* POSIX format with GNU extensions */
};


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

int IsTarHeader(const unsigned char *Data,int DataSize);
DWORD GetOctal(const char *Str);
int GetArcItemGZIP(struct PluginPanelItem *Item,struct ArcItemInfo *Info);
int GetArcItemTAR(struct PluginPanelItem *Item,struct ArcItemInfo *Info);
char *AdjustTARFileName(char *FileName);
static __int64 Oct2Size (const char *where0, size_t digs0);

enum {TAR_FORMAT,GZ_FORMAT,Z_FORMAT,BZ_FORMAT};

typedef union {
  __int64 i64;
  struct {
    DWORD LowPart;
    LONG  HighPart;
  } Part;
} FAR_INT64;


HANDLE ArcHandle;
DWORD SFXSize;
FAR_INT64 NextPosition,FileSize;
int ArcType;
enum archive_format TarArchiveFormat;
char ZipName[NM];

FARSTDLOCALSTRICMP LStricmp;

void  WINAPI _export SetFarInfo(const struct PluginStartupInfo *Info)
{
  LStricmp=Info->FSF->LStricmp;
}

// Number of 100 nanosecond units from 01.01.1601 to 01.01.1970
#define EPOCH_BIAS    116444736000000000i64

void WINAPI UnixTimeToFileTime( DWORD time, FILETIME * ft )
{
  *(__int64*)ft = EPOCH_BIAS + time * 10000000i64;
}

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
    if (LStricmp(Dot,"tgz")==0 || LStricmp(Dot,"taz")==0)
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

  FileSize.Part.LowPart=GetFileSize(ArcHandle,(LPDWORD) &FileSize.Part.HighPart);

  NextPosition.i64=0;
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
        Item->PackSize=Item->FindData.nFileSizeLow=FileSize.Part.LowPart;
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
  DWORD ReadSize;
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

  Item->PackSize=FileSize.Part.LowPart;

  if (ArcType==Z_FORMAT)
  {
    strcpy(Item->FindData.cFileName,ZipName);
    *ZipName=0;
    Item->FindData.nFileSizeLow=FileSize.Part.LowPart;
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
  BOOL SkipItem;//=FALSE;
  char *LongName = NULL;
  do
  {
    NextPosition.Part.LowPart=SetFilePointer(ArcHandle,NextPosition.Part.LowPart,&NextPosition.Part.HighPart,FILE_BEGIN);
    if (NextPosition.Part.LowPart==0xFFFFFFFF && GetLastError() != NO_ERROR)
      return(GETARC_READERROR);

    if (NextPosition.i64>FileSize.i64)
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
    }
    FAR_INT64 TarItemSize;
    TarItemSize.i64=Oct2Size(TAR_hdr.header.size,sizeof(TAR_hdr.header.size));
    Item->PackSizeHigh=Item->FindData.nFileSizeHigh=TarItemSize.Part.HighPart;
    Item->PackSize=Item->FindData.nFileSizeLow=TarItemSize.Part.LowPart;

    strcpy(Info->HostOS,TarArchiveFormat==POSIX_FORMAT?"POSIX":(TarArchiveFormat==V7_FORMAT?"V7":""));
    Info->UnpVer=256+11+(TarArchiveFormat >= POSIX_FORMAT?1:0); //!!!

    FAR_INT64 PrevPosition=NextPosition;
    // for LNKTYPE - only sizeof(TAR_hdr)
    NextPosition.i64+=sizeof(TAR_hdr)+(TAR_hdr.header.typeflag == LNKTYPE?0:Item->PackSize);

    if (NextPosition.i64 & 511)
      NextPosition.i64+=512-(NextPosition.i64 & 511);

    if (PrevPosition.i64>=NextPosition.i64)
      return(GETARC_BROKEN);

    if (TAR_hdr.header.typeflag == GNUTYPE_LONGNAME)
    {
      SetFilePointer (ArcHandle,PrevPosition.Part.LowPart+sizeof(TAR_hdr),&PrevPosition.Part.HighPart,FILE_BEGIN);
      // we can't have two LONGNAME records in a row without a file between them
      if (LongName != NULL)
        return GETARC_BROKEN;
      LongName = (char *)HeapAlloc(GetProcessHeap(), 0, Item->PackSize);
      DWORD BytesRead;
      ReadFile(ArcHandle,LongName,Item->PackSize,&BytesRead,NULL);
      if (BytesRead != Item->PackSize)
      {
        HeapFree(GetProcessHeap(),0,LongName);
        return GETARC_BROKEN;
      }
    }
  } while (SkipItem);

  if (LongName)
    HeapFree(GetProcessHeap(), 0, LongName);
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
       "tar --force-local -xf %%A %%FSq32768",
       "%comspec% /c tar --force-local -O -xf %%A %%fSq > %%fWq",
       "",
       "tar --delete --force-local -f %%A %%FSq32768",
       "",
       "",
       "",
       "",
       "",
       "",
       "tar --force-local -rf %%A %%FSq32768",
       "tar --force-local --remove-files -rf %%A %%FSq32768",
       "tar --force-local -rf %%A %%FSq32768",
       "tar --force-local --remove-files -rf %%A %%FSq32768",
       "*"
     },

     { // GZ_FORMAT
       "%COMSPEC% /c gzip -cd %%A >%%fq",
       "%COMSPEC% /c gzip -cd %%A >%%fq",
       "gzip -t %%A",
       "",
       "",
       "",
       "",
       "",
       "",
       "",
       "%COMSPEC% /c gzip -c %%fq >%%A",
       "gzip %%fq",
       "%COMSPEC% /c gzip -c %%fq >%%A",
       "gzip %%fq",
       "*"
     },

     { // Z_FORMAT
       "%COMSPEC% /c gzip -cd %%A >%%fq",
       "%COMSPEC% /c gzip -cd %%A >%%fq",
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
       "%COMSPEC% /c bzip2 -cd %%A >%%fq",
       "%COMSPEC% /c bzip2 -cd %%A >%%fq",
       "%COMSPEC% /c bzip2 -cd %%A >nul",
       "",
       "",
       "",
       "",
       "",
       "",
       "",
       "%COMSPEC% /c bzip2 -c %%fq >%%A",
       "bzip2 %%fq",
       "%COMSPEC% /c bzip2 -c %%fq >%%A",
       "bzip2 %%fq",
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

int IsTarHeader(const BYTE *Data,int DataSize)
{
  int I;
  struct posix_header *Header;
  if (DataSize<sizeof(struct posix_header))
    return(FALSE);

  Header=(struct posix_header *)Data;

  if(!strcmp (Header->magic, TMAGIC))
    TarArchiveFormat = POSIX_FORMAT;
  else if(!strcmp (Header->magic, OLDGNU_MAGIC))
    TarArchiveFormat = OLDGNU_FORMAT;
  else
    TarArchiveFormat = V7_FORMAT;

  for (I=0;Header->name[I];I++)
    if (I==sizeof(Header->name) || Header->name[I] < ' ')
      return(FALSE);

  for (I=0;I < 8;I++)
    if (Header->mode[I] > '7' || Header->mode[I]<'0' && Header->mode[I]!=0 && Header->mode[I]!=' ')
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

static __int64 Oct2Size (const char *where0, size_t digs0)
{
  __int64 value;
  const char *where = where0;
  size_t digs = digs0;

  for (;;)
  {
    if (!digs)
    {
       return -1;
    }

    if (*where != ' ')
      break;
    where++;
    digs--;
  }

  value = 0;
  while (digs != 0 && *where >= '0' && *where <= '7')
  {
    if (((value << 3) >> 3) != value)
      goto out_of_range;
    value = (value << 3) | (*where++ - '0');
    --digs;
  }

  if (digs && *where && *where != ' ')
    return -1;

  return value;

out_of_range:
  return -1;
}
