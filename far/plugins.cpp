/*
plugins.cpp

Работа с плагинами (низкий уровень, кое-что повыше в flplugin.cpp)

*/

#include "headers.hpp"
#pragma hdrstop

#include "plugins.hpp"
#include "lang.hpp"
#include "keys.hpp"
#include "plugin.hpp"
#include "fn.hpp"
#include "flink.hpp"
#include "global.hpp"
#include "scantree.hpp"
#include "chgprior.hpp"
#include "constitle.hpp"
#include "cmdline.hpp"
#include "filepanels.hpp"
#include "panel.hpp"
#include "vmenu.hpp"
#include "dialog.hpp"
#include "rdrwdsk.hpp"
#include "savescr.hpp"
#include "ctrlobj.hpp"
#include "scrbuf.hpp"
#include "udlist.hpp"
#include "farexcpt.hpp"
#include "fileedit.hpp"
#include "RefreshFrameManager.hpp"
#include "BlockExtKey.hpp"

#ifdef _MSC_VER
#pragma warning(disable:4509)
#endif


static void CheckScreenLock();

static const wchar_t *FmtPluginsCache_PluginDW=L"PluginsCache\\Plugin%d";
static const wchar_t *FmtPluginsCache_PluginDExportW=L"PluginsCache\\Plugin%d\\Exports";
static const wchar_t *FmtDiskMenuStringDW=L"DiskMenuString%d";
static const wchar_t *FmtDiskMenuNumberDW=L"DiskMenuNumber%d";
static const wchar_t *FmtPluginMenuStringDW=L"PluginMenuString%d";
static const wchar_t *FmtPluginConfigStringDW=L"PluginConfigString%d";
static const wchar_t *NFMP_PreloadW=L"Preload";
static const wchar_t *NFMP_SysIDW=L"SysID";

static const char NFMP_OpenPlugin[]="OpenPlugin";
static const char NFMP_OpenFilePlugin[]="OpenFilePlugin";
static const char NFMP_SetFindList[]="SetFindList";
static const char NFMP_ProcessEditorInput[]="ProcessEditorInput";
static const char NFMP_ProcessEditorEvent[]="ProcessEditorEvent";
static const char NFMP_ProcessViewerEvent[]="ProcessViewerEvent";
static const char NFMP_SetStartupInfo[]="SetStartupInfo";
static const char NFMP_ClosePlugin[]="ClosePlugin";
static const char NFMP_GetPluginInfo[]="GetPluginInfo";
static const char NFMP_GetOpenPluginInfo[]="GetOpenPluginInfo";
static const char NFMP_GetFindData[]="GetFindData";
static const char NFMP_FreeFindData[]="FreeFindData";
static const char NFMP_GetVirtualFindData[]="GetVirtualFindData";
static const char NFMP_FreeVirtualFindData[]="FreeVirtualFindData";
static const char NFMP_SetDirectory[]="SetDirectory";
static const char NFMP_GetFiles[]="GetFiles";
static const char NFMP_PutFiles[]="PutFiles";
static const char NFMP_DeleteFiles[]="DeleteFiles";
static const char NFMP_MakeDirectory[]="MakeDirectory";
static const char NFMP_ProcessHostFile[]="ProcessHostFile";
static const char NFMP_Configure[]="Configure";
static const char NFMP_ExitFAR[]="ExitFAR";
static const char NFMP_ProcessKey[]="ProcessKey";
static const char NFMP_ProcessEvent[]="ProcessEvent";
static const char NFMP_Compare[]="Compare";
static const char NFMP_GetMinFarVersion[]="GetMinFarVersion";


static const wchar_t NFMP_OpenPluginW[]=L"OpenPlugin";
static const wchar_t NFMP_OpenFilePluginW[]=L"OpenFilePlugin";
static const wchar_t NFMP_SetFindListW[]=L"SetFindList";
static const wchar_t NFMP_ProcessEditorInputW[]=L"ProcessEditorInput";
static const wchar_t NFMP_ProcessEditorEventW[]=L"ProcessEditorEvent";
static const wchar_t NFMP_ProcessViewerEventW[]=L"ProcessViewerEvent";
static const wchar_t NFMP_SetStartupInfoW[]=L"SetStartupInfo";
static const wchar_t NFMP_ClosePluginW[]=L"ClosePlugin";
static const wchar_t NFMP_GetPluginInfoW[]=L"GetPluginInfo";
static const wchar_t NFMP_GetOpenPluginInfoW[]=L"GetOpenPluginInfo";
static const wchar_t NFMP_GetFindDataW[]=L"GetFindData";
static const wchar_t NFMP_FreeFindDataW[]=L"FreeFindData";
static const wchar_t NFMP_GetVirtualFindDataW[]=L"GetVirtualFindData";
static const wchar_t NFMP_FreeVirtualFindDataW[]=L"FreeVirtualFindData";
static const wchar_t NFMP_SetDirectoryW[]=L"SetDirectory";
static const wchar_t NFMP_GetFilesW[]=L"GetFiles";
static const wchar_t NFMP_PutFilesW[]=L"PutFiles";
static const wchar_t NFMP_DeleteFilesW[]=L"DeleteFiles";
static const wchar_t NFMP_MakeDirectoryW[]=L"MakeDirectory";
static const wchar_t NFMP_ProcessHostFileW[]=L"ProcessHostFile";
static const wchar_t NFMP_ConfigureW[]=L"Configure";
static const wchar_t NFMP_ExitFARW[]=L"ExitFAR";
static const wchar_t NFMP_ProcessKeyW[]=L"ProcessKey";
static const wchar_t NFMP_ProcessEventW[]=L"ProcessEvent";
static const wchar_t NFMP_CompareW[]=L"Compare";
static const wchar_t NFMP_GetMinFarVersionW[]=L"GetMinFarVersion";



static const wchar_t *RKN_PluginsCacheW=L"PluginsCache";

static int _cdecl PluginsSort(const void *el1,const void *el2);
static BOOL PrepareModulePath(const char *ModuleName);

Plugin::Plugin()
{
	hModule = NULL;

	//more initialization here!!!
}

PluginsSet::PluginsSet()
{
  PluginsData=NULL;
  PluginsCount=0;
  CurPluginItem=NULL;
  Reserved=0;
  CurEditor=NULL;
  CurViewer=NULL;
}


PluginsSet::~PluginsSet()
{
  CurPluginItem=NULL;
  Plugin *PData;
  for (int I=0;I<PluginsCount;I++)
  {
    PData = PluginsData[I];

    if (PData->WorkFlags.Check(PIWF_CACHED))
      continue;
    FreeLibrary(PData->hModule);
    PData->Lang.Close();
  }
  if(PluginsData)
  {
    for (int i = 0; i < PluginsCount; i++)
        delete PluginsData[i];
    xf_free(PluginsData);
  }
}

void PluginsSet::SendExit()
{
  Plugin *PData;
  for (int I=0;I<PluginsCount;I++)
  {
    PData = PluginsData[I];
    if (!PData->WorkFlags.Check(PIWF_CACHED) && PData->pExitFAR && !ProcessException)
    {
      //CurPluginItem=PData;
      PData->FuncFlags.Set(PICFF_EXITFAR);
      if(Opt.ExceptRules)
      {
        TRY{
          PData->pExitFAR();
        }
        EXCEPT(xfilter(EXCEPT_EXITFAR,GetExceptionInformation(),PData,1)){
          ;
        }
        ProcessException=FALSE;
      }
      else
        PData->pExitFAR();
      //CurPluginItem=NULL;
      PData->FuncFlags.Clear(PICFF_EXITFAR);
    }
  }
}

void PluginsSet::LoadPlugins()
{
  BlockExtKey blockExtKey;
  Flags.Clear(PSIF_PLUGINSLOADDED);

  if (Opt.LoadPlug.PluginsCacheOnly)  // $ 01.09.2000 tran  '/co' switch
  {
     LoadPluginsFromCache();
     return;
  }

  if(Opt.LoadPlug.MainPluginDir || !Opt.LoadPlug.strCustomPluginsPath.IsEmpty() || (Opt.LoadPlug.PluginsPersonal && !Opt.LoadPlug.strPersonalPluginsPath.IsEmpty()))
  {
    ScanTree ScTree(FALSE,TRUE);
    UserDefinedListW PluginPathList;  // хранение списка каталогов

    int I;
    string strPluginsDir;
    string strFullName;
    FAR_FIND_DATA_EX FindData;
    Plugin *PData;

    PluginPathList.SetParameters(0,0,ULF_UNIQUE);
    // сначала подготовим список
    if(Opt.LoadPlug.MainPluginDir) // только основные и персональные?
    {
      strPluginsDir.Format (L"%s%s",(const wchar_t*)g_strFarPath, PluginsFolderNameW);
      PluginPathList.AddItem(strPluginsDir);
      // ...а персональные есть?
      if(Opt.LoadPlug.PluginsPersonal && !Opt.LoadPlug.strPersonalPluginsPath.IsEmpty() && !(Opt.Policies.DisabledOptions&FFPOL_PERSONALPATH))
        PluginPathList.AddItem(Opt.LoadPlug.strPersonalPluginsPath);
    }
    else if( !Opt.LoadPlug.strCustomPluginsPath.IsEmpty() ) // только "заказные" пути?
      PluginPathList.AddItem(Opt.LoadPlug.strCustomPluginsPath);

    const wchar_t *NamePtr;
    PluginPathList.Reset();
    // теперь пройдемся по всему ранее собранному списку
    while(NULL!=(NamePtr=PluginPathList.GetNext()))
    {
      // расширяем значение пути
      apiExpandEnvironmentStrings (NamePtr,strFullName);
      UnquoteW(strFullName); //??? здесь ХЗ
      if(!PathMayBeAbsoluteW(strFullName))
      {
        strPluginsDir = g_strFarPath;
        strPluginsDir += strFullName;
        strFullName = strPluginsDir;
      }
      // Получим реальное значение полного длинного пути с учетом символических связей.
      ConvertNameToRealW(strFullName,strFullName);
      RawConvertShortNameToLongNameW(strFullName,strFullName);

      strPluginsDir = strFullName;

      if( strPluginsDir.IsEmpty() ) // Хмм... а нужно ли ЭТО условие после такой модернизации алгоритма загрузки?
        continue;

      // ставим на поток очередной путь из списка...
      ScTree.SetFindPathW(strPluginsDir,L"*.*");

      // ...и пройдемся по нему
      while (ScTree.GetNextNameW(&FindData,strFullName))
      {
        if ( CmpNameW(L"*.dll",FindData.strFileName,FALSE) &&
             (FindData.dwFileAttributes & FA_DIREC)==0 )
        {
          Plugin *CurPlugin = new Plugin;
          string strRegKey;
          //memset(CurPlugin,0,sizeof(Plugin)); ///!!! BUGBUG

          CurPlugin->strModuleName = strFullName;
          int CachePos=GetCacheNumber(strFullName,&FindData,0);
          int LoadCached;
          if(CachePos!=-1)
          {
            LoadCached=TRUE;
            /* $ 12.10.2000 tran
               Preload=1 нужно для корректной обработки -co */
            strRegKey.Format (FmtPluginsCache_PluginDW,CachePos);
            if ( GetRegKeyW(strRegKey,NFMP_PreloadW,0)==1 )
            {
              LoadCached=FALSE;
              CachePos=-1;
            }
            /* tran $ */
          }
          else
            LoadCached=FALSE;

          if (LoadCached)
          {
            string strRegKey;
            strRegKey.Format (FmtPluginsCache_PluginDExportW,CachePos);
            CurPlugin->SysID=GetRegKeyW(strRegKey,NFMP_SysIDW,0);
            CurPlugin->pOpenPlugin=(PLUGINOPENPLUGIN)(INT_PTR)GetRegKeyW(strRegKey,NFMP_OpenPluginW,0);
            CurPlugin->pOpenFilePlugin=(PLUGINOPENFILEPLUGIN)(INT_PTR)GetRegKeyW(strRegKey,NFMP_OpenFilePluginW,0);
            CurPlugin->pSetFindList=(PLUGINSETFINDLIST)(INT_PTR)GetRegKeyW(strRegKey,NFMP_SetFindListW,0);
            CurPlugin->pProcessEditorInput=(PLUGINPROCESSEDITORINPUT)(INT_PTR)GetRegKeyW(strRegKey,NFMP_ProcessEditorInputW,0);
            CurPlugin->pProcessEditorEvent=(PLUGINPROCESSEDITOREVENT)(INT_PTR)GetRegKeyW(strRegKey,NFMP_ProcessEditorEventW,0);
            CurPlugin->pProcessViewerEvent=(PLUGINPROCESSVIEWEREVENT)(INT_PTR)GetRegKeyW(strRegKey,NFMP_ProcessViewerEventW,0);
            CurPlugin->CachePos=CachePos;
          }
          if (LoadCached || LoadPlugin(CurPlugin,-1,TRUE))
          {
            Plugin **NewPluginsData=(Plugin**)xf_realloc(PluginsData,sizeof(*PluginsData)*(PluginsCount+1));
            if (NewPluginsData==NULL)
            {
              delete CurPlugin;
              break;
            }

            PluginsData=NewPluginsData;
            CurPlugin->WorkFlags.Change(PIWF_CACHED,LoadCached);
            CurPlugin->FindData=FindData;
            PluginsData[PluginsCount]=CurPlugin;
            PluginsCount++;
          }
          else
          	delete CurPlugin;
        }
      } // end while
    }

    far_qsort(PluginsData,PluginsCount,sizeof(*PluginsData),PluginsSort);

    int NewPlugin=FALSE;

    for (I=0;I<PluginsCount;I++)
    {
      PData = PluginsData[I];
      if (!PData->WorkFlags.Check(PIWF_CACHED))
      {
        SetPluginStartupInfo(PData,I);
        if (SavePluginSettings(PData,PData->FindData))
          NewPlugin=TRUE;
      }
    }

    if (NewPlugin)
    {
      for (int I=0;;I++)
      {
        string strRegKey, strPluginName;
        strRegKey.Format (FmtPluginsCache_PluginDW,I);
        GetRegKeyW(strRegKey,L"Name",strPluginName,L"");
        if ( strPluginName.IsEmpty() )
          break;
        if (GetFileAttributesW(strPluginName)==0xFFFFFFFF)
        {
          DeleteKeyRecordW(FmtPluginsCache_PluginDW,I);
          I--;
        }
      }
    }
  }
  Flags.Set(PSIF_PLUGINSLOADDED);
}

/* $ 01.09.2000 tran
   Load cache only plugins  - '/co' switch */
