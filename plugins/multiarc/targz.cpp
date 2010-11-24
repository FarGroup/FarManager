/*
  TARGZ.CPP

  Second-level plugin module for FAR Manager and MultiArc plugin

  Copyright (c) 1996 Eugene Roshal
  Copyrigth (c) 2000 FAR group
*/

//#define USE_TAR_H


#include <windows.h>
#include <string.h>
#include <dos.h>
#include <CRT/crt.hpp>
#include <plugin.hpp>
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

#if defined(__GNUC__)
#ifdef __cplusplus
extern "C"{
#endif
  BOOL WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved);
#ifdef __cplusplus
};
#endif

BOOL WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved)
{
  (void) lpReserved;
  (void) dwReason;
  (void) hDll;
  return TRUE;
}
#endif

/*
#ifdef _MSC_VER
#if _MSC_VER < 1310
#pragma comment(linker, "/ignore:4078")
#pragma comment(linker, "-subsystem:console")
#pragma comment(linker, "-merge:.rdata=.text")

#endif
#endif
*/

unsigned __int64 __cdecl _strtoui64 (const char *nptr,char **endptr,int ibase);
static unsigned __int64 __cdecl _strtoxq (const char *nptr,const char **endptr,int ibase,int flags);
__int64 __cdecl _strtoi64(const char *nptr,char **endptr,int ibase);

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

#define TMAGIC   "ustar"    // ustar and a null
#define TMAGLEN  6
#define GNUTMAGIC  "GNUtar"    // 7 chars and a null
#define GNUTMAGLEN 7
#define TVERSION "00"       // 00 and no null
#define TVERSLEN 2

/* OLDGNU_MAGIC uses both magic and version fields, which are contiguous.
   Found in an archive, it indicates an old GNU header format, which will be
   hopefully become obsolescent.  With OLDGNU_MAGIC, uname and gname are
   valid, though the header is not truly POSIX conforming.  */
#define OLDGNU_MAGIC "ustar  "  /* 7 chars and a null */


