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
static int ArcComment,ArcVolume,ArcRecovery;

BOOL WINAPI _export IsArchive(const char *Name,const unsigned char *Data,int DataSize)
{
  for (int I=0;I<DataSize-11;I++)
  {
    const unsigned char *D=Data+I;
    if (D[0]==0x60 && D[1]==0xea && D[3]<0xb && D[7]<0x10 && D[10]==2)
    {
      SFXSize=I;
      return(TRUE);
    }
  }
  return(FALSE);
}


BOOL WINAPI _export OpenArchive(const char *Name,int *Type)
{
  struct ARJHd1
  {
    WORD Mark;            // header id (main and local file) = 0x60 0xEA
    WORD HeadSize;        // basic header size
    BYTE FirstHeadSize;   // size up to and including 'extra data'
    BYTE ARJVer;          // archiver version number
    BYTE ARJExtrVer;      // minimum archiver version to extract
    BYTE HostOS;
    BYTE Flags;
    BYTE SecurityVer;     // (2 = current)
    BYTE FileType;        // file type (must equal 2)
    BYTE Reserved;
    DWORD ftime;          // date time when original archive was created
    DWORD ltime;          // date time when archive was last modified
    DWORD ArcSize;        // currently used only for secured archives
    DWORD SecurityPos;    // security envelope file position
    WORD FileSpec;        // filespec position in filename

    WORD SecuritySize;    // length in bytes of security envelope data
    BYTE EncrtypVer;      // encryption version
    BYTE LastChapter;     // last chapter
                          // ... extra data
  } ArjHeader;

  DWORD ReadSize;
  WORD ARJComm,ExtHdSize;
  ArcHandle=CreateFile(Name,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,
                       NULL,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,NULL);
  if (ArcHandle==INVALID_HANDLE_VALUE)
    return(FALSE);

  *Type=0;

  FileSize=GetFileSize(ArcHandle,NULL);

  SetFilePointer(ArcHandle,SFXSize,NULL,FILE_BEGIN);
  ReadFile(ArcHandle,&ArjHeader,sizeof(ArjHeader),&ReadSize,NULL);
  SetFilePointer(ArcHandle,SFXSize+ArjHeader.HeadSize+2,NULL,FILE_BEGIN);
  ReadFile(ArcHandle,&ARJComm,sizeof(ARJComm),&ReadSize,NULL);

  ArcVolume=((ArjHeader.Flags & 4)!=0);
  ArcRecovery=((ArjHeader.Flags & 8)!=0);
  ArcComment=(ARJComm!=0);
  NextPosition=SetFilePointer(ArcHandle,4,NULL,FILE_CURRENT);
  ReadFile(ArcHandle,&ExtHdSize,sizeof(ExtHdSize),&ReadSize,NULL);
  NextPosition+=2;
  if (ExtHdSize>0)
    NextPosition+=ExtHdSize+4;
  return(TRUE);
}


