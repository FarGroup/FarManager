#include <Rtl.Base.h>
#include <FarPluginBase.hpp>
#include <string.h>
#include <dos.h>
#include "../../module.hpp"
#include "Archive.hpp"
#include "ArchiveGzip.hpp"
#include "detect.hpp"
#include "fileutils.hpp"
#include "FarZlib.hpp"

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

#ifdef _MSC_VER
#pragma comment(linker, "-subsystem:console")
#pragma comment(linker, "/ignore:4078")
#pragma comment(linker, "-merge:.rdata=.text")
#endif

enum
{
  INVALID_FORMAT=-1,
  TAR_FORMAT=0, //basic formats
  CPIO_FORMAT,
  GZ_FORMAT,
  BZ_FORMAT,
  Z_FORMAT,
  TGZ_FORMAT, //zlib formats
  CPZ_FORMAT,
  RPM_FORMAT,
  TBZ2_FORMAT, //bz2lib formats
  CPBZ_FORMAT
};

PluginStartupInfo Info;
FARSTANDARDFUNCTIONS FSF;

DWORD NextPosition,SFXSize,FileSize;
int timezone=0;
int ArcType;

//ArchiveBase *Archive;

static int MyIsArchive(const char *Name,const unsigned char *Data,int DataSize)
{
  int res=INVALID_FORMAT;
  unsigned char DataIn[512];
  if(IsTarHeader(Data,DataSize))
  {
    res=TAR_FORMAT;
  }
  else if(IsCpioHeader(Data,DataSize))
  {
    res=CPIO_FORMAT;
  }
  else if(DataSize>1)
  {
    if(Data[0]==0x1f&&Data[1]==0x8b)
    {
      res=GZ_FORMAT;
      if(ZlibOk)
      {
        ArchiveDetect *detect=new TarGzip<ArchiveDetect>(Name);
        if(detect->Read(DataIn,sizeof(DataIn)))
        {
          if(IsTarHeader(DataIn,sizeof(DataIn)))
            res=TGZ_FORMAT;
          else if(IsCpioHeader(DataIn,sizeof(DataIn)))
            res=CPZ_FORMAT;
        }
        delete detect;
      }
    }
    else if(Data[0]==0x1f&&Data[1]==0x9d)
    {
      res=Z_FORMAT;
    }
    else if(Data[0]=='B'&&Data[1]=='Z')
    {
      res=BZ_FORMAT;
      if(BlibOk)
      {
        ArchiveDetect *detect=new TarBzip<ArchiveDetect>(Name);
        if(detect->Read(DataIn,sizeof(DataIn)))
        {
          if(IsTarHeader(DataIn,sizeof(DataIn)))
            res=TBZ2_FORMAT;
          else if(IsCpioHeader(DataIn,sizeof(DataIn)))
            res=CPBZ_FORMAT;
        }
        delete detect;
      }
    }
  }
  return res;
}

static ArchiveBase *CreateArchive(const char *Name,int Type)
{
  switch(Type)
  {
    case TAR_FORMAT:
      return new TarPlain<TarBase>(Name);
    case CPIO_FORMAT:
      return new TarPlain<CpioBase>(Name);
    case GZ_FORMAT:
      return new ArchiveGzip(Name);
    case BZ_FORMAT:
      return new ArchiveOne(Name);
    case Z_FORMAT:
      return new ArchiveOne(Name);
    case TGZ_FORMAT:
      return new TarGzip<TarBase>(Name);
    case CPZ_FORMAT:
      return new TarGzip<CpioBase>(Name);
//    case RPM_FORMAT:
    case TBZ2_FORMAT:
      return new TarBzip<TarBase>(Name);
    case CPBZ_FORMAT:
      return new TarBzip<CpioBase>(Name);
  }
  return NULL;
}

