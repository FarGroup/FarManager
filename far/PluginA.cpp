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
#include "PluginA.hpp"

#include "wrap.cpp"

static const wchar_t *wszReg_Preload=L"Preload";
static const wchar_t *wszReg_SysID=L"SysID";

static const wchar_t wszReg_OpenPlugin[]=L"OpenPlugin";
static const wchar_t wszReg_OpenFilePlugin[]=L"OpenFilePlugin";
static const wchar_t wszReg_SetFindList[]=L"SetFindList";
static const wchar_t wszReg_ProcessEditorInput[]=L"ProcessEditorInput";
static const wchar_t wszReg_ProcessEditorEvent[]=L"ProcessEditorEvent";
static const wchar_t wszReg_ProcessViewerEvent[]=L"ProcessViewerEvent";
static const wchar_t wszReg_SetStartupInfo[]=L"SetStartupInfo";
static const wchar_t wszReg_ClosePlugin[]=L"ClosePlugin";
static const wchar_t wszReg_GetPluginInfo[]=L"GetPluginInfo";
static const wchar_t wszReg_GetOpenPluginInfo[]=L"GetOpenPluginInfo";
static const wchar_t wszReg_GetFindData[]=L"GetFindData";
static const wchar_t wszReg_FreeFindData[]=L"FreeFindData";
static const wchar_t wszReg_GetVirtualFindData[]=L"GetVirtualFindData";
static const wchar_t wszReg_FreeVirtualFindData[]=L"FreeVirtualFindData";
static const wchar_t wszReg_SetDirectory[]=L"SetDirectory";
static const wchar_t wszReg_GetFiles[]=L"GetFiles";
static const wchar_t wszReg_PutFiles[]=L"PutFiles";
static const wchar_t wszReg_DeleteFiles[]=L"DeleteFiles";
static const wchar_t wszReg_MakeDirectory[]=L"MakeDirectory";
static const wchar_t wszReg_ProcessHostFile[]=L"ProcessHostFile";
static const wchar_t wszReg_Configure[]=L"Configure";
static const wchar_t wszReg_ExitFAR[]=L"ExitFAR";
static const wchar_t wszReg_ProcessKey[]=L"ProcessKey";
static const wchar_t wszReg_ProcessEvent[]=L"ProcessEvent";
static const wchar_t wszReg_Compare[]=L"Compare";
static const wchar_t wszReg_GetMinFarVersion[]=L"GetMinFarVersion";


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


static BOOL PrepareModulePath(const wchar_t *ModuleName)
{
	string strModulePath;
	strModulePath = ModuleName;

	CutToSlash(strModulePath); //??

	return FarChDir(strModulePath,TRUE);
}

static void CheckScreenLock()
{
	if ( ScrBuf.GetLockCount() > 0 && !CtrlObject->Macro.PeekKey() )
	{
		ScrBuf.SetLockCount(0);
		ScrBuf.Flush();
	}
}



PluginA::PluginA (
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

	RootKey = NULL;

	m_strModuleName = lpwszModuleName;

	ClearExports();

	memset(&PI,0,sizeof(PI));
}

PluginA::~PluginA()
{
	if (RootKey) free(RootKey);
	FreePluginInfo();
	Lang.Close();
}


