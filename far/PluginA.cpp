/*
Copyright (c) 1996 Eugene Roshal
Copyright (c) 2000 Far Group
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "headers.hpp"
#pragma hdrstop

#include "plugins.hpp"
#include "lang.hpp"
#include "keys.hpp"
#include "codepage.hpp"
#include "flink.hpp"
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
#include "plclass.hpp"
#include "PluginA.hpp"
#include "registry.hpp"
#include "localOEM.hpp"
#include "plugapi.hpp"
#include "keyboard.hpp"
#include "message.hpp"
#include "interf.hpp"
#include "clipboard.hpp"
#include "xlat.hpp"
#include "fileowner.hpp"
#include "pathmix.hpp"
#include "dirmix.hpp"
#include "processname.hpp"
#include "mix.hpp"
#include "lasterror.hpp"

#include "wrap.cpp"

static const wchar_t *wszReg_Preload=L"Preload";
static const wchar_t *wszReg_SysID=L"SysID";

static const wchar_t wszReg_OpenPlugin[]=L"OpenPlugin";
static const wchar_t wszReg_OpenFilePlugin[]=L"OpenFilePlugin";
static const wchar_t wszReg_SetFindList[]=L"SetFindList";
static const wchar_t wszReg_ProcessEditorInput[]=L"ProcessEditorInput";
static const wchar_t wszReg_ProcessEditorEvent[]=L"ProcessEditorEvent";
static const wchar_t wszReg_ProcessViewerEvent[]=L"ProcessViewerEvent";
static const wchar_t wszReg_ProcessDialogEvent[]=L"ProcessDialogEvent";
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
static const char NFMP_ProcessDialogEvent[]="ProcessDialogEvent";
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
	return FarChDir(strModulePath);
}

static void CheckScreenLock()
{
	if (ScrBuf.GetLockCount() > 0 && !CtrlObject->Macro.PeekKey())
	{
		ScrBuf.SetLockCount(0);
		ScrBuf.Flush();
	}
}



PluginA::PluginA(PluginManager *owner, const wchar_t *lpwszModuleName):
	m_owner(owner),
	m_strModuleName(lpwszModuleName),
	m_strCacheName(lpwszModuleName),
	m_hModule(nullptr),
	RootKey(nullptr)
	//more initialization here!!!
{
	wchar_t *p = m_strCacheName.GetBuffer();
	while (*p)
	{
		if (*p == L'\\')
			*p = L'/';

		p++;
	}

	m_strCacheName.ReleaseBuffer();
	ClearExports();
	memset(&PI,0,sizeof(PI));
	memset(&OPI,0,sizeof(OPI));
}

PluginA::~PluginA()
{
	if (RootKey) xf_free(RootKey);

	FreePluginInfo();
	FreeOpenPluginInfo();
	Lang.Close();
}


bool PluginA::LoadFromCache(const FAR_FIND_DATA_EX &FindData)
{
	string strRegKey;
	strRegKey.Format(FmtPluginsCache_PluginS, m_strCacheName.CPtr());

	if (CheckRegKey(strRegKey))
	{
		if (GetRegKey(strRegKey,wszReg_Preload,0) == 1)   //PF_PRELOAD plugin, skip cache
			return Load();

		{
			string strPluginID, strCurPluginID;
			strCurPluginID.Format(
			    L"%I64x%x%x",
			    FindData.nFileSize,
			    FindData.ftCreationTime.dwLowDateTime,
			    FindData.ftLastWriteTime.dwLowDateTime
			);
			GetRegKey(strRegKey, L"ID", strPluginID, L"");

			if (StrCmp(strPluginID, strCurPluginID) != 0)   //одинаковые ли бинарники?
				return false;
		}
		strRegKey += L"\\Exports";
		SysID=GetRegKey(strRegKey,wszReg_SysID,0);
		pOpenPlugin=(PLUGINOPENPLUGIN)(INT_PTR)GetRegKey(strRegKey,wszReg_OpenPlugin,0);
		pOpenFilePlugin=(PLUGINOPENFILEPLUGIN)(INT_PTR)GetRegKey(strRegKey,wszReg_OpenFilePlugin,0);
		pSetFindList=(PLUGINSETFINDLIST)(INT_PTR)GetRegKey(strRegKey,wszReg_SetFindList,0);
		pProcessEditorInput=(PLUGINPROCESSEDITORINPUT)(INT_PTR)GetRegKey(strRegKey,wszReg_ProcessEditorInput,0);
		pProcessEditorEvent=(PLUGINPROCESSEDITOREVENT)(INT_PTR)GetRegKey(strRegKey,wszReg_ProcessEditorEvent,0);
		pProcessViewerEvent=(PLUGINPROCESSVIEWEREVENT)(INT_PTR)GetRegKey(strRegKey,wszReg_ProcessViewerEvent,0);
		pProcessDialogEvent=(PLUGINPROCESSDIALOGEVENT)(INT_PTR)GetRegKey(strRegKey,wszReg_ProcessDialogEvent,0);
		pConfigure=(PLUGINCONFIGURE)(INT_PTR)GetRegKey(strRegKey,wszReg_Configure,0);
		WorkFlags.Set(PIWF_CACHED); //too much "cached" flags
		return true;
	}

	return false;
}

bool PluginA::SaveToCache()
{
	if (pGetPluginInfo ||
	        pOpenPlugin ||
	        pOpenFilePlugin ||
	        pSetFindList ||
	        pProcessEditorInput ||
	        pProcessEditorEvent ||
	        pProcessViewerEvent ||
	        pProcessDialogEvent)
	{
		PluginInfo Info;
		GetPluginInfo(&Info);
		SysID = Info.SysID; //LAME!!!
		string strRegKey;
		strRegKey.Format(FmtPluginsCache_PluginS, m_strCacheName.CPtr());
		DeleteKeyTree(strRegKey);
		{
			bool bPreload = (Info.Flags & PF_PRELOAD);
			SetRegKey(strRegKey, wszReg_Preload, bPreload?1:0);
			WorkFlags.Change(PIWF_PRELOADED, bPreload);

			if (bPreload)
				return true;
		}
		{
			string strCurPluginID;
			FAR_FIND_DATA_EX fdata;
			apiGetFindDataEx(m_strModuleName, fdata);
			strCurPluginID.Format(
			    L"%I64x%x%x",
			    fdata.nFileSize,
			    fdata.ftCreationTime.dwLowDateTime,
			    fdata.ftLastWriteTime.dwLowDateTime
			);
			SetRegKey(strRegKey, L"ID", strCurPluginID);
		}

		for (int i = 0; i < Info.DiskMenuStringsNumber; i++)
		{
			string strValue;
			strValue.Format(FmtDiskMenuStringD, i);
			SetRegKey(strRegKey, strValue, Info.DiskMenuStrings[i]);

			if (Info.DiskMenuNumbers)
			{
				strValue.Format(FmtDiskMenuNumberD, i);
				SetRegKey(strRegKey, strValue, Info.DiskMenuNumbers[i]);
			}
		}

		for (int i = 0; i < Info.PluginMenuStringsNumber; i++)
		{
			string strValue;
			strValue.Format(FmtPluginMenuStringD, i);
			SetRegKey(strRegKey, strValue, Info.PluginMenuStrings[i]);
		}

		for (int i = 0; i < Info.PluginConfigStringsNumber; i++)
		{
			string strValue;
			strValue.Format(FmtPluginConfigStringD, i);
			SetRegKey(strRegKey,strValue,Info.PluginConfigStrings[i]);
		}

		SetRegKey(strRegKey, L"CommandPrefix", NullToEmpty(Info.CommandPrefix));
		SetRegKey(strRegKey, L"Flags", Info.Flags);
		strRegKey += L"\\Exports";
		SetRegKey(strRegKey, wszReg_SysID, SysID);
		SetRegKey(strRegKey, wszReg_OpenPlugin, pOpenPlugin!=nullptr);
		SetRegKey(strRegKey, wszReg_OpenFilePlugin, pOpenFilePlugin!=nullptr);
		SetRegKey(strRegKey, wszReg_SetFindList, pSetFindList!=nullptr);
		SetRegKey(strRegKey, wszReg_ProcessEditorInput, pProcessEditorInput!=nullptr);
		SetRegKey(strRegKey, wszReg_ProcessEditorEvent, pProcessEditorEvent!=nullptr);
		SetRegKey(strRegKey, wszReg_ProcessViewerEvent, pProcessViewerEvent!=nullptr);
		SetRegKey(strRegKey, wszReg_ProcessDialogEvent, pProcessDialogEvent!=nullptr);
		SetRegKey(strRegKey, wszReg_Configure, pConfigure!=nullptr);
		return true;
	}

	return false;
}

bool PluginA::Load()
{
	if (WorkFlags.Check(PIWF_DONTLOADAGAIN))
		return false;

	if (m_hModule)
		return true;

	if (!m_hModule)
	{
		string strCurPath, strCurPlugDiskPath;
		wchar_t Drive[]={0,L' ',L':',0}; // ставим 0, как признак того, что вертать обратно ненадо!
		apiGetCurrentDirectory(strCurPath);

		if (IsLocalPath(m_strModuleName))  // если указан локальный путь, то...
		{
			Drive[0] = L'=';
			Drive[1] = m_strModuleName.At(0);
			apiGetEnvironmentVariable(Drive,strCurPlugDiskPath);
		}

		PrepareModulePath(m_strModuleName);
		m_hModule = LoadLibraryEx(m_strModuleName,nullptr,LOAD_WITH_ALTERED_SEARCH_PATH);
		GuardLastError Err;
		FarChDir(strCurPath);

		if (Drive[0]) // вернем ее (переменную окружения) обратно
			SetEnvironmentVariable(Drive,strCurPlugDiskPath);
	}

	if (!m_hModule)
	{
		if (!Opt.LoadPlug.SilentLoadPlugin) //убрать в PluginSet
		{
			SetMessageHelp(L"ErrLoadPlugin");
			Message(MSG_WARNING|MSG_ERRORTYPE,1,MSG(MError),MSG(MPlgLoadPluginError),m_strModuleName,MSG(MOk));
		}

		//чтоб не пытаться загрузить опять а то ошибка будет постоянно показываться.
		WorkFlags.Set(PIWF_DONTLOADAGAIN);

		return false;
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
	pProcessDialogEvent=(PLUGINPROCESSDIALOGEVENT)GetProcAddress(m_hModule,NFMP_ProcessDialogEvent);
	pMinFarVersion=(PLUGINMINFARVERSION)GetProcAddress(m_hModule,NFMP_GetMinFarVersion);
	bool bUnloaded = false;

	if (!CheckMinFarVersion(bUnloaded) || !SetStartupInfo(bUnloaded))
	{
		if (!bUnloaded)
			Unload();

		return false;
	}

	FuncFlags.Set(PICFF_LOADED);
	SaveToCache();
	return true;
}

static void CreatePluginStartupInfoA(PluginA *pPlugin, oldfar::PluginStartupInfo *PSI, oldfar::FarStandardFunctions *FSF)
{
	static oldfar::PluginStartupInfo StartupInfo={0};
	static oldfar::FarStandardFunctions StandardFunctions={0};

	// заполняем структуру StandardFunctions один раз!!!
	if (!StandardFunctions.StructSize)
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
		StandardFunctions.FarKeyToName=FarKeyToNameA;
		StandardFunctions.FarNameToKey=KeyNameToKeyA;
		StandardFunctions.FarInputRecordToKey=InputRecordToKey;
		StandardFunctions.XLat=XlatA;
		StandardFunctions.GetFileOwner=GetFileOwnerA;
		StandardFunctions.GetNumberOfLinks=GetNumberOfLinksA;
		StandardFunctions.FarRecursiveSearch=FarRecursiveSearchA;
		StandardFunctions.MkTemp=FarMkTempA;
		StandardFunctions.DeleteBuffer=DeleteBufferA;
		StandardFunctions.ProcessName=ProcessNameA;
		StandardFunctions.MkLink=FarMkLinkA;
		StandardFunctions.ConvertNameToReal=ConvertNameToRealA;
		StandardFunctions.GetReparsePointInfo=FarGetReparsePointInfoA;
		StandardFunctions.ExpandEnvironmentStr=ExpandEnvironmentStrA;
	}

	if (!StartupInfo.StructSize)
	{
		StartupInfo.StructSize=sizeof(StartupInfo);
		StartupInfo.Menu=FarMenuFnA;
		StartupInfo.Dialog=FarDialogFnA;
		StartupInfo.GetMsg=FarGetMsgFnA;
		StartupInfo.Message=FarMessageFnA;
		StartupInfo.Control=FarControlA;
		StartupInfo.SaveScreen=FarSaveScreen;
		StartupInfo.RestoreScreen=FarRestoreScreen;
		StartupInfo.GetDirList=FarGetDirListA;
		StartupInfo.GetPluginDirList=FarGetPluginDirListA;
		StartupInfo.FreeDirList=FarFreeDirListA;
		StartupInfo.Viewer=FarViewerA;
		StartupInfo.Editor=FarEditorA;
		StartupInfo.CmpName=FarCmpNameA;
		StartupInfo.CharTable=FarCharTableA;
		StartupInfo.Text=FarTextA;
		StartupInfo.EditorControl=FarEditorControlA;
		StartupInfo.ViewerControl=FarViewerControlA;
		StartupInfo.ShowHelp=FarShowHelpA;
		StartupInfo.AdvControl=FarAdvControlA;
		StartupInfo.DialogEx=FarDialogExA;
		StartupInfo.SendDlgMessage=FarSendDlgMessageA;
		StartupInfo.DefDlgProc=FarDefDlgProcA;
		StartupInfo.InputBox=FarInputBoxA;
	}

	*PSI=StartupInfo;
	*FSF=StandardFunctions;
	PSI->ModuleNumber=(INT_PTR)pPlugin;
	PSI->FSF=FSF;
	pPlugin->GetModuleName().GetCharString(PSI->ModuleName,sizeof(PSI->ModuleName));
	PSI->RootKey=nullptr;
}

struct ExecuteStruct
{
	int id; //function id
	union
	{
		INT_PTR nResult;
		HANDLE hResult;
		BOOL bResult;
	};

	union
	{
		INT_PTR nDefaultResult;
		HANDLE hDefaultResult;
		BOOL bDefaultResult;
	};

	bool bUnloaded;
};

#define EXECUTE_FUNCTION(function, es) \
	{ \
		SetFileApisToOEM(); \
		es.nResult = 0; \
		es.nDefaultResult = 0; \
		es.bUnloaded = false; \
		if ( Opt.ExceptRules ) \
		{ \
			__try \
			{ \
				function; \
			} \
			__except(xfilter(es.id, GetExceptionInformation(), this, 0)) \
			{ \
				m_owner->UnloadPlugin(this, es.id, true); \
				es.bUnloaded = true; \
				ProcessException=FALSE; \
			} \
		} \
		else \
		{ \
			function; \
		} \
		SetFileApisToANSI(); \
	}


#define EXECUTE_FUNCTION_EX(function, es) \
	{ \
		SetFileApisToOEM(); \
		es.bUnloaded = false; \
		es.nResult = 0; \
		if ( Opt.ExceptRules ) \
		{ \
			__try \
			{ \
				es.nResult = (INT_PTR)function; \
			} \
			__except(xfilter(es.id, GetExceptionInformation(), this, 0)) \
			{ \
				m_owner->UnloadPlugin(this, es.id, true); \
				es.bUnloaded = true; \
				es.nResult = es.nDefaultResult; \
				ProcessException=FALSE; \
			} \
		} \
		else \
		{ \
			es.nResult = (INT_PTR)function; \
		} \
		SetFileApisToANSI(); \
	}


bool PluginA::SetStartupInfo(bool &bUnloaded)
{
	if (pSetStartupInfo && !ProcessException)
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

		if (es.bUnloaded)
		{
			bUnloaded = true;
			return false;
		}
	}

	return true;
}

static void ShowMessageAboutIllegalPluginVersion(const wchar_t* plg,int required)
{
	string strMsg1, strMsg2;
	strMsg1.Format(MSG(MPlgRequired),(WORD)HIBYTE(LOWORD(required)),(WORD)LOBYTE(LOWORD(required)),HIWORD(required));
	strMsg2.Format(MSG(MPlgRequired2),(WORD)HIBYTE(LOWORD(FAR_VERSION)),(WORD)LOBYTE(LOWORD(FAR_VERSION)),HIWORD(FAR_VERSION));
	Message(MSG_WARNING,1,MSG(MError),MSG(MPlgBadVers),plg,strMsg1,strMsg2,MSG(MOk));
}


bool PluginA::CheckMinFarVersion(bool &bUnloaded)
{
	if (pMinFarVersion && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_MINFARVERSION;
		es.nDefaultResult = 0;
		EXECUTE_FUNCTION_EX(pMinFarVersion(), es);

		if (es.bUnloaded)
		{
			bUnloaded = true;
			return false;
		}

		DWORD FVer = (DWORD)es.nResult;

		if (LOWORD(FVer) >  LOWORD(FAR_VERSION) ||
		        (LOWORD(FVer) == LOWORD(FAR_VERSION) &&
		         HIWORD(FVer) >  HIWORD(FAR_VERSION)))
		{
			ShowMessageAboutIllegalPluginVersion(m_strModuleName,FVer);
			return false;
		}
	}

	return true;
}

int PluginA::Unload(bool bExitFAR)
{
	int nResult = TRUE;

	if (bExitFAR)
		ExitFAR();

	if (!WorkFlags.Check(PIWF_CACHED))
	{
		nResult = FreeLibrary(m_hModule);
		ClearExports();
	}

	m_hModule = nullptr;
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

HANDLE PluginA::OpenPlugin(int OpenFrom, INT_PTR Item)
{
	ChangePriority *ChPriority = new ChangePriority(THREAD_PRIORITY_NORMAL);

	CheckScreenLock(); //??

	{
//		string strCurDir;
//		CtrlObject->CmdLine->GetCurDir(strCurDir);
//		FarChDir(strCurDir);
		g_strDirToSet.Clear();
	}

	HANDLE hResult = INVALID_HANDLE_VALUE;

	if (Load() && pOpenPlugin && !ProcessException)
	{
		//CurPluginItem=this; //BUGBUG
		ExecuteStruct es;
		es.id = EXCEPT_OPENPLUGIN;
		es.hDefaultResult = INVALID_HANDLE_VALUE;
		es.hResult = INVALID_HANDLE_VALUE;
		char *ItemA = nullptr;

		if (Item && (OpenFrom == OPEN_COMMANDLINE  || OpenFrom == OPEN_SHORTCUT))
		{
			ItemA = UnicodeToAnsi((const wchar_t *)Item);
			Item = (INT_PTR)ItemA;
		}

		EXECUTE_FUNCTION_EX(pOpenPlugin(OpenFrom,Item), es);

		if (ItemA) xf_free(ItemA);

		hResult = es.hResult;
		//CurPluginItem=nullptr; //BUGBUG
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
							CtrlObject->Cp()->ActivePanel->SetCurDir(g_strDirToSet,TRUE);
		          CtrlObject->Cp()->ActivePanel->Redraw();
		        }
		    } */
	}

	delete ChPriority;

	return hResult;
}

