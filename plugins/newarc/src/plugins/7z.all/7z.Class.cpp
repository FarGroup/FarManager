#include "7z.h"
#include "Guid.h"
#include "commands.h"
#include "detectarchive.h"
#include <array.hpp>
#include <debug.h>

const unsigned char BZipSig[]     = {'B' , 'Z', 'h'};
const unsigned char GZipSig[]     = {0x1F, 0x8B};
const unsigned char ZSig[]        = {0x1F, 0x9D};
const unsigned char RpmSig[]      = {0xED, 0xAB, 0xEE, 0xDB};
const unsigned char DebSig[]      = {'!', '<', 'a', 'r', 'c', 'h', '>', 0x0A, 'd', 'e', 'b', 'i', 'a', 'n', '-', 'b', 'i', 'n', 'a', 'r', 'y'};
const unsigned char CpioSig[]     = {'0', '7', '0', '7', '0'}; //BUGBUG: вроде не совсем точно
const unsigned char ChmSig[]      = {'I', 'T', 'S', 'F'};
const unsigned char WimSig[]      = {'M', 'S', 'W', 'I', 'M', 0, 0, 0};
const unsigned char CompoundSig[] = {0xD0, 0xCF, 0x11, 0xE0, 0xA1, 0xB1, 0x1A, 0xE1};
const unsigned char ELFSig[]      = {0x7F, 'E', 'L', 'F'};
const unsigned char MubSig[]      = {0xCA, 0xFE, 0xBA, 0xBE, 0, 0, 0};
const unsigned char XarSig[]      = {'x', 'a', 'r', '!', 0, 0x1C};
const unsigned char DmgSig[]      = {0x78, 0xDA}; //BUGBUG: тупо наугад поставил, в тех 2-ух dmg что я видел было так.
const unsigned char XzSig[]       = {0xFD, '7' , 'z', 'X', 'Z', '\0'};
const unsigned char VhdSig[]      = { 'c', 'o', 'n', 'e', 'c', 't', 'i', 'x', 0, 0 };
const unsigned char MbrSig[]      = { 1, 1, 0 };
const unsigned char FatSig[]      = { 0x55, 0xAA };
const unsigned char NtfsSig[]     = { 'N', 'T', 'F', 'S', ' ', ' ', ' ', ' ', 0 };
const unsigned char MsLzSig[]     = { 0x53, 0x5A, 0x44, 0x44, 0x88, 0xF0, 0x27, 0x33, 0x41 };
const unsigned char FlvSig[]      = { 'F', 'L', 'V' };
const unsigned char SwfSig[]      = { 'F', 'W', 'S' };

struct FormatInfo {
	const GUID *puid;
	const unsigned char *psig;
	DWORD size;
	bool bPosZero;
	DETECTARCHIVE pDetectArchive;
};

