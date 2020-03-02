#include "7z.h"

#include "GUID.h"

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
//const unsigned char NtfsSig[]     = { 'N', 'T', 'F', 'S', ' ', ' ', ' ', ' ', 0 };
const unsigned char NtfsSig[]     = { 0xEB, 0x52, 0x90, 'N', 'T', 'F', 'S', ' ', ' ', ' ', ' ', 0 };
const unsigned char MsLzSig[]     = { 0x53, 0x5A, 0x44, 0x44, 0x88, 0xF0, 0x27, 0x33, 0x41 };
const unsigned char FlvSig[]      = { 'F', 'L', 'V' };
const unsigned char SwfSig[]      = { 'F', 'W', 'S' };

struct FormatInfo {
	const GUID* puid;
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


bool ConvertToFormatInfo(
		CPropVariant& vGUID,
		CPropVariant& vUpdate,
		CPropVariant& vExtension,
		CPropVariant& vSignature,
		CPropVariant& vName,
		ArchiveFormatInfo* pInfo
		)
{
	if ( (vGUID.vt != VT_BSTR) ||
		 (vUpdate.vt != VT_BOOL) ||
		 (vName.vt != VT_BSTR) )
		 return false;

	bool bUpdate = (vUpdate.boolVal == VARIANT_TRUE);

	string strDefaultExtention;

	if ( vExtension.vt == VT_BSTR )
	{
#ifdef UNICODE
		strDefaultExtention = vExtension.bstrVal;
#else
		strDefaultExtention.SetData(vExtension.bstrVal, CP_OEMCP);
#endif

		TCHAR *pBuffer = strDefaultExtention.GetBuffer();

		TCHAR *p = _tcschr(pBuffer, _T(' '));

		if ( p )
			*p = 0;

		strDefaultExtention.ReleaseBuffer();
	}
	else
		strDefaultExtention = NULL;

	/*if ( vSignature.vt == VT_BSTR )
	{
		nLength = SysStringByteLen(vSignature.bstrVal);

		pInfo->pSignature = (unsigned char*)malloc(nLength);
		pInfo->nSignatureLength = nLength;

		memcpy (pInfo->pSignature, vSignature.bstrVal, nLength);
	}
	else
		pInfo->pSignature = NULL;
	*/ //not currently used, but do not delete

	string strName;

#ifdef UNICODE
	strName = vName.bstrVal;
#else
	strName.SetData(vName.bstrVal, CP_OEMCP);
#endif

	strName += _T(" archive [7z]");

	memcpy(&pInfo->uid, vGUID.bstrVal, sizeof (GUID));

	pInfo->lpName = StrDuplicate(strName);
	pInfo->lpDefaultExtention = StrDuplicate(strDefaultExtention);

	pInfo->dwFlags = AFF_NEED_EXTERNAL_NOTIFICATIONS;
	pInfo->dwFlags |= AFF_SUPPORT_INTERNAL_EXTRACT|AFF_SUPPORT_INTERNAL_TEST;

	if ( bUpdate )
		pInfo->dwFlags |= (AFF_SUPPORT_INTERNAL_DELETE|AFF_SUPPORT_INTERNAL_ADD|AFF_SUPPORT_INTERNAL_CREATE);

	if ( pInfo->uid == CLSID_CFormat7z )
	{
		pInfo->dwFlags |= AFF_SUPPORT_CONFIG_CREATE;
	//	pInfo->dwFlags |= AFF_SUPPORT_INTERNAL_CONFIG;
	}

	if ( (pInfo->uid == CLSID_CFormat7z) || 
		 (pInfo->uid == CLSID_CRarHandler) ||
		 (pInfo->uid == CLSID_CZipHandler) ||
		 (pInfo->uid == CLSID_CArjHandler) ||
		 (pInfo->uid == CLSID_CTarHandler) ||
		 (pInfo->uid == CLSID_CGZipHandler) ||
		 (pInfo->uid == CLSID_CBZip2Handler) ||
		 (pInfo->uid == CLSID_CZHandler) ||
		 (pInfo->uid == CLSID_CCabHandler) ||
		 (pInfo->uid == CLSID_CLzhHandler) ||
		 (pInfo->uid == CLSID_CCpioHandler) ||
		 (pInfo->uid == CLSID_CRpmHandler) ||
		 (pInfo->uid == CLSID_CDebHandler) )
		 pInfo->dwFlags |= AFF_SUPPORT_DEFAULT_COMMANDS;

	return true;
}

SevenZipPlugin::SevenZipPlugin(const GUID& uid, const TCHAR* lpModuleName)
{
	m_strModuleName = lpModuleName;
	m_uid = uid;

	m_pfnCreateObject = nullptr;
	m_pfnGetHandlerProperty = nullptr;
	m_pfnGetHandlerProperty2 = nullptr;
	m_pfnGetNumberOfFormats = nullptr;
}

const GUID& SevenZipPlugin::GetUID()
{
	return m_uid;
}

const TCHAR* SevenZipPlugin::GetModuleName()
{
	return m_strModuleName;
}

unsigned int SevenZipPlugin::GetNumberOfFormats()
{
	return m_pFormatInfo.count();
}

ArchiveFormatInfo* SevenZipPlugin::GetFormats()
{
	return m_pFormatInfo.data();
}

SevenZipPlugin::~SevenZipPlugin()
{
	for (unsigned int i = 0; i < m_pFormatInfo.count(); i++)
	{
		ArchiveFormatInfo* info = &m_pFormatInfo[i];

		StrFree((void*)info->lpName);
		StrFree((void*)info->lpDefaultExtention);
	}

	if ( m_hModule )
		FreeLibrary(m_hModule);
}

bool SevenZipPlugin::Load()
{
	bool bResult = false;

	m_hModule = LoadLibraryEx (
			m_strModuleName,
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
				unsigned int uNumberOfFormats;

				if ( m_pfnGetNumberOfFormats(&uNumberOfFormats) == S_OK )
				{
					bResult = true;

					for (unsigned int i = 0; i < uNumberOfFormats; i++)
					{
						ArchiveFormatInfo format;
						memset(&format, 0, sizeof(format));

						if ( (m_pfnGetHandlerProperty2(i, NArchive::kClassID, &vGUID) == S_OK) &&
							 (m_pfnGetHandlerProperty2(i, NArchive::kUpdate, &vUpdate) == S_OK) &&
							 (m_pfnGetHandlerProperty2(i, NArchive::kExtension, &vExtension) == S_OK) &&
							 (m_pfnGetHandlerProperty2(i, NArchive::kStartSignature, &vSignature) == S_OK) &&
							 (m_pfnGetHandlerProperty2(i, NArchive::kName, &vName) == S_OK) &&
							 ConvertToFormatInfo(vGUID, vUpdate, vExtension, vSignature, vName, &format) )
							m_pFormatInfo.add(format);
					}
				}
			}
			else
			{
				ArchiveFormatInfo format;
				memset(&format, 0, sizeof(format));

				if ( (m_pfnGetHandlerProperty(NArchive::kClassID, &vGUID) == S_OK) &&
					 (m_pfnGetHandlerProperty(NArchive::kUpdate, &vUpdate) == S_OK) &&
					 (m_pfnGetHandlerProperty(NArchive::kExtension, &vExtension) == S_OK) &&
					 (m_pfnGetHandlerProperty(NArchive::kStartSignature, &vSignature) == S_OK) &&
					 (m_pfnGetHandlerProperty(NArchive::kName, &vName) == S_OK) &&
					 ConvertToFormatInfo(vGUID, vUpdate, vExtension, vSignature, vName, &format) )
					m_pFormatInfo.add(format);
			}
			
			bResult = (m_pFormatInfo.count() > 0);
		}
	}

	return bResult;
}

