#include <FarPluginBase.h>
#include <stdio.h>
#include "zip.class.h"

PluginStartupInfo Info;
FARSTANDARDFUNCTIONS FSF;

ZipModule *pModule;

int OnInitialize (PluginStartupInfo *pInfo)
{
	Info = *pInfo;
	FSF = *pInfo->FSF;

	pModule = new ZipModule (NULL);

	return NAERROR_SUCCESS;
}

int OnGetArchivePluginInfo (ArchivePluginInfo *ai)
{
	static ArchiveFormatInfo FormatInfo[1] = {
			{AFF_SUPPORT_INTERNAL_EXTRACT, "ZIP Archive", "zip"}
			};

	ai->nFormats = 1;
	ai->pFormatInfo = (ArchiveFormatInfo*)&FormatInfo;

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
	ZipArchive *pArchive = (ZipArchive*)pGAF->hArchive;

	pGAF->nFormat = pArchive->pGetArchiveType ();

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
    static char *pCommands[]={
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
/*	char s[100];

	sprintf (s, "%d", nFunctionID);

	MessageBox (0, s, "asd", MB_OK);*/

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
