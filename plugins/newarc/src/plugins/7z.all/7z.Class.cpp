#include "7z.h"
#include "Guid.h"
#include "commands.h"
#include "detectarchive.h"
#include <array.hpp>
#include <debug.h>

const GUID FormatGUIDs[] = {
		CLSID_CFormat7z,
		CLSID_CArjHandler,
		CLSID_CCabHandler,
		CLSID_CChmHandler,
		CLSID_CDebHandler,
		CLSID_CIsoHandler,
		CLSID_CLzhHandler,
		CLSID_CNsisHandler,
		CLSID_CRarHandler,
		CLSID_CRpmHandler,
		CLSID_CSplitHandler,
		CLSID_CTarHandler,
		CLSID_CZipHandler,
		CLSID_CGZipHandler,
		CLSID_CZHandler,
		CLSID_CBZip2Handler,
		CLSID_CCpioHandler,
};


const unsigned char SevenZipSig[] = {'7' , 'z', 0xBC, 0xAF, 0x27, 0x1C};
const unsigned char ZipSig[]      = {0x50, 0x4B, 0x03, 0x04};
const unsigned char BZipSig[]     = {'B' , 'Z', 'h'};
const unsigned char GZipSig[]     = {0x1F, 0x8B};
const unsigned char ArjSig[]      = {0x60, 0xEA};
const unsigned char ZSig[]        = {0x1F, 0x9D};
const unsigned char RarSig[]      = {'R', 'a', 'r', '!'};
const unsigned char CabSig[]      = {'M', 'S', 'C', 'F'};
const unsigned char RpmSig[]      = {0xED, 0xAB, 0xEE, 0xDB};
const unsigned char DebSig[]      = {'!', '<', 'a', 'r', 'c', 'h', '>', 0x0A, 'd', 'e', 'b', 'i', 'a', 'n', '-', 'b', 'i', 'n', 'a', 'r', 'y'};
const unsigned char CpioSig[]     = {'0', '7', '0', '7', '0'}; //BUG BUG: ўа®¤Ґ ­Ґ б®ўбҐ¬ в®з­®
const unsigned char ChmSig[]      = {'I', 'T', 'S', 'F'};
const unsigned char NsisSig[]     = {0xEF, 0xBE, 0xAD, 0xDE, 0x4E, 0x75, 0x6C, 0x6C, 0x73, 0x6F, 0x66, 0x74, 0x49, 0x6E, 0x73, 0x74};
const unsigned char IsoSig[]      = {'C', 'D', '0', '0', '1', 0x1};

struct FormatInfo {
	const GUID *puid;
	const unsigned char *psig;
	DWORD size;
	bool bPosZero;
	DETECTARCHIVE pDetectArchive;
};

const FormatInfo signs[] = {
	{&CLSID_CFormat7z,     (const unsigned char *)&SevenZipSig, 6, false, Is7zHeader},
	{&CLSID_CRarHandler,   (const unsigned char *)&RarSig,      4, false, IsRarHeader},
	{&CLSID_CZipHandler,   (const unsigned char *)&ZipSig,      4, false, IsZipHeader},
	{&CLSID_CRpmHandler,   (const unsigned char *)&RpmSig,      4, true,  NULL},
	{&CLSID_CDebHandler,   (const unsigned char *)&DebSig,     21, true,  NULL},
	{&CLSID_CCabHandler,   (const unsigned char *)&CabSig,      4, false, IsCabHeader},
	{&CLSID_CBZip2Handler, (const unsigned char *)&BZipSig,     3, true,  NULL},
	{&CLSID_CArjHandler,   (const unsigned char *)&ArjSig,      2, false, IsArjHeader},
	{&CLSID_CTarHandler,   NULL,                                0, true,  IsTarHeader},
	{&CLSID_CGZipHandler,  (const unsigned char *)&GZipSig,     2, true,  NULL},
	{&CLSID_CZHandler,     (const unsigned char *)&ZSig,        2, true,  NULL},
	{&CLSID_CLzhHandler,   NULL,                                0, false, IsLzhHeader},
	{&CLSID_CCpioHandler,  (const unsigned char *)&CpioSig,     5, true,  NULL},
	{&CLSID_CChmHandler,   (const unsigned char *)&ChmSig,      4, true,  NULL},
	{&CLSID_CNsisHandler,  (const unsigned char *)&NsisSig,    16, false, IsNSISHeader},
	{&CLSID_CIsoHandler,   (const unsigned char *)&IsoSig,      6, true,  IsIsoHeader},
};



