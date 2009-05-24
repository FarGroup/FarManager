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
#include "plugapi.hpp"
#include "lang.hpp"
#include "keys.hpp"
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
#include "BlockExtKey.hpp"
#include "plclass.hpp"
#include "PluginW.hpp"
#include "registry.hpp"
#include "keyboard.hpp"
#include "message.hpp"
#include "clipboard.hpp"
#include "xlat.hpp"
#include "pathmix.hpp"
#include "dirmix.hpp"
#include "strmix.hpp"
#include "processname.hpp"
#include "mix.hpp"

static const wchar_t *wszReg_Preload=L"Preload";
static const wchar_t *wszReg_SysID=L"SysID";

static const wchar_t wszReg_OpenPlugin[]=L"OpenPluginW";
static const wchar_t wszReg_OpenFilePlugin[]=L"OpenFilePluginW";
static const wchar_t wszReg_SetFindList[]=L"SetFindListW";
static const wchar_t wszReg_ProcessEditorInput[]=L"ProcessEditorInputW";
static const wchar_t wszReg_ProcessEditorEvent[]=L"ProcessEditorEventW";
static const wchar_t wszReg_ProcessViewerEvent[]=L"ProcessViewerEventW";
static const wchar_t wszReg_ProcessDialogEvent[]=L"ProcessDialogEventW";
static const wchar_t wszReg_SetStartupInfo[]=L"SetStartupInfoW";
static const wchar_t wszReg_ClosePlugin[]=L"ClosePluginW";
static const wchar_t wszReg_GetPluginInfo[]=L"GetPluginInfoW";
static const wchar_t wszReg_GetOpenPluginInfo[]=L"GetOpenPluginInfoW";
static const wchar_t wszReg_GetFindData[]=L"GetFindDataW";
static const wchar_t wszReg_FreeFindData[]=L"FreeFindDataW";
static const wchar_t wszReg_GetVirtualFindData[]=L"GetVirtualFindDataW";
static const wchar_t wszReg_FreeVirtualFindData[]=L"FreeVirtualFindDataW";
static const wchar_t wszReg_SetDirectory[]=L"SetDirectoryW";
static const wchar_t wszReg_GetFiles[]=L"GetFilesW";
static const wchar_t wszReg_PutFiles[]=L"PutFilesW";
static const wchar_t wszReg_DeleteFiles[]=L"DeleteFilesW";
static const wchar_t wszReg_MakeDirectory[]=L"MakeDirectoryW";
static const wchar_t wszReg_ProcessHostFile[]=L"ProcessHostFileW";
static const wchar_t wszReg_Configure[]=L"ConfigureW";
static const wchar_t wszReg_ExitFAR[]=L"ExitFARW";
static const wchar_t wszReg_ProcessKey[]=L"ProcessKeyW";
static const wchar_t wszReg_ProcessEvent[]=L"ProcessEventW";
static const wchar_t wszReg_Compare[]=L"CompareW";
static const wchar_t wszReg_GetMinFarVersion[]=L"GetMinFarVersionW";


static const char NFMP_OpenPlugin[]="OpenPluginW";
static const char NFMP_OpenFilePlugin[]="OpenFilePluginW";
static const char NFMP_SetFindList[]="SetFindListW";
static const char NFMP_ProcessEditorInput[]="ProcessEditorInputW";
static const char NFMP_ProcessEditorEvent[]="ProcessEditorEventW";
static const char NFMP_ProcessViewerEvent[]="ProcessViewerEventW";
static const char NFMP_ProcessDialogEvent[]="ProcessDialogEventW";
static const char NFMP_SetStartupInfo[]="SetStartupInfoW";
static const char NFMP_ClosePlugin[]="ClosePluginW";
static const char NFMP_GetPluginInfo[]="GetPluginInfoW";
static const char NFMP_GetOpenPluginInfo[]="GetOpenPluginInfoW";
static const char NFMP_GetFindData[]="GetFindDataW";
static const char NFMP_FreeFindData[]="FreeFindDataW";
static const char NFMP_GetVirtualFindData[]="GetVirtualFindDataW";
static const char NFMP_FreeVirtualFindData[]="FreeVirtualFindDataW";
static const char NFMP_SetDirectory[]="SetDirectoryW";
static const char NFMP_GetFiles[]="GetFilesW";
static const char NFMP_PutFiles[]="PutFilesW";
static const char NFMP_DeleteFiles[]="DeleteFilesW";
static const char NFMP_MakeDirectory[]="MakeDirectoryW";
static const char NFMP_ProcessHostFile[]="ProcessHostFileW";
static const char NFMP_Configure[]="ConfigureW";
static const char NFMP_ExitFAR[]="ExitFARW";
static const char NFMP_ProcessKey[]="ProcessKeyW";
static const char NFMP_ProcessEvent[]="ProcessEventW";
static const char NFMP_Compare[]="CompareW";
static const char NFMP_GetMinFarVersion[]="GetMinFarVersionW";


