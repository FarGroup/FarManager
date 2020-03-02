/*
  HA.CPP

  Second-level plugin module for FAR Manager and MultiArc plugin

  Copyright (c) 1996 Eugene Roshal
  Copyrigth (c) 2000 FAR group
*/

#include <windows.h>
#include <string.h>
#include <dos.h>
#include <plugin.hpp>
#include "fmt.hpp"

static HANDLE ArcHandle;
static DWORD NextPosition,FileSize;


// Number of 100 nanosecond units from 01.01.1601 to 01.01.1970
#define EPOCH_BIAS    116444736000000000ll

void WINAPI UnixTimeToFileTime( DWORD time, FILETIME * ft )
{
  *(__int64*)ft = EPOCH_BIAS + time * 10000000ll;
}

void  WINAPI _export SetFarInfo(const PluginStartupInfo *Info)
{
  ;
}

BOOL WINAPI _export IsArchive(const char *Name,const unsigned char *Data,int DataSize)
{
  if (DataSize<26 || Data[0]!='H' || Data[1]!='A' || Data[3]>32)
    return(FALSE);
  int Type=Data[4] & 0xf;
  if ((Type>2 && Type<14) || Data[4]>0x2f)
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

  NextPosition=4;
  return(TRUE);
}


int WINAPI _export GetArcItem(PluginPanelItem *Item, ArcItemInfo *Info)
{
PACK_PUSH(1)
  struct HaHeader
  {
    BYTE Type;
    DWORD PackSize;
    DWORD UnpSize;
    DWORD CRC;
    DWORD FileTime;
  } Header;
PACK_POP()
PACK_CHECK(HaHeader, 1);

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
  int PathLength=lstrlen(Path)+1;
  lstrcpyn(Name,Path+PathLength,sizeof(Name));
  int Length=PathLength+lstrlen(Name)+1;
  DWORD PrevPosition=NextPosition;
  NextPosition+=sizeof(Header)+Length+Path[Length]+1+Header.PackSize;
  if (PrevPosition>=NextPosition)
    return(GETARC_BROKEN);
  char *EndSym=strrchr(Path,255);
  if (EndSym!=NULL)
    *EndSym=0;
  if (*Path)
    lstrcat(Path,"\\");
  lstrcat(Path,Name);
  for (int I=0;Path[I]!=0;I++)
    if ((unsigned char)Path[I]==0xff)
      Path[I]='\\';
  lstrcpyn(Item->FindData.cFileName,Path,sizeof(Item->FindData.cFileName));
  Item->FindData.dwFileAttributes=(Header.Type & 0xf)==0xe ? FILE_ATTRIBUTE_DIRECTORY:0;
  Item->CRC32=Header.CRC;
  UnixTimeToFileTime(Header.FileTime,&Item->FindData.ftLastWriteTime);
  Item->FindData.nFileSizeLow=Header.UnpSize;
  Item->FindData.nFileSizeHigh=0;
  Item->PackSize=Header.PackSize;
  return(GETARC_SUCCESS);
}


BOOL WINAPI _export CloseArchive(ArcInfo *Info)
{
  return(CloseHandle(ArcHandle));
}

BOOL WINAPI _export GetFormatName(int Type,char *FormatName,char *DefaultExt)
{
  if (Type==0)
  {
    lstrcpy(FormatName,"HA");
    lstrcpy(DefaultExt,"ha");
    return(TRUE);
  }
  return(FALSE);
}


BOOL WINAPI _export GetDefaultCommands(int Type,int Command,char *Dest)
{
  if (Type==0)
  {
    static const char *Commands[]={
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
    if (Command<(int)(ARRAYSIZE(Commands)))
    {
      lstrcpy(Dest,Commands[Command]);
      return(TRUE);
    }
  }
  return(FALSE);
}