bool GetFormatCommand(const GUID &guid, int nCommand, char *lpCommand)
{
	if ( IsEqualGUID(guid, CLSID_CFormat7z) )
		strcpy (lpCommand, p7Z[nCommand]);
	else if ( IsEqualGUID(guid, CLSID_CRarHandler) )
		strcpy (lpCommand, pRAR[nCommand]);
	else if ( IsEqualGUID(guid, CLSID_CZipHandler) )
		strcpy (lpCommand, pZIP[nCommand]);
	else if ( IsEqualGUID(guid, CLSID_CArjHandler) )
		strcpy (lpCommand, pARJ[nCommand]);
	else if ( IsEqualGUID(guid, CLSID_CTarHandler) )
		strcpy (lpCommand, pTAR[nCommand]);
	else if ( IsEqualGUID(guid, CLSID_CGZipHandler) )
		strcpy (lpCommand, pGZIP[nCommand]);
	else if ( IsEqualGUID(guid, CLSID_CBZip2Handler) )
		strcpy (lpCommand, pBZIP[nCommand]);
	else if ( IsEqualGUID(guid, CLSID_CZHandler) )
		strcpy (lpCommand, pZ[nCommand]);
	else if ( IsEqualGUID(guid, CLSID_CCabHandler) )
		strcpy (lpCommand, pCAB[nCommand]);
	else if ( IsEqualGUID(guid, CLSID_CLzhHandler) )
		strcpy (lpCommand, pLZH[nCommand]);
	else if ( IsEqualGUID(guid, CLSID_CCpioHandler) )
		strcpy (lpCommand, pCPIO[nCommand]);
	else if ( IsEqualGUID(guid, CLSID_CRpmHandler) )
		strcpy (lpCommand, pRPM[nCommand]);
	else if ( IsEqualGUID(guid, CLSID_CDebHandler) )
		strcpy (lpCommand, pDEB[nCommand]);
/*
	else if ( IsEqualGUID(guid, CLSID_CChmHandler) )
		strcpy (lpCommand, "");
	else if ( IsEqualGUID(guid, CLSID_CSplitHandler) )
		strcpy (lpCommand, "");
	else if ( IsEqualGUID(guid, CLSID_CNsisHandler) )
		strcpy (lpCommand, "");
	else if ( IsEqualGUID(guid, CLSID_CIsoHandler) )
		strcpy (lpCommand, "");
*/
	else
		return false;

	return true;
}


