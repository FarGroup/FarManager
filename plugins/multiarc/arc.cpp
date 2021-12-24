/*
  ARC.CPP

  Second-level plugin module for FAR Manager and MultiArc plugin

  Copyrigth (c) 2004 FAR group
*/

#include <windows.h>
#include <string.h>
#include <dos.h>
#include <plugin.hpp>
#include "fmt.hpp"

#define ARCMARK        0x1A    // special archive marker
#define FNLEN          13      // file name length
#define ARCVER         9       // archive header version code

/*
OFFSET              Count TYPE   Description

0000h                   1 byte   ID=1Ah
0001h                   1 byte   Compression method (see table 0001)
0002h                  12 char   File name
000Fh                   1 dword  Compressed file size
0013h                   1 dword  File date in MS-DOS format (see table 0009)
0017h                   1 word   16-bit CRC
0019h                   1 dword  Original file size

(Table 0001)
ARC compression types
    0 - End of archive marker
    1 - unpacked (obsolete) - ARC 1.0 ?
    2 - unpacked - ARC 3.1
    3 - packed (RLE encoding)
    4 - squeezed (after packing)
    5 - crunched (obsolete) - ARC 4.0
    6 - crunched (after packing) (obsolete) - ARC 4.1
    7 - crunched (after packing, using faster hash algorithm) - ARC 4.6
    8 - crunched (after packing, using dynamic LZW variations) - ARC 5.0
    9 - Squashed c/o Phil Katz (no packing) (var. on crunching)
   10 - crushed (PAK only)
   11 - distilled (PAK only)
12-19 -  to 19 unknown (ARC 6.0 or later) - ARC 7.0 (?)
20-29 - ?informational items? - ARC 6.0
30-39 - ?control items? - ARC 6.0
  40+ - reserved

According to SEA's technical memo, the information and control items
were added to ARC 6.0. Information items use the same headers as archived
files, although the original file size (and name?) can be ignored.

OFFSET              Count TYPE   Description
0000h                   2 byte   Length of header (includes "length"
                                 and "type"?)
0002h                   1 byte   (sub)type
0003h                   ? byte   data

Informational item types as used by ARC 6.0 :

Block type    Subtype   Description
   20                   archive info
                0       archive description (ASCIIZ)
                1       name of creator program (ASCIIZ)
                2       name of modifier program (ASCIIZ)

   21                   file info
                0       file description (ASCIIZ)
                1       long name (if not MS-DOS "8.3" filename)
                2       extended date-time info (reserved)
                3       icon (reserved)
                4       file attributes (ASCIIZ)

                        Attributes use an uppercase letter to signify the
                        following:

                                R       read access
                                W       write access
                                H       hidden file
                                S       system file
                                N       network shareable

   22                   operating system info (reserved)


Informational item types as used by PAK release 1.5 :

     Basic archives end with a short header, containing just the
marker (26) and the end of file value (0).  PAK release 1.5 extended
this format by adding information after this end of file marker.  Each
extended record has the following header:

Marker (1 byte)  - always 254
type (1 byte)    - type of record
File (2 bytes)   - # of file in archive to which this record refers,
                   or 0 for the entire archive.
length (4 bytes) - size of record

     Type      Meaning
       0  End of file
       1  Remark
       2  Path
       3  Security envelope
       4  Error correction codes (not implemented in PAK 2.xx)


(Table 0009)
Format of the MS-DOS time stamp (32-bit)
The MS-DOS time stamp is limited to an even count of seconds, since the
count for seconds is only 5 bits wide.

  31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16
 |<---- year-1980 --->|<- month ->|<--- day ---->|

  15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
 |<--- hour --->|<---- minute --->|<- second/2 ->|


*/

