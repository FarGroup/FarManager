/*
  CAB.CPP

  Second-level plugin module for FAR Manager 1.70 and MultiArc plugin

  Copyright (c) 1996-2000 Eugene Roshal
  Copyrigth (c) 2000-2001 FAR group
*/
/* $Revision: 1.02 12.06.2001 $ */

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
  u1  signature[4];
  u4  reserved1;
  u4  cbCabinet;
  u4  reserved2;
  u4  coffFiles;
  u4  reserved3;
  u1  versionMinor;
  u1  versionMajor;
  u2  cFolders;
  u2  cFiles;
  u2  flags;
  u2  setID;
  u2  iCabinet;
};

static HANDLE ArcHandle;
static DWORD SFXSize,FilesNumber;
static DWORD UnpVer;

BOOL WINAPI _export IsArchive(const char *Name,const unsigned char *Data,int DataSize)
{
  for (int I=0;I<(int)(DataSize-sizeof(struct CFHEADER));I++)
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

  ArcHandle=CreateFile(Name,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,
                       NULL,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,NULL);
  if (ArcHandle==INVALID_HANDLE_VALUE)
    return(FALSE);

  *Type=0;

  SetFilePointer(ArcHandle,SFXSize,NULL,FILE_BEGIN);
  if (!ReadFile(ArcHandle,&MainHeader,sizeof(MainHeader),&ReadSize,NULL) ||
      ReadSize!=sizeof(MainHeader))
  {
    CloseHandle(ArcHandle);
    return(FALSE);
  }

  SetFilePointer(ArcHandle,SFXSize+MainHeader.coffFiles,NULL,FILE_BEGIN);
  FilesNumber=MainHeader.cFiles;
  UnpVer=MainHeader.versionMajor*256+MainHeader.versionMinor;
  /* $ 12.06.2001
     Mateusz Brsostek <mateuszb@inetia.pl>
  */
  if( FilesNumber && (MainHeader.flags & 1) )
  {
    char Name[ 276 ], *EndPos;

    if( !ReadFile( ArcHandle, Name, sizeof(Name), &ReadSize, NULL ) || ReadSize <= 16 )
    {
    blad:
      CloseHandle( ArcHandle );
      return FALSE;
    }
    if( *(WORD*)(Name + 8) == 0xFFFD || *(WORD*)(Name + 8) == 0xFFFF )
    {
      EndPos = Name + 16;
      while( *EndPos && EndPos - Name < sizeof(Name) )
        EndPos++;
      if( *EndPos )
        goto blad;

      SetFilePointer( ArcHandle, (EndPos-Name+1) - ReadSize, NULL, FILE_CURRENT );
      FilesNumber--;
    }
    else
      SetFilePointer( ArcHandle, 0 - ReadSize, NULL, FILE_CURRENT );
  }
///
  return(TRUE);
}


int WINAPI _export GetArcItem(struct PluginPanelItem *Item,struct ArcItemInfo *Info)
{
  struct CFFILE
  {
    u4  cbFile;
    u4  uoffFolderStart;
    u2  iFolder;
    u2  date;
    u2  time;
    u2  attribs;
    u1  szName[];
  } FileHeader;

  DWORD ReadSize;

  if (FilesNumber-- == 0)
    return(GETARC_EOF);
  if (!ReadFile(ArcHandle,&FileHeader,sizeof(FileHeader),&ReadSize,NULL) ||
      ReadSize!=sizeof(FileHeader))
    return(GETARC_READERROR);
  char Name[NM];
  if (!ReadFile(ArcHandle,Name,sizeof(Name),&ReadSize,NULL))
    return(GETARC_READERROR);

  char *EndPos = Name;
  while( *EndPos && EndPos - Name < sizeof(Name) )
    EndPos++;
  if( *EndPos )
    return(GETARC_BROKEN);
  SetFilePointer(ArcHandle, (EndPos-Name+1) - ReadSize, NULL,FILE_CURRENT);

  /* $ 06.05.2001
     Mateusz Brsostek <mateuszb@inetia.pl>
     supports more SFX CAB archives (for example ar500enu.exe -
     Adobe Acrobat Reader 5.0 install file)
  */
  EndPos = Name;
  while( *EndPos )
  {
    if( *EndPos == '/' )
      *EndPos = '\\';
    EndPos++;
  }

  if( Name[ 0 ] == '\\' && Name[ 1 ] != '\\' )
    EndPos = Name + 1;
  else
    EndPos = Name;

  CharToOem(EndPos,Item->FindData.cFileName);
  /* MB $ */

  Item->FindData.dwFileAttributes=FileHeader.attribs;
  Item->PackSize=0;
  Item->FindData.nFileSizeLow=FileHeader.cbFile;
  FILETIME lft;
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
    strcpy(FormatName,"CAB");
    strcpy(DefaultExt,"cab");
    return(TRUE);
  }
  return(FALSE);
}


BOOL WINAPI _export GetDefaultCommands(int Type,int Command,char *Dest)
{
  if (Type==0)
  {
    static char *Commands[]={
    /*Extract               */"MsCab {-sp%%R} x {%%S} %%A %%LMA",
    /*Extract without paths */"MsCab e {%%S} %%A %%LMA",
    /*Test                  */"MsCab t {%%S} %%A %%LMA",
    /*Delete                */"MsCab -i0 d {%%S} %%A %%LMA",
    /*Comment archive       */"",
    /*Comment files         */"",
    /*Convert to SFX        */"",
    /*Lock archive          */"",
    /*Protect archive       */"",
    /*Recover archive       */"",
    /*Add files             */"MsCab -e -i0 {-ap%%R} {%%S} a %%A %%LMA",
    /*Move files            */"MsCab -e -i0 {-ap%%R} {%%S} m %%A %%LMA",
    /*Add files and folders */"MsCab -r -e -i0 {-ap%%R} {%%S} a %%A %%LMA",
    /*Move files and folders*/"MsCab -r -e -i0 {-ap%%R} {%%S} m %%A %%LMA",
    /*"All files" mask      */"*.*"
    };
    if (Command<sizeof(Commands)/sizeof(Commands[0]))
    {
      strcpy(Dest,Commands[Command]);
      return(TRUE);
    }
  }
  return(FALSE);
}
