#include <FarPluginBase.hpp>
#include "ace.class.h"
#include "unace.h"

MY_DEFINE_GUID (CLSID_FormatACE, 0x08183761, 0x87D3, 0x4C85, 0xBF, 0xB7, 0x7C, 0x5E, 0x8A, 0x7F, 0x81, 0xEA);

PluginStartupInfo Info;
FARSTANDARDFUNCTIONS FSF;

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

	return NAERROR_SUCCESS;
}

int OnFinalize ()
{
	return NAERROR_SUCCESS;
}

int OnQueryArchive (QueryArchiveStruct *pQAS)
{
	AceArchive *pArchive = new AceArchive (pQAS->lpFileName);

	if ( pArchive && pArchive->IsArchive() )
	{
		pQAS->hResult = (HANDLE)pArchive;
		return NAERROR_SUCCESS;
	}

	delete pArchive;
	return NAERROR_INTERNAL;
}

int OnOpenArchive (OpenArchiveStruct *pOAS)
{
	AceArchive *pArchive = (AceArchive*)pOAS->hArchive;

	pOAS->bResult = pArchive->pOpenArchive (pOAS->nMode, pOAS->pfnCallback);

	return NAERROR_SUCCESS;
}

int OnCloseArchive (CloseArchiveStruct *pCAS)
{
	AceArchive *pArchive = (AceArchive*)pCAS->hArchive;

	pArchive->pCloseArchive ();

	return NAERROR_SUCCESS;
}

int OnFinalizeArchive (AceArchive *pArchive)
{
	delete pArchive;

	return NAERROR_SUCCESS;
}

int OnGetArchivePluginInfo (
		ArchivePluginInfo *ai
		)
{
	static ArchiveFormatInfo formatInfo;

	formatInfo.uid = CLSID_FormatACE;
	formatInfo.dwFlags = AFF_SUPPORT_INTERNAL_EXTRACT;
	formatInfo.lpName = "ACE Archive";
	formatInfo.lpDefaultExtention = "ace";

	ai->nFormats = 1;
	ai->pFormatInfo = (ArchiveFormatInfo*)&formatInfo;

	return NAERROR_SUCCESS;
}

int OnGetArchiveItem (GetArchiveItemStruct *pGAI)
{
	AceArchive *pArchive = (AceArchive*)pGAI->hArchive;

	pGAI->nResult = pArchive->pGetArchiveItem (pGAI->pItem);

	return NAERROR_SUCCESS;
}

int OnExtract (ExtractStruct *pES)
{
	AceArchive *pArchive = (AceArchive*)pES->hArchive;

	pES->bResult = pArchive->pExtract (
			pES->pItems,
			pES->nItemsNumber,
			pES->lpDestPath,
			pES->lpCurrentPath
			);

	return NAERROR_SUCCESS;
}


int OnGetArchiveFormat (GetArchiveFormatStruct *pGAF)
{
	//RarArchive *pArchive = (RarArchive*)pGAF->hArchive;

	pGAF->uid = CLSID_FormatACE;

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


	if ( pGDC->uid == CLSID_FormatACE )
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
		return OnFinalizeArchive ((AceArchive*)pParams);

	case FID_GETARCHIVEPLUGININFO:
		return OnGetArchivePluginInfo ((ArchivePluginInfo*)pParams);

	case FID_GETARCHIVEITEM:
		return OnGetArchiveItem ((GetArchiveItemStruct*)pParams);

	case FID_GETARCHIVEFORMAT:
		return OnGetArchiveFormat ((GetArchiveFormatStruct*)pParams);

	case FID_EXTRACT:
		return OnExtract ((ExtractStruct*)pParams);

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