void PluginsSet::LoadPluginsFromCache()
{
  /*
    [HKEY_CURRENT_USER\Software\Far\PluginsCache\Plugin0]
    "Name"="C:\\PROGRAM FILES\\FAR\\Plugins\\ABOOK\\AddrBook.dll"
    "ID"="e400a14def00a37ea900"
    "DiskMenuString0"="Address Book"
    "PluginMenuString0"="Address Book"
    "PluginConfigString0"="Address Book"
    "PluginConfigString1"="Address: E-Mail"
    "PluginConfigString2"="Address: Birthday"
    "PluginConfigString3"="Address: Phone number"
    "PluginConfigString4"="Address: Fidonet"
    "CommandPrefix"=""
    "Flags"=dword:00000000

    [HKEY_CURRENT_USER\Software\Far\PluginsCache\Plugin0\Exports]
    "OpenPlugin"=dword:00000001
    "OpenFilePlugin"=dword:00000000
    "SetFindList"=dword:00000000
    "ProcessEditorInput"=dword:00000000
    "ProcessEditorEvent"=dword:00000000
  */
  int I;
  string strPlgKey;
  string strRegKey;
  Plugin *CurPlugin = new Plugin;

  for (I=0;;I++)
  {
    if (!EnumRegKeyW(RKN_PluginsCacheW,I,strPlgKey))
    {
      delete CurPlugin;
      break;
    }

    //memset(CurPlugin,0,sizeof(PluginItem)); //BUGBUG
                        //  012345678901234567890
    strRegKey = strPlgKey; // "PLuginsCache\PluginXX"
    GetRegKeyW(strRegKey,L"Name",CurPlugin->strModuleName,L"");
    /* $ 12.10.2000 tran
      -co должен понимать PRELOAD плагины */
    if ( GetRegKeyW(strRegKey,NFMP_PreloadW,0)==1 )
    {

      if (!LoadPlugin(CurPlugin,-1,TRUE))
        continue; // загрузка не удалась
      CurPlugin->WorkFlags.Clear(PIWF_CACHED);
    }
    else
    {
    /* tran $ */
      strRegKey += L"\\";
      strRegKey += L"Exports";
      CurPlugin->SysID=GetRegKeyW(strRegKey,NFMP_SysIDW,0);
      CurPlugin->pOpenPlugin=(PLUGINOPENPLUGIN)(INT_PTR)GetRegKeyW(strRegKey,NFMP_OpenPluginW,0);
      CurPlugin->pOpenFilePlugin=(PLUGINOPENFILEPLUGIN)(INT_PTR)GetRegKeyW(strRegKey,NFMP_OpenFilePluginW,0);
      CurPlugin->pSetFindList=(PLUGINSETFINDLIST)(INT_PTR)GetRegKeyW(strRegKey,NFMP_SetFindListW,0);
      CurPlugin->pProcessEditorInput=(PLUGINPROCESSEDITORINPUT)(INT_PTR)GetRegKeyW(strRegKey,NFMP_ProcessEditorInputW,0);
      CurPlugin->pProcessEditorEvent=(PLUGINPROCESSEDITOREVENT)(INT_PTR)GetRegKeyW(strRegKey,NFMP_ProcessEditorEventW,0);
      CurPlugin->pProcessViewerEvent=(PLUGINPROCESSVIEWEREVENT)(INT_PTR)GetRegKeyW(strRegKey,NFMP_ProcessViewerEventW,0);
      CurPlugin->CachePos=_wtoi((const wchar_t*)strPlgKey+19);
      CurPlugin->WorkFlags.Set(PIWF_CACHED);
      // вот тут это поле не заполнено, надеюсь, что оно не критично
      // CurPlugin.FindData=FindData;
    }
    Plugin **NewPluginsData=(Plugin**)xf_realloc(PluginsData,sizeof(*PluginsData)*(PluginsCount+1));
    if (NewPluginsData==NULL)
    {
      delete CurPlugin;
      break;
    }
    PluginsData=NewPluginsData;
    PluginsData[PluginsCount]=CurPlugin;
    PluginsCount++;
  }

  far_qsort(PluginsData,PluginsCount,sizeof(*PluginsData),PluginsSort);
  /* $ 19.10.2000 tran
     забыл вызвать SetStartupInfo :) */
  Plugin *PData;
  for (I=0;I<PluginsCount;I++)
  {
    PData = PluginsData[I];
    if (!PData->WorkFlags.Check(PIWF_CACHED))
    {
      SetPluginStartupInfo(PData,I);
    }
  }
  /* tran $ */
  Flags.Set(PSIF_PLUGINSLOADDED);
}
/* tran $ */

int _cdecl PluginsSort(const void *el1,const void *el2)
{
  Plugin *Plugin1=*((Plugin**)el1);
  Plugin *Plugin2=*((Plugin**)el2);
  return(LocalStricmpW(PointToNameW(Plugin1->strModuleName),PointToNameW(Plugin2->strModuleName)));
}

static BOOL PrepareModulePathW(const wchar_t *ModuleName)
{
  string strModulePath;
  strModulePath = ModuleName;

  CutToSlashW (strModulePath); //??

  return FarChDirW(strModulePath,TRUE);
}


unsigned long CRC32(
    unsigned long crc,
    const char *buf,
    unsigned int len
    )
{
  static unsigned long crc_table[256];

  if (!crc_table[1])
  {
    unsigned long c;
    int n, k;

    for (n = 0; n < 256; n++)
    {
      c = (unsigned long)n;
      for (k = 0; k < 8; k++) c = (c >> 1) ^ (c & 1 ? 0xedb88320L : 0);
        crc_table[n] = c;
    }
  }

  crc = crc ^ 0xffffffffL;
  while (len-- > 0) {
    crc = crc_table[(crc ^ (*buf++)) & 0xff] ^ (crc >> 8);
  }

  return crc ^ 0xffffffffL;
}


#define CRC32_SETSTARTUPINFO  0xF537107A
#define CRC32_GETPLUGININFO   0xDB6424B4
#define CRC32_OPENPLUGIN    0x601AEDE8
#define CRC32_OPENFILEPLUGIN  0xAC9FF5CD
#define CRC32_EXITFAR     0x04419715
#define CRC32_SETFINDLIST   0x7A74A2E5
#define CRC32_CONFIGURE     0x4DC1BC1A
#define CRC32_GETMINFARVERSION  0x2BBAD952

DWORD ExportCRC32[7] = {
    CRC32_SETSTARTUPINFO,
    CRC32_GETPLUGININFO,
    CRC32_OPENPLUGIN,
    CRC32_OPENFILEPLUGIN,
    CRC32_EXITFAR,
    CRC32_SETFINDLIST,
//    CRC32_CONFIGURE,
    CRC32_GETMINFARVERSION
    };

BOOL IsModulePlugin2 (
	PBYTE hModule
	)
{
	DWORD dwExportAddr;

	PIMAGE_DOS_HEADER pDOSHeader = (PIMAGE_DOS_HEADER)hModule;
	PIMAGE_NT_HEADERS pPEHeader;

	TRY {

		if ( pDOSHeader->e_magic != IMAGE_DOS_SIGNATURE )
			return FALSE;

		pPEHeader = (PIMAGE_NT_HEADERS)&hModule[pDOSHeader->e_lfanew];

		if ( pPEHeader->Signature != IMAGE_NT_SIGNATURE )
			return FALSE;

		if ( (pPEHeader->FileHeader.Characteristics & IMAGE_FILE_DLL) == 0 )
			return FALSE;

		dwExportAddr = pPEHeader->OptionalHeader.DataDirectory[0].VirtualAddress;

		if ( !dwExportAddr )
			return FALSE;

		PIMAGE_SECTION_HEADER pSection = (PIMAGE_SECTION_HEADER)IMAGE_FIRST_SECTION (pPEHeader);

		for (int i = 0; i < pPEHeader->FileHeader.NumberOfSections; i++)
		{
			if ( (pSection[i].VirtualAddress == dwExportAddr) ||
				 ((pSection[i].VirtualAddress <= dwExportAddr ) && ((pSection[i].Misc.VirtualSize+pSection[i].VirtualAddress) > dwExportAddr)) )
			{
				int nDiff = pSection[i].VirtualAddress-pSection[i].PointerToRawData;

				PIMAGE_EXPORT_DIRECTORY pExportDir = (PIMAGE_EXPORT_DIRECTORY)&hModule[dwExportAddr-nDiff];

				DWORD* pNames = (DWORD *)&hModule[pExportDir->AddressOfNames-nDiff];

				for (int n = 0; n < pExportDir->NumberOfNames; n++)
				{
					const char *lpExportName = (const char *)&hModule[pNames[n]-nDiff];

					DWORD dwCRC32 = CRC32 (0, lpExportName, strlen (lpExportName));

					for (int j = 0; j < 7; j++)  // а это вам не фиг знает что, это вам оптимизация, типа 8-)
						if ( dwCRC32 == ExportCRC32[j] )
							return TRUE;

/*          if ( !strcmp (lpExportName, "GetPluginInfo") ||
             !strcmp (lpExportName, "SetStartupInfo") ||
             !strcmp (lpExportName, "OpenPlugin") ||
             !strcmp (lpExportName, "OpenFilePlugin") ||
             !strcmp (lpExportName, "SetFindList") ||
             !strcmp (lpExportName, "Configure") ||
             !strcmp (lpExportName, "GetMinFarVersion") ||
             !strcmp (lpExportName, "ExitFAR") )
            return LOAD_ERROR_SUCCESS;*/
				}
			}
		}

		return FALSE;
	}
	EXCEPT (EXCEPTION_EXECUTE_HANDLER)
	{
		return FALSE;
	}
}

BOOL IsModulePlugin (const wchar_t *lpModuleName)
{
	int bResult = FALSE;

	HANDLE hModuleFile = CreateFileW (
			lpModuleName,
			GENERIC_READ,
			FILE_SHARE_READ,
			NULL,
			OPEN_EXISTING,
			0,
			NULL
			);

	if ( hModuleFile != INVALID_HANDLE_VALUE )
	{
		HANDLE hModuleMapping = CreateFileMapping (
				hModuleFile,
				NULL,
				PAGE_READONLY,
				0,
				0,
				NULL
				);

		if ( hModuleMapping )
		{
			PBYTE pData = (PBYTE)MapViewOfFile (hModuleMapping, FILE_MAP_READ, 0, 0, 0);

			bResult = IsModulePlugin2 (pData);

			UnmapViewOfFile (pData);
			CloseHandle (hModuleMapping);
		}

		CloseHandle (hModuleFile);
	}

	return bResult;
}


int PluginsSet::LoadPlugin(Plugin *CurPlugin,int ModuleNumber,int Init)
{
  if(CurPlugin->WorkFlags.Check(PIWF_DONTLOADAGAIN))
  {
    return (FALSE);
  }

  DWORD LstErr;
  HMODULE hModule=CurPlugin->hModule;
  if(!hModule)
  {
   /* $ 05.02.2003 VVM
     - После загрузки плагина восстановим тот путь, что был */
    string strCurPath, strCurPlugDiskPath;
    wchar_t Drive[4];
    Drive[0]=0; // ставим 0, как признак того, что вертать обратно ненадо!
    FarGetCurDirW(strCurPath);
    if(IsLocalPathW(CurPlugin->strModuleName)) // если указан локальный путь, то...
    {
      // ...получим соответствующую переменную окружения
      wcscpy(Drive,L"= :");
      Drive[1]=CurPlugin->strModuleName.At(0);
      apiGetEnvironmentVariable (Drive,strCurPlugDiskPath);
    }
    PrepareModulePathW(CurPlugin->strModuleName);

    if ( !IsModulePlugin (CurPlugin->strModuleName) )
    	return FALSE;

    hModule=LoadLibraryExW(CurPlugin->strModuleName,NULL,LOAD_WITH_ALTERED_SEARCH_PATH);
    if(!hModule)
      LstErr=GetLastError();
    FarChDirW(strCurPath, TRUE);
    if(Drive[0]) // вернем ее (переменную окружения) обратно
      SetEnvironmentVariableW(Drive,strCurPlugDiskPath);
    /* VVM $ */
  }

  /* "...И добавь первичную загрузку с DONT_RESOLVE_DLL_REFERENCES..."  */
  if ( !hModule )
  {
    if(!Opt.LoadPlug.SilentLoadPlugin)
    {
      string strPlgName;
      strPlgName = CurPlugin->strModuleName;
      TruncPathStrW(strPlgName,ScrX-20);
      SetMessageHelp(L"ErrLoadPlugin");
      MessageW(MSG_WARNING,1,UMSG(MError),UMSG(MPlgLoadPluginError),strPlgName,UMSG(MOk));
    }
    CurPlugin->WorkFlags.Set(PIWF_DONTLOADAGAIN);
    return(FALSE);
  }

  CurPlugin->hModule=hModule;
  CurPlugin->WorkFlags.Clear(PIWF_CACHED);
  CurPlugin->pSetStartupInfo=(PLUGINSETSTARTUPINFO)GetProcAddress(hModule,NFMP_SetStartupInfo);
  CurPlugin->pOpenPlugin=(PLUGINOPENPLUGIN)GetProcAddress(hModule,NFMP_OpenPlugin);
  CurPlugin->pOpenFilePlugin=(PLUGINOPENFILEPLUGIN)GetProcAddress(hModule,NFMP_OpenFilePlugin);
  CurPlugin->pClosePlugin=(PLUGINCLOSEPLUGIN)GetProcAddress(hModule,NFMP_ClosePlugin);
  CurPlugin->pGetPluginInfo=(PLUGINGETPLUGININFO)GetProcAddress(hModule,NFMP_GetPluginInfo);
  CurPlugin->pGetOpenPluginInfo=(PLUGINGETOPENPLUGININFO)GetProcAddress(hModule,NFMP_GetOpenPluginInfo);
  CurPlugin->pGetFindData=(PLUGINGETFINDDATA)GetProcAddress(hModule,NFMP_GetFindData);
  CurPlugin->pFreeFindData=(PLUGINFREEFINDDATA)GetProcAddress(hModule,NFMP_FreeFindData);
  CurPlugin->pGetVirtualFindData=(PLUGINGETVIRTUALFINDDATA)GetProcAddress(hModule,NFMP_GetVirtualFindData);
  CurPlugin->pFreeVirtualFindData=(PLUGINFREEVIRTUALFINDDATA)GetProcAddress(hModule,NFMP_FreeVirtualFindData);
  CurPlugin->pSetDirectory=(PLUGINSETDIRECTORY)GetProcAddress(hModule,NFMP_SetDirectory);
  CurPlugin->pGetFiles=(PLUGINGETFILES)GetProcAddress(hModule,NFMP_GetFiles);
  CurPlugin->pPutFiles=(PLUGINPUTFILES)GetProcAddress(hModule,NFMP_PutFiles);
  CurPlugin->pDeleteFiles=(PLUGINDELETEFILES)GetProcAddress(hModule,NFMP_DeleteFiles);
  CurPlugin->pMakeDirectory=(PLUGINMAKEDIRECTORY)GetProcAddress(hModule,NFMP_MakeDirectory);
  CurPlugin->pProcessHostFile=(PLUGINPROCESSHOSTFILE)GetProcAddress(hModule,NFMP_ProcessHostFile);
  CurPlugin->pSetFindList=(PLUGINSETFINDLIST)GetProcAddress(hModule,NFMP_SetFindList);
  CurPlugin->pConfigure=(PLUGINCONFIGURE)GetProcAddress(hModule,NFMP_Configure);
  CurPlugin->pExitFAR=(PLUGINEXITFAR)GetProcAddress(hModule,NFMP_ExitFAR);
  CurPlugin->pProcessKey=(PLUGINPROCESSKEY)GetProcAddress(hModule,NFMP_ProcessKey);
  CurPlugin->pProcessEvent=(PLUGINPROCESSEVENT)GetProcAddress(hModule,NFMP_ProcessEvent);
  CurPlugin->pCompare=(PLUGINCOMPARE)GetProcAddress(hModule,NFMP_Compare);
  CurPlugin->pProcessEditorInput=(PLUGINPROCESSEDITORINPUT)GetProcAddress(hModule,NFMP_ProcessEditorInput);
  CurPlugin->pProcessEditorEvent=(PLUGINPROCESSEDITOREVENT)GetProcAddress(hModule,NFMP_ProcessEditorEvent);
  CurPlugin->pProcessViewerEvent=(PLUGINPROCESSVIEWEREVENT)GetProcAddress(hModule,NFMP_ProcessViewerEvent);
  CurPlugin->pMinFarVersion=(PLUGINMINFARVERSION)GetProcAddress(hModule,NFMP_GetMinFarVersion);
  CurPlugin->LinkedFrame=NULL;
  /*$ 13.09.2001 SKV
    Если плагин не экспортирует ни одной
    функции, то не будем его грузить.
  */
  if(!(CurPlugin->pSetStartupInfo ||
    CurPlugin->pOpenPlugin ||
    CurPlugin->pOpenFilePlugin ||
    CurPlugin->pClosePlugin ||
    CurPlugin->pGetPluginInfo ||
    CurPlugin->pGetOpenPluginInfo ||
    CurPlugin->pGetFindData ||
    CurPlugin->pFreeFindData ||
    CurPlugin->pGetVirtualFindData ||
    CurPlugin->pFreeVirtualFindData ||
    CurPlugin->pSetDirectory ||
    CurPlugin->pGetFiles ||
    CurPlugin->pPutFiles ||
    CurPlugin->pDeleteFiles ||
    CurPlugin->pMakeDirectory ||
    CurPlugin->pProcessHostFile ||
    CurPlugin->pSetFindList ||
    CurPlugin->pConfigure ||
    CurPlugin->pExitFAR ||
    CurPlugin->pProcessKey ||
    CurPlugin->pProcessEvent ||
    CurPlugin->pCompare ||
    CurPlugin->pProcessEditorInput ||
    CurPlugin->pProcessEditorEvent ||
    CurPlugin->pProcessViewerEvent ||
    CurPlugin->pMinFarVersion))
  {
    FreeLibrary(hModule);
    CurPlugin->hModule=0;
    CurPlugin->WorkFlags.Set(PIWF_DONTLOADAGAIN);
    return FALSE;
  }
  /* SKV$*/
  CurPlugin->FuncFlags.Set(PICFF_LOADED);
  if (ModuleNumber!=-1 && Init)
  {
    /* $ 22.05.2001 DJ
       проверка мин. версии перенесена в SetPluginStartupInfo()
    */
    return(SetPluginStartupInfo(CurPlugin,ModuleNumber));
    /* DJ $ */
  }
  return(TRUE);
}

