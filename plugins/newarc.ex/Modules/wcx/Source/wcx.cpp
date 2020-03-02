#include "wcx.h"



PluginStartupInfo Info;
FARSTANDARDFUNCTIONS FSF;

WcxModule *pModule;

int OnInitialize (StartupInfo *pInfo)
{
	Info = pInfo->Info;
	FSF = *pInfo->Info.FSF;
	Info.FSF = &FSF;

	pModule = new WcxModule;
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


int OnOpenCreateArchive(OpenCreateArchiveStruct *pOAS)
{
	pOAS->hResult = (HANDLE)pModule->OpenCreateArchive(
				pOAS->uidPlugin, 
				pOAS->uidFormat, 
				pOAS->lpFileName, 
				pOAS->hCallback, 
				pOAS->pfnCallback,
				pOAS->bCreate
				);

	return NAERROR_SUCCESS;
}

int OnCloseArchive(CloseArchiveStruct *pCAS)
{
	pModule->CloseArchive(pCAS->uidPlugin, (WcxArchive*)pCAS->hArchive);
	return NAERROR_SUCCESS;
}


int OnGetArchiveModuleInfo(ArchiveModuleInfo *ai)
{
	ai->dwFlags = AMF_SUPPORT_SINGLE_PLUGIN_QUERY;
	ai->pPlugins = pModule->GetPlugins();
	ai->uPlugins = pModule->GetNumberOfPlugins();
	ai->uid = pModule->GetUID();
	return NAERROR_SUCCESS;
}

int OnGetArchiveItem(GetArchiveItemStruct *pGAI)
{
	WcxArchive *pArchive = (WcxArchive*)pGAI->hArchive;
	pGAI->nResult = pArchive->GetArchiveItem (pGAI->pItem);

	return NAERROR_SUCCESS;
}

int OnFreeArchiveItem(FreeArchiveItemStruct *pFAI)
{
	WcxArchive* pArchive = (WcxArchive*)pFAI->hArchive;
	pFAI->bResult = true;
	
	pArchive->FreeArchiveItem(pFAI->pItem);

	return NAERROR_SUCCESS;
}



int OnExtract(ExtractStruct *pES)
{
	WcxArchive *pArchive = (WcxArchive *)pES->hArchive;

	pES->nResult = pArchive->Extract(
			pES->pItems,
			pES->uItemsNumber,
			pES->lpDestPath,
			pES->lpCurrentPath
			);

	return NAERROR_SUCCESS;
}


int OnTest (TestStruct *pTS)
{
/*
	WcxArchive *pArchive = (WcxArchive *)pTS->hArchive;

	pTS->bResult = pArchive->pTest (
			pTS->pItems,
			pTS->nItemsNumber
			);
*/
	return NAERROR_SUCCESS;
}

int OnAdd(AddStruct *pAS)
{
	WcxArchive *pArchive = (WcxArchive*)pAS->hArchive;

	pAS->nResult = pArchive->AddFiles(
			pAS->pItems,
			pAS->uItemsNumber,
			pAS->lpSourcePath,
			pAS->lpCurrentPath
			);

	return NAERROR_SUCCESS;
}

int OnDelete(DeleteStruct *pDS)
{
	WcxArchive *pArchive = (WcxArchive*)pDS->hArchive;

	pDS->nResult = pArchive->Delete(
			pDS->pItems,
			pDS->uItemsNumber
			);

	return NAERROR_SUCCESS;
}



int OnStartOperation(OperationStruct* pSOS)
{
	WcxArchive* pArchive = (WcxArchive*)pSOS->hArchive;
	pSOS->bResult = pArchive->StartOperation(pSOS->nOperation, pSOS->bInternal);

	return NAERROR_SUCCESS;
}

int OnEndOperation(OperationStruct* pEOS)
{
	WcxArchive* pArchive = (WcxArchive*)pEOS->hArchive;
	pEOS->bResult = pArchive->EndOperation(pEOS->nOperation, pEOS->bInternal);

	return NAERROR_SUCCESS;
}


int OnGetDefaultCommand (GetDefaultCommandStruct* pGDC)
{
	return NAERROR_SUCCESS;
}

int OnConfigureFormat(ConfigureFormatStruct* pCFS)
{
	WcxPlugin* pPlugin = pModule->GetPlugin(pCFS->uidPlugin);

	if ( pPlugin )
		pPlugin->ConfigurePacker();

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

	case FID_STARTOPERATION:
		return OnStartOperation ((OperationStruct*)pParams);

	case FID_ENDOPERATION:
		return OnEndOperation ((OperationStruct*)pParams);

	case FID_GETARCHIVEMODULEINFO:
		return OnGetArchiveModuleInfo ((ArchiveModuleInfo*)pParams);

	case FID_GETARCHIVEITEM:
		return OnGetArchiveItem((GetArchiveItemStruct*)pParams);

	case FID_FREEARCHIVEITEM:
		return OnFreeArchiveItem((FreeArchiveItemStruct*)pParams);

	case FID_EXTRACT:
		return OnExtract ((ExtractStruct*)pParams);

	case FID_TEST:
		return OnTest ((TestStruct*)pParams);

	case FID_GETDEFAULTCOMMAND:
		return OnGetDefaultCommand ((GetDefaultCommandStruct*)pParams);
	
	case FID_DELETE:
		return OnDelete ((DeleteStruct*)pParams);

	case FID_ADD:
		return OnAdd ((AddStruct*)pParams);

	case FID_CREATEARCHIVE:
	case FID_OPENARCHIVE:
		return OnOpenCreateArchive ((OpenCreateArchiveStruct*)pParams);

	case FID_CONFIGUREFORMAT:
		return OnConfigureFormat((ConfigureFormatStruct*)pParams);

	case FID_CLOSEARCHIVE:
		return OnCloseArchive((CloseArchiveStruct*)pParams);
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
