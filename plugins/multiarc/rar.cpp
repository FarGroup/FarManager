/*
  RAR.CPP

  Second-level plugin module for FAR Manager and MultiArc plugin

  Copyright (c) 1996 Eugene Roshal
  Copyrigth (c) 2000 FAR group
*/

#define STRICT

#include <windows.h>
#include <string.h>
#include <dos.h>
#include <CRT/crt.hpp>
#include <plugin.hpp>
#include "fmt.hpp"
#include "marclng.hpp"
#include "unrar.h"

#if defined(__BORLANDC__)
  #pragma option -a1
#elif defined(__GNUC__) || (defined(__WATCOMC__) && (__WATCOMC__ < 1100)) || defined(__LCC__)
  #pragma pack(1)
#else
  #pragma pack(push,1)
#endif

#if defined(__GNUC__)
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

#define  LHD_LARGE          0x0100
#define  LHD_UNICODE        0x0200
#define  LHD_SALT           0x0400
#define  LHD_EXTTIME        0x1000
#define  LHD_WINDOWMASK     0x00e0
#define  LHD_DIRECTORY      0x00e0
#define  LONG_BLOCK         0x8000

enum HEADER_TYPE {
  MARK_HEAD=0x72,
  MAIN_HEAD=0x73,
  FILE_HEAD=0x74,
  COMM_HEAD=0x75,
  AV_HEAD=0x76,
  SUB_HEAD=0x77,
  PROTECT_HEAD=0x78,
  SIGN_HEAD=0x79,
  NEWSUB_HEAD=0x7a,
  ENDARC_HEAD=0x7b
};

typedef HANDLE (PASCAL *RAROPENARCHIVEEX)(struct RAROpenArchiveDataEx *ArchiveData);
typedef int (PASCAL *RARCLOSEARCHIVE)(HANDLE hArcData);
typedef void (PASCAL *RARSETCALLBACK)(HANDLE hArcData,UNRARCALLBACK Callback,LPARAM UserData);
typedef int (PASCAL *RARREADHEADEREX)(HANDLE hArcData,struct RARHeaderDataEx *HeaderData);
typedef int (PASCAL *RARPROCESSFILE)(HANDLE hArcData,int Operation,char *DestPath,char *DestName);

#ifndef _WIN64
const char UnRARName[]="UNRAR.DLL";
#else
const char UnRARName[]="UNRAR64.DLL";
#endif
static const char * const RarOS[]={"DOS","OS/2","Windows","Unix","MacOS","BeOS"};

static HANDLE ArcHandle;
static DWORD NextPosition,SFXSize,FileSize,FileSizeHigh,Flags;
static long NextPositionHigh;
static int OldFormat;

static BOOL UsedUnRAR_DLL=FALSE,NeedUsedUnRAR_DLL=FALSE;
static HANDLE hArcData;
static int RHCode,PFCode;
static struct RAROpenArchiveDataEx OpenArchiveData;
static struct RARHeaderDataEx HeaderData;

static RAROPENARCHIVEEX pRAROpenArchiveEx=NULL;
static RARCLOSEARCHIVE pRARCloseArchive=NULL;
static RARSETCALLBACK pRARSetCallback=NULL;
static RARREADHEADEREX pRARReadHeaderEx=NULL;
static RARPROCESSFILE pRARProcessFile=NULL;

static char Password[NM/2];

static FARAPIINPUTBOX FarInputBox=NULL;
static FARAPIGETMSG   FarGetMsg=NULL;
static INT_PTR MainModuleNumber=-1;
static FARAPIMESSAGE FarMessage=NULL;
static FARSTDSPRINTF FarSprintf=NULL;

void UtfToWide(const char *Src,wchar_t *Dest,int DestSize);
void DecodeFileName(const char *Name,BYTE *EncName,int EncSize,wchar_t *NameW,int MaxDecSize);
#define UnicodeToOEM(src,dst,lendst)    WideCharToMultiByte(CP_OEMCP,0,(src),-1,(dst),(lendst),NULL,FALSE)
#define  Min(x,y) (((x)<(y)) ? (x):(y))