/* $ 03.08.2000 tran
   функция проверки минимальной версии */
/* $ 07.12.2000 SVS
   Проверка не только версии, но и номера билда
*/
int  PluginsSet::CheckMinVersion(Plugin *CurPlugin)
{
  DWORD FVer;

  if ( CurPlugin->pMinFarVersion==0 || ProcessException) // плагин не эскпортирует, ему или неважно, или он для <1.65
    return (TRUE);

  //CurPluginItem=&CurPlugin; //??
  CurPlugin->FuncFlags.Set(PICFF_MINFARVERSION);
  if(Opt.ExceptRules)
  {
    TRY {
      FVer=(DWORD)CurPlugin->pMinFarVersion();
    }
    EXCEPT ( xfilter(EXCEPT_MINFARVERSION, GetExceptionInformation(),CurPlugin,0) )
    {
      CurPlugin->FuncFlags.Clear(PICFF_MINFARVERSION);
      UnloadPlugin(CurPlugin,EXCEPT_MINFARVERSION); // тест не пройден, выгружаем его
      ProcessException=FALSE;
      return (FALSE);
    }
  }
  else
    FVer=(DWORD)CurPlugin->pMinFarVersion();
  CurPlugin->FuncFlags.Clear(PICFF_MINFARVERSION);
  //CurPluginItem=NULL;

  if (LOWORD(FVer) >  LOWORD(FAR_VERSION) ||
     (LOWORD(FVer) == LOWORD(FAR_VERSION) &&
      HIWORD(FVer) >  HIWORD(FAR_VERSION)))
  {
    // кранты - плагин требует старший фар
    ShowMessageAboutIllegalPluginVersion(CurPlugin->strModuleName,FVer);
    return (FALSE);
  }
  return (TRUE); // нормально, свой парень
}

// выгрузка плагина
// причем без всяких ему объяснений
void PluginsSet::UnloadPlugin(Plugin *CurPlugin,DWORD Exception)
{
//_SVS(SysLog("UnloadPlugin(%s)",CurPlugin.ModuleName));
//    if(FrameManager->GetBottomFrame() != FrameManager->GetCurrentFrame())
//      FrameManager->DeleteFrame();

  CurPluginItem=NULL;
  Frame *frame;
  if((frame=FrameManager->GetBottomFrame()) != NULL)
    frame->Unlock();

  if(Flags.Check(PSIF_DIALOG)) // BugZ#52 exception handling for floating point incorrect
  {
    Flags.Clear(PSIF_DIALOG);
    FrameManager->DeleteFrame();
  }

  BOOL Ret=FreeLibrary(CurPlugin->hModule);
  CurPlugin->FuncFlags.Clear(PICFF_LOADED); //??

  // имя оставляем обязательно!!! :-(
  string strModuleName;
  strModuleName = CurPlugin->strModuleName;

  BOOL NeedUpdatePanels=CurPlugin->FuncFlags.Check(PICFF_PANELPLUGIN);

//  memset(CurPlugin,0,sizeof(PluginItem)); //BUGBUG

  CurPlugin->strModuleName = strModuleName;
  CurPlugin->WorkFlags.Set(PIWF_DONTLOADAGAIN);

  // BugZ#137 - обработка падения панельного плагина
  // здесь нужно проверить на "панельность" плагина,
  if(NeedUpdatePanels)
  {
    CtrlObject->Cp()->ActivePanel->SetCurDirW(L".",TRUE);

//    FrameManager->RefreshFrame();

    Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;

    ActivePanel->Update(UPDATE_KEEP_SELECTION);
    ActivePanel->Redraw();

    Panel *AnotherPanel=CtrlObject->Cp()->GetAnotherPanel(ActivePanel);
    AnotherPanel->Update(UPDATE_KEEP_SELECTION|UPDATE_SECONDARY);
    AnotherPanel->Redraw();
  }
}

void PluginsSet::ShowMessageAboutIllegalPluginVersion(const wchar_t* plg,int required)
{
    string strMsg1, strMsg2;
    string strPlgName;

    strPlgName = plg;
    TruncPathStrW(strPlgName,ScrX-20);
    strMsg1.Format(UMSG(MPlgRequired),
           (WORD)HIBYTE(LOWORD(required)),(WORD)LOBYTE(LOWORD(required)),HIWORD(required));
    strMsg2.Format(UMSG(MPlgRequired2),
           (WORD)HIBYTE(LOWORD(FAR_VERSION)),(WORD)LOBYTE(LOWORD(FAR_VERSION)),HIWORD(FAR_VERSION));
    MessageW(MSG_WARNING,1,UMSG(MError),UMSG(MPlgBadVers),strPlgName,strMsg1,strMsg2,UMSG(MOk));
}
/* tran 03.08.2000 $ */
/* SVS $ */

void PluginsSet::CreatePluginStartupInfo(struct PluginStartupInfo *PSI,
                                         struct FarStandardFunctions *FSF,
                                         const wchar_t *ModuleName,
                                         int ModuleNumber)
{
  static PluginStartupInfo StartupInfo={0};
  static FarStandardFunctions StandardFunctions={0};

  // заполняем структуру StandardFunctions один раз!!!
  if(!StandardFunctions.StructSize)
  {
    StandardFunctions.StructSize=sizeof(StandardFunctions);
    StandardFunctions.sprintf=swprintf;
    StandardFunctions.snprintf=_snwprintf;
    StandardFunctions.sscanf=swscanf;
    StandardFunctions.qsort=FarQsort;
    StandardFunctions.qsortex=FarQsortEx;
    StandardFunctions.atoi=FarAtoi;
    StandardFunctions.atoi64=FarAtoi64;
    StandardFunctions.itoa=FarItoa;
    StandardFunctions.itoa64=FarItoa64;
    StandardFunctions.bsearch=FarBsearch;
    StandardFunctions.LIsLower   =LocalIslowerW;
    StandardFunctions.LIsUpper   =LocalIsupperW;
    StandardFunctions.LIsAlpha   =LocalIsalphaW;
    StandardFunctions.LIsAlphanum=LocalIsalphanumW;
    StandardFunctions.LUpper     =LocalUpperW;
    StandardFunctions.LUpperBuf  =LocalUpperBufW;
    StandardFunctions.LLowerBuf  =LocalLowerBufW;
    StandardFunctions.LLower     =LocalLowerW;
    StandardFunctions.LStrupr    =LocalStruprW;
    StandardFunctions.LStrlwr    =LocalStrlwrW;
    StandardFunctions.LStricmp   =LocalStricmpW;
    StandardFunctions.LStrnicmp  =LocalStrnicmpW;
    /* SVS $ */

    StandardFunctions.Unquote=UnquoteW;
    StandardFunctions.LTrim=RemoveLeadingSpacesW;
    StandardFunctions.RTrim=RemoveTrailingSpacesW;
    StandardFunctions.Trim=RemoveExternalSpacesW;
    StandardFunctions.TruncStr=TruncStrW;
    StandardFunctions.TruncPathStr=TruncPathStrW;
    StandardFunctions.QuoteSpaceOnly=QuoteSpaceOnlyW;
    StandardFunctions.PointToName=PointToNameW;
    //StandardFunctions.GetPathRoot=GetPathRoot; BUGBUG
    StandardFunctions.AddEndSlash=AddEndSlashW;
    StandardFunctions.CopyToClipboard=CopyToClipboardW;
    StandardFunctions.PasteFromClipboard=PasteFromClipboardW;
    //StandardFunctions.FarKeyToName=KeyToText; //BUGBUG
    ///StandardFunctions.FarNameToKey=KeyNameToKey; //BUGBUG
    StandardFunctions.FarInputRecordToKey=InputRecordToKey;
    StandardFunctions.XLat=Xlat;
    //StandardFunctions.GetFileOwner=GetFileOwner; //BUGBUG
    StandardFunctions.GetNumberOfLinks=GetNumberOfLinksW;
    StandardFunctions.FarRecursiveSearch=FarRecursiveSearch;
    StandardFunctions.MkTemp=FarMkTemp;
    StandardFunctions.DeleteBuffer=DeleteBuffer;
    StandardFunctions.ProcessName=ProcessName;
    StandardFunctions.MkLink=FarMkLinkW;
    //StandardFunctions.ConvertNameToReal=ConvertNameToReal; //BUGBUG
    //StandardFunctions.GetReparsePointInfo=FarGetReparsePointInfo; //BUGBUG
  }

  if(!StartupInfo.StructSize)
  {
    StartupInfo.StructSize=sizeof(StartupInfo);
    StartupInfo.Menu=FarMenuFn; //BUGBUG
    StartupInfo.Dialog=FarDialogFn;
    StartupInfo.GetMsg=FarGetMsgFn;
    StartupInfo.Message=FarMessageFn;
    StartupInfo.Control=FarControl;
    StartupInfo.SaveScreen=FarSaveScreen;
    StartupInfo.RestoreScreen=FarRestoreScreen;
    StartupInfo.GetDirList=FarGetDirList;
    StartupInfo.GetPluginDirList=FarGetPluginDirList;
    StartupInfo.FreeDirList=FarFreeDirList;
    StartupInfo.Viewer=FarViewer;
    StartupInfo.Editor=FarEditor;
    StartupInfo.CmpName=FarCmpName;
    StartupInfo.CharTable=FarCharTable;
    StartupInfo.Text=FarText;
    StartupInfo.EditorControl=FarEditorControl;
    StartupInfo.ViewerControl=FarViewerControl;
    StartupInfo.ShowHelp=FarShowHelp;
    StartupInfo.AdvControl=FarAdvControl;
    StartupInfo.DialogEx=FarDialogEx;
    StartupInfo.SendDlgMessage=FarSendDlgMessage;
    StartupInfo.DefDlgProc=FarDefDlgProc;
    StartupInfo.InputBox=FarInputBox;
  }

  memcpy(PSI,&StartupInfo,sizeof(StartupInfo));
  memcpy(FSF,&StandardFunctions,sizeof(StandardFunctions));
  PSI->ModuleNumber=ModuleNumber;
  PSI->FSF=FSF;

  PSI->ModuleName = ModuleName;
  PSI->RootKey=NULL;
}

/* $ 22.05.2001 DJ
   проверка мин. версии перенесена в SetPluginStartupInfo()
*/
int PluginsSet::SetPluginStartupInfo(Plugin *CurPlugin,int ModuleNumber)
{
  _ALGO(CleverSysLog clv("PluginsSet::SetPluginStartupInfo()"));
  _ALGO(SysLog("ModuleNumber=%d",ModuleNumber));
  //PrepareModulePath(CurPlugin.ModuleName);

  /* $ 03.08.2000 tran
     проверка на минимальную версию фара */
  if (!CheckMinVersion(CurPlugin))
  {
    UnloadPlugin(CurPlugin,-1); // тест не пройден, выгружаем его
    return (FALSE);
  }

  if (CurPlugin->pSetStartupInfo!=NULL && !ProcessException)
  {
    // Это есть локальные копии статических структур, что бы случайно
    // како-нить урод не засрал адреса.
    PluginStartupInfo LocalStartupInfo;
    FarStandardFunctions LocalStandardFunctions;

    CreatePluginStartupInfo(&LocalStartupInfo,&LocalStandardFunctions,CurPlugin->strModuleName,ModuleNumber);

    // скорректирем адреса и плагино-зависимые поля
    CurPlugin->strRootKey = Opt.strRegRoot;
    CurPlugin->strRootKey += L"\\Plugins";
    LocalStartupInfo.RootKey=CurPlugin->strRootKey;

    //CurPluginItem=&CurPlugin;
    CurPlugin->FuncFlags.Set(PICFF_SETSTARTUPINFO);
    if(Opt.ExceptRules)
    {
      TRY {
        CurPlugin->pSetStartupInfo(&LocalStartupInfo);
      }
      EXCEPT(xfilter(EXCEPT_SETSTARTUPINFO,GetExceptionInformation(),CurPlugin,0)){
        CurPlugin->FuncFlags.Clear(PICFF_SETSTARTUPINFO);
        UnloadPlugin(CurPlugin,EXCEPT_SETSTARTUPINFO); // тест не пройден, выгружаем его
        ProcessException=FALSE;
        return FALSE;
      }
    }
    else
      CurPlugin->pSetStartupInfo(&LocalStartupInfo);
    //CurPluginItem=NULL;
    CurPlugin->FuncFlags.Clear(PICFF_SETSTARTUPINFO);
  }
  return TRUE;
}
/* DJ $ */


int PluginsSet::PreparePlugin(int PluginNumber)
{
  Plugin *PData=PluginsData[PluginNumber];
  if (!PData->WorkFlags.Check(PIWF_CACHED))
    return(TRUE);
  return(LoadPlugin(PData,PluginNumber,TRUE));
}


int PluginsSet::GetCacheNumber(const wchar_t *FullName,FAR_FIND_DATA_EX *FindData,int CachePos)
{
  for (int I=-1;;I++)
  {
    if (I==-1 && CachePos==0)
      continue;
    int Pos=(I==-1) ? CachePos:I;

    string strRegKey, strPluginName, strPluginID, strCurPluginID;
    strRegKey.Format (FmtPluginsCache_PluginDW,Pos);
    GetRegKeyW(strRegKey,L"Name",strPluginName,L"");
    if ( strPluginName.IsEmpty() )
      break;
    if ( LocalStricmpW(strPluginName, FullName)!=0)
      continue;
    if (FindData!=NULL)
    {
      GetRegKeyW(strRegKey,L"ID",strPluginID,L"");
      strCurPluginID.Format (L"%I64x%x%x",FindData->nFileSize,
              FindData->ftCreationTime.dwLowDateTime,
              FindData->ftLastWriteTime.dwLowDateTime);
      if (wcscmp(strPluginID,strCurPluginID)!=0)
        continue;
    }
    return(Pos);
  }
  return(-1);
}