bool FindSplitFormat(const TCHAR* lpFileName, FormatPosition* pPosition)
{
	if ( _tcsstr(lpFileName, _T(".")) && StrLength(lpFileName) > 2 )
	{
		bool bMatch = false;
		int len = StrLength(lpFileName);

		if ( lpFileName[len-2] == _T('0') && lpFileName[len-1] == _T('1') )
		{
			int i=len-3;
			for (; i>0; i--)
				if ( lpFileName[i] != _T('0') )
					break;
			if ( lpFileName[i] == _T('.') )
				bMatch = true;
		}
		else if ( (lpFileName[len-1] == _T('A') || lpFileName[len-1] == _T('a')) &&
				  (lpFileName[len-2] == _T('A') || lpFileName[len-2] == _T('a')) )
			bMatch = true;

		if ( bMatch )
		{
			pPosition->puid = &CLSID_CSplitHandler;
			pPosition->position = 0;

			return true;
		}
	}

	return false;
}

bool FindFormat(const unsigned char* pBuffer, DWORD dwBufferSize, const FormatInfo* pInfo, FormatPosition* pPosition)
{
	if ( pInfo->pDetectArchive )
	{
		int nPosition = pInfo->pDetectArchive(pBuffer, dwBufferSize);

		if ( nPosition != -1 )
		{
			if ( !pInfo->bPosZero || (nPosition == 0) )
			{
				pPosition->puid = pInfo->puid;
				pPosition->position = nPosition;
				return true;
			}
		}
	}
	else
	{
		if ( dwBufferSize >= pInfo->size )
		{
			for (size_t i = 0; i <= dwBufferSize-pInfo->size; i++)
			{
				if ( !memcmp (pBuffer+i, pInfo->psig, pInfo->size) )
				{
					pPosition->puid = pInfo->puid;
					pPosition->position = i;
					return true;
				}

				if ( pInfo->bPosZero )
					break;
			}	
		}
	}

	return false;
}


