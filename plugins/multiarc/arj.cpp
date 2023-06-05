/*
  ARJ.CPP

  Second-level plugin module for FAR Manager and MultiArc plugin

  Copyright (c) 1996 Eugene Roshal
  Copyrigth (c) 2000 FAR group
*/

#include <windows.h>
#include <limits.h>
#include <string.h>
#include <dos.h>
#include <plugin.hpp>
#include "fmt.hpp"

static HANDLE ArcHandle;
static DWORD NextPosition,SFXSize,FileSize;
static int ArcComment, ArcVolume, ArcRecovery, ArcLastChapter;

#ifndef FNAME_MAX
#define FNAME_MAX           512
#endif
#define FIRST_HDR_SIZE    30
#define COMMENT_MAX     2048
#define HEADERSIZE_MAX   (FIRST_HDR_SIZE + 10 + FNAME_MAX + COMMENT_MAX)

#define CRC_MASK        0xFFFFFFFFL
static DWORD crctable[UCHAR_MAX + 1];
static BOOL CRCInit=FALSE;
DWORD CRC;

#define UPDATE_CRC(r,c) r=crctable[((BYTE)(r)^(BYTE)(c))&0xff]^(r>>CHAR_BIT)
#define CRCPOLY         0xEDB88320L

static const char *ArjOS[]={"MSDOS","PRIMOS","UNIX","AMIGA","MAC-OS",
                      "OS/2","APPLE GS","ATARI ST","NEXT",
                      "VAX VMS","WIN95","WIN32"};

enum {
  ARJFMAIN_GARBLED_FLAG     = 0x01,
  ARJFMAIN_OLD_SECURED_FLAG = 0x02, //obsolete
  ARJFMAIN_ANSIPAGE_FLAG    = 0x02, // indicates ANSI codepage used by ARJ32
  ARJFMAIN_VOLUME_FLAG      = 0x04, //  indicates presence of succeeding volume
  ARJFMAIN_ARJPROT_FLAG     = 0x08,
  ARJFMAIN_PATHSYM_FLAG     = 0x10, // indicates archive name translated ("\" changed to "/")
  ARJFMAIN_BACKUP_FLAG      = 0x20, // obsolete
  ARJFMAIN_SECURED_FLAG     = 0x40,
  ARJFMAIN_ALTNAME_FLAG     = 0x80, // indicates dual-name archive
};

enum{
  ARJFFILE_GARBLED_FLAG     = 0x01, // indicates passworded file
  ARJFFILE_NOT_USED         = 0x02,
  ARJFFILE_VOLUME_FLAG      = 0x04, // indicates continued file to next volume (file is split)
  ARJFFILE_EXTFILE_FLAG     = 0x08, // indicates file starting position field (for split files)
  ARJFFILE_PATHSYM_FLAG     = 0x10, // indicates filename translated ("\" changed to "/")
  ARJFFILE_BACKUP_FLAG      = 0x20, // obsolete
};

static void make_crctable(void)
{
  UINT i, j;
  DWORD r;

  for (i = 0; i <= UCHAR_MAX; i++)
  {
    r = i;
    for (j = CHAR_BIT; j > 0; j--)
    {
      if (r & 1)
          r = (r >> 1) ^ CRCPOLY;
      else
          r >>= 1;
    }
    crctable[i] = r;
  }
}

static void crc_buf(const char *str, int len)
{
    while (len--)
        UPDATE_CRC(CRC, *str++);
}

BOOL WINAPI _export IsArchive(const char *Name,const unsigned char *Data,int DataSize)
{
  if(!CRCInit)
  {
    make_crctable();
    CRCInit=TRUE;
  }

  for (int I=0;I<DataSize-11;I++)
  {
    const unsigned char *D=Data+I;
    WORD HeaderSize;
    if (D[0]==0x60 && D[1]==0xEA &&
       (HeaderSize=*(const WORD*)(D+2)) <= HEADERSIZE_MAX &&
       D[7]<0x10 &&
       D[10]==2 &&
       I+4+HeaderSize < DataSize-11)
    {
      CRC=CRC_MASK;
      crc_buf((const char *)D+4, (int) HeaderSize);
      if ((CRC ^ CRC_MASK) == *(const DWORD*)(D+4+HeaderSize))
      {
        SFXSize=I;
        return(TRUE);
      }
    }
  }
  return(FALSE);
}