int PluginsSet::SavePluginSettings(Plugin *CurPlugin,
                                    FAR_FIND_DATA_EX &FindData)
{
  if(!(CurPlugin->pGetPluginInfo     ||
      CurPlugin->pOpenPlugin         ||
      CurPlugin->pOpenFilePlugin     ||
      CurPlugin->pSetFindList        ||
      CurPlugin->pProcessEditorInput ||
      CurPlugin->pProcessEditorEvent ||
      CurPlugin->pProcessViewerEvent
  // Сюда добавлять те функции, из-за которых плагин имеет место быть в кэше
  ))
   return FALSE;

  struct PluginInfoW Info;
  memset(&Info,0,sizeof(Info));

  if(CurPlugin->pGetPluginInfo && !ProcessException)
  {
    //CurPluginItem=&CurPlugin;
    CurPlugin->FuncFlags.Set(PICFF_GETPLUGININFO);
    if(Opt.ExceptRules)
    {
      TRY {
        CurPlugin->pGetPluginInfo(&Info);
      }
      EXCEPT(xfilter(EXCEPT_GETPLUGININFO,GetExceptionInformation(),CurPlugin,0)){
        CurPlugin->FuncFlags.Clear(PICFF_GETPLUGININFO);
        UnloadPlugin(CurPlugin,EXCEPT_GETPLUGININFO); // тест не пройден, выгружаем его
        ProcessException=FALSE;
        return FALSE;
      }
      if(!TestPluginInfo(CurPlugin,&Info))
      {
        CurPlugin->FuncFlags.Clear(PICFF_GETPLUGININFO);
        return FALSE;
      }
    }
    else
      CurPlugin->pGetPluginInfo(&Info);
    //CurPluginItem=NULL;
    CurPlugin->FuncFlags.Clear(PICFF_GETPLUGININFO);
  }

  CurPlugin->SysID=Info.SysID;

  int I,I0;
  for (I0=0;;I0++)
  {
    string strRegKey, strPluginName, strCurPluginID;
    strRegKey.Format (FmtPluginsCache_PluginDW,I0);
    GetRegKeyW(strRegKey,L"Name",strPluginName,L"");
    if ( strPluginName.IsEmpty() || LocalStricmpW(strPluginName,CurPlugin->strModuleName)==0)
    {
      DeleteKeyTreeW(strRegKey);

      SetRegKeyW(strRegKey,L"Name",CurPlugin->strModuleName);
      strCurPluginID.Format (L"%I64x%x%x",FindData.nFileSize,
              FindData.ftCreationTime.dwLowDateTime,
              FindData.ftLastWriteTime.dwLowDateTime);
      SetRegKeyW(strRegKey,L"ID",strCurPluginID);
      /* $ 12.10.2000 tran
         если плагин PRELOAD, в кеш пишется об этом */
      if (Info.Flags & PF_PRELOAD)
      {
        SetRegKeyW(strRegKey,NFMP_PreloadW,1);
        CurPlugin->WorkFlags.Set(PIWF_PRELOADED);
        break;
      }
      else
      {
        SetRegKeyW(strRegKey,NFMP_PreloadW,(DWORD)0);
        CurPlugin->WorkFlags.Clear(PIWF_PRELOADED);
      }
      /* tran $ */

      for (I=0;I<Info.DiskMenuStringsNumber;I++)
      {
        string strValue;
        strValue.Format (FmtDiskMenuStringDW,I);
        SetRegKeyW(strRegKey,strValue,Info.DiskMenuStrings[I]);
        if (Info.DiskMenuNumbers)
        {
          strValue.Format (FmtDiskMenuNumberDW,I);
          SetRegKeyW(strRegKey,strValue,Info.DiskMenuNumbers[I]);
        }
      }
      for (I=0;I<Info.PluginMenuStringsNumber;I++)
      {
        string strValue;
        strValue.Format (FmtPluginMenuStringDW,I);
        SetRegKeyW(strRegKey,strValue,Info.PluginMenuStrings[I]);
      }
      for (I=0;I<Info.PluginConfigStringsNumber;I++)
      {
        string strValue;
        strValue.Format (FmtPluginConfigStringDW,I);
        SetRegKeyW(strRegKey,strValue,Info.PluginConfigStrings[I]);
      }
      SetRegKeyW(strRegKey,L"CommandPrefix",NullToEmptyW(Info.CommandPrefix));
      SetRegKeyW(strRegKey,L"Flags",Info.Flags);

      strRegKey.Format (FmtPluginsCache_PluginDExportW,I0);

      SetRegKeyW (strRegKey,NFMP_SysIDW,CurPlugin->SysID);
      SetRegKeyW (strRegKey,NFMP_OpenPluginW,CurPlugin->pOpenPlugin!=NULL);
      SetRegKeyW (strRegKey,NFMP_OpenFilePluginW,CurPlugin->pOpenFilePlugin!=NULL);
      SetRegKeyW (strRegKey,NFMP_SetFindListW,CurPlugin->pSetFindList!=NULL);
      SetRegKeyW (strRegKey,NFMP_ProcessEditorInputW,CurPlugin->pProcessEditorInput!=NULL);
      SetRegKeyW (strRegKey,NFMP_ProcessEditorEventW,CurPlugin->pProcessEditorEvent!=NULL);
      SetRegKeyW (strRegKey,NFMP_ProcessViewerEventW,CurPlugin->pProcessViewerEvent!=NULL);
      break;
    }
  }
  /* IS $ */
  return(TRUE);
}


HANDLE PluginsSet::OpenPlugin(int PluginNumber,int OpenFrom,INT_PTR Item)
{
  _ALGO(CleverSysLog clv("PluginsSet::OpenPlugin()"));
  _ALGO(SysLog("PluginNumber=%d, OpenFrom=%d, Item=%d",PluginNumber,OpenFrom,Item));
  ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
  CheckScreenLock();
  string strCurDir;
  CtrlObject->CmdLine->GetCurDirW(strCurDir);
  FarChDirW(strCurDir);
  g_strDirToSet=L"";
  HANDLE hInternal=0;
  if (PluginNumber<PluginsCount)
  {
    Plugin *PData=PluginsData[PluginNumber];
    if (PData->pOpenPlugin && PreparePlugin(PluginNumber) && !ProcessException)
    {
      BOOL IsUnload=FALSE;
      {
        _KEYMACRO(CleverSysLog clv("PluginsSet::OpenPlugin()"));
        _KEYMACRO(SysLog("**** Enter to Plugin **** (%s)",PData->ModuleName));
        /* $ 16.10.2000 SVS
           + Обработка исключений при вызове галимого плагина.
        */
        CurPluginItem=PData;
        PData->FuncFlags.Set(PICFF_OPENPLUGIN);
        CtrlObject->Macro.SetRedrawEditor(FALSE);
        Flags.Set(PSIF_ENTERTOOPENPLUGIN);
        if(Opt.ExceptRules)
        {
          _ECTLLOG(CleverSysLog SL(NFMP_OpenPlugin));
          TRY {
             _SVS(SysLog("OPENPLUGIN >>> '%s'",(char *)Item));
             hInternal=PData->pOpenPlugin(OpenFrom,Item);
             _SVS(SysLog("OPENPLUGIN <<< '%s'",(char *)Item));
             /* $ 26.02.2201 VVM
                 ! Выгрузить плагин, если вернули NULL */
             if (!hInternal)
               RaiseException(STATUS_INVALIDFUNCTIONRESULT, 0, 0, 0);
             /* VVM $ */
             if(CtrlObject->Plugins.Flags.Check(PSIF_DIALOG))
               RaiseException(EXCEPT_FARDIALOG, 0, 0, 0);
          }
          EXCEPT(xfilter(EXCEPT_OPENPLUGIN,GetExceptionInformation(),PData,1)){
            IsUnload=TRUE;
          }
        }
        else
        {
          hInternal=PData->pOpenPlugin(OpenFrom,Item);
          // при отключенной обработке не выгружаем плагин.
        }
        PData->FuncFlags.Clear(PICFF_OPENPLUGIN);
        CurPluginItem=NULL;
        Flags.Clear(PSIF_ENTERTOOPENPLUGIN);
        CtrlObject->Macro.SetRedrawEditor(TRUE);
        _KEYMACRO(SysLog("**** Leave from Plugin **** (%s)",PData->ModuleName));
      }
      /* SVS $ */
      /*$ 10.08.2000 skv
        If we are in editor mode, and CurEditor defined,
        we need to call this events.
        EE_REDRAW 2 - to notify that text changed.
        EE_REDRAW 0 - to notify that whole screen updated
        ->Show() to actually update screen.

        This duplication take place since ShowEditor method
        will NOT send this event while screen is locked.
      */
      if(OpenFrom == OPEN_EDITOR &&
         !CtrlObject->Macro.IsExecuting() &&
         CtrlObject->Plugins.CurEditor &&
         CtrlObject->Plugins.CurEditor->IsVisible() && !IsUnload)
      {
//_SVS(SysLog("**** Enter to EE_REDRAW (return from Plugin)"));
        CtrlObject->Plugins.ProcessEditorEvent(EE_REDRAW,EEREDRAW_CHANGE);
        CtrlObject->Plugins.ProcessEditorEvent(EE_REDRAW,EEREDRAW_ALL);
        CtrlObject->Plugins.CurEditor->Show();
      }
      /* skv$*/
      if(IsUnload)
      {
        UnloadPlugin(PData,EXCEPT_OPENPLUGIN);
        ProcessException=FALSE;
        return(INVALID_HANDLE_VALUE);
      }

      if (hInternal!=INVALID_HANDLE_VALUE)
      {
        PluginHandle *hPlugin=new PluginHandle;
        hPlugin->InternalHandle=hInternal;
        hPlugin->PluginNumber=PluginNumber;
        return((HANDLE)hPlugin);
      }
      else
        if ( !g_strDirToSet.IsEmpty() )
        {
          CtrlObject->Cp()->ActivePanel->SetCurDirW(g_strDirToSet,TRUE);
          CtrlObject->Cp()->ActivePanel->Redraw();
        }
    }
  }
  return(INVALID_HANDLE_VALUE);
}


HANDLE PluginsSet::OpenFilePlugin(const wchar_t *Name,const unsigned char *Data,int DataSize)
{
  ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
  /* $ 20.03.2001 tran */
  ConsoleTitle ct(Opt.ShowCheckingFile?UMSG(MCheckingFileInPlugin):NULL);
  /* tran $ */

  Plugin *PData;
  for (int I=0;I<PluginsCount;I++)
  {
    PData = PluginsData[I];
    if (PData->pOpenFilePlugin && PreparePlugin(I) && !ProcessException)
    {
      char *NamePtr=NULL;

      string strFullName;


      if (Name!=NULL)
        ConvertNameToFullW(Name,strFullName);

      HANDLE hInternal;
      /* $ 20.03.2001 tran */
      if(Opt.ShowCheckingFile)
      {
        ct.Set(L"%s - [%s]...",UMSG(MCheckingFileInPlugin),wcsrchr(PluginsData[I]->strModuleName,L'\\')+1);
      }
      /* tran $ */
      PData->FuncFlags.Set(PICFF_OPENFILEPLUGIN);

      if(Opt.ExceptRules)
      {
        TRY
        {
           hInternal=PData->pOpenFilePlugin(strFullName,Data,DataSize);
           if (!hInternal)
             RaiseException(STATUS_INVALIDFUNCTIONRESULT, 0, 0, 0);
           //????????????????????????????????????????????????????
        }
        EXCEPT(xfilter(EXCEPT_OPENFILEPLUGIN,GetExceptionInformation(),PData,1)){
          UnloadPlugin(PData,EXCEPT_OPENFILEPLUGIN);
          hInternal=INVALID_HANDLE_VALUE;
          ProcessException=FALSE;
        }
      }
      else
      {
        hInternal=PData->pOpenFilePlugin(strFullName,Data,DataSize);
        // при отключенной обработке не выгружаем плагин.
      }

      PData->FuncFlags.Clear(PICFF_OPENFILEPLUGIN);

      if (hInternal==(HANDLE)-2)
        return((HANDLE)-2);
      if (hInternal!=INVALID_HANDLE_VALUE)
      {
        PluginHandle *hPlugin=new PluginHandle;
        hPlugin->InternalHandle=hInternal;
        hPlugin->PluginNumber=I;
        return((HANDLE)hPlugin);
      }
    }
  }
  return(INVALID_HANDLE_VALUE);
}


HANDLE PluginsSet::OpenFindListPlugin(const PluginPanelItemW *PanelItem,int ItemsNumber)
{
  _ALGO(CleverSysLog clv("PluginsSet::OpenFindListPlugin()"));
  _ALGO(SysLog("PanelItem=%p, ItemsNumber=%d",PanelItem,ItemsNumber));
  ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
  Plugin *PData;
  for (int I=0;I<PluginsCount;I++)
  {
    PData = PluginsData[I];
    if (PData->pOpenPlugin && PData->pSetFindList && PreparePlugin(I) && !ProcessException)
    {
      /* $ 26.02.2001 VVM
          + Обработка исключения при OpenPlugin(OPEN_FINDLIST) */
      HANDLE hInternal;
      PData->FuncFlags.Set(PICFF_OPENPLUGIN);
      if(Opt.ExceptRules)
      {
        TRY {
           hInternal = PData->pOpenPlugin(OPEN_FINDLIST,0);
           /* $ 26.02.2201 VVM
               ! Выгрузить плагин, если вернули NULL */
           if (!hInternal)
             RaiseException(STATUS_INVALIDFUNCTIONRESULT, 0, 0, 0);
           //????????????????????????????????????????????????????
           /* VVM $ */
        }
        EXCEPT(xfilter(EXCEPT_OPENPLUGIN,GetExceptionInformation(),PData,1)){
          UnloadPlugin(PData,EXCEPT_OPENPLUGIN);
          hInternal=INVALID_HANDLE_VALUE;
          ProcessException=FALSE;
        }
        /* VVM $ */
      }
      else
      {
        hInternal = PData->pOpenPlugin(OPEN_FINDLIST,0);
        // при отключенной обработке не выгружаем плагин.
      }
      PData->FuncFlags.Clear(PICFF_OPENPLUGIN);

      if (hInternal!=INVALID_HANDLE_VALUE)
      {
        //EXCEPTION_POINTERS *xp;
        BOOL Ret;

        PData->FuncFlags.Set(PICFF_SETFINDLIST);
        if(Opt.ExceptRules)
        {
          TRY {
            Ret=PData->pSetFindList(hInternal,PanelItem,ItemsNumber);
          }
          EXCEPT(xfilter(EXCEPT_SETFINDLIST,GetExceptionInformation(),PData,1)){
             UnloadPlugin(PData,EXCEPT_SETFINDLIST);
             Ret=FALSE;
             ProcessException=FALSE; //???
          }
        }
        else
          Ret=PData->pSetFindList(hInternal,PanelItem,ItemsNumber);
        PData->FuncFlags.Clear(PICFF_SETFINDLIST);

        if (!Ret)
          continue;

        PluginHandle *hPlugin=new PluginHandle;
        hPlugin->InternalHandle=hInternal;
        hPlugin->PluginNumber=I;
        return((HANDLE)hPlugin);
      }
    }
  }
  return(INVALID_HANDLE_VALUE);
}


void PluginsSet::ClosePlugin(HANDLE hPlugin)
{
  _ALGO(CleverSysLog clv("PluginsSet::ClosePlugin()"));
  _ALGO(SysLog("hPlugin=0x%08X",hPlugin));
  ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
  struct PluginHandle *ph=(struct PluginHandle *)hPlugin;
  Plugin *PData=PluginsData[ph->PluginNumber];
  if (PData->pClosePlugin && !ProcessException)
  {
    //CurPluginItem=PData;
    PData->FuncFlags.Set(PICFF_CLOSEPLUGIN);
    if(Opt.ExceptRules)
    {
      TRY {
        PData->pClosePlugin(ph->InternalHandle);
      }
      EXCEPT(xfilter(EXCEPT_CLOSEPLUGIN,GetExceptionInformation(),PData,1))
      {
        UnloadPlugin(PData,EXCEPT_CLOSEPLUGIN);
        ProcessException=FALSE;
      }
    }
    else
      PData->pClosePlugin(ph->InternalHandle);
    PData->FuncFlags.Clear(PICFF_CLOSEPLUGIN);
    //CurPluginItem=NULL;
  }
  delete ph;
}


int PluginsSet::ProcessEditorInput(INPUT_RECORD *Rec)
{
  _ALGO(CleverSysLog clv("PluginsSet::ProcessEditorInput()"));
  _ALGO(SysLog("Rec=%s",_INPUT_RECORD_Dump(Rec)));
  //EXCEPTION_POINTERS *xp;
  Plugin *PData;
  _KEYMACRO(CleverSysLog SL("PluginsSet::ProcessEditorInput()"));
  for (int I=0;I<PluginsCount;I++)
  {
    PData = PluginsData[I];
    if (PData->pProcessEditorInput && PreparePlugin(I) && !ProcessException)
    {
      /* $ 13.07.2000 IS
         Фиксит трап при входе в редактор (подсказал tran)
      */
      if (PData->pProcessEditorInput)
      {
        int Ret;
        _KEYMACRO(SysLog("CALL pProcessEditorInput(): '%s'",PData->ModuleName));

        PData->FuncFlags.Set(PICFF_PROCESSEDITORINPUT);
        if(Opt.ExceptRules)
        {
          TRY {
            Ret=PData->pProcessEditorInput(Rec);
          }
          EXCEPT(xfilter(EXCEPT_PROCESSEDITORINPUT,GetExceptionInformation(),PData,1))
          {
            UnloadPlugin(PData,EXCEPT_PROCESSEDITORINPUT); //????!!!!
            Ret=FALSE;
            ProcessException=FALSE; //??
          }
        }
        else
          Ret=PData->pProcessEditorInput(Rec);
        PData->FuncFlags.Clear(PICFF_PROCESSEDITORINPUT);

        _KEYMACRO(SysLog("Ret=%d",Ret));
        if(Ret)
          return(TRUE);
        /* IS $ */
      }
    }
  }
  return(FALSE);
}


