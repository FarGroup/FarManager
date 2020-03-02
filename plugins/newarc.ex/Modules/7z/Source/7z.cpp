#include "7z.h"

PluginStartupInfo Info;
FARSTANDARDFUNCTIONS FSF;

SevenZipModule* pModule;

int OnInitialize (StartupInfo *pInfo)
{
	Info = pInfo->Info;
	FSF = *pInfo->Info.FSF;

	pModule = new SevenZipModule;
	pModule->Load();


//	MessageBoxW(0, _M(MTest), _T("123"), MB_OK);

	return NAERROR_SUCCESS;
}

int OnFinalize ()
{
	delete pModule;
	return NAERROR_SUCCESS;
}

int OnQueryArchive(QueryArchiveStruct* pQAS)
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
	SevenZipArchive* pArchive = (SevenZipArchive*)pSOS->hArchive;
	pSOS->bResult = pArchive->StartOperation(pSOS->nOperation, pSOS->bInternal);

	return NAERROR_SUCCESS;
}

int OnEndOperation(OperationStruct* pEOS)
{
	SevenZipArchive* pArchive = (SevenZipArchive*)pEOS->hArchive;
	pEOS->bResult = pArchive->EndOperation(pEOS->nOperation, pEOS->bInternal);

	return NAERROR_SUCCESS;
}


int OnCloseArchive(CloseArchiveStruct *pCAS)
{
	pModule->CloseArchive(pCAS->uidPlugin, (SevenZipArchive*)pCAS->hArchive);

	return NAERROR_SUCCESS;
}

int OnGetArchiveModuleInfo(ArchiveModuleInfo* ai)
{
	ai->dwFlags = AMF_SUPPORT_SINGLE_PLUGIN_QUERY;
	ai->uPlugins = pModule->GetNumberOfPlugins();
	ai->pPlugins = pModule->GetPlugins();
	ai->uid = pModule->GetUID();

	return NAERROR_SUCCESS;
}

int OnGetArchiveItem(GetArchiveItemStruct *pGAI)
{
	SevenZipArchive* pArchive = (SevenZipArchive*)pGAI->hArchive;
	pGAI->nResult = pArchive->GetArchiveItem(pGAI->pItem);

	return NAERROR_SUCCESS;
}

bool OnFreeArchiveItem(FreeArchiveItemStruct *pFAI)
{
	SevenZipArchive* pArchive = (SevenZipArchive*)pFAI->hArchive;
	pFAI->bResult = pArchive->FreeArchiveItem(pFAI->pItem);

	return NAERROR_SUCCESS;
}

int OnExtract(ExtractStruct *pES)
{
	SevenZipArchive* pArchive = (SevenZipArchive*)pES->hArchive;

	pES->nResult = pArchive->Extract(
			pES->pItems,
			pES->uItemsNumber,
			pES->lpDestPath,
			pES->lpCurrentPath
			);

	return NAERROR_SUCCESS;
}


int OnTest(TestStruct *pTS)
{
	SevenZipArchive *pArchive = (SevenZipArchive *)pTS->hArchive;

	pTS->nResult = pArchive->Test(
			pTS->pItems,
			pTS->uItemsNumber
			);

	return NAERROR_SUCCESS;
}

int OnAdd(AddStruct* pAS)
{
	SevenZipArchive* pArchive = (SevenZipArchive* )pAS->hArchive;

	pAS->nResult = pArchive->AddFiles(
			pAS->pItems,
			pAS->uItemsNumber,
			pAS->lpSourcePath,
			pAS->lpCurrentPath,
			pAS->pConfig
			);

	return NAERROR_SUCCESS;
}


int OnGetDefaultCommand(GetDefaultCommandStruct* pGDC)
{
	SevenZipPlugin* pPlugin = pModule->GetPlugin(pGDC->uidPlugin);

	if ( pPlugin )
	{
		pGDC->bResult = pPlugin->GetDefaultCommand(pGDC->uidFormat, pGDC->nCommand, &pGDC->lpCommand);
		pGDC->bEnabled = true;
	}
	else
		pGDC->bResult = false;
	
	return NAERROR_SUCCESS;
}

