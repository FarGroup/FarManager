#include <windows.h>
#include <string.h>
#include <dos.h>
#include "plugin.hpp"
#include "fmt.hpp"

#if defined(__BORLANDC__)
  #pragma option -a1
#elif defined(__GNUC__) || (defined(__WATCOMC__) && (__WATCOMC__ < 1100)) || defined(__LCC__)
  #pragma pack(1)
#else
  #pragma pack(push,1)
#endif

static HANDLE ArcHandle;
static DWORD NextPosition,SFXSize,FileSize,FileSizeHigh,Flags;
static LONG NextPositionHigh;
static int OldFormat;

BOOL WINAPI _export IsArchive(char *Name,const unsigned char *Data,int DataSize)
{
  for (int I=0;I<DataSize-7;I++)
  {
    const unsigned char *D=Data+I;
    if (D[0]==0x52 && D[1]==0x45 && D[2]==0x7e && D[3]==0x5e)
    {
      OldFormat=TRUE;
      SFXSize=I;
      return(TRUE);
    }
    if (D[0]==0x52 && D[1]==0x61 && D[2]==0x72 && D[3]==0x21 &&
        D[4]==0x1a && D[5]==0x07 && D[6]==0)
    {
      OldFormat=FALSE;
      SFXSize=I;
      return(TRUE);
    }
  }
  return(FALSE);
}


BOOL WINAPI _export OpenArchive(char *Name,int *Type)
{
  DWORD ReadSize;

  ArcHandle=CreateFile(Name,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,
                       NULL,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,NULL);
  if (ArcHandle==INVALID_HANDLE_VALUE)
    return(FALSE);

  *Type=0;

  FileSize=GetFileSize(ArcHandle,&FileSizeHigh);

  if (OldFormat)
  {
    struct MainHeader
    {
      BYTE Mark[4];
      WORD HeadSize;
      BYTE Flags;
    } MainHeader;

    SetFilePointer(ArcHandle,SFXSize,NULL,FILE_BEGIN);
    if (!ReadFile(ArcHandle,&MainHeader,sizeof(MainHeader),&ReadSize,NULL) ||
        ReadSize!=sizeof(MainHeader))
    {
      CloseHandle(ArcHandle);
      return(FALSE);
    }
    Flags=MainHeader.Flags;
    NextPosition=SFXSize+MainHeader.HeadSize;
  }
  else
  {
    struct NewMainArchiveHeader
    {
      WORD HeadCRC;
      BYTE HeadType;
      WORD Flags;
      WORD HeadSize;
      WORD HighPosAV;
      DWORD PosAV;
    } MainHeader;
    SetFilePointer(ArcHandle,SFXSize+7,NULL,FILE_BEGIN);
    if (!ReadFile(ArcHandle,&MainHeader,sizeof(MainHeader),&ReadSize,NULL) ||
        ReadSize!=sizeof(MainHeader))
    {
      CloseHandle(ArcHandle);
      return(FALSE);
    }
    Flags=MainHeader.Flags;
    if (MainHeader.HighPosAV!=0 || MainHeader.PosAV!=0)
      Flags|=0x20;
    NextPosition=SFXSize+MainHeader.HeadSize+7;
  }
  NextPositionHigh=0;
  return(TRUE);
}