BOOL WINAPI _export OpenArchive(const char *Name,int *Type)
{
PACK_PUSH(1)
  struct ARJHd1
  {
    WORD Mark;            // header id (main and local file) = 0x60 0xEA
    WORD HeadSize;        // basic header size (from 'first_hdr_size' thru 'comment' below)
                          //     = first_hdr_size + strlen(filename) + 1 + strlen(comment) + 1
                          //     = 0 if end of archive
                          //   maximum header size is 2600
    BYTE FirstHeadSize;   // size up to and including 'extra data'
    BYTE ARJVer;          // archiver version number
    BYTE ARJExtrVer;      // minimum archiver version to extract
    BYTE HostOS;          // see ArjOS[]
    BYTE Flags;           // see ARJFMAIN_*
    BYTE SecurityVer;     // (2 = current)
    BYTE FileType;        // file type (must equal 2)
    BYTE Reserved;
    DWORD ftime;          // date time when original archive was created
    DWORD ltime;          // date time when archive was last modified
    DWORD ArcSize;        // currently used only for secured archives
    DWORD SecurityPos;    // security envelope file position
    WORD FileSpec;        // filespec position in filename

    WORD SecuritySize;    // length in bytes of security envelope data
    BYTE EncrtypVer;      // encryption version (0 and 1 = old, 2 = new, 3 = reserved, 4 = 40 bit key GOST)
    BYTE LastChapter;     // last chapter
                          // ... extra data
    /*
       ?   extra data
           1   arj protection factor
           1   arj flags (second series)
                     (0x01 = ALTVOLNAME_FLAG) indicates special volume naming
                                              option
                     (0x02 = reserved bit)
           2   spare bytes

       ?   filename of archive when created (null-terminated string)
       ?   archive comment  (null-terminated string)

       4   basic header CRC

       2   1st extended header size (0 if none)
       --

       ?   1st extended header (currently not used)
       4   1st extended header's CRC (not present when 0 extended header size)
    */
  } ArjHeader;
PACK_POP()
PACK_CHECK(ARJHd1, 1);

  DWORD ReadSize;
  WORD ARJComm,ExtHdSize;
  ArcHandle=CreateFile(Name,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,
                       NULL,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,NULL);
  if (ArcHandle==INVALID_HANDLE_VALUE)
    return(FALSE);

  *Type=0;

  FileSize=GetFileSize(ArcHandle,NULL);

  SetFilePointer(ArcHandle,SFXSize,NULL,FILE_BEGIN);
  // Read main header
  ReadFile(ArcHandle,&ArjHeader,sizeof(ArjHeader),&ReadSize,NULL);
  // Seek to 'comment'
  SetFilePointer(ArcHandle,SFXSize+ArjHeader.HeadSize+2,NULL,FILE_BEGIN);
  ReadFile(ArcHandle,&ARJComm,sizeof(ARJComm),&ReadSize,NULL);

  ArcVolume=((ArjHeader.Flags & ARJFMAIN_VOLUME_FLAG)!=0);
  ArcRecovery=((ArjHeader.Flags & ARJFMAIN_ARJPROT_FLAG)!=0);
  ArcComment=(ARJComm!=0);
  ArcLastChapter=ArjHeader.LastChapter;

  NextPosition=SetFilePointer(ArcHandle,4,NULL,FILE_CURRENT);
  ReadFile(ArcHandle,&ExtHdSize,sizeof(ExtHdSize),&ReadSize,NULL);

  NextPosition+=2;

  if (ExtHdSize>0)
    NextPosition+=ExtHdSize+4;

  return(TRUE);
}


