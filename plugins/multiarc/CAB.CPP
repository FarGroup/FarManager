/*
  CAB.CPP

  Second-level plugin module for FAR Manager 1.70 and MultiArc plugin

  Copyright (c) 1996-2000 Eugene Roshal
  Copyrigth (c) 2000-2002 FAR group
*/
/* $Revision: 1.05 31.01.2002 $ */

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

typedef BYTE u1;
typedef WORD u2;
typedef DWORD u4;

struct CFHEADER
{
  u1 signature[4];
  u4 reserved1;
  u4 cbCabinet;
  u4 reserved2;
  u4 coffFiles;
  u4 reserved3;
  u1 versionMinor;
  u1 versionMajor;
  u2 cFolders;
  u2 cFiles;
  u2 flags;
  u2 setID;
  u2 iCabinet;
};

struct CFFILE
{
  u4 cbFile;
  u4 uoffFolderStart;
  u2 iFolder;
  u2 date;
  u2 time;
  u2 attribs;
  u1 szName[256];
};

static HANDLE ArcHandle;
static DWORD SFXSize,FilesNumber;
static DWORD UnpVer;

BOOL WINAPI _export IsArchive(const char *Name,const unsigned char *Data,int DataSize)
{
  int I;

  for( I=0; I <= (int)(DataSize-sizeof(struct CFHEADER)); I++ )
  {
    const unsigned char *D=Data+I;
    if (D[0]=='M' && D[1]=='S' && D[2]=='C' && D[3]=='F')
    {
      struct CFHEADER *Header=(struct CFHEADER *)(Data+I);
      if (Header->cbCabinet>sizeof(Header) && Header->coffFiles>sizeof(Header) &&
          Header->coffFiles<0xffff && Header->versionMajor>0 &&
          Header->versionMajor<0x10 && Header->cFolders>0)
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
  struct CFHEADER MainHeader;
  DWORD ReadSize;
  int I;

  ArcHandle = CreateFile( Name, GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,
              NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL );
  if (ArcHandle == INVALID_HANDLE_VALUE)
    return FALSE;

  *Type=0;

  SetFilePointer(ArcHandle,SFXSize,NULL,FILE_BEGIN);
  I = SFXSize;
  if (!ReadFile( ArcHandle, &MainHeader, sizeof(MainHeader), &ReadSize, NULL ) ||
    ReadSize != sizeof(MainHeader) || !IsArchive( NULL, (u1*)&MainHeader, sizeof(MainHeader) ))
  {
    HANDLE hMapFile;
    LPBYTE Data;

    if ((ReadSize = GetFileSize( ArcHandle, NULL )) == 0xFFFFFFFF)
    {
    blad:
      CloseHandle( ArcHandle );
      return FALSE;
    }
    if (ReadSize > 1024*1024)
      ReadSize = 1024*1024;
    hMapFile = CreateFileMapping( ArcHandle, NULL, PAGE_READONLY, 0, ReadSize, NULL );
    if (hMapFile == NULL)
      goto blad;
    Data = (LPBYTE)MapViewOfFile( hMapFile, FILE_MAP_READ, 0, 0, ReadSize );
    CloseHandle( hMapFile );
    if (Data == NULL)
      goto blad;
    I = IsArchive( NULL, Data, ReadSize );
    if (I)
      memcpy( &MainHeader, Data + SFXSize, sizeof(MainHeader) );
    UnmapViewOfFile( Data );
    if (I == 0)
      goto blad;
  }
  else
    SFXSize = I;

  SetFilePointer(ArcHandle,SFXSize+MainHeader.coffFiles,NULL,FILE_BEGIN);
  FilesNumber=MainHeader.cFiles;
  UnpVer=MainHeader.versionMajor*256+MainHeader.versionMinor;

  if( FilesNumber && (MainHeader.flags & 1) )
  {
    char *EndPos;
    struct CFFILE FileHeader;

    if (!ReadFile( ArcHandle, &FileHeader, sizeof(FileHeader), &ReadSize, NULL )
        || ReadSize < 18)
      goto blad;
    if (FileHeader.iFolder == 0xFFFD || FileHeader.iFolder == 0xFFFF)
    {
      EndPos = (char*)FileHeader.szName;
      while (EndPos - (char*)&FileHeader < sizeof(FileHeader) && *EndPos)
        EndPos++;
      if (EndPos - (char*)&FileHeader >= sizeof(FileHeader))
        goto blad;

      SetFilePointer( ArcHandle, (EndPos-(char*)&FileHeader+1) - ReadSize, NULL, FILE_CURRENT );
      FilesNumber--;
    }
    else
    {
      SetFilePointer( ArcHandle, 0 - ReadSize, NULL, FILE_CURRENT );
    }
  }
///
  return(TRUE);
}


int WINAPI _export GetArcItem(struct PluginPanelItem *Item,struct ArcItemInfo *Info)
{
  struct CFFILE FileHeader;

  DWORD ReadSize;
  char *EndPos;
  FILETIME lft;

  if (FilesNumber-- == 0)
    return GETARC_EOF;
  if (!ReadFile(ArcHandle,&FileHeader,sizeof(FileHeader),&ReadSize,NULL)
      || ReadSize < 18)
    return GETARC_READERROR;

  EndPos = (char *)FileHeader.szName;
  while (EndPos - (char*)&FileHeader < sizeof(FileHeader) && *EndPos)
    EndPos++;
  if (EndPos - (char*)&FileHeader >= sizeof(FileHeader))
    return GETARC_BROKEN;

  SetFilePointer( ArcHandle, (EndPos-(char*)&FileHeader+1) - ReadSize, NULL, FILE_CURRENT );

  EndPos = (char *)FileHeader.szName;
  while (*EndPos)
  {
    if (*EndPos == '/')
      *EndPos = '\\';
    EndPos++;
  }

  EndPos = (char *)FileHeader.szName;
  if (EndPos[ 0 ] == '\\' && EndPos[ 1 ] != '\\')
    EndPos++;

  CharToOem( EndPos, Item->FindData.cFileName );

  Item->FindData.dwFileAttributes=FileHeader.attribs;
  Item->PackSize=0;
  Item->FindData.nFileSizeLow=FileHeader.cbFile;
  DosDateTimeToFileTime(FileHeader.date,FileHeader.time,&lft);
  LocalFileTimeToFileTime(&lft,&Item->FindData.ftLastWriteTime);
  Info->UnpVer=UnpVer;
  return(GETARC_SUCCESS);
}


BOOL WINAPI _export CloseArchive(struct ArcInfo *Info)
{
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
    lstrcpy(FormatName,"CAB");
    lstrcpy(DefaultExt,"cab");
    return(TRUE);
  }
  return(FALSE);
}


BOOL WINAPI _export GetDefaultCommands(int Type,int Command,char *Dest)
{
  if (Type==0)
  {
    static char *Commands[]={
    /*Extract               */"MsCab -i0 {-ap%%R} {%%S} x %%A @%%LMA",
    /*Extract without paths */"MsCab -i0 {%%S} e %%A @%%LMA",
    /*Test                  */"MsCab -i0 {%%S} t %%A @%%LMA",
    /*Delete                */"MsCab -i0 {%%S} d %%A @%%LMA",
    /*Comment archive       */"",
    /*Comment files         */"",
    /*Convert to SFX        */"MsCab {%%S} s %%A",
    /*Lock archive          */"",
    /*Protect archive       */"",
    /*Recover archive       */"",
    /*Add files             */"MsCab -i0 {-ap%%R} {%%S} a %%A @%%LMA",
    /*Move files            */"MsCab -i0 {-ap%%R} {%%S} m %%A @%%LMA",
    /*Add files and folders */"MsCab -r -i0 {-ap%%R} {%%S} a %%A @%%LMA",
    /*Move files and folders*/"MsCab -r -i0 {-ap%%R} {%%S} m %%A @%%LMA",
    /*"All files" mask      */"*.*"
    };
    if (Command < sizeof(Commands)/sizeof(Commands[0]))
    {
      lstrcpy(Dest,Commands[Command]);
      return(TRUE);
    }
  }
  return(FALSE);
}