const FormatInfo signs[] = {
	{&CLSID_CFormat7z,        NULL,                                0, false, Is7zHeader},
	{&CLSID_CRarHandler,      NULL,                                0, false, IsRarHeader},
	{&CLSID_CZipHandler,      NULL,                                0, false, IsZipHeader},
	{&CLSID_CRpmHandler,      (const unsigned char *)&RpmSig,      4, true,  NULL},
	{&CLSID_CDebHandler,      (const unsigned char *)&DebSig,     21, true,  NULL},
	{&CLSID_CCabHandler,      NULL,                                0, false, IsCabHeader},
	{&CLSID_CBZip2Handler,    (const unsigned char *)&BZipSig,     3, true,  NULL},
	{&CLSID_CArjHandler,      NULL,                                0, false, IsArjHeader},
	{&CLSID_CTarHandler,      NULL,                                0, true,  IsTarHeader},
	{&CLSID_CGZipHandler,     (const unsigned char *)&GZipSig,     2, true,  NULL},
	{&CLSID_CZHandler,        (const unsigned char *)&ZSig,        2, true,  NULL},
	{&CLSID_CLzhHandler,      NULL,                                0, false, IsLzhHeader},
	{&CLSID_CCpioHandler,     (const unsigned char *)&CpioSig,     5, true,  NULL},
	{&CLSID_CChmHandler,      (const unsigned char *)&ChmSig,      4, true,  NULL},
	{&CLSID_CNsisHandler,     NULL,                                0, false, IsNSISHeader},
	{&CLSID_CIsoHandler,      NULL,                                0, true,  IsIsoHeader},
	{&CLSID_CWimHandler,      (const unsigned char *)&WimSig,      8, true,  NULL},
	{&CLSID_CCompoundHandler, (const unsigned char *)&CompoundSig, 8, true,  NULL},
	{&CLSID_CUdfHandler,      NULL,                                0, true,  IsUdfHeader},
	{&CLSID_CPeHandler,       NULL,                                0, true,  IsPEHeader},
	{&CLSID_CElfHandler,      (const unsigned char *)&ELFSig,      4, true,  NULL},
	{&CLSID_CMachoHandler,    NULL,                                0, true,  IsMachoHeader},
	{&CLSID_CMubHandler,      (const unsigned char *)&MubSig,      7, true,  NULL},
	{&CLSID_CXarHandler,      (const unsigned char *)&XarSig,      6, true,  NULL},
	{&CLSID_CHfsHandler,      NULL,                                0, true,  IsHfsHeader},
	{&CLSID_CLzmaHandler,     NULL,                                0, true,  IsLzmaHeader},
//	{&CLSID_CLzma86Handler,     NULL,                                0, true,  IsLzma86Header},
	{&CLSID_CDmgHandler,      (const unsigned char *)&DmgSig,      2, true,  NULL},
	{&CLSID_CXzHandler,       (const unsigned char *)&XzSig,       6, true,  NULL},
	{&CLSID_CVhdHandler,      (const unsigned char *)&VhdSig,     10, true,  NULL},
	{&CLSID_CMbrHandler,      (const unsigned char *)&MbrSig,      3, true,  NULL},
	{&CLSID_CFatHandler,      (const unsigned char *)&FatSig,      2, true,  NULL},
	{&CLSID_CNtfsHandler,     (const unsigned char *)&NtfsSig,     9, true,  NULL},
	{&CLSID_CMsLzHandler,     (const unsigned char *)&MsLzSig,     9, true,  NULL},
	{&CLSID_CFlvHandler,      (const unsigned char *)&FlvSig,      3, true,  NULL},
	{&CLSID_CSwfHandler,      (const unsigned char *)&SwfSig,      3, true,  NULL},
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

			if ( strstr(lpFileName, ".") && StrLength(lpFileName) > 2 )
			{
				bool bMatch = false;
				int len = StrLength(lpFileName);

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


bool ConvertToFormatInfo (
		CPropVariant &vGUID,
		CPropVariant &vUpdate,
		CPropVariant &vExtension,
		CPropVariant &vSignature,
		CPropVariant &vName,
		FormatInfoInternal *pInfo
		)
{
	if ( (vGUID.vt != VT_BSTR) ||
		 (vUpdate.vt != VT_BOOL) ||
		 (vName.vt != VT_BSTR) )
		 return false;

	memcpy (&pInfo->uid, vGUID.bstrVal, sizeof (GUID));
	pInfo->bUpdate = (vUpdate.boolVal == VARIANT_TRUE);

	int nLength;

	if ( vExtension.vt == VT_BSTR )
	{
		nLength = SysStringLen (vExtension.bstrVal);

		pInfo->lpDefaultExt = (char*)malloc (nLength+1);
		memset (pInfo->lpDefaultExt, 0, nLength+1);

		WideCharToMultiByte (CP_OEMCP, 0, vExtension.bstrVal, -1, pInfo->lpDefaultExt, nLength, NULL, NULL);

		char *p = strchr(pInfo->lpDefaultExt, ' ');

		if ( p )
			*p = 0;
	}
	else
		pInfo->lpDefaultExt = NULL;

	if ( vSignature.vt == VT_BSTR )
	{
		nLength = SysStringByteLen(vSignature.bstrVal);

		pInfo->lpSignature = (char*)malloc(nLength);
		pInfo->nSignatureLength = nLength;

		memcpy (pInfo->lpSignature, vSignature.bstrVal, nLength);
	}
	else
		pInfo->lpSignature = NULL;


	nLength = SysStringLen (vName.bstrVal);

	pInfo->lpName = (char*)malloc (nLength+1+strlen(" archive [7z]"));
	memset (pInfo->lpName, 0, nLength+1);

	WideCharToMultiByte (CP_OEMCP, 0, vName.bstrVal, -1, pInfo->lpName, nLength, NULL, NULL);

	strcat (pInfo->lpName, " archive [7z]");

	return true;
}


bool SevenZipModule::Initialize (const char *lpFileName)
{
	bool bResult = false;

	m_pInfo = NULL;

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

		CPropVariant vGUID, vUpdate, vExtension, vSignature, vName;

		if ( m_pfnCreateObject &&
			 (m_pfnGetHandlerProperty || (m_pfnGetHandlerProperty2 && m_pfnGetNumberOfFormats)) )
		{
			if ( m_pfnGetHandlerProperty2 && m_pfnGetNumberOfFormats )
			{
				if ( m_pfnGetNumberOfFormats (&m_nNumberOfFormats) == S_OK )
				{
					m_pInfo = (FormatInfoInternal*)malloc (m_nNumberOfFormats*sizeof (FormatInfoInternal));

					bResult = true;

					for (unsigned int i = 0; i < m_nNumberOfFormats; i++)
					{
						if ( (m_pfnGetHandlerProperty2 (i, NArchive::kClassID, &vGUID) != S_OK) ||
							 (m_pfnGetHandlerProperty2 (i, NArchive::kUpdate, &vUpdate) != S_OK) ||
							 (m_pfnGetHandlerProperty2 (i, NArchive::kExtension, &vExtension) != S_OK) ||
							 (m_pfnGetHandlerProperty2 (i, NArchive::kStartSignature, &vSignature) != S_OK) ||
							 (m_pfnGetHandlerProperty2 (i, NArchive::kName, &vName) != S_OK) ||
							 !ConvertToFormatInfo (vGUID, vUpdate, vExtension, vSignature, vName, &m_pInfo[i]) )
						{
							bResult = false;
							break;
						}
					}
				}
			}
			else
			{
				m_nNumberOfFormats= 1;

				m_pInfo = (FormatInfoInternal*)malloc (m_nNumberOfFormats*sizeof (FormatInfoInternal));

				if ( (m_pfnGetHandlerProperty (NArchive::kClassID, &vGUID) != S_OK) ||
					 (m_pfnGetHandlerProperty (NArchive::kUpdate, &vUpdate) != S_OK) ||
					 (m_pfnGetHandlerProperty (NArchive::kExtension, &vExtension) != S_OK) ||
					 (m_pfnGetHandlerProperty (NArchive::kStartSignature, &vSignature) != S_OK) ||
					 (m_pfnGetHandlerProperty (NArchive::kName, &vName) != S_OK) ||
					 !ConvertToFormatInfo (vGUID, vUpdate, vExtension, vSignature, vName, &m_pInfo[0]) )
					bResult = false;
			}
		}
	}

	return bResult;
}


SevenZipModule::~SevenZipModule ()
{
	if ( m_pInfo )
	{
		for (unsigned int i = 0; i < m_nNumberOfFormats; i++)
		{
			free (m_pInfo[i].lpDefaultExt);
			free (m_pInfo[i].lpSignature);
		}

		free (m_pInfo);
	}

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
	pInfo->dwFlags = AFF_SUPPORT_INTERNAL_EXTRACT|AFF_SUPPORT_INTERNAL_TEST;
	pInfo->uid = m_pInfo[nFormatIndex].uid;

	if ( m_pInfo[nFormatIndex].bUpdate )
		pInfo->dwFlags |= (AFF_SUPPORT_INTERNAL_DELETE|AFF_SUPPORT_INTERNAL_ADD|AFF_SUPPORT_INTERNAL_CREATE);


	//if ( m_pInfo[nFormatIndex].uid == CLSID_CFormat7z )
		pInfo->dwFlags |= (AFF_SUPPORT_INTERNAL_CONFIG);

	pInfo->lpName = m_pInfo[nFormatIndex].lpName;
	pInfo->lpDefaultExtention = m_pInfo[nFormatIndex].lpDefaultExt;
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
						&m_pModule->m_pInfo[m_nFormatIndex].uid,
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
						&m_pModule->m_pInfo[m_nFormatIndex].uid,
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
						//HACK!!! Ўшэшўэю яюърч√трхь яєёЄє■ ярэхы№ яЁш эхяЁртшы№эюь ярЁюых эр
						//ышёЄшэух. р Єю Far эхрфхътрЄэю тюёяЁшэшьрхЄ FALSE шч GetFindData

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

		if ( m_pArchive->GetProperty (m_nItemsNumber, kpidAttrib, &value) == S_OK )
		{
			if ( value.vt == VT_UI4 )
				pItem->pi.FindData.dwFileAttributes = value.ulVal;
		}

		if ( m_pArchive->GetProperty (m_nItemsNumber, kpidIsDir, &value) == S_OK )
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

		if ( m_pArchive->GetProperty (m_nItemsNumber, kpidPackSize, &value) == S_OK )
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

		if ( m_pArchive->GetProperty (m_nItemsNumber, kpidCTime, &value) == S_OK )
		{
			if ( value.vt == VT_FILETIME )
				memcpy (&pItem->pi.FindData.ftCreationTime, &value.filetime, sizeof (FILETIME));
		}

		if ( m_pArchive->GetProperty (m_nItemsNumber, kpidATime, &value) == S_OK )
		{
			if ( value.vt == VT_FILETIME )
				memcpy (&pItem->pi.FindData.ftLastAccessTime, &value.filetime, sizeof (FILETIME));
		}

		if ( m_pArchive->GetProperty (m_nItemsNumber, kpidMTime, &value) == S_OK )
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

		for (unsigned int i = 0; i < nArchiveItemsNumber; i++) //ырцр яюыэр  эряшёрэр
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
		CArchiveUpdateCallback *pCallback = new CArchiveUpdateCallback (this, &indicies, NULL, NULL);

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

			indices[lastitem] = (unsigned int)pItems[i].UserData-1; //GetIndex (m_pArchive, pItems[i].FindData.cFileName);

			items[lastitem].nIndex = indices[lastitem];
			items[lastitem].pItem = &pItems[i];

			lastitem++;
		}
		//else
		//	__debug ("non indexed - %s %d", pItems[i].FindData.cFileName, pItems[i].UserData);
	}

	FSF.qsort (indices, lastitem, sizeof(unsigned int), compare);
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
				indicies.add (item);
			}
		}

//-------------------------

		ISetProperties *setProperties;

		if ( SUCCEEDED (outArchive->QueryInterface (
				IID_ISetProperties,
				(void**)&setProperties
				)) )
		{
			//MessageBox (0, "set", "asd", MB_OK);

			setProperties->Release ();
		}


//-------------------------

		char szTempName[MAX_PATH];

		CreateTempName (m_lpFileName, szTempName);

		COutFile *file = new COutFile (szTempName);

		CArchiveUpdateCallback *pCallback = new CArchiveUpdateCallback (
				this,
				&indicies,
				lpSourcePath,
				lpCurrentPath
				);

		if ( file->Open () )
		{
			char sz[MAX_PATH];

			wsprintf (sz, "%d", indicies.count());

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

LONG_PTR __stdcall SevenZipArchive::Callback (int nMsg, int nParam1, LONG_PTR nParam2)
{
	if ( m_pfnCallback )
		return m_pfnCallback (nMsg, nParam1, nParam2);

	return FALSE;
}