//////////////////////////////////

HANDLE PluginA::OpenFilePlugin(
    const wchar_t *Name,
    const unsigned char *Data,
    int DataSize,
    int OpMode
)
{
	HANDLE hResult = INVALID_HANDLE_VALUE;

	if (Load() && pOpenFilePlugin && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_OPENFILEPLUGIN;
		es.hDefaultResult = INVALID_HANDLE_VALUE;
		char *NameA = nullptr;

		if (Name)
			NameA = UnicodeToAnsi(Name);

		EXECUTE_FUNCTION_EX(pOpenFilePlugin(NameA, Data, DataSize), es);

		if (NameA) xf_free(NameA);

		hResult = es.hResult;
	}

	return hResult;
}


int PluginA::SetFindList(
    HANDLE hPlugin,
    const PluginPanelItem *PanelItem,
    int ItemsNumber
)
{
	BOOL bResult = FALSE;

	if (pSetFindList && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_SETFINDLIST;
		es.bDefaultResult = FALSE;
		oldfar::PluginPanelItem *PanelItemA = nullptr;
		ConvertPanelItemsArrayToAnsi(PanelItem,PanelItemA,ItemsNumber);
		EXECUTE_FUNCTION_EX(pSetFindList(hPlugin, PanelItemA, ItemsNumber), es);
		FreePanelItemA(PanelItemA,ItemsNumber);
		bResult = es.bResult;
	}

	return bResult;
}