/*
HANDLE __stdcall QueryArchive (
		const char *lpFileName,
		const char *lpBuffer,
		dword dwBufferSize
		)
{
  SFXSize=0;
  ArcType=MyIsArchive(lpFileName,lpBuffer,dwBufferSize);
  if(ArcType==INVALID_FORMAT) return INVALID_HANDLE_VALUE;
  return (HANDLE)CreateArchive(lpFileName,ArcType);
}

BOOL WINAPI _export OpenArchive(const char *Name,int *Type)
{
  *Type=ArcType;
  Archive=CreateArchive(Name,*Type);
  return(TRUE);
}

DWORD WINAPI _export GetSFXPos(void)
{
  return SFXSize;
}

int WINAPI _export GetArcItem(struct PluginPanelItem *Item,struct ArcItemInfo *Info)
{
  return Archive->Next(Item);
}

BOOL WINAPI _export CloseArchive(struct ArcInfo *Info)
{
  delete Archive;
  return TRUE;
}

BOOL WINAPI _export GetFormatName(int Type,char *FormatName,char *DefaultExt)
{
  static const char * const FmtAndExt[][2]={
    {"TAR","tar"},
    {"CPIO","cpio"},
    {"GZip","gz"},
    {"BZip","bz2"},
    {"Z(Unix)","z"},
    {"TGZ","tgz"},
    {"CPZ","cpz"},
    {"RPM","rpm"},
    {"TBZ2","tbz2"},
    {"CPBZ","cpbz"},
  };
  switch(Type)
  {
    case TAR_FORMAT:
    case CPIO_FORMAT:
    case GZ_FORMAT:
    case BZ_FORMAT:
    case Z_FORMAT:
    case TGZ_FORMAT:
    case TBZ2_FORMAT:
    case CPZ_FORMAT:
    case CPBZ_FORMAT:
    case RPM_FORMAT:
      strcpy(FormatName,FmtAndExt[Type][0]);
      strcpy(DefaultExt,FmtAndExt[Type][1]);
      if(Type>Z_FORMAT&&!ZlibOk) break;
      if(Type>CPZ_FORMAT&&!BlibOk) break;
      return(TRUE);
  }
  return(FALSE);
}

BOOL WINAPI _export GetDefaultCommands(int Type,int Command,char *Dest)
{
   static const char * Commands[15]=
   {
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
     "",
     "",
     "*"
   };
   if(Type>=TAR_FORMAT&&Type<=RPM_FORMAT&&(unsigned int)Command<sizeof(Commands)/sizeof(Commands[0]))
   {
     strcpy(Dest,Commands[Command]);
     return(TRUE);
   }
  return(FALSE);
}

void  WINAPI _export SetFarInfo(const struct PluginStartupInfo *Info)
{
  memset(&::Info,0,sizeof(::Info));
  memmove(&::Info,Info,(Info->StructSize>(int)sizeof(::Info))?sizeof(::Info):Info->StructSize);
  ::FSF=*Info->FSF;
  ::Info.FSF=&::FSF;
}

int WINAPI _export ProcessArchive(int Command, void *InData, void *OutData)
{
  {FILE *log; log=fopen("c:\\plugins.log","at"); fprintf(log, "Command: %d\n",Command); fclose(log);}
  if(InData==NULL)
    return Command==PA_EXTRACT || Command==PA_EXTRACTWITHOUTPATH;

  struct ProcArcExtract *pae=(struct ProcArcExtract *)InData;
  unsigned char Data[4096]; bool ReadOk=false; DWORD Readed; int MyArcType=INVALID_FORMAT;
  HANDLE file=CreateFile(pae->ArcName,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,NULL);
  if(file!=INVALID_HANDLE_VALUE)
  {
    if(ReadFile(file,Data,sizeof(Data),&Readed,NULL))
    {
      ReadOk=true;
      MyArcType=MyIsArchive(pae->ArcName,Data,Readed);
    }
    CloseHandle(file);
  }
  if(ReadOk&&MyArcType!=INVALID_FORMAT)
  {
    Archive=CreateArchive(pae->ArcName,MyArcType);
    char Destination[NM],DestinationDir[NM];
    for(int i=0;i<pae->ItemsNumber;i++)
    {
      {FILE *log; log=fopen("c:\\plugins.log","at"); fprintf(log, "%s\n",pae->Item[i]->FindData.cFileName); fclose(log);}
      strcpy(Destination,pae->DestPath);
      FSF.AddEndSlash(Destination);
      if(Command==PA_EXTRACTWITHOUTPATH)
        strcat(Destination,GetFilePtr(pae->Item[i]->FindData.cFileName));
      else
        strcat(Destination,pae->Item[i]->FindData.cFileName);
      strcpy(DestinationDir,Destination);
      *GetFilePtr(DestinationDir)=0;
      FSF.AddEndSlash(DestinationDir);
      if(CreateDirEx(DestinationDir)) //FIXME: show error
      {
        Archive->Extract(pae->Item[i],Destination);
      }
    }
    delete Archive;
  }
  return 1;
}
*/

