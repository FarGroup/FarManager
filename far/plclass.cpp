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
#include "plclass.hpp"

static const wchar_t *NFMP_PreloadW=L"Preload";
static const wchar_t *NFMP_SysIDW=L"SysID";

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


BOOL PrepareModulePathW(const wchar_t *ModuleName)
{
	string strModulePath;
	strModulePath = ModuleName;

	CutToSlashW (strModulePath); //??

	return FarChDirW(strModulePath,TRUE);
}

void CheckScreenLock()
{
	if ( ScrBuf.GetLockCount() > 0 && !CtrlObject->Macro.PeekKey() )
	{
		ScrBuf.SetLockCount(0);
		ScrBuf.Flush();
	}
}



Plugin::Plugin (
		PluginManager *owner,
		const wchar_t *lpwszModuleName,
		const FAR_FIND_DATA_EX *fdata
		)
{
	m_hModule = NULL;
	CachePos = 0;
	//more initialization here!!!

	m_owner = owner;

	FindData.Clear();

	if ( fdata )
		FindData = *fdata;

	m_strModuleName = lpwszModuleName;

	pSetStartupInfo=0;
	pOpenPlugin=0;
	pOpenFilePlugin=0;
	pClosePlugin=0;
	pGetPluginInfo=0;
	pGetOpenPluginInfo=0;
	pGetFindData=0;
	pFreeFindData=0;
	pGetVirtualFindData=0;
	pFreeVirtualFindData=0;
	pSetDirectory=0;
	pGetFiles=0;
	pPutFiles=0;
	pDeleteFiles=0;
	pMakeDirectory=0;
	pProcessHostFile=0;
	pSetFindList=0;
	pConfigure=0;
	pExitFAR=0;
	pProcessKey=0;
	pProcessEvent=0;
	pCompare=0;
	pProcessEditorInput=0;
	pProcessEditorEvent=0;
	pProcessViewerEvent=0;
	pMinFarVersion=0;
}

Plugin::~Plugin()
{
    Lang.Close();
}


int Plugin::LoadFromCache ()
{
	string strRegKey;

	int cp = GetCacheNumber();

	if ( cp != -1 )
	{
		strRegKey.Format (FmtPluginsCache_PluginDW, cp);

        if ( GetRegKey(strRegKey,NFMP_PreloadW,0) == 1 ) //PF_PRELOAD plugin, skip cache
        	return Load ();

		strRegKey.Format (FmtPluginsCache_PluginDExportW,CachePos);
		SysID=GetRegKey(strRegKey,NFMP_SysIDW,0);

		pOpenPlugin=(PLUGINOPENPLUGIN)(INT_PTR)GetRegKey(strRegKey,NFMP_OpenPluginW,0);
		pOpenFilePlugin=(PLUGINOPENFILEPLUGIN)(INT_PTR)GetRegKey(strRegKey,NFMP_OpenFilePluginW,0);
		pSetFindList=(PLUGINSETFINDLIST)(INT_PTR)GetRegKey(strRegKey,NFMP_SetFindListW,0);
		pProcessEditorInput=(PLUGINPROCESSEDITORINPUT)(INT_PTR)GetRegKey(strRegKey,NFMP_ProcessEditorInputW,0);
		pProcessEditorEvent=(PLUGINPROCESSEDITOREVENT)(INT_PTR)GetRegKey(strRegKey,NFMP_ProcessEditorEventW,0);
		pProcessViewerEvent=(PLUGINPROCESSVIEWEREVENT)(INT_PTR)GetRegKey(strRegKey,NFMP_ProcessViewerEventW,0);
		pConfigure=(PLUGINCONFIGURE)(INT_PTR)GetRegKey(strRegKey,NFMP_ConfigureW,0);

		CachePos = cp;

		WorkFlags.Set(PIWF_CACHED); //too much "cached" flags

		return TRUE;
	}

	return FALSE;
}