int PluginA::ProcessEditorInput(
    const INPUT_RECORD *D
)
{
	BOOL bResult = FALSE;

	if (Load() && pProcessEditorInput && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_PROCESSEDITORINPUT;
		es.bDefaultResult = TRUE; //(TRUE) treat the result as a completed request on exception!
		const INPUT_RECORD *Ptr=D;
		INPUT_RECORD OemRecord;

		if (Ptr->EventType==KEY_EVENT)
		{
			OemRecord=*D;
			CharToOemBuff(&D->Event.KeyEvent.uChar.UnicodeChar,&OemRecord.Event.KeyEvent.uChar.AsciiChar,1);
			Ptr=&OemRecord;
		}

		EXECUTE_FUNCTION_EX(pProcessEditorInput(Ptr), es);
		bResult = es.bResult;
	}

	return bResult;
}

int PluginA::ProcessEditorEvent(
    int Event,
    PVOID Param
)
{
	if (Load() && pProcessEditorEvent && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_PROCESSEDITOREVENT;
		es.nDefaultResult = 0;
		EXECUTE_FUNCTION_EX(pProcessEditorEvent(Event, Param), es);
	}

	return 0; //oops!
}

int PluginA::ProcessViewerEvent(
    int Event,
    void *Param
)
{
	if (Load() && pProcessViewerEvent && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_PROCESSVIEWEREVENT;
		es.nDefaultResult = 0;
		EXECUTE_FUNCTION_EX(pProcessViewerEvent(Event, Param), es);
	}

	return 0; //oops, again!
}

