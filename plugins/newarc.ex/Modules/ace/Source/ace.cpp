#include "ace.h"

PluginStartupInfo Info;
FARSTANDARDFUNCTIONS FSF;

AceModule *pModule;

int OnInitialize (StartupInfo *pInfo)
{
	Info = pInfo->Info;
	FSF = *pInfo->Info.FSF;

	pModule = new AceModule;

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
	ArchiveQueryResult Result;

	if ( pModule->QueryArchive(pQAS, &Result) )
	{
		pQAS->bResult = true;
		
		pQAS->uidFormat = Result.uidFormat;
		pQAS->uidPlugin = Result.uidPlugin;
	}

	return NAERROR_SUCCESS;
}

int OnOpenCreateArchive(OpenCreateArchiveStruct* pOAS)
{
	pOAS->hResult = (HANDLE)pModule->OpenCreateArchive(pOAS->lpFileName, pOAS->hCallback, pOAS->pfnCallback, pOAS->bCreate);
	return NAERROR_SUCCESS;
}

int OnCloseArchive(CloseArchiveStruct* pCAS)
{
	pModule->CloseArchive((AceArchive*)pCAS->hArchive);
	return NAERROR_SUCCESS;
}

int OnStartOperation(OperationStruct* pOS)
{
	AceArchive* pArchive = (AceArchive*)pOS->hArchive;
	pOS->bResult = pArchive->StartOperation(pOS->nOperation, pOS->bInternal);

	return NAERROR_SUCCESS;
}

int OnEndOperation(OperationStruct* pOS)
{
	AceArchive* pArchive = (AceArchive*)pOS->hArchive;
	pArchive->EndOperation(pOS->nOperation, pOS->bInternal);

	return NAERROR_SUCCESS;
}

int OnGetArchiveModuleInfo (
		ArchiveModuleInfo *ai
		)
{
	ai->uid = pModule->GetUID();
	ai->dwFlags = AMF_SUPPORT_SINGLE_PLUGIN_QUERY; //we just dont care, only one format
	ai->uPlugins = pModule->GetNumberOfPlugins();
	ai->pPlugins = pModule->GetPlugins();

	return NAERROR_SUCCESS;
}

int OnGetArchiveItem(GetArchiveItemStruct *pGAI)
{
	AceArchive *pArchive = (AceArchive*)pGAI->hArchive;

	pGAI->nResult = pArchive->GetArchiveItem(pGAI->pItem);

	return NAERROR_SUCCESS;
}

bool OnFreeArchiveItem(FreeArchiveItemStruct *pFAI)
{
	AceArchive* pArchive = (AceArchive*)pFAI->hArchive;
	
	pArchive->FreeArchiveItem(pFAI->pItem);
	pFAI->bResult = true;

	return NAERROR_SUCCESS;
}


int OnExtract (ExtractStruct *pES)
{
	AceArchive *pArchive = (AceArchive*)pES->hArchive;

	pES->bResult = pArchive->Extract(
			pES->pItems,
			pES->nItemsNumber,
			pES->lpDestPath,
			pES->lpCurrentPath
			);

	return NAERROR_SUCCESS;
}


int OnGetDefaultCommand (GetDefaultCommandStruct *pGDC)
{
    static const TCHAR *pCommands[]={
    /*Extract               */_T("ace32 x {-p%%P} -y -c- -std {%%S} %%A @%%LN"),
    /*Extract without paths */_T("ace32 e -av- {-p%%P} -y -c- -std {%%S} %%A @%%LN"),
    /*Test                  */_T("ace32 t -y {-p%%P} -c- -std {%%S} %%A"),
    /*Delete                */_T("ace32 d -y -std {-t%%W} {%%S} %%A @%%LN"),
    /*Comment archive       */_T("ace32 cm -y -std {-t%%W} {%%S} %%A"),
    /*Comment files         */_T("ace32 cf -y -std {-t%%W} {%%S} %%A {@%%LN}"),
    /*Convert to SFX        */_T("ace32 s -y -std {%%S} %%A"),
    /*Lock archive          */_T("ace32 k -y -std {%%S} %%A"),
    /*Protect archive       */_T("ace32 rr -y -std {%%S} %%A"),
    /*Recover archive       */_T("ace32 r -y -std {%%S} %%A"),
    /*Add files             */_T("ace32 a -c2 -y -std {-p%%P} {-t%%W} {%%S} %%A @%%LN"),
    };


	/*if ( pGDC->uid == CLSID_FormatACE )
	{
		_tcscpy (pGDC->lpCommand, pCommands[pGDC->nCommand]); //BUGBUG!!!
		pGDC->bResult = true;
	}
	else*/
		pGDC->bResult = false;

	return NAERROR_SUCCESS;
}

int OnAdd(AddStruct *pAS)
{
	AceArchive *pArchive = (AceArchive*)pAS->hArchive;

	pAS->bResult = pArchive->AddFiles(
			pAS->pItems,
			pAS->nItemsNumber,
			pAS->lpSourcePath,
			pAS->lpCurrentPath
			);

	return NAERROR_SUCCESS;
}


int OnDelete (DeleteStruct *pDS)
{
	AceArchive *pArchive = (AceArchive*)pDS->hArchive;

	pDS->bResult = pArchive->Delete(
			pDS->pItems,
			pDS->nItemsNumber
			);

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

	case FID_GETDEFAULTCOMMAND:
		return OnGetDefaultCommand ((GetDefaultCommandStruct*)pParams);

	case FID_ADD:
		return OnAdd ((AddStruct*)pParams);

	case FID_DELETE:
		return OnDelete ((DeleteStruct*)pParams);

	case FID_OPENARCHIVE:
	case FID_CREATEARCHIVE:
		return OnOpenCreateArchive((OpenCreateArchiveStruct*)pParams);

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