int OnInitialize (StartupInfo *pInfo)
{
    Info = pInfo->Info;
	FSF = *pInfo->Info.FSF;

	char *lpModuleName = StrDuplicate(Info.ModuleName, 260);

	CutToSlash(lpModuleName);
    strcat (lpModuleName, "zlib.dll");
	InitZlib (lpModuleName);

	CutToSlash(lpModuleName);
	strcat (lpModuleName, "bzip2.dll");
	InitBlib (lpModuleName);

	StrFree (lpModuleName);

	return NAERROR_SUCCESS;
}

int OnFinalize ()
{
	CloseZlib ();
	CloseBlib ();

	return NAERROR_SUCCESS;
}

int OnQueryArchive (QueryArchiveStruct *pQAS)
{
	SFXSize=0;
	ArcType=MyIsArchive(pQAS->lpFileName,(const unsigned char*)pQAS->lpBuffer,pQAS->dwBufferSize);
	if(ArcType!=INVALID_FORMAT)
	{
		pQAS->nFormats = -1;
		pQAS->hResult = (HANDLE)CreateArchive(pQAS->lpFileName,ArcType);
		return NAERROR_SUCCESS;
	}

	return NAERROR_INTERNAL;
}

int OnOpenArchive (OpenArchiveStruct *pOAS)
{
//	ArchiveBase *pArchive = (ArchiveBase*)pOAS->hArchive;

	pOAS->bResult = true;

	return NAERROR_SUCCESS;
}

int OnCloseArchive (CloseArchiveStruct *pCAS)
{
	//ArchiveBase *pArchive = (ArchiveBase*)pCAS->hArchive;

	//pArchive->pCloseArchive ();

	return NAERROR_SUCCESS;
}

int OnFinalizeArchive (ArchiveBase *pArchive)
{
	delete pArchive;

	return NAERROR_SUCCESS;
}

MY_DEFINE_GUID (CLSID_FormatTAR, 0x40292CA4, 0xB8A7, 0x4516, 0xB3, 0xFA, 0x13, 0xBF, 0x9E, 0x0B, 0x88, 0x36);
MY_DEFINE_GUID (CLSID_FormatCPIO, 0x6E3F270C, 0x168A, 0x4C38, 0xBC, 0x16, 0x50, 0x78, 0xF6, 0xA6, 0xD1, 0x1F);
MY_DEFINE_GUID (CLSID_FormatGZip, 0xFCB44EEC, 0x5590, 0x4B52, 0x95, 0xDC, 0x7B, 0x9A, 0xBD, 0x80, 0xDC, 0x9C);
MY_DEFINE_GUID (CLSID_FormatBZip, 0x16C0BE5C, 0x9781, 0x45C9, 0x9A, 0x3C, 0x74, 0xFA, 0x26, 0x37, 0xEB, 0x6C);
MY_DEFINE_GUID (CLSID_FormatZ, 0xDDD0B294, 0x6DD9, 0x47D7, 0x9D, 0x34, 0xF0, 0x89, 0x43, 0x85, 0xD1, 0x6D);
MY_DEFINE_GUID (CLSID_FormatTGZ, 0x8749D59A, 0x6E72, 0x4478, 0xBE, 0x58, 0x95, 0x1C, 0x2E, 0x80, 0xE4, 0x2C);
MY_DEFINE_GUID (CLSID_FormatCPZ, 0x46B3B5E3, 0x30F7, 0x4653, 0xA1, 0xB1, 0x7D, 0x84, 0x25, 0xA0, 0x5C, 0xB8);
MY_DEFINE_GUID (CLSID_FormatRPM, 0x43BB11A7, 0x9CF9, 0x4708, 0x95, 0x3A, 0x66, 0xD5, 0x35, 0x55, 0x9A, 0xC5);
MY_DEFINE_GUID (CLSID_FormatTBZ2, 0x3AF211ED, 0x111A, 0x4408, 0xAB, 0xD9, 0x55, 0x9A, 0xDD, 0x18, 0x56, 0x8E);
MY_DEFINE_GUID (CLSID_FormatCPBZ, 0x21C6E7D0, 0x2D85, 0x427D, 0xBB, 0x41, 0x03, 0xEB, 0xB2, 0xE0, 0x7D, 0xBA);