PACK_PUSH(1)
struct RecHeader
{
  BYTE  HeadId;                // special archive marker = 0x1A or 0xFE
  BYTE  Type;                  // Compression method or
                               //  type of record:  0  End of file,
                               //                   1  Remark,
                               //                   2  Path,
                               //                   3  Security envelope,
                               //                   4  Error correction codes (not implemented in PAK 2.xx)
};
PACK_POP()
PACK_CHECK(RecHeader, 1);

PACK_PUSH(1)
struct ARCHeader
{
  BYTE  HeadId;                // special archive marker = 0x1A or 0xFE
  BYTE  Type;                  // Compression method or
  char  Name[FNLEN];           // File name
  DWORD CompSize;              // Compressed file size
  WORD  FileDate;              // File date in MS-DOS format
  WORD  FileTime;              // File time in MS-DOS format
  WORD  CRC16;                 // 16-bit CRC
  DWORD OrigSize;              // Original file size
};
PACK_POP()
PACK_CHECK(ARCHeader, 1);


PACK_PUSH(1)
struct PAKExtHeader
{
  RecHeader RHead;
  WORD  FileId;                // # of file in archive to which this record refers, or 0 for the entire archive.
  DWORD RecSize;               // size of record
  //BYTE  Data[];              // Original file size
};
PACK_POP()
PACK_CHECK(PAKExtHeader, 1);

enum {ARC_FORMAT};

static HANDLE ArcHandle;
static DWORD NextPosition,FileSize,SFXSize;
static int ArcType;
static DWORD OffsetComment;//, OffsetComment0;
//static int CommentSize=-1;
static int ArcComment;

BOOL WINAPI _export IsArchive(const char *Name,const unsigned char *Data,int DataSize)
{
  int I=0;

/*
  if(Data[0] == 'M' && Data[1] == 'Z') // SFX
  {
    PIMAGE_DOS_HEADER pMZHeader=(PIMAGE_DOS_HEADER)Data;
    I=(pMZHeader->e_cp-1)*512+pMZHeader->e_cblp;
  }
*/
  if(DataSize > (int)(sizeof(RecHeader)+sizeof(ARCHeader)))
  {
    const ARCHeader *D=(const ARCHeader*)(Data+I);
    if (D->HeadId == ARCMARK &&
        D->Type <= 11 && //???
        DataSize >= (int)(D->CompSize+sizeof(ARCHeader)))
    {
      SFXSize=I;
      ArcType=ARC_FORMAT;
      return(TRUE);
    }
  }
  return(FALSE);
}

DWORD WINAPI _export GetSFXPos(void)
{
  return SFXSize;
}


BOOL WINAPI _export OpenArchive(const char *Name,int *Type)
{
  ArcHandle=CreateFile(Name,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,
                       NULL,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,NULL);
  if (ArcHandle==INVALID_HANDLE_VALUE)
    return(FALSE);

  if(Type)
    *Type=ArcType;

  FileSize=GetFileSize(ArcHandle,NULL);
  NextPosition=SFXSize;

  OffsetComment=0;
  ArcComment=0;

  return(TRUE);
}