int FindFormats (const char *lpFileName, pointer_array<FormatPosition*> &formats)
{
	HANDLE hFile = CreateFile (
			lpFileName,
			GENERIC_READ,
			FILE_SHARE_READ,
			0,
			OPEN_EXISTING,
			FILE_FLAG_SEQUENTIAL_SCAN,
			NULL
			);

	if ( hFile != INVALID_HANDLE_VALUE )
	{
		unsigned char *buffer = (unsigned char*)malloc (1 << 17);
		DWORD dwRead;

		if ( ReadFile (hFile, buffer, 1 << 17, &dwRead, NULL) )
		{
			for (size_t j = 0; j < sizeof(signs)/sizeof(signs[0]); j++)
			{
				const FormatInfo *info = &signs[j];

				if ( info->pDetectArchive)
				{
					int position = info->pDetectArchive(buffer, dwRead);

					if ( position != -1 )
					{
						if ( !info->bPosZero || (position == 0) )
						{
							FormatPosition *pos = new FormatPosition;

							pos->puid = info->puid;
							pos->position = position;

							formats.add (pos);
						}
					}
				}
				else
				{
					if ( dwRead >= info->size )
					{
						for (size_t i = 0; i <= dwRead-info->size; i++)
						{
							if ( !memcmp (buffer+i, info->psig, info->size) )
							{
								FormatPosition *pos = new FormatPosition;

								pos->puid = info->puid;
								pos->position = i;

								formats.add (pos);
								break;
							}
							if (info->bPosZero)
								break;
						}
					}
				}
			}

			if ( strstr(lpFileName, ".") && strlen(lpFileName) > 2 )
			{
				bool bMatch = false;
				int len = strlen(lpFileName);

				if ( lpFileName[len-2] == '0' && lpFileName[len-1] == '1' )
				{
					int i=len-3;
					for (; i>0; i--)
						if ( lpFileName[i] != '0' )
							break;
					if ( lpFileName[i] == '.' )
						bMatch = true;
				}
				else if ( (lpFileName[len-1] == 'A' || lpFileName[len-1] == 'a') &&
						  (lpFileName[len-2] == 'A' || lpFileName[len-2] == 'a') )
					bMatch = true;

				if ( bMatch )
				{
					FormatPosition *pos = new FormatPosition;

					pos->puid = &CLSID_CSplitHandler;
					pos->position = 0;

					formats.add (pos);
				}
			}
		}

		free (buffer);

		CloseHandle (hFile);
	}

	return formats.count();
}



bool SevenZipModule::Initialize (const char *lpFileName)
{
	bool bResult = false;

	m_puids = NULL;
	m_hModule = LoadLibraryEx (
			lpFileName,
			NULL,
			LOAD_WITH_ALTERED_SEARCH_PATH
			);

	if ( m_hModule )
	{
		m_pfnCreateObject = (CREATEOBJECT)GetProcAddress (m_hModule, "CreateObject");
		m_pfnGetHandlerProperty = (GETHANDLERPROPERTY)GetProcAddress (m_hModule, "GetHandlerProperty");
		m_pfnGetHandlerProperty2 = (GETHANDLERPROPERTY2)GetProcAddress (m_hModule, "GetHandlerProperty2");
		m_pfnGetNumberOfFormats = (GETNUMBEROFFORMATS)GetProcAddress (m_hModule, "GetNumberOfFormats");

		if ( m_pfnCreateObject &&
			 (m_pfnGetHandlerProperty || (m_pfnGetHandlerProperty2 && m_pfnGetNumberOfFormats)) )
		{
			if ( m_pfnGetHandlerProperty2 && m_pfnGetNumberOfFormats )
			{
				if ( m_pfnGetNumberOfFormats (&m_nNumberOfFormats) == S_OK )
				{
					m_puids = (GUID*)malloc (m_nNumberOfFormats*sizeof (GUID));

					CPropVariant value;

					bResult = true;

					for (int i = 0; i < m_nNumberOfFormats; i++)
					{
						if ( (m_pfnGetHandlerProperty2 (i, NArchive::kClassID, &value) != S_OK) ||
							 (value.vt != VT_BSTR) )
						{
							bResult = false;
							break;
						}

						memcpy (&m_puids[i], value.bstrVal, sizeof (GUID));
					}
				}
			}
			else
			{
				m_nNumberOfFormats= 1;

				m_puids = (GUID*)malloc (m_nNumberOfFormats*sizeof (GUID));

				CPropVariant value;

				if ( (m_pfnGetHandlerProperty (NArchive::kClassID, &value) == S_OK) && 
					 (value.vt == VT_BSTR) )
				{
					memcpy (&m_puids[0], value.bstrVal, sizeof (GUID));
					bResult = true;
				}
			}
		}
	}

	return bResult;
}


SevenZipModule::~SevenZipModule ()
{
	if ( m_puids )
		free (m_puids);

	FreeLibrary (m_hModule);
}

