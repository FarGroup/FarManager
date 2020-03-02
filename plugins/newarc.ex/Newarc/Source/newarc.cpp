#include "newarc.h"

FARSTANDARDFUNCTIONS FSF;
PluginStartupInfo Info;

ArchiveModuleManager* pManager;

const TCHAR* pCommandNames[MAX_COMMANDS] = {
		_T("Extract"),
		_T("ExtractWithoutPath"),
		_T("Test"),
		_T("Delete"),
		_T("ArchiveComment"),
		_T("FileComment"),
		_T("ConvertToSFX"),
		_T("Lock"),
		_T("AddRecoveryRecord"),
		_T("Recover"),
		_T("Add")
		};



const TCHAR *GUID2STR (const GUID &uid)
{
	static TCHAR szGUID[64];
	LPOLESTR str;

	StringFromIID (uid, &str);

	//int length = wcslen (string)+1;
	//TCHAR *result = StrCreate (length);

	memset (&szGUID, 0, sizeof (szGUID));

#ifdef UNICODE
	wcscpy((TCHAR*)&szGUID, str);
#else
	WideCharToMultiByte (CP_OEMCP, 0, str, -1, (TCHAR*)&szGUID, sizeof (szGUID), NULL, NULL);
#endif

	CoTaskMemFree (str);

	return (const TCHAR*)&szGUID;
}

const GUID& STR2GUID (const TCHAR *lpStr)
{
	static GUID uid;

	wchar_t wszGUID[64];

	memset (&wszGUID, 0, sizeof (wszGUID));

#ifdef UNICODE
	wcscpy((TCHAR*)&wszGUID, lpStr);
#else
	MultiByteToWideChar (CP_OEMCP, 0, lpStr, -1, (wchar_t*)&wszGUID, sizeof (wszGUID)/sizeof(wszGUID[0]));
#endif

	IIDFromString ((LPOLESTR)&wszGUID, &uid);

	return uid;
}

HANDLE __stdcall EXP_NAME(OpenPlugin) (
		int OpenFrom,
		INT_PTR Item
		)
{
	if ( pManager->LoadIfNeeded() )
	{
		if ( OpenFrom == OPEN_SHORTCUT )
		{
			/* не работает
			 
			const TCHAR* lpShortcutData = (const TCHAR*)Item;
			TCHAR szGUID[64];

			GUID uidFormat;
			GUID uidPlugin;
			GUID uidModule;

			memcpy(szGUID, lpShortcutData, sizeof(szGUID));
			uidFormat = STR2GUID(szGUID);

			memcpy(szGUID, &lpShortcutData[64], sizeof(szGUID));
			uidPlugin = STR2GUID(szGUID);

			memcpy(szGUID, &lpShortcutData[128], sizeof(szGUID));
			uidModule = STR2GUID(szGUID);

			ArchiveFormat* pFormat = pManager->GetFormat(uidFormat, uidPlugin, uidModule);

			if ( pFormat )
			{
				__debug(_T("%s"), pFormat->GetName());
			}*/

			return INVALID_HANDLE_VALUE;
		}
	}

	return INVALID_HANDLE_VALUE;
}

HANDLE __stdcall EXP_NAME(OpenFilePlugin) (
		const TCHAR *lpFileName,
		const unsigned char *pData,
		int nDataSize
#ifdef UNICODE
		,
		int nOpMode
#endif
		)
{
	if ( pManager->LoadIfNeeded() )
	{
		ArchivePanel *pPanel = new ArchivePanel(pManager, lpFileName);
			
		if ( pPanel )
		{
			if ( !lpFileName )
				return pPanel;
			else
			{
				Array<ArchiveFormat*>& formats = pPanel->GetFormats();

				pManager->QueryArchives(lpFileName, pData, nDataSize, formats);

				if ( formats.count() )
					return pPanel;
			}

			delete pPanel;
		}
	}

	return INVALID_HANDLE_VALUE;
}


int __stdcall EXP_NAME(GetFindData) (
		ArchivePanel *pPanel,
		PluginPanelItem **pPanelItem,
		int *pItemsNumber,
		int OpMode
		)
{
	return pPanel->pGetFindData (
			pPanelItem,
			pItemsNumber,
			OpMode
			);
}

void __stdcall EXP_NAME(FreeFindData) (
		ArchivePanel *pPanel,
		PluginPanelItem *pPanelItem,
		int nItemsNumber
		)
{
	pPanel->pFreeFindData (
			pPanelItem,
			nItemsNumber
			);
}

void __stdcall EXP_NAME(ClosePlugin) (
		ArchivePanel *pPanel
		)
{
	pPanel->pClosePlugin ();
	delete pPanel;
}