ArchiveFormatInfo FormatInfo[] = {
			{CLSID_FormatTAR, AFF_SUPPORT_INTERNAL_EXTRACT, "TAR", "tar"},
			{CLSID_FormatCPIO, AFF_SUPPORT_INTERNAL_EXTRACT, "CPIO", "cpio"},
			{CLSID_FormatGZip, AFF_SUPPORT_INTERNAL_EXTRACT, "GZip", "gz"},
			{CLSID_FormatBZip, AFF_SUPPORT_INTERNAL_EXTRACT, "BZip", "bz2"},
			{CLSID_FormatZ, AFF_SUPPORT_INTERNAL_EXTRACT, "Z(Unix)", "z"},
			{CLSID_FormatTGZ, AFF_SUPPORT_INTERNAL_EXTRACT, "TGZ", "tgz"},
			{CLSID_FormatCPZ, AFF_SUPPORT_INTERNAL_EXTRACT, "CPZ", "cpz"},
			{CLSID_FormatRPM, AFF_SUPPORT_INTERNAL_EXTRACT, "RPM", "rpm"},
			{CLSID_FormatTBZ2, AFF_SUPPORT_INTERNAL_EXTRACT, "TBZ2", "tbz2"},
			{CLSID_FormatCPBZ, AFF_SUPPORT_INTERNAL_EXTRACT, "CPBZ", "cpbz"},
			};


int OnGetArchivePluginInfo (
		ArchivePluginInfo *ai
		)
{
	ai->nFormats = sizeof(FormatInfo)/sizeof(FormatInfo[0]);
	ai->pFormatInfo = (ArchiveFormatInfo*)&FormatInfo;

	return NAERROR_SUCCESS;
}

int OnGetArchiveItem (GetArchiveItemStruct *pGAI)
{
	ArchiveBase *pArchive = (ArchiveBase*)pGAI->hArchive;

	pGAI->nResult = pArchive->Next(&(pGAI->pItem->pi));

	return NAERROR_SUCCESS;
}

int OnGetArchiveFormat (GetArchiveFormatStruct *pGAF)
{
	//ArchiveBase *pArchive = (ArchiveBase*)pGAF->hArchive;

	memcpy (&pGAF->uid, &FormatInfo[ArcType].uid, sizeof (GUID));

	return NAERROR_SUCCESS;
}

int OnExtract (ExtractStruct *pES)
{
	ArchiveBase *pArchive = (ArchiveBase*)pES->hArchive;

	pES->bResult = false;

	char Destination[NM],DestinationDir[NM];
	for(int i=0;i<pES->nItemsNumber;i++)
	{
		//{FILE *log; log=fopen("c:\\plugins.log","at"); fprintf(log, "%s\n",pae->Item[i]->FindData.cFileName); fclose(log);}
		strcpy(Destination,pES->lpDestPath);
		FSF.AddEndSlash(Destination);
		//if(Command==PA_EXTRACTWITHOUTPATH)
		//	strcat(Destination,GetFilePtr(pae->Item[i]->FindData.cFileName));
		//else
		strcat(Destination,(pES->pItems)[i].FindData.cFileName + (*pES->lpCurrentPath ? strlen (pES->lpCurrentPath) : 0));
		strcpy(DestinationDir,Destination);
		*GetFilePtr(DestinationDir)=0;
		FSF.AddEndSlash(DestinationDir);
		if(CreateDirEx(DestinationDir)) //FIXME: show error
		{
			pES->bResult = pArchive->Extract(&(pES->pItems)[i],Destination);
		}
	}

	return NAERROR_SUCCESS;
}