int WINAPI _export GetArcItem(struct PluginPanelItem *Item,struct ArcItemInfo *Info)
{
  while (1)
  {
    DWORD ReadSize;
    NextPosition=SetFilePointer(ArcHandle,NextPosition,&NextPositionHigh,FILE_BEGIN);
    if (NextPosition==0xFFFFFFFF && GetLastError()!=NO_ERROR)
      return(GETARC_READERROR);
    if (NextPositionHigh>FileSizeHigh || NextPositionHigh==FileSizeHigh && NextPosition>FileSize)
      return(GETARC_UNEXPEOF);
    if (OldFormat)
    {
      struct OldFileHeader
      {
        DWORD PackSize;
        DWORD UnpSize;
        WORD FileCRC;
        WORD HeadSize;
        DWORD FileTime;
        BYTE FileAttr;
        BYTE Flags;
        BYTE UnpVer;
        BYTE NameSize;
        BYTE Method;
      } RarHeader;

      if (!ReadFile(ArcHandle,&RarHeader,sizeof(RarHeader),&ReadSize,NULL))
        return(GETARC_READERROR);
      if (ReadSize==0)
        return(GETARC_EOF);
      if (!ReadFile(ArcHandle,&Item->FindData.cFileName,RarHeader.NameSize,&ReadSize,NULL) ||
          ReadSize!=RarHeader.NameSize)
        return(GETARC_READERROR);
      DWORD PrevPosition=NextPosition;
      NextPosition+=RarHeader.HeadSize+RarHeader.PackSize;
      if (PrevPosition>=NextPosition)
        return(GETARC_BROKEN);
      Item->FindData.dwFileAttributes=RarHeader.FileAttr;
      Item->PackSize=RarHeader.PackSize;
      Item->FindData.nFileSizeLow=RarHeader.UnpSize;
      FILETIME lft;
      DosDateTimeToFileTime(HIWORD(RarHeader.FileTime),LOWORD(RarHeader.FileTime),&lft);
      LocalFileTimeToFileTime(&lft,&Item->FindData.ftLastWriteTime);
      strcpy(Info->HostOS,"MS DOS");
      Info->Solid=Flags & 8;
      Info->Comment=RarHeader.Flags & 8;
      Info->Encrypted=RarHeader.Flags & 4;
      Info->DictSize=64;
      Info->UnpVer=1*256+3;
      break;
    }
    else
    {
      struct NewFileHeader
      {
        WORD HeadCRC;
        BYTE HeadType;
        WORD Flags;
        WORD HeadSize;
        DWORD PackSize;
        DWORD UnpSize;
        BYTE HostOS;
        DWORD FileCRC;
        DWORD FileTime;
        BYTE UnpVer;
        BYTE Method;
        WORD NameSize;
        DWORD FileAttr;
      } RarHeader;
      if (!ReadFile(ArcHandle,&RarHeader,sizeof(RarHeader),&ReadSize,NULL))
        return(GETARC_READERROR);
      if (ReadSize==0)
        return(GETARC_EOF);
      NextPosition+=RarHeader.HeadSize;
      if (NextPosition<RarHeader.HeadSize)
        NextPositionHigh++;
      if (RarHeader.Flags & 0x8000)
      {
        NextPosition+=RarHeader.PackSize;
        if (NextPosition<RarHeader.PackSize)
          NextPositionHigh++;
      }
      if (RarHeader.HeadSize==0)
        return(GETARC_BROKEN);
      if (RarHeader.HeadType!=0x74)
        continue;
      DWORD PackSizeHigh=0,UnpSizeHigh=0;
      if (RarHeader.Flags & 0x100)
      {
        if (!ReadFile(ArcHandle,&PackSizeHigh,4,&ReadSize,NULL))
          return(GETARC_READERROR);
        if (ReadSize==0)
          return(GETARC_EOF);
        if (!ReadFile(ArcHandle,&UnpSizeHigh,4,&ReadSize,NULL))
          return(GETARC_READERROR);
        if (ReadSize==0)
          return(GETARC_EOF);
        NextPositionHigh+=PackSizeHigh;
      }
      if (RarHeader.NameSize>sizeof(Item->FindData.cFileName)-1)
        return(GETARC_BROKEN);
      if (RarHeader.HostOS>=3)
        RarHeader.FileAttr=(RarHeader.Flags & 0x00e0)==0x00e0 ? 0x10:0x20;
      if (!ReadFile(ArcHandle,&Item->FindData.cFileName,RarHeader.NameSize,&ReadSize,NULL) ||
          ReadSize!=RarHeader.NameSize)
        return(GETARC_READERROR);
      Item->FindData.dwFileAttributes=RarHeader.FileAttr;
      Item->PackSizeHigh=PackSizeHigh;
      Item->PackSize=RarHeader.PackSize;
      Item->FindData.nFileSizeLow=RarHeader.UnpSize;
      Item->FindData.nFileSizeHigh=UnpSizeHigh;
      FILETIME lft;
      DosDateTimeToFileTime(HIWORD(RarHeader.FileTime),LOWORD(RarHeader.FileTime),&lft);
      LocalFileTimeToFileTime(&lft,&Item->FindData.ftLastWriteTime);

      static char *RarOS[]={"DOS","OS/2","Windows","Unix","MacOS","BeOS"};
      if (RarHeader.HostOS<sizeof(RarOS)/sizeof(RarOS[0]))
        strcpy(Info->HostOS,RarOS[RarHeader.HostOS]);
      Info->Solid=Flags & 8;
      Info->Comment=RarHeader.Flags & 8;
      Info->Encrypted=RarHeader.Flags & 4;
      Info->DictSize=64<<((RarHeader.Flags & 0x00e0)>>5);
      if (Info->DictSize>1024)
        Info->DictSize=64;
      Info->UnpVer=(RarHeader.UnpVer/10)*256+(RarHeader.UnpVer%10);
      break;
    }
  }
  return(GETARC_SUCCESS);
}


BOOL WINAPI _export CloseArchive(struct ArcInfo *Info)
{
  Info->SFXSize=SFXSize;
  Info->Volume=Flags & 1;
  Info->Comment=Flags & 2;
  Info->Recovery=Flags & 64;
  Info->Lock=Flags & 4;
  if (Flags & 32)
    Info->Flags|=AF_AVPRESENT;
  return(CloseHandle(ArcHandle));
}


BOOL WINAPI _export GetFormatName(int Type,char *FormatName,char *DefaultExt)
{
  if (Type==0)
  {
    strcpy(FormatName,"RAR");
    strcpy(DefaultExt,"RAR");
    return(TRUE);
  }
  return(FALSE);
}



BOOL WINAPI _export GetDefaultCommands(int Type,int Command,char *Dest)
{
  if (Type==0)
  {
    static char *Commands[]={
      "rar x {-p%%P} -y -c- %%S %%A @%%LNM",
      "rar e -av- {-p%%P} -y -c- %%S %%A @%%LNM",
      "rar t -y {-p%%P} -c- %%S %%A @%%LNM",
      "rar d -y {-w%%W} %%S %%A @%%LNM",
      "rar c -y {-w%%W} %%S %%A",
      "rar cf -y {-w%%W} %%S %%A {@%%LNM}",
      "rar s -y %%S %%A",
      "rar k -y %%S %%A",
      "rar rr -y %%S %%A",
      "rar r -y %%S %%A",
      "rar a -y {-p%%P} {-w%%W} %%S %%A @%%LN",
      "rar m -y {-p%%P} {-w%%W} %%S %%A @%%LN",
      "rar a -r0 -y {-p%%P} {-w%%W} %%S %%A @%%LN",
      "rar m -r0 -y {-p%%P} {-w%%W} %%S %%A @%%LN",
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