int WINAPI _export GetArcItem(PluginPanelItem *Item, ArcItemInfo *Info)
{
PACK_PUSH(1)
  struct ARJHd2
  {
    WORD Mark;              // header id (main and local file) = 0x60 0xEA
    WORD HeadSize;          // basic header size
                            //     = first_hdr_size + strlen(filename) + 1 + strlen(comment) + 1
                            //     = 0 if end of archive
                            //   maximum header size is 2600
    BYTE FirstHeadSize;     // size up to and including 'extra data'
    BYTE ARJVer;            // archiver version number
    BYTE ARJExtrVer;        // minimum archiver version to extract
    BYTE HostOS;            // see ArjOS[]
    BYTE Flags;             // see ARJFFILE_*
    BYTE Method;            // method:
                            //      0 = stored
                            //      1 = compressed most ...
                            //      4 = compressed fastest
                            //      8 = no data, no CRC
                            //      9= no data
    BYTE FileType;          // file type:
                            //      0 = binary
                            //      1 = 7-bit text
                            //      3 = directory
                            //      4 = volume label
                            //      5 = chapter label - ARJ v 2.50+
                            //      6 = UNIX special file - ARJ v 2.77+
    BYTE Reserved;          //
    DWORD ftime;            // date time modified
    DWORD PackSize;         // compressed size
    DWORD UnpSize;          // original size (this will be different for text mode compression)
    DWORD CRC;              // original file's CRC
    WORD FileSpec;          // filespec position in filename
    WORD AccessMode;        // file access mode
    //WORD HostData;        //
    BYTE FirstChapter;      // first chapter of file's lifespan
    BYTE LastChapter;       // last chapter of file's lifespan
                            // ... extra data
    ////////////////
    //  ?   extra data
    DWORD Extra1;           // 4 bytes for extended file position
                            // the following twelve bytes may be present in ARJ 2.62 and above
    DWORD atime;            // 4 bytes for date-time accessed
    DWORD ctime;            // 4 bytes for date-time created
    DWORD FSizeVol;         // 4 bytes for original file size even for volumes
    /*
       ?   filename (null-terminated string)
       ?   comment  (null-terminated string)

       4   basic header CRC

       2   1st extended header size (0 if none)
       ?   1st extended header (currently not used)
       4   1st extended header's CRC (not present when 0 extended header size)

       ...

       ?   compressed file
    */
  } ArjHeader;
PACK_POP()
PACK_CHECK(ARJHd2, 1);

  DWORD ReadSize;

  NextPosition=SetFilePointer(ArcHandle,NextPosition,NULL,FILE_BEGIN);

  if (NextPosition==0xFFFFFFFF)
    return(GETARC_READERROR);

  if (NextPosition>FileSize)
    return(GETARC_UNEXPEOF);

  if (!ReadFile(ArcHandle,&ArjHeader,sizeof(ArjHeader),&ReadSize,NULL))
    return(GETARC_READERROR);

  if (ReadSize==0 || ArjHeader.HeadSize==0)
    return(GETARC_EOF);

  SetFilePointer(ArcHandle,ArjHeader.FirstHeadSize+NextPosition+4,NULL,FILE_BEGIN);

  char Name[NM];
  if (!ReadFile(ArcHandle,Name,sizeof(Name),&ReadSize,NULL) || ReadSize==0)
    return(GETARC_READERROR);
  Name[sizeof(Name)-1]=0;
  if (Name[lstrlen(Name)+1]!=0)
    Info->Comment=TRUE;
  lstrcpy(Item->FindData.cFileName,Name);

  DWORD PrevPosition=NextPosition;
  NextPosition+=8+ArjHeader.HeadSize;
  SetFilePointer(ArcHandle,NextPosition,NULL,FILE_BEGIN);

  WORD ExtHdSize;
  ReadFile(ArcHandle,&ExtHdSize,sizeof(ExtHdSize),&ReadSize,NULL); //??
  NextPosition+=2+ArjHeader.PackSize;
  if (ExtHdSize>0)
    NextPosition+=ExtHdSize+6;

  if (PrevPosition>=NextPosition)
    return(GETARC_BROKEN);

  if (ArjHeader.Flags & ARJFFILE_GARBLED_FLAG)
    Info->Encrypted=TRUE;
  Info->DictSize=32;

  Item->CRC32=ArjHeader.CRC;
  Item->FindData.dwFileAttributes=ArjHeader.AccessMode & 0x3f;
  Item->PackSize=ArjHeader.PackSize;
  Item->FindData.nFileSizeLow=ArjHeader.UnpSize;

  FILETIME lft;
  DosDateTimeToFileTime(HIWORD(ArjHeader.ftime),LOWORD(ArjHeader.ftime),&lft);
  LocalFileTimeToFileTime(&lft,&Item->FindData.ftLastWriteTime);

  if(ArjHeader.ARJVer >= 0x09 &&                                 // ??? 2.62 ???
     (sizeof(ArjHeader)-sizeof(DWORD)*5) < ArjHeader.FirstHeadSize) // 5 = (Mark+HeadSize)+Extra1+atime+ctime+FSizeVol
  {
    DosDateTimeToFileTime(HIWORD(ArjHeader.atime),LOWORD(ArjHeader.atime),&lft);
    LocalFileTimeToFileTime(&lft,&Item->FindData.ftLastAccessTime);
    DosDateTimeToFileTime(HIWORD(ArjHeader.ctime),LOWORD(ArjHeader.ctime),&lft);
    LocalFileTimeToFileTime(&lft,&Item->FindData.ftCreationTime);
  }

  Info->Chapter=ArjHeader.LastChapter; //???? FirstChapter ????

  Info->UnpVer=(ArjHeader.ARJExtrVer/10)*256+(ArjHeader.ARJExtrVer%10);
  if (ArjHeader.HostOS<ARRAYSIZE(ArjOS))
    lstrcpy(Info->HostOS,ArjOS[ArjHeader.HostOS]);

  return(GETARC_SUCCESS);
}