static BOOL PrepareModulePath(const wchar_t *ModuleName)
{
	string strModulePath;
	strModulePath = ModuleName;

	CutToSlash(strModulePath); //??

	return FarChDir(strModulePath);
}

static void CheckScreenLock()
{
	if ( ScrBuf.GetLockCount() > 0 && !CtrlObject->Macro.PeekKey() )
	{
		ScrBuf.SetLockCount(0);
		ScrBuf.Flush();
	}
}

static size_t WINAPI FarKeyToName(int Key,wchar_t *KeyText,size_t Size)
{
  string strKT;
  if (!KeyToText(Key,strKT))
      return 0;
  size_t len = strKT.GetLength();
  if (Size && KeyText) {
    if (Size <= len) len = Size-1;
    wmemcpy(KeyText, (const wchar_t*)strKT, len);
    KeyText[len] = 0;
  } else if(KeyText) *KeyText = 0;
  return (len+1);
}

int WINAPI KeyNameToKeyW(const wchar_t *Name)
{
	string strN(Name);
	return KeyNameToKey(strN);
}

PluginW::PluginW (
		PluginManager *owner,
		const wchar_t *lpwszModuleName
		)
{
	m_hModule = NULL;
	//more initialization here!!!

	m_owner = owner;

	m_strModuleName = lpwszModuleName;
	m_strCacheName = lpwszModuleName;

	wchar_t *p = m_strCacheName.GetBuffer();
	while (*p)
	{
		if (*p == L'\\')
			*p = L'/';
		p++;
	}
	m_strCacheName.ReleaseBuffer();

	ClearExports();
}

PluginW::~PluginW()
{
	Lang.Close();
}


bool PluginW::LoadFromCache (const FAR_FIND_DATA_EX &FindData)
{
	string strRegKey;

	strRegKey.Format (FmtPluginsCache_PluginS, m_strCacheName.CPtr());

	if ( CheckRegKey(strRegKey) )
	{
		if ( GetRegKey(strRegKey,wszReg_Preload,0) == 1 ) //PF_PRELOAD plugin, skip cache
			return Load ();

		{
			string strPluginID, strCurPluginID;

			strCurPluginID.Format (
					L"%I64x%x%x",
					FindData.nFileSize,
					FindData.ftCreationTime.dwLowDateTime,
					FindData.ftLastWriteTime.dwLowDateTime
					);

			GetRegKey(strRegKey, L"ID", strPluginID, L"");

			if ( StrCmp(strPluginID, strCurPluginID) != 0 ) //одинаковые ли бинарники?
				return false;
		}

		strRegKey += L"\\Exports";
		SysID=GetRegKey(strRegKey,wszReg_SysID,0);

		pOpenPluginW=(PLUGINOPENPLUGINW)(INT_PTR)GetRegKey(strRegKey,wszReg_OpenPlugin,0);
		pOpenFilePluginW=(PLUGINOPENFILEPLUGINW)(INT_PTR)GetRegKey(strRegKey,wszReg_OpenFilePlugin,0);
		pSetFindListW=(PLUGINSETFINDLISTW)(INT_PTR)GetRegKey(strRegKey,wszReg_SetFindList,0);
		pProcessEditorInputW=(PLUGINPROCESSEDITORINPUTW)(INT_PTR)GetRegKey(strRegKey,wszReg_ProcessEditorInput,0);
		pProcessEditorEventW=(PLUGINPROCESSEDITOREVENTW)(INT_PTR)GetRegKey(strRegKey,wszReg_ProcessEditorEvent,0);
		pProcessViewerEventW=(PLUGINPROCESSVIEWEREVENTW)(INT_PTR)GetRegKey(strRegKey,wszReg_ProcessViewerEvent,0);
		pProcessDialogEventW=(PLUGINPROCESSDIALOGEVENTW)(INT_PTR)GetRegKey(strRegKey,wszReg_ProcessDialogEvent,0);
		pConfigureW=(PLUGINCONFIGUREW)(INT_PTR)GetRegKey(strRegKey,wszReg_Configure,0);

		WorkFlags.Set(PIWF_CACHED); //too much "cached" flags

		return true;
	}

	return false;
}