int PluginA::LoadFromCache ()
{
	string strRegKey;

	int cp = GetCacheNumber();

	if ( cp != -1 )
	{
		strRegKey.Format (FmtPluginsCache_PluginD, cp);

		if ( GetRegKey(strRegKey,wszReg_Preload,0) == 1 ) //PF_PRELOAD plugin, skip cache
			return Load ();

		strRegKey.Format (FmtPluginsCache_PluginDExport,CachePos);
		SysID=GetRegKey(strRegKey,wszReg_SysID,0);

		pOpenPlugin=(PLUGINOPENPLUGIN)(INT_PTR)GetRegKey(strRegKey,wszReg_OpenPlugin,0);
		pOpenFilePlugin=(PLUGINOPENFILEPLUGIN)(INT_PTR)GetRegKey(strRegKey,wszReg_OpenFilePlugin,0);
		pSetFindList=(PLUGINSETFINDLIST)(INT_PTR)GetRegKey(strRegKey,wszReg_SetFindList,0);
		pProcessEditorInput=(PLUGINPROCESSEDITORINPUT)(INT_PTR)GetRegKey(strRegKey,wszReg_ProcessEditorInput,0);
		pProcessEditorEvent=(PLUGINPROCESSEDITOREVENT)(INT_PTR)GetRegKey(strRegKey,wszReg_ProcessEditorEvent,0);
		pProcessViewerEvent=(PLUGINPROCESSVIEWEREVENT)(INT_PTR)GetRegKey(strRegKey,wszReg_ProcessViewerEvent,0);
		pConfigure=(PLUGINCONFIGURE)(INT_PTR)GetRegKey(strRegKey,wszReg_Configure,0);

		CachePos = cp;

		WorkFlags.Set(PIWF_CACHED); //too much "cached" flags

		return TRUE;
	}

	return FALSE;
}

int PluginA::SaveToCache()
{
	if ( pGetPluginInfo ||
		 pOpenPlugin ||
		 pOpenFilePlugin ||
		 pSetFindList ||
		 pProcessEditorInput ||
		 pProcessEditorEvent ||
		 pProcessViewerEvent )
	{
		PluginInfo Info;

		GetPluginInfo(&Info);

		SysID = Info.SysID; //LAME!!!

		int j = 0;

		while ( true )
		{
			string strRegKey, strPluginName, strCurPluginID;

			strRegKey.Format (FmtPluginsCache_PluginD, j);

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

				SetRegKey(strRegKey, wszReg_Preload, bPreload?1:0);
				WorkFlags.Change(PIWF_PRELOADED, bPreload);

				if ( bPreload )
					break;

				for (int i = 0; i < Info.DiskMenuStringsNumber; i++)
				{
					string strValue;

					strValue.Format (FmtDiskMenuStringD, i);

					SetRegKey(strRegKey, strValue, Info.DiskMenuStrings[i]);

					if ( Info.DiskMenuNumbers )
					{
						strValue.Format (FmtDiskMenuNumberD, i);
						SetRegKey(strRegKey, strValue, Info.DiskMenuNumbers[i]);
					}
				}

				for (int i = 0; i < Info.PluginMenuStringsNumber; i++)
				{
					string strValue;

					strValue.Format (FmtPluginMenuStringD, i);
					SetRegKey(strRegKey, strValue, Info.PluginMenuStrings[i]);
				}

				for (int i = 0; i < Info.PluginConfigStringsNumber; i++)
				{
					string strValue;

					strValue.Format (FmtPluginConfigStringD, i);
					SetRegKey(strRegKey,strValue,Info.PluginConfigStrings[i]);
				}

				SetRegKey(strRegKey, L"CommandPrefix", NullToEmpty(Info.CommandPrefix));
				SetRegKey(strRegKey, L"Flags", Info.Flags);

				strRegKey.Format (FmtPluginsCache_PluginDExport, j);

				SetRegKey(strRegKey, wszReg_SysID, SysID);
				SetRegKey(strRegKey, wszReg_OpenPlugin, pOpenPlugin!=NULL);
				SetRegKey(strRegKey, wszReg_OpenFilePlugin, pOpenFilePlugin!=NULL);
				SetRegKey(strRegKey, wszReg_SetFindList, pSetFindList!=NULL);
				SetRegKey(strRegKey, wszReg_ProcessEditorInput, pProcessEditorInput!=NULL);
				SetRegKey(strRegKey, wszReg_ProcessEditorEvent, pProcessEditorEvent!=NULL);
				SetRegKey(strRegKey, wszReg_ProcessViewerEvent, pProcessViewerEvent!=NULL);
				SetRegKey(strRegKey, wszReg_Configure, pConfigure!=NULL);

				break;
			}

			j++;
		}

		return TRUE;
	}

	return FALSE;
}

