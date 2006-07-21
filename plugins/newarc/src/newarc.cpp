#include "newarc.h"

FARSTANDARDFUNCTIONS FSF;
PluginStartupInfo Info;

Collection <ArchivePlugin*> Plugins;

ViewCollection <ArchivePanel*> Panels;

char *lpCurrentLanguage;

char *pCommandNames[11] = {
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



ArchivePanel *__stdcall OpenFilePlugin (
		const char *lpFileName,
		const unsigned char *pData,
		int nDataSize
		)
{
	ArchivePanel *pPanelResult = (ArchivePanel*)INVALID_HANDLE_VALUE;

	Archive **pArchives = (Archive**)malloc (5*4);
	int nCount = 5;
	int nArchivesCount = 0;

	if ( lpFileName )
	{
		for (int i = 0; i < Plugins.GetCount(); i++)
		{
			Archive *pArchive = Plugins[i]->QueryArchive (
					lpFileName,
					(const char*)pData,
					nDataSize
					);

			if ( pArchive )
			{
				pArchives[nArchivesCount++] = pArchive;

				if ( nArchivesCount == nCount )
				{
					nCount += 5;

					pArchives = (Archive**)realloc (
							pArchives,
							nCount*4
							);
				}
			}
		}

	}
	else
		pPanelResult = new ArchivePanel (NULL, 0);

	if ( nArchivesCount )
		pPanelResult = new ArchivePanel (pArchives, nArchivesCount);
	else
		free (pArchives);

	if ( pPanelResult != INVALID_HANDLE_VALUE )
		Panels.Add (pPanelResult);

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
			Plugins.Add (pPlugin);
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

	Panels.Remove (pPanel);

	delete pPanel;
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

	Plugins.Create (5);

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

	Panels.Create (5);
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
	for (int i = 0; i < Plugins.GetCount(); i++)
		Plugins[i]->Finalize ();

	Plugins.Free ();

	Panels.Free ();

	StrFree (lpCurrentLanguage);
}

void __stdcall GetPluginInfo (
		PluginInfo *pi
		)
{
	static char *PluginConfigStrings[1];

	PluginConfigStrings[0] = "Поддержка архивов (newarc)";

	pi->PluginConfigStrings = (const char *const*)&PluginConfigStrings;
	pi->PluginConfigStringsNumber = 1;

   	char *lpLanguage = StrCreate (260);
	GetEnvironmentVariable ("FARLANG", lpLanguage, 260);

	if ( FSF.LStricmp (lpLanguage, lpCurrentLanguage) )
	{
		for (int i = 0; i < Plugins.GetCount(); i++)
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
	FarDialog *D = new FarDialog (-1, -1, 78, 9);

	D->DoubleBox (3, 1, 74, 7, "Общие настройки");

	D->RadioButton (5, 2, false, "Всегда показывать сообщения архиваторов");
	D->RadioButton (5, 3, false, "Не показывать сообщения архиваторов при редактировании/просмотре");
	D->RadioButton (5, 4, false, "Не показывать сообщения архиваторов");

	D->Separator (5);

	D->Button (-1, 6, "Принять");
	D->DefaultButton ();

	D->Button (-1, 6, "Отмена");

	if ( D->ShowEx () == D->m_nFirstButton )
	{
		//do something
	}

	delete D;
}

#define DECLARE_COMMAND(string, command) \
	{\
		D->Text (5, Y, string); \
		pPlugin->pGetDefaultCommand (nFormat, command, lpCommand); \
		D->Edit (29, Y++, 42, lpCommand); \
	}

void dlgCommandLinesAndParams (
		ArchivePlugin *pPlugin,
		int nFormat
		)
{
	int nHeight = 19;

	int Y = 2;
	char *lpCommand = StrCreate (260);

	FarDialog *D = new FarDialog (-1, -1, 76, nHeight);

	char *lpTitle = StrCreate (260);

	FSF.sprintf (lpTitle, "Параметры архиватора %s", pPlugin->m_ArchivePluginInfo.pFormatInfo[nFormat].lpName);

	D->DoubleBox (3, 1, 72, nHeight-2, lpTitle); //0

	for (int i = 0; i < 11; i++)
		DECLARE_COMMAND (_M(MSG_dlgCLAP_S_EXTRACT+i), i);

	D->Separator (Y++);

	D->Text (5, Y, "Расширение файлов :");
	D->Edit (25, Y, 10, pPlugin->m_ArchivePluginInfo.pFormatInfo[nFormat].lpDefaultExtention);

	D->Text (40, Y, "Маска \"все файлы\" :");
	D->Edit (60, Y++, 10);

	D->Separator (Y++);

	D->Button (-1, Y, "Принять");
	D->DefaultButton ();

	D->Button (-1, Y, "Отмена");
	D->Button (-1, Y++, "Сбросить");

	if ( D->Show () == D->m_nFirstButton )
	{
	//	SaveArchivePluginParams (pPlugin, nFormat);

		HKEY hKey;

		char *lpRegKey = StrCreate (260);

		FSF.sprintf (
				lpRegKey,
				"%s\\newarc\\formats\\%s",
				Info.RootKey,
				pPlugin->m_ArchivePluginInfo.pFormatInfo[nFormat].lpName
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
						D->m_Items[2+i*2].Data
						);
			}

			RegCloseKey (hKey);
		}

		StrFree (lpRegKey);
	}

	StrFree (lpCommand);

	delete D;
}

