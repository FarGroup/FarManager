#include "rar.h"

// {D7495BDD-BD70-4FEA-A20B-DE421ABA5847}
MY_DEFINE_GUID(CLSID_ModuleRAR, 0xd7495bdd, 0xbd70, 0x4fea, 0xa2, 0xb, 0xde, 0x42, 0x1a, 0xba, 0x58, 0x47);
MY_DEFINE_GUID (CLSID_FormatRAR, 0x8CD19CED, 0xC1B1, 0x4CC0, 0x88, 0x5B, 0x4F, 0x37, 0xFB, 0x64, 0xA2, 0x9F);

PluginStartupInfo Info;
FARSTANDARDFUNCTIONS FSF;

RarModule *pModule;

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

	pModule = new RarModule (NULL);

	return NAERROR_SUCCESS;
}

int OnFinalize ()
{
	delete pModule;

	return NAERROR_SUCCESS;
}

extern const GUID CLSID_FormatRAR;

int OnQueryArchive (QueryArchiveStruct *pQAS)
{
	RAROpenArchiveDataEx arcData;

	if ( pModule->m_hModule )
	{
		memset (&arcData, 0, sizeof (arcData));

		UNICODE_NAME(arcData.ArcName) = (TCHAR*)pQAS->lpFileName;
		arcData.OpenMode = RAR_OM_LIST;

		HANDLE hArchive = pModule->m_pfnOpenArchiveEx (&arcData);

		if ( !arcData.OpenResult )
		{
			pModule->m_pfnCloseArchive (hArchive);
			pQAS->hResult = (HANDLE)new RarArchive (pModule, pQAS->lpFileName);
			pQAS->uid = CLSID_FormatRAR;

			pQAS->dwFlags = QUERY_FLAG_UID_READY;

			return NAERROR_SUCCESS;
		}
	}

	return NAERROR_INTERNAL;
}

int OnOpenArchive (OpenArchiveStruct *pOAS)
{
	RarArchive *pArchive = (RarArchive*)pOAS->hArchive;

	pOAS->bResult = pArchive->OpenArchive (pOAS->nMode, pOAS->hCallback, pOAS->pfnCallback);

	return NAERROR_SUCCESS;
}

int OnCloseArchive (CloseArchiveStruct *pCAS)
{
	RarArchive *pArchive = (RarArchive*)pCAS->hArchive;

	pArchive->CloseArchive ();

	return NAERROR_SUCCESS;
}

int OnFinalizeArchive (RarArchive *pArchive)
{
	delete pArchive;

	return NAERROR_SUCCESS;
}

int OnGetArchiveModuleInfo (
		ArchiveModuleInfo *ai
		)
{
	static ArchiveFormatInfo formatInfo;

	formatInfo.uid = CLSID_FormatRAR;
	formatInfo.dwFlags = AFF_SUPPORT_INTERNAL_EXTRACT;
	formatInfo.lpName = _T("RAR Archive");
	formatInfo.lpDefaultExtention = _T("rar");

	ai->uid = CLSID_ModuleRAR;
	ai->nFormats = 1;
	ai->pFormatInfo = (ArchiveFormatInfo*)&formatInfo;
	ai->dwFlags = AMF_SUPPORT_SINGLE_FORMAT_QUERY; //we just dont care, only one format


	return NAERROR_SUCCESS;
}

int OnGetArchiveItem (GetArchiveItemStruct *pGAI)
{
	RarArchive *pArchive = (RarArchive*)pGAI->hArchive;

	pGAI->nResult = pArchive->GetArchiveItem (pGAI->pItem);

	return NAERROR_SUCCESS;
}

bool OnFreeArchiveItem(FreeArchiveItemStruct *pFAI)
{
	RarArchive* pArchive = (RarArchive*)pFAI->hArchive;
	pFAI->bResult = pArchive->FreeArchiveItem(pFAI->pItem);

	return NAERROR_SUCCESS;
}



int OnExtract (ExtractStruct *pES)
{
	RarArchive *pArchive = (RarArchive*)pES->hArchive;

	pES->bResult = pArchive->Extract (
			pES->pItems,
			pES->nItemsNumber,
			pES->lpDestPath,
			pES->lpCurrentPath
			);

	return NAERROR_SUCCESS;
}


int OnTest (TestStruct *pTS)
{
	RarArchive *pArchive = (RarArchive*)pTS->hArchive;

	pTS->bResult = pArchive->Test (
			pTS->pItems,
			pTS->nItemsNumber
			);

	return NAERROR_SUCCESS;
}

int OnGetDefaultCommand (GetDefaultCommandStruct *pGDC)
{
    static const TCHAR *pCommands[]={
    /*Extract               */_T("rar x {-p%%P} {-ap%%R} -y -c- -kb -- %%A @%%LNM"),
    /*Extract without paths */_T("rar e {-p%%P} -y -c- -kb -- %%A @%%LNM"),
    /*Test                  */_T("rar t -y {-p%%P} -- %%A"),
    /*Delete                */_T("rar d -y {-p%%P} {-w%%W} -- %%A @%%LNM"),
    /*Comment archive       */_T("rar c -y {-w%%W} -- %%A"),
    /*Comment files         */_T("rar cf -y {-w%%W} -- %%A @%%LNM"),
    /*Convert to SFX        */_T("rar s -y -- %%A"),
    /*Lock archive          */_T("rar k -y -- %%A"),
    /*Protect archive       */_T("rar rr -y -- %%A"),
    /*Recover archive       */_T("rar r -y -- %%A"),
    /*Add files             */_T("rar a -y {-p%%P} {-ap%%R} {-w%%W} {%%S} -- %%A @%%LNM")
    };


	if ( pGDC->uid == CLSID_FormatRAR )
	{
		_tcscpy (pGDC->lpCommand, pCommands[pGDC->nCommand]); //BUGBUG
		pGDC->bResult = true;
	}
	else
		pGDC->bResult = false;

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
		return OnOpenArchive ((OpenArchiveStruct*)pParams);

	case FID_CLOSEARCHIVE:
		return OnCloseArchive ((CloseArchiveStruct*)pParams);

	case FID_FINALIZEARCHIVE:
		return OnFinalizeArchive ((RarArchive*)pParams);

	case FID_GETARCHIVEMODULEINFO:
		return OnGetArchiveModuleInfo ((ArchiveModuleInfo*)pParams);

	case FID_GETARCHIVEITEM:
		return OnGetArchiveItem ((GetArchiveItemStruct*)pParams);

	case FID_FREEARCHIVEITEM:
		return OnFreeArchiveItem((FreeArchiveItemStruct*)pParams);

	case FID_EXTRACT:
		return OnExtract ((ExtractStruct*)pParams);

	case FID_TEST:
		return OnTest ((TestStruct*)pParams);

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
