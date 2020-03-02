#include "ma.h"

PluginStartupInfo Info;
FARSTANDARDFUNCTIONS FSF;

MaModule* pModule;

int OnInitialize(StartupInfo *pInfo)
{
	Info = pInfo->Info;
	FSF = *pInfo->Info.FSF;
	Info.FSF = &FSF;

	pModule = new MaModule;
	pModule->Load();

	return NAERROR_SUCCESS;
}

int OnFinalize()
{
	delete pModule;
	return NAERROR_SUCCESS;
}

int OnQueryArchive(QueryArchiveStruct *pQAS)
{
	bool bMoreArchives = false;

	const ArchiveQueryResult* pResult = pModule->QueryArchive(pQAS, bMoreArchives);

	if ( pResult )
	{
		pQAS->uidFormat = pResult->uidFormat;
		pQAS->uidPlugin = pResult->uidPlugin;

		if ( bMoreArchives )
			pQAS->dwFlags |= QUERY_FLAG_MORE_ARCHIVES;

		pQAS->bResult = true;
	}

	return NAERROR_SUCCESS;
}


int OnOpenArchive(OpenCreateArchiveStruct *pOAS)
{
	pOAS->hResult = (HANDLE)pModule->OpenArchive(pOAS->uidPlugin, pOAS->uidFormat, pOAS->lpFileName, pOAS->hCallback, pOAS->pfnCallback);
	return NAERROR_SUCCESS;
}

int OnCloseArchive(CloseArchiveStruct *pCAS)
{
	pModule->CloseArchive(pCAS->uidPlugin, (MaArchive*)pCAS->hArchive);
	return NAERROR_SUCCESS;
}

int OnGetArchiveModuleInfo (ArchiveModuleInfo *ai)
{
	ai->dwFlags = AMF_SUPPORT_SINGLE_PLUGIN_QUERY;
	ai->uPlugins = pModule->GetNumberOfPlugins();
	ai->pPlugins = pModule->GetPlugins();
	ai->uid = pModule->GetUID();

	return NAERROR_SUCCESS;
}

int OnGetArchiveItem(GetArchiveItemStruct *pGAI)
{
	MaArchive *pArchive = (MaArchive*)pGAI->hArchive;
	pGAI->nResult = pArchive->GetArchiveItem (pGAI->pItem);

	return NAERROR_SUCCESS;
}

bool OnFreeArchiveItem(FreeArchiveItemStruct *pFAI)
{
	MaArchive* pArchive = (MaArchive*)pFAI->hArchive;
	pFAI->bResult = true;
	
	pArchive->FreeArchiveItem(pFAI->pItem);

	return NAERROR_SUCCESS;
}


int OnStartOperation(OperationStruct* pSOS)
{
	MaArchive* pArchive = (MaArchive*)pSOS->hArchive;
	pSOS->bResult = pArchive->StartOperation(pSOS->nOperation, pSOS->bInternal);

	return NAERROR_SUCCESS;
}

int OnEndOperation(OperationStruct* pEOS)
{
	MaArchive* pArchive = (MaArchive*)pEOS->hArchive;
	pEOS->bResult = pArchive->EndOperation(pEOS->nOperation, pEOS->bInternal);

	return NAERROR_SUCCESS;
}

int OnGetDefaultCommand (GetDefaultCommandStruct *pGDC)
{
	MaPlugin* pPlugin = pModule->GetPlugin(pGDC->uidPlugin);

	if ( pPlugin )
	{
		pGDC->bResult = pPlugin->GetDefaultCommand(pGDC->uidFormat, pGDC->nCommand, &pGDC->lpCommand);
		pGDC->bEnabled = true;
	}
	else
		pGDC->bResult = false;
	
	return NAERROR_SUCCESS;
}


int OnGetArchiveInfo(ArchiveInfoStruct* pAIS)
{
	MaArchive* pArchive = (MaArchive*)pAIS->hArchive;

	pAIS->nInfoItems = pArchive->GetArchiveInfo(&pAIS->pInfo);
	pAIS->bResult = (pAIS->nInfoItems > 0); //USELESS

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
		return OnOpenArchive ((OpenCreateArchiveStruct*)pParams);

	case FID_CLOSEARCHIVE:
		return OnCloseArchive ((CloseArchiveStruct*)pParams);

	case FID_GETARCHIVEMODULEINFO:
		return OnGetArchiveModuleInfo ((ArchiveModuleInfo*)pParams);

	case FID_GETARCHIVEITEM:
		return OnGetArchiveItem((GetArchiveItemStruct*)pParams);

	case FID_FREEARCHIVEITEM:
		return OnFreeArchiveItem((FreeArchiveItemStruct*)pParams);

	case FID_GETDEFAULTCOMMAND:
		return OnGetDefaultCommand ((GetDefaultCommandStruct*)pParams);

	case FID_STARTOPERATION:
		return OnStartOperation((OperationStruct*)pParams);

	case FID_ENDOPERATION:
		return OnEndOperation((OperationStruct*)pParams);

	case FID_GETARCHIVEINFO:
		return OnGetArchiveInfo((ArchiveInfoStruct*)pParams);
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
	(void)lpReserved;                                              s
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
}
*/
#endif
