#include "newarc.h"


FARSTANDARDFUNCTIONS FSF;
PluginStartupInfo Info;

pointer_array<ArchivePlugin*> Plugins;
pointer_array<ArchivePanel*> Panels;

char *lpCurrentLanguage;
char *lpINIFileName;

const char *pCommandNames[11] = {
	"Extract",
	"ExtractWithoutPath",
	"Test",
	"Delete",
	"ArchiveComment",
	"FileComment",
	"ConvertToSFX",
	"Lock",
	"AddRecoveryRecord",
	"Recover",
	"Add"
};

const char *GUID2STR (const GUID &uid)
{
	static char szGUID[64];
	LPOLESTR string;

	StringFromIID (uid, &string);

	//int length = wcslen (string)+1;
	//char *result = StrCreate (length);

	memset (&szGUID, 0, sizeof (szGUID));

	WideCharToMultiByte (CP_OEMCP, 0, string, -1, (char*)&szGUID, sizeof (szGUID), NULL, NULL);

	CoTaskMemFree (string);

	return (const char*)&szGUID;
}

const GUID& STR2GUID (const char *lpStr)
{
	static GUID uid;

	wchar_t wszGUID[64];

	memset (&wszGUID, 0, sizeof (wszGUID));

	MultiByteToWideChar (CP_OEMCP, 0, lpStr, -1, (wchar_t*)&wszGUID, sizeof (wszGUID)/sizeof(wszGUID[0]));

	IIDFromString ((LPOLESTR)&wszGUID, &uid);

	return uid;
}

ArchivePlugin *GetPluginFromUID (const GUID &uid)
{
	for (int i = 0; i < Plugins.count(); i++)
	{
		ArchivePlugin *pPlugin = Plugins[i];

		for (int j = 0; j < pPlugin->m_ArchivePluginInfo.nFormats; j++)
		{
			if ( uid == pPlugin->m_ArchivePluginInfo.pFormatInfo[j].uid )
				return pPlugin;
		}
	}

	return NULL;
}

ArchivePanel *__stdcall OpenFilePlugin (
		const char *lpFileName,
		const unsigned char *pData,
		int nDataSize
		)
{
	ArchivePanel *pPanelResult = (ArchivePanel*)INVALID_HANDLE_VALUE;

	pointer_array<Archive*> *pArchives = new pointer_array<Archive*>(ARRAY_OPTIONS_DELETE);

	if ( pArchives && lpFileName )
	{
		for (int i = 0; i < Plugins.count(); i++)
		{
			pointer_array<Archive*> *pCurArchives = Plugins[i]->QueryArchive (
					lpFileName,
					(const char*)pData,
					nDataSize
					);


			if ( pCurArchives )
			{
				while ( pCurArchives->count() )
				{
					if ( pArchives->add(pCurArchives->at(0)) )
						pCurArchives->remove(0, false); //remove without delete
					else
						break;
				}

				//to prevent memory leak in case of no memory above
				while ( pCurArchives->count() )
				{
					pCurArchives->at(0)->m_pPlugin->FinalizeArchive (pCurArchives->at(0));
					pCurArchives->remove(0, false); //remove without delete
				}

				delete pCurArchives;
			}
		}

		if ( pArchives->count() )
			pPanelResult = new ArchivePanel (pArchives);
	}
	else if ( !lpFileName )
	{
		pPanelResult = new ArchivePanel (NULL);
	}

	if ( pPanelResult != INVALID_HANDLE_VALUE )
	{
		Panels.add (pPanelResult);
	}
	else if ( pArchives )
	{
		delete pArchives;
	}

	return pPanelResult;
}

int __stdcall LoadAndInitializePlugins (
		const WIN32_FIND_DATA *FData,
		const char *lpFullName,
		void *Param
		)
{
	if ( Info.CmpName ("*.module", lpFullName, TRUE) )
	{
		ArchivePlugin *pPlugin = new ArchivePlugin;

		if ( pPlugin->Initialize (lpFullName, lpCurrentLanguage) )
			Plugins.add (pPlugin);
		else
			delete pPlugin;
	}

	return 1;
}