int FindFormats(const unsigned char* pBuffer, DWORD dwBufferSize, const TCHAR *lpFileName, Array<FormatPosition*> &formats)
{
	for (size_t i = 0; i < sizeof(signs)/sizeof(signs[0]); i++)
	{
		FormatPosition* pPosition = new FormatPosition;

		if ( FindFormat(pBuffer, dwBufferSize, &signs[i], pPosition) ||
			 FindSplitFormat(lpFileName, pPosition) )
		{
			formats.add(pPosition);
		}
	}

	return formats.count();
}

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

	return 0;
}

int SevenZipPlugin::QueryArchive(
		const unsigned char* pBuffer, 
		DWORD dwBufferSize, 
		const GUID& uid, 
		const TCHAR* lpFileName,
		Array<ArchiveQueryResult*>& result
		)
{
	for (int i = 0; i < sizeof(signs)/sizeof(signs[0]); i++)
	{
		if ( *signs[i].puid == uid )
		{
			FormatPosition Position;

			if ( FindFormat(pBuffer, dwBufferSize, &signs[i], &Position) ||
				 FindSplitFormat(lpFileName, &Position) )
			{
				ArchiveQueryResult* pResult = new ArchiveQueryResult;

				pResult->uidFormat = uid;
				pResult->uidPlugin = m_uid;

				result.add(pResult);
			}

			break;
		}
	}

	return 1;
}

int SevenZipPlugin::QueryArchives(
		const unsigned char* pBuffer, 
		DWORD dwBufferSize, 
		const TCHAR* lpFileName, 
		Array<ArchiveQueryResult*>& result
		)
{
	ObjectArray<FormatPosition*> formats;

	FindFormats(pBuffer, dwBufferSize, lpFileName, formats);

	formats.sort((void*)SortFormats, NULL);

	for (unsigned int i = 0; i < formats.count(); i++)
	{
		ArchiveQueryResult* pResult = new ArchiveQueryResult;

		pResult->uidPlugin = m_uid;
		pResult->uidFormat = *formats[i]->puid;

		result.add(pResult);
	}

	return formats.count(); //badbad, return added only!!!
}

SevenZipArchive* SevenZipPlugin::OpenCreateArchive(
			const GUID& uid, 
			const TCHAR* lpFileName, 
			HANDLE hCallback, 
			ARCHIVECALLBACK pfnCallback,
			bool bCreate
			)
{
	return new SevenZipArchive(this, uid, lpFileName, hCallback, pfnCallback, bCreate);
}


