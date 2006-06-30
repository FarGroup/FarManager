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

	if ( pos1->position == pos2->position )
		return 0;
}


int OnQueryArchive (QueryArchiveStruct *pQAS)
{
	Collection <FormatPosition*> formats;

	formats.Create (5);

	FindFormats (pQAS->lpFileName, formats);

	formats.Sort (SortFormats, NULL);

	for (int j = 0; j < formats.GetCount(); j++)
	{
		FormatPosition *pos = formats[j];

		for (int i = 0; i < Formats.GetCount (); i++)
		{
			SevenZipModule *pModule = Formats[i];

			if ( pModule && IsEqualGUID (pModule->m_uid, *pos->puid) )
			{
				SevenZipArchive *pArchive = new SevenZipArchive (pModule, pQAS->lpFileName);

				if ( pArchive->pOpenArchive (0, NULL, true) )
				{
					pArchive->pCloseArchive ();

					pQAS->hResult = (HANDLE)pArchive;

					formats.Free ();
					return NAERROR_SUCCESS;
				}

				delete pArchive;
			}
		}
	}

	formats.Free ();

	for (int j = 0; j < 2; j++)
	{
		for (int i = 0; i < Formats.GetCount (); i++)
		{
			SevenZipModule *pModule = Formats[i];

			if ( pModule && !pModule->HasSignature () )
			{
				SevenZipArchive *pArchive = new SevenZipArchive (pModule, pQAS->lpFileName);

				if ( pArchive->pOpenArchive (0, NULL, j > 0) )
				{
					pArchive->pCloseArchive ();

					pQAS->hResult = (HANDLE)pArchive;

					return NAERROR_SUCCESS;
				}

				delete pArchive;
			}
		}
	}

	return NAERROR_INTERNAL;
}

int OnOpenArchive (OpenArchiveStruct *pOAS)
{
	SevenZipArchive *pArchive = (SevenZipArchive*)pOAS->hArchive;

	pOAS->bResult = pArchive->pOpenArchive (pOAS->nMode, pOAS->pfnCallback, true);

	return NAERROR_SUCCESS;
}

int OnCloseArchive (CloseArchiveStruct *pCAS)
{
	SevenZipArchive *pArchive = (SevenZipArchive*)pCAS->hArchive;

	pArchive->pCloseArchive ();

	return NAERROR_SUCCESS;
}

int OnFinalizeArchive (SevenZipArchive *pArchive)
{
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

int OnGetDefaultCommand (GetDefaultCommandStruct *pGDC)
{
    static char *pCommands[]={
    /*Extract               */"rar x {-p%%P} {-ap%%R} -y -c- -kb -- %%A @%%LNM",
    /*Extract without paths */"rar e {-p%%P} -y -c- -kb -- %%A @%%LNM",
    /*Test                  */"rar t -y {-p%%P} -- %%A",
    /*Delete                */"rar d -y {-p%%P} {-w%%W} -- %%A @%%LNM",
    /*Comment archive       */"rar c -y {-w%%W} -- %%A",
    /*Comment files         */"rar cf -y {-w%%W} -- %%A @%%LNM",
    /*Convert to SFX        */"rar s -y -- %%A",
    /*Lock archive          */"rar k -y -- %%A",
    /*Protect archive       */"rar rr -y -- %%A",
    /*Recover archive       */"rar r -y -- %%A",
    /*Add files             */"rar a -y {-p%%P} {-ap%%R} {-w%%W} {%%S} -- %%A @%%LNM"
    };


	if ( pGDC->nFormat == 0 )
	{
		strcpy (pGDC->lpCommand, pCommands[pGDC->nCommand]);
		pGDC->bResult = true;
	}
	else
		pGDC->bResult = false;

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
	}

	return NAERROR_NOTIMPLEMENTED;
}



BOOL __stdcall DllMain (
		HINSTANCE hinstDLL,
		DWORD fdwReason,
		LPVOID lpvReserved
		)
{
	return TRUE;
}