int WINAPI _export GetArcItem(PluginPanelItem *Item, ArcItemInfo *Info)
{
  ARCHeader Header;
  DWORD ReadSize;
  NextPosition=SetFilePointer(ArcHandle,NextPosition,NULL,FILE_BEGIN);

  if (NextPosition==0xFFFFFFFF)
    return(GETARC_READERROR);

  if (NextPosition>FileSize)
    return(GETARC_UNEXPEOF);

  if (!ReadFile(ArcHandle,&Header,sizeof(WORD),&ReadSize,NULL))
    return(GETARC_READERROR);

  if (!ReadSize || ReadSize!=2)
    return(GETARC_EOF);

  if (Header.HeadId != 0x1A || ((Header.Type)&0x80))
    return GETARC_BROKEN;

  if(Header.Type == 0)
  {
    DWORD CurPos=SetFilePointer(ArcHandle,ReadSize+NextPosition,NULL,FILE_BEGIN);
    if(CurPos != (DWORD)-1 && CurPos < FileSize)
      ArcComment=1;
    return(GETARC_EOF);
  }

  if (!ReadFile(ArcHandle,((char*)&Header)+sizeof(WORD),sizeof(Header)-sizeof(WORD),&ReadSize,NULL))
    return(GETARC_READERROR);

  NextPosition+=sizeof(Header)+Header.CompSize;
  if(Header.Type == 1) // old style is shorter
  {
    // SetFilePointer(ArcHandle,-(sizeof(DWORD)),NULL,FILE_CURRENT); // для варианта с извлечением!
    Header.Type=2;  // convert header to new format
    Header.OrigSize=Header.CompSize;  // size is same when not packed
    NextPosition-=sizeof(DWORD);
  }

  if(OffsetComment)
  {
    SetFilePointer(ArcHandle,OffsetComment,NULL,FILE_BEGIN);
    ReadFile(ArcHandle,Info->Description,32,&ReadSize,NULL);
    OffsetComment+=32;
    Info->Comment=1;
  }

  lstrcpyn(Item->FindData.cFileName,Header.Name,sizeof(Item->FindData.cFileName));

  lstrcpyn(Item->FindData.cAlternateFileName,Header.Name,sizeof(Item->FindData.cAlternateFileName));

  Item->FindData.nFileSizeLow=Header.OrigSize;
  Item->FindData.nFileSizeHigh=0;
  Item->FindData.dwFileAttributes=FILE_ATTRIBUTE_ARCHIVE; //??
  Item->PackSize=Header.CompSize;

  Item->CRC32=(DWORD)Header.CRC16;

  FILETIME lft;
  DosDateTimeToFileTime(Header.FileDate,Header.FileTime,&lft);
  LocalFileTimeToFileTime(&lft,&Item->FindData.ftLastWriteTime);

  int Ver=6*256;
  if(Header.Type == 2)
    Ver=3*256+1;
  else if(Header.Type <= 5)
    Ver=4*256;
  else if(Header.Type == 6)
    Ver=4*256+1;
  else if(Header.Type == 7)
    Ver=4*256+6;
  else if(Header.Type <= 9)
    Ver=5*256;

  Info->UnpVer=Ver;

  return(GETARC_SUCCESS);
}


BOOL WINAPI _export CloseArchive(ArcInfo *Info)
{
  Info->Comment=ArcComment;
  return(CloseHandle(ArcHandle));
}

BOOL WINAPI _export GetFormatName(int Type,char *FormatName,char *DefaultExt)
{
  if (Type==ARC_FORMAT)
  {
    lstrcpy(FormatName,"ARC");
    lstrcpy(DefaultExt,"arc");
    return(TRUE);
  }
  return(FALSE);
}


BOOL WINAPI _export GetDefaultCommands(int Type,int Command,char *Dest)
{
  if (Type==ARC_FORMAT)
  {
    static const char *Commands[]={
    /*Extract               */"arc32 xo{%%S}{g%%P} %%a %%FMQ",
    /*Extract without paths */"arc32 eo{%%S}{g%%P} %%a %%FMQ",
    /*Test                  */"arc32 t{g%%P} %%a %%FMQ",
    /*Delete                */"arc32 d{g%%P} %%a %%FMQ",
    /*Comment archive       */"",
    /*Comment files         */"",
    /*Convert to SFX        */"",
    /*Lock archive          */"",
    /*Protect archive       */"",
    /*Recover archive       */"",
    /*Add files             */"arc32 a{%%S}{g%%P} %%a %%FMQ",
    /*Move files            */"arc32 m{%%S}{g%%P} %%a %%FMQ",
    /*Add files and folders */"arc32 a{%%S}{g%%P} %%a %%FMQ",
    /*Move files and folders*/"arc32 m{%%S}{g%%P} %%a %%FMQ",
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