void  WINAPI SetFarInfo(const struct PluginStartupInfo *Info)
{
   FarInputBox=Info->InputBox;
   FarGetMsg=Info->GetMsg;
   MainModuleNumber=Info->ModuleNumber;
   FarMessage=Info->Message;
   FarSprintf=Info->FSF->sprintf;
}

int CALLBACK CallbackProc(UINT msg,LPARAM UserData,LPARAM P1,LPARAM P2)
{
  switch(msg)
  {
    case UCM_NEEDPASSWORD:
    {
      if(FarInputBox(FarGetMsg(MainModuleNumber,MGetPasswordTitle),
                     FarGetMsg(MainModuleNumber,MGetPassword),NULL,
                     Password,Password,sizeof(Password)-1,NULL,FIB_PASSWORD))
      {
        OemToChar(Password, Password);
        lstrcpyn((char *)P1,Password,(int)P2);
        return(0);
      }
      return 1;
    }
  }
  return(0);
}

BOOL WINAPI _export IsArchive(const char *Name,const unsigned char *Data,int DataSize)
{
  NeedUsedUnRAR_DLL=FALSE;

  for (int I=0;I<DataSize-7;I++)
  {
    const unsigned char *D=Data+I;
    if (D[0]==0x52 && D[1]==0x45 && D[2]==0x7e && D[3]==0x5e &&
        (I==0 || (DataSize>31 && Data[28]==0x52 && Data[29]==0x53 &&
        Data[30]==0x46 && Data[31]==0x58)))
    //if (D[0]==0x52 && D[1]==0x45 && D[2]==0x7e && D[3]==0x5e)
    {
      OldFormat=TRUE;
      SFXSize=I;
      return(TRUE);
    }
    // check marker block
    // The marker block is actually considered as a fixed byte sequence: 0x52 0x61 0x72 0x21 0x1a 0x07 0x00
    if (D[0]==0x52 && D[1]==0x61 && D[2]==0x72 && D[3]==0x21 &&
        D[4]==0x1a && D[5]==0x07 && D[6]==0 &&
        D[9]==0x73)                                             // next "archive header"? (Header type: 0x73)
    {
      if(D[10]&0x80)
        NeedUsedUnRAR_DLL=TRUE;
      OldFormat=FALSE;
      SFXSize=I;
      return(TRUE);
    }
  }
  return(FALSE);
}


BOOL WINAPI _export OpenArchive(const char *Name,int *Type)
{
  DWORD ReadSize;

  UsedUnRAR_DLL=FALSE;
  if(NeedUsedUnRAR_DLL)
  {
    HINSTANCE hModule=LoadLibraryEx(UnRARName,NULL,LOAD_WITH_ALTERED_SEARCH_PATH);
    if(hModule)
    {
      pRAROpenArchiveEx=(RAROPENARCHIVEEX)GetProcAddress(hModule,"RAROpenArchiveEx");
      pRARCloseArchive =(RARCLOSEARCHIVE )GetProcAddress(hModule,"RARCloseArchive");
      pRARSetCallback  =(RARSETCALLBACK  )GetProcAddress(hModule,"RARSetCallback");
      pRARReadHeaderEx =(RARREADHEADEREX )GetProcAddress(hModule,"RARReadHeaderEx");
      pRARProcessFile  =(RARPROCESSFILE  )GetProcAddress(hModule,"RARProcessFile");


      if(pRAROpenArchiveEx && pRARCloseArchive && pRARSetCallback && pRARReadHeaderEx && pRARProcessFile)
        UsedUnRAR_DLL=TRUE;
      else
        FreeLibrary(hModule);
    }
    if(!UsedUnRAR_DLL)
    {
      TCHAR ErrStr[1024];
      FarSprintf(ErrStr,FarGetMsg(MainModuleNumber,MCannotFindArchivator),UnRARName);
      LPCTSTR Msg[]={FarGetMsg(MainModuleNumber,MError),ErrStr};
      FarMessage(MainModuleNumber,FMSG_WARNING|FMSG_MB_OK,NULL,Msg,sizeof(Msg)/sizeof(*Msg),0);
      return FALSE;
    }
  }

  if(UsedUnRAR_DLL)
  {
    memset(&OpenArchiveData,0,sizeof(OpenArchiveData));
    OpenArchiveData.ArcName=(char*)Name;
    OpenArchiveData.CmtBuf=NULL;
    OpenArchiveData.CmtBufSize=0;
    OpenArchiveData.OpenMode=RAR_OM_LIST;
    hArcData=pRAROpenArchiveEx(&OpenArchiveData);
    if (OpenArchiveData.OpenResult!=0)
      return FALSE;

    Flags=OpenArchiveData.Flags;
    pRARSetCallback(hArcData,(UNRARCALLBACK)CallbackProc,0);
    HeaderData.CmtBuf=NULL;
    HeaderData.CmtBufSize=0;
    /*
    if(Flags&0x80)
    {
      if((RHCode=pRARReadHeaderEx(hArcData,&HeaderData)) != 0)
        return FALSE;
    }
    */
  }
  else
  {
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
  }
  return(TRUE);
}