int Plugin::SaveToCache()
{
	if ( pGetPluginInfo ||
		 pOpenPlugin ||
		 pOpenFilePlugin ||
		 pSetFindList ||
		 pProcessEditorInput ||
		 pProcessEditorEvent ||
		 pProcessViewerEvent )
	{
		PluginInfoW Info;

		GetPluginInfo(&Info);

        SysID = Info.SysID; //LAME!!!

		int j = 0;

		while ( true )
		{
			string strRegKey, strPluginName, strCurPluginID;

            strRegKey.Format (FmtPluginsCache_PluginDW, j);

            GetRegKey(strRegKey, L"Name", strPluginName, L"");

            if ( strPluginName.IsEmpty() || LocalStricmpW(strPluginName, m_strModuleName) == 0)
            {
            	DeleteKeyTree(strRegKey);

            	SetRegKey(strRegKey, L"Name", m_strModuleName);

            	strCurPluginID.Format (
            			L"%I64x%x%x",
            			FindData.nFileSize,
            			FindData.ftCreationTime.dwLowDateTime,
            			FindData.ftLastWriteTime.dwLowDateTime
            			);

				SetRegKey(strRegKey, L"ID", strCurPluginID);

				bool bPreload = (Info.Flags & PF_PRELOAD);

                SetRegKey(strRegKey, NFMP_PreloadW, bPreload?1:0);
                WorkFlags.Change(PIWF_PRELOADED, bPreload);

                if ( bPreload )
                	break;

				for (int i = 0; i < Info.DiskMenuStringsNumber; i++)
				{
					string strValue;

					strValue.Format (FmtDiskMenuStringDW, i);

					SetRegKey(strRegKey, strValue, Info.DiskMenuStrings[i]);

					if ( Info.DiskMenuNumbers )
					{
						strValue.Format (FmtDiskMenuNumberDW, i);
						SetRegKey(strRegKey, strValue, Info.DiskMenuNumbers[i]);
					}
				}

                for (int i = 0; i < Info.PluginMenuStringsNumber; i++)
                {
                	string strValue;

                	strValue.Format (FmtPluginMenuStringDW, i);
                	SetRegKey(strRegKey, strValue, Info.PluginMenuStrings[i]);
                }

                for (int i = 0; i < Info.PluginConfigStringsNumber; i++)
                {
                	string strValue;

                	strValue.Format (FmtPluginConfigStringDW, i);
                	SetRegKey(strRegKey,strValue,Info.PluginConfigStrings[i]);
                }

                SetRegKey(strRegKey, L"CommandPrefix", NullToEmptyW(Info.CommandPrefix));
                SetRegKey(strRegKey, L"Flags", Info.Flags);

				strRegKey.Format (FmtPluginsCache_PluginDExportW, j);

				SetRegKey(strRegKey, NFMP_SysIDW, SysID);
				SetRegKey(strRegKey, NFMP_OpenPluginW, pOpenPlugin!=NULL);
				SetRegKey(strRegKey, NFMP_OpenFilePluginW, pOpenFilePlugin!=NULL);
				SetRegKey(strRegKey, NFMP_SetFindListW, pSetFindList!=NULL);
				SetRegKey(strRegKey, NFMP_ProcessEditorInputW, pProcessEditorInput!=NULL);
				SetRegKey(strRegKey, NFMP_ProcessEditorEventW, pProcessEditorEvent!=NULL);
				SetRegKey(strRegKey, NFMP_ProcessViewerEventW, pProcessViewerEvent!=NULL);
				SetRegKey(strRegKey, NFMP_ConfigureW, pConfigure!=NULL);

				break;
			}

			j++;
		}

		return TRUE;
	}

	return FALSE;
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

				for (DWORD n = 0; n < pExportDir->NumberOfNames; n++)
				{
					const char *lpExportName = (const char *)&hModule[pNames[n]-nDiff];

					DWORD dwCRC32 = CRC32 (0, lpExportName, (unsigned int)strlen (lpExportName));

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
		HANDLE hModuleMapping = CreateFileMappingW (
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



int Plugin::Load()
{
	if ( WorkFlags.Check(PIWF_DONTLOADAGAIN) )
		return (FALSE);

	if ( m_hModule )
		return TRUE; //BUGBUG

	DWORD LstErr;

	if( !m_hModule )
	{
		string strCurPath, strCurPlugDiskPath;
		wchar_t Drive[4];

		Drive[0]=0; // ставим 0, как признак того, что вертать обратно ненадо!
		FarGetCurDir(strCurPath);

		if( IsLocalPathW (m_strModuleName) ) // если указан локальный путь, то...
		{
			wcscpy(Drive,L"= :");
			Drive[1] = m_strModuleName.At(0);
			apiGetEnvironmentVariable (Drive,strCurPlugDiskPath);
		}

		PrepareModulePathW(m_strModuleName);

		if ( IsModulePlugin (m_strModuleName) )
		{
		    m_hModule = LoadLibraryExW(m_strModuleName,NULL,LOAD_WITH_ALTERED_SEARCH_PATH);

		    if( !m_hModule )
		    	LstErr=GetLastError();

		    FarChDirW(strCurPath, TRUE);

		    if(Drive[0]) // вернем ее (переменную окружения) обратно
		    	SetEnvironmentVariableW(Drive,strCurPlugDiskPath);
		}
		else
			return FALSE;
	}

	if ( !m_hModule )
	{
		if (!Opt.LoadPlug.SilentLoadPlugin) //убрать в PluginSet
		{
			string strPlgName;
			strPlgName = m_strModuleName;

			TruncPathStr(strPlgName,ScrX-20);
			SetMessageHelp(L"ErrLoadPlugin");

			Message(MSG_WARNING,1,UMSG(MError),UMSG(MPlgLoadPluginError),strPlgName,UMSG(MOk));
		}

		//WorkFlags.Set(PIWF_DONTLOADAGAIN); //это с чего бы вдруг?

		return FALSE;
	}

	WorkFlags.Clear(PIWF_CACHED);

	pSetStartupInfo=(PLUGINSETSTARTUPINFO)GetProcAddress(m_hModule,NFMP_SetStartupInfo);
	pOpenPlugin=(PLUGINOPENPLUGIN)GetProcAddress(m_hModule,NFMP_OpenPlugin);
	pOpenFilePlugin=(PLUGINOPENFILEPLUGIN)GetProcAddress(m_hModule,NFMP_OpenFilePlugin);
	pClosePlugin=(PLUGINCLOSEPLUGIN)GetProcAddress(m_hModule,NFMP_ClosePlugin);
	pGetPluginInfo=(PLUGINGETPLUGININFO)GetProcAddress(m_hModule,NFMP_GetPluginInfo);
	pGetOpenPluginInfo=(PLUGINGETOPENPLUGININFO)GetProcAddress(m_hModule,NFMP_GetOpenPluginInfo);
	pGetFindData=(PLUGINGETFINDDATA)GetProcAddress(m_hModule,NFMP_GetFindData);
	pFreeFindData=(PLUGINFREEFINDDATA)GetProcAddress(m_hModule,NFMP_FreeFindData);
	pGetVirtualFindData=(PLUGINGETVIRTUALFINDDATA)GetProcAddress(m_hModule,NFMP_GetVirtualFindData);
	pFreeVirtualFindData=(PLUGINFREEVIRTUALFINDDATA)GetProcAddress(m_hModule,NFMP_FreeVirtualFindData);
	pSetDirectory=(PLUGINSETDIRECTORY)GetProcAddress(m_hModule,NFMP_SetDirectory);
	pGetFiles=(PLUGINGETFILES)GetProcAddress(m_hModule,NFMP_GetFiles);
	pPutFiles=(PLUGINPUTFILES)GetProcAddress(m_hModule,NFMP_PutFiles);
	pDeleteFiles=(PLUGINDELETEFILES)GetProcAddress(m_hModule,NFMP_DeleteFiles);
	pMakeDirectory=(PLUGINMAKEDIRECTORY)GetProcAddress(m_hModule,NFMP_MakeDirectory);
	pProcessHostFile=(PLUGINPROCESSHOSTFILE)GetProcAddress(m_hModule,NFMP_ProcessHostFile);
	pSetFindList=(PLUGINSETFINDLIST)GetProcAddress(m_hModule,NFMP_SetFindList);
	pConfigure=(PLUGINCONFIGURE)GetProcAddress(m_hModule,NFMP_Configure);
	pExitFAR=(PLUGINEXITFAR)GetProcAddress(m_hModule,NFMP_ExitFAR);
	pProcessKey=(PLUGINPROCESSKEY)GetProcAddress(m_hModule,NFMP_ProcessKey);
	pProcessEvent=(PLUGINPROCESSEVENT)GetProcAddress(m_hModule,NFMP_ProcessEvent);
	pCompare=(PLUGINCOMPARE)GetProcAddress(m_hModule,NFMP_Compare);
	pProcessEditorInput=(PLUGINPROCESSEDITORINPUT)GetProcAddress(m_hModule,NFMP_ProcessEditorInput);
	pProcessEditorEvent=(PLUGINPROCESSEDITOREVENT)GetProcAddress(m_hModule,NFMP_ProcessEditorEvent);
	pProcessViewerEvent=(PLUGINPROCESSVIEWEREVENT)GetProcAddress(m_hModule,NFMP_ProcessViewerEvent);
	pMinFarVersion=(PLUGINMINFARVERSION)GetProcAddress(m_hModule,NFMP_GetMinFarVersion);

	bool bUnloaded = false;

	if ( !CheckMinFarVersion(bUnloaded) || !SetStartupInfo(bUnloaded) )
	{
		if ( !bUnloaded )
			Unload();

		return FALSE;
	}

	FuncFlags.Set(PICFF_LOADED);

	if ( SaveToCache () )
	{
      for (int I=0;;I++)
      {
        string strRegKey, strPluginName;
        strRegKey.Format (FmtPluginsCache_PluginDW,I);
        GetRegKey(strRegKey,L"Name",strPluginName,L"");
        if ( strPluginName.IsEmpty() )
          break;
        if (GetFileAttributesW(strPluginName)==0xFFFFFFFF)
        {
          DeleteKeyRecord(FmtPluginsCache_PluginDW,I);
          I--;
        }
	  }
	}


	return TRUE;
}

void CreatePluginStartupInfo (Plugin *pPlugin, PluginStartupInfo *PSI, FarStandardFunctions *FSF)
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

    StandardFunctions.Unquote=Unquote;
    StandardFunctions.LTrim=RemoveLeadingSpacesW;
    StandardFunctions.RTrim=RemoveTrailingSpacesW;
    StandardFunctions.Trim=RemoveExternalSpacesW;
    StandardFunctions.TruncStr=TruncStr;
    StandardFunctions.TruncPathStr=TruncPathStr;
    StandardFunctions.QuoteSpaceOnly=QuoteSpaceOnly;
    StandardFunctions.PointToName=PointToName;
    //StandardFunctions.GetPathRoot=GetPathRoot; BUGBUG
    StandardFunctions.AddEndSlash=AddEndSlash;
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

  PSI->ModuleNumber=(INT_PTR)pPlugin;
  PSI->FSF=FSF;

  PSI->ModuleName = (const wchar_t*)pPlugin->m_strModuleName;
  PSI->RootKey=NULL;
}

struct ExecuteStruct {
	int id; //function id
	union {
		INT_PTR nResult;
		HANDLE hResult;
		BOOL bResult;
	};

	union {
    	INT_PTR nDefaultResult;
    	HANDLE hDefaultResult;
    	BOOL bDefaultResult;
	};

	bool bUnloaded;
};


#define EXECUTE_FUNCTION(function, es) \
	{ \
		es.nResult = 0; \
		es.nDefaultResult = 0; \
		es.bUnloaded = false; \
		if ( Opt.ExceptRules ) \
		{ \
			TRY { \
				function; \
			} \
			EXCEPT(xfilter(es.id, GetExceptionInformation(), this, 0)) \
			{ \
				m_owner->UnloadPlugin(this, es.id, true); \
				es.bUnloaded = true; \
				ProcessException=FALSE; \
			} \
		} \
		else \
			function; \
	}


#define EXECUTE_FUNCTION_EX(function, es) \
	{ \
		es.bUnloaded = false; \
		es.nResult = 0; \
		if ( Opt.ExceptRules ) \
		{ \
			TRY { \
				es.nResult = (INT_PTR)function; \
			} \
			EXCEPT(xfilter(es.id, GetExceptionInformation(), this, 0)) \
			{ \
				m_owner->UnloadPlugin(this, es.id, true); \
				es.bUnloaded = true; \
				es.nResult = es.nDefaultResult; \
				ProcessException=FALSE; \
			} \
		} \
		else \
			es.nResult = (INT_PTR)function; \
	}



int Plugin::SetStartupInfo (bool &bUnloaded)
{
	if ( pSetStartupInfo && !ProcessException )
	{
		PluginStartupInfo _info;
		FarStandardFunctions _fsf;

		CreatePluginStartupInfo(this, &_info, &_fsf);

		// скорректирем адреса и плагино-зависимые поля
		strRootKey = Opt.strRegRoot;
		strRootKey += L"\\Plugins";

		_info.RootKey = (const wchar_t*)strRootKey;

	    ExecuteStruct es;
		es.id = EXCEPT_SETSTARTUPINFO;

		EXECUTE_FUNCTION(pSetStartupInfo(&_info), es);

		if ( es.bUnloaded )
		{
			bUnloaded = true;
			return FALSE;
		}
	}

	return TRUE;
}

void ShowMessageAboutIllegalPluginVersion(const wchar_t* plg,int required)
{
    string strMsg1, strMsg2;
    string strPlgName;

    strPlgName = plg;
    TruncPathStr(strPlgName,ScrX-20);
    strMsg1.Format(UMSG(MPlgRequired),
           (WORD)HIBYTE(LOWORD(required)),(WORD)LOBYTE(LOWORD(required)),HIWORD(required));
    strMsg2.Format(UMSG(MPlgRequired2),
           (WORD)HIBYTE(LOWORD(FAR_VERSION)),(WORD)LOBYTE(LOWORD(FAR_VERSION)),HIWORD(FAR_VERSION));
    Message(MSG_WARNING,1,UMSG(MError),UMSG(MPlgBadVers),strPlgName,strMsg1,strMsg2,UMSG(MOk));
}


int Plugin::CheckMinFarVersion (bool &bUnloaded)
{
	if ( pMinFarVersion && !ProcessException )
	{
		ExecuteStruct es;

		es.id = EXCEPT_MINFARVERSION;
		es.nDefaultResult = 0;

		EXECUTE_FUNCTION_EX(pMinFarVersion(), es);

		if ( es.bUnloaded )
		{
			bUnloaded = true;
			return FALSE;
		}

		DWORD FVer = (DWORD)es.nResult;

		if ( LOWORD(FVer) >  LOWORD(FAR_VERSION) ||
			(LOWORD(FVer) == LOWORD(FAR_VERSION) &&
			HIWORD(FVer) >  HIWORD(FAR_VERSION)) )
		{
			ShowMessageAboutIllegalPluginVersion(m_strModuleName,FVer);
			return FALSE;
		}
	}

	return TRUE;
}

int Plugin::Unload (bool bExitFAR)
{
	int nResult = TRUE;

	if ( bExitFAR )
		ExitFAR();

	if ( !WorkFlags.Check(PIWF_CACHED) )
	{
		nResult = FreeLibrary(m_hModule);
		ClearExports();
	}

	m_hModule = NULL;
	FuncFlags.Clear(PICFF_LOADED); //??

	return nResult;
}

bool Plugin::IsPanelPlugin()
{
	return pSetFindList ||
		pGetFindData ||
		pGetVirtualFindData ||
		pSetDirectory ||
		pGetFiles ||
		pPutFiles ||
		pDeleteFiles ||
		pMakeDirectory ||
		pProcessHostFile ||
		pProcessKey ||
		pProcessEvent ||
		pCompare ||
		pGetOpenPluginInfo ||
		pFreeFindData ||
		pFreeVirtualFindData ||
		pClosePlugin;
}

HANDLE Plugin::OpenPlugin (int OpenFrom, INT_PTR Item)
{
  //BUGBUG???
  //AY - непонятно нафига нужно, в других вызовах нету,
  //     притом ещё делает варнинги при сборке из за того что внизу есть SEH.
  //     Если это да надо, то надо выносить вызов SEH в отдельную функцию.
  //ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);

  CheckScreenLock(); //??

  {
		string strCurDir;
		CtrlObject->CmdLine->GetCurDir(strCurDir);
		FarChDirW(strCurDir);
		g_strDirToSet=L"";
  }


  if ( pOpenPlugin && Load() && !ProcessException )
  {
		//CurPluginItem=this; //BUGBUG

		ExecuteStruct es;

		es.id = EXCEPT_OPENPLUGIN;
		es.hDefaultResult = INVALID_HANDLE_VALUE;
		es.hResult = INVALID_HANDLE_VALUE;

		EXECUTE_FUNCTION_EX(pOpenPlugin(OpenFrom,Item), es);

		return es.hResult;
    //CurPluginItem=NULL; //BUGBUG

/*    CtrlObject->Macro.SetRedrawEditor(TRUE); //BUGBUG

    if ( !es.bUnloaded )
    {

      if(OpenFrom == OPEN_EDITOR &&
         !CtrlObject->Macro.IsExecuting() &&
         CtrlObject->Plugins.CurEditor &&
         CtrlObject->Plugins.CurEditor->IsVisible() )
      {
        CtrlObject->Plugins.ProcessEditorEvent(EE_REDRAW,EEREDRAW_CHANGE);
        CtrlObject->Plugins.ProcessEditorEvent(EE_REDRAW,EEREDRAW_ALL);
        CtrlObject->Plugins.CurEditor->Show();
      }
      if (hInternal!=INVALID_HANDLE_VALUE)
      {
        PluginHandle *hPlugin=new PluginHandle;
        hPlugin->InternalHandle=es.hResult;
        hPlugin->PluginNumber=(INT_PTR)this;
        return((HANDLE)hPlugin);
      }
      else
        if ( !g_strDirToSet.IsEmpty() )
        {
          CtrlObject->Cp()->ActivePanel->SetCurDirW(g_strDirToSet,TRUE);
          CtrlObject->Cp()->ActivePanel->Redraw();
        }
    } */
  }

  return(INVALID_HANDLE_VALUE);
}

//////////////////////////////////

HANDLE Plugin::OpenFilePlugin (
		const wchar_t *Name,
		const unsigned char *Data,
		int DataSize,
		int OpMode
		)
{
//	if ( m_bCached && HAS_EXPORT(EXPORT_OPENFILEPLUGIN) )
//		Load (FORCE_LOAD);
	HANDLE hResult = INVALID_HANDLE_VALUE;

	if ( pOpenFilePlugin && Load() && !ProcessException )
	{
		ExecuteStruct es;

		es.id = EXCEPT_OPENFILEPLUGIN;
		es.hDefaultResult = INVALID_HANDLE_VALUE;

		EXECUTE_FUNCTION_EX(pOpenFilePlugin(Name, Data, DataSize, OpMode), es);

		hResult = es.hResult;
	}

	return hResult;
}


int Plugin::SetFindList (
		HANDLE hPlugin,
		const PluginPanelItemW *PanelItem,
		int ItemsNumber
		)
{
	BOOL bResult = FALSE;

	if ( pSetFindList && !ProcessException )
	{
		ExecuteStruct es;

		es.id = EXCEPT_SETFINDLIST;
		es.bDefaultResult = FALSE;

		EXECUTE_FUNCTION_EX(pSetFindList(hPlugin, PanelItem, ItemsNumber), es);

		bResult = es.bResult;
	}

	return bResult;
}

int Plugin::ProcessEditorInput (
		const INPUT_RECORD *D
		)
{
	BOOL bResult = FALSE;

	if ( pProcessEditorInput && Load() && !ProcessException )
	{
		ExecuteStruct es;

		es.id = EXCEPT_PROCESSEDITORINPUT;
		es.bDefaultResult = TRUE; //(TRUE) treat the result as a completed request on exception!

		EXECUTE_FUNCTION_EX(pProcessEditorInput(D), es);

		bResult = es.bResult;
	}

	return bResult;
}

int Plugin::ProcessEditorEvent (
		int Event,
		PVOID Param
		)
{
	if ( pProcessEditorEvent && Load() && !ProcessException )
	{
		ExecuteStruct es;

		es.id = EXCEPT_PROCESSEDITOREVENT;
		es.nDefaultResult = 0;

		EXECUTE_FUNCTION_EX(pProcessEditorEvent(Event, Param), es);
	}

	return 0; //oops!
}

int Plugin::ProcessViewerEvent (
		int Event,
		void *Param
		)
{
	if ( pProcessViewerEvent && Load() && !ProcessException )
	{
		ExecuteStruct es;

		es.id = EXCEPT_PROCESSVIEWEREVENT;
		es.nDefaultResult = 0;

		EXECUTE_FUNCTION_EX(pProcessViewerEvent(Event, Param), es);
	}

	return 0; //oops, again!
}


int Plugin::GetVirtualFindData (
		HANDLE hPlugin,
		PluginPanelItemW **pPanelItem,
		int *pItemsNumber,
		const wchar_t *Path
		)
{
	BOOL bResult = FALSE;

	if ( pGetVirtualFindData && !ProcessException )
	{
		ExecuteStruct es;

		es.id = EXCEPT_GETVIRTUALFINDDATA;
		es.bDefaultResult = FALSE;

		EXECUTE_FUNCTION_EX(pGetVirtualFindData(hPlugin, pPanelItem, pItemsNumber, Path), es);

		bResult = es.bResult;
	}

	return bResult;
}


void Plugin::FreeVirtualFindData (
		HANDLE hPlugin,
		PluginPanelItemW *PanelItem,
		int ItemsNumber
		)
{
	if ( pFreeVirtualFindData && !ProcessException )
	{
		ExecuteStruct es;
		es.id = EXCEPT_FREEVIRTUALFINDDATA;

		EXECUTE_FUNCTION(pFreeVirtualFindData(hPlugin, PanelItem, ItemsNumber), es);
	}
}



int Plugin::GetFiles (
		HANDLE hPlugin,
		PluginPanelItemW *PanelItem,
		int ItemsNumber,
		int Move,
		const wchar_t *DestPath,
		int OpMode
		)
{
	int nResult = -1;

	if ( pGetFiles && !ProcessException )
	{
		ExecuteStruct es;

		es.id = EXCEPT_GETFILES;
		es.nDefaultResult = -1;

		EXECUTE_FUNCTION_EX(pGetFiles(hPlugin, PanelItem, ItemsNumber, Move, DestPath, OpMode), es);

		nResult = (int)es.nResult;
	}

	return nResult;
}


int Plugin::PutFiles (
		HANDLE hPlugin,
		PluginPanelItemW *PanelItem,
		int ItemsNumber,
		int Move,
		int OpMode
		)
{
	int nResult = -1;

	if ( pPutFiles && !ProcessException )
	{
		ExecuteStruct es;

		es.id = EXCEPT_PUTFILES;
		es.nDefaultResult = -1;

		EXECUTE_FUNCTION_EX(pPutFiles(hPlugin, PanelItem, ItemsNumber, Move, OpMode), es);

		nResult = (int)es.nResult;
	}

	return nResult;
}

int Plugin::DeleteFiles (
		HANDLE hPlugin,
		PluginPanelItemW *PanelItem,
		int ItemsNumber,
		int OpMode
		)
{
	BOOL bResult = FALSE;

	if ( pDeleteFiles && !ProcessException )
	{
		ExecuteStruct es;

		es.id = EXCEPT_DELETEFILES;
		es.bDefaultResult = FALSE;

		EXECUTE_FUNCTION_EX(pDeleteFiles(hPlugin, PanelItem, ItemsNumber, OpMode), es);

		bResult = (int)es.bResult;
	}

	return bResult;
}


int Plugin::MakeDirectory (
		HANDLE hPlugin,
		const wchar_t *Name,
		int OpMode
		)
{
	int nResult = -1;

	if ( pMakeDirectory && !ProcessException )
	{
		ExecuteStruct es;

		es.id = EXCEPT_MAKEDIRECTORY;
		es.nDefaultResult = -1;

		EXECUTE_FUNCTION_EX(pMakeDirectory(hPlugin, Name, OpMode), es);

		nResult = (int)es.nResult;
	}

	return nResult;
}


int Plugin::ProcessHostFile (
		HANDLE hPlugin,
		PluginPanelItemW *PanelItem,
		int ItemsNumber,
		int OpMode
		)
{
	BOOL bResult = FALSE;

	if ( pProcessHostFile && !ProcessException )
	{
		ExecuteStruct es;

		es.id = EXCEPT_PROCESSHOSTFILE;
		es.bDefaultResult = FALSE;

		EXECUTE_FUNCTION_EX(pProcessHostFile(hPlugin, PanelItem, ItemsNumber, OpMode), es);

		bResult = es.bResult;
	}

	return bResult;
}


int Plugin::ProcessEvent (
		HANDLE hPlugin,
		int Event,
		PVOID Param
		)
{
	BOOL bResult = FALSE;

	if ( pProcessEvent && !ProcessException )
	{
		ExecuteStruct es;

		es.id = EXCEPT_PROCESSEVENT;
		es.bDefaultResult = FALSE;

		EXECUTE_FUNCTION_EX(pProcessEvent(hPlugin, Event, Param), es);

		bResult = es.bResult;
	}

	return bResult;
}


int Plugin::Compare (
		HANDLE hPlugin,
		const PluginPanelItemW *Item1,
		const PluginPanelItemW *Item2,
		DWORD Mode
		)
{
	int nResult = -2;

	if ( pCompare && !ProcessException )
	{
		ExecuteStruct es;

		es.id = EXCEPT_COMPARE;
		es.nDefaultResult = -2;

		EXECUTE_FUNCTION_EX(pCompare(hPlugin, Item1, Item2, Mode), es);

		nResult = (int)es.nResult;
	}

	return nResult;
}


int Plugin::GetFindData (
		HANDLE hPlugin,
		PluginPanelItemW **pPanelItem,
		int *pItemsNumber,
		int OpMode
		)
{
	BOOL bResult = FALSE;

	if ( pGetFindData && !ProcessException )
	{
		ExecuteStruct es;

		es.id = EXCEPT_GETFINDDATA;
		es.bDefaultResult = FALSE;

		EXECUTE_FUNCTION_EX(pGetFindData(hPlugin, pPanelItem, pItemsNumber, OpMode), es);

		bResult = es.bResult;
	}

	return bResult;
}


void Plugin::FreeFindData (
		HANDLE hPlugin,
		PluginPanelItemW *PanelItem,
		int ItemsNumber
		)
{
	if ( pFreeFindData && !ProcessException )
	{
		ExecuteStruct es;
		es.id = EXCEPT_FREEFINDDATA;

		EXECUTE_FUNCTION(pFreeFindData(hPlugin, PanelItem, ItemsNumber), es);
	}
}

int Plugin::ProcessKey (
		HANDLE hPlugin,
		int Key,
		unsigned int dwControlState
		)
{
	BOOL bResult = FALSE;

	if ( pProcessKey && !ProcessException )
	{
		ExecuteStruct es;

		es.id = EXCEPT_PROCESSKEY;
		es.bDefaultResult = TRUE; // do not pass this key to far on exception

		EXECUTE_FUNCTION_EX(pProcessKey(hPlugin, Key, dwControlState), es);

		bResult = es.bResult;
	}

	return bResult;
}


void Plugin::ClosePlugin (
		HANDLE hPlugin
		)
{
	if ( pClosePlugin && !ProcessException )
	{
		ExecuteStruct es;

		es.id = EXCEPT_CLOSEPLUGIN;

		EXECUTE_FUNCTION(pClosePlugin(hPlugin), es);
	}

//	m_pManager->m_pCurrentPlugin = (Plugin*)-1;
}


int Plugin::SetDirectory (
		HANDLE hPlugin,
		const wchar_t *Dir,
		int OpMode
		)
{
	BOOL bResult = FALSE;

	if ( pSetDirectory && !ProcessException )
	{
		ExecuteStruct es;

		es.id = EXCEPT_SETDIRECTORY;
		es.bDefaultResult = FALSE;

		EXECUTE_FUNCTION_EX(pSetDirectory(hPlugin, Dir, OpMode), es);

		bResult = es.bResult;
	}

	return bResult;
}


void Plugin::GetOpenPluginInfo (
		HANDLE hPlugin,
		OpenPluginInfoW *pInfo
		)
{
//	m_pManager->m_pCurrentPlugin = this;

	pInfo->StructSize = sizeof(OpenPluginInfoW);

	if ( pGetOpenPluginInfo && !ProcessException )
	{
		ExecuteStruct es;
		es.id = EXCEPT_GETOPENPLUGININFO;

		EXECUTE_FUNCTION(pGetOpenPluginInfo(hPlugin, pInfo), es);
	}
}


int Plugin::Configure(
		int MenuItem
		)
{
	BOOL bResult = FALSE;

	if ( pConfigure && Load() && !ProcessException )
	{
		ExecuteStruct es;

		es.id = EXCEPT_CONFIGURE;
		es.bDefaultResult = FALSE;

		EXECUTE_FUNCTION_EX(pConfigure(MenuItem), es);

		bResult = es.bResult;
	}

	return bResult;
}


void Plugin::GetPluginInfo (PluginInfoW *pi)
{
	memset (pi, 0, sizeof (PluginInfoW));

	if ( pGetPluginInfo && !ProcessException )
	{
		ExecuteStruct es;

		es.id = EXCEPT_GETPLUGININFO;

		EXECUTE_FUNCTION(pGetPluginInfo(pi), es);
	}
}

void Plugin::ExitFAR()
{
	if ( pExitFAR && !ProcessException )
	{
		ExecuteStruct es;

		es.id = EXCEPT_EXITFAR;

		EXECUTE_FUNCTION(pExitFAR(), es);
	}
}

int Plugin::GetCacheNumber () //ничего не понимаю....
{
	for (int i = -1 ;; i++)
	{
		if ( (i == -1) && (CachePos == 0) )
			continue;

		int Pos = (i == -1)?CachePos:i;

		string strRegKey, strPluginName, strPluginID, strCurPluginID;

        strRegKey.Format (FmtPluginsCache_PluginDW, Pos);

		GetRegKey(strRegKey, L"Name", strPluginName, L"");

        if ( strPluginName.IsEmpty() )
        	break;

		if ( LocalStricmpW(strPluginName, m_strModuleName) != 0 )
			continue;

		GetRegKey(strRegKey, L"ID", strPluginID, L"");

		if ( !FindData.nFileSize == 0 ) //BUGBUG!!!
		{
			strCurPluginID.Format (
					L"%I64x%x%x",
					FindData.nFileSize,
					FindData.ftCreationTime.dwLowDateTime,
					FindData.ftLastWriteTime.dwLowDateTime
					);

			if ( wcscmp(strPluginID, strCurPluginID) != 0 )
				continue;
		}

	    return Pos;
	}

	return -1;
}


void Plugin::ClearExports()
{
	pSetStartupInfo=0;
	pOpenPlugin=0;
	pOpenFilePlugin=0;
	pClosePlugin=0;
	pGetPluginInfo=0;
	pGetOpenPluginInfo=0;
	pGetFindData=0;
	pFreeFindData=0;
	pGetVirtualFindData=0;
	pFreeVirtualFindData=0;
	pSetDirectory=0;
	pGetFiles=0;
	pPutFiles=0;
	pDeleteFiles=0;
	pMakeDirectory=0;
	pProcessHostFile=0;
	pSetFindList=0;
	pConfigure=0;
	pExitFAR=0;
	pProcessKey=0;
	pProcessEvent=0;
	pCompare=0;
	pProcessEditorInput=0;
	pProcessEditorEvent=0;
	pProcessViewerEvent=0;
	pMinFarVersion=0;
}