int PluginA::Load()
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

		if( IsLocalPath(m_strModuleName) ) // если указан локальный путь, то...
		{
			wcscpy(Drive,L"= :");
			Drive[1] = m_strModuleName.At(0);
			apiGetEnvironmentVariable (Drive,strCurPlugDiskPath);
		}

		PrepareModulePath(m_strModuleName);

		m_hModule = LoadLibraryExW(m_strModuleName,NULL,LOAD_WITH_ALTERED_SEARCH_PATH);

		if( !m_hModule )
			LstErr=GetLastError();

		FarChDir(strCurPath, TRUE);

		if(Drive[0]) // вернем ее (переменную окружения) обратно
			SetEnvironmentVariableW(Drive,strCurPlugDiskPath);
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
			strRegKey.Format (FmtPluginsCache_PluginD,I);
			GetRegKey(strRegKey,L"Name",strPluginName,L"");
			if ( strPluginName.IsEmpty() )
				break;
			if (GetFileAttributesW(strPluginName)==0xFFFFFFFF)
			{
				DeleteKeyRecord(FmtPluginsCache_PluginD,I);
				I--;
			}
	  }
	}


	return TRUE;
}

static void CreatePluginStartupInfoA (PluginA *pPlugin, oldfar::PluginStartupInfo *PSI, oldfar::FarStandardFunctions *FSF)
{
  static oldfar::PluginStartupInfo StartupInfo={0};
  static oldfar::FarStandardFunctions StandardFunctions={0};

  // заполняем структуру StandardFunctions один раз!!!
  if(!StandardFunctions.StructSize)
  {
    StandardFunctions.StructSize=sizeof(StandardFunctions);
    StandardFunctions.sprintf=sprintf;
    StandardFunctions.snprintf=_snprintf;
    StandardFunctions.sscanf=sscanf;
    StandardFunctions.qsort=FarQsort;
    StandardFunctions.qsortex=FarQsortEx;
    StandardFunctions.atoi=FarAtoiA;
    StandardFunctions.atoi64=FarAtoi64A;
    StandardFunctions.itoa=FarItoaA;
    StandardFunctions.itoa64=FarItoa64A;
    StandardFunctions.bsearch=FarBsearch;
    StandardFunctions.LIsLower   =LocalIslower;
    StandardFunctions.LIsUpper   =LocalIsupper;
    StandardFunctions.LIsAlpha   =LocalIsalpha;
    StandardFunctions.LIsAlphanum=LocalIsalphanum;
    StandardFunctions.LUpper     =LocalUpper;
    StandardFunctions.LUpperBuf  =LocalUpperBuf;
    StandardFunctions.LLowerBuf  =LocalLowerBuf;
    StandardFunctions.LLower     =LocalLower;
    StandardFunctions.LStrupr    =LocalStrupr;
    StandardFunctions.LStrlwr    =LocalStrlwr;
    StandardFunctions.LStricmp   =LStricmp;
    StandardFunctions.LStrnicmp  =LStrnicmp;

    StandardFunctions.Unquote=UnquoteA;
    StandardFunctions.LTrim=RemoveLeadingSpacesA;
    StandardFunctions.RTrim=RemoveTrailingSpacesA;
    StandardFunctions.Trim=RemoveExternalSpacesA;
    StandardFunctions.TruncStr=TruncStrA;
    StandardFunctions.TruncPathStr=TruncPathStrA;
    StandardFunctions.QuoteSpaceOnly=QuoteSpaceOnlyA;
    StandardFunctions.PointToName=PointToNameA;
    StandardFunctions.GetPathRoot=GetPathRootA;
    StandardFunctions.AddEndSlash=AddEndSlashA;
    StandardFunctions.CopyToClipboard=CopyToClipboardA;
    StandardFunctions.PasteFromClipboard=PasteFromClipboardA;
    StandardFunctions.FarKeyToName=FarKeyToNameA; //BUGBUG или нет?
    StandardFunctions.FarNameToKey=KeyNameToKeyA; //BUGBUG или нет?
    StandardFunctions.FarInputRecordToKey=InputRecordToKey;//BUGBUG или нет?
    //StandardFunctions.XLat=XlatA;
    //StandardFunctions.GetFileOwner=GetFileOwner; //BUGBUG
    //StandardFunctions.GetNumberOfLinks=GetNumberOfLinks;
    //StandardFunctions.FarRecursiveSearch=FarRecursiveSearch;
    StandardFunctions.MkTemp=FarMkTempA;
    StandardFunctions.DeleteBuffer=DeleteBufferA;
    StandardFunctions.ProcessName=ProcessNameA;
    //StandardFunctions.MkLink=FarMkLink;
    //StandardFunctions.ConvertNameToReal=ConvertNameToReal; //BUGBUG
    //StandardFunctions.GetReparsePointInfo=FarGetReparsePointInfo; //BUGBUG
    StandardFunctions.ExpandEnvironmentStr=ExpandEnvironmentStrA;
  }

  if(!StartupInfo.StructSize)
  {
    StartupInfo.StructSize=sizeof(StartupInfo);
    StartupInfo.Menu=FarMenuFnA; //заглушка
    StartupInfo.Dialog=FarDialogFnA; //заглушка
    StartupInfo.GetMsg=FarGetMsgFnA; //заглушка
    StartupInfo.Message=FarMessageFnA;
    StartupInfo.Control=FarControlA; //заглушка
    StartupInfo.SaveScreen=FarSaveScreen;
    StartupInfo.RestoreScreen=FarRestoreScreen;
    StartupInfo.GetDirList=FarGetDirListA; //заглушка
    StartupInfo.GetPluginDirList=FarGetPluginDirListA; //заглушка
    StartupInfo.FreeDirList=FarFreeDirListA; //заглушка
    StartupInfo.Viewer=FarViewerA;
    StartupInfo.Editor=FarEditorA;
    StartupInfo.CmpName=FarCmpNameA;
    //StartupInfo.CharTable=FarCharTable;
    StartupInfo.Text=FarTextA;
    StartupInfo.EditorControl=FarEditorControlA; //заглушка
    StartupInfo.ViewerControl=FarViewerControlA; //заглушка
    StartupInfo.ShowHelp=FarShowHelpA;
    StartupInfo.AdvControl=FarAdvControlA; //заглушка
    StartupInfo.DialogEx=FarDialogExA; //заглушка
    //StartupInfo.SendDlgMessage=FarSendDlgMessage;
    //StartupInfo.DefDlgProc=FarDefDlgProc;
    StartupInfo.InputBox=FarInputBoxA;
  }

  memcpy(PSI,&StartupInfo,sizeof(StartupInfo));
  memcpy(FSF,&StandardFunctions,sizeof(StandardFunctions));

  PSI->ModuleNumber=(INT_PTR)pPlugin;
  PSI->FSF=FSF;

  pPlugin->GetModuleName().GetCharString(PSI->ModuleName,sizeof(PSI->ModuleName)-1);
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
			EXCEPT(xfilter(es.id, GetExceptionInformation(), (Plugin *)this, 0)) \
			{ \
				m_owner->UnloadPlugin((Plugin *)this, es.id, true); \
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
			EXCEPT(xfilter(es.id, GetExceptionInformation(), (Plugin *)this, 0)) \
			{ \
				m_owner->UnloadPlugin((Plugin *)this, es.id, true); \
				es.bUnloaded = true; \
				es.nResult = es.nDefaultResult; \
				ProcessException=FALSE; \
			} \
		} \
		else \
			es.nResult = (INT_PTR)function; \
	}