int PluginsSet::ProcessEditorEvent(int Event,void *Param)
{
  _ALGO(CleverSysLog clv("PluginsSet::ProcessEditorEvent()"));
  _ALGO(SysLog("Params: Event=%s, Param=%s",_EE_ToName(Event),_EEREDRAW_ToName((int)Param)));
  _ECTLLOG(CleverSysLog Clev("PluginsSet::ProcessEditorEvent()"));
  int Ret=0;
  //EXCEPTION_POINTERS *xp;
  if(CtrlObject->Plugins.CurEditor)
  {
    Plugin *PData;
    for (int I=0;I<PluginsCount;I++)
    {
      PData = PluginsData[I];
      if (PData->pProcessEditorEvent && PreparePlugin(I) && !ProcessException)
      {
        //_ECTLLOG(CleverSysLog Clev2("Call ProcessEditorEvent()"));
        _ECTLLOG(SysLog("Plugin: '%s'",PData->ModuleName));
        _ECTLLOG(SysLog("Params: Event=%s, Param=%s",_EE_ToName(Event),_EEREDRAW_ToName((int)Param)));
        PData->FuncFlags.Set(PICFF_PROCESSEDITOREVENT);
        if(Opt.ExceptRules)
        {
          TRY {
            Ret=PData->pProcessEditorEvent(Event,Param);
          }
          EXCEPT(xfilter(EXCEPT_PROCESSEDITOREVENT,GetExceptionInformation(),PData,1))
          {
            UnloadPlugin(PData,EXCEPT_PROCESSEDITOREVENT);
            ProcessException=FALSE;
          }
        }
        else
          Ret=PData->pProcessEditorEvent(Event,Param);
        _ECTLLOG(SysLog("Return=%d",Ret));
        PData->FuncFlags.Clear(PICFF_PROCESSEDITOREVENT);
      }
    }
  }
  return Ret;
}


/* $ 27.09.2000 SVS
   События во вьювере
*/
int PluginsSet::ProcessViewerEvent(int Event,void *Param)
{
  _ALGO(CleverSysLog clv("PluginsSet::ProcessViewerEvent()"));
  //EXCEPTION_POINTERS *xp;
  Plugin *PData;
  int Ret=0;
  for (int I=0;I<PluginsCount;I++)
  {
    PData = PluginsData[I];
    if (PData->pProcessViewerEvent && PreparePlugin(I) && !ProcessException)
    {
      PData->FuncFlags.Set(PICFF_PROCESSVIEWEREVENT);
      if(Opt.ExceptRules)
      {
        TRY {
          Ret=PData->pProcessViewerEvent(Event,Param);
        }
        EXCEPT(xfilter(EXCEPT_PROCESSVIEWEREVENT,GetExceptionInformation(),PData,1))
        {
          UnloadPlugin(PData,EXCEPT_PROCESSVIEWEREVENT);
          ProcessException=FALSE;
        }
      }
      else
        Ret=PData->pProcessViewerEvent(Event,Param);
      PData->FuncFlags.Clear(PICFF_PROCESSVIEWEREVENT);
    }
  }
  return Ret;
}
/* SVS $ */

int PluginsSet::GetFindData(HANDLE hPlugin,PluginPanelItemW **pPanelData,int *pItemsNumber,int OpMode)
{
  _ALGO(CleverSysLog clv("PluginsSet::GetFindData()"));
  _ALGO(SysLog("hPlugin=%p, pPanelData=%p, pItemsNumber=%p, OpMode=%d",hPlugin,pPanelData,pItemsNumber,OpMode));
  //EXCEPTION_POINTERS *xp;
  ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
  struct PluginHandle *ph=(struct PluginHandle *)hPlugin;
  Plugin *PData=PluginsData[ph->PluginNumber];
  *pItemsNumber=0;
  if (PData->pGetFindData && !ProcessException)
  {
    int Ret;
    //CurPluginItem=PData;
    PData->FuncFlags.Set(PICFF_GETFINDDATA);
    if(Opt.ExceptRules)
    {
      TRY {
        Ret=PData->pGetFindData(ph->InternalHandle,pPanelData,pItemsNumber,OpMode);
      }
      EXCEPT(xfilter(EXCEPT_GETFINDDATA,GetExceptionInformation(),PData,1))
      {
        UnloadPlugin(PData,EXCEPT_GETFINDDATA); //????!!!!
        Ret=FALSE;
        ProcessException=FALSE; //??
      }
    }
    else
      Ret=PData->pGetFindData(ph->InternalHandle,pPanelData,pItemsNumber,OpMode);
    //CurPluginItem=NULL;
    PData->FuncFlags.Clear(PICFF_GETFINDDATA);
    _ALGO(SysLog("Ret=%d",Ret));
    return(Ret);
  }
  return(FALSE);
}


void PluginsSet::FreeFindData(HANDLE hPlugin,PluginPanelItemW *PanelItem,int ItemsNumber)
{
  _ALGO(CleverSysLog clv("PluginsSet::FreeFindData()"));
  _ALGO(SysLog("hPlugin=%p, PanelItem=%p, ItemsNumber=%d",hPlugin,PanelItem,ItemsNumber));
  //EXCEPTION_POINTERS *xp;
  struct PluginHandle *ph=(struct PluginHandle *)hPlugin;
  Plugin *PData=PluginsData[ph->PluginNumber];
  if (PData->pFreeFindData && !ProcessException)
  {
    //CurPluginItem=PData;
    PData->FuncFlags.Set(PICFF_FREEFINDDATA);
    if(Opt.ExceptRules)
    {
      TRY {
        PData->pFreeFindData(ph->InternalHandle,PanelItem,ItemsNumber);
      }
      EXCEPT(xfilter(EXCEPT_FREEFINDDATA,GetExceptionInformation(),PData,1))
      {
        UnloadPlugin(PData,EXCEPT_FREEFINDDATA); //????!!!!
        ProcessException=FALSE;
      }
    }
    else
      PData->pFreeFindData(ph->InternalHandle,PanelItem,ItemsNumber);
    //CurPluginItem=NULL;
    PData->FuncFlags.Clear(PICFF_FREEFINDDATA);
  }
}


int PluginsSet::GetVirtualFindData(HANDLE hPlugin,PluginPanelItemW **pPanelData,int *pItemsNumber,const wchar_t *Path)
{
  _ALGO(CleverSysLog clv("PluginsSet::GetVirtualFindData()"));
  //_ALGO(SysLog("hPlugin=%p, PanelItem=%p, ItemsNumber=%d",hPlugin,PanelItem,ItemsNumber));
  ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
  struct PluginHandle *ph=(struct PluginHandle *)hPlugin;
  Plugin *PData=PluginsData[ph->PluginNumber];
  *pItemsNumber=0;
  if (PData->pGetVirtualFindData && !ProcessException)
  {
    int Ret;
    PData->FuncFlags.Set(PICFF_GETVIRTUALFINDDATA);
    //CurPluginItem=PData;
    if(Opt.ExceptRules)
    {
      TRY{
        Ret=PData->pGetVirtualFindData(ph->InternalHandle,pPanelData,pItemsNumber,Path);
      }
      EXCEPT(xfilter(EXCEPT_GETVIRTUALFINDDATA,GetExceptionInformation(),PData,1))
      {
        UnloadPlugin(PData,EXCEPT_GETVIRTUALFINDDATA); //????!!!!
        Ret=FALSE;
        ProcessException=FALSE;
      }
    }
    else
      Ret=PData->pGetVirtualFindData(ph->InternalHandle,pPanelData,pItemsNumber,Path);
    //CurPluginItem=NULL;
    PData->FuncFlags.Clear(PICFF_GETVIRTUALFINDDATA);
    return(Ret);
  }
  return(FALSE);
}


void PluginsSet::FreeVirtualFindData(HANDLE hPlugin,PluginPanelItemW *PanelItem,int ItemsNumber)
{
  _ALGO(CleverSysLog clv("PluginsSet::FreeVirtualFindData()"));
  _ALGO(SysLog("hPlugin=%p, PanelItem=%p, ItemsNumber=%d",hPlugin,PanelItem,ItemsNumber));
  struct PluginHandle *ph=(struct PluginHandle *)hPlugin;
  Plugin *PData=PluginsData[ph->PluginNumber];
  if (PData->pFreeVirtualFindData && !ProcessException)
  {
    //CurPluginItem=PData;
    PData->FuncFlags.Set(PICFF_FREEVIRTUALFINDDATA);
    if(Opt.ExceptRules)
    {
      TRY {
        PData->pFreeVirtualFindData(ph->InternalHandle,PanelItem,ItemsNumber);
      }
      EXCEPT(xfilter(EXCEPT_FREEVIRTUALFINDDATA,GetExceptionInformation(),PData,1))
      {
        UnloadPlugin(PData,EXCEPT_FREEVIRTUALFINDDATA); //????!!!!
        ProcessException=FALSE;
      }
    }
    else
      PData->pFreeVirtualFindData(ph->InternalHandle,PanelItem,ItemsNumber);
    //CurPluginItem=NULL;
    PData->FuncFlags.Clear(PICFF_FREEVIRTUALFINDDATA);
  }
}


int PluginsSet::SetDirectory(HANDLE hPlugin,const wchar_t *Dir,int OpMode)
{
  _ALGO(CleverSysLog clv("PluginsSet::SetDirectory()"));
  _ALGO(SysLog("hPlugin=%p, Dir='%s', OpMode=%d",hPlugin,(Dir?Dir:"(NULL)"),OpMode));
  //EXCEPTION_POINTERS *xp;
  ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
  struct PluginHandle *ph=(struct PluginHandle *)hPlugin;
  Plugin *PData=PluginsData[ph->PluginNumber];
  if (PData->pSetDirectory && !ProcessException)
  {
    int Ret;
    //CurPluginItem=PData;
    PData->FuncFlags.Set(PICFF_SETDIRECTORY);

    if(Opt.ExceptRules)
    {
      TRY{
        Ret=PData->pSetDirectory(ph->InternalHandle,Dir,OpMode);
      }
      EXCEPT(xfilter(EXCEPT_SETDIRECTORY,GetExceptionInformation(),PData,1))
      {
        UnloadPlugin(PData,EXCEPT_SETDIRECTORY); //????!!!!
        Ret=FALSE;
        ProcessException=FALSE;
      }
    }
    else
      Ret=PData->pSetDirectory(ph->InternalHandle,Dir,OpMode);

    //CurPluginItem=NULL;
    PData->FuncFlags.Clear(PICFF_SETDIRECTORY);
    return(Ret);
  }
  return(FALSE);
}


int PluginsSet::GetFile(HANDLE hPlugin,struct PluginPanelItemW *PanelItem,
                        const wchar_t *DestPath,string &strResultName,int OpMode)
{
  _ALGO(CleverSysLog clv("PluginsSet::GetFile()"));
  _ALGO(SysLog("hPlugin=%p, PanelItem=%p, DestPath='%s', ResultName='%s', OpMode=%d",hPlugin,PanelItem,(DestPath?DestPath:"(NULL)"),(ResultName?ResultName:"(NULL)"),OpMode));
  //EXCEPTION_POINTERS *xp;
  ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
  struct PluginHandle *ph=(struct PluginHandle *)hPlugin;
  Plugin *PData=PluginsData[ph->PluginNumber];
  SaveScreen *SaveScr=NULL;
  int Found=FALSE;
  KeepUserScreen=FALSE;
  if (PData->pGetFiles && !ProcessException)
  {
    if ((OpMode & OPM_FIND)==0)
      SaveScr=new SaveScreen;
    UndoGlobalSaveScrPtr UndSaveScr(SaveScr);

    int GetCode;
    //CurPluginItem=PData;
    PData->FuncFlags.Set(PICFF_GETFILES);

    if(Opt.ExceptRules)
    {
      TRY{
        GetCode=PData->pGetFiles(ph->InternalHandle,PanelItem,1,0,DestPath,OpMode);
      }
      EXCEPT(xfilter(EXCEPT_GETFILES,GetExceptionInformation(),PData,1))
      {
        PData->FuncFlags.Clear(PICFF_GETFILES);
        UnloadPlugin(PData,EXCEPT_GETFILES); //????!!!!
        ProcessException=FALSE;
        // ??????????
        ReadUserBackgound(SaveScr);
        delete SaveScr;
        // ??????????
        return(Found);
      }
    }
    else
      GetCode=PData->pGetFiles(ph->InternalHandle,PanelItem,1,0,DestPath,OpMode);

    //CurPluginItem=NULL;
    PData->FuncFlags.Clear(PICFF_GETFILES);

    string strFindPath;
    strFindPath = DestPath;
    AddEndSlashW(strFindPath);
    strFindPath += L"*.*";
    HANDLE FindHandle;
    FAR_FIND_DATA_EX fdata;
    if ((FindHandle=apiFindFirstFile(strFindPath,&fdata))!=INVALID_HANDLE_VALUE)
    {
      int Done=0;
      while (!Done)
      {
        if ((fdata.dwFileAttributes & FA_DIREC)==0)
          break;
        Done=!apiFindNextFile(FindHandle,&fdata);
      }
      FindClose(FindHandle);
      if (!Done)
      {
        strResultName = DestPath;
        AddEndSlashW(strResultName);
        strResultName += fdata.strFileName;
        if (GetCode!=1)
        {
          SetFileAttributesW(strResultName,FILE_ATTRIBUTE_NORMAL);
          DeleteFileW(strResultName); //BUGBUG
        }
        else
          Found=TRUE;
      }
    }
    ReadUserBackgound(SaveScr);

    delete SaveScr;
  }

  return(Found);
}


int PluginsSet::DeleteFiles(HANDLE hPlugin,struct PluginPanelItemW *PanelItem,
                            int ItemsNumber,int OpMode)
{
  _ALGO(CleverSysLog clv("PluginsSet::DeleteFiles()"));
  _ALGO(SysLog("hPlugin=%p, PanelItem=%p, ItemsNumber=%d, OpMode=%d",hPlugin,PanelItem,ItemsNumber,OpMode));
  struct PluginHandle *ph=(struct PluginHandle *)hPlugin;
  Plugin *PData=PluginsData[ph->PluginNumber];
  if (PData->pDeleteFiles && !ProcessException)
  {
    ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
    SaveScreen SaveScr;
    KeepUserScreen=FALSE;
    int Code;
    //CurPluginItem=PData;
    PData->FuncFlags.Set(PICFF_DELETEFILES);
    if(Opt.ExceptRules)
    {
      TRY{
        Code=PData->pDeleteFiles(ph->InternalHandle,PanelItem,ItemsNumber,OpMode);
      }
      EXCEPT(xfilter(EXCEPT_DELETEFILES,GetExceptionInformation(),PData,1) )
      {
        UnloadPlugin(PData,EXCEPT_DELETEFILES); //????!!!!
        Code=FALSE;
        ProcessException=FALSE;
      }
    }
    else
      Code=PData->pDeleteFiles(ph->InternalHandle,PanelItem,ItemsNumber,OpMode);
    //CurPluginItem=NULL;
    PData->FuncFlags.Clear(PICFF_DELETEFILES);
    ReadUserBackgound(&SaveScr);
    return(Code);
  }
  return(FALSE);
}


int PluginsSet::MakeDirectory(HANDLE hPlugin,const wchar_t *Name,int OpMode)
{
  _ALGO(CleverSysLog clv("PluginsSet::MakeDirectory()"));
  _ALGO(SysLog("hPlugin=%p, Name='%s', OpMode=%d",hPlugin,(Name?Name:"(NULL)"),OpMode));
  struct PluginHandle *ph=(struct PluginHandle *)hPlugin;
  Plugin *PData=PluginsData[ph->PluginNumber];
  if (PData->pMakeDirectory && !ProcessException)
  {
    ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
    SaveScreen SaveScr;
    KeepUserScreen=FALSE;
    int Code;

    //CurPluginItem=PData;
    PData->FuncFlags.Set(PICFF_MAKEDIRECTORY);

    if(Opt.ExceptRules)
    {
      TRY{
        Code=PData->pMakeDirectory(ph->InternalHandle,Name,OpMode);
      }
      EXCEPT(xfilter(EXCEPT_MAKEDIRECTORY,GetExceptionInformation(),PData,1) )
      {
        UnloadPlugin(PData,EXCEPT_MAKEDIRECTORY); //????!!!!
        Code=-1;
        ProcessException=FALSE;
      }
    }
    else
      Code=PData->pMakeDirectory(ph->InternalHandle,Name,OpMode);

    //CurPluginItem=NULL;
    PData->FuncFlags.Clear(PICFF_MAKEDIRECTORY);

    ReadUserBackgound(&SaveScr);
    return(Code);
  }
  return(-1);
}