int PluginA::ProcessDialogEvent(
    int Event,
    void *Param
)
{
	BOOL bResult = FALSE;

	if (Load() && pProcessDialogEvent && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_PROCESSDIALOGEVENT;
		es.bDefaultResult = FALSE;
		EXECUTE_FUNCTION_EX(pProcessDialogEvent(Event, Param), es);
		bResult = es.bResult;
	}

	return bResult;
}

int PluginA::GetVirtualFindData(
    HANDLE hPlugin,
    PluginPanelItem **pPanelItem,
    int *pItemsNumber,
    const wchar_t *Path
)
{
	BOOL bResult = FALSE;

	if (pGetVirtualFindData && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_GETVIRTUALFINDDATA;
		es.bDefaultResult = FALSE;
		pVFDPanelItemA = nullptr;
		size_t Size=StrLength(Path)+1;
		LPSTR PathA=new char[Size];
		UnicodeToOEM(Path,PathA,Size);
		EXECUTE_FUNCTION_EX(pGetVirtualFindData(hPlugin, &pVFDPanelItemA, pItemsNumber, PathA), es);
		bResult = es.bResult;
		delete[] PathA;

		if (bResult && *pItemsNumber)
		{
			ConvertPanelItemA(pVFDPanelItemA, pPanelItem, *pItemsNumber);
		}
	}

	return bResult;
}


