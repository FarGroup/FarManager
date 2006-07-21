#include "7z.h"
#include <Collections.h>

PluginStartupInfo Info;
FARSTANDARDFUNCTIONS FSF;

Collection <SevenZipModule*> Formats;
ArchiveFormatInfo *pFormatInfo = NULL;

struct ArchiveModuleInformation {

	int nTypes;
	char *pTypeNames;

	int nConfigStringsNumber;

	char **pConfigStrings;
};

int OnInitialize (PluginStartupInfo *pInfo)
{
	Info = *pInfo;
	FSF = *pInfo->FSF;

	Formats.Create (5);

	WIN32_FIND_DATA fdata;
	char *lpMask = StrDuplicate (Info.ModuleName, 260);

	CutToSlash (lpMask);
	strcat (lpMask, "Formats\\*.dll");

	HANDLE hSearch = FindFirstFile (lpMask, &fdata);

	if ( hSearch != INVALID_HANDLE_VALUE )
	{
		do {

			if ( !OptionIsOn (fdata.dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY) )
			{
				SevenZipModule *pModule = new SevenZipModule;

				char *lpModuleName = StrDuplicate (Info.ModuleName, 260);
				CutToSlash(lpModuleName);

				strcat (lpModuleName, "Formats\\");
				strcat (lpModuleName, fdata.cFileName);

				if ( pModule->Initialize (lpModuleName) )
					Formats.Add (pModule);
				else
					delete pModule;

				free (lpModuleName);
			}
		} while ( FindNextFile (hSearch, &fdata) );

		FindClose (hSearch);
	}

	pFormatInfo = NULL;

	return NAERROR_SUCCESS;
}

int OnFinalize ()
{
	Formats.Free ();

	free (pFormatInfo);

	return NAERROR_SUCCESS;
}

extern int FindFormats (const char *lpFileName, Collection <FormatPosition*> &formats);

int __cdecl SortFormats (
    FormatPosition *pos1,
    FormatPosition *pos2,
    void *pParam
    )
{
	if ( pos1->position > pos2->position )
		return 1;

	if ( pos1->position < pos2->position )
		return -1;

	//if ( pos1->position == pos2->position )
	return 0;
}


int OnQueryArchive (QueryArchiveStruct *pQAS)
{
/*	SevenZipModule *pSplitModule = NULL;
	bool bSplit;

   	for (int i = 0; i < Formats.GetCount (); i++)
   	{
   		bSplit = false;
   		SevenZipModule *pModule = Formats[i];

   		if ( pModule->IsSplitModule() )
   		{
   			pSplitModule = pModule;
   			bSplit = true;
		}

   		if ( pModule && !bSplit )
   		{
   			SevenZipArchive *pArchive = new SevenZipArchive (pModule, pQAS->lpFileName);

   			pArchive->m_bIsArchive = false;

   			if ( pArchive->pOpenArchive (0, NULL) )
   				pArchive->pCloseArchive ();

   				if ( pArchive->m_bIsArchive )
   				{
   					ArchiveFormatInfo info;

   					pModule->GetArchiveFormatInfo (&info);

   					MessageBox (0, info.lpName, "asd", MB_OK);
	   				pQAS->hResult = (HANDLE)pArchive;

   					return NAERROR_SUCCESS;
				}

   			delete pArchive;
   		}
   	}

   	if ( pSplitModule )
   	{
		SevenZipArchive *pArchive = new SevenZipArchive (pSplitModule, pQAS->lpFileName);

		pArchive->m_bIsArchive = false;

		if ( pArchive->pOpenArchive (0, NULL) )
			pArchive->pCloseArchive ();

			if ( pArchive->m_bIsArchive )
			{
				pQAS->hResult = (HANDLE)pArchive;

				return NAERROR_SUCCESS;
			}

		delete pArchive;
   	} */


	Collection <FormatPosition*> formats;

	formats.Create (5);

	FindFormats (pQAS->lpFileName, formats);

	formats.Sort ((void *)SortFormats, NULL);

	for (int j = 0; j < formats.GetCount(); j++)
	{
		FormatPosition *pos = formats[j];

		for (int i = 0; i < Formats.GetCount (); i++)
		{
			SevenZipModule *pModule = Formats[i];

			if ( pModule && IsEqualGUID (pModule->m_uid, *pos->puid) )
			{
				SevenZipArchive *pArchive = new SevenZipArchive (pModule, pQAS->lpFileName, false);

				/*ArchiveFormatInfo info;

				pModule->GetArchiveFormatInfo (&info);

				MessageBox (0, info.lpName, "asD", MB_OK);*/

				pQAS->hResult = (HANDLE)pArchive;

				formats.Free ();

				return NAERROR_SUCCESS;
			}
		}
	}

	formats.Free ();

	return NAERROR_INTERNAL;
}

int OnCreateArchive (CreateArchiveStruct *pCAS)
{
	SevenZipModule *pModule = Formats[pCAS->nFormat];

	if ( pModule )
	{
		SevenZipArchive *pArchive = new SevenZipArchive (pModule, pCAS->lpFileName, true);

		pCAS->hResult = (HANDLE)pArchive;

		return NAERROR_SUCCESS;
	}

	return NAERROR_INTERNAL;
}

int OnOpenArchive (OpenArchiveStruct *pOAS)
{
	SevenZipArchive *pArchive = (SevenZipArchive*)pOAS->hArchive;

	pOAS->bResult = pArchive->pOpenArchive (pOAS->nMode, pOAS->pfnCallback);

	return NAERROR_SUCCESS;
}

