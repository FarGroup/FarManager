/*
  ACE.CPP

  Second-level plugin module for FAR Manager 1.70 and MultiArc plugin

  Copyright (c) 1996-2000 Eugene Roshal
  Copyrigth (c) 2000-2005 FAR group
*/
/* Revision: 1.14 09.04.2005 $ */

#include <windows.h>
#include <string.h>
#include <dos.h>
#include <malloc.h>
#include <stddef.h>
#include <memory.h>
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

#if defined(__GNUC__)
#include "crt.hpp"
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
#pragma comment(linker, "/merge:.data=.")
#pragma comment(linker, "/merge:.rdata=.")
#pragma comment(linker, "/merge:.text=.")
#pragma comment(linker, "/section:.,RWE")
#endif
#endif
*/

//#define CALC_CRC

static struct OSIDType{
  BYTE Type;
  char Name[15];
} OSID[]={
  {0,"MS-DOS"},   {1,"OS/2"},     {2,"Win32"},
  {3,"Unix"},     {4,"MAC-OS"},   {5,"Win NT"},
  {6,"Primos"},   {7,"APPLE GS"}, {8,"ATARI"},
  {9,"VAX VMS"},  {10,"AMIGA"},   {11,"NEXT"},
};

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

static HANDLE ArcHandle;
static DWORD NextPosition,FileSize,SFXSize;
static struct ACEHEADER MainHeader;
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
  for (int I=0;I<(int)(DataSize-sizeof(struct ACEHEADER));I++)
  {
    const unsigned char *D=Data+I;
    if (D[0]=='*' && D[1]=='*' && D[2]=='A' && D[3]=='C' && D[4]=='E' && D[5]=='*' && D[6]=='*' )
    {
      struct ACEHEADER *Header=(struct ACEHEADER *)(Data+I-7);
#if defined(CALC_CRC)
      DWORD crc=CRC_MASK;
      crc=getcrc(crc,&Header->HeaderType,Header->HeaderSize);
#endif

#ifndef offsetof
#define offsetof( s_name, m_name )  (_SIZE_T)&(((s_name _FAR *)0)->m_name)
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
  size_t I;

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

  for(I=0; I < sizeof(OSID)/sizeof(OSID[0]); ++I)
    if(OSID[I].Type == MainHeader.Host)
    {
      HostOS=I;
      break;
    }

  UnpVer=(MainHeader.VerExtract/10)*256+(MainHeader.VerExtract%10);

  return(TRUE);
}



int WINAPI _export GetArcItem(struct PluginPanelItem *Item,struct ArcItemInfo *Info)
{
  struct ACEHEADERBLOCK
  {
    WORD  CRC16;
    WORD  HeaderSize;
  } Block;
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
      FileHeader=(struct ACEHEADERFILE*)TempBuf;
      if(FileHeader->HeaderFlags&1)
      {
        Item->FindData.dwFileAttributes=FileHeader->FileAttr;
        if(FileHeader->FileNameSize)
          memcpy(Item->FindData.cFileName,FileHeader->FileName,FileHeader->FileNameSize);
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
    struct ACERECORDS
    {
      BYTE  HeaderType;    // header type of recovery records is 2
      WORD  HeaderFlags;   // Bit 0   1 (RecSize field present)
      DWORD RecSize;
    } *RecHeader;

    RecHeader=(struct ACERECORDS*)TempBuf;
    if(RecHeader->HeaderFlags&1)
      NextPosition+=RecHeader->RecSize;
    HeapFree(hHeap,0,TempBuf);
  }
  HeapFree(hHeap,0,TempBuf);
  return(GETARC_SUCCESS);
}


BOOL WINAPI _export CloseArchive(struct ArcInfo *Info)
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
    static char *Commands[]={
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
    if (Command<sizeof(Commands)/sizeof(Commands[0]))
    {
      lstrcpy(Dest,Commands[Command]);
      return(TRUE);
    }
  }
  return(FALSE);
}