enum archive_format
{
  DEFAULT_FORMAT,       /* format to be decided later */
  V7_FORMAT,            /* old V7 tar format */
  OLDGNU_FORMAT,        /* GNU format as per before tar 1.12 */
  POSIX_FORMAT,         /* restricted, pure POSIX format */
  GNU_FORMAT            /* POSIX format with GNU extensions */
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

enum {TAR_FORMAT,GZ_FORMAT,Z_FORMAT,BZ_FORMAT};

typedef union {
  __int64 i64;
  struct {
    DWORD LowPart;
    LONG  HighPart;
  } Part;
} FAR_INT64;



int IsTarHeader(const unsigned char *Data,int DataSize);
__int64 GetOctal(const char *Str);
int GetArcItemGZIP(struct PluginPanelItem *Item,struct ArcItemInfo *Info);
int GetArcItemTAR(struct PluginPanelItem *Item,struct ArcItemInfo *Info);
char *AdjustTARFileName(char *FileName);
static __int64 Oct2Size (const char *where0, size_t digs0);

HANDLE ArcHandle;
DWORD SFXSize;
FAR_INT64 NextPosition,FileSize;
int ArcType;
enum archive_format TarArchiveFormat;
char ZipName[NM];

FARSTDLOCALSTRICMP LStricmp;

typedef int  (WINAPI *FARSTDMKLINK)(const char *Src,const char *Dest,DWORD Flags);

typedef void (__cdecl *MAFREE)(void *block);
typedef void * (__cdecl *MAMALLOC)(size_t size);

MAFREE MA_free;
MAMALLOC MA_malloc;

void  WINAPI _export SetFarInfo(const struct PluginStartupInfo *Info)
{
  LStricmp=Info->FSF->LStricmp;
  MA_free=(MAFREE)Info->FSF->Reserved[1];
  MA_malloc=(MAMALLOC)Info->FSF->Reserved[0];
}

// Number of 100 nanosecond units from 01.01.1601 to 01.01.1970
#define EPOCH_BIAS    _i64(116444736000000000)

void WINAPI UnixTimeToFileTime( DWORD time, FILETIME * ft )
{
  *(__int64*)ft = EPOCH_BIAS + (__int64)time * _i64(10000000);
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
  lstrcpy(ZipName,NamePtr);
  const char *Dot=(const char *)strrchr((char*)NamePtr,'.');

  if (Dot!=NULL)
  {
    Dot++;
    if (LStricmp(Dot,"tgz")==0 || LStricmp(Dot,"taz")==0)
      lstrcpy(&ZipName[Dot-NamePtr],"tar");
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
        Item->PackSizeHigh=Item->FindData.nFileSizeHigh=FileSize.Part.HighPart;
        lstrcpy(Item->FindData.cFileName,ZipName);
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
  Item->PackSizeHigh=FileSize.Part.HighPart;

  if (ArcType==Z_FORMAT)
  {
    lstrcpy(Item->FindData.cFileName,ZipName);
    *ZipName=0;
    Item->FindData.nFileSizeLow=FileSize.Part.LowPart;
    Item->FindData.nFileSizeHigh=FileSize.Part.HighPart;
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
    lstrcpy(Item->FindData.cFileName,ZipName);

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
    NextPosition.Part.LowPart=SetFilePointer(ArcHandle,NextPosition.Part.LowPart,&NextPosition.Part.HighPart,FILE_BEGIN);

    if (NextPosition.i64 == (__int64)-1 && GetLastError() != NO_ERROR)
      return(GETARC_READERROR);

    if (NextPosition.i64 > FileSize.i64)
      return(GETARC_UNEXPEOF);

    if (!ReadFile(ArcHandle,&TAR_hdr,sizeof(TAR_hdr),&ReadSize,NULL))
      return(GETARC_READERROR);

    if (ReadSize==0 || *TAR_hdr.header.name==0)
      return(GETARC_EOF);

    if (TAR_hdr.header.typeflag == GNUTYPE_LONGLINK || TAR_hdr.header.typeflag == GNUTYPE_LONGNAME)
    {
      SkipItem=TRUE;
    }
    else
    {
      // TODO: GNUTYPE_LONGLINK
      DWORD dwAddFileAttr=0;
      SkipItem=FALSE;
      char *EndPos;
      if (LongName != NULL)
        EndPos = AdjustTARFileName (LongName);
      else
      {
        char namebuf[sizeof(TAR_hdr.header.prefix) + 1 + sizeof(TAR_hdr.header.name) + 1];
        char *np = namebuf;
        if(TAR_hdr.header.prefix[0])
        {
          memcpy (np, TAR_hdr.header.prefix, sizeof(TAR_hdr.header.prefix));
          np[sizeof (TAR_hdr.header.prefix)] = '\0';
          np += lstrlen(np);
          *np++ = '/';
        }
        memcpy (np, TAR_hdr.header.name, sizeof(TAR_hdr.header.name));
        np[sizeof(TAR_hdr.header.name)] = '\0';
        EndPos = AdjustTARFileName (namebuf);
      }
      lstrcpyn(Item->FindData.cFileName,EndPos,sizeof(Item->FindData.cFileName));
      Item->FindData.nFileSizeHigh=0;
      if(((DWORD)GetOctal(TAR_hdr.header.mode) & 0x4000) || ((TAR_hdr.header.typeflag-'0') & 4))
         dwAddFileAttr|=FILE_ATTRIBUTE_DIRECTORY;

      if(TAR_hdr.header.typeflag == SYMTYPE || TAR_hdr.header.typeflag == LNKTYPE)
      {
        if(TAR_hdr.header.typeflag == SYMTYPE)
        {
          #ifndef IO_REPARSE_TAG_SYMLINK
            #define IO_REPARSE_TAG_SYMLINK 0xA000000C
          #endif
          dwAddFileAttr|=FILE_ATTRIBUTE_REPARSE_POINT;
          Item->FindData.dwReserved0=IO_REPARSE_TAG_SYMLINK;
        }

        if((Item->UserData=(DWORD_PTR)MA_malloc(lstrlen(TAR_hdr.header.linkname)+2)) != NULL)
        {
          EndPos = AdjustTARFileName (TAR_hdr.header.linkname);
          if(TAR_hdr.header.typeflag == LNKTYPE)
            *(char*)Item->UserData='/';
          lstrcpyn((char*)Item->UserData+(TAR_hdr.header.typeflag == LNKTYPE?1:0),EndPos,lstrlen(TAR_hdr.header.linkname)+1);
        }
      }

      Item->FindData.dwFileAttributes=dwAddFileAttr;

      UnixTimeToFileTime((DWORD)GetOctal(TAR_hdr.header.mtime),&Item->FindData.ftLastWriteTime);
    }
    FAR_INT64 TarItemSize;
    TarItemSize.i64=Oct2Size(TAR_hdr.header.size,sizeof(TAR_hdr.header.size));
    Item->PackSize=Item->FindData.nFileSizeLow=TarItemSize.Part.LowPart;
    Item->PackSizeHigh=Item->FindData.nFileSizeHigh=TarItemSize.Part.HighPart;

    lstrcpy(Info->HostOS,TarArchiveFormat==POSIX_FORMAT?"POSIX":(TarArchiveFormat==V7_FORMAT?"V7":""));
    Info->UnpVer=256+11+(TarArchiveFormat >= POSIX_FORMAT?1:0); //!!!

    FAR_INT64 PrevPosition=NextPosition;
    // for LNKTYPE - only sizeof(TAR_hdr)
    NextPosition.i64+=(__int64)sizeof(TAR_hdr)+(TAR_hdr.header.typeflag == LNKTYPE?_i64(0):TarItemSize.i64);

    if (NextPosition.i64 & _i64(511))
      NextPosition.i64+=_i64(512)-(NextPosition.i64 & _i64(511));

    if (PrevPosition.i64 >= NextPosition.i64)
      return(GETARC_BROKEN);

    // TODO: GNUTYPE_LONGLINK
    if (TAR_hdr.header.typeflag == GNUTYPE_LONGNAME || TAR_hdr.header.typeflag == GNUTYPE_LONGLINK)
    {
      PrevPosition.i64+=(__int64)sizeof(TAR_hdr);
      SetFilePointer (ArcHandle,PrevPosition.Part.LowPart,&PrevPosition.Part.HighPart,FILE_BEGIN);
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
      lstrcpy(FormatName,FmtAndExt[Type][0]);
      lstrcpy(DefaultExt,FmtAndExt[Type][1]);
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
   if (Type >= TAR_FORMAT && Type <= BZ_FORMAT && Command < (int)(ARRAYSIZE(Commands[Type])))
   {
     lstrcpy(Dest,Commands[Type][Command]);
     return(TRUE);
   }
   return(FALSE);
}

int IsTarHeader(const BYTE *Data,int DataSize)
{
  size_t I;
  struct posix_header *Header;

  if (DataSize<(int)sizeof(struct posix_header))
    return(FALSE);

  Header=(struct posix_header *)Data;

  if(!lstrcmp (Header->magic, TMAGIC))
    TarArchiveFormat = POSIX_FORMAT;
  else if(!lstrcmp (Header->magic, OLDGNU_MAGIC))
    TarArchiveFormat = OLDGNU_FORMAT;
  else
    TarArchiveFormat = V7_FORMAT;

  for (I=0; Header->name[I] && I < sizeof(Header->name); I++)
    if (Header->name[I] < ' ')
      return FALSE;

  //for (I=0; I < (&Header->typeflag - &Header->mode[0]); I++)
  for (I=0; I < sizeof(Header->mode); I++)
  {
    int Mode=Header->mode[I];
    if (Mode > '7' || (Mode < '0' && Mode && Mode != ' '))
      return FALSE;
  }

  for (I=0; Header->mtime[I] && I < sizeof(Header->mtime); I++)
    if (Header->mtime[I] < ' ')
      return FALSE;

  __int64 Sum=256;
  for(I=0; I <= 147; I++)
    Sum+=Data[I];

  for(I=156; I < 512; I++)
    Sum+=Data[I];

  return(Sum==GetOctal(Header->chksum));
/*
  if(lstrcmp(Header->name,"././@LongLink"))
  {
    __int64 Seconds=GetOctal(Header->mtime);
    if (Seconds < 300000000i64 || Seconds > 1500000000i64)
    {
      if(Header->typeflag != DIRTYPE && Header->typeflag != SYMTYPE)
        return(FALSE);
    }
  }
  return(TRUE);
*/
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


__int64 GetOctal(const char *Str)
{
  char *endptr;
  return _strtoi64(Str,&endptr,8);
//  return(strtoul(Str,&endptr,8));
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

//#if _MSC_VER < 1310

// strtoq.c

/***
*strtoi64, strtoui64(nptr,endptr,ibase) - Convert ascii string to __int64 un/signed
*    int.
*
*Purpose:
*    Convert an ascii string to a 64-bit __int64 value.  The base
*    used for the caculations is supplied by the caller.  The base
*    must be in the range 0, 2-36.  If a base of 0 is supplied, the
*    ascii string must be examined to determine the base of the
*    number:
*        (a) First char = '0', second char = 'x' or 'X',
*            use base 16.
*        (b) First char = '0', use base 8
*        (c) First char in range '1' - '9', use base 10.
*
*    If the 'endptr' value is non-NULL, then strtoq/strtouq places
*    a pointer to the terminating character in this value.
*    See ANSI standard for details
*
*Entry:
*    nptr == NEAR/FAR pointer to the start of string.
*    endptr == NEAR/FAR pointer to the end of the string.
*    ibase == integer base to use for the calculations.
*
*    string format: [whitespace] [sign] [0] [x] [digits/letters]
*
*Exit:
*    Good return:
*        result
*
*    Overflow return:
*        strtoi64 -- _I64_MAX or _I64_MIN
*        strtoui64 -- _UI64_MAX
*        strtoi64/strtoui64 -- errno == ERANGE
*
*    No digits or bad base return:
*        0
*        endptr = nptr*
*
*Exceptions:
*    None.
*******************************************************************************/

static unsigned __int64 __cdecl _strtoxq (const char *nptr,const char **endptr,int ibase,int flags)
{
/* flag values */
#define FL_UNSIGNED   1       /* strtouq called */
#define FL_NEG        2       /* negative sign found */
#define FL_OVERFLOW   4       /* overflow occured */
#define FL_READDIGIT  8       /* we've read at least one correct digit */
#undef _UI64_MAX
#define _UI64_MAX     _ui64(0xFFFFFFFFFFFFFFFF)
#define _UI64_MAXDIV8 _ui64(0x1FFFFFFFFFFFFFFF)
#undef _I64_MIN
#define _I64_MIN    (_i64(-9223372036854775807) - 1)
#undef _I64_MAX
#define _I64_MAX      _i64(9223372036854775807)

    const char *p;
    char c;
    unsigned __int64 number;
    unsigned digval;
    unsigned __int64 maxval;

    p = nptr;            /* p is our scanning pointer */
    number = 0;            /* start with zero */

    c = *p++;            /* read char */
    while (c == 0x09 || c == 0x0D || c == 0x20)
        c = *p++;        /* skip whitespace */

    if (c == '-') {
        flags |= FL_NEG;    /* remember minus sign */
        c = *p++;
    }
    else if (c == '+')
        c = *p++;        /* skip sign */

    if (ibase < 0 || ibase == 1 || ibase > 36) {
        /* bad base! */
        if (endptr)
            /* store beginning of string in endptr */
            *endptr = nptr;
        return 0L;        /* return 0 */
    }
    else if (ibase == 0) {
        /* determine base free-lance, based on first two chars of
           string */
        if (c != '0')
            ibase = 10;
        else if (*p == 'x' || *p == 'X')
            ibase = 16;
        else
            ibase = 8;
    }

    if (ibase == 16) {
        /* we might have 0x in front of number; remove if there */
        if (c == '0' && (*p == 'x' || *p == 'X')) {
            ++p;
            c = *p++;    /* advance past prefix */
        }
    }

    /* if our number exceeds this, we will overflow on multiply */
#ifdef _MSC_VER
#if _MSC_VER >= 1310
    maxval = (unsigned __int64)0x1FFFFFFFFFFFFFFFi64; // hack for VC.2003 = _UI64_MAX/8 :-)
#else
    maxval = _UI64_MAX / (unsigned __int64)ibase;
#endif
#else
    maxval = _UI64_MAX / (unsigned __int64)ibase;
#endif

    for (;;) {    /* exit in middle of loop */
        /* convert c to value */
        if ( (BYTE)c >= '0' && (BYTE)c <= '9' )
            digval = c - '0';
        else if ( (BYTE)c >= 'A' && (BYTE)c <= 'Z')
            digval = c - 'A' + 10;
        else if ((BYTE)c >= 'a' && (BYTE)c <= 'z')
            digval = c - 'a' + 10;
        else
            break;
        if (digval >= (unsigned)ibase)
            break;        /* exit loop if bad digit found */

        /* record the fact we have read one digit */
        flags |= FL_READDIGIT;

        /* we now need to compute number = number * base + digval,
           but we need to know if overflow occured.  This requires
           a tricky pre-check. */

        if (number < maxval || (number == maxval &&
        (unsigned __int64)digval <= _UI64_MAX % (unsigned __int64)ibase)) {
            /* we won't overflow, go ahead and multiply */
            number = number * (unsigned __int64)ibase + (unsigned __int64)digval;
        }
        else {
            /* we would have overflowed -- set the overflow flag */
            flags |= FL_OVERFLOW;
        }

        c = *p++;        /* read next digit */
    }

    --p;                /* point to place that stopped scan */

    if (!(flags & FL_READDIGIT)) {
        /* no number there; return 0 and point to beginning of
           string */
        if (endptr)
            /* store beginning of string in endptr later on */
            p = nptr;
        number = _i64(0);        /* return 0 */
    }
    else if ( (flags & FL_OVERFLOW) ||
              ( !(flags & FL_UNSIGNED) &&
                ( ( (flags & FL_NEG) && (number > -_I64_MIN) ) ||
                  ( !(flags & FL_NEG) && (number > _I64_MAX) ) ) ) )
    {
        /* overflow or signed overflow occurred */
//        errno = ERANGE;
        if ( flags & FL_UNSIGNED )
            number = _UI64_MAX;
        else if ( flags & FL_NEG )
            number = _I64_MIN;
        else
            number = _I64_MAX;
    }
    if (endptr != NULL)
        /* store pointer to char that stopped the scan */
        *endptr = p;

    if (flags & FL_NEG)
        /* negate result if there was a neg sign */
        number = (unsigned __int64)(-(__int64)number);

    return number;            /* done. */
}

__int64 __cdecl _strtoi64(const char *nptr,char **endptr,int ibase)
{
    return (__int64) _strtoxq(nptr, (const char **)endptr, ibase, 0);
}
unsigned __int64 __cdecl _strtoui64 (const char *nptr,char **endptr,int ibase)
{
    return _strtoxq(nptr, (const char **)endptr, ibase, FL_UNSIGNED);
}
//#endif