int PluginA::SetStartupInfo (bool &bUnloaded)
{
	if ( pSetStartupInfo && !ProcessException )
	{
		oldfar::PluginStartupInfo _info;
		oldfar::FarStandardFunctions _fsf;

		CreatePluginStartupInfoA(this, &_info, &_fsf);

		// скорректирем адреса и плагино-зависимые поля
		strRootKey = Opt.strRegRoot;
		strRootKey += L"\\Plugins";
		RootKey = UnicodeToAnsi(strRootKey);

		_info.RootKey = RootKey;

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

static void ShowMessageAboutIllegalPluginVersion(const wchar_t* plg,int required)
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


int PluginA::CheckMinFarVersion (bool &bUnloaded)
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

int PluginA::Unload (bool bExitFAR)
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

bool PluginA::IsPanelPlugin()
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

HANDLE PluginA::OpenPlugin (int OpenFrom, INT_PTR Item)
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
		FarChDir(strCurDir);
		g_strDirToSet=L"";
  }


  if ( pOpenPlugin && Load() && !ProcessException )
  {
		//CurPluginItem=this; //BUGBUG

		ExecuteStruct es;

		es.id = EXCEPT_OPENPLUGIN;
		es.hDefaultResult = INVALID_HANDLE_VALUE;
		es.hResult = INVALID_HANDLE_VALUE;

		char *ItemA = NULL;

		if (Item && (OpenFrom == OPEN_COMMANDLINE  || OpenFrom == OPEN_SHORTCUT))
		{
			ItemA = UnicodeToAnsi((const wchar_t *)Item);
			Item = (INT_PTR)ItemA;
		}

		EXECUTE_FUNCTION_EX(pOpenPlugin(OpenFrom,Item), es);

		if (ItemA) free(ItemA);

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

HANDLE PluginA::OpenFilePlugin (
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

		char *NameA = UnicodeToAnsi(Name);

		EXECUTE_FUNCTION_EX(pOpenFilePlugin(NameA, Data, DataSize), es);

		if (NameA) free(NameA);

		hResult = es.hResult;
	}

	return hResult;
}


int PluginA::SetFindList (
		HANDLE hPlugin,
		const PluginPanelItem *PanelItem,
		int ItemsNumber
		)
{
	BOOL bResult = FALSE;

	if ( pSetFindList && !ProcessException )
	{
		ExecuteStruct es;

		es.id = EXCEPT_SETFINDLIST;
		es.bDefaultResult = FALSE;

		//TODO!!! EXECUTE_FUNCTION_EX(pSetFindList(hPlugin, PanelItem, ItemsNumber), es);
  	es.nResult = es.nDefaultResult; //REMOVE WHEN TODO REMOVED

		bResult = es.bResult;
	}

	return bResult;
}

int PluginA::ProcessEditorInput (
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

int PluginA::ProcessEditorEvent (
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

int PluginA::ProcessViewerEvent (
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


int PluginA::GetVirtualFindData (
		HANDLE hPlugin,
		PluginPanelItem **pPanelItem,
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

		//TODO!!! EXECUTE_FUNCTION_EX(pGetVirtualFindData(hPlugin, pPanelItem, pItemsNumber, Path), es);
		es.nResult = es.nDefaultResult; //REMOVE WHEN TODO REMOVED

		bResult = es.bResult;
	}

	return bResult;
}


void PluginA::FreeVirtualFindData (
		HANDLE hPlugin,
		PluginPanelItem *PanelItem,
		int ItemsNumber
		)
{
	if ( pFreeVirtualFindData && !ProcessException )
	{
		ExecuteStruct es;
		es.id = EXCEPT_FREEVIRTUALFINDDATA;

		//TODO!!! EXECUTE_FUNCTION(pFreeVirtualFindData(hPlugin, PanelItem, ItemsNumber), es);
	}
}



int PluginA::GetFiles (
		HANDLE hPlugin,
		PluginPanelItem *PanelItem,
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

		//TODO!!! EXECUTE_FUNCTION_EX(pGetFiles(hPlugin, PanelItem, ItemsNumber, Move, DestPath, OpMode), es);
		es.nResult = es.nDefaultResult; //REMOVE WHEN TODO REMOVED

		nResult = (int)es.nResult;
	}

	return nResult;
}


int PluginA::PutFiles (
		HANDLE hPlugin,
		PluginPanelItem *PanelItem,
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

		//TODO!!! EXECUTE_FUNCTION_EX(pPutFiles(hPlugin, PanelItem, ItemsNumber, Move, OpMode), es);
		es.nResult = es.nDefaultResult; //REMOVE WHEN TODO REMOVED

		nResult = (int)es.nResult;
	}

	return nResult;
}

int PluginA::DeleteFiles (
		HANDLE hPlugin,
		PluginPanelItem *PanelItem,
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

		//TODO!!! EXECUTE_FUNCTION_EX(pDeleteFiles(hPlugin, PanelItem, ItemsNumber, OpMode), es);
		es.nResult = es.nDefaultResult; //REMOVE WHEN TODO REMOVED

		bResult = (int)es.bResult;
	}

	return bResult;
}


