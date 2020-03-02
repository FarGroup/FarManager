#include "observer.h"

PluginStartupInfo Info;
FARSTANDARDFUNCTIONS FSF;

ObserverModule* pModule;

int OnInitialize (StartupInfo *pInfo)
{
	Info = pInfo->Info;
	FSF = *pInfo->Info.FSF;
	Info.FSF = &FSF;

	pModule = new ObserverModule;
	pModule->Load();

	return NAERROR_SUCCESS;
}

int OnFinalize ()
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
	pModule->CloseArchive(pCAS->uidPlugin, (ObserverArchive*)pCAS->hArchive);
	return NAERROR_SUCCESS;
}


int OnGetArchiveModuleInfo (ArchiveModuleInfo *ai)
{
	ai->uid = pModule->GetUID();
	ai->uPlugins = pModule->GetNumberOfPlugins();
	ai->pPlugins = pModule->GetPlugins();
	ai->dwFlags = AMF_SUPPORT_SINGLE_PLUGIN_QUERY;

	return NAERROR_SUCCESS;
}

int OnGetArchiveItem (GetArchiveItemStruct *pGAI)
{
	ObserverArchive *pArchive = (ObserverArchive*)pGAI->hArchive;
	pGAI->nResult = pArchive->GetArchiveItem (pGAI->pItem);

	return NAERROR_SUCCESS;
}

bool OnFreeArchiveItem(FreeArchiveItemStruct *pFAI)
{
	ObserverArchive* pArchive = (ObserverArchive*)pFAI->hArchive;
	pFAI->bResult = pArchive->FreeArchiveItem(pFAI->pItem);

	return NAERROR_SUCCESS;
}


int OnExtract (ExtractStruct *pES)
{
	ObserverArchive *pArchive = (ObserverArchive *)pES->hArchive;

	pES->nResult = pArchive->Extract(
			pES->pItems,
			pES->uItemsNumber,
			pES->lpDestPath,
			pES->lpCurrentPath
			);

	return NAERROR_SUCCESS;
}

int OnStartOperation(OperationStruct* pSOS)
{
	ObserverArchive* pArchive = (ObserverArchive*)pSOS->hArchive;
	pSOS->bResult = pArchive->StartOperation(pSOS->nOperation, pSOS->bInternal);

	return NAERROR_SUCCESS;
}

int OnEndOperation(OperationStruct* pEOS)
{
	ObserverArchive* pArchive = (ObserverArchive*)pEOS->hArchive;
	pEOS->bResult = pArchive->EndOperation(pEOS->nOperation, pEOS->bInternal);

	return NAERROR_SUCCESS;
}

int OnGetArchiveInfo(ArchiveInfoStruct* pAIS)
{
	ObserverArchive* pArchive = (ObserverArchive*)pAIS->hArchive;

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
		return OnGetArchiveItem ((GetArchiveItemStruct*)pParams);

	case FID_FREEARCHIVEITEM:
		return OnFreeArchiveItem((FreeArchiveItemStruct*)pParams);
		
	case FID_EXTRACT:
		return OnExtract ((ExtractStruct*)pParams);

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