/*
int OnTest (TestStruct *pTS)
{
	ArchiveBase *pArchive = (ArchiveBase*)pTS->hArchive;

	pTS->bResult = pArchive->pTest (
			pTS->pItems,
			pTS->nItemsNumber
			);

	return NAERROR_SUCCESS;
}
*/

int OnGetDefaultCommand (GetDefaultCommandStruct *pGDC)
{
	static const char *pCommands[]={
	/*Extract               */"",
	/*Extract without paths */"",
	/*Test                  */"",
	/*Delete                */"",
	/*Comment archive       */"",
	/*Comment files         */"",
	/*Convert to SFX        */"",
	/*Lock archive          */"",
	/*Protect archive       */"",
	/*Recover archive       */"",
	/*Add files             */"",
	};

	pGDC->bResult = false;

	if ( pGDC->nCommand < (int)(sizeof(pCommands)/sizeof(pCommands[0])) )
	{
		for (unsigned int i = 0; i < sizeof (FormatInfo)/sizeof (FormatInfo[0]); i++)
		{
			if ( FormatInfo[i].uid == pGDC->uid )
			{
				strcpy (pGDC->lpCommand, pCommands[pGDC->nCommand]);
				pGDC->bResult = true;
				break;
			}
		}
	}

	return NAERROR_SUCCESS;
}


int __stdcall PluginEntry (
		int nFunctionID,
		void *pParams
		)
{
	switch ( nFunctionID ) {

	case FID_INITIALIZE:
		return OnInitialize ((StartupInfo*)pParams);

	case FID_FINALIZE:
		return OnFinalize ();

	case FID_QUERYARCHIVE:
		return OnQueryArchive ((QueryArchiveStruct*)pParams);

	case FID_OPENARCHIVE:
		return OnOpenArchive ((OpenArchiveStruct*)pParams);

	case FID_CLOSEARCHIVE:
		return OnCloseArchive ((CloseArchiveStruct*)pParams);

	case FID_FINALIZEARCHIVE:
		return OnFinalizeArchive ((ArchiveBase*)pParams);

	case FID_GETARCHIVEPLUGININFO:
		return OnGetArchivePluginInfo ((ArchivePluginInfo*)pParams);

	case FID_GETARCHIVEITEM:
		return OnGetArchiveItem ((GetArchiveItemStruct*)pParams);

	case FID_GETARCHIVEFORMAT:
		return OnGetArchiveFormat ((GetArchiveFormatStruct*)pParams);

	case FID_EXTRACT:
		return OnExtract ((ExtractStruct*)pParams);

	//case FID_TEST:
	//	return OnTest ((TestStruct*)pParams);

	case FID_GETDEFAULTCOMMAND:
		return OnGetDefaultCommand ((GetDefaultCommandStruct*)pParams);
	}

	return NAERROR_NOTIMPLEMENTED;
}

#if defined(__GNUC__)
#ifdef __cplusplus
extern "C"{
#endif
	BOOL WINAPI DllMainCRTStartup (HANDLE hDll, DWORD dwReason, LPVOID lpReserved);
#ifdef __cplusplus
};
#endif

BOOL WINAPI DllMainCRTStartup (HANDLE hDll, DWORD dwReason, LPVOID lpReserved)
{
	(void)hDll;
	(void)dwReason;
	(void)lpReserved;
	return TRUE;
}

#else
/*
BOOL __stdcall DllMain (
		HINSTANCE hinstDLL,
		DWORD fdwReason,
		LPVOID lpvReserved
		)
{
	return true;
}*/
#endif
