#include "multiex.h"

PluginStartupInfo Info;
FARSTANDARDFUNCTIONS FSF;

MultiExModule *pModule;

int OnInitialize (StartupInfo *pInfo)
{
	Info = pInfo->Info;
	FSF = *pInfo->Info.FSF;
	Info.FSF = &FSF;

	pModule = new MultiExModule;
	pModule->Load();

	return NAERROR_SUCCESS;
}

int OnFinalize ()
{
	delete pModule;
	return NAERROR_SUCCESS;
}

int OnQueryArchive (QueryArchiveStruct *pQAS)
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

int OnOpenCreateArchive(OpenCreateArchiveStruct *pCAS)
{
	pCAS->hResult = (HANDLE)pModule->OpenCreateArchive(
			pCAS->uidPlugin, 
			pCAS->uidFormat, 
			pCAS->lpFileName, 
			pCAS->hCallback, 
			pCAS->pfnCallback,
			pCAS->bCreate
			);

	return NAERROR_SUCCESS;
}

int OnStartOperation(OperationStruct* pSOS)
{
	MultiExArchive* pArchive = (MultiExArchive*)pSOS->hArchive;
	pSOS->bResult = pArchive->StartOperation(pSOS->nOperation, pSOS->bInternal);

	return NAERROR_SUCCESS;
}

int OnEndOperation(OperationStruct* pEOS)
{
	MultiExArchive* pArchive = (MultiExArchive*)pEOS->hArchive;
	pArchive->EndOperation(pEOS->nOperation, pEOS->bInternal);

	return NAERROR_SUCCESS;
}

int OnCloseArchive(CloseArchiveStruct *pCAS)
{
	pModule->CloseArchive(pCAS->uidPlugin, (MultiExArchive*)pCAS->hArchive);
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

int OnGetArchiveItem (GetArchiveItemStruct *pGAI)
{
	MultiExArchive *pArchive = (MultiExArchive*)pGAI->hArchive;
	pGAI->nResult = pArchive->GetArchiveItem (pGAI->pItem);

	return NAERROR_SUCCESS;
}

bool OnFreeArchiveItem(FreeArchiveItemStruct *pFAI)
{
	MultiExArchive* pArchive = (MultiExArchive*)pFAI->hArchive;
	pFAI->bResult = true;
	
	pArchive->FreeArchiveItem(pFAI->pItem);

	return NAERROR_SUCCESS;
}



int OnExtract(ExtractStruct *pES)
{
	MultiExArchive *pArchive = (MultiExArchive *)pES->hArchive;

	pES->bResult = pArchive->Extract (
			pES->pItems,
			pES->nItemsNumber,
			pES->lpDestPath,
			pES->lpCurrentPath
			);

	return NAERROR_SUCCESS;
}


int OnAdd(AddStruct *pAS)
{
	MultiExArchive *pArchive = (MultiExArchive*)pAS->hArchive;

	pAS->bResult = pArchive->AddFiles(
			pAS->pItems,
			pAS->nItemsNumber,
			pAS->lpSourcePath,
			pAS->lpCurrentPath
			);

	return NAERROR_SUCCESS;
}

int OnDelete(DeleteStruct *pDS)
{
	MultiExArchive *pArchive = (MultiExArchive*)pDS->hArchive;

	pDS->bResult = pArchive->Delete(
			pDS->pItems,
			pDS->nItemsNumber
			);

	return NAERROR_SUCCESS;
}


int __stdcall ModuleEntry(
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
	case FID_CREATEARCHIVE:
		return OnOpenCreateArchive((OpenCreateArchiveStruct*)pParams);

	case FID_CLOSEARCHIVE:
		return OnCloseArchive ((CloseArchiveStruct*)pParams);

	case FID_GETARCHIVEMODULEINFO:
		return OnGetArchiveModuleInfo ((ArchiveModuleInfo*)pParams);

	case FID_GETARCHIVEITEM:
		return OnGetArchiveItem((GetArchiveItemStruct*)pParams);

	case FID_FREEARCHIVEITEM:
		return OnFreeArchiveItem((FreeArchiveItemStruct*)pParams);

	case FID_EXTRACT:
		return OnExtract ((ExtractStruct*)pParams);

	case FID_DELETE:
		return OnDelete ((DeleteStruct*)pParams);

	case FID_ADD:
		return OnAdd ((AddStruct*)pParams);

	case FID_STARTOPERATION:
		return OnStartOperation((OperationStruct*)pParams);

	case FID_ENDOPERATION:
		return OnEndOperation((OperationStruct*)pParams);
		
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