void PluginA::FreeVirtualFindData(
    HANDLE hPlugin,
    PluginPanelItem *PanelItem,
    int ItemsNumber
)
{
	FreeUnicodePanelItem(PanelItem, ItemsNumber);

	if (pFreeVirtualFindData && !ProcessException && pVFDPanelItemA)
	{
		ExecuteStruct es;
		es.id = EXCEPT_FREEVIRTUALFINDDATA;
		EXECUTE_FUNCTION(pFreeVirtualFindData(hPlugin, pVFDPanelItemA, ItemsNumber), es);
		pVFDPanelItemA = nullptr;
	}
}



int PluginA::GetFiles(
    HANDLE hPlugin,
    PluginPanelItem *PanelItem,
    int ItemsNumber,
    int Move,
    const wchar_t **DestPath,
    int OpMode
)
{
	int nResult = -1;

	if (pGetFiles && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_GETFILES;
		es.nDefaultResult = -1;
		oldfar::PluginPanelItem *PanelItemA = nullptr;
		ConvertPanelItemsArrayToAnsi(PanelItem,PanelItemA,ItemsNumber);
		char DestA[oldfar::NM];
		UnicodeToOEM(*DestPath,DestA,sizeof(DestA));
		EXECUTE_FUNCTION_EX(pGetFiles(hPlugin, PanelItemA, ItemsNumber, Move, DestA, OpMode), es);
		static wchar_t DestW[oldfar::NM];
		OEMToUnicode(DestA,DestW,countof(DestW));
		*DestPath=DestW;
		FreePanelItemA(PanelItemA,ItemsNumber);
		nResult = (int)es.nResult;
	}

	return nResult;
}


