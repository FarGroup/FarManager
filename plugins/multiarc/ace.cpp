/*
  ACE.CPP

  Second-level plugin module for FAR Manager and MultiArc plugin

  Copyright (c) 1996 Eugene Roshal
  Copyrigth (c) 2000 FAR group
*/

#include <windows.h>
#include <string.h>
#include <dos.h>
#include <malloc.h>
#include <stddef.h>
#include <memory.h>
#include <plugin.hpp>
#include "fmt.hpp"

//#define CALC_CRC

static const struct OSIDType
{
  BYTE Type;
  char Name[15];
}
 OSID[]=
{
  {0,"MS-DOS"},   {1,"OS/2"},     {2,"Win32"},
  {3,"Unix"},     {4,"MAC-OS"},   {5,"Win NT"},
  {6,"Primos"},   {7,"APPLE GS"}, {8,"ATARI"},
  {9,"VAX VMS"},  {10,"AMIGA"},   {11,"NEXT"},
};

PACK_PUSH(1)
struct ACEHEADER
{
  WORD  CRC16;        // CRC16 over block
  WORD  HeaderSize;   // size of the block(from HeaderType)
  BYTE  HeaderType;   // archive header type is 0
  WORD  HeaderFlags;
  BYTE  Signature[7]; // '**ACE**'
  BYTE  VerExtract;   // version needed to extract archive
  BYTE  VerCreate;    // version used to create the archive
  BYTE  Host;         // HOST-OS for ACE used to create the archive
  BYTE  VolumeNum;    // which volume of a multi-volume-archive is it?
  DWORD AcrTime;      // date and time in MS-DOS format
  BYTE  Reserved[8];  // 8 bytes reserved for the future
};
PACK_POP()
PACK_CHECK(ACEHEADER, 1);


static HANDLE ArcHandle;
static DWORD NextPosition,FileSize,SFXSize;
static ACEHEADER MainHeader;
int HostOS=0, UnpVer=0;

#if defined(CALC_CRC)
#define CRCPOLY 0xEDB88320
#define CRC_MASK 0xFFFFFFFF

static unsigned crctable[256];
static BOOL CRCTableCreated=FALSE;


static void make_crctable(void)
{
  unsigned r,i,j;
  for (i=0;i<=255;i++)
  {
    for (r=i,j=8;j;j--)
      r=(r&1)?(r>>1)^CRCPOLY:(r>>1);
    crctable[i] = r;
  }
  CRCTableCreated=TRUE;
}

static DWORD getcrc(DWORD crc,BYTE *addr,int len)
{
  if(!CRCTableCreated) make_crctable();
  while (len--)
    crc=crctable[(BYTE)crc^(*addr++)]^(crc>>8);
  return(crc);
}
#endif


BOOL WINAPI _export IsArchive(const char *Name,const unsigned char *Data,int DataSize)
{
  for (int I=0;I<(int)(DataSize-sizeof(ACEHEADER));I++)
  {
    const unsigned char *D=Data+I;
    if (D[0]=='*' && D[1]=='*' && D[2]=='A' && D[3]=='C' && D[4]=='E' && D[5]=='*' && D[6]=='*' )
    {
      const ACEHEADER *Header=(const ACEHEADER *)(Data+I-7);
#if defined(CALC_CRC)
      DWORD crc=CRC_MASK;
      crc=getcrc(crc,&Header->HeaderType,Header->HeaderSize);
#endif

      if (Header->HeaderType == 0
          && Header->HeaderSize >= (sizeof(ACEHEADER)-offsetof(ACEHEADER,HeaderFlags))
#if defined(CALC_CRC)
          && LOWORD(crc) == LOWORD(Header->CRC16)
#endif
         )
      {
        SFXSize=I-7;
        return(TRUE);
      }
    }
  }
  return(FALSE);
}