bool SevenZipPlugin::CreateObject(const GUID& uidFormat, const GUID& uidInterface, void** pObject)
{
	return m_pfnCreateObject(&uidFormat, &uidInterface, pObject);
}


void SevenZipPlugin::CloseArchive(SevenZipArchive* pArchive)
{
	delete pArchive;
}

bool SevenZipPlugin::GetDefaultCommand(const GUID& uid, int nCommand, const TCHAR** ppCommand)
{
	if ( uid == CLSID_CFormat7z )
		*ppCommand = p7Z[nCommand];
	else 
	
	if ( uid == CLSID_CRarHandler )
		*ppCommand = pRAR[nCommand];
	else

	if ( uid == CLSID_CZipHandler )
		*ppCommand = pZIP[nCommand];
	else

	if ( uid == CLSID_CArjHandler )
		*ppCommand = pARJ[nCommand];
	else

	if ( uid == CLSID_CTarHandler )
		*ppCommand = pTAR[nCommand];
	else 
	
	if ( uid == CLSID_CGZipHandler )
		*ppCommand = pGZIP[nCommand];
	else 
	
	if ( uid == CLSID_CBZip2Handler )
		*ppCommand = pBZIP[nCommand];
	else
	
	if ( uid == CLSID_CZHandler )
		*ppCommand = pZ[nCommand];
	else 
	
	if ( uid == CLSID_CCabHandler )
		*ppCommand = pCAB[nCommand];
	else

	if ( uid == CLSID_CLzhHandler )
		*ppCommand = pLZH[nCommand];
	else
	
	if ( uid == CLSID_CCpioHandler )
		*ppCommand = pCPIO[nCommand];
	else 
	
	if ( uid == CLSID_CRpmHandler )
		*ppCommand = pRPM[nCommand];
	else 
	
	if ( uid == CLSID_CDebHandler )
		*ppCommand = pDEB[nCommand];
	else
		return false;

	return true;
}

/*
const TCHAR *SevenZipLevel[] = {_T("None"), _T("Fastest"), _T("Fast"), _T("Normal"), _T("Maximum"), _T("Ultra")};
const TCHAR *SevenZipLevelStrings[] = {_T("X0"), _T("X1"), _T("X3"), _T("X5"), _T("X7"), _T("X9")};

const TCHAR *SevenZipLZMADictionary[] = {_T("64 KB"), _T("1 MB"), _T("2 MB"), _T("3 MB"), _T("4 MB"), _T("6 MB"), _T("8 MB"), _T("12 MB"), _T("16 MB"), _T("24 MB"), _T("32 MB"), _T("48 MB"), _T("64 MB")};
const TCHAR *SevenZipLZMADictionaryStrings[] = {_T("64K"), _T("1M"), _T("2M"), _T("3M"), _T("4M"), _T("6M"), _T("8M"), _T("12M"), _T("16M"), _T("24M"), _T("32M"), _T("48M"), _T("64M")};
const TCHAR *SevenZipLZMAWordSize[] = {_T("8"), _T("12"), _T("16"), _T("24"), _T("32"), _T("48"), _T("64"), _T("96"), _T("128"), _T("192"), _T("256"), _T("273")};
const TCHAR *SevenZipLZMASBSize[] = {_T("Non solid"), _T("1 MB"), _T("2 MB"), _T("4 MB"), _T("8 MB"), _T("16 MB"), _T("32 MB"), _T("64 MB"), _T("128 MB"), _T("256 MB"), _T("512 MB"), _T("1 GB"), _T("2 GB"), _T("4 GB"), _T("8 GB"), _T("16 GB"), _T("32 GB"), _T("64 GB"), _T("Solid")};
const TCHAR *SevenZipLZMASBSizeStrings[] = {_T("???"), _T("E1M"), _T("E2M"), _T("E4M"), _T("E8M"), _T("E16M"), _T("E32M"), _T("E64M"), _T("E128M"), _T("E256M"), _T("E512M"), _T("E1G"), _T("E2G"), _T("E4G"), _T("E8G"), _T("E16G"), _T("E32G"), _T("E64G"), _T("???")};

*/