int PluginA::PutFiles(
    HANDLE hPlugin,
    PluginPanelItem *PanelItem,
    int ItemsNumber,
    int Move,
    int OpMode
)
{
	int nResult = -1;

	if (pPutFiles && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_PUTFILES;
		es.nDefaultResult = -1;
		oldfar::PluginPanelItem *PanelItemA = nullptr;
		ConvertPanelItemsArrayToAnsi(PanelItem,PanelItemA,ItemsNumber);
		EXECUTE_FUNCTION_EX(pPutFiles(hPlugin, PanelItemA, ItemsNumber, Move, OpMode), es);
		FreePanelItemA(PanelItemA,ItemsNumber);
		nResult = (int)es.nResult;
	}

	return nResult;
}

int PluginA::DeleteFiles(
    HANDLE hPlugin,
    PluginPanelItem *PanelItem,
    int ItemsNumber,
    int OpMode
)
{
	BOOL bResult = FALSE;

	if (pDeleteFiles && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_DELETEFILES;
		es.bDefaultResult = FALSE;
		oldfar::PluginPanelItem *PanelItemA = nullptr;
		ConvertPanelItemsArrayToAnsi(PanelItem,PanelItemA,ItemsNumber);
		EXECUTE_FUNCTION_EX(pDeleteFiles(hPlugin, PanelItemA, ItemsNumber, OpMode), es);
		FreePanelItemA(PanelItemA,ItemsNumber);
		bResult = (int)es.bResult;
	}

	return bResult;
}


int PluginA::MakeDirectory(
    HANDLE hPlugin,
    const wchar_t **Name,
    int OpMode
)
{
	int nResult = -1;

	if (pMakeDirectory && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_MAKEDIRECTORY;
		es.nDefaultResult = -1;
		char NameA[oldfar::NM];
		UnicodeToOEM(*Name,NameA,sizeof(NameA));
		EXECUTE_FUNCTION_EX(pMakeDirectory(hPlugin, NameA, OpMode), es);
		static wchar_t NameW[oldfar::NM];
		OEMToUnicode(NameA,NameW,countof(NameW));
		*Name=NameW;
		nResult = (int)es.nResult;
	}

	return nResult;
}


int PluginA::ProcessHostFile(
    HANDLE hPlugin,
    PluginPanelItem *PanelItem,
    int ItemsNumber,
    int OpMode
)
{
	BOOL bResult = FALSE;

	if (pProcessHostFile && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_PROCESSHOSTFILE;
		es.bDefaultResult = FALSE;
		oldfar::PluginPanelItem *PanelItemA = nullptr;
		ConvertPanelItemsArrayToAnsi(PanelItem,PanelItemA,ItemsNumber);
		EXECUTE_FUNCTION_EX(pProcessHostFile(hPlugin, PanelItemA, ItemsNumber, OpMode), es);
		FreePanelItemA(PanelItemA,ItemsNumber);
		bResult = es.bResult;
	}

	return bResult;
}


int PluginA::ProcessEvent(
    HANDLE hPlugin,
    int Event,
    PVOID Param
)
{
	BOOL bResult = FALSE;

	if (pProcessEvent && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_PROCESSEVENT;
		es.bDefaultResult = FALSE;
		PVOID ParamA = Param;

		if (Param && (Event == FE_COMMAND || Event == FE_CHANGEVIEWMODE))
			ParamA = (PVOID)UnicodeToAnsi((const wchar_t *)Param);

		EXECUTE_FUNCTION_EX(pProcessEvent(hPlugin, Event, ParamA), es);

		if (ParamA && (Event == FE_COMMAND || Event == FE_CHANGEVIEWMODE))
			xf_free(ParamA);

		bResult = es.bResult;
	}

	return bResult;
}


int PluginA::Compare(
    HANDLE hPlugin,
    const PluginPanelItem *Item1,
    const PluginPanelItem *Item2,
    DWORD Mode
)
{
	int nResult = -2;

	if (pCompare && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_COMPARE;
		es.nDefaultResult = -2;
		oldfar::PluginPanelItem *Item1A = nullptr;
		oldfar::PluginPanelItem *Item2A = nullptr;
		ConvertPanelItemsArrayToAnsi(Item1,Item1A,1);
		ConvertPanelItemsArrayToAnsi(Item2,Item2A,1);
		EXECUTE_FUNCTION_EX(pCompare(hPlugin, Item1A, Item2A, Mode), es);
		FreePanelItemA(Item1A,1);
		FreePanelItemA(Item2A,1);
		nResult = (int)es.nResult;
	}

	return nResult;
}