int PluginA::MakeDirectory (
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

		//TODO!!! EXECUTE_FUNCTION_EX(pMakeDirectory(hPlugin, Name, OpMode), es);
		es.nResult = es.nDefaultResult; //REMOVE WHEN TODO REMOVED

		nResult = (int)es.nResult;
	}

	return nResult;
}


int PluginA::ProcessHostFile (
		HANDLE hPlugin,
		PluginPanelItem *PanelItem,
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

		//TODO!!! EXECUTE_FUNCTION_EX(pProcessHostFile(hPlugin, PanelItem, ItemsNumber, OpMode), es);
		es.nResult = es.nDefaultResult; //REMOVE WHEN TODO REMOVED

		bResult = es.bResult;
	}

	return bResult;
}


int PluginA::ProcessEvent (
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

		char *ParamA = NULL;
		if (Param && Event == FE_COMMAND)
		{
			ParamA = UnicodeToAnsi((const wchar_t *)Param);
			Param = (PVOID)ParamA;
		}

		EXECUTE_FUNCTION_EX(pProcessEvent(hPlugin, Event, Param), es);

		if (ParamA) free(ParamA);

		bResult = es.bResult;
	}

	return bResult;
}


int PluginA::Compare (
		HANDLE hPlugin,
		const PluginPanelItem *Item1,
		const PluginPanelItem *Item2,
		DWORD Mode
		)
{
	int nResult = -2;

	if ( pCompare && !ProcessException )
	{
		ExecuteStruct es;

		es.id = EXCEPT_COMPARE;
		es.nDefaultResult = -2;

		//TODO!!! EXECUTE_FUNCTION_EX(pCompare(hPlugin, Item1, Item2, Mode), es);
		es.nResult = es.nDefaultResult; //REMOVE WHEN TODO REMOVED

		nResult = (int)es.nResult;
	}

	return nResult;
}


