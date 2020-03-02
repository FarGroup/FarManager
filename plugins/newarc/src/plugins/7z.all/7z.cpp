#include "7z.h"
#include <array.hpp>
#include <FarDialogs.hpp>

PluginStartupInfo Info;
FARSTANDARDFUNCTIONS FSF;

pointer_array <SevenZipModule*> modules;
ArchiveFormatInfo *pFormatInfo = NULL;

struct ArchiveModuleInformation {

	int nTypes;
	char *pTypeNames;

	int nConfigStringsNumber;

	char **pConfigStrings;
};

const SevenZipModule *GetModuleFromGUID (const GUID &uid, unsigned int *formatIndex)
{
	for (int i = 0; i < modules.count(); i++)
	{
		SevenZipModule *pModule = modules[i];

		for (unsigned int j = 0; j < pModule->m_nNumberOfFormats; j++)
		{
			if ( pModule->m_pInfo[j].uid == uid )
			{
				if ( formatIndex )
					*formatIndex = j;

				return pModule;
			}
		}
	}

	return NULL;
}

int OnInitialize (StartupInfo *pInfo)
{
	Info = pInfo->Info;
	FSF = *pInfo->Info.FSF;

	modules.create (ARRAY_OPTIONS_DELETE);

	WIN32_FIND_DATA fdata;
	char *lpMask = StrDuplicate (Info.ModuleName, 260);

	CutToSlash (lpMask);
	strcat (lpMask, "Formats\\*.dll");

	HANDLE hSearch = FindFirstFile (lpMask, &fdata);

	if ( hSearch != INVALID_HANDLE_VALUE )
	{
		do {

			if ( !OptionIsOn (fdata.dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY) )
			{
				SevenZipModule *pModule = new SevenZipModule;

				char *lpModuleName = StrDuplicate (Info.ModuleName, 260);
				CutToSlash(lpModuleName);

				strcat (lpModuleName, "Formats\\");
				strcat (lpModuleName, fdata.cFileName);

				if ( pModule->Initialize (lpModuleName) )
					modules.add (pModule);
				else
					delete pModule;

				free (lpModuleName);
			}
		} while ( FindNextFile (hSearch, &fdata) );

		FindClose (hSearch);
	}

	pFormatInfo = NULL;

	return NAERROR_SUCCESS;
}

int OnFinalize ()
{
	modules.free ();
	free (pFormatInfo);

	return NAERROR_SUCCESS;
}

extern int FindFormats (const char *lpFileName, pointer_array<FormatPosition*> &formats);

int __cdecl SortFormats (
    FormatPosition *pos1,
    FormatPosition *pos2,
    void *pParam
    )
{
	if ( pos1->position > pos2->position )
		return 1;

	if ( pos1->position < pos2->position )
		return -1;

	//if ( pos1->position == pos2->position )
	return 0;
}

static pointer_array<FormatPosition*> *formats;
static char *lpFileName=NULL;

int OnQueryArchive (QueryArchiveStruct *pQAS)
{
	formats = new pointer_array<FormatPosition*>(ARRAY_OPTIONS_DELETE);

	FindFormats (pQAS->lpFileName, *formats);

	formats->sort ((void *)SortFormats, NULL);

	lpFileName = StrDuplicate(pQAS->lpFileName);

	pQAS->nFormats = formats->count();
	pQAS->hResult = NULL;

	return NAERROR_SUCCESS;
}

int OnQueryArchiveFormat (QueryArchiveFormatStruct *pQAFS)
{
	if (pQAFS->nFormat < 0 || pQAFS->nFormat >= formats->count())
		return NAERROR_INTERNAL;

	FormatPosition *pos = formats->at(pQAFS->nFormat);

	for (int i = 0; i < modules.count (); i++)
	{
		SevenZipModule *pModule = modules[i];

		for (unsigned int k = 0; k < pModule->m_nNumberOfFormats; k++)
		{
			if ( IsEqualGUID (pModule->m_pInfo[k].uid, *pos->puid) )
			{
				SevenZipArchive *pArchive = new SevenZipArchive (pModule, k, lpFileName, false);

				pQAFS->hResult = (HANDLE)pArchive;

				return NAERROR_SUCCESS;
			}
		}
	}

	return NAERROR_INTERNAL;
}

int OnQueryArchiveEnd()
{
	delete formats;
	StrDelete(lpFileName);
	lpFileName=NULL;
	return NAERROR_SUCCESS;
}

int OnCreateArchive (CreateArchiveStruct *pCAS)
{
	unsigned int formatIndex = 0;
	const SevenZipModule *pModule = GetModuleFromGUID (pCAS->uid, &formatIndex);

	if ( pModule )
	{
		SevenZipArchive *pArchive = new SevenZipArchive (pModule, formatIndex, pCAS->lpFileName, true);

		pCAS->hResult = (HANDLE)pArchive;

		return NAERROR_SUCCESS;
	}

	return NAERROR_INTERNAL;
}

