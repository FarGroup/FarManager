/*
  LZH.CPP

  Second-level plugin module for FAR Manager 1.70 and MultiArc plugin

  Copyright (c) 1996-2000 Eugene Roshal
  Copyrigth (c) 2000-2001 FAR group
*/
/* $Revision: 1.03 27.04.2001 $ */

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

static HANDLE ArcHandle;
static DWORD NextPosition,SFXSize,FileSize;
static struct OSIDType{
  BYTE Type;
  char Name[15];
} OSID[]={
  {'M',"MS-DOS"},   {'2',"OS/2"},     {'9',"OS9"},
  {'K',"OS/68K"},   {'3',"OS/386"},   {'H',"HUMAN"},
  {'U',"Unix"},     {'C',"CP/M"},     {'F',"FLEX"},
  {'m',"Mac"},      {'R',"Runser"},
};

static struct DictSizeType{
  BYTE Type[2];
  short Size;
} DictSize[]={
  {{'h','0'},0},   {{'h','1'},4},
  {{'h','2'},8},   {{'h','3'},8},
  {{'h','4'},4},   {{'h','5'},8},
  {{'z','s'},2},   {{'z','4'},0},
  {{'h','6'},32},  {{'h','7'},64},
  {{'h','d'},0},
};


BOOL WINAPI _export IsArchive(const char *Name,const unsigned char *Data,int DataSize)
{
  for (int I=2;I<DataSize-5;I++)
  {
    const unsigned char *D=Data+I;
    if (D[0]=='-' && D[1]=='l' && (D[2]=='h' || D[2]=='z') &&
        (D[3]>='0' && D[3]<='9' || D[3]=='d' || D[3]=='s' || D[3]=='d') &&
        D[4]=='-')
    {
      SFXSize=I-2;
      return(TRUE);
    }
  }
  return(FALSE);
}


BOOL WINAPI _export OpenArchive(const char *Name,int *Type)
{
  ArcHandle=CreateFile(Name,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,
                       NULL,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,NULL);
  if (ArcHandle==INVALID_HANDLE_VALUE)
    return(FALSE);
  *Type=0;
  FileSize=GetFileSize(ArcHandle,NULL);

  NextPosition=SFXSize;
  return(TRUE);
}

