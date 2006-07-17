#include "7z.h"
#include "Guid.h"

SevenZipModule::SevenZipModule (const char *lpFileName)
{
	char *lpModuleName = StrDuplicate(Info.ModuleName, 260);

	CutToSlash(lpModuleName);
    strcat (lpModuleName, "7zxa.dll");

	m_hModule = LoadLibraryEx (
			lpModuleName,
			NULL,
			LOAD_WITH_ALTERED_SEARCH_PATH
			);

	StrFree (lpModuleName);

	if ( m_hModule )
		m_pfnCreateObject = (CREATEOBJECT)GetProcAddress (m_hModule, "CreateObject");
}


SevenZipModule::~SevenZipModule()
{
	FreeLibrary (m_hModule);
}


SevenZipArchive::SevenZipArchive (SevenZipModule *pModule, const char *lpFileName)
{
	m_pArchive = NULL;
	m_pInFile = NULL;

	m_pModule = pModule;
	m_lpFileName = StrDuplicate (lpFileName);
}

SevenZipArchive::~SevenZipArchive ()
{
	StrFree (m_lpFileName);
}

bool __stdcall SevenZipArchive::pOpenArchive (
		int nOpMode,
		ARCHIVECALLBACK pfnCallback
		)
{
	m_pfnCallback = pfnCallback;

	m_pInFile = new CInFile;

	if ( m_pInFile->Open (m_lpFileName) )
	{
		if ( SUCCEEDED (m_pModule->m_pfnCreateObject (
				&CLSID_CFormat7z,
				&IID_IInArchive,
				(void**)&m_pArchive
				)) )
		{
			unsigned __int64 max = 1024;

			if ( SUCCEEDED (m_pArchive->Open (m_pInFile, &max, 0)) )
			{
				m_pArchive->GetNumberOfItems((unsigned int*)&m_nItemsNumber);

				if ( m_nItemsNumber )
				{
					m_nItemsNumber--;

					return true;
				}

				m_pArchive->Close();
			}

			//if we get here, there is an error

			m_pArchive->Release ();
		}

		m_pInFile->Release ();
	}

	return false;
}

void __stdcall SevenZipArchive::pCloseArchive ()
{
	if ( m_pArchive )
	{
		m_pArchive->Close ();
		m_pArchive->Release ();
		m_pArchive = NULL;
	}

	if ( m_pInFile )
	{
		m_pInFile->Release (); //???
		m_pInFile = NULL;
	}
}

unsigned __int64 VariantToInt64 (const PROPVARIANT &value)
{
	switch ( value.vt )
	{
		case VT_UI1:
			return value.bVal;
		case VT_UI2:
			return value.uiVal;
		case VT_UI4:
			return value.ulVal;
		case VT_UI8:
			return (unsigned __int64)value.uhVal.QuadPart;
		default:
			return 0;
	}
}


int __stdcall SevenZipArchive::pGetArchiveItem (
		ArchiveItemInfo *pItem
		)
{
	if ( (int)m_nItemsNumber == -1)
		return E_EOF;

	int nResult = E_BROKEN;

	PROPVARIANT value;

	VariantInit ((VARIANTARG*)&value);

	if ( SUCCEEDED (m_pArchive->GetProperty (
			m_nItemsNumber,
			kpidPath,
			&value
			)) )
	{
		WideCharToMultiByte (CP_OEMCP, 0, value.bstrVal, -1, pItem->pi.FindData.cFileName, sizeof (pItem->pi.FindData.cFileName), NULL, NULL);

		VariantClear ((VARIANTARG*)&value);
		VariantInit ((VARIANTARG*)&value);

		if ( SUCCEEDED (m_pArchive->GetProperty (
				m_nItemsNumber,
				kpidAttributes,
				&value
				)) )
		{
			if ( value.vt == VT_UI4 )
		        pItem->pi.FindData.dwFileAttributes = value.ulVal;
		}

		if ( SUCCEEDED (m_pArchive->GetProperty (
				m_nItemsNumber,
				kpidIsFolder,
				&value
				)) )
		{
			if ( value.vt == VT_BOOL )
			{
				if ( value.boolVal == VARIANT_TRUE )
					pItem->pi.FindData.dwFileAttributes |= FILE_ATTRIBUTE_DIRECTORY;
			}
		}

		if ( SUCCEEDED (m_pArchive->GetProperty(
				m_nItemsNumber,
				kpidSize,
				&value
				)) )
		{
			unsigned __int64 size = VariantToInt64 (value);

			pItem->pi.FindData.nFileSizeLow = (DWORD)size;
			pItem->pi.FindData.nFileSizeHigh = (DWORD)(size >> 32);
		}

		pItem->pi.UserData = m_nItemsNumber;

		nResult = E_SUCCESS;
	}

	m_nItemsNumber--;

	return nResult;
}


bool __stdcall SevenZipArchive::pTest (
		const PluginPanelItem *pItems,
		int nItemsNumber
		)
{
	return true;
}

/*
int GetIndex (IInArchive *pArchive, const char *lpFileName)
{
	unsigned int dwIndex = 0;
	PROPVARIANT value;
	char szArcFileName[NM];

	pArchive->GetNumberOfItems((unsigned int*)&dwIndex);

	for (int i = 0; i < dwIndex; i++)
	{
		VariantInit ((VARIANTARG*)&value);

		if ( SUCCEEDED (pArchive->GetProperty (
				i,
				kpidPath,
				&value
				)) )
		{
			WideCharToMultiByte (CP_OEMCP, 0, value.bstrVal, -1, szArcFileName, sizeof (szArcFileName), NULL, NULL);

            if ( !FSF.LStricmp (
            		lpFileName,
            		szArcFileName
            		) )
			{
				VariantClear ((VARIANTARG*)&value);
				return i;
			}
		}

		VariantClear ((VARIANTARG*)&value);
	}

	return -1;
}
*/

int __cdecl compare(const void *p1, const void *p2)
{
	int i1 = *(int*)p1;
	int i2 = *(int*)p2;

	if ( i1 > i2 )
		return 1;

	if ( i1 < i2 )
		return -1;

	return 0;
}


bool __stdcall SevenZipArchive::pExtract (
		const PluginPanelItem *pItems,
		int nItemsNumber,
		const char *lpDestPath,
		const char *lpCurrentFolder
		)
{
	bool bResult = false;
	unsigned int *indices = (unsigned int*)malloc (nItemsNumber*sizeof (unsigned int));
	ArchiveItem *items = (ArchiveItem*)malloc (nItemsNumber*sizeof (ArchiveItem));

	for (int i = 0; i < nItemsNumber; i++)
	{
		indices[i] = pItems[i].UserData; //GetIndex (m_pArchive, pItems[i].FindData.cFileName);

		items[i].nIndex = indices[i];
		items[i].pItem = &pItems[i];
	}

	FSF.qsort (indices, nItemsNumber, 4, compare);

	//unsigned int nItems = 0;
	//m_pArchive->GetNumberOfItems((unsigned int*)&nItems);

	CArchiveExtractCallback *pCallback = new CArchiveExtractCallback (this, items, nItemsNumber, lpDestPath, lpCurrentFolder);

	if ( m_pArchive->Extract(
			indices,
			(unsigned int)nItemsNumber,
			0,
			pCallback
			) == S_OK )
		bResult = true;

	delete pCallback;
	free (indices);
	free (items);

	return bResult;
}