int OnOpenArchive (OpenArchiveStruct *pOAS)
{
	SevenZipArchive *pArchive = (SevenZipArchive*)pOAS->hArchive;

	pOAS->bResult = pArchive->pOpenArchive (pOAS->nMode, pOAS->pfnCallback);

	return NAERROR_SUCCESS;
}

int OnCloseArchive (CloseArchiveStruct *pCAS)
{
/*	SevenZipArchive *pArchive = (SevenZipArchive*)pCAS->hArchive;

	pArchive->pCloseArchive ();*/

	return NAERROR_SUCCESS;
}

int OnFinalizeArchive (SevenZipArchive *pArchive)
{
	pArchive->pCloseArchive ();

	delete pArchive;

	return NAERROR_SUCCESS;
}

int OnGetArchivePluginInfo (
		ArchivePluginInfo *ai
		)
{
	int nCount = 0;

	for (int i = 0; i < modules.count (); i++)
		nCount += modules[i]->m_nNumberOfFormats;

	pFormatInfo = (ArchiveFormatInfo*)realloc (pFormatInfo, nCount*sizeof (ArchiveFormatInfo));

	int index = 0;

	for (int i = 0; i < modules.count (); i++)
	{
		SevenZipModule *pModule = modules[i];

		for (unsigned int j = 0; j < pModule->m_nNumberOfFormats; j++)
		{
			pModule->GetArchiveFormatInfo (j, &pFormatInfo[index]);
			index++;
		}
	}


	ai->nFormats = nCount;
	ai->pFormatInfo = pFormatInfo;

	return NAERROR_SUCCESS;
}

int OnGetArchiveItem (GetArchiveItemStruct *pGAI)
{
	SevenZipArchive *pArchive = (SevenZipArchive *)pGAI->hArchive;

	pGAI->nResult = pArchive->pGetArchiveItem (pGAI->pItem);

	return NAERROR_SUCCESS;
}

int OnGetArchiveFormat (GetArchiveFormatStruct *pGAF)
{
	SevenZipArchive *pArchive = (SevenZipArchive *)pGAF->hArchive;
	pGAF->uid = pArchive->m_pModule->m_pInfo[pArchive->m_nFormatIndex].uid;

	return NAERROR_SUCCESS;
}

int OnExtract (ExtractStruct *pES)
{
	SevenZipArchive *pArchive = (SevenZipArchive *)pES->hArchive;

	pES->bResult = pArchive->pExtract (
			pES->pItems,
			pES->nItemsNumber,
			pES->lpDestPath,
			pES->lpCurrentPath
			);

	return NAERROR_SUCCESS;
}


int OnTest (TestStruct *pTS)
{
	SevenZipArchive *pArchive = (SevenZipArchive *)pTS->hArchive;

	pTS->bResult = pArchive->pTest (
			pTS->pItems,
			pTS->nItemsNumber
			);

	return NAERROR_SUCCESS;
}

int OnAdd (AddStruct *pAS)
{
	SevenZipArchive *pArchive = (SevenZipArchive *)pAS->hArchive;

	pAS->bResult = pArchive->pAddFiles (
			pAS->lpSourcePath,
			pAS->lpCurrentPath,
			pAS->pItems,
			pAS->nItemsNumber
			);

	return NAERROR_SUCCESS;
}


int OnGetDefaultCommand (GetDefaultCommandStruct *pGDC)
{
	pGDC->bResult = GetFormatCommand (pGDC->uid, pGDC->nCommand, pGDC->lpCommand);
	return NAERROR_SUCCESS;
}

int OnDelete (DeleteStruct *pDS)
{
	SevenZipArchive *pArchive = (SevenZipArchive *)pDS->hArchive;

	pDS->bResult = pArchive->pDelete (
			pDS->pItems,
			pDS->nItemsNumber
			);

	return NAERROR_SUCCESS;
}

int OnNotify (NotifyStruct *pNS)
{
	SevenZipArchive *pArchive = (SevenZipArchive *)pNS->hArchive;

	pArchive->pNotify (
			pNS->nEvent,
			pNS->pEventData
			);

	return NAERROR_SUCCESS;
}

extern "C" const GUID CLSID_CFormat7z;

const char *SevenZipLevel[] = {"None", "Fastest", "Fast", "Normal", "Maximum", "Ultra"};
const char *SevenZipLevelStrings[] = {"X0", "X1", "X3", "X5", "X7", "X9"};