int PluginsSet::ProcessHostFile(HANDLE hPlugin,struct PluginPanelItemW *PanelItem,int ItemsNumber,int OpMode)
{
  _ALGO(CleverSysLog clv("PluginsSet::ProcessHostFile()"));
  _ALGO(SysLog("hPlugin=%p, PanelItem=%p, ItemsNumber=%d, OpMode=%d",hPlugin,PanelItem,ItemsNumber,OpMode));
  struct PluginHandle *ph=(struct PluginHandle *)hPlugin;
  Plugin *PData=PluginsData[ph->PluginNumber];
  if (PData->pProcessHostFile && !ProcessException)
  {
    ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
    SaveScreen SaveScr;
    KeepUserScreen=FALSE;
    int Code;
    //CurPluginItem=PData;
    PData->FuncFlags.Set(PICFF_PROCESSHOSTFILE);
    if(Opt.ExceptRules)
    {
      TRY{
        Code=PData->pProcessHostFile(ph->InternalHandle,PanelItem,ItemsNumber,OpMode);
      }
      EXCEPT(xfilter(EXCEPT_PROCESSHOSTFILE,GetExceptionInformation(),PData,1)){
        UnloadPlugin(PData,EXCEPT_PROCESSHOSTFILE); //????!!!!
        Code=FALSE;
        ProcessException=FALSE;
      }
    }
    else
      Code=PData->pProcessHostFile(ph->InternalHandle,PanelItem,ItemsNumber,OpMode);
    //CurPluginItem=NULL;
    PData->FuncFlags.Clear(PICFF_PROCESSHOSTFILE);
    ReadUserBackgound(&SaveScr);
    return(Code);
  }
  return(FALSE);
}


int PluginsSet::GetFiles(HANDLE hPlugin,struct PluginPanelItemW *PanelItem,
                int ItemsNumber,int Move,const wchar_t *DestPath,int OpMode)
{
  _ALGO(CleverSysLog clv("PluginsSet::GetFiles()"));
  //_ALGO(SysLog("hPlugin=%p, PanelItem=%p, ItemsNumber=%d, OpMode=%d",hPlugin,PanelItem,ItemsNumber,OpMode));
  ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
  int ExitCode=FALSE;
  struct PluginHandle *ph=(struct PluginHandle *)hPlugin;
  Plugin *PData=PluginsData[ph->PluginNumber];
  if (PData->pGetFiles && !ProcessException)
  {
    SaveScreen SaveScr;
    KeepUserScreen=FALSE;
    //CurPluginItem=PData;
    PData->FuncFlags.Set(PICFF_GETFILES);

    if(Opt.ExceptRules)
    {
      TRY{
        ExitCode=PData->pGetFiles(ph->InternalHandle,PanelItem,ItemsNumber,Move,DestPath,OpMode);
      }
      EXCEPT(xfilter(EXCEPT_GETFILES,GetExceptionInformation(),PData,1)){
        UnloadPlugin(PData,EXCEPT_GETFILES); //????!!!!
        ExitCode=0;
        ProcessException=FALSE;
      }
    }
    else
      ExitCode=PData->pGetFiles(ph->InternalHandle,PanelItem,ItemsNumber,Move,DestPath,OpMode);

    //CurPluginItem=NULL;
    PData->FuncFlags.Clear(PICFF_GETFILES);
  }
  return(ExitCode);
}


int PluginsSet::PutFiles(HANDLE hPlugin,struct PluginPanelItemW *PanelItem,int ItemsNumber,int Move,int OpMode)
{
  _ALGO(CleverSysLog clv("PluginsSet::PutFiles()"));
  _ALGO(SysLog("hPlugin=%p, PanelItem=%p, ItemsNumber=%d, Move=%d, OpMode=%d",hPlugin,PanelItem,ItemsNumber,Move,OpMode));
  struct PluginHandle *ph=(struct PluginHandle *)hPlugin;
  Plugin *PData=PluginsData[ph->PluginNumber];
  if (PData->pPutFiles && !ProcessException)
  {
    ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);
    SaveScreen SaveScr;
    KeepUserScreen=FALSE;
    int Code;
    //CurPluginItem=PData;
    PData->FuncFlags.Set(PICFF_PUTFILES);
    if(Opt.ExceptRules)
    {
      TRY{
        Code=PData->pPutFiles(ph->InternalHandle,PanelItem,ItemsNumber,Move,OpMode);
      }
      EXCEPT(xfilter(EXCEPT_PUTFILES,GetExceptionInformation(),PData,1) )
      {
        UnloadPlugin(PData,EXCEPT_PUTFILES); //????!!!!
        Code=0;
        ProcessException=FALSE;
      }
    }
    else
      Code=PData->pPutFiles(ph->InternalHandle,PanelItem,ItemsNumber,Move,OpMode);
    _ALGO(SysLog("return Code=%d",Code));
    //CurPluginItem=NULL;
    PData->FuncFlags.Clear(PICFF_PUTFILES);
    ReadUserBackgound(&SaveScr);
    return(Code);
  }
  return(FALSE);
}


int PluginsSet::GetPluginInfo(int PluginNumber,struct PluginInfoW *Info)
{
  _ALGO(CleverSysLog clv("PluginsSet::GetPluginInfo()"));
  _ALGO(SysLog("PluginNumber=%p, Info=%p",PluginNumber,Info));
  if (PluginNumber>=PluginsCount || !Info)
    return(FALSE);
  memset(Info,0,sizeof(*Info));
  Plugin *PData=PluginsData[PluginNumber];
  if (PData->pGetPluginInfo && !ProcessException)
  {
    //CurPluginItem=PData;
    PData->FuncFlags.Set(PICFF_GETPLUGININFO);
    if(Opt.ExceptRules)
    {
      TRY{
        PData->pGetPluginInfo(Info);
      }
      EXCEPT(xfilter(EXCEPT_GETPLUGININFO,GetExceptionInformation(),PData,1)){
        PData->FuncFlags.Clear(PICFF_GETPLUGININFO);
        UnloadPlugin(PData,EXCEPT_GETPLUGININFO); //????!!!!
        ProcessException=FALSE;
        return FALSE;
      }
      if(!TestPluginInfo(PData,Info)) //???
      {
        PData->FuncFlags.Clear(PICFF_GETPLUGININFO);
        return FALSE;
      }
    }
    else
      PData->pGetPluginInfo(Info);
    //CurPluginItem=NULL;
    PData->FuncFlags.Clear(PICFF_GETPLUGININFO);
  }
  return(TRUE);
}


void PluginsSet::GetOpenPluginInfo(HANDLE hPlugin,struct OpenPluginInfoW *Info)
{
  _ALGO(CleverSysLog clv("PluginsSet::GetOpenPluginInfo()"));
  _ALGO(SysLog("hPlugin=%p, Info=%p",hPlugin,Info));
  if (!Info)
    return;
  memset(Info,0,sizeof(*Info));
  struct PluginHandle *ph=(struct PluginHandle *)hPlugin;
  Plugin *PData=PluginsData[ph->PluginNumber];
  if (PData->pGetOpenPluginInfo && !ProcessException)
  {
    //CurPluginItem=PData;
    PData->FuncFlags.Set(PICFF_GETOPENPLUGININFO);
    if(Opt.ExceptRules)
    {
      TRY{
        PData->pGetOpenPluginInfo(ph->InternalHandle,Info);
      }
      EXCEPT(xfilter(EXCEPT_GETOPENPLUGININFO,GetExceptionInformation(),PData,1)){
        PData->FuncFlags.Clear(PICFF_GETOPENPLUGININFO);
        UnloadPlugin(PData,EXCEPT_GETOPENPLUGININFO); //????!!!!
        ProcessException=FALSE;
        return;
      }
      if(!TestOpenPluginInfo(PData,Info)) // ??
      {
        PData->FuncFlags.Clear(PICFF_GETOPENPLUGININFO);
        return;
      }
    }
    else
      PData->pGetOpenPluginInfo(ph->InternalHandle,Info);
    //CurPluginItem=NULL;
    PData->FuncFlags.Clear(PICFF_GETOPENPLUGININFO);
    _ALGO(GetOpenPluginInfo_Dump("",Info,NULL));
  }
  if (Info->CurDir==NULL)
    Info->CurDir=L"";
}


int PluginsSet::ProcessKey(HANDLE hPlugin,int Key,unsigned int ControlState)
{
  _ALGO(CleverSysLog clv("PluginsSet::ProcessKey()"));
  _ALGO(SysLog("hPlugin=%p, Key=%u (0x%08X) ControlState=%u (0x%08X) ",hPlugin,Key,Key,ControlState,ControlState));
  struct PluginHandle *ph=(struct PluginHandle *)hPlugin;
  Plugin *PData=PluginsData[ph->PluginNumber];
  if (PData->pProcessKey && !ProcessException)
  {
    int Ret;
    //CurPluginItem=PData;
    PData->FuncFlags.Set(PICFF_PROCESSKEY);
    if(Opt.ExceptRules)
    {
      TRY{
        Ret=PData->pProcessKey(ph->InternalHandle,Key,ControlState);
      }
      EXCEPT(xfilter(EXCEPT_PROCESSKEY,GetExceptionInformation(),PData,1)){
        UnloadPlugin(PData,EXCEPT_PROCESSKEY); //????!!!!
        Ret=FALSE;
        ProcessException=FALSE;
      }
    }
    else
      Ret=PData->pProcessKey(ph->InternalHandle,Key,ControlState);
    //CurPluginItem=NULL;
    PData->FuncFlags.Clear(PICFF_PROCESSKEY);
    return(Ret);
  }
  return(FALSE);
}


int PluginsSet::ProcessEvent(HANDLE hPlugin,int Event,void *Param)
{
  struct PluginHandle *ph=(struct PluginHandle *)hPlugin;
  Plugin *PData=PluginsData[ph->PluginNumber];
  if (PData->pProcessEvent && !ProcessException)
  {
    int Ret;
    //CurPluginItem=PData;
    PData->FuncFlags.Set(PICFF_PROCESSEVENT);
    if(Opt.ExceptRules)
    {
      TRY{
        Ret=PData->pProcessEvent(ph->InternalHandle,Event,Param);
      }
      EXCEPT(xfilter(EXCEPT_PROCESSEVENT,GetExceptionInformation(),PData,1)){
        UnloadPlugin(PData,EXCEPT_PROCESSEVENT); //????!!!!
        Ret=FALSE;
        ProcessException=FALSE;
      }
    }
    else
      Ret=PData->pProcessEvent(ph->InternalHandle,Event,Param);
    //CurPluginItem=NULL;
    PData->FuncFlags.Clear(PICFF_PROCESSEVENT);
    return(Ret);
  }
  return(FALSE);
}


int PluginsSet::Compare(HANDLE hPlugin,const struct PluginPanelItemW *Item1,
                        const struct PluginPanelItemW *Item2,unsigned int Mode)
{
  struct PluginHandle *ph=(struct PluginHandle *)hPlugin;
  Plugin *PData=PluginsData[ph->PluginNumber];
  if (PData->pCompare && !ProcessException)
  {
    int Ret;
    //CurPluginItem=PData;
    PData->FuncFlags.Set(PICFF_COMPARE);
    if(Opt.ExceptRules)
    {
      TRY{
        Ret=PData->pCompare(ph->InternalHandle,Item1,Item2,Mode);
      }
      EXCEPT(xfilter(EXCEPT_COMPARE,GetExceptionInformation(),PData,1)) {
        UnloadPlugin(PData,EXCEPT_COMPARE); //????!!!!
        Ret=-3;
        ProcessException=FALSE;
      }
    }
    else
      Ret=PData->pCompare(ph->InternalHandle,Item1,Item2,Mode);
    //CurPluginItem=NULL;
    PData->FuncFlags.Clear(PICFF_COMPARE);
    return(Ret);
  }
  return(-3);
}

void PluginsSet::ConfigureCurrent(int PNum,int INum)
{
  if (PreparePlugin(PNum) && PluginsData[PNum]->pConfigure!=NULL && !ProcessException)
  {
    int Ret;
    Plugin *PData=PluginsData[PNum];
    //CurPluginItem=PData;
    PData->FuncFlags.Set(PICFF_CONFIGURE);
    if(Opt.ExceptRules)
    {
      TRY{
        Ret=PData->pConfigure(INum);
      }
      EXCEPT(xfilter(EXCEPT_CONFIGURE,GetExceptionInformation(),PData,1))
      {
        PData->FuncFlags.Clear(PICFF_CONFIGURE);
        UnloadPlugin(PData,EXCEPT_CONFIGURE); //????!!!!
        ProcessException=FALSE;
        return;
      }
    }
    else
      Ret=PData->pConfigure(INum);
    //CurPluginItem=NULL;
    PData->FuncFlags.Clear(PICFF_CONFIGURE);

    if (Ret)
    {
      int PMode[2];
      PMode[0]=CtrlObject->Cp()->LeftPanel->GetMode();
      PMode[1]=CtrlObject->Cp()->RightPanel->GetMode();
      for(int I=0; I < sizeof(PMode)/sizeof(PMode[0]); ++I)
      {
        if(PMode[I] == PLUGIN_PANEL)
        {
          Panel *pPanel=(I==0?CtrlObject->Cp()->LeftPanel:CtrlObject->Cp()->RightPanel);
          pPanel->Update(UPDATE_KEEP_SELECTION);
          pPanel->SetViewMode(pPanel->GetViewMode());
          pPanel->Redraw();
        }
      }
    }
    SavePluginSettings(PData,PData->FindData);
  }
}