BOOL WINAPI _export CloseArchive(ArcInfo *Info)
{
  Info->SFXSize=SFXSize;
  Info->Volume=ArcVolume;
  Info->Comment=ArcComment;
  Info->Recovery=ArcRecovery;
  Info->Chapters=ArcLastChapter;
  return(CloseHandle(ArcHandle));
}

DWORD WINAPI _export GetSFXPos(void)
{
  return SFXSize;
}

BOOL WINAPI _export GetFormatName(int Type,char *FormatName,char *DefaultExt)
{
  if (Type==0)
  {
    lstrcpy(FormatName,"ARJ");
    lstrcpy(DefaultExt,"arj");
    return(TRUE);
  }
  return(FALSE);
}


BOOL WINAPI _export GetDefaultCommands(int Type,int Command,char *Dest)
{
  if (Type==0)
  {
    // Correct Arj/Win32 commands
    static const char *Commands[]={
    /*Extract               */"arj32 x -+ {-g%%P} -v -y -p1 -- %%A !%%LM",
    /*Extract without paths */"arj32 e -+ {-g%%P} -v -y -p1 -- %%A !%%LM",
    /*Test                  */"arj32 t -+ -y {-g%%P} -v -p1 -- %%A",
    /*Delete                */"arj32 d -+ -y {-w%%W} -p1 -- %%A !%%LNM",
    /*Comment archive       */"arj32 c -+ -y {-w%%W} -z -- %%A",
    /*Comment files         */"arj32 c -+ -y {-w%%W} -p1 -- %%A !%%LM",
    /*Convert to SFX        */"arj32 y -+ -je -y -p %%A",
    /*Lock archive          */"",
    /*Protect archive       */"arj32 t -hk -y %%A",
    /*Recover archive       */"arj32 q -y %%A",
    /*Add files             */"arj32 a -+ -y -a1 {-g%%P} {-w%%W} -p {%%S} -- %%A !%%LM",
    /*Move files            */"arj32 m -+ -y -a1 {-g%%P} {-w%%W} -p {%%S} -- %%A !%%LM",
    /*Add files and folders */"arj32 a -+ -r -y -a1 {-g%%P} {-w%%W} -p {%%S} -- %%A !%%LM",
    /*Move files and folders*/"arj32 m -+ -r -y -a1 {-g%%P} {-w%%W} -p {%%S} -- %%A !%%LM",
    /*"All files" mask      */"*.*"
    };
    if (Command<(int)(ARRAYSIZE(Commands)))
    {
      lstrcpy(Dest,Commands[Command]);
      return(TRUE);
    }
  }
  return(FALSE);
}