void mnuCommandLinesAndParams ()
{
	ArchivePlugin *pPlugin;
	int nItem;

	int nCount = 0;

	for (int i = 0; i < Plugins.GetCount(); i++)
		for (int j = 0; j < Plugins[i]->m_ArchivePluginInfo.nFormats; j++)
			nCount++;

	FarMenuItem *pItems = (FarMenuItem*)malloc (
			nCount*sizeof (FarMenuItem)
			);

	nCount = 0;

	for (int i = 0; i < Plugins.GetCount(); i++)
		for (int j = 0; j < Plugins[i]->m_ArchivePluginInfo.nFormats; j++)
			strcpy (pItems[nCount++].Text, Plugins[i]->m_ArchivePluginInfo.pFormatInfo[j].lpName);

	int nResult = Info.Menu (
			Info.ModuleNumber,
			-1,
			-1,
			0,
			FMENU_WRAPMODE,
			"Формат архива",
			NULL,
			NULL,
			NULL,
			NULL,
			(const FarMenuItem*)pItems,
			nCount
			);

	if ( nResult != -1 )
	{
		pPlugin = NULL;
		nItem = 0;

		nCount = 0;

		for (int i = 0; i < Plugins.GetCount(); i++)
			for (int j = 0; j < Plugins[i]->m_ArchivePluginInfo.nFormats; j++)
			{
				if ( nResult == nCount )
				{
					pPlugin = Plugins[i];
					nItem = j;

					goto l_Found;
				}

				nCount++;
			}

l_Found:

		if ( pPlugin )
			dlgCommandLinesAndParams (pPlugin, nItem);
	}

	free (pItems);
}



int __stdcall Configure (
		int nItem
		)
{
	FarMenuItem *pItems = (FarMenuItem*)malloc (
			2*sizeof (FarMenuItem)
			);

	strcpy (pItems[0].Text, "Настройка");
	strcpy (pItems[1].Text, "Командные строки архиваторов");

	int nResult = Info.Menu (
			Info.ModuleNumber,
			-1,
			-1,
			0,
			FMENU_WRAPMODE,
			NULL,
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

/*
BOOL __stdcall DllMain (
		HINSTANCE hinstDLL,
		DWORD fdwReason,
		LPVOID lpvReserved
		)
{
	return true;
}*/
