#include "d5d.h"

// {567FAD13-AF54-45EC-BD70-AD60FA1C2CA4}
MY_DEFINE_GUID(CLSID_ModuleD5D, 0x567fad13, 0xaf54, 0x45ec, 0xbd, 0x70, 0xad, 0x60, 0xfa, 0x1c, 0x2c, 0xa4);


PluginStartupInfo Info;
FARSTANDARDFUNCTIONS FSF;

D5DModule *pModule;

int OnInitialize (StartupInfo *pInfo)
{
	Info = pInfo->Info;
	FSF = *pInfo->Info.FSF;

	pModule = new D5DModule;
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

int OnOpenArchive(OpenCreateArchiveStruct *pOAS)
{
	pOAS->hResult = (HANDLE)pModule->OpenArchive(pOAS->uidPlugin, pOAS->uidFormat, pOAS->lpFileName, pOAS->hCallback, pOAS->pfnCallback);
	return NAERROR_SUCCESS;
}

int OnCloseArchive(CloseArchiveStruct *pCAS)
{
	pModule->CloseArchive(pCAS->uidPlugin, (D5DArchive*)pCAS->hArchive);
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
	D5DArchive *pArchive = (D5DArchive*)pGAI->hArchive;
	pGAI->nResult = pArchive->GetArchiveItem (pGAI->pItem);

	return NAERROR_SUCCESS;
}

bool OnFreeArchiveItem(FreeArchiveItemStruct *pFAI)
{
	D5DArchive* pArchive = (D5DArchive*)pFAI->hArchive;
	pFAI->bResult = true;
	pArchive->FreeArchiveItem(pFAI->pItem);

	return NAERROR_SUCCESS;
}



int OnExtract(ExtractStruct *pES)
{
	D5DArchive *pArchive = (D5DArchive *)pES->hArchive;

	pES->bResult = pArchive->Extract (
			pES->pItems,
			pES->nItemsNumber,
			pES->lpDestPath,
			pES->lpCurrentPath
			);

	return NAERROR_SUCCESS;
}


int OnStartOperation(OperationStruct* pSOS)
{
	D5DArchive* pArchive = (D5DArchive*)pSOS->hArchive;
	pSOS->bResult = pArchive->StartOperation(pSOS->nOperation, pSOS->bInternal);

	return NAERROR_SUCCESS;
}

int OnEndOperation(OperationStruct* pEOS)
{
	D5DArchive* pArchive = (D5DArchive*)pEOS->hArchive;
	pEOS->bResult = pArchive->EndOperation(pEOS->nOperation, pEOS->bInternal);

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
		return OnOpenArchive((OpenCreateArchiveStruct*)pParams);

	case FID_CLOSEARCHIVE:
		return OnCloseArchive ((CloseArchiveStruct*)pParams);

	case FID_GETARCHIVEMODULEINFO:
		return OnGetArchiveModuleInfo ((ArchiveModuleInfo*)pParams);

	case FID_FREEARCHIVEITEM:
		return OnFreeArchiveItem((FreeArchiveItemStruct*)pParams);

	case FID_GETARCHIVEITEM:
		return OnGetArchiveItem ((GetArchiveItemStruct*)pParams);

	case FID_EXTRACT:
		return OnExtract ((ExtractStruct*)pParams);

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

HINSTANCE hinst;

BOOL __stdcall DllMain (
		HINSTANCE hinstDLL,
		DWORD fdwReason,
		LPVOID lpvReserved
		)
{
	hinst = hinstDLL;
	return true;
}

#endif