/* $ 29.05.2001 IS
   ! При настройке "параметров внешних модулей" закрывать окно с их
     списком только при нажатии на ESC
*/
void PluginsSet::Configure(int StartPos)
{
  // Полиция 4 - Параметры внешних модулей
  if(Opt.Policies.DisabledOptions&FFPOL_MAINMENUPLUGINS)
    return;

  {
    VMenu PluginList(UMSG(MPluginConfigTitle),NULL,0,TRUE,ScrY-4);
    PluginList.SetFlags(VMENU_WRAPMODE);
    PluginList.SetHelp(L"PluginsConfig");

    for(;;)
    {
      DWORD Data;
      int I, J;
      BOOL NeedUpdateItems=TRUE;
      BOOL Done=FALSE;
      int MenuItemNumber=0;


      string strFirstHotKey;
      int HotKeysPresent=EnumRegKeyW(L"PluginHotkeys",0,strFirstHotKey);


      if(NeedUpdateItems)
      {
        PluginList.ClearDone();
        PluginList.DeleteItems();
        PluginList.SetPosition(-1,-1,0,0);
        MenuItemNumber=0;

        LoadIfCacheAbsent();

        string strHotRegKey, strHotKey;

        for (I=0;I<PluginsCount;I++)
        {
          if (PluginsData[I]->WorkFlags.Check(PIWF_CACHED))
          {
            string strRegKey, strValue;
            int RegNumber=GetCacheNumber(PluginsData[I]->strModuleName,NULL,PluginsData[I]->CachePos);
            if (RegNumber==-1)
              continue;
            else
              for (J=0;;J++)
              {
                strHotKey=L"";
                if (GetHotKeyRegKey(I,J,strHotRegKey))
                  GetRegKeyW(strHotRegKey,L"ConfHotkey",strHotKey,L"");
                MenuItemEx ListItem;
                ListItem.Clear ();
                strRegKey.Format (FmtPluginsCache_PluginDW,RegNumber);
                strValue.Format (FmtPluginConfigStringDW,J);
                string strName;
                GetRegKeyW(strRegKey,strValue,strName,L"");
                if ( strName.IsEmpty() )
                  break;
                if (!HotKeysPresent)
                  ListItem.strName = strName;
                else
                  if ( !strHotKey.IsEmpty() )
                    ListItem.strName.Format (L"&%c%s  %s",strHotKey.At(0),(strHotKey.At(0)==L'&'?L"&":L""), (const wchar_t*)strName);
                  else
                    ListItem.strName.Format (L"   %s", (const wchar_t*)strName);
                //ListItem.SetSelect(MenuItemNumber++ == StartPos);
                MenuItemNumber++;
                Data=MAKELONG(I,J);
                PluginList.SetUserData((void*)(DWORD_PTR)Data,sizeof(Data),PluginList.AddItemW(&ListItem));
              }
          }
          else
          {
            struct PluginInfoW Info;
            if (!GetPluginInfo(I,&Info))
              continue;
            for (J=0;J<Info.PluginConfigStringsNumber;J++)
            {
              strHotKey=L"";
              if (GetHotKeyRegKey(I,J,strHotRegKey))
                GetRegKeyW(strHotRegKey,L"ConfHotkey",strHotKey,L"");
              MenuItemEx ListItem;

              memset(&ListItem,0,sizeof(ListItem));
              string strName = NullToEmptyW(Info.PluginConfigStrings[J]);
              if (!HotKeysPresent)
                ListItem.strName = strName;
              else
                if ( !strHotKey.IsEmpty() )
                  ListItem.strName.Format (L"&%c%s  %s",strHotKey.At(0),(strHotKey.At(0)==L'&'?L"&":L""), (const wchar_t*)strName);
                else
                  ListItem.strName.Format (L"   %s", (const wchar_t*)strName);
              //ListItem.SetSelect(MenuItemNumber++ == StartPos);
              MenuItemNumber++;
              Data=MAKELONG(I,J);
              PluginList.SetUserData((void*)(DWORD_PTR)Data,sizeof(Data),PluginList.AddItemW(&ListItem));
            }
          }
        }
        PluginList.AssignHighlights(FALSE);
        PluginList.SetBottomTitle(UMSG(MPluginHotKeyBottom));

        PluginList.ClearDone();
        PluginList.SortItems(0,3);
        PluginList.SetSelectPos(StartPos,1);
        NeedUpdateItems=FALSE;
      }

      /* $ 18.12.2000 SVS
         Shift-F1 в списке плагинов вызывает хелп по данному плагину
      */

      string strPluginModuleName;

      PluginList.Show();
      while (!PluginList.Done())
      {
        int SelPos=PluginList.GetSelectPos();
        Data=(DWORD)(DWORD_PTR)PluginList.GetUserData(NULL,0,SelPos);
        string strRegKey;
        switch(PluginList.ReadInput())
        {
          case KEY_SHIFTF1:

            strPluginModuleName = PluginsData[LOWORD(Data)]->strModuleName;
            if(!FarShowHelp(strPluginModuleName,L"Config",FHELP_SELFHELP|FHELP_NOSHOWERROR) &&
               !FarShowHelp(strPluginModuleName,L"Configure",FHELP_SELFHELP|FHELP_NOSHOWERROR))
            {
              //strcpy(PluginModuleName,PluginsData[Data[0]].ModuleName);
              FarShowHelp(strPluginModuleName,NULL,FHELP_SELFHELP|FHELP_NOSHOWERROR);
            }
            break;
          case KEY_F4:
            if (PluginList.GetItemCount() > 0 && SelPos<MenuItemNumber && GetHotKeyRegKey(LOWORD(Data),HIWORD(Data),strRegKey))
            {
              BlockExtKey blockExtKey;
              string strName00;

              int nOffset = HotKeysPresent?3:0;

              strName00 = (const wchar_t*)PluginList.GetItemPtr()->strName+nOffset;
              RemoveExternalSpacesW(strName00);

              string strHotKey;

              if(GetMenuHotKeyW(strHotKey,1,
                        (const wchar_t *)MPluginHotKeyTitle,
                        (const wchar_t *)MPluginHotKey,
                        strName00,
                        NULL,strRegKey,L"ConfHotkey"))
              {
                PluginList.Hide();
                NeedUpdateItems=TRUE;
                StartPos=SelPos;
                PluginList.SetExitCode(SelPos);
                /* Грязный хак :-( */
//              PluginList.Hide();
                PluginList.Show();
                break;
              }
            }
            break;
          default:
            PluginList.ProcessInput();
            break;
        }
      }

      if(!NeedUpdateItems)
      {
        StartPos=PluginList.Modal::GetExitCode();
        PluginList.Hide();
        if (StartPos<0)
          break;
        Data=(DWORD)(DWORD_PTR)PluginList.GetUserData(NULL,0,StartPos);
        // ОЧЕРЕДНОЙ КОСТЫЛЬ, ИБО ВСЕ НЕПРАВИЛЬНО!
        //  Меню уже погашено, так зачем же его где опять высвечивать,
        //  если об это не просили?
        ConfigureCurrent(LOWORD(Data),HIWORD(Data));
      }
    }
  }
}
/* IS $ */

///int PluginsSet::CommandsMenu(int Editor,int Viewer,int StartPos,char *HistoryName)
int PluginsSet::CommandsMenu(int ModalType,int StartPos,const wchar_t *HistoryName)
{
  int MenuItemNumber=0;
/* $ 04.05.2001 OT */
  int Editor,Viewer;

  Editor = ModalType==MODALTYPE_EDITOR;
  Viewer = ModalType==MODALTYPE_VIEWER;
  /* OT $ */

  string strRegKey;

  DWORD Data;
  {
    VMenu PluginList(UMSG(MPluginCommandsMenuTitle),NULL,0,TRUE,ScrY-4);
    PluginList.SetFlags(VMENU_WRAPMODE);
    PluginList.SetHelp(L"Plugins");
    BOOL NeedUpdateItems=TRUE;
    BOOL Done=FALSE;


    while(!Done)
    {

      string strFirstHotKey;
      int HotKeysPresent=EnumRegKeyW(L"PluginHotkeys",0,strFirstHotKey);

      if(NeedUpdateItems)
      {
        PluginList.ClearDone();
        PluginList.DeleteItems();
        PluginList.SetPosition(-1,-1,0,0);

        LoadIfCacheAbsent();

        for (int I=0;I<PluginsCount;I++)
        {
          string strHotRegKey, strHotKey;
          if (PluginsData[I]->WorkFlags.Check(PIWF_CACHED))
          {
            string strRegKey, strValue;
            int RegNumber=GetCacheNumber(PluginsData[I]->strModuleName,NULL,PluginsData[I]->CachePos);
            if (RegNumber==-1)
              continue;
            else
            {
              strRegKey.Format (FmtPluginsCache_PluginDW,RegNumber);
              int IFlags=GetRegKeyW(strRegKey,L"Flags",0);
              /* todo: тут надо не смотреть на Editor/Viewer
              а сделать четкий анализ на ModalType */
              if (Editor && (IFlags & PF_EDITOR)==0 ||
                Viewer && (IFlags & PF_VIEWER)==0 ||
                !Editor && !Viewer && (IFlags & PF_DISABLEPANELS))
                continue;
              for (int J=0;;J++)
              {
                strHotKey=L"";
                if (GetHotKeyRegKey(I,J,strHotRegKey))
                  GetRegKeyW(strHotRegKey,L"Hotkey",strHotKey,L"");
                struct MenuItemEx ListItem;
                memset(&ListItem,0,sizeof(ListItem));
                strValue.Format (FmtPluginMenuStringDW,J);
                string strName;
                GetRegKeyW(strRegKey,strValue,strName,L"");
                if ( strName.IsEmpty() )
                  break;
                if (!HotKeysPresent)
                  ListItem.strName.Format (L"   %s", (const wchar_t*)strName);//strcpy(ListItem.Name,Name);
                else
                  if ( !strHotKey.IsEmpty() )
                    ListItem.strName.Format (L"&%c%s  %s",strHotKey.At(0),(strHotKey.At(0)==L'&'?L"&":L""),(const wchar_t*)strName);
                  else
                    ListItem.strName.Format (L"   %s", (const wchar_t*)strName);
                  //ListItem.SetSelect(MenuItemNumber++ == StartPos);
                  MenuItemNumber++;
                  Data=MAKELONG(I,J);
                  PluginList.SetUserData((void*)(DWORD_PTR)Data,sizeof(Data),PluginList.AddItemW(&ListItem));
              }
            }
          }
          else
          {
            struct PluginInfoW Info;
            if (!GetPluginInfo(I,&Info))
              continue;
            if (Editor && (Info.Flags & PF_EDITOR)==0 ||
              Viewer && (Info.Flags & PF_VIEWER)==0 ||
              !Editor && !Viewer && (Info.Flags & PF_DISABLEPANELS))
              continue;
            for (int J=0;J<Info.PluginMenuStringsNumber;J++)
            {
              strHotKey=L"";
              if (GetHotKeyRegKey(I,J,strHotRegKey))
                GetRegKeyW(strHotRegKey,L"Hotkey",strHotKey,L"");
              struct MenuItemEx ListItem;
              memset(&ListItem,0,sizeof(ListItem));
              string strName = NullToEmptyW(Info.PluginMenuStrings[J]);
              if (!HotKeysPresent)
                ListItem.strName.Format (L"   %s", (const wchar_t*)strName);//strcpy(ListItem.Name,Name);
              else
                if ( !strHotKey.IsEmpty() )
                  ListItem.strName.Format (L"&%c%s  %s",strHotKey.At(0),(strHotKey.At(0)==L'&'?L"&":L""), (const wchar_t*)strName);
                else
                  ListItem.strName.Format (L"   %s", (const wchar_t*)strName);
                //ListItem.SetSelect(MenuItemNumber++ == StartPos);
                MenuItemNumber++;
                Data=MAKELONG(I,J);
                PluginList.SetUserData((void*)(DWORD_PTR)Data,sizeof(Data),PluginList.AddItemW(&ListItem));
            }
          }
        }

        PluginList.AssignHighlights(FALSE);
        PluginList.SetBottomTitle(UMSG(MPluginHotKeyBottom));

        PluginList.SortItems(0,3);
        PluginList.SetSelectPos(StartPos,1);
        NeedUpdateItems=FALSE;
      }

      PluginList.Show();

      while (!PluginList.Done())
      {
        int SelPos=PluginList.GetSelectPos();

        Data=(DWORD)(DWORD_PTR)PluginList.GetUserData(NULL,0,SelPos);
        switch(PluginList.ReadInput())
        {
        /* $ 18.12.2000 SVS
        Shift-F1 в списке плагинов вызывает хелп по данному плагину
          */
        case KEY_SHIFTF1:
          {
            // Вызываем нужный топик, который передали в CommandsMenu()
            FarShowHelp(PluginsData[LOWORD(Data)]->strModuleName,HistoryName,FHELP_SELFHELP|FHELP_NOSHOWERROR|FHELP_USECONTENTS);
            break;
          }
          /* SVS $ */
        case KEY_ALTF11:
          WriteEvent(FLOG_PLUGINSINFO);
          break;
        case KEY_F4:
          if (PluginList.GetItemCount() > 0 && SelPos<MenuItemNumber && GetHotKeyRegKey(LOWORD(Data),HIWORD(Data),strRegKey))
          {
            BlockExtKey blockExtKey;
            string strName00;

            int nOffset = HotKeysPresent?3:0;

            strName00 = (const wchar_t*)PluginList.GetItemPtr()->strName+nOffset;
            RemoveExternalSpacesW(strName00);

            string strHotKey;

            if(GetMenuHotKeyW(strHotKey,1,(const wchar_t *)MPluginHotKeyTitle,(const wchar_t *)MPluginHotKey,strName00,NULL,strRegKey,L"Hotkey"))
            {
              PluginList.Hide();
              NeedUpdateItems=TRUE;
              StartPos=SelPos;
              PluginList.SetExitCode(SelPos);
              /* Грязный хак :-( */
//            PluginList.Hide();
              PluginList.Show();
              break;
            }
          }
          break;
        default:
          PluginList.ProcessInput();
          break;
        }
      }

      if(!NeedUpdateItems && PluginList.Done())
        break;
    }

    int ExitCode=PluginList.Modal::GetExitCode();

    PluginList.Hide();
    if (ExitCode<0)
      return(FALSE);
    ScrBuf.Flush();
    Data=(DWORD)(DWORD_PTR)PluginList.GetUserData(NULL,0,ExitCode);
  }
  if (PreparePlugin(LOWORD(Data)) && PluginsData[LOWORD(Data)]->pOpenPlugin!=NULL && !ProcessException)
  {
    Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
//    if (ActivePanel->ProcessPluginEvent(FE_CLOSE,NULL))
//      return(FALSE);
    int OpenCode=OPEN_PLUGINSMENU;
    if (Editor)
      OpenCode=OPEN_EDITOR;
    if (Viewer)
      OpenCode=OPEN_VIEWER;
    HANDLE hPlugin=OpenPlugin(LOWORD(Data),OpenCode,HIWORD(Data));
    if (hPlugin!=INVALID_HANDLE_VALUE && !Editor && !Viewer)
    {
      if ( ActivePanel->ProcessPluginEvent(FE_CLOSE,NULL) )
      {
        ClosePlugin (hPlugin);
        return(FALSE);
      }


      Panel *NewPanel=CtrlObject->Cp()->ChangePanel(ActivePanel,FILE_PANEL,TRUE,TRUE);
      NewPanel->SetPluginMode(hPlugin,L"");
      NewPanel->Update(0);
      NewPanel->Show();
      NewPanel->SetFocus();
    }
    /* $ 14.05.2002 SKV
       Возможна ситуация, когда в этом месте CurEditor==NULL
    */
    if (Editor && CurEditor){
      CurEditor->SetPluginTitle(NULL);
    }
    /* SKV $ */
  }
  return(TRUE);
}

int PluginsSet::GetHotKeyRegKey(int PluginNumber,int ItemNumber,string &strRegKey)
{
#if 1
// Старый вариант
/*
FarPath
C:\Program Files\Far\

ModuleName                                             PluginName
---------------------------------------------------------------------------------------
C:\Program Files\Far\Plugins\MultiArc\MULTIARC.DLL  -> Plugins\MultiArc\MULTIARC.DLL
C:\MultiArc\MULTIARC.DLL                            -> DLL
---------------------------------------------------------------------------------------
*/
  unsigned int FarPathLength=g_strFarPath.GetLength();
  strRegKey=L"";
  if (FarPathLength < PluginsData[PluginNumber]->strModuleName.GetLength())
  {
    string strPluginName;

//    strcpy(PluginName,PluginsData[PluginNumber].ModuleName+FarPathLength);
    strPluginName = (const wchar_t*)PluginsData[PluginNumber]->strModuleName+(wcsncmp(PluginsData[PluginNumber]->strModuleName,g_strFarPath,FarPathLength)?0:FarPathLength);

    wchar_t *Ptr = strPluginName.GetBuffer (strPluginName.GetLength()+20);

    while ( *Ptr )
    {
      if (*Ptr==L'\\')
        *Ptr=L'/';

      Ptr++;

      if (ItemNumber>0)
        swprintf(Ptr,L"%%%d",ItemNumber);
    }

    strPluginName.ReleaseBuffer ();

    strRegKey.Format (L"PluginHotkeys\\%s",(const wchar_t*)strPluginName);
    return(TRUE);
  }
  return(FALSE);
#else
// Новый вариант, с полными путями
  *RegKey=0;
  char PluginName[NM], *Ptr;
  strcpy(PluginName,PluginsData[PluginNumber]->ModuleName);
  for (Ptr=PluginName;*Ptr;++Ptr)
  {
    if (*Ptr=='\\')
      *Ptr='/';
  }
  if (ItemNumber>0)
    sprintf(Ptr,"%%%d",ItemNumber);

  sprintf(RegKey,"PluginHotkeys\\%s",PluginName);

#if 1
/*  // конвертация - потом убрать
  unsigned int FarPathLength=strlen(FarPath);
  if (FarPathLength < strlen(PluginsData[PluginNumber]->ModuleName))
  {
    char OldRevKey[NM];
    sprintf(OldRevKey,"PluginHotkeys\\%s",PluginName+FarPathLength);
    if(CheckRegKey(OldRevKey))
    {
      char ConfHotkey[32];
      char Hotkey[32];
      GetRegKey(OldRevKey,"ConfHotkey",ConfHotkey,"",sizeof(ConfHotkey));
      GetRegKey(OldRevKey,"Hotkey",Hotkey,"",sizeof(Hotkey));
      if(*ConfHotkey)
        SetRegKey(RegKey,"ConfHotkey",ConfHotkey);
      if(*Hotkey)
        SetRegKey(RegKey,"Hotkey",Hotkey);
      DeleteRegKey(OldRevKey); //??? А нужно?
    }
  }*/
#endif
  return(TRUE);
#endif
}