/*
bool SevenZipModule::HasSignature ()
{
	if ( IsEqualGUID (m_uid, CLSID_CSplitHandler) )
		return true;

	for (size_t i = 0; i < sizeof(signs)/sizeof(signs[0]); i++)
	{
    	if ( IsEqualGUID (m_uid, *(signs[i].puid)) )
    		return true;
	}

	return false;
}
*/


void SevenZipModule::GetArchiveFormatInfo (unsigned int nFormatIndex, ArchiveFormatInfo *pInfo)
{
	CPropVariant value;

	if ( m_pfnGetHandlerProperty2 )
		m_pfnGetHandlerProperty2 (nFormatIndex, NArchive::kUpdate, &value);
	else
		m_pfnGetHandlerProperty (NArchive::kUpdate, &value);

	GUID uid = m_puids[nFormatIndex];

	pInfo->dwFlags = AFF_SUPPORT_INTERNAL_EXTRACT|AFF_SUPPORT_INTERNAL_TEST;
	pInfo->uid = uid;

	if ( (value.vt == VT_BOOL) && (value.boolVal == VARIANT_TRUE) )
		pInfo->dwFlags |= (AFF_SUPPORT_INTERNAL_DELETE|AFF_SUPPORT_INTERNAL_ADD|AFF_SUPPORT_INTERNAL_CREATE);

	if ( IsEqualGUID (uid, CLSID_CFormat7z) )
	{
		pInfo->lpName = "7z archive [7z]";
		pInfo->lpDefaultExtention = "7z";
	}
	else

	if ( IsEqualGUID (uid, CLSID_CArjHandler) )
	{
		pInfo->lpName = "ARJ archive [7z]";
		pInfo->lpDefaultExtention = "arj";
	}
	else

	if ( IsEqualGUID (uid, CLSID_CBZip2Handler) )
	{
		pInfo->lpName = "BZip2 archive [7z]";
		pInfo->lpDefaultExtention = "bz2";
	}
	else

	if ( IsEqualGUID (uid, CLSID_CCabHandler) )
	{
		pInfo->lpName = "CAB archive [7z]";
		pInfo->lpDefaultExtention = "cab";
	}
	else
	if ( IsEqualGUID (uid, CLSID_CChmHandler) )
	{
		pInfo->lpName = "CHM archive [7z]";
		pInfo->lpDefaultExtention = "chm";
	}
	else
	if ( IsEqualGUID (uid, CLSID_CCpioHandler) )
	{
		pInfo->lpName = "Cpio archive [7z]";
		pInfo->lpDefaultExtention = "cpio";
	}
	else
	if ( IsEqualGUID (uid, CLSID_CDebHandler) )
	{
		pInfo->lpName = "Debian archive [7z]";
		pInfo->lpDefaultExtention = "deb";
	}
	else
	if ( IsEqualGUID (uid, CLSID_CGZipHandler) )
	{
		pInfo->lpName = "GZip archive [7z]";
		pInfo->lpDefaultExtention = "gz";
	}
	else
	if ( IsEqualGUID (uid, CLSID_CIsoHandler) )
	{
		pInfo->lpName = "ISO archive [7z]";
		pInfo->lpDefaultExtention = "iso";
	}
	else
	if ( IsEqualGUID (uid, CLSID_CLzhHandler) )
	{
		pInfo->lpName = "LZH archive [7z]";
		pInfo->lpDefaultExtention = "lzh";
	}
	else
	if ( IsEqualGUID (uid, CLSID_CNsisHandler) )
	{
		pInfo->lpName = "NSIS archive [7z]";
		pInfo->lpDefaultExtention = "exe";
	}
	else
	if ( IsEqualGUID (uid, CLSID_CRarHandler) )
	{
		pInfo->lpName = "RAR archive [7z]";
		pInfo->lpDefaultExtention = "rar";
	}
	else
	if ( IsEqualGUID (uid, CLSID_CRpmHandler) )
	{
		pInfo->lpName = "RPM archive [7z]";
		pInfo->lpDefaultExtention = "rpm";
	}
	else
	if ( IsEqualGUID (uid, CLSID_CSplitHandler) )
	{
		pInfo->lpName = "Split archive [7z]";
		pInfo->lpDefaultExtention = "001";
	}
	else
	if ( IsEqualGUID (uid, CLSID_CTarHandler) )
	{
		pInfo->lpName = "TAR archive [7z]";
		pInfo->lpDefaultExtention = "tar";
	}
	else
	if ( IsEqualGUID (uid, CLSID_CZHandler) )
	{
		pInfo->lpName = "Z archive [7z]";
		pInfo->lpDefaultExtention = "z";
	}
	else
	if ( IsEqualGUID (uid, CLSID_CZipHandler) )
	{
		pInfo->lpName = "ZIP archive [7z]";
		pInfo->lpDefaultExtention = "zip";
	}
}