int WINAPI _export GetArcItem(struct PluginPanelItem *Item,struct ArcItemInfo *Info)
{
  if(UsedUnRAR_DLL)
  {
    RHCode=pRARReadHeaderEx(hArcData,&HeaderData);
    if(!RHCode)
    {
      UnicodeToOEM(HeaderData.FileNameW,Item->FindData.cFileName,sizeof(Item->FindData.cFileName)-1);
      //lstrcpyn(Item->FindData.cFileName,HeaderData.FileName,sizeof(Item->FindData.cFileName)-1);
      Item->FindData.dwFileAttributes=HeaderData.FileAttr;
      Item->FindData.nFileSizeLow=HeaderData.UnpSize;
      Item->FindData.nFileSizeHigh=HeaderData.UnpSizeHigh;
      Item->PackSizeHigh=HeaderData.PackSizeHigh;
      Item->PackSize=HeaderData.PackSize;
      Item->CRC32=(DWORD)HeaderData.FileCRC;
      FILETIME lft;
      DosDateTimeToFileTime(HIWORD(HeaderData.FileTime),LOWORD(HeaderData.FileTime),&lft);
      LocalFileTimeToFileTime(&lft,&Item->FindData.ftLastWriteTime);

      if (HeaderData.HostOS<ArraySize(RarOS))
        lstrcpy(Info->HostOS,RarOS[HeaderData.HostOS]);
      Info->Solid=Flags & 8;
      Info->Comment=HeaderData.Flags & 8;
      Info->Encrypted=HeaderData.Flags & 4;
      Info->DictSize=64<<((HeaderData.Flags & 0x00e0)>>5);
      if (Info->DictSize > 4096)
        Info->DictSize=64;
      Info->UnpVer=(HeaderData.UnpVer/10)*256+(HeaderData.UnpVer%10);
/*
struct RARHeaderDataEx
{
  char         ArcName[1024];
  wchar_t      ArcNameW[1024];
  char         FileName[1024];
  wchar_t      FileNameW[1024];
  unsigned int Flags;
  unsigned int PackSize;
  unsigned int PackSizeHigh;
  unsigned int UnpSize;
  unsigned int UnpSizeHigh;
  unsigned int HostOS;
  unsigned int FileCRC;
  unsigned int FileTime;
  unsigned int UnpVer;
  unsigned int Method;
  unsigned int FileAttr;
  char         *CmtBuf;
  unsigned int CmtBufSize;
  unsigned int CmtSize;
  unsigned int CmtState;
  unsigned int Reserved[1024];
};
*/

      if ((PFCode=pRARProcessFile(hArcData,RAR_SKIP,NULL,NULL))!=0)
      {
        if(RHCode==ERAR_BAD_DATA)
          return GETARC_BROKEN;
        else
          return(GETARC_READERROR);
      }
      return GETARC_SUCCESS;
    }
    else
    {
      if(RHCode==ERAR_BAD_DATA)
         return GETARC_READERROR;//GETARC_BROKEN;
      return GETARC_EOF;
    }
  }

  while (1)
  {
    DWORD ReadSize;
    NextPosition=SetFilePointer(ArcHandle,NextPosition,&NextPositionHigh,FILE_BEGIN);
    if (NextPosition==0xFFFFFFFF && GetLastError()!=NO_ERROR)
      return(GETARC_READERROR);
    if ((DWORD)NextPositionHigh>FileSizeHigh || ((DWORD)NextPositionHigh==FileSizeHigh && NextPosition>FileSize))
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
      Item->CRC32=(DWORD)RarHeader.FileCRC;
      FILETIME lft;
      DosDateTimeToFileTime(HIWORD(RarHeader.FileTime),LOWORD(RarHeader.FileTime),&lft);
      LocalFileTimeToFileTime(&lft,&Item->FindData.ftLastWriteTime);
      lstrcpy(Info->HostOS,RarOS[0]);
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
      if (RarHeader.HeadType==ENDARC_HEAD)
        return GETARC_EOF;
      NextPosition+=RarHeader.HeadSize;
      if (NextPosition<RarHeader.HeadSize)
        NextPositionHigh++;
      if (RarHeader.Flags & LONG_BLOCK)
      {
        NextPosition+=RarHeader.PackSize;
        if (NextPosition<RarHeader.PackSize)
          NextPositionHigh++;
      }
      if (RarHeader.HeadSize==0)
        return(GETARC_BROKEN);
      if (RarHeader.HeadType!=FILE_HEAD)
        continue;
      DWORD PackSizeHigh=0,UnpSizeHigh=0;
      if (RarHeader.Flags & LHD_LARGE)
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

      if (RarHeader.HostOS >= 3)
        RarHeader.FileAttr=(RarHeader.Flags & LHD_WINDOWMASK) == LHD_DIRECTORY ? 0x10:0x20;

      //if (RarHeader.NameSize>sizeof(Item->FindData.cFileName)-1)
      //  return(GETARC_BROKEN);
      /*
      if (RarHeader.NameSize>sizeof(Item->FindData.cFileName)-1)
      {
        RarHeader.NameSize=sizeof(Item->FindData.cFileName)-1;
        Item->FindData.cFileName[RarHeader.NameSize]=0;
      }
      */

      char FileName[1024];
      int NameSize=Min(RarHeader.NameSize,sizeof(FileName)-1);

      if (!ReadFile(ArcHandle,FileName,NameSize,&ReadSize,NULL) || ReadSize!=RarHeader.NameSize)
        return(GETARC_READERROR);
      FileName[NameSize]=0;

      if(RarHeader.Flags&LHD_UNICODE)
      {
        wchar_t FileNameW[1024];
        int Length=lstrlen(FileName);
        if (Length == RarHeader.NameSize)
        {
          UtfToWide(FileName,FileNameW,ArraySize(FileNameW)-1);
          UnicodeToOEM(FileNameW,FileName,ArraySize(FileName)-1);
        }
        else
        {
          Length++;
          DecodeFileName(FileName,(BYTE *)FileName+Length,RarHeader.NameSize-Length,FileNameW,ArraySize(FileNameW));
          UnicodeToOEM(FileNameW,FileName,ArraySize(FileName)-1);
        }
      }

      lstrcpyn(Item->FindData.cFileName,FileName,sizeof(Item->FindData.cFileName)-1);

      Item->CRC32=RarHeader.FileCRC;
      Item->FindData.dwFileAttributes=RarHeader.FileAttr;
      Item->PackSizeHigh=PackSizeHigh;
      Item->PackSize=RarHeader.PackSize;
      Item->FindData.nFileSizeLow=RarHeader.UnpSize;
      Item->FindData.nFileSizeHigh=UnpSizeHigh;
      FILETIME lft;
      DosDateTimeToFileTime(HIWORD(RarHeader.FileTime),LOWORD(RarHeader.FileTime),&lft);
      LocalFileTimeToFileTime(&lft,&Item->FindData.ftLastWriteTime);
      if(RarHeader.Flags & LHD_EXTTIME)
      {
        // Skip Salt (8 bytes)
        if (RarHeader.Flags & LHD_SALT)
          SetFilePointer(ArcHandle,8,NULL,FILE_CURRENT);
        BYTE ExtRARTime[19], *PtrExtTime=ExtRARTime+2;
/*
mtime - define in RarHeader.FileTime
ctime
atime
arctime
                                             F0 FB         EXT_TIME
                                                   1C A3   - add 3 bytes for mtime
000040: 68                                                 /
         { 37 A1 C1 2E                                     - ctime
                       46 18 7E }                          - add 3 bytes for cmtime
                               { 39 A1 C1 2E               - atime
                                             1C A3 68 }    - add 3 bytes for cmtime
*/
        WORD RarExtTimeFlags;
        memset(ExtRARTime,0,sizeof(ExtRARTime));
        ReadFile(ArcHandle,ExtRARTime,19,&ReadSize,NULL);
        RarExtTimeFlags=*(WORD*)&ExtRARTime[0];
        for (int I=0;I<4;I++)
        {
          DWORD rmode=RarExtTimeFlags>>(3-I)*4;
          if ((rmode & 8)==0)
            continue;

          if(I)
          {
            DosDateTimeToFileTime(HIWORD(*(DWORD*)PtrExtTime),LOWORD(*(DWORD*)PtrExtTime),&lft);
            if(I == 1)
              LocalFileTimeToFileTime(&lft,&Item->FindData.ftCreationTime);
            else if(I == 2)
              LocalFileTimeToFileTime(&lft,&Item->FindData.ftLastAccessTime);
            PtrExtTime+=4;
          }
          PtrExtTime+=rmode&3;
        }
      }

      if (RarHeader.HostOS<ArraySize(RarOS))
        lstrcpy(Info->HostOS,RarOS[RarHeader.HostOS]);
      Info->Solid=Flags & 8;
      Info->Comment=RarHeader.Flags & 8;
      Info->Encrypted=RarHeader.Flags & 4;
      Info->DictSize=64<<((RarHeader.Flags & 0x00e0)>>5);
      if (Info->DictSize > 4096)
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
  Info->Lock=Flags & 4;
  Info->Recovery=Flags & 64;

  if (Flags & 32)
    Info->Flags|=AF_AVPRESENT;
  if (Flags & 0x80)
    Info->Flags|=AF_HDRENCRYPTED;

  *Password=0;

  if(UsedUnRAR_DLL)
    return pRARCloseArchive(hArcData);

  return CloseHandle(ArcHandle);
}

DWORD WINAPI _export GetSFXPos(void)
{
  return SFXSize;
}


BOOL WINAPI _export GetFormatName(int Type,char *FormatName,char *DefaultExt)
{
  if (Type==0)
  {
    lstrcpy(FormatName,"RAR");
    lstrcpy(DefaultExt,"rar");
    return(TRUE);
  }
  return(FALSE);
}



BOOL WINAPI _export GetDefaultCommands(int Type,int Command,char *Dest)
{
  if (Type==0)
  {
    // Console RAR 2.50 commands
    static const char *Commands[]={
    /*Extract               */"rar x {-p%%P} {-ap%%R} -y -c- -kb -- %%A @%%LNM",
    /*Extract without paths */"rar e {-p%%P} -y -c- -kb -- %%A @%%LNM",
    /*Test                  */"rar t -y {-p%%P} -- %%A",
    /*Delete                */"rar d -y {-w%%W} -- %%A @%%LNM",
    /*Comment archive       */"rar c -y {-w%%W} -- %%A",
    /*Comment files         */"rar cf -y {-w%%W} -- %%A @%%LNM",
    /*Convert to SFX        */"rar s -y -- %%A",
    /*Lock archive          */"rar k -y -- %%A",
    /*Protect archive       */"rar rr -y -- %%A",
    /*Recover archive       */"rar r -y -- %%A",
    /*Add files             */"rar a -y {-p%%P} {-ap%%R} {-w%%W} {%%S} -- %%A @%%LN",
    /*Move files            */"rar m -y {-p%%P} {-ap%%R} {-w%%W} {%%S} -- %%A @%%LN",
    /*Add files and folders */"rar a -r0 -y {-p%%P} {-ap%%R} {-w%%W} {%%S} -- %%A @%%LN",
    /*Move files and folders*/"rar m -r0 -y {-p%%P} {-ap%%R} {-w%%W} {%%S} -- %%A @%%LN",
    /*"All files" mask      */"*.*"
    };
    if (Command<(int)(ArraySize(Commands)))
    {
      lstrcpy(Dest,Commands[Command]);
      return(TRUE);
    }
  }
  return(FALSE);
}


void DecodeFileName(const char *Name,BYTE *EncName,int EncSize,wchar_t *NameW,int MaxDecSize)
{
  int FlagBits=0;
  BYTE Flags=0;
  int EncPos=0,DecPos=0;
  BYTE HighByte=EncName[EncPos++];

  while (EncPos < EncSize && DecPos < MaxDecSize)
  {
    if (FlagBits==0)
    {
      Flags=EncName[EncPos++];
      FlagBits=8;
    }
    switch(Flags >> 6)
    {
      case 0:
        NameW[DecPos++]=EncName[EncPos++];
        break;
      case 1:
        NameW[DecPos++]=EncName[EncPos++]+(HighByte<<8);
        break;
      case 2:
        NameW[DecPos++]=EncName[EncPos]+(EncName[EncPos+1]<<8);
        EncPos+=2;
        break;
      case 3:
        {
          int Length=EncName[EncPos++];
          if (Length & 0x80)
          {
            BYTE Correction=EncName[EncPos++];
            for (Length=(Length&0x7f)+2; Length >0 && DecPos < MaxDecSize; Length--,DecPos++)
              NameW[DecPos]=((Name[DecPos]+Correction)&0xff)+(HighByte<<8);
          }
          else
            for (Length+=2;Length>0 && DecPos<MaxDecSize;Length--,DecPos++)
              NameW[DecPos]=Name[DecPos];
        }
        break;
    }
    Flags<<=2;
    FlagBits-=2;
  }
  NameW[DecPos<MaxDecSize ? DecPos:MaxDecSize-1]=0;
}

void UtfToWide(const char *Src,wchar_t *Dest,int DestSize)
{
  DestSize--;
  while (*Src!=0)
  {
    UINT c=(BYTE)*(Src++),d;
    if (c<0x80)
      d=c;
    else
      if ((c>>5)==6)
      {
        if ((*Src&0xc0)!=0x80)
          break;
        d=((c&0x1f)<<6)|(*Src&0x3f);
        Src++;
      }
      else
        if ((c>>4)==14)
        {
          if ((Src[0]&0xc0)!=0x80 || (Src[1]&0xc0)!=0x80)
            break;
          d=((c&0xf)<<12)|((Src[0]&0x3f)<<6)|(Src[1]&0x3f);
          Src+=2;
        }
        else
          if ((c>>3)==30)
          {
            if ((Src[0]&0xc0)!=0x80 || (Src[1]&0xc0)!=0x80 || (Src[2]&0xc0)!=0x80)
              break;
            d=((c&7)<<18)|((Src[0]&0x3f)<<12)|((Src[1]&0x3f)<<6)|(Src[2]&0x3f);
            Src+=3;
          }
          else
            break;
    if (--DestSize<0)
      break;
    if (d>0xffff)
    {
      if (--DestSize<0 || d>0x10ffff)
        break;
      *(Dest++)=((d-0x10000)>>10)+0xd800;
      *(Dest++)=(d&0x3ff)+0xdc00;
    }
    else
      *(Dest++)=d;
  }
  *Dest=0;
}