const char *SevenZipLZMADictionary[] = {"64 KB", "1 MB", "2 MB", "3 MB", "4 MB", "6 MB", "8 MB", "12 MB", "16 MB", "24 MB", "32 MB", "48 MB", "64 MB"};
const char *SevenZipLZMADictionaryStrings[] = {"64K", "1M", "2M", "3M", "4M", "6M", "8M", "12M", "16M", "24M", "32M", "48M", "64M"};
const char *SevenZipLZMAWordSize[] = {"8", "12", "16", "24", "32", "48", "64", "96", "128", "192", "256", "273"};
const char *SevenZipLZMASBSize[] = {"Non solid", "1 MB", "2 MB", "4 MB", "8 MB", "16 MB", "32 MB", "64 MB", "128 MB", "256 MB", "512 MB", "1 GB", "2 GB", "4 GB", "8 GB", "16 GB", "32 GB", "64 GB", "Solid"};
const char *SevenZipLZMASBSizeStrings[] = {"???", "E1M", "E2M", "E4M", "E8M", "E16M", "E32M", "E64M", "E128M", "E256M", "E512M", "E1G", "E2G", "E4G", "E8G", "E16G", "E32G", "E64G", "???"};

#define countof(x) (sizeof(x)/sizeof(x[0]))

LONG_PTR __stdcall hndConfigureFormat(FarDialogHandler *D, int nMsg, int Param1, LONG_PTR Param2)
{
	if ( nMsg == DN_INITDIALOG )
	{
		D->ListAddStr (2, "LZMA");
		D->ListAddStr (2, "PPMd");
		D->ListAddStr (2, "BZip2");

		for (unsigned int i = 0; i < countof(SevenZipLevel); i++)
			D->ListAddStr (4, SevenZipLevel[i]);

		for (unsigned int i = 0; i < countof(SevenZipLZMADictionary); i++)
			D->ListAddStr (6, SevenZipLZMADictionary[i]);

		for (unsigned int i = 0; i < countof(SevenZipLZMASBSize); i++)
			D->ListAddStr (8, SevenZipLZMASBSize[i]);

		for (unsigned int i = 0; i < countof(SevenZipLZMAWordSize); i++)
			D->ListAddStr (10, SevenZipLZMAWordSize[i]);
	}

	return D->DefDlgProc (nMsg, Param1, Param2);
}

int OnConfigureFormat (ConfigureFormatStruct *pCF)
{
//	if ( pCF->uid == CLSID_CFormat7z )
	{
		FarDialog D(-1, -1, 60, 20);

		D.DoubleBox (3, 1, 56, 18, "7z config"); //0

		D.Text(5, 2, "Method:"); //1
		D.ComboBox (25, 2, 20, NULL); //2
		D.SetFlags(DIF_DROPDOWNLIST);

		D.Text(5, 3, "Level:"); //3
		D.ComboBox (25, 3, 20, NULL); //4
		D.SetFlags(DIF_DROPDOWNLIST);

		D.Text(5, 4, "Dict. size:"); //5
		D.ComboBox (25, 4, 20, NULL); //6
		D.SetFlags(DIF_DROPDOWNLIST);

		D.Text(5, 5, "Solid block size:"); //7
		D.ComboBox (25, 5, 20, NULL); //8
		D.SetFlags(DIF_DROPDOWNLIST);

		D.Text(5, 6, "Fast bytes:"); //9
		D.ComboBox (25, 6, 20, NULL); //10
		D.SetFlags(DIF_DROPDOWNLIST);

		D.Separator(7);

		D.CheckBox(5, 8, false, "Compress headers");
		D.CheckBox(5, 9, false, "Compress headers full");
		D.CheckBox(5, 10, false, "Encrypt headers");

		D.Separator(11);

		D.Button (-1, 12, "Ok");
		D.Button (-1, 12, "Cancel");

		D.ShowEx (hndConfigureFormat);
	}

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

	case FID_QUERYARCHIVEFORMAT:
		return OnQueryArchiveFormat ((QueryArchiveFormatStruct*)pParams);

	case FID_QUERYARCHIVEEND:
		return OnQueryArchiveEnd ();

	case FID_OPENARCHIVE:
		return OnOpenArchive ((OpenArchiveStruct*)pParams);

	case FID_CLOSEARCHIVE:
		return OnCloseArchive ((CloseArchiveStruct*)pParams);

	case FID_FINALIZEARCHIVE:
		return OnFinalizeArchive ((SevenZipArchive *)pParams);

	case FID_GETARCHIVEPLUGININFO:
		return OnGetArchivePluginInfo ((ArchivePluginInfo*)pParams);

	case FID_GETARCHIVEITEM:
		return OnGetArchiveItem ((GetArchiveItemStruct*)pParams);

	case FID_GETARCHIVEFORMAT:
		return OnGetArchiveFormat ((GetArchiveFormatStruct*)pParams);

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
		return OnCreateArchive ((CreateArchiveStruct*)pParams);

	case FID_NOTIFY:
		return OnNotify ((NotifyStruct*)pParams);

	case FID_CONFIGUREFORMAT:
		return OnConfigureFormat ((ConfigureFormatStruct*)pParams);
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