int WINAPI _export GetArcItem(struct PluginPanelItem *Item,struct ArcItemInfo *Info)
{
  struct LZHHd
  {
    BYTE  HeadSize;     // Header Size in Bytes
    BYTE  HeadChk;      // Header Checksum
    BYTE  HeadID[3];    // Header ID Code
    BYTE  Method;       // Compression Method
    BYTE  free1;
    DWORD PackSize;     // Packed File Size
    DWORD UnpSize;      // Original File Size not decompressed yet
    WORD FTime;         // File Time an Date Stamp
    WORD FDate;         // File Time an Date Stamp
    BYTE FAttr;         // File Attributes
    BYTE FLevel;        // level = ?
    // .....
  } LzhHeader;

  int I;
  DWORD ReadSize;
  BYTE OsId=0;
  char FileName[NM];
  char PathName[NM];
  WORD PathSize=0;
  WORD CRC16;

  NextPosition=SetFilePointer(ArcHandle,NextPosition,NULL,FILE_BEGIN);

  if (NextPosition==0xFFFFFFFF)
    return(GETARC_READERROR);

  if (NextPosition>FileSize)
    return(GETARC_UNEXPEOF);

  if (!ReadFile(ArcHandle,&LzhHeader,sizeof(LzhHeader),&ReadSize,NULL))
    return(GETARC_READERROR);

  if(ReadSize<=10 ||
      !(LzhHeader.HeadID[0]=='-' && LzhHeader.HeadID[1]=='l' &&
        (LzhHeader.HeadID[2]=='h' || LzhHeader.HeadID[2]=='z')
       )
    )
    return(GETARC_EOF);

  Item->FindData.cFileName[0]=0;
  if(LzhHeader.FLevel != 2)
  {
    BYTE FnLen;
    ReadFile(ArcHandle,&FnLen,1,&ReadSize,NULL);
    if(FnLen > 0)
      ReadFile(ArcHandle,FileName,FnLen,&ReadSize,NULL);
    FileName[FnLen]=0;
  }

  ReadFile(ArcHandle,&CRC16,2,&ReadSize,NULL); // CRC-16

  PathName[0]=0;
  if(LzhHeader.FLevel == 1 || LzhHeader.FLevel == 2)
  {
    WORD NextHeaderSize;
    BYTE ExtType, FileNameSize;

    ReadFile(ArcHandle,&OsId,1,&ReadSize,NULL); // OS ID

    do {
      ReadFile(ArcHandle,&NextHeaderSize,2,&ReadSize,NULL); // Next Header Size
      if(!NextHeaderSize)
        break;

      ReadFile(ArcHandle,&ExtType,1,&ReadSize,NULL); // Extension type

      if(ExtType == 0x01) // File name header
      {
        ReadFile(ArcHandle,FileName,NextHeaderSize-3,&ReadSize,NULL);
        FileName[NextHeaderSize-3]=0;
      }
      else if(ExtType == 0x02) // Directory name header
      {
        ReadFile(ArcHandle,PathName,NextHeaderSize-3,&ReadSize,NULL);
        PathName[PathSize=NextHeaderSize-3]=0;

        // Convert 0xFF to '\'
        for (I=0;PathName[I];I++)
          if (PathName[I]=='\xff')
            PathName[I]='\\';

        if(PathName[strlen(PathName)-1] != '\\')
          strcat(PathName,"\\");

        if(!FileName[0])
          LzhHeader.FAttr=FILE_ATTRIBUTE_DIRECTORY;

        strcpy(Item->FindData.cFileName,PathName);
      }
      else // Skip other
        SetFilePointer(ArcHandle,NextHeaderSize-3,NULL,FILE_CURRENT);

    } while(NextHeaderSize != 0);
  }

  // correct NextPosition
  DWORD PrevPosition=NextPosition;
  if(LzhHeader.FLevel == 2)
    CRC16=*(WORD*)&LzhHeader.HeadSize;
  else
    CRC16=(WORD)LzhHeader.HeadSize+2;
  NextPosition+=LzhHeader.PackSize+CRC16;

  if (PrevPosition>=NextPosition || PathSize>NM)
    return(GETARC_BROKEN);

  strcat(Item->FindData.cFileName,FileName);

  Item->FindData.dwFileAttributes=LzhHeader.FAttr & 0x3f;

  //<????>
  if(LzhHeader.Method == '0' ||
      (LzhHeader.Method == '4' && LzhHeader.HeadID[2] == 'z'))
    Item->PackSize=LzhHeader.UnpSize;
  else
    Item->PackSize=(LzhHeader.Method == 'd')?0:LzhHeader.PackSize;
  //</????>
  Item->FindData.nFileSizeLow=LzhHeader.UnpSize;

  FILETIME lft;
  DosDateTimeToFileTime(LzhHeader.FDate,LzhHeader.FTime,&lft);
  LocalFileTimeToFileTime(&lft,&Item->FindData.ftLastWriteTime);

  // OS ID - Host OS
  for(I=0; I < sizeof(OSID)/sizeof(OSID[0]); ++I)
  {
    if(OSID[I].Type == OsId)
    {
      strcpy(Info->HostOS,OSID[I].Name);
      break;
    }
  }

  // Dictionary size
  Info->DictSize=0;
  for(I=0; I < sizeof(DictSize)/sizeof(DictSize[0]); ++I)
  {
    if(DictSize[I].Type[0] == LzhHeader.HeadID[2] &&
       DictSize[I].Type[1] == LzhHeader.Method)
    {
      Info->DictSize=DictSize[I].Size;
      break;
    }
  }

  return(GETARC_SUCCESS);
}


BOOL WINAPI _export CloseArchive(struct ArcInfo *Info)
{
  if(Info)
    Info->SFXSize=SFXSize;
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
    strcpy(FormatName,"LZH");
    strcpy(DefaultExt,"LZH");
    return(TRUE);
  }
  return(FALSE);
}


BOOL WINAPI _export GetDefaultCommands(int Type,int Command,char *Dest)
{
  if (Type==0)
  {
    static char *Commands[]={
      "lha x -a -c -d -m {%%S} %%a @%%lM",
      "lha e -a -c -m {%%S} %%a @%%lM",
      "lha t -m -r2 {%%S} %%a", 
      "lha d -a -m {-w%%W} {%%S} %%a @%%lM",
      "",
      "",
      "lha s -m -x {%%S} %%a",
      "",
      "",
      "",
      "lha a -a -m {%%S} %%a @%%lM",
      "lha m -a -m {%%S} %%a @%%lM",
      "lha a -a -r -x -p -m {%%S} %%a @%%lM",
      "lha m -a -r -x -p -m {%%S} %%a @%%lM",
      "*.*"
    };
    if (Command<sizeof(Commands)/sizeof(Commands[0]))
    {
      strcpy(Dest,Commands[Command]);
      return(TRUE);
    }
  }
  return(FALSE);
}
