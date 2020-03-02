#include <Rtl.Base.h>
#include <FarPluginBase.hpp>
#include "ace.class.h"
#include "ace.h"

PluginStartupInfo Info;
FARSTANDARDFUNCTIONS FSF;
AceModule *pModule;

struct ArchiveModuleInformation {

	int nTypes;
	char *pTypeNames;

	int nConfigStringsNumber;

	char **pConfigStrings;
};

int OnInitialize (StartupInfo *pInfo)
{
	Info = pInfo->Info;
	FSF = *pInfo->Info.FSF;

	pModule = new AceModule;

	return NAERROR_SUCCESS;
}

int OnFinalize ()
{
	delete pModule;
	return NAERROR_SUCCESS;
}

int OnQueryArchive (QueryArchiveStruct *pQAS)
{
	AceArchive *pArchive = new AceArchive (pModule, pQAS->lpFileName, false);

	if ( pArchive && pArchive->IsArchive() )
	{
		pQAS->nFormats = -1;
		pQAS->hResult = (HANDLE)pArchive;
		return NAERROR_SUCCESS;
	}

	delete pArchive;

	return NAERROR_INTERNAL;
}

int OnCreateArchive (CreateArchiveStruct *pCAS)
{
	AceArchive *pArchive = new AceArchive(pModule, pCAS->lpFileName, true);

	pCAS->hResult = (HANDLE)pArchive;

	return NAERROR_SUCCESS;
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

	pModule->GetArchiveFormatInfo(&formatInfo);

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
	pGAF->uid = CLSID_FormatACE;

	return NAERROR_SUCCESS;
}

int OnGetDefaultCommand (GetDefaultCommandStruct *pGDC)
{
    static const char *pCommands[]={
    /*Extract               */"ace32 x {-p%%P} -y -c- -std {%%S} %%A @%%LN",
    /*Extract without paths */"ace32 e -av- {-p%%P} -y -c- -std {%%S} %%A @%%LN",
    /*Test                  */"ace32 t -y {-p%%P} -c- -std {%%S} %%A",
    /*Delete                */"ace32 d -y -std {-t%%W} {%%S} %%A @%%LN",
    /*Comment archive       */"ace32 cm -y -std {-t%%W} {%%S} %%A",
    /*Comment files         */"ace32 cf -y -std {-t%%W} {%%S} %%A {@%%LN}",
    /*Convert to SFX        */"ace32 s -y -std {%%S} %%A",
    /*Lock archive          */"ace32 k -y -std {%%S} %%A",
    /*Protect archive       */"ace32 rr -y -std {%%S} %%A",
    /*Recover archive       */"ace32 r -y -std {%%S} %%A",
    /*Add files             */"ace32 a -c2 -y -std {-p%%P} {-t%%W} {%%S} %%A @%%LN",
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

int OnAdd (AddStruct *pAS)
{
	AceArchive *pArchive = (AceArchive*)pAS->hArchive;

	pAS->bResult = pArchive->pAddFiles (
			pAS->lpSourcePath,
			pAS->lpCurrentPath,
			pAS->pItems,
			pAS->nItemsNumber
			);

	return NAERROR_SUCCESS;
}


int OnDelete (DeleteStruct *pDS)
{
	AceArchive *pArchive = (AceArchive*)pDS->hArchive;

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

	case FID_ADD:
		return OnAdd ((AddStruct*)pParams);

	case FID_DELETE:
		return OnDelete ((DeleteStruct*)pParams);

	case FID_CREATEARCHIVE:
		return OnCreateArchive ((CreateArchiveStruct*)pParams);
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