bool PluginW::SaveToCache()
{
	if ( pGetPluginInfoW ||
		 pOpenPluginW ||
		 pOpenFilePluginW ||
		 pSetFindListW ||
		 pProcessEditorInputW ||
		 pProcessEditorEventW ||
		 pProcessViewerEventW ||
		 pProcessDialogEventW )
	{
		PluginInfo Info;

		GetPluginInfo(&Info);

		SysID = Info.SysID; //LAME!!!

		string strRegKey;

		strRegKey.Format (FmtPluginsCache_PluginS, m_strCacheName.CPtr());

		DeleteKeyTree(strRegKey);

		{
			bool bPreload = (Info.Flags & PF_PRELOAD);

			SetRegKey(strRegKey, wszReg_Preload, bPreload?1:0);
			WorkFlags.Change(PIWF_PRELOADED, bPreload);

			if ( bPreload )
				return true;
		}

		{
			string strCurPluginID;
			FAR_FIND_DATA_EX fdata;
			apiGetFindDataEx(m_strModuleName, &fdata);

			strCurPluginID.Format (
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

		strRegKey += L"\\Exports";

		SetRegKey(strRegKey, wszReg_SysID, SysID);
		SetRegKey(strRegKey, wszReg_OpenPlugin, pOpenPluginW!=NULL);
		SetRegKey(strRegKey, wszReg_OpenFilePlugin, pOpenFilePluginW!=NULL);
		SetRegKey(strRegKey, wszReg_SetFindList, pSetFindListW!=NULL);
		SetRegKey(strRegKey, wszReg_ProcessEditorInput, pProcessEditorInputW!=NULL);
		SetRegKey(strRegKey, wszReg_ProcessEditorEvent, pProcessEditorEventW!=NULL);
		SetRegKey(strRegKey, wszReg_ProcessViewerEvent, pProcessViewerEventW!=NULL);
		SetRegKey(strRegKey, wszReg_ProcessDialogEvent, pProcessDialogEventW!=NULL);
		SetRegKey(strRegKey, wszReg_Configure, pConfigureW!=NULL);

		return true;
	}

	return false;
}

bool PluginW::Load()
{
	if ( WorkFlags.Check(PIWF_DONTLOADAGAIN) )
		return false;

	if ( m_hModule )
		return true; //BUGBUG

	DWORD LstErr;

	if( !m_hModule )
	{
		string strCurPath, strCurPlugDiskPath;
		wchar_t Drive[4];

		Drive[0]=0; // ставим 0, как признак того, что вертать обратно ненадо!
		apiGetCurrentDirectory(strCurPath);

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

		FarChDir(strCurPath);

		if(Drive[0]) // вернем ее (переменную окружения) обратно
			SetEnvironmentVariableW(Drive,strCurPlugDiskPath);
	}

	if ( !m_hModule )
	{
		if (!Opt.LoadPlug.SilentLoadPlugin) //убрать в PluginSet
		{
			SetMessageHelp(L"ErrLoadPlugin");

			Message(MSG_WARNING,1,MSG(MError),MSG(MPlgLoadPluginError),m_strModuleName,MSG(MOk));
		}

		//WorkFlags.Set(PIWF_DONTLOADAGAIN); //это с чего бы вдруг?

		return false;
	}

	WorkFlags.Clear(PIWF_CACHED);

	pSetStartupInfoW=(PLUGINSETSTARTUPINFOW)GetProcAddress(m_hModule,NFMP_SetStartupInfo);
	pOpenPluginW=(PLUGINOPENPLUGINW)GetProcAddress(m_hModule,NFMP_OpenPlugin);
	pOpenFilePluginW=(PLUGINOPENFILEPLUGINW)GetProcAddress(m_hModule,NFMP_OpenFilePlugin);
	pClosePluginW=(PLUGINCLOSEPLUGINW)GetProcAddress(m_hModule,NFMP_ClosePlugin);
	pGetPluginInfoW=(PLUGINGETPLUGININFOW)GetProcAddress(m_hModule,NFMP_GetPluginInfo);
	pGetOpenPluginInfoW=(PLUGINGETOPENPLUGININFOW)GetProcAddress(m_hModule,NFMP_GetOpenPluginInfo);
	pGetFindDataW=(PLUGINGETFINDDATAW)GetProcAddress(m_hModule,NFMP_GetFindData);
	pFreeFindDataW=(PLUGINFREEFINDDATAW)GetProcAddress(m_hModule,NFMP_FreeFindData);
	pGetVirtualFindDataW=(PLUGINGETVIRTUALFINDDATAW)GetProcAddress(m_hModule,NFMP_GetVirtualFindData);
	pFreeVirtualFindDataW=(PLUGINFREEVIRTUALFINDDATAW)GetProcAddress(m_hModule,NFMP_FreeVirtualFindData);
	pSetDirectoryW=(PLUGINSETDIRECTORYW)GetProcAddress(m_hModule,NFMP_SetDirectory);
	pGetFilesW=(PLUGINGETFILESW)GetProcAddress(m_hModule,NFMP_GetFiles);
	pPutFilesW=(PLUGINPUTFILESW)GetProcAddress(m_hModule,NFMP_PutFiles);
	pDeleteFilesW=(PLUGINDELETEFILESW)GetProcAddress(m_hModule,NFMP_DeleteFiles);
	pMakeDirectoryW=(PLUGINMAKEDIRECTORYW)GetProcAddress(m_hModule,NFMP_MakeDirectory);
	pProcessHostFileW=(PLUGINPROCESSHOSTFILEW)GetProcAddress(m_hModule,NFMP_ProcessHostFile);
	pSetFindListW=(PLUGINSETFINDLISTW)GetProcAddress(m_hModule,NFMP_SetFindList);
	pConfigureW=(PLUGINCONFIGUREW)GetProcAddress(m_hModule,NFMP_Configure);
	pExitFARW=(PLUGINEXITFARW)GetProcAddress(m_hModule,NFMP_ExitFAR);
	pProcessKeyW=(PLUGINPROCESSKEYW)GetProcAddress(m_hModule,NFMP_ProcessKey);
	pProcessEventW=(PLUGINPROCESSEVENTW)GetProcAddress(m_hModule,NFMP_ProcessEvent);
	pCompareW=(PLUGINCOMPAREW)GetProcAddress(m_hModule,NFMP_Compare);
	pProcessEditorInputW=(PLUGINPROCESSEDITORINPUTW)GetProcAddress(m_hModule,NFMP_ProcessEditorInput);
	pProcessEditorEventW=(PLUGINPROCESSEDITOREVENTW)GetProcAddress(m_hModule,NFMP_ProcessEditorEvent);
	pProcessViewerEventW=(PLUGINPROCESSVIEWEREVENTW)GetProcAddress(m_hModule,NFMP_ProcessViewerEvent);
	pProcessDialogEventW=(PLUGINPROCESSDIALOGEVENTW)GetProcAddress(m_hModule,NFMP_ProcessDialogEvent);
	pMinFarVersionW=(PLUGINMINFARVERSIONW)GetProcAddress(m_hModule,NFMP_GetMinFarVersion);

	bool bUnloaded = false;

	if ( !CheckMinFarVersion(bUnloaded) || !SetStartupInfo(bUnloaded) )
	{
		if ( !bUnloaded )
			Unload();

		return false;
	}

	FuncFlags.Set(PICFF_LOADED);

	SaveToCache();

	return true;
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
    StandardFunctions.LIsLower = farIsLower;
    StandardFunctions.LIsUpper = farIsUpper;
    StandardFunctions.LIsAlpha = farIsAlpha;
    StandardFunctions.LIsAlphanum = farIsAlphaNum;
    StandardFunctions.LUpper = farUpper;
    StandardFunctions.LUpperBuf = farUpperBuf;
    StandardFunctions.LLowerBuf = farLowerBuf;
    StandardFunctions.LLower = farLower;
    StandardFunctions.LStrupr = farStrUpper;
    StandardFunctions.LStrlwr = farStrLower;
    StandardFunctions.LStricmp = farStrCmpI;
    StandardFunctions.LStrnicmp = farStrCmpNI;

    StandardFunctions.Unquote=Unquote;
    StandardFunctions.LTrim=RemoveLeadingSpaces;
    StandardFunctions.RTrim=RemoveTrailingSpaces;
    StandardFunctions.Trim=RemoveExternalSpaces;
    StandardFunctions.TruncStr=TruncStr;
    StandardFunctions.TruncPathStr=TruncPathStr;
    StandardFunctions.QuoteSpaceOnly=QuoteSpaceOnly;
    StandardFunctions.PointToName=PointToName;
    StandardFunctions.GetPathRoot=farGetPathRoot;
    StandardFunctions.AddEndSlash=AddEndSlash;
    StandardFunctions.CopyToClipboard=CopyToClipboard;
    StandardFunctions.PasteFromClipboard=PasteFromClipboard;
    StandardFunctions.FarKeyToName=FarKeyToName;
    StandardFunctions.FarNameToKey=KeyNameToKeyW;
    StandardFunctions.FarInputRecordToKey=InputRecordToKey;
    StandardFunctions.XLat=Xlat;
    StandardFunctions.GetFileOwner=farGetFileOwner;
    StandardFunctions.GetNumberOfLinks=GetNumberOfLinks;
    StandardFunctions.FarRecursiveSearch=FarRecursiveSearch;
    StandardFunctions.MkTemp=FarMkTemp;
    StandardFunctions.DeleteBuffer=DeleteBuffer;
    StandardFunctions.ProcessName=ProcessName;
    StandardFunctions.MkLink=FarMkLink;
    StandardFunctions.ConvertNameToReal=farConvertNameToReal;
    StandardFunctions.GetReparsePointInfo=farGetReparsePointInfo;
  }

  if(!StartupInfo.StructSize)
  {
    StartupInfo.StructSize=sizeof(StartupInfo);
    StartupInfo.Menu=FarMenuFn; //BUGBUG
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
    StartupInfo.Text=FarText;
    StartupInfo.EditorControl=FarEditorControl;
    StartupInfo.ViewerControl=FarViewerControl;
    StartupInfo.ShowHelp=FarShowHelp;
    StartupInfo.AdvControl=FarAdvControl;
    StartupInfo.DialogInit=FarDialogInit;
    StartupInfo.DialogRun=FarDialogRun;
    StartupInfo.DialogFree=FarDialogFree;
    StartupInfo.SendDlgMessage=FarSendDlgMessage;
    StartupInfo.DefDlgProc=FarDefDlgProc;
    StartupInfo.InputBox=FarInputBox;
    StartupInfo.PluginsControl=farPluginsControl;
    StartupInfo.FileFilterControl=farFileFilterControl;
  }

  memcpy(PSI,&StartupInfo,sizeof(StartupInfo));
  memcpy(FSF,&StandardFunctions,sizeof(StandardFunctions));

  PSI->FSF=FSF;

  PSI->RootKey=NULL;

  PSI->ModuleNumber=(INT_PTR)pPlugin;
  if (pPlugin)
  {
    PSI->ModuleName = (const wchar_t*)pPlugin->GetModuleName();
  }
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

bool PluginW::SetStartupInfo (bool &bUnloaded)
{
	if ( pSetStartupInfoW && !ProcessException )
	{
		PluginStartupInfo _info;
		FarStandardFunctions _fsf;

		CreatePluginStartupInfo((Plugin *)this, &_info, &_fsf);

		// скорректирем адреса и плагино-зависимые поля
		strRootKey = Opt.strRegRoot;
		strRootKey += L"\\Plugins";

		_info.RootKey = (const wchar_t*)strRootKey;

		ExecuteStruct es;
		es.id = EXCEPT_SETSTARTUPINFO;

		EXECUTE_FUNCTION(pSetStartupInfoW(&_info), es);

		if ( es.bUnloaded )
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
	string strPlgName;

	strMsg1.Format(MSG(MPlgRequired),
					(WORD)HIBYTE(LOWORD(required)),(WORD)LOBYTE(LOWORD(required)),HIWORD(required));
	strMsg2.Format(MSG(MPlgRequired2),
					(WORD)HIBYTE(LOWORD(FAR_VERSION)),(WORD)LOBYTE(LOWORD(FAR_VERSION)),HIWORD(FAR_VERSION));
	Message(MSG_WARNING,1,MSG(MError),MSG(MPlgBadVers),plg,strMsg1,strMsg2,MSG(MOk));
}


bool PluginW::CheckMinFarVersion (bool &bUnloaded)
{
	if ( pMinFarVersionW && !ProcessException )
	{
		ExecuteStruct es;

		es.id = EXCEPT_MINFARVERSION;
		es.nDefaultResult = 0;

		EXECUTE_FUNCTION_EX(pMinFarVersionW(), es);

		if ( es.bUnloaded )
		{
			bUnloaded = true;
			return false;
		}

		DWORD FVer = (DWORD)es.nResult;

		if ( LOWORD(FVer) >  LOWORD(FAR_VERSION) ||
			(LOWORD(FVer) == LOWORD(FAR_VERSION) &&
			HIWORD(FVer) >  HIWORD(FAR_VERSION)) )
		{
			ShowMessageAboutIllegalPluginVersion(m_strModuleName,FVer);
			return false;
		}
	}

	return true;
}

int PluginW::Unload (bool bExitFAR)
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

bool PluginW::IsPanelPlugin()
{
	return pSetFindListW ||
		pGetFindDataW ||
		pGetVirtualFindDataW ||
		pSetDirectoryW ||
		pGetFilesW ||
		pPutFilesW ||
		pDeleteFilesW ||
		pMakeDirectoryW ||
		pProcessHostFileW ||
		pProcessKeyW ||
		pProcessEventW ||
		pCompareW ||
		pGetOpenPluginInfoW ||
		pFreeFindDataW ||
		pFreeVirtualFindDataW ||
		pClosePluginW;
}

HANDLE PluginW::OpenPlugin (int OpenFrom, INT_PTR Item)
{
  //BUGBUG???
  //AY - непонятно нафига нужно, в других вызовах нету,
  //     притом ещё делает варнинги при сборке из за того что внизу есть SEH.
  //     Если это да надо, то надо выносить вызов SEH в отдельную функцию.
  //ChangePriority ChPriority(THREAD_PRIORITY_NORMAL);

  CheckScreenLock(); //??

  {
//		string strCurDir;
//		CtrlObject->CmdLine->GetCurDir(strCurDir);
//		FarChDir(strCurDir);
		g_strDirToSet=L"";
  }


  if ( Load() && pOpenPluginW && !ProcessException )
  {
		//CurPluginItem=this; //BUGBUG

		ExecuteStruct es;

		es.id = EXCEPT_OPENPLUGIN;
		es.hDefaultResult = INVALID_HANDLE_VALUE;
		es.hResult = INVALID_HANDLE_VALUE;

		EXECUTE_FUNCTION_EX(pOpenPluginW(OpenFrom,Item), es);

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

HANDLE PluginW::OpenFilePlugin (
		const wchar_t *Name,
		const unsigned char *Data,
		int DataSize,
		int OpMode
		)
{
//	if ( m_bCached && HAS_EXPORT(EXPORT_OPENFILEPLUGIN) )
//		Load (FORCE_LOAD);
	HANDLE hResult = INVALID_HANDLE_VALUE;

	if ( Load() && pOpenFilePluginW && !ProcessException )
	{
		ExecuteStruct es;

		es.id = EXCEPT_OPENFILEPLUGIN;
		es.hDefaultResult = INVALID_HANDLE_VALUE;

		EXECUTE_FUNCTION_EX(pOpenFilePluginW(Name, Data, DataSize, OpMode), es);

		hResult = es.hResult;
	}

	return hResult;
}


int PluginW::SetFindList (
		HANDLE hPlugin,
		const PluginPanelItem *PanelItem,
		int ItemsNumber
		)
{
	BOOL bResult = FALSE;

	if ( pSetFindListW && !ProcessException )
	{
		ExecuteStruct es;

		es.id = EXCEPT_SETFINDLIST;
		es.bDefaultResult = FALSE;

		EXECUTE_FUNCTION_EX(pSetFindListW(hPlugin, PanelItem, ItemsNumber), es);

		bResult = es.bResult;
	}

	return bResult;
}

int PluginW::ProcessEditorInput (
		const INPUT_RECORD *D
		)
{
	BOOL bResult = FALSE;

	if ( Load() && pProcessEditorInputW && !ProcessException )
	{
		ExecuteStruct es;

		es.id = EXCEPT_PROCESSEDITORINPUT;
		es.bDefaultResult = TRUE; //(TRUE) treat the result as a completed request on exception!

		EXECUTE_FUNCTION_EX(pProcessEditorInputW(D), es);

		bResult = es.bResult;
	}

	return bResult;
}

int PluginW::ProcessEditorEvent (
		int Event,
		PVOID Param
		)
{
	if ( Load() && pProcessEditorEventW && !ProcessException )
	{
		ExecuteStruct es;

		es.id = EXCEPT_PROCESSEDITOREVENT;
		es.nDefaultResult = 0;

		EXECUTE_FUNCTION_EX(pProcessEditorEventW(Event, Param), es);
	}

	return 0; //oops!
}

int PluginW::ProcessViewerEvent (
		int Event,
		void *Param
		)
{
	if ( Load() && pProcessViewerEventW && !ProcessException )
	{
		ExecuteStruct es;

		es.id = EXCEPT_PROCESSVIEWEREVENT;
		es.nDefaultResult = 0;

		EXECUTE_FUNCTION_EX(pProcessViewerEventW(Event, Param), es);
	}

	return 0; //oops, again!
}

int PluginW::ProcessDialogEvent (
		int Event,
		void *Param
		)
{
	BOOL bResult = FALSE;

	if ( Load() && pProcessDialogEventW && !ProcessException )
	{
		ExecuteStruct es;

		es.id = EXCEPT_PROCESSDIALOGEVENT;
		es.bDefaultResult = FALSE;

		EXECUTE_FUNCTION_EX(pProcessDialogEventW(Event, Param), es);
		bResult = es.bResult;
	}

	return bResult;
}

int PluginW::GetVirtualFindData (
		HANDLE hPlugin,
		PluginPanelItem **pPanelItem,
		int *pItemsNumber,
		const wchar_t *Path
		)
{
	BOOL bResult = FALSE;

	if ( pGetVirtualFindDataW && !ProcessException )
	{
		ExecuteStruct es;

		es.id = EXCEPT_GETVIRTUALFINDDATA;
		es.bDefaultResult = FALSE;

		EXECUTE_FUNCTION_EX(pGetVirtualFindDataW(hPlugin, pPanelItem, pItemsNumber, Path), es);

		bResult = es.bResult;
	}

	return bResult;
}


void PluginW::FreeVirtualFindData (
		HANDLE hPlugin,
		PluginPanelItem *PanelItem,
		int ItemsNumber
		)
{
	if ( pFreeVirtualFindDataW && !ProcessException )
	{
		ExecuteStruct es;
		es.id = EXCEPT_FREEVIRTUALFINDDATA;

		EXECUTE_FUNCTION(pFreeVirtualFindDataW(hPlugin, PanelItem, ItemsNumber), es);
	}
}



int PluginW::GetFiles (
		HANDLE hPlugin,
		PluginPanelItem *PanelItem,
		int ItemsNumber,
		int Move,
		const wchar_t **DestPath,
		int OpMode
		)
{
	int nResult = -1;

	if ( pGetFilesW && !ProcessException )
	{
		ExecuteStruct es;

		es.id = EXCEPT_GETFILES;
		es.nDefaultResult = -1;

		EXECUTE_FUNCTION_EX(pGetFilesW(hPlugin, PanelItem, ItemsNumber, Move, DestPath, OpMode), es);

		nResult = (int)es.nResult;
	}

	return nResult;
}


int PluginW::PutFiles (
		HANDLE hPlugin,
		PluginPanelItem *PanelItem,
		int ItemsNumber,
		int Move,
		int OpMode
		)
{
	int nResult = -1;

	if ( pPutFilesW && !ProcessException )
	{
		ExecuteStruct es;

		es.id = EXCEPT_PUTFILES;
		es.nDefaultResult = -1;

		EXECUTE_FUNCTION_EX(pPutFilesW(hPlugin, PanelItem, ItemsNumber, Move, OpMode), es);

		nResult = (int)es.nResult;
	}

	return nResult;
}

int PluginW::DeleteFiles (
		HANDLE hPlugin,
		PluginPanelItem *PanelItem,
		int ItemsNumber,
		int OpMode
		)
{
	BOOL bResult = FALSE;

	if ( pDeleteFilesW && !ProcessException )
	{
		ExecuteStruct es;

		es.id = EXCEPT_DELETEFILES;
		es.bDefaultResult = FALSE;

		EXECUTE_FUNCTION_EX(pDeleteFilesW(hPlugin, PanelItem, ItemsNumber, OpMode), es);

		bResult = (int)es.bResult;
	}

	return bResult;
}


int PluginW::MakeDirectory (
		HANDLE hPlugin,
		const wchar_t **Name,
		int OpMode
		)
{
	int nResult = -1;

	if ( pMakeDirectoryW && !ProcessException )
	{
		ExecuteStruct es;

		es.id = EXCEPT_MAKEDIRECTORY;
		es.nDefaultResult = -1;

		EXECUTE_FUNCTION_EX(pMakeDirectoryW(hPlugin, Name, OpMode), es);

		nResult = (int)es.nResult;
	}

	return nResult;
}


int PluginW::ProcessHostFile (
		HANDLE hPlugin,
		PluginPanelItem *PanelItem,
		int ItemsNumber,
		int OpMode
		)
{
	BOOL bResult = FALSE;

	if ( pProcessHostFileW && !ProcessException )
	{
		ExecuteStruct es;

		es.id = EXCEPT_PROCESSHOSTFILE;
		es.bDefaultResult = FALSE;

		EXECUTE_FUNCTION_EX(pProcessHostFileW(hPlugin, PanelItem, ItemsNumber, OpMode), es);

		bResult = es.bResult;
	}

	return bResult;
}


int PluginW::ProcessEvent (
		HANDLE hPlugin,
		int Event,
		PVOID Param
		)
{
	BOOL bResult = FALSE;

	if ( pProcessEventW && !ProcessException )
	{
		ExecuteStruct es;

		es.id = EXCEPT_PROCESSEVENT;
		es.bDefaultResult = FALSE;

		EXECUTE_FUNCTION_EX(pProcessEventW(hPlugin, Event, Param), es);

		bResult = es.bResult;
	}

	return bResult;
}


int PluginW::Compare (
		HANDLE hPlugin,
		const PluginPanelItem *Item1,
		const PluginPanelItem *Item2,
		DWORD Mode
		)
{
	int nResult = -2;

	if ( pCompareW && !ProcessException )
	{
		ExecuteStruct es;

		es.id = EXCEPT_COMPARE;
		es.nDefaultResult = -2;

		EXECUTE_FUNCTION_EX(pCompareW(hPlugin, Item1, Item2, Mode), es);

		nResult = (int)es.nResult;
	}

	return nResult;
}


int PluginW::GetFindData (
		HANDLE hPlugin,
		PluginPanelItem **pPanelItem,
		int *pItemsNumber,
		int OpMode
		)
{
	BOOL bResult = FALSE;

	if ( pGetFindDataW && !ProcessException )
	{
		ExecuteStruct es;

		es.id = EXCEPT_GETFINDDATA;
		es.bDefaultResult = FALSE;

		EXECUTE_FUNCTION_EX(pGetFindDataW(hPlugin, pPanelItem, pItemsNumber, OpMode), es);

		bResult = es.bResult;
	}

	return bResult;
}


void PluginW::FreeFindData (
		HANDLE hPlugin,
		PluginPanelItem *PanelItem,
		int ItemsNumber
		)
{
	if ( pFreeFindDataW && !ProcessException )
	{
		ExecuteStruct es;
		es.id = EXCEPT_FREEFINDDATA;

		EXECUTE_FUNCTION(pFreeFindDataW(hPlugin, PanelItem, ItemsNumber), es);
	}
}

int PluginW::ProcessKey (
		HANDLE hPlugin,
		int Key,
		unsigned int dwControlState
		)
{
	BOOL bResult = FALSE;

	if ( pProcessKeyW && !ProcessException )
	{
		ExecuteStruct es;

		es.id = EXCEPT_PROCESSKEY;
		es.bDefaultResult = TRUE; // do not pass this key to far on exception

		EXECUTE_FUNCTION_EX(pProcessKeyW(hPlugin, Key, dwControlState), es);

		bResult = es.bResult;
	}

	return bResult;
}


void PluginW::ClosePlugin (
		HANDLE hPlugin
		)
{
	if ( pClosePluginW && !ProcessException )
	{
		ExecuteStruct es;

		es.id = EXCEPT_CLOSEPLUGIN;

		EXECUTE_FUNCTION(pClosePluginW(hPlugin), es);
	}

//	m_pManager->m_pCurrentPlugin = (Plugin*)-1;
}


int PluginW::SetDirectory (
		HANDLE hPlugin,
		const wchar_t *Dir,
		int OpMode
		)
{
	BOOL bResult = FALSE;

	if ( pSetDirectoryW && !ProcessException )
	{
		ExecuteStruct es;

		es.id = EXCEPT_SETDIRECTORY;
		es.bDefaultResult = FALSE;

		EXECUTE_FUNCTION_EX(pSetDirectoryW(hPlugin, Dir, OpMode), es);

		bResult = es.bResult;
	}

	return bResult;
}


void PluginW::GetOpenPluginInfo (
		HANDLE hPlugin,
		OpenPluginInfo *pInfo
		)
{
//	m_pManager->m_pCurrentPlugin = this;

	pInfo->StructSize = sizeof(OpenPluginInfo);

	if ( pGetOpenPluginInfoW && !ProcessException )
	{
		ExecuteStruct es;
		es.id = EXCEPT_GETOPENPLUGININFO;

		EXECUTE_FUNCTION(pGetOpenPluginInfoW(hPlugin, pInfo), es);
	}
}


int PluginW::Configure(
		int MenuItem
		)
{
	BOOL bResult = FALSE;

	if ( Load() && pConfigureW && !ProcessException )
	{
		ExecuteStruct es;

		es.id = EXCEPT_CONFIGURE;
		es.bDefaultResult = FALSE;

		EXECUTE_FUNCTION_EX(pConfigureW(MenuItem), es);

		bResult = es.bResult;
	}

	return bResult;
}


bool PluginW::GetPluginInfo (PluginInfo *pi)
{
	memset (pi, 0, sizeof (PluginInfo));

	if ( pGetPluginInfoW && !ProcessException )
	{
		ExecuteStruct es;

		es.id = EXCEPT_GETPLUGININFO;

		EXECUTE_FUNCTION(pGetPluginInfoW(pi), es);

		if ( !es.bUnloaded )
			return true;
	}

	return false;
}

void PluginW::ExitFAR()
{
	if ( pExitFARW && !ProcessException )
	{
		ExecuteStruct es;

		es.id = EXCEPT_EXITFAR;

		EXECUTE_FUNCTION(pExitFARW(), es);
	}
}

void PluginW::ClearExports()
{
	pSetStartupInfoW=0;
	pOpenPluginW=0;
	pOpenFilePluginW=0;
	pClosePluginW=0;
	pGetPluginInfoW=0;
	pGetOpenPluginInfoW=0;
	pGetFindDataW=0;
	pFreeFindDataW=0;
	pGetVirtualFindDataW=0;
	pFreeVirtualFindDataW=0;
	pSetDirectoryW=0;
	pGetFilesW=0;
	pPutFilesW=0;
	pDeleteFilesW=0;
	pMakeDirectoryW=0;
	pProcessHostFileW=0;
	pSetFindListW=0;
	pConfigureW=0;
	pExitFARW=0;
	pProcessKeyW=0;
	pProcessEventW=0;
	pCompareW=0;
	pProcessEditorInputW=0;
	pProcessEditorEventW=0;
	pProcessViewerEventW=0;
	pProcessDialogEventW=0;
	pMinFarVersionW=0;
}
