#include <Rtl.Base.h>
#include <FarPluginBase.hpp>
#include "ma.class.h"

PluginStartupInfo Info;
FARSTANDARDFUNCTIONS FSF;

MaModules *pModules;

int OnInitialize (StartupInfo *pInfo)
{
	Info = pInfo->Info;
	FSF = *pInfo->Info.FSF;
	Info.FSF=&FSF;

	pModules = new MaModules;

	return NAERROR_SUCCESS;
}

int OnFinalize ()
{
	delete pModules;

	return NAERROR_SUCCESS;
}

int OnQueryArchive (QueryArchiveStruct *pQAS)
{
	int nModuleNum;
	MaModule *pModule = pModules->IsArchive (pQAS, &nModuleNum);

	if ( pModule )
	{
		bool bResult = false;

		MaArchive *pArchive = new MaArchive (pModule, nModuleNum, pQAS->lpFileName);

		if ( pArchive->pOpenArchive () )
		{
			bResult = true;
			pArchive->pCloseArchive ();
		}

		if ( bResult )
		{
			pQAS->nFormats = -1; //BUGBUG надо сделать поддержку попадания под несколько форматок
			pQAS->hResult = (HANDLE)pArchive;
			return NAERROR_SUCCESS;
		}
	}

	return NAERROR_INTERNAL;
}

int OnOpenArchive (OpenArchiveStruct *pOAS)
{
	MaArchive *pArchive = (MaArchive*)pOAS->hArchive;

	pOAS->bResult = pArchive->pOpenArchive ();

	return NAERROR_SUCCESS;
}

int OnCloseArchive (CloseArchiveStruct *pCAS)
{
	MaArchive *pArchive = (MaArchive*)pCAS->hArchive;

	pArchive->pCloseArchive ();

	return NAERROR_SUCCESS;
}

int OnFinalizeArchive (MaArchive *pArchive)
{
	delete pArchive;

	return NAERROR_SUCCESS;
}

int OnGetArchivePluginInfo (
		ArchivePluginInfo *ai
		)
{
	pModules->GetArchivePluginInfo( ai );

	return NAERROR_SUCCESS;
}

int OnGetArchiveItem (GetArchiveItemStruct *pGAI)
{
	MaArchive *pArchive = (MaArchive*)pGAI->hArchive;

	pGAI->nResult = pArchive->pGetArchiveItem (pGAI->pItem);

	return NAERROR_SUCCESS;
}

int OnGetArchiveFormat (GetArchiveFormatStruct *pGAF)
{
	MaArchive *pArchive = (MaArchive*)pGAF->hArchive;

    pArchive->pGetArchiveType (&pGAF->uid);

	return NAERROR_SUCCESS;
}

int OnGetDefaultCommand (GetDefaultCommandStruct *pGDC)
{
	pGDC->bResult = pModules->GetDefaultCommand (pGDC->uid, pGDC->nCommand, pGDC->lpCommand);

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
		return OnFinalizeArchive ((MaArchive*)pParams);

	case FID_GETARCHIVEPLUGININFO:
		return OnGetArchivePluginInfo ((ArchivePluginInfo*)pParams);

	case FID_GETARCHIVEITEM:
		return OnGetArchiveItem ((GetArchiveItemStruct*)pParams);

	case FID_GETARCHIVEFORMAT:
		return OnGetArchiveFormat ((GetArchiveFormatStruct*)pParams);

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

BOOL __stdcall DllMain (
		HINSTANCE hinstDLL,
		DWORD fdwReason,
		LPVOID lpvReserved
		)
{
	return true;
}

#endif