int OnDelete(DeleteStruct *pDS)
{
	SevenZipArchive *pArchive = (SevenZipArchive *)pDS->hArchive;

	pDS->nResult = pArchive->Delete(
			pDS->pItems,
			pDS->uItemsNumber
			);

	return NAERROR_SUCCESS;
}


int OnConfigure(ConfigureStruct* pCF)
{

/*
	SevenZipPlugin* pPlugin = pModule->GetPluginFromUID(pCF->uid);

	if ( pPlugin )
		pPlugin->Configure(pCF->uid);
	*/
	return NAERROR_SUCCESS;
}

int OnConfigureFormat(ConfigureFormatStruct* pCFS)
{
	//пока считаем, что тут у нас uidFormat уникален (а так оно и есть)

	const CompressionFormatInfo* pFormat = GetCompressionFormatInfo(pCFS->uidFormat);

	if ( pFormat )
	{
		SevenZipCompressionConfig* pCfg = SevenZipCompressionConfig::FromString(pFormat, pCFS->lpConfig);

		if ( dlgSevenZipPluginConfigure(pCfg) )
		{
			string strResult;

			pCfg->ToString(strResult);

			MessageBoxW (0, strResult, strResult, MB_OK);

			pCFS->lpResult = StrDuplicate(strResult);
			pCFS->bResult = true;

			return NAERROR_SUCCESS;
		}

		pCFS->bResult = false;
		pCFS->lpResult = nullptr;
	}
	
	return NAERROR_SUCCESS;
}

int OnFreeConfigResult(FreeConfigResultStruct* pFCR)
{
//	Config::Free
	StrFree(pFCR->lpResult);

	return NAERROR_SUCCESS;
}

int OnGetArchiveInfo(ArchiveInfoStruct* pAIS)
{
	SevenZipArchive* pArchive = (SevenZipArchive*)pAIS->hArchive;

	pAIS->nInfoItems = pArchive->GetArchiveInfo(pAIS->bMultiVolume, &pAIS->pInfo);
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
		return OnInitialize((StartupInfo*)pParams);

	case FID_FINALIZE:
		return OnFinalize();

	case FID_GETARCHIVEMODULEINFO:
		return OnGetArchiveModuleInfo((ArchiveModuleInfo*)pParams);

	case FID_QUERYARCHIVE:
		return OnQueryArchive((QueryArchiveStruct*)pParams);

	case FID_OPENARCHIVE:
	case FID_CREATEARCHIVE:
		return OnOpenCreateArchive((OpenCreateArchiveStruct*)pParams);

	case FID_STARTOPERATION:
		return OnStartOperation((OperationStruct*)pParams);

	case FID_ENDOPERATION:
		return OnEndOperation((OperationStruct*)pParams);

	case FID_CLOSEARCHIVE:
		return OnCloseArchive((CloseArchiveStruct*)pParams);

	case FID_GETARCHIVEITEM:
		return OnGetArchiveItem((GetArchiveItemStruct*)pParams);
	    
	case FID_FREEARCHIVEITEM:
		return OnFreeArchiveItem((FreeArchiveItemStruct*)pParams);

	case FID_EXTRACT:
		return OnExtract((ExtractStruct*)pParams);

	case FID_TEST:
		return OnTest((TestStruct*)pParams);

	case FID_GETDEFAULTCOMMAND:
		return OnGetDefaultCommand((GetDefaultCommandStruct*)pParams);

	case FID_DELETE:
		return OnDelete((DeleteStruct*)pParams);

	case FID_ADD:
		return OnAdd((AddStruct*)pParams);

	case FID_CONFIGURE:
		return OnConfigure((ConfigureStruct*)pParams);

	case FID_CONFIGUREFORMAT:
		return OnConfigureFormat((ConfigureFormatStruct*)pParams);

	case FID_FREECONFIGRESULT:
		return OnFreeConfigResult((FreeConfigResultStruct*)pParams);

	case FID_GETARCHIVEINFO:
		return OnGetArchiveInfo((ArchiveInfoStruct*)pParams);
	}

	return NAERROR_NOTIMPLEMENTED;
}


#if !defined(__GNUC__)

BOOL __stdcall DllMain (
		HINSTANCE hinstDLL,
		DWORD fdwReason,
		LPVOID lpvReserved
		)
{
	return TRUE;
}

#endif