/* $ 21.08.2002 IS
   + Параметр PluginTextSize, чтобы знать, сколько брать
*/
int PluginsSet::GetDiskMenuItem(int PluginNumber,int PluginItem,
                int &ItemPresent,int &PluginTextNumber,string &strPluginText)
{
  if (PluginNumber>=PluginsCount)
    return(FALSE);

  LoadIfCacheAbsent();

  Plugin *PData=PluginsData[PluginNumber];
  if (PData->WorkFlags.Check(PIWF_CACHED))
  {
    string strRegKey, strValue;
    int RegNumber=GetCacheNumber(PData->strModuleName,NULL,PData->CachePos);
    if (RegNumber==-1)
      ItemPresent=0;
    else
    {
      strRegKey.Format (FmtPluginsCache_PluginDW,RegNumber);
      strValue.Format (FmtDiskMenuStringDW,PluginItem);
      GetRegKeyW(strRegKey,strValue,strPluginText,L"");
      strValue.Format (FmtDiskMenuNumberDW,PluginItem);
      GetRegKeyW(strRegKey,strValue,PluginTextNumber,0);
      ItemPresent=!strPluginText.IsEmpty();
    }
    return(TRUE);
  }
  struct PluginInfoW Info;
  if (!GetPluginInfo(PluginNumber,&Info) || Info.DiskMenuStringsNumber <= PluginItem)
    ItemPresent=FALSE;
  else
  {
    if (Info.DiskMenuNumbers)
      PluginTextNumber=Info.DiskMenuNumbers[PluginItem];
    else
      PluginTextNumber=0;
    strPluginText = Info.DiskMenuStrings[PluginItem];
    ItemPresent=TRUE;
  }
  return(TRUE);
}
/* IS $ */

int PluginsSet::UseFarCommand(HANDLE hPlugin,int CommandType)
{
  struct OpenPluginInfoW Info;
  GetOpenPluginInfo(hPlugin,&Info);
  if ((Info.Flags & OPIF_REALNAMES)==0)
    return(FALSE);

  Plugin *PData=PluginsData[((PluginHandle *)hPlugin)->PluginNumber];
  switch(CommandType)
  {
    case PLUGIN_FARGETFILE:
    case PLUGIN_FARGETFILES:
      return(PData->pGetFiles==NULL || (Info.Flags & OPIF_EXTERNALGET));
    case PLUGIN_FARPUTFILES:
      return(PData->pPutFiles==NULL || (Info.Flags & OPIF_EXTERNALPUT));
    case PLUGIN_FARDELETEFILES:
      return(PData->pDeleteFiles==NULL || (Info.Flags & OPIF_EXTERNALDELETE));
    case PLUGIN_FARMAKEDIRECTORY:
      return(PData->pMakeDirectory==NULL || (Info.Flags & OPIF_EXTERNALMKDIR));
  }
  return(TRUE);
}


void PluginsSet::ReloadLanguage()
{
  Plugin *PData;
  for (int I=0;I<PluginsCount;I++)
  {
    PData = PluginsData[I];
      PData->Lang.Close();
  }
  DiscardCache();
}


void PluginsSet::DiscardCache()
{
  for (int I=0;I<PluginsCount;I++)
    PreparePlugin(I);
  DeleteKeyTreeW(RKN_PluginsCacheW);
}


void PluginsSet::LoadIfCacheAbsent()
{
  if (!CheckRegKeyW(RKN_PluginsCacheW))
    for (int I=0;I<PluginsCount;I++)
      PreparePlugin(I);
}


int PluginsSet::ProcessCommandLine(const wchar_t *CommandParam,Panel *Target)
{
  int PrefixLength=0;

  string strCommand=CommandParam;

  UnquoteExternalW(strCommand);
  RemoveLeadingSpacesW(strCommand);

  while (1)
  {
    wchar_t Ch=strCommand.At(PrefixLength);
    if (Ch==0 || IsSpaceW(Ch) || Ch==L'/' || PrefixLength>64)
      return(FALSE);
    if (Ch==L':' && PrefixLength>0)
      break;
    PrefixLength++;
  }

  LoadIfCacheAbsent();

  string strPrefix;

  wchar_t *Prefix = strPrefix.GetBuffer(PrefixLength);

  xwcsncpy(Prefix,strCommand,PrefixLength);

  strPrefix.ReleaseBuffer ();

  int PluginPos=-1;
  /* $ 07.09.2000 VVM 1.18
     + Несколько префиксов у плагина, разделенных через ":"
  */
  DWORD PluginFlags = 0;
  string strPluginPrefix;

  Plugin *PData;
  for (int I=0;I<PluginsCount;I++)
  {
    PData = PluginsData[I];
    if (PData->WorkFlags.Check(PIWF_CACHED))
    {
      int RegNumber=GetCacheNumber(PData->strModuleName,NULL,PData->CachePos);
      if (RegNumber!=-1)
      {
        string strRegKey;
        strRegKey.Format (FmtPluginsCache_PluginDW, RegNumber);
        GetRegKeyW(strRegKey,L"CommandPrefix",strPluginPrefix, L"");
        /* $ 07.09.2000 SVS
             По каким-то непонятным причинам из кэше для Flags возвращалось
             значение равное 0 (хотя вижу что в реестре стоит 0x10) :-(
        */
        PluginFlags=GetRegKeyW(strRegKey,L"Flags",0);
        /* SVS $ */
      } /* if */
      else
        continue;
    } /* if */
    else
    {
      struct PluginInfoW Info;
      if (GetPluginInfo(I,&Info))
      {
        /* $ 10.09.2000 IS
             Вай, вай, как не хорошо... Забыли проверку Info.CommandPrefix на
             NULL сделать, соответственно фар иногда с конвульсиями помирал,
             теперь - нет.
        */
        strPluginPrefix = NullToEmptyW(Info.CommandPrefix);
        /* IS $ */
        PluginFlags = Info.Flags;
      } /* if */
      else
        continue;
    } /* else */
    if ( strPluginPrefix.IsEmpty() )
      continue;

    const wchar_t *PrStart = strPluginPrefix;
    /*$ 10.07.2001 SKV
      префикс должен совпадать ПОЛНОСТЬЮ,
      а не начало...
    */
    int PrefixLength=strPrefix.GetLength ();
    while(1)
    {
      const wchar_t *PrEnd = wcschr(PrStart, L':');
      int Len=PrEnd==NULL ? wcslen(PrStart):(PrEnd-PrStart);
      if(Len<PrefixLength)Len=PrefixLength;
      if (LocalStrnicmpW(Prefix, PrStart, Len)==0)
      /* SKV$*/
      {
        PluginPos=I;
        break;
      } /* if */
      if (PrEnd == NULL)
        break;
      PrStart = ++PrEnd;
    } /* while */
    if (PluginPos >= 0)
      break;
  } /* for */
  /* VVM $ */

  if (PluginPos==-1)
    return(FALSE);
  if (!PreparePlugin(PluginPos) || PluginsData[PluginPos]->pOpenPlugin==NULL)
    return(FALSE);
  Panel *ActivePanel=CtrlObject->Cp()->ActivePanel;
  Panel *CurPanel=(Target)?Target:ActivePanel;
  if (CurPanel->ProcessPluginEvent(FE_CLOSE,NULL))
    return(FALSE);
  CtrlObject->CmdLine->SetStringW(L"");

  /* $ 05.03.2002 DJ
     сделаем буфер побольше, и не забудем проверить
  */
  string strPluginCommand;
  /* $ 07.09.2000 VVM 1.18
    + Если флаг PF_FULLCMDLINE - отдавать с префиксом
  */
  strPluginCommand = (const wchar_t*)strCommand+(PluginFlags & PF_FULLCMDLINE ? 0:PrefixLength+1);
  /* VVM $ */
  /* DJ $ */
  RemoveTrailingSpacesW(strPluginCommand);
  HANDLE hPlugin=OpenPlugin(PluginPos,OPEN_COMMANDLINE,(INT_PTR)(const wchar_t*)strPluginCommand); //BUGBUG
  if (hPlugin!=INVALID_HANDLE_VALUE)
  {
    Panel *NewPanel=CtrlObject->Cp()->ChangePanel(CurPanel,FILE_PANEL,TRUE,TRUE);
    NewPanel->SetPluginMode(hPlugin,L"");
    NewPanel->Update(0);
    NewPanel->Show();
    if(!Target || Target == ActivePanel)
      NewPanel->SetFocus();
  }
  return(TRUE);
}


void PluginsSet::ReadUserBackgound(SaveScreen *SaveScr)
{
  FilePanels *FPanel=CtrlObject->Cp();
  FPanel->LeftPanel->ProcessingPluginCommand++;
  FPanel->RightPanel->ProcessingPluginCommand++;
  if (KeepUserScreen)
  {
    if(SaveScr)
      SaveScr->Discard();
    RedrawDesktop Redraw;
  }
  FPanel->LeftPanel->ProcessingPluginCommand--;
  FPanel->RightPanel->ProcessingPluginCommand--;
}


void CheckScreenLock()
{
  if (ScrBuf.GetLockCount()>0 && !CtrlObject->Macro.PeekKey())
  {
    ScrBuf.SetLockCount(0);
    ScrBuf.Flush();
  }
}

/* $ 27.09.2000 SVS
  Функция CallPlugin - найти плагин по ID и запустить
  в зачаточном состоянии!
*/
int PluginsSet::CallPlugin(DWORD SysID,int OpenFrom, void *Data)
{
  int I;
  if((I=FindPlugin(SysID)) != -1)
  {
    if (PluginsData[I]->pOpenPlugin && !ProcessException)
    {
      HANDLE hNewPlugin=OpenPlugin(I,OpenFrom,(INT_PTR)Data);

      if (hNewPlugin!=INVALID_HANDLE_VALUE &&
         (OpenFrom == OPEN_PLUGINSMENU || OpenFrom == OPEN_FILEPANEL))
      {
        int CurFocus=CtrlObject->Cp()->ActivePanel->GetFocus();
        Panel *NewPanel=CtrlObject->Cp()->ChangePanel(CtrlObject->Cp()->ActivePanel,FILE_PANEL,TRUE,TRUE);
        NewPanel->SetPluginMode(hNewPlugin,L"");
        if (Data && *(const wchar_t *)Data)
          SetDirectory(hNewPlugin,(const wchar_t *)Data,0);
        /* $ 04.04.2001 SVS
           Код закомментирован! Попытка исключить ненужные вызовы в CallPlugin()
           Если что-то не так - раскомментировать!!!
        */
//        NewPanel->Update(0);
        if (CurFocus || !CtrlObject->Cp()->GetAnotherPanel(NewPanel)->IsVisible())
          NewPanel->SetFocus();
//        NewPanel->Show();
        /* SVS $ */
      }
      return TRUE;
    }
  }
  return FALSE;
}

int PluginsSet::FindPlugin(DWORD SysID)
{
  if(SysID != 0 && SysID != 0xFFFFFFFFUl) // не допускается 0 и -1
  {
    Plugin *PData;
    for (int I=0;I<PluginsCount;I++)
    {
      PData = PluginsData[I];
      if (PData->SysID == SysID)
        return I;
    }
  }
  return -1;
}
/* SVS $ */

/* $ 23.10.2000 SVS
   Функция TestPluginInfo - проверка на вшивость переданных плагином данных
*/
#if defined(__BORLANDC__)
#pragma warn -par
#endif
BOOL PluginsSet::TestPluginInfo(Plugin *Item,struct PluginInfoW *Info)
{
  if(!Opt.ExceptRules)
    return TRUE;

  char Buf[1];
  int I=FALSE;
  //EXCEPTION_POINTERS *xp;
  TRY {
    if(Info->DiskMenuStringsNumber > 0 && !Info->DiskMenuStrings)
      RaiseException( STATUS_STRUCTWRONGFILLED, 0, 0, 0);
    else for (I=0; I<Info->DiskMenuStringsNumber; I++)
      memcpy(Buf,Info->DiskMenuStrings[I],1);

    if(Info->PluginMenuStringsNumber > 0 && !Info->PluginMenuStrings)
      RaiseException( STATUS_STRUCTWRONGFILLED+1, 0, 0, 0);
    else for (I=0; I<Info->PluginMenuStringsNumber; I++)
     memcpy(Buf,Info->PluginMenuStrings[I],1);

    if(Info->PluginConfigStringsNumber > 0 && !Info->PluginConfigStrings)
      RaiseException( STATUS_STRUCTWRONGFILLED+2, 0, 0, 0);
    else for (I=0; I<Info->PluginConfigStringsNumber; I++)
      memcpy(Buf,Info->PluginConfigStrings[I],1);

    if (Info->CommandPrefix)
      memcpy(Buf,Info->CommandPrefix,1);

    I=TRUE;
  }
  EXCEPT(xfilter(EXCEPT_GETPLUGININFO_DATA,GetExceptionInformation(),Item,1))
  {
     UnloadPlugin(Item,EXCEPT_GETPLUGININFO_DATA); // тест не пройден, выгружаем его
     I=FALSE;
//     ProcessException=FALSE;
  }
  return I;
}
#if defined(__BORLANDC__)
#pragma warn +par
#endif
/* SVS $ */

/* $ 31.10.2000 SVS
   Функция TestOpenPluginInfo - проверка на вшивость переданных плагином данных
*/
#if defined(__BORLANDC__)
#pragma warn -par
#endif
BOOL PluginsSet::TestOpenPluginInfo(Plugin *Item,struct OpenPluginInfoW *Info)
{
  if(!Opt.ExceptRules)
    return TRUE;

  char Buf[1];
  int I=FALSE;
  //EXCEPTION_POINTERS *xp;
  TRY {
    if(Info->HostFile) memcpy(Buf,Info->HostFile,1);
    if(Info->CurDir) memcpy(Buf,Info->CurDir,1);
    if(Info->Format) memcpy(Buf,Info->Format,1);
    if(Info->PanelTitle) memcpy(Buf,Info->PanelTitle,1);

    if(Info->InfoLinesNumber > 0 && !Info->InfoLines)
      RaiseException( STATUS_STRUCTWRONGFILLED, 0, 0, 0);
    else for (I=0; I<Info->InfoLinesNumber; I++)
      memcpy(Buf,&Info->InfoLines[I],1);

    if(Info->DescrFilesNumber > 0 && !Info->DescrFiles)
      RaiseException( STATUS_STRUCTWRONGFILLED+1, 0, 0, 0);
    else for (I=0; I<Info->DescrFilesNumber; I++)
      memcpy(Buf,Info->DescrFiles[I],1);

    if(Info->PanelModesNumber > 0 && !Info->PanelModesArray)
      RaiseException( STATUS_STRUCTWRONGFILLED+2, 0, 0, 0);
    for (I=0; I<Info->PanelModesNumber; I++)
      memcpy(Buf,&Info->PanelModesArray[I],1);

    if(Info->KeyBar) memcpy(Buf,Info->KeyBar,1);
    if(Info->ShortcutData) memcpy(Buf,Info->ShortcutData,1);
    I=TRUE;
  }
  EXCEPT(xfilter(EXCEPT_GETOPENPLUGININFO_DATA,GetExceptionInformation(),Item,1))
  {
     UnloadPlugin(Item,EXCEPT_GETOPENPLUGININFO_DATA); // тест не пройден, выгружаем его
     I=FALSE;
//     ProcessException=FALSE;
  }
  return I;
}
#if defined(__BORLANDC__)
#pragma warn +par
#endif
/* SVS $ */