int PluginA::GetFindData(
    HANDLE hPlugin,
    PluginPanelItem **pPanelItem,
    int *pItemsNumber,
    int OpMode
)
{
	BOOL bResult = FALSE;

	if (pGetFindData && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_GETFINDDATA;
		es.bDefaultResult = FALSE;
		pFDPanelItemA = nullptr;
		EXECUTE_FUNCTION_EX(pGetFindData(hPlugin, &pFDPanelItemA, pItemsNumber, OpMode), es);
		bResult = es.bResult;

		if (bResult && *pItemsNumber)
		{
			ConvertPanelItemA(pFDPanelItemA, pPanelItem, *pItemsNumber);
		}
	}

	return bResult;
}


void PluginA::FreeFindData(
    HANDLE hPlugin,
    PluginPanelItem *PanelItem,
    int ItemsNumber
)
{
	FreeUnicodePanelItem(PanelItem, ItemsNumber);

	if (pFreeFindData && !ProcessException && pFDPanelItemA)
	{
		ExecuteStruct es;
		es.id = EXCEPT_FREEFINDDATA;
		EXECUTE_FUNCTION(pFreeFindData(hPlugin, pFDPanelItemA, ItemsNumber), es);
		pFDPanelItemA = nullptr;
	}
}

int PluginA::ProcessKey(
    HANDLE hPlugin,
    int Key,
    unsigned int dwControlState
)
{
	BOOL bResult = FALSE;

	if (pProcessKey && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_PROCESSKEY;
		es.bDefaultResult = TRUE; // do not pass this key to far on exception
		EXECUTE_FUNCTION_EX(pProcessKey(hPlugin, Key, dwControlState), es);
		bResult = es.bResult;
	}

	return bResult;
}


void PluginA::ClosePlugin(
    HANDLE hPlugin
)
{
	if (pClosePlugin && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_CLOSEPLUGIN;
		EXECUTE_FUNCTION(pClosePlugin(hPlugin), es);
	}

	FreeOpenPluginInfo();
	//	m_pManager->m_pCurrentPlugin = (Plugin*)-1;
}


int PluginA::SetDirectory(
    HANDLE hPlugin,
    const wchar_t *Dir,
    int OpMode
)
{
	BOOL bResult = FALSE;

	if (pSetDirectory && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_SETDIRECTORY;
		es.bDefaultResult = FALSE;
		char *DirA = UnicodeToAnsi(Dir);
		EXECUTE_FUNCTION_EX(pSetDirectory(hPlugin, DirA, OpMode), es);

		if (DirA) xf_free(DirA);

		bResult = es.bResult;
	}

	return bResult;
}

void PluginA::FreeOpenPluginInfo()
{
	if (OPI.CurDir)
		xf_free((void *)OPI.CurDir);

	if (OPI.HostFile)
		xf_free((void *)OPI.HostFile);

	if (OPI.Format)
		xf_free((void *)OPI.Format);

	if (OPI.PanelTitle)
		xf_free((void *)OPI.PanelTitle);

	if (OPI.InfoLines && OPI.InfoLinesNumber)
	{
		FreeUnicodeInfoPanelLines((InfoPanelLine*)OPI.InfoLines,OPI.InfoLinesNumber);
	}

	if (OPI.DescrFiles)
	{
		FreeArrayUnicode((wchar_t**)OPI.DescrFiles);
	}

	if (OPI.PanelModesArray)
	{
		FreeUnicodePanelModes((PanelMode*)OPI.PanelModesArray, OPI.PanelModesNumber);
	}

	if (OPI.KeyBar)
	{
		FreeUnicodeKeyBarTitles((KeyBarTitles*)OPI.KeyBar);
		xf_free((void *)OPI.KeyBar);
	}

	if (OPI.ShortcutData)
		xf_free((void *)OPI.ShortcutData);

	memset(&OPI,0,sizeof(OPI));
}

void PluginA::ConvertOpenPluginInfo(oldfar::OpenPluginInfo &Src, OpenPluginInfo *Dest)
{
	FreeOpenPluginInfo();
	OPI.StructSize = sizeof(OPI);
	OPI.Flags = Src.Flags;

	if (Src.CurDir)
		OPI.CurDir = AnsiToUnicode(Src.CurDir);

	if (Src.HostFile)
		OPI.HostFile = AnsiToUnicode(Src.HostFile);

	if (Src.Format)
		OPI.Format = AnsiToUnicode(Src.Format);

	if (Src.PanelTitle)
		OPI.PanelTitle = AnsiToUnicode(Src.PanelTitle);

	if (Src.InfoLines && Src.InfoLinesNumber)
	{
		ConvertInfoPanelLinesA(Src.InfoLines, (InfoPanelLine**)&OPI.InfoLines, Src.InfoLinesNumber);
		OPI.InfoLinesNumber = Src.InfoLinesNumber;
	}

	if (Src.DescrFiles && Src.DescrFilesNumber)
	{
		OPI.DescrFiles = ArrayAnsiToUnicode((char**)Src.DescrFiles, Src.DescrFilesNumber);
		OPI.DescrFilesNumber = Src.DescrFilesNumber;
	}

	if (Src.PanelModesArray && Src.PanelModesNumber)
	{
		ConvertPanelModesA(Src.PanelModesArray, (PanelMode**)&OPI.PanelModesArray, Src.PanelModesNumber);
		OPI.PanelModesNumber	= Src.PanelModesNumber;
		OPI.StartPanelMode		= Src.StartPanelMode;
		OPI.StartSortMode			= Src.StartSortMode;
		OPI.StartSortOrder		= Src.StartSortOrder;
	}

	if (Src.KeyBar)
	{
		OPI.KeyBar=(KeyBarTitles*) xf_malloc(sizeof(KeyBarTitles));
		ConvertKeyBarTitlesA(Src.KeyBar, (KeyBarTitles*)OPI.KeyBar, Src.StructSize>=(int)sizeof(oldfar::OpenPluginInfo));
	}

	if (Src.ShortcutData)
		OPI.ShortcutData = AnsiToUnicode(Src.ShortcutData);

	*Dest=OPI;
}

