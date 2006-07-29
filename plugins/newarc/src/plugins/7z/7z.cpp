#include "7z.h"

MY_DEFINE_GUID (CLSID_Format7z, 0x9F48C737, 0xC878, 0x4B3B, 0xB5, 0x31, 0x09, 0x7E, 0x96, 0x50, 0x62, 0xC2);


PluginStartupInfo Info;
FARSTANDARDFUNCTIONS FSF;

SevenZipModule *pModule;

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

	pModule = new SevenZipModule (NULL);

	return NAERROR_SUCCESS;
}

int OnFinalize ()
{
	delete pModule;

	return NAERROR_SUCCESS;
}

int OnQueryArchive (QueryArchiveStruct *pQAS)
{
	if ( pModule->m_hModule )
	{
		SevenZipArchive *pArchive = new SevenZipArchive (pModule, pQAS->lpFileName);

		if ( pArchive->pOpenArchive (0, NULL) )
		{
			pArchive->pCloseArchive ();

			pQAS->hResult = (HANDLE)pArchive;

			return NAERROR_SUCCESS;
		}

		delete pArchive;
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
	static ArchiveFormatInfo formatInfo;

	formatInfo.uid = CLSID_Format7z;
	formatInfo.dwFlags = AFF_SUPPORT_INTERNAL_EXTRACT;
	formatInfo.lpName = "7z Archive";
	formatInfo.lpDefaultExtention = "7z";

	ai->nFormats = 1;
	ai->pFormatInfo = (ArchiveFormatInfo*)&formatInfo;

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

	pGAF->uid = CLSID_Format7z;

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


	if ( pGDC->uid == CLSID_Format7z )
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


#if defined(__GNUC__)
#ifdef __cplusplus
extern "C"{
#endif
	BOOL WINAPI DllMainCRTStartup (HANDLE hDll, DWORD dwReason, LPVOID lpReserved);
#ifdef __cplusplus
};
#endif

BOOL WINAPI DllMainCRTStartup (HANDLE hDll, DWORD dwReason, LPVOID lpReserved)
{
	(void)hDll;
	(void)dwReason;
	(void)lpReserved;
	return TRUE;
}

#else
/*
BOOL __stdcall DllMain (
		HINSTANCE hinstDLL,
		DWORD fdwReason,
		LPVOID lpvReserved
		)
{
	return TRUE;
}
*/
#endif