BOOL WINAPI _export OpenArchive(const char *Name,int *Type)
{
  DWORD ReadSize;

  ArcHandle=CreateFile(Name,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,
                       NULL,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,NULL);
  if (ArcHandle==INVALID_HANDLE_VALUE)
    return(FALSE);

  *Type=0;

  FileSize=GetFileSize(ArcHandle,NULL);
  NextPosition=SFXSize;

  SetFilePointer(ArcHandle,NextPosition,NULL,FILE_BEGIN);
  if (!ReadFile(ArcHandle,&MainHeader,sizeof(MainHeader),&ReadSize,NULL) ||
      ReadSize!=sizeof(MainHeader))
  {
    CloseHandle(ArcHandle);
    return(FALSE);
  }

  NextPosition=SFXSize+MainHeader.HeaderSize+sizeof(WORD)*2;
  SetFilePointer(ArcHandle,NextPosition,NULL,FILE_BEGIN);

  for(int I=0; I < (int)(ARRAYSIZE(OSID)); ++I)
    if(OSID[I].Type == MainHeader.Host)
    {
      HostOS=I;
      break;
    }

  UnpVer=(MainHeader.VerExtract/10)*256+(MainHeader.VerExtract%10);

  return(TRUE);
}



int WINAPI _export GetArcItem(PluginPanelItem *Item, ArcItemInfo *Info)
{
PACK_PUSH(1)
  struct ACEHEADERBLOCK
  {
    WORD  CRC16;
    WORD  HeaderSize;
  } Block;
PACK_POP()
PACK_CHECK(ACEHEADERBLOCK, 1);
  HANDLE hHeap=GetProcessHeap();

  DWORD ReadSize;
  BYTE *TempBuf;

  while(1)
  {
    NextPosition=SetFilePointer(ArcHandle,NextPosition,NULL,FILE_BEGIN);
    if (NextPosition==0xFFFFFFFF)
      return(GETARC_READERROR);

    if (NextPosition>FileSize)
      return(GETARC_UNEXPEOF);

    if (!ReadFile(ArcHandle,&Block,sizeof(Block),&ReadSize,NULL))
      return(GETARC_READERROR);

    if (!ReadSize || !Block.HeaderSize) //???
      return(GETARC_EOF);

    TempBuf=(BYTE*)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, Block.HeaderSize);
    if(!TempBuf)
      return(GETARC_READERROR);

    if (!ReadFile(ArcHandle,TempBuf,Block.HeaderSize,&ReadSize,NULL))
    {
      HeapFree(hHeap,0,TempBuf);
      return(GETARC_READERROR);
    }

    if (ReadSize==0 || Block.HeaderSize != ReadSize)
    {
      HeapFree(hHeap,0,TempBuf);
      return(GETARC_EOF);
    }

#if defined(CALC_CRC)
    DWORD crc=CRC_MASK;
    crc=getcrc(crc,TempBuf,Block.HeaderSize);
    if (LOWORD(crc) != LOWORD(Block.CRC16))
    {
      HeapFree(hHeap,0,TempBuf);
      return(GETARC_BROKEN);
    }
#endif

    NextPosition+=sizeof(Block)+Block.HeaderSize;

    if(*TempBuf == 1) // File block
    {
PACK_PUSH(1)
      struct ACEHEADERFILE
      {
        BYTE  HeaderType;
        WORD  HeaderFlags;
        DWORD PackSize;
        DWORD UnpSize;
        WORD  FTime;         // File Time an Date Stamp
        WORD  FDate;         // File Time an Date Stamp
        DWORD FileAttr;
        DWORD CRC32;
        WORD  TechInfo;
        WORD  DictSize;     //????
        WORD  Reserved;
        WORD  FileNameSize;
        char  FileName[1];
      } *FileHeader;
PACK_POP()
PACK_CHECK(ACEHEADERFILE, 1);

      FileHeader=(ACEHEADERFILE*)TempBuf;
      if(FileHeader->HeaderFlags&1)
      {
        Item->FindData.dwFileAttributes=FileHeader->FileAttr;
        if(FileHeader->FileNameSize)
        {
          if(FileHeader->FileNameSize >= (WORD)sizeof(Item->FindData.cFileName))
            return(GETARC_BROKEN);
          memcpy(Item->FindData.cFileName,FileHeader->FileName,FileHeader->FileNameSize);
        }
        Item->FindData.cFileName[FileHeader->FileNameSize]=0;
        Item->FindData.nFileSizeLow=FileHeader->UnpSize;
        Item->FindData.nFileSizeHigh=0;
        Item->PackSize=FileHeader->PackSize;
        FILETIME lft;
        DosDateTimeToFileTime(FileHeader->FDate,FileHeader->FTime,&lft);
        LocalFileTimeToFileTime(&lft,&Item->FindData.ftLastWriteTime);

        Info->Solid=FileHeader->HeaderFlags&(1<<15)?1:0;
        Info->Encrypted=FileHeader->HeaderFlags&(1<<14)?1:0;
        Info->Comment=FileHeader->HeaderFlags&2?1:0;
        Info->UnpVer=UnpVer;
        //?????
        Info->DictSize=1<<FileHeader->DictSize;
        //?????
        lstrcpy(Info->HostOS,OSID[HostOS].Name);
        Item->CRC32=FileHeader->CRC32;

        NextPosition+=FileHeader->PackSize;
        break;
      }
    }
PACK_PUSH(1)
    struct ACERECORDS
    {
      BYTE  HeaderType;    // header type of recovery records is 2
      WORD  HeaderFlags;   // Bit 0   1 (RecSize field present)
      DWORD RecSize;
    } *RecHeader;
PACK_POP()
PACK_CHECK(ACERECORDS, 1);

    RecHeader=(ACERECORDS*)TempBuf;
    if(RecHeader->HeaderFlags&1)
      NextPosition+=RecHeader->RecSize;
    HeapFree(hHeap,0,TempBuf);
  }
  HeapFree(hHeap,0,TempBuf);
  return(GETARC_SUCCESS);
}


