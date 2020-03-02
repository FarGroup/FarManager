#include "zip.h"

MY_DEFINE_GUID (CLSID_FormatZIP, 0x946378F0, 0xB7C0, 0x4811, 0x82, 0x7A, 0x27, 0xB2, 0xE9, 0xBC, 0xE8, 0x54);

PluginStartupInfo Info;
FARSTANDARDFUNCTIONS FSF;

ZipModule *pModule;

int OnInitialize (StartupInfo *pInfo)
{
	Info = pInfo->Info;
	FSF = *pInfo->Info.FSF;

	pModule = new ZipModule (NULL);

	return NAERROR_SUCCESS;
}

int OnGetArchivePluginInfo (ArchivePluginInfo *ai)
{
	static ArchiveFormatInfo formatInfo;

	formatInfo.uid = CLSID_FormatZIP;
	formatInfo.dwFlags = AFF_SUPPORT_INTERNAL_EXTRACT;
	formatInfo.lpName = "ZIP Archive";
	formatInfo.lpDefaultExtention = "zip";

	ai->nFormats = 1;
	ai->pFormatInfo = (ArchiveFormatInfo*)&formatInfo;

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
		unzFile hFile = pModule->m_pfnUnzOpen (pQAS->lpFileName);

		if ( hFile )
		{
			pModule->m_pfnUnzClose (hFile);
			pQAS->nFormats = -1;
			pQAS->hResult = (HANDLE)new ZipArchive (pModule, pQAS->lpFileName);

			return NAERROR_SUCCESS;
		}
	}

	return NAERROR_INTERNAL; //а вот это неверно. должен быть все равно SUCCESS, но INVALID_HANDLE_VALUE
}

int OnOpenArchive (OpenArchiveStruct *pOAS)
{
	ZipArchive *pArchive = (ZipArchive*)pOAS->hArchive;

	pOAS->bResult = pArchive->pOpenArchive (pOAS->nMode, pOAS->pfnCallback);

	return NAERROR_SUCCESS;
}

int OnCloseArchive (CloseArchiveStruct *pCAS)
{
	ZipArchive *pArchive = (ZipArchive*)pCAS->hArchive;

	pArchive->pCloseArchive ();

	return NAERROR_SUCCESS;
}

int OnFinalizeArchive (ZipArchive *pArchive)
{
	delete pArchive;

	return NAERROR_SUCCESS;
}

int OnGetArchiveItem (GetArchiveItemStruct *pGAI)
{
	ZipArchive *pArchive = (ZipArchive*)pGAI->hArchive;

	pGAI->nResult = pArchive->pGetArchiveItem (pGAI->pItem);

	return NAERROR_SUCCESS;
}

int OnGetArchiveFormat (GetArchiveFormatStruct *pGAF)
{
	//ZipArchive *pArchive = (ZipArchive*)pGAF->hArchive;

	pGAF->uid = CLSID_FormatZIP;

	return NAERROR_SUCCESS;
}

int OnExtract (ExtractStruct *pES)
{
	ZipArchive *pArchive = (ZipArchive*)pES->hArchive;

	pES->bResult = pArchive->pExtract (
			pES->pItems,
			pES->nItemsNumber,
			pES->lpDestPath,
			pES->lpCurrentPath
			);

	return NAERROR_SUCCESS;
}

int OnGetDefaultCommand (GetDefaultCommandStruct *pGDC)
{
    // Console PKZIP 4.0/Win32 commands
    static const char *pCommands[]={
    /*Extract               */"pkzipc -ext -dir -over=all -nozip -mask=none -times=mod {-pass=%%P} %%A @%%LNMA",
    /*Extract without paths */"pkzipc -ext -over=all -nozip -mask=none -times=mod {-pass=%%P} %%A @%%LNMA",
    /*Test                  */"pkzipc -test=all -nozip {-pass=%%P} %%A",
    /*Delete                */"pkzipc -delete -nozip {-temp=%%W} %%A @%%LNMA",
    /*Comment archive       */"pkzipc -hea -nozip {-temp=%%W} %%A",
    /*Comment files         */"pkzipc -com=all -nozip {-temp=%%W} %%A",
    /*Convert to SFX        */"pkzipc -sfx -nozip %%A",
    /*Lock archive          */"",
    /*Protect archive       */"",
    /*Recover archive       */"%comspec% /c echo.|pkzipc -fix -nozip %%A",
    /*Add files             */"pkzipc -add -attr=all -nozip {-pass=%%P} {-temp=%%W} %%A @%%LNMA"
    };

	if ( pGDC->uid == CLSID_FormatZIP )
	{
		strcpy (pGDC->lpCommand, pCommands[pGDC->nCommand]);

		pGDC->bResult = true;
	}
	else
		pGDC->bResult = false;

	return NAERROR_SUCCESS;
}

int __stdcall ModuleEntry (
		int nFunctionID,
		void *pParams
		)
{
	switch ( nFunctionID ) {

	case FID_INITIALIZE:
		return OnInitialize ((StartupInfo*)pParams);

	case FID_FINALIZE:
		return OnFinalize ();

	case FID_QUERYARCHIVE:
		return OnQueryArchive ((QueryArchiveStruct*)pParams);

	case FID_OPENARCHIVE:
		return OnOpenArchive ((OpenArchiveStruct*)pParams);

	case FID_CLOSEARCHIVE:
		return OnCloseArchive ((CloseArchiveStruct*)pParams);

	case FID_FINALIZEARCHIVE:
		return OnFinalizeArchive ((ZipArchive*)pParams);

	case FID_GETARCHIVEPLUGININFO:
		return OnGetArchivePluginInfo ((ArchivePluginInfo*)pParams);

	case FID_GETARCHIVEITEM:
		return OnGetArchiveItem ((GetArchiveItemStruct*)pParams);

	case FID_GETARCHIVEFORMAT:
		return OnGetArchiveFormat ((GetArchiveFormatStruct*)pParams);

	case FID_EXTRACT:
		return OnExtract ((ExtractStruct*)pParams);

//	case FID_TEST:
//		return OnTest ((TestStruct*)pParams);

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
	return true;
}*/
#endif