int __stdcall GetFindData (
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

void __stdcall FreeFindData (
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

void __stdcall ClosePlugin (
		ArchivePanel *pPanel
		)
{
	pPanel->pClosePlugin ();
	Panels.remove (pPanel);
}

void __stdcall GetOpenPluginInfo (
		ArchivePanel *pPanel,
		OpenPluginInfo *pInfo
		)
{
	pPanel->pGetOpenPluginInfo (pInfo);
}

int __stdcall GetFiles (
		ArchivePanel *pPanel,
		PluginPanelItem *PanelItem,
		int ItemsNumber,
		int Move,
		char *DestPath,
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

int __stdcall PutFiles (
		ArchivePanel *pPanel,
		PluginPanelItem *PanelItem,
		int ItemsNumber,
		int Move,
		int OpMode
		)
{
	return pPanel->pPutFiles (
			PanelItem,
			ItemsNumber,
			Move,
			OpMode
			);
}

int __stdcall DeleteFiles (
		ArchivePanel *pPanel,
		PluginPanelItem *PanelItem,
		int ItemsNumber,
		int OpMode
		)
{
	return pPanel->pDeleteFiles (
			PanelItem,
			ItemsNumber,
			OpMode
			);
}


void __stdcall SetStartupInfo (
		PluginStartupInfo *pInfo
		)
{
	Info = *pInfo;
	FSF = *Info.FSF;

	lpCurrentLanguage = StrCreate (260);
	GetEnvironmentVariable("FARLANG", lpCurrentLanguage, 260);

	Plugins.create (ARRAY_OPTIONS_DELETE);
	Panels.create (ARRAY_OPTIONS_DELETE);

	char *lpPluginsPath = StrDuplicate (Info.ModuleName, 260);

	CutToSlash (lpPluginsPath);

	strcat (lpPluginsPath, "plugins");

	FSF.FarRecursiveSearch (
			lpPluginsPath,
			"*.module",
			LoadAndInitializePlugins,
			FRS_RECUR,
			NULL
			);

	StrFree (lpPluginsPath);

	lpINIFileName = StrDuplicate (Info.ModuleName, 260);

	CutToSlash (lpINIFileName);
	strcat (lpINIFileName, "templates.ini");
}

int __stdcall ProcessHostFile(
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

int __stdcall SetDirectory (
		ArchivePanel *pPanel,
		const char *Dir,
		int nOpMode
		)
{
	return pPanel->pSetDirectory (
			Dir,
			nOpMode
			);
}

int __stdcall ProcessKey (
		ArchivePanel *pPanel,
		int nKey,
		dword dwControlState
		)
{
	return pPanel->pProcessKey (
			nKey,
			dwControlState
			);
}

void __stdcall ExitFAR ()
{
	for (int i = 0; i < Plugins.count(); i++)
		Plugins[i]->Finalize ();

	Plugins.free ();
	Panels.free ();

	StrFree (lpCurrentLanguage);
	StrFree (lpINIFileName);
}

void __stdcall GetPluginInfo (
		PluginInfo *pi
		)
{
	static char *PluginConfigStrings[1];

	PluginConfigStrings[0] = _M(MPluginTitle);

	pi->PluginConfigStrings = (const char *const*)&PluginConfigStrings;
	pi->PluginConfigStringsNumber = 1;

   	char *lpLanguage = StrCreate (260);
	GetEnvironmentVariable ("FARLANG", lpLanguage, 260);

	if ( FSF.LStricmp (lpLanguage, lpCurrentLanguage) )
	{
		for (int i = 0; i < Plugins.count(); i++)
			Plugins[i]->ReloadLanguage (lpLanguage);

		lpCurrentLanguage = StrReplace (
				lpCurrentLanguage,
				lpLanguage
				);
	}

	StrFree (lpLanguage);

	pi->StructSize = sizeof (PluginInfo);
}


void dlgConfigure ()
{
	FarDialog D (-1, -1, 78, 9);

	D.DoubleBox (3, 1, 74, 7, _M(MConfigCommonTitle));

	D.RadioButton (5, 2, false, _M(MConfigCommonAlwaysShowOutput));
	D.RadioButton (5, 3, false, _M(MConfigCommonDontShowOutputWhenViewingEditing));
	D.RadioButton (5, 4, false, _M(MConfigCommonNeverShowOutput));

	D.Separator (5);

	D.Button (-1, 6, _M(MSG_cmn_B_OK));
	D.DefaultButton ();

	D.Button (-1, 6, _M(MSG_cmn_B_CANCEL));

	if ( D.ShowEx () == D.FirstButton() )
	{
		//do something
	}
}

#define DECLARE_COMMAND(string, command) \
	{\
		D.Text (5, Y, string); \
		pPlugin->pGetDefaultCommand (uid, command, lpCommand); \
		D.Edit (29, Y++, 42, lpCommand); \
	}

void dlgCommandLinesAndParams (GUID &uid)
{
	ArchivePlugin *pPlugin = GetPluginFromUID(uid);

	int nHeight = 19;

	int Y = 2;
	char *lpCommand = StrCreate (260);

	FarDialog D(-1, -1, 76, nHeight);

	char *lpTitle = StrCreate (260);

	const ArchiveFormatInfo *info = pPlugin->GetArchiveFormatInfo (uid);

	FSF.sprintf (lpTitle, _M(MCommandLinesAndParamsTitleDialog), info->lpName);

	D.DoubleBox (3, 1, 72, nHeight-2, lpTitle); //0

	for (int i = 0; i < 11; i++)
		DECLARE_COMMAND (_M(MSG_dlgCLAP_S_EXTRACT+i), i);

	D.Separator (Y++);

	D.Text (5, Y, _M(MCommandLinesAndParamsFileExtension));
	D.Edit (25, Y, 10, info->lpDefaultExtention);

	D.Text (40, Y, _M(MCommandLinesAndParamsAllFilesMask));
	D.Edit (60, Y++, 10);

	D.Separator (Y++);

	D.Button (-1, Y, _M(MSG_cmn_B_OK));
	D.DefaultButton ();

	D.Button (-1, Y, _M(MSG_cmn_B_CANCEL));
	D.Button (-1, Y++, _M(MReset));

	if ( D.Show () == D.FirstButton() )
	{
	//	SaveArchivePluginParams (pPlugin, nFormat);

		HKEY hKey;

		char *lpRegKey = StrCreate (260);

		FSF.sprintf (
				lpRegKey,
				"%s\\newarc\\formats\\%s",
				Info.RootKey,
				GUID2STR (info->uid)
				);

		if ( RegCreateKeyEx (
				HKEY_CURRENT_USER,
				lpRegKey,
				0,
				NULL,
				0,
				KEY_WRITE,
				NULL,
				&hKey,
				NULL
				) == ERROR_SUCCESS )
		{
			for (int i = 0; i < 11; i++)
			{
				RegSetStringValue (
						hKey,
						pCommandNames[i],
						D[2+i*2].Data
						);
			}

			RegCloseKey (hKey);
		}

		StrFree (lpRegKey);
	}

	StrFree (lpCommand);
}

void mnuCommandLinesAndParams ()
{
	//ArchivePlugin *pPlugin;
	//int nItem;

	int nCount = 0;

	for (int i = 0; i < Plugins.count(); i++)
		for (int j = 0; j < Plugins[i]->m_ArchivePluginInfo.nFormats; j++)
			nCount++;

	FarMenuItemEx *pItems = (FarMenuItemEx*)malloc (
			nCount*sizeof (FarMenuItemEx)
			);

	memset (pItems, 0, nCount*sizeof (FarMenuItemEx));

	nCount = 0;

	for (int i = 0; i < Plugins.count(); i++)
	{
		for (int j = 0; j < Plugins[i]->m_ArchivePluginInfo.nFormats; j++)
		{
			GUID *puid = new GUID;

			ArchiveFormatInfo *info = &Plugins[i]->m_ArchivePluginInfo.pFormatInfo[j];

			*puid = info->uid;

			pItems[nCount].UserData = (DWORD_PTR)puid;
			pItems[nCount].Flags = MIF_USETEXTPTR;
			pItems[nCount].Text.TextPtr = info->lpName;

			nCount++;
		}
	}

	int nResult = Info.Menu (
			Info.ModuleNumber,
			-1,
			-1,
			0,
			FMENU_WRAPMODE|FMENU_USEEXT,
			_M(MCommandLinesAndParamsArchiveFormat),
			NULL,
			NULL,
			NULL,
			NULL,
			(const FarMenuItem*)pItems,
			nCount
			);

	if ( nResult != -1 )
	{
		GUID uid = *(GUID*)pItems[nResult].UserData;
		dlgCommandLinesAndParams (uid);
	}

	for (int i = 0; i < nCount; i++)
		delete (GUID*)pItems[i].UserData;

	free (pItems);
}



int __stdcall Configure (
		int nItem
		)
{
	FarMenuItem *pItems = (FarMenuItem*)malloc ( 2*sizeof (FarMenuItem) );

	memset (pItems, 0, 2*sizeof (FarMenuItem));

	strcpy (pItems[0].Text, _M(MConfigCommonTitle));
	strcpy (pItems[1].Text, _M(MCommandLinesAndParamsTitleMenu));

	int nResult = Info.Menu (
			Info.ModuleNumber,
			-1,
			-1,
			0,
			FMENU_WRAPMODE,
			_M(MPluginTitle),
			NULL,
			NULL,
			NULL,
			NULL,
			(const FarMenuItem*)pItems,
			2
			);

	if ( nResult == 0 )
		dlgConfigure ();

	if ( nResult == 1 )
		mnuCommandLinesAndParams ();

	free (pItems);

	return FALSE;
}


BOOL __stdcall DllMain (
		HINSTANCE hinstDLL,
		DWORD fdwReason,
		LPVOID lpvReserved
		)
{
	return true;
}