void __stdcall EXP_NAME(GetOpenPluginInfo) (
		ArchivePanel *pPanel,
		OpenPluginInfo *pInfo
		)
{
	pPanel->pGetOpenPluginInfo (pInfo);
}

int __stdcall EXP_NAME(GetFiles) (
		ArchivePanel *pPanel,
		PluginPanelItem *PanelItem,
		int ItemsNumber,
		int Move,
		TCHAR *DestPath,
		int OpMode
		)
{
	return pPanel->pGetFiles (
			PanelItem,
			ItemsNumber,
			Move,
			DestPath,
			OpMode
			);
}

int __stdcall EXP_NAME(PutFiles) (
		ArchivePanel *pPanel,
		PluginPanelItem *PanelItem,
		int ItemsNumber,
		int Move,
#ifdef UNICODE
		const wchar_t *SrcPath,
#endif
		int OpMode
		)
{
	return pPanel->pPutFiles(
			PanelItem,
			ItemsNumber,
			Move,
#ifdef UNICODE
			SrcPath,
#endif
			OpMode
			);
}

int __stdcall EXP_NAME(DeleteFiles) (
		ArchivePanel *pPanel,
		PluginPanelItem *PanelItem,
		int ItemsNumber,
		int OpMode
		)
{
	return pPanel->pDeleteFiles(
			PanelItem,
			ItemsNumber,
			OpMode
			);
}


void __stdcall EXP_NAME(SetStartupInfo) (
		PluginStartupInfo *pInfo
		)
{
	Info = *pInfo;
	FSF = *Info.FSF;

	string strLanguage;
	apiGetEnvironmentVariable(_T("FARLANG"), strLanguage);

	pManager = new ArchiveModuleManager(strLanguage);

#ifdef UNICODE
	pManager->LoadIfNeeded(); //теперь вроде можно так
#endif
	//cfg.uArchiverOutput = ARCHIVER_OUTPUT_SHOW_ALWAYS;
}

int __stdcall EXP_NAME(ProcessHostFile) (
		ArchivePanel *pPanel,
		PluginPanelItem *PanelItem,
		int ItemsNumber,
		int OpMode
		)
{
	return pPanel->pProcessHostFile (
			PanelItem,
			ItemsNumber,
			OpMode
			);
}

int __stdcall EXP_NAME(SetDirectory)(
		ArchivePanel *pPanel,
		const TCHAR *Dir,
		int nOpMode
		)
{
	return pPanel->pSetDirectory(
			Dir,
			nOpMode
			);
}

int __stdcall EXP_NAME(ProcessKey)(
		ArchivePanel *pPanel,
		int nKey,
		DWORD dwControlState
		)
{
	return pPanel->pProcessKey(
			nKey,
			dwControlState
			);
}

int __stdcall EXP_NAME(MakeDirectory) (
		ArchivePanel* pPanel,
		const TCHAR* lpDirectory,
		int nOpMode
		)
{
	return pPanel->pMakeDirectory(lpDirectory, nOpMode);
}


void __stdcall EXP_NAME(ExitFAR) ()
{
	delete pManager;
}

void __stdcall EXP_NAME(GetPluginInfo) (
		PluginInfo *pi
		)
{
	static TCHAR *PluginConfigStrings[1];

	PluginConfigStrings[0] = _M(MPluginTitle);

	pi->PluginConfigStrings = (const TCHAR *const*)&PluginConfigStrings;
	pi->PluginConfigStringsNumber = 1;

	string strLanguage;
	apiGetEnvironmentVariable(_T("FARLANG"), strLanguage);

	pManager->SetCurrentLanguage(strLanguage);

	pi->StructSize = sizeof(PluginInfo);
}

#include "dlg/dlgConfigure.cpp"
#include "dlg/dlgCommandLinesAndParams.cpp"
#include "mnu/mnuCommandLinesAndParams.cpp"
#include "mnu/mnuConfigSelect.cpp"

int __stdcall EXP_NAME(Configure)(
		int nItem
		)
{
	pManager->LoadIfNeeded();
	mnuConfigSelect(pManager->GetConfig());
	return FALSE;
}

int __stdcall EXP_NAME(GetMinFarVersion)()
{
#ifdef _UNICODE
// PCTL_FORCEDLOADPLUGIN
	return MAKEFARVERSION(2,0,1807);
#else
	return FARMANAGERVERSION;
#endif
}


BOOL __stdcall DllMain(
		HINSTANCE hinstDLL,
		DWORD fdwReason,
		LPVOID lpvReserved
		)
{
	return true;
}