int PluginA::GetFindData (
		HANDLE hPlugin,
		PluginPanelItem **pPanelItem,
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

		//TODO!!! EXECUTE_FUNCTION_EX(pGetFindData(hPlugin, pPanelItem, pItemsNumber, OpMode), es);
		es.nResult = es.nDefaultResult; //REMOVE WHEN TODO REMOVED

		bResult = es.bResult;
	}

	return bResult;
}


void PluginA::FreeFindData (
		HANDLE hPlugin,
		PluginPanelItem *PanelItem,
		int ItemsNumber
		)
{
	if ( pFreeFindData && !ProcessException )
	{
		ExecuteStruct es;
		es.id = EXCEPT_FREEFINDDATA;

		//TODO!!! EXECUTE_FUNCTION(pFreeFindData(hPlugin, PanelItem, ItemsNumber), es);
	}
}

int PluginA::ProcessKey (
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

		//TODO!!! EXECUTE_FUNCTION_EX(pProcessKey(hPlugin, Key, dwControlState), es);
		es.nResult = es.nDefaultResult; //REMOVE WHEN TODO REMOVED

		bResult = es.bResult;
	}

	return bResult;
}


void PluginA::ClosePlugin (
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


int PluginA::SetDirectory (
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

		char *DirA = UnicodeToAnsi(Dir);

		EXECUTE_FUNCTION_EX(pSetDirectory(hPlugin, DirA, OpMode), es);

		if (DirA) free(DirA);

		bResult = es.bResult;
	}

	return bResult;
}


void PluginA::GetOpenPluginInfo (
		HANDLE hPlugin,
		OpenPluginInfo *pInfo
		)
{
//	m_pManager->m_pCurrentPlugin = this;

	pInfo->StructSize = sizeof(OpenPluginInfo);

	if ( pGetOpenPluginInfo && !ProcessException )
	{
		ExecuteStruct es;
		es.id = EXCEPT_GETOPENPLUGININFO;

		//TODO!!! EXECUTE_FUNCTION(pGetOpenPluginInfo(hPlugin, pInfo), es);
	}
}