SevenZipArchive::SevenZipArchive (
		const SevenZipModule *pModule, 
		unsigned int nFormatIndex,
		const char *lpFileName, 
		bool bNewArchive
		)
{
	m_pArchive = NULL;
	m_pInFile = NULL;

	m_nFormatIndex = nFormatIndex;
	m_pModule = pModule;
	m_lpFileName = StrDuplicate (lpFileName);

	//m_bForcedUpdate = false;
	m_bOpened = false;
	m_bNewArchive = bNewArchive;
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

	if ( /*m_bForcedUpdate || */!m_bOpened )
	{
		//m_bForcedUpdate = false;

		if ( m_bOpened )
			pCloseArchive ();

	  	m_pInFile = new CInFile (m_lpFileName);

	  	if ( m_bNewArchive )
	  	{
	  		if ( m_pInFile->Create () )
	  		{
				HRESULT hr = m_pModule->m_pfnCreateObject (
  						&m_pModule->m_puids[m_nFormatIndex],
  						&IID_IInArchive,
						(void**)&m_pArchive
	  					);

				if ( hr == S_OK )
				{
					m_bOpened = true;
					return true;
				}
	  		}

			m_pInFile->Release ();
			m_pInFile = NULL;
	  	}
	  	else
	  	{
  			if ( m_pInFile->Open () )
	  		{
				HRESULT hr = m_pModule->m_pfnCreateObject (
  						&m_pModule->m_puids[m_nFormatIndex],
  						&IID_IInArchive,
						(void**)&m_pArchive
	  					);

				if ( hr == S_OK )
	  			{
  					unsigned __int64 max = 1 << 17;

					CArchiveOpenCallback *pCallback = new CArchiveOpenCallback (this);

					m_bListPassword = false;

					hr = m_pArchive->Open (m_pInFile, &max, pCallback);

					if ( hr == S_OK )
		  			{
  						if ( m_pArchive->GetNumberOfItems((unsigned int*)&m_nItemsNumber) == S_OK )
  						{
  							m_nItemsNumber--;

  							m_bOpened = true;

	  						delete pCallback;
  							return true;
	  					}

  						m_pArchive->Close();
  					}
	  				else
		  			{
			  			//HACK!!! цинично показываем пустую панель при неправильном пароле на
	  					//листинге. а то Far неадекватно воспринимает FALSE из GetFindData

  						if ( m_bListPassword )
  						{
  							m_nItemsNumber = (DWORD)-1;
		  					delete pCallback;
  							return true;
  						}
					}

					delete pCallback;

	  				//if we get here, there is an error

  					m_pArchive->Release ();
  					m_pArchive = NULL;
	  			}

	  			m_pInFile->Release ();
  				m_pInFile = NULL;
		  	}
	  	}
	}
	else
	{
		if ( m_pArchive->GetNumberOfItems((unsigned int*)&m_nItemsNumber) == S_OK )
		{
			m_nItemsNumber--;
			return true;
		}
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

	m_bOpened = false;
}

unsigned __int64 VariantToInt64 (CPropVariant *value)
{
	switch ( value->vt )
	{
		case VT_UI1:
			return value->bVal;
		case VT_UI2:
			return value->uiVal;
		case VT_UI4:
			return value->ulVal;
		case VT_UI8:
			return (unsigned __int64)value->uhVal.QuadPart;
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

	CPropVariant value;

	if ( m_pArchive->GetProperty (m_nItemsNumber, kpidPath, &value) == S_OK )
	{
		if ( value.vt == VT_BSTR )
			WideCharToMultiByte (CP_OEMCP, 0, value.bstrVal, -1, pItem->pi.FindData.cFileName, sizeof (pItem->pi.FindData.cFileName), NULL, NULL);
		else
		{
			strcpy (pItem->pi.FindData.cFileName, FSF.PointToName (m_lpFileName));
			CutTo (pItem->pi.FindData.cFileName, '.', true);
		}

		if ( m_pArchive->GetProperty (m_nItemsNumber, kpidAttributes, &value) == S_OK )
		{
			if ( value.vt == VT_UI4 )
		        pItem->pi.FindData.dwFileAttributes = value.ulVal;
		}

		if ( m_pArchive->GetProperty (m_nItemsNumber, kpidIsFolder, &value) == S_OK )
		{
			if ( value.vt == VT_BOOL )
			{
				if ( value.boolVal == VARIANT_TRUE )
					pItem->pi.FindData.dwFileAttributes |= FILE_ATTRIBUTE_DIRECTORY;
			}
		}

		if ( m_pArchive->GetProperty (m_nItemsNumber, kpidSize, &value) == S_OK )
		{
			unsigned __int64 size = VariantToInt64 (&value);

			pItem->pi.FindData.nFileSizeLow = (DWORD)size;
			pItem->pi.FindData.nFileSizeHigh = (DWORD)(size >> 32);
		}

		if ( m_pArchive->GetProperty (m_nItemsNumber, kpidPackedSize, &value) == S_OK )
		{
			unsigned __int64 size = VariantToInt64 (&value);

			pItem->pi.PackSize = (DWORD)size;
			pItem->pi.PackSizeHigh = (DWORD)(size >> 32);
		}

		if ( m_pArchive->GetProperty (m_nItemsNumber, kpidCRC, &value) == S_OK )
		{
			if ( value.vt == VT_UI4 )
				pItem->pi.CRC32 = value.ulVal;
		}

		if ( m_pArchive->GetProperty (m_nItemsNumber, kpidCreationTime, &value) == S_OK )
		{
			if ( value.vt == VT_FILETIME )
				memcpy (&pItem->pi.FindData.ftCreationTime, &value.filetime, sizeof (FILETIME));
		}

		if ( m_pArchive->GetProperty (m_nItemsNumber, kpidLastAccessTime, &value) == S_OK )
		{
			if ( value.vt == VT_FILETIME )
				memcpy (&pItem->pi.FindData.ftLastAccessTime, &value.filetime, sizeof (FILETIME));
		}

		if ( m_pArchive->GetProperty (m_nItemsNumber, kpidLastWriteTime, &value) == S_OK )
		{
			if ( value.vt == VT_FILETIME )
				memcpy (&pItem->pi.FindData.ftLastWriteTime, &value.filetime, sizeof (FILETIME));
		}


		//__debug ("%d", m_nItemsNumber+1);

		pItem->pi.UserData = m_nItemsNumber+1;

		nResult = E_SUCCESS;
	}

	m_nItemsNumber--;

	return nResult;
}


bool __stdcall SevenZipArchive::pTest (
		PluginPanelItem *pItems,
		int nItemsNumber
		)
{
	return true;
}

void CreateTempName (const char *lpArchiveName, char *lpResultName) //MAX_PATH result!!
{
	int i = 0;

	do {
		FSF.sprintf (lpResultName, "%s.%d", lpArchiveName, i++);
	} while ( GetFileAttributes (lpResultName) != INVALID_FILE_ATTRIBUTES );
}


bool __stdcall SevenZipArchive::pDelete (
		PluginPanelItem *pItems,
		int nItemsNumber
		)
{
	IOutArchive *outArchive;
	unsigned int nArchiveItemsNumber;
	bool bResult = false;

	m_pArchive->GetNumberOfItems (&nArchiveItemsNumber);

	if ( SUCCEEDED (m_pArchive->QueryInterface(
			IID_IOutArchive,
			(void**)&outArchive
			)) )
	{
		pointer_array<ArchiveUpdateItem*> indicies (ARRAY_OPTIONS_DELETE);

		for (unsigned int i = 0; i < nArchiveItemsNumber; i++) //лажа полная написана
		{
			bool bFound = false;

			for (int j = 0; j < nItemsNumber; j++)
			{
				if ( i == (pItems[j].UserData-1) )
				{
					bFound = true;
					break;
				}
			}

			if ( !bFound )
			{
				ArchiveUpdateItem *item = new ArchiveUpdateItem;

				item->index = i;
				item->bNewFile = false;

				indicies.add (item);
			}
		}

		char szTempName[MAX_PATH];

		CreateTempName (m_lpFileName, szTempName);

		COutFile *file = new COutFile (szTempName);
		CArchiveUpdateCallback *pCallback = new CArchiveUpdateCallback (this, &indicies);

		if ( file->Open () )
		{
			if ( outArchive->UpdateItems (
					(ISequentialOutStream *)file,
					indicies.count(),
					pCallback
					) == S_OK )
				bResult = true;
		}

		delete file;

		if ( bResult )
		{
			//MessageBox (0, szTempName, m_lpFileName, MB_OK);

			pCloseArchive ();  //???
			//m_pInFile->Close ();
			MoveFileEx (szTempName, m_lpFileName, MOVEFILE_COPY_ALLOWED|MOVEFILE_REPLACE_EXISTING);
			//m_pInFile->Open ();

			//m_bForcedUpdate = true;
		}
		else
			DeleteFile (szTempName);

		delete pCallback;

		outArchive->Release ();
		indicies.free ();
	}

	return bResult;
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
		PluginPanelItem *pItems,
		int nItemsNumber,
		const char *lpDestPath,
		const char *lpCurrentFolder
		)
{
	bool bResult = false;
	unsigned int *indices = (unsigned int*)malloc (nItemsNumber*sizeof (unsigned int));
	ArchiveItem *items = (ArchiveItem*)malloc (nItemsNumber*sizeof (ArchiveItem));

	int lastitem = 0;

	for (int i = 0; i < nItemsNumber; i++)
	{
		if ( pItems[i].UserData )
		{
			//__debug ("indexed - %s %d", pItems[i].FindData.cFileName, pItems[i].UserData);

			indices[lastitem] = pItems[i].UserData-1; //GetIndex (m_pArchive, pItems[i].FindData.cFileName);

			items[lastitem].nIndex = indices[lastitem];
			items[lastitem].pItem = &pItems[i];

			lastitem++;
		}
		//else
		//	__debug ("non indexed - %s %d", pItems[i].FindData.cFileName, pItems[i].UserData);
	}

	FSF.qsort (indices, lastitem, 4, compare);

	//unsigned int nItems = 0;
	//m_pArchive->GetNumberOfItems((unsigned int*)&nItems);

	Callback (AM_START_OPERATION, OPERATION_EXTRACT, 0);

	CArchiveExtractCallback *pCallback = new CArchiveExtractCallback (this, items, lastitem, lpDestPath, lpCurrentFolder);

	if ( m_pArchive->Extract(
			indices,
			(unsigned int)lastitem,
			0,
			pCallback
			) == S_OK )
		bResult = true;

	delete pCallback;
	free (indices);
	free (items);

	return bResult;
}

bool __stdcall SevenZipArchive::pAddFiles (
		const char *lpSourcePath,
		const char *lpCurrentPath,
		PluginPanelItem *pItems,
		int nItemsNumber
		)
{
	IOutArchive *outArchive;

	unsigned int nArchiveItemsNumber;
	bool bResult = false;

	if ( SUCCEEDED (m_pArchive->QueryInterface(
			IID_IOutArchive,
			(void**)&outArchive
			)) )
	{
		pointer_array <ArchiveUpdateItem*> indicies (ARRAY_OPTIONS_DELETE);
		CPropVariant value;

		if ( !m_bNewArchive )
		{
			m_pArchive->GetNumberOfItems (&nArchiveItemsNumber);

			for (unsigned int i = 0; i < nArchiveItemsNumber; i++)
			{
				ArchiveUpdateItem *item = new ArchiveUpdateItem;

				item->index = i;
				item->bNewFile = false;
				item->lpCurrentPath = lpCurrentPath;
				item->lpSourcePath = lpSourcePath;

				indicies.add (item);
			}
		}

		for (int i = 0; i < nItemsNumber; i++)
		{
			bool bFound = false;

			if ( !m_bNewArchive )
			{
				char szCheckName[MAX_PATH];

				memset (szCheckName, 0, MAX_PATH);

				if ( lpCurrentPath && *lpCurrentPath )
				{
					strcpy (szCheckName, lpCurrentPath);
					FSF.AddEndSlash (szCheckName);
				}

				strcat (szCheckName, pItems[i].FindData.cFileName);

				for (unsigned int j = 0; j < nArchiveItemsNumber; j++)
				{
					char szArchiveFileName [MAX_PATH];

					memset (szArchiveFileName, 0, MAX_PATH);

					if ( m_pArchive->GetProperty (j, kpidPath, &value) == S_OK )
					{
						if ( value.vt == VT_BSTR )
							WideCharToMultiByte (CP_OEMCP, 0, value.bstrVal, -1, szArchiveFileName, MAX_PATH, NULL, NULL);
					}

					if ( !FSF.LStricmp (szArchiveFileName, szCheckName) )
					{
						bFound = true;

						ArchiveUpdateItem *item = indicies.at(j);

						item->bNewFile = true;
						item->pItem = &pItems[i];
						item->lpCurrentPath = lpCurrentPath;
						item->lpSourcePath = lpSourcePath;

						break;
					}
				}
			}

			if ( !bFound )
			{
				ArchiveUpdateItem *item = new ArchiveUpdateItem;
				item->index = (unsigned int)-1;
				item->bNewFile = true;
				item->pItem = &pItems[i];
				item->lpCurrentPath = lpCurrentPath;
				item->lpSourcePath = lpSourcePath;

				indicies.add (item);
			}
		}

		char szTempName[MAX_PATH];

		CreateTempName (m_lpFileName, szTempName);

		COutFile *file = new COutFile (szTempName);
		CArchiveUpdateCallback *pCallback = new CArchiveUpdateCallback (this, &indicies);

		if ( file->Open () )
		{
			if ( outArchive->UpdateItems (
					(ISequentialOutStream *)file,
					indicies.count(),
					pCallback
					) == S_OK )
				bResult = true;
		}

		delete file;

		if ( bResult )
		{
			pCloseArchive ();
			MoveFileEx (szTempName, m_lpFileName, MOVEFILE_COPY_ALLOWED|MOVEFILE_REPLACE_EXISTING);
		}
		else
			DeleteFile (szTempName);

		delete pCallback;

		outArchive->Release ();
		indicies.free ();
	}

	return bResult;
}

void __stdcall SevenZipArchive::pNotify (int nEvent, void *pEventData)
{
	if ( nEvent == NOTIFY_EXTERNAL_ADD_START || nEvent == NOTIFY_EXTERNAL_DELETE_START )
		pCloseArchive ();
}

int SevenZipArchive::Callback (int nMsg, int nParam1, int nParam2)
{
	if ( m_pfnCallback )
		return m_pfnCallback (nMsg, nParam1, nParam2);

	return FALSE;
}
