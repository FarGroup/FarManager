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
#include <plugin.hpp>
#include "fmt.hpp"
#include "marclng.hpp"
#include "unrar.h"

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

typedef HANDLE (PASCAL *RAROPENARCHIVEEX)(RAROpenArchiveDataEx *ArchiveData);
typedef int (PASCAL *RARCLOSEARCHIVE)(HANDLE hArcData);
typedef void (PASCAL *RARSETCALLBACK)(HANDLE hArcData,UNRARCALLBACK Callback,LPARAM UserData);
typedef int (PASCAL *RARREADHEADEREX)(HANDLE hArcData,RARHeaderDataEx *HeaderData);
typedef int (PASCAL *RARPROCESSFILE)(HANDLE hArcData,int Operation,char *DestPath,char *DestName);

#ifndef _WIN64
const char UnRARName[]="UNRAR.DLL";
#else
const char UnRARName[]="UNRAR64.DLL";
#endif
static const char * const RarOS[]={"DOS","OS/2","Windows","Unix","MacOS","BeOS"};

static HANDLE ArcHandle;
static DWORD SFXSize,Flags;
static int RarFormat=15;
static bool MissingVolume;

static BOOL UsedUnRAR_DLL=FALSE;
static HANDLE hArcData;
static int RHCode,PFCode;
static RAROpenArchiveDataEx OpenArchiveData;
static RARHeaderDataEx HeaderData;

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

static char ModuleName[NM+50];

void UtfToWide(const char *Src,wchar_t *Dest,int DestSize);
void DecodeFileName(const char *Name,BYTE *EncName,int EncSize,wchar_t *NameW,int MaxDecSize);
#define UnicodeToOEM(src,dst,lendst)    WideCharToMultiByte(CP_OEMCP,0,(src),-1,(dst),(lendst),NULL,FALSE)
#define  Min(x,y) (((x)<(y)) ? (x):(y))


void  WINAPI SetFarInfo(const PluginStartupInfo *Info)
{
   FarInputBox=Info->InputBox;
   FarGetMsg=Info->GetMsg;
   MainModuleNumber=Info->ModuleNumber;
   FarMessage=Info->Message;
   FarSprintf=Info->FSF->sprintf;
   lstrcpy(ModuleName,Info->ModuleName);
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
        return 1;
      }
      return -1;
    }
    case UCM_CHANGEVOLUMEW: // Do not hang if volume is missing.
    {
      if (P2==RAR_VOL_ASK)
      {
        LPCTSTR Msg[]={FarGetMsg(MainModuleNumber,MError),"Volume is missing"};
        FarMessage(MainModuleNumber,FMSG_WARNING|FMSG_MB_OK,NULL,Msg,sizeof(Msg)/sizeof(*Msg),0);
        MissingVolume=true;
        return -1;
      }
    }
  }
  return 0;
}

BOOL WINAPI _export IsArchive(const char *Name,const unsigned char *Data,int DataSize)
{
  for (int I=0;I<DataSize-9;I++)
  {
    const unsigned char *D=Data+I;
    if (D[0]==0x52 && D[1]==0x45 && D[2]==0x7e && D[3]==0x5e &&
        (I==0 || (DataSize>31 && Data[28]==0x52 && Data[29]==0x53 &&
        Data[30]==0x46 && Data[31]==0x58)))
    {
      RarFormat = 14; // RAR 1.4 archive format.
      SFXSize=I;
      return TRUE;
    }
    // check marker block
    // The marker block is actually considered as a fixed byte sequence: 0x52 0x61 0x72 0x21 0x1a 0x07 0x00
    if (D[0]==0x52 && D[1]==0x61 && D[2]==0x72 && D[3]==0x21 &&
        D[4]==0x1a && D[5]==0x07 &&
        ((D[6]==0 && D[9]==0x73) || // RAR 1.5 signature followed by main archive header (Header type: 0x73)
         (D[6]==1 && D[7]==0)))     // RAR 5.0 signature.
    {
      RarFormat=D[6]==0 ? 15 : 50; // RAR 1.5 or 5.0 archive format.
      SFXSize=I;
      return TRUE;
    }
  }
  return FALSE;
}


BOOL WINAPI _export OpenArchive(const char *Name,int *Type)
{
  UsedUnRAR_DLL=FALSE;
  MissingVolume=false;

  // We attempt to load unrar.dll first from "Plugins\MultiArc\Formats"
  // and then from root FAR folder. We use absolute paths for security reason.
  char DllName[NM+50];
  lstrcpy(DllName,ModuleName);
  char *NamePtr=strrchr(DllName,'\\');
  NamePtr=(NamePtr==NULL) ? DllName:NamePtr+1;
  lstrcpy(NamePtr,"Formats\\");
  lstrcat(NamePtr,UnRARName);

  HINSTANCE hModule=LoadLibraryEx(DllName,NULL,LOAD_WITH_ALTERED_SEARCH_PATH);
  if (hModule==NULL)
  {
    strcpy(NamePtr,UnRARName);
    hModule=LoadLibraryEx(DllName,NULL,LOAD_WITH_ALTERED_SEARCH_PATH);
  }

  if (hModule!=NULL)
  {
    pRAROpenArchiveEx=(RAROPENARCHIVEEX)GetProcAddress(hModule,"RAROpenArchiveEx");
    pRARCloseArchive =(RARCLOSEARCHIVE )GetProcAddress(hModule,"RARCloseArchive");
    pRARSetCallback  =(RARSETCALLBACK  )GetProcAddress(hModule,"RARSetCallback");
    pRARReadHeaderEx =(RARREADHEADEREX )GetProcAddress(hModule,"RARReadHeaderEx");
    pRARProcessFile  =(RARPROCESSFILE  )GetProcAddress(hModule,"RARProcessFile");

    if (pRAROpenArchiveEx && pRARCloseArchive && pRARSetCallback && pRARReadHeaderEx && pRARProcessFile)
      UsedUnRAR_DLL=TRUE;
    else
      FreeLibrary(hModule);
  }

  if (!UsedUnRAR_DLL)
  {
    TCHAR ErrStr[1024];
    FarSprintf(ErrStr,FarGetMsg(MainModuleNumber,MCannotFindArchivator),UnRARName);
    LPCTSTR Msg[]={FarGetMsg(MainModuleNumber,MError),ErrStr};
    FarMessage(MainModuleNumber,FMSG_WARNING|FMSG_MB_OK,NULL,Msg,sizeof(Msg)/sizeof(*Msg),0);
    return FALSE;
  }

  memset(&OpenArchiveData,0,sizeof(OpenArchiveData));
  OpenArchiveData.ArcName=(char*)Name;
  OpenArchiveData.OpenMode=RAR_OM_LIST;
  OpenArchiveData.Callback=CallbackProc;
  hArcData=pRAROpenArchiveEx(&OpenArchiveData);
  if (OpenArchiveData.OpenResult!=0)
    return FALSE;

  Flags=OpenArchiveData.Flags;
  memset(&HeaderData,0,sizeof(HeaderData));
  return TRUE;
}


