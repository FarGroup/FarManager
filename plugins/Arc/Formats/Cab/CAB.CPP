#include <windows.h>
#include <string.h>
#include <dos.h>
#include "..\\fmt.hpp"
#include "d:\lang\bc5\far\plugin.hpp"

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

BOOL WINAPI _export IsArchive(char *Name,const unsigned char *Data,int DataSize)
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


BOOL WINAPI _export OpenArchive(char *Name,int *Type)
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
  char *EndPos=(char *)memchr(Name,0,sizeof(Name));
  if (Name==NULL)
    return(GETARC_BROKEN);
  SetFilePointer(ArcHandle,-(ReadSize-(EndPos-Name+1)),NULL,FILE_CURRENT);

  CharToOem(Name,Item->FindData.cFileName);
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


BOOL WINAPI _export GetFormatName(int Type,char *FormatName,char *DefaultExt)
{
  if (Type==0)
  {
    strcpy(FormatName,"CAB");
    strcpy(DefaultExt,"CAB");
    return(TRUE);
  }
  return(FALSE);
}


BOOL WINAPI _export GetDefaultCommands(int Type,int Command,char *Dest)
{
  if (Type==0)
  {
    static char *Commands[]={
      "extrac32 /Y %%a %%FQ512",
      "extrac32 /Y %%a %%FQ512",
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
      "",
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