BOOL WINAPI _export CloseArchive(ArcInfo *Info)
{
  if(Info)
  {
    Info->SFXSize=SFXSize;
    Info->Volume=MainHeader.HeaderFlags&(1<<11)?1:0;
    Info->Comment=MainHeader.HeaderFlags&2?1:0;
    Info->Recovery=MainHeader.HeaderFlags&(1<<13)?1:0;
    Info->Lock=MainHeader.HeaderFlags&(1<<14)?1:0;
    Info->Flags|=MainHeader.HeaderFlags&(1<<12)?AF_AVPRESENT:0;
  }
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
    lstrcpy(FormatName,"ACE");
    lstrcpy(DefaultExt,"ace");
    return(TRUE);
  }
  return(FALSE);
}


BOOL WINAPI _export GetDefaultCommands(int Type,int Command,char *Dest)
{
  if (Type==0)
  {
    static const char *Commands[]={
    /*Extract               */"ace32 x {-p%%P} -y -c- -std {%%S} %%A @%%LN",
    /*Extract without paths */"ace32 e -av- {-p%%P} -y -c- -std {%%S} %%A @%%LN",
    /*Test                  */"ace32 t -y {-p%%P} -c- -std {%%S} %%A",
    /*Delete                */"ace32 d -y -std {-t%%W} {%%S} %%A @%%LN",
    /*Comment archive       */"ace32 cm -y -std {-t%%W} {%%S} %%A",
    /*Comment files         */"ace32 cf -y -std {-t%%W} {%%S} %%A {@%%LN}",
    /*Convert to SFX        */"ace32 s -y -std {%%S} %%A",
    /*Lock archive          */"ace32 k -y -std {%%S} %%A",
    /*Protect archive       */"ace32 rr -y -std {%%S} %%A",
    /*Recover archive       */"ace32 r -y -std {%%S} %%A",
    /*Add files             */"ace32 a -c2 -y -std {-p%%P} {-t%%W} {%%S} %%A @%%LN",
    /*Move files            */"ace32 m -c2 -y -std {-p%%P} {-t%%W} {%%S} %%A @%%LN",
    /*Add files and folders */"ace32 a -y -c2 -r -f -std {-p%%P} {-t%%W} {%%S} %%A @%%LN",
    /*Move files and folders*/"ace32 m -y -c2 -r -f -std {-p%%P} {-t%%W} {%%S} %%A @%%LN",
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