int WINAPI _export GetArcItem(PluginPanelItem *Item, ArcItemInfo *Info)
{
  RHCode=pRARReadHeaderEx(hArcData,&HeaderData);
  if (RHCode!=0)
    return RHCode==ERAR_BAD_DATA && !MissingVolume ? GETARC_BROKEN : GETARC_EOF;

  UnicodeToOEM(HeaderData.FileNameW,Item->FindData.cFileName,sizeof(Item->FindData.cFileName)-1);
  //lstrcpyn(Item->FindData.cFileName,HeaderData.FileName,sizeof(Item->FindData.cFileName)-1);

  if (HeaderData.HostOS==3) // Unix.
    Item->FindData.dwFileAttributes=(HeaderData.Flags & RHDF_DIRECTORY)!=0 ? 0x10:0;
  else
    Item->FindData.dwFileAttributes=HeaderData.FileAttr;
  Item->FindData.nFileSizeLow=HeaderData.UnpSize;
  Item->FindData.nFileSizeHigh=HeaderData.UnpSizeHigh;
  Item->PackSizeHigh=HeaderData.PackSizeHigh;
  Item->PackSize=HeaderData.PackSize;
  Item->CRC32=(DWORD)HeaderData.FileCRC;
  FILETIME lft;
  DosDateTimeToFileTime(HIWORD(HeaderData.FileTime),LOWORD(HeaderData.FileTime),&lft);
  LocalFileTimeToFileTime(&lft,&Item->FindData.ftLastWriteTime);

  static const char* const RarOS[]={"DOS","OS/2","Windows","Unix"};
  if (HeaderData.HostOS<ARRAYSIZE(RarOS))
    lstrcpy(Info->HostOS,RarOS[HeaderData.HostOS]);
  Info->Solid=Flags & 8;
  Info->Comment=HeaderData.Flags & 8;
  Info->Encrypted=HeaderData.Flags & 4;
  Info->DictSize=HeaderData.DictSize;
  Info->UnpVer=(HeaderData.UnpVer/10)*256+(HeaderData.UnpVer%10);

  if ((PFCode=pRARProcessFile(hArcData,RAR_SKIP,NULL,NULL))!=0)
  {
    if (MissingVolume)
      return GETARC_EOF;
    return RHCode==ERAR_BAD_DATA ? GETARC_BROKEN : GETARC_READERROR;
  }

  return GETARC_SUCCESS;
}


BOOL WINAPI _export CloseArchive(ArcInfo *Info)
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

  if (UsedUnRAR_DLL)
    return pRARCloseArchive(hArcData);

  return CloseHandle(ArcHandle);
}

DWORD WINAPI _export GetSFXPos()
{
  return SFXSize;
}


BOOL WINAPI _export GetFormatName(int Type,char *FormatName,char *DefaultExt)
{
  if (Type==0)
  {
// Plugin "Save settings" does not work well if we return different format names.
//    lstrcpy(FormatName,RarFormat==14 ? "RAR1.4":(RarFormat==15 ? "RAR4":"RAR5"));
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
    // Console RAR 5.0 commands
    static const char *Commands[]={
    /*Extract               */"rar x {-p%%P} {-ap%%R} -y -c- -kb -- %%A @%%LNMA",
    /*Extract without paths */"rar e {-p%%P} -y -c- -kb -- %%A @%%LNMA",
    /*Test                  */"rar t -y {-p%%P} -- %%A",
    /*Delete                */"rar d -y {-w%%W} -- %%A @%%LNMA",
    /*Comment archive       */"rar c -y {-w%%W} -- %%A",
    /*Comment files         */"",
    /*Convert to SFX        */"rar s -y -- %%A",
    /*Lock archive          */"rar k -y -- %%A",
    /*Protect archive       */"rar rr -y -- %%A",
    /*Recover archive       */"rar r -y -- %%A",
    /*Add files             */"rar a -y {-p%%P} {-ap%%R} {-w%%W} {%%S} -- %%A @%%LNA",
    /*Move files            */"rar m -y {-p%%P} {-ap%%R} {-w%%W} {%%S} -- %%A @%%LNA",
    /*Add files and folders */"rar a -r0 -y {-p%%P} {-ap%%R} {-w%%W} {%%S} -- %%A @%%LNA",
    /*Move files and folders*/"rar m -r0 -y {-p%%P} {-ap%%R} {-w%%W} {%%S} -- %%A @%%LNA",
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