int PluginA::Configure(
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

void PluginA::FreePluginInfo()
{
	if (PI.DiskMenuStringsNumber)
	{
		for (int i=0; i<PI.DiskMenuStringsNumber; i++)
			free((void *)PI.DiskMenuStrings[i]);
		free((void *)PI.DiskMenuStrings);
  }
	if (PI.PluginMenuStringsNumber)
	{
		for (int i=0; i<PI.PluginMenuStringsNumber; i++)
			free((void *)PI.PluginMenuStrings[i]);
		free((void *)PI.PluginMenuStrings);
  }
	if (PI.PluginConfigStringsNumber)
	{
		for (int i=0; i<PI.PluginConfigStringsNumber; i++)
			free((void *)PI.PluginConfigStrings[i]);
		free((void *)PI.PluginConfigStrings);
  }
  if (PI.CommandPrefix)
  	free((void *)PI.CommandPrefix);

	memset(&PI,0,sizeof(PI));
}

void PluginA::ConvertPluginInfo(oldfar::PluginInfo &Src, PluginInfo *Dest)
{
	FreePluginInfo();

	PI.StructSize = sizeof(PI);
	PI.Flags = Src.Flags;

	if (Src.DiskMenuStringsNumber)
	{
		wchar_t **p = (wchar_t **) malloc(Src.DiskMenuStringsNumber*sizeof(wchar_t*));
		for (int i=0; i<Src.DiskMenuStringsNumber; i++)
			p[i] = AnsiToUnicode(Src.DiskMenuStrings[i]);
		PI.DiskMenuStrings = p;
		PI.DiskMenuNumbers = Src.DiskMenuNumbers;
		PI.DiskMenuStringsNumber = Src.DiskMenuStringsNumber;
  }
	if (Src.PluginMenuStringsNumber)
	{
		wchar_t **p = (wchar_t **) malloc(Src.PluginMenuStringsNumber*sizeof(wchar_t*));
		for (int i=0; i<Src.PluginMenuStringsNumber; i++)
			p[i] = AnsiToUnicode(Src.PluginMenuStrings[i]);
		PI.PluginMenuStrings = p;
		PI.PluginMenuStringsNumber = Src.PluginMenuStringsNumber;
  }
	if (Src.PluginConfigStringsNumber)
	{
		wchar_t **p = (wchar_t **) malloc(Src.PluginConfigStringsNumber*sizeof(wchar_t*));
		for (int i=0; i<Src.PluginConfigStringsNumber; i++)
			p[i] = AnsiToUnicode(Src.PluginConfigStrings[i]);
		PI.PluginConfigStrings = p;
		PI.PluginConfigStringsNumber = Src.PluginConfigStringsNumber;
  }
  if (Src.CommandPrefix)
  	PI.CommandPrefix = AnsiToUnicode(Src.CommandPrefix);

  memcpy(Dest,&PI,sizeof(*Dest));
}

void PluginA::GetPluginInfo (PluginInfo *pi)
{
	memset (pi, 0, sizeof (PluginInfo));

	if ( pGetPluginInfo && !ProcessException )
	{
		ExecuteStruct es;

		es.id = EXCEPT_GETPLUGININFO;

		oldfar::PluginInfo InfoA;
		memset(&InfoA,0,sizeof(InfoA));

		EXECUTE_FUNCTION(pGetPluginInfo(&InfoA), es);

		ConvertPluginInfo(InfoA, pi);
	}
}

void PluginA::ExitFAR()
{
	if ( pExitFAR && !ProcessException )
	{
		ExecuteStruct es;

		es.id = EXCEPT_EXITFAR;

		EXECUTE_FUNCTION(pExitFAR(), es);
	}
}

int PluginA::GetCacheNumber () //ничего не понимаю....
{
	for (int i = -1 ;; i++)
	{
		if ( (i == -1) && (CachePos == 0) )
			continue;

		int Pos = (i == -1)?CachePos:i;

		string strRegKey, strPluginName, strPluginID, strCurPluginID;

		strRegKey.Format (FmtPluginsCache_PluginD, Pos);

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


void PluginA::ClearExports()
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