int OnCloseArchive (CloseArchiveStruct *pCAS)
{
/*	SevenZipArchive *pArchive = (SevenZipArchive*)pCAS->hArchive;

	pArchive->pCloseArchive ();*/

	return NAERROR_SUCCESS;
}

int OnFinalizeArchive (SevenZipArchive *pArchive)
{
	pArchive->pCloseArchive ();
	delete pArchive;

	return NAERROR_SUCCESS;
}

int OnGetArchivePluginInfo (
		ArchivePluginInfo *ai
		)
{
	int nCount = Formats.GetCount ();

	pFormatInfo = (ArchiveFormatInfo*)realloc (pFormatInfo, nCount*sizeof (ArchiveFormatInfo));

	for (int i = 0; i < nCount; i++)
		Formats[i]->GetArchiveFormatInfo (&pFormatInfo[i]);

	ai->nFormats = nCount;
	ai->pFormatInfo = pFormatInfo;

	return NAERROR_SUCCESS;
}

int OnGetArchiveItem (GetArchiveItemStruct *pGAI)
{
	SevenZipArchive *pArchive = (SevenZipArchive *)pGAI->hArchive;

	pGAI->nResult = pArchive->pGetArchiveItem (pGAI->pItem);

	return NAERROR_SUCCESS;
}

int OnGetArchiveFormat (GetArchiveFormatStruct *pGAF)
{
	SevenZipArchive *pArchive = (SevenZipArchive *)pGAF->hArchive;

	SevenZipModule *pModule = pArchive->m_pModule;

	for (int i = 0; i < Formats.GetCount(); i++)
	{
		if ( Formats[i] == pModule )
		{
			pGAF->nFormat = i;
			break;
		}
	}

	return NAERROR_SUCCESS;
}

int OnExtract (ExtractStruct *pES)
{
	SevenZipArchive *pArchive = (SevenZipArchive *)pES->hArchive;

	pES->bResult = pArchive->pExtract (
			pES->pItems,
			pES->nItemsNumber,
			pES->lpDestPath,
			pES->lpCurrentPath
			);

	return NAERROR_SUCCESS;
}


int OnTest (TestStruct *pTS)
{
	SevenZipArchive *pArchive = (SevenZipArchive *)pTS->hArchive;

	pTS->bResult = pArchive->pTest (
			pTS->pItems,
			pTS->nItemsNumber
			);

	return NAERROR_SUCCESS;
}

int OnAdd (AddStruct *pAS)
{
	SevenZipArchive *pArchive = (SevenZipArchive *)pAS->hArchive;

	pAS->bResult = pArchive->pAddFiles (
			pAS->lpSourcePath,
			pAS->lpCurrentPath,
			pAS->pItems,
			pAS->nItemsNumber
			);

	return NAERROR_SUCCESS;
}


int OnGetDefaultCommand (GetDefaultCommandStruct *pGDC)
{
	if ( pGDC->nFormat < Formats.GetCount() )
		pGDC->bResult = GetFormatCommand (Formats[pGDC->nFormat]->m_uid,
										  pGDC->nCommand,
										  pGDC->lpCommand);
	else
		pGDC->bResult = false;

	return NAERROR_SUCCESS;
}

int OnDelete (DeleteStruct *pDS)
{
	SevenZipArchive *pArchive = (SevenZipArchive *)pDS->hArchive;

	pDS->bResult = pArchive->pDelete (
			pDS->pItems,
			pDS->nItemsNumber
			);

	return NAERROR_SUCCESS;
}


int __stdcall PluginEntry (
		int nFunctionID,
		void *pParams
		)
{
	switch ( nFunctionID ) {

	case FID_INITIALIZE:
		return OnInitialize ((PluginStartupInfo*)pParams);

	case FID_FINALIZE:
		return OnFinalize ();

	case FID_QUERYARCHIVE:
		return OnQueryArchive ((QueryArchiveStruct*)pParams);

	case FID_OPENARCHIVE:
		return OnOpenArchive ((OpenArchiveStruct*)pParams);

	case FID_CLOSEARCHIVE:
		return OnCloseArchive ((CloseArchiveStruct*)pParams);

	case FID_FINALIZEARCHIVE:
		return OnFinalizeArchive ((SevenZipArchive *)pParams);

	case FID_GETARCHIVEPLUGININFO:
		return OnGetArchivePluginInfo ((ArchivePluginInfo*)pParams);

	case FID_GETARCHIVEITEM:
		return OnGetArchiveItem ((GetArchiveItemStruct*)pParams);

	case FID_GETARCHIVEFORMAT:
		return OnGetArchiveFormat ((GetArchiveFormatStruct*)pParams);

	case FID_EXTRACT:
		return OnExtract ((ExtractStruct*)pParams);

	case FID_TEST:
		return OnTest ((TestStruct*)pParams);

	case FID_GETDEFAULTCOMMAND:
		return OnGetDefaultCommand ((GetDefaultCommandStruct*)pParams);

	case FID_DELETE:
		return OnDelete ((DeleteStruct*)pParams);

	case FID_ADD:
		return OnAdd ((AddStruct*)pParams);

	case FID_CREATEARCHIVE:
		return OnCreateArchive ((CreateArchiveStruct*)pParams);
	}

	return NAERROR_NOTIMPLEMENTED;
}


#if !defined(__GNUC__)

BOOL __stdcall DllMain (
		HINSTANCE hinstDLL,
		DWORD fdwReason,
		LPVOID lpvReserved
		)
{
	return TRUE;
}

#endif