int WINAPI _export GetArcItem(struct PluginPanelItem *Item,struct ArcItemInfo *Info)
{
  struct ARJHd2
  {
    WORD Mark;              // header id (main and local file) = 0x60 0xEA
    WORD HeadSize;          // basic header size
    BYTE FirstHeadSize;     // size up to and including 'extra data'
    BYTE ARJVer;            // archiver version number
    BYTE ARJExtrVer;        // minimum archiver version to extract
    BYTE HostOS;            //
    BYTE Flags;             //
    BYTE Method;            // method
    BYTE FileType;          // file type
    BYTE Reserved;          //
    DWORD ftime;            // date time modified
    DWORD PackSize;         // compressed size
    DWORD UnpSize;          // original size (this will be different for text mode compression)
    DWORD CRC;              // original file's CRC
    WORD FileSpec;          // filespec position in filename
    WORD AccessMode;        // file access mode
    //WORD HostData;        //
    BYTE FirstChapter;      // first chapter of file's lifespan
    BYTE LastChapter;       // last chapter of file's lifespan
                            // ... extra data
  } ArjHeader;

  DWORD ReadSize;

  NextPosition=SetFilePointer(ArcHandle,NextPosition,NULL,FILE_BEGIN);
  if (NextPosition==0xFFFFFFFF)
    return(GETARC_READERROR);
  if (NextPosition>FileSize)
    return(GETARC_UNEXPEOF);
  if (!ReadFile(ArcHandle,&ArjHeader,sizeof(ArjHeader),&ReadSize,NULL))
    return(GETARC_READERROR);
  if (ReadSize==0 || ArjHeader.HeadSize==0)
    return(GETARC_EOF);
  SetFilePointer(ArcHandle,ArjHeader.FirstHeadSize+NextPosition+4,NULL,FILE_BEGIN);
  char Name[NM+1];
  if (!ReadFile(ArcHandle,Name,sizeof(Name),&ReadSize,NULL) || ReadSize==0)
    return(GETARC_READERROR);
  Name[sizeof(Name)-1]=0;
  if (Name[strlen(Name)+1]!=0)
    Info->Comment=TRUE;
  strcpy(Item->FindData.cFileName,Name);

  DWORD PrevPosition=NextPosition;
  NextPosition+=8+ArjHeader.HeadSize;
  SetFilePointer(ArcHandle,NextPosition,NULL,FILE_BEGIN);
  WORD ExtHdSize;
  ReadFile(ArcHandle,&ExtHdSize,sizeof(ExtHdSize),&ReadSize,NULL);
  NextPosition+=2+ArjHeader.PackSize;
  if (ExtHdSize>0)
    NextPosition+=ExtHdSize+4;
  if (PrevPosition>=NextPosition)
    return(GETARC_BROKEN);

  if (ArjHeader.Flags & 1)
    Info->Encrypted=TRUE;
  Info->DictSize=32;

  Item->FindData.dwFileAttributes=ArjHeader.AccessMode & 0x3f;
  Item->PackSize=ArjHeader.PackSize;
  Item->FindData.nFileSizeLow=ArjHeader.UnpSize;
  FILETIME lft;
  DosDateTimeToFileTime(HIWORD(ArjHeader.ftime),LOWORD(ArjHeader.ftime),&lft);
  LocalFileTimeToFileTime(&lft,&Item->FindData.ftLastWriteTime);
  return(GETARC_SUCCESS);
}


BOOL WINAPI _export CloseArchive(struct ArcInfo *Info)
{
  Info->SFXSize=SFXSize;
  Info->Volume=ArcVolume;
  Info->Comment=ArcComment;
  Info->Recovery=ArcRecovery;
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
    strcpy(FormatName,"ARJ");
    strcpy(DefaultExt,"ARJ");
    return(TRUE);
  }
  return(FALSE);
}


BOOL WINAPI _export GetDefaultCommands(int Type,int Command,char *Dest)
{
  if (Type==0)
  {
    static char *Commands[]={
      "arj x -+ {-g%%P} -v -y -p1 {%%S} -- %%a !%%LM",
      "arj e -+ {-g%%P} -v -y -p1 {%%S} -- %%a !%%LM",
      "arj t -+ -y {-g%%P} -v -p1 {%%S} -- %%a !%%LM",
      "arj d -+ -y {-w%%W} -p {%%S} -- %%a !%%LM",
      "arj c -+ -y {-w%%W} -z -p {%%S} -- %%a",
      "arj c -+ -y {-w%%W} -p {%%S} -- %%a !%%LM",
      "arj y -+ -je -y -p %%a",
      "",
      "arj t -+ -a1 -y {-g%%P} {-w%%W} -hk -p {%%S} -- %%a",
      "arj x -+ -a1 {-g%%P} {-w%%W} -jr -p {%%S} -- %%a !%%LM",
      "arj a -+ -y -a1 {-g%%P} {-w%%W} -p {%%S} -- %%a !%%LM",
      "arj m -+ -y -a1 {-g%%P} {-w%%W} -p {%%S} -- %%a !%%LM",
      "arj a -+ -r -y -a1 {-g%%P} {-w%%W} -p {%%S} -- %%a !%%LM",
      "arj m -+ -r -y -a1 {-g%%P} {-w%%W} -p {%%S} -- %%a !%%LM",
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