void PluginA::GetOpenPluginInfo(
    HANDLE hPlugin,
    OpenPluginInfo *pInfo
)
{
//	m_pManager->m_pCurrentPlugin = this;
	pInfo->StructSize = sizeof(OpenPluginInfo);

	if (pGetOpenPluginInfo && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_GETOPENPLUGININFO;
		oldfar::OpenPluginInfo InfoA={0};
		EXECUTE_FUNCTION(pGetOpenPluginInfo(hPlugin, &InfoA), es);
		ConvertOpenPluginInfo(InfoA,pInfo);
	}
}


int PluginA::Configure(
    int MenuItem
)
{
	BOOL bResult = FALSE;

	if (Load() && pConfigure && !ProcessException)
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
			xf_free((void *)PI.DiskMenuStrings[i]);

		xf_free((void *)PI.DiskMenuStrings);
	}

	if (PI.PluginMenuStringsNumber)
	{
		for (int i=0; i<PI.PluginMenuStringsNumber; i++)
			xf_free((void *)PI.PluginMenuStrings[i]);

		xf_free((void *)PI.PluginMenuStrings);
	}

	if (PI.PluginConfigStringsNumber)
	{
		for (int i=0; i<PI.PluginConfigStringsNumber; i++)
			xf_free((void *)PI.PluginConfigStrings[i]);

		xf_free((void *)PI.PluginConfigStrings);
	}

	if (PI.CommandPrefix)
		xf_free((void *)PI.CommandPrefix);

	memset(&PI,0,sizeof(PI));
}

void PluginA::ConvertPluginInfo(oldfar::PluginInfo &Src, PluginInfo *Dest)
{
	FreePluginInfo();
	PI.StructSize = sizeof(PI);
	PI.Flags = Src.Flags;

	if (Src.DiskMenuStringsNumber)
	{
		wchar_t **p = (wchar_t **) xf_malloc(Src.DiskMenuStringsNumber*sizeof(wchar_t*));

		for (int i=0; i<Src.DiskMenuStringsNumber; i++)
			p[i] = AnsiToUnicode(Src.DiskMenuStrings[i]);

		PI.DiskMenuStrings = p;
		PI.DiskMenuNumbers = Src.DiskMenuNumbers;
		PI.DiskMenuStringsNumber = Src.DiskMenuStringsNumber;
	}

	if (Src.PluginMenuStringsNumber)
	{
		wchar_t **p = (wchar_t **) xf_malloc(Src.PluginMenuStringsNumber*sizeof(wchar_t*));

		for (int i=0; i<Src.PluginMenuStringsNumber; i++)
			p[i] = AnsiToUnicode(Src.PluginMenuStrings[i]);

		PI.PluginMenuStrings = p;
		PI.PluginMenuStringsNumber = Src.PluginMenuStringsNumber;
	}

	if (Src.PluginConfigStringsNumber)
	{
		wchar_t **p = (wchar_t **) xf_malloc(Src.PluginConfigStringsNumber*sizeof(wchar_t*));

		for (int i=0; i<Src.PluginConfigStringsNumber; i++)
			p[i] = AnsiToUnicode(Src.PluginConfigStrings[i]);

		PI.PluginConfigStrings = p;
		PI.PluginConfigStringsNumber = Src.PluginConfigStringsNumber;
	}

	if (Src.CommandPrefix)
		PI.CommandPrefix = AnsiToUnicode(Src.CommandPrefix);

	*Dest=PI;
}

bool PluginA::GetPluginInfo(PluginInfo *pi)
{
	memset(pi, 0, sizeof(PluginInfo));

	if (pGetPluginInfo && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_GETPLUGININFO;
		oldfar::PluginInfo InfoA={0};
		EXECUTE_FUNCTION(pGetPluginInfo(&InfoA), es);

		if (!es.bUnloaded)
		{
			ConvertPluginInfo(InfoA, pi);
			return true;
		}
	}

	return false;
}

void PluginA::ExitFAR()
{
	if (pExitFAR && !ProcessException)
	{
		ExecuteStruct es;
		es.id = EXCEPT_EXITFAR;
		EXECUTE_FUNCTION(pExitFAR(), es);
	}
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
	pProcessDialogEvent=0;
	pMinFarVersion=0;
}
