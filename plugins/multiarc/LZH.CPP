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

BOOL WINAPI _export IsArchive(char *Name,const unsigned char *Data,int DataSize)
{
  for (int I=2;I<DataSize-5;I++)
  {
    const unsigned char *D=Data+I;
    if (D[0]=='-' && D[1]=='l' && D[2]=='h' && D[3]>='0' && D[3]<='9' &&
        D[4]=='-')
    {
      SFXSize=I-2;
      return(TRUE);
    }
  }
  return(FALSE);
}


BOOL WINAPI _export OpenArchive(char *Name,int *Type)
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
    BYTE HeadSize;
    BYTE Sum;
    BYTE Mark[5];
    DWORD PackSize;
    DWORD UnpSize;
    DWORD ftime;
    WORD Attr;
    BYTE NameSize;
  } LzhHeader;
  DWORD ReadSize;
  char FileName[NM];

  NextPosition=SetFilePointer(ArcHandle,NextPosition,NULL,FILE_BEGIN);
  if (NextPosition==0xFFFFFFFF)
    return(GETARC_READERROR);
  if (NextPosition>FileSize)
    return(GETARC_UNEXPEOF);
  if (!ReadFile(ArcHandle,&LzhHeader,sizeof(LzhHeader),&ReadSize,NULL))
    return(GETARC_READERROR);
  if (ReadSize<=10 || memcmp(LzhHeader.Mark,"-lh",3)!=0)
    return(GETARC_EOF);
  ReadFile(ArcHandle,FileName,LzhHeader.NameSize,&ReadSize,NULL);
  FileName[LzhHeader.NameSize]=0;

  WORD PathSize=0;
  if (LzhHeader.HeadSize-sizeof(LzhHeader)-LzhHeader.NameSize > 2)
  {
    BYTE Buf[3];
    SetFilePointer(ArcHandle,3,NULL,FILE_CURRENT);
    ReadFile(ArcHandle,Buf,sizeof(Buf),&ReadSize,NULL);
    if (Buf[2]==2)
      PathSize=Buf[0]+256*Buf[1];
  }

  DWORD PrevPosition=NextPosition;
  NextPosition+=LzhHeader.HeadSize+LzhHeader.PackSize+2;
  if (PrevPosition>=NextPosition || PathSize>NM)
    return(GETARC_BROKEN);
  if (PathSize>=4)
  {
    ReadFile(ArcHandle,Item->FindData.cFileName,PathSize-4,&ReadSize,NULL);
    for (int I=0;Item->FindData.cFileName[I];I++)
      if (Item->FindData.cFileName[I]=='\xff')
        Item->FindData.cFileName[I]='\\';
    strcat(Item->FindData.cFileName,"\\");
  }
  strcat(Item->FindData.cFileName,FileName);

  Item->FindData.dwFileAttributes=LzhHeader.Attr & 0x3f;
  Item->PackSize=LzhHeader.PackSize;
  Item->FindData.nFileSizeLow=LzhHeader.UnpSize;
  FILETIME lft;
  DosDateTimeToFileTime(HIWORD(LzhHeader.ftime),LOWORD(LzhHeader.ftime),&lft);
  LocalFileTimeToFileTime(&lft,&Item->FindData.ftLastWriteTime);
  Info->DictSize=8;
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
      "lha x -a -c -d -m %%a @%%lM",
      "lha e -a -c -m %%a @%%lM",
      "lha t -a -m %%a @%%lM",
      "lha d -a -m {-w%%W} %%a @%%lM",
      "",
      "",
      "lha s -a -m %%a",
      "",
      "",
      "",
      "lha a -a -m %%A @%%lM",
      "lha m -a -m %%A @%%lM",
      "lha a -a -r -x -p -m %%A @%%lM",
      "lha m -a -r -x -p -m %%A @%%lM",
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

