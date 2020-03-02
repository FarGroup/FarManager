#include <Rtl.Base.h>
#include <FarPluginBase.hpp>
#include <debug.h>
#include "wcx.class.h"

MY_DEFINE_GUID (CLSID_TemplateWCX, 0x66B22359, 0x4332, 0x4257, 0xA1, 0x1A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

unsigned long CRC32(
		unsigned long crc,
		const char *buf,
		unsigned int len
		)
{
	static unsigned long crc_table[256];

	if (!crc_table[1])
	{
		unsigned long c;
		int n, k;

		for (n = 0; n < 256; n++)
		{
			c = (unsigned long)n;
			for (k = 0; k < 8; k++) c = (c >> 1) ^ (c & 1 ? 0xedb88320L : 0);
				crc_table[n] = c;
		}
	}

	crc = crc ^ 0xffffffffL;
	while (len-- > 0) {
		crc = crc_table[(crc ^ (*buf++)) & 0xff] ^ (crc >> 8);
	}

	return crc ^ 0xffffffffL;
}


const char *GUID2STR (const GUID &uid)
{
	static char szGUID[64];
	LPOLESTR string;

	StringFromIID (uid, &string);

	//int length = wcslen (string)+1;
	//char *result = StrCreate (length);

	memset (&szGUID, 0, sizeof (szGUID));

	WideCharToMultiByte (CP_OEMCP, 0, string, -1, (char*)&szGUID, sizeof (szGUID), NULL, NULL);

	CoTaskMemFree (string);

	return (const char*)&szGUID;
}


bool GetGUIDFromModule (const WcxModule *pModule, WORD wFormat, GUID *puid)
{
	if ( puid && pModule )
	{
		memcpy (puid, &CLSID_TemplateWCX, sizeof (GUID));

		memcpy ((unsigned char*)puid+10, &pModule->m_dwCRC, 4);
		memcpy ((unsigned char*)puid+14, &wFormat, 2);

		return true;
	}

	return false;
}


WcxModule::WcxModule (const char *lpFileName)
{
	m_lpModuleName = StrDuplicate (lpFileName);

	FSF.LStrupr (m_lpModuleName);

	const char *lpName = FSF.PointToName (m_lpModuleName);

	m_dwCRC = CRC32 (0, lpName, StrLength(lpName));

	m_hModule = LoadLibraryEx (
			lpFileName,
			NULL,
			LOAD_WITH_ALTERED_SEARCH_PATH
			);

	if ( m_hModule )
	{
		m_pfnOpenArchive=(PLUGINOPENARCHIVE)GetProcAddress(m_hModule,"OpenArchive");
		m_pfnCloseArchive=(PLUGINCLOSEARCHIVE)GetProcAddress(m_hModule,"CloseArchive");
		m_pfnReadHeader=(PLUGINREADHEADER)GetProcAddress(m_hModule,"ReadHeader");
		m_pfnProcessFile=(PLUGINPROCESSFILE)GetProcAddress(m_hModule,"ProcessFile");
		m_pfnPackFiles=(PLUGINPACKFILES)GetProcAddress(m_hModule,"PackFiles");
		m_pfnDeleteFiles=(PLUGINDELETEFILES)GetProcAddress(m_hModule,"DeleteFiles");
		m_pfnSetChangeVolProc=(PLUGINSETCHANGEVOLPROC)GetProcAddress(m_hModule,"SetChangeVolProc");
		m_pfnSetProcessDataProc=(PLUGINSETPROCESSDATAPROC)GetProcAddress(m_hModule,"SetProcessDataProc");
		m_pfnConfigurePacker=(PLUGINCONFIGUREPACKER)GetProcAddress(m_hModule,"ConfigurePacker");
		m_pfnGetPackerCaps=(PLUGINGETPACKERCAPS)GetProcAddress(m_hModule,"GetPackerCaps");
		m_pfnCanYouHandleThisFile=(PLUGINCANYOUHANDLETHISFILE)GetProcAddress(m_hModule,"CanYouHandleThisFile");
		m_pfnPackSetDefaultParams=(PLUGINPACKSETDEFAULTPARAMS)GetProcAddress(m_hModule,"PackSetDefaultParams");
	}

	if ( LoadedOK() )
	{
		if (m_pfnPackSetDefaultParams)
		{
			PackDefaultParamStruct dps;

			dps.size = sizeof(dps);
			dps.PluginInterfaceVersionLow = 10;
			dps.PluginInterfaceVersionHi = 2;
			strcpy(dps.DefaultIniName,"");

			m_pfnPackSetDefaultParams(&dps);
		}
	}
}

bool WcxModule::LoadedOK ()
{
	if ( !(m_hModule && m_pfnOpenArchive && m_pfnCloseArchive && m_pfnReadHeader && m_pfnProcessFile) )
		return false;

	return true;
}

WcxModule::~WcxModule ()
{
	FreeLibrary (m_hModule);

	StrFree (m_lpModuleName);
}


WcxModules::WcxModules ()
{
	memset (&m_PluginInfo, 0, sizeof (m_PluginInfo));

	char *lpPluginsPath = StrDuplicate (Info.ModuleName, 260);

	CutToSlash (lpPluginsPath);

	strcat (lpPluginsPath, "Formats");

	m_Modules.create(ARRAY_OPTIONS_DELETE);

	FSF.FarRecursiveSearch(lpPluginsPath,"*.wcx",(FRSUSERFUNC)LoadWcxModules,FRS_RECUR,this);

	StrFree(lpPluginsPath);

	m_PluginInfo.pFormatInfo = (ArchiveFormatInfo *) realloc (m_PluginInfo.pFormatInfo, sizeof (ArchiveFormatInfo) * m_PluginInfo.nFormats);

	GUID uid;
	int index = 0;

	for (int i = 0; i < m_Modules.count(); i++)
	{
		WcxModule *pModule = m_Modules[i];
		WcxPluginInfo *info = &pModule->m_Info;

		GetGUIDFromModule(pModule, 0, &uid);

		m_PluginInfo.pFormatInfo[index].uid = uid;
		m_PluginInfo.pFormatInfo[index].dwFlags = AFF_SUPPORT_INTERNAL_EXTRACT|AFF_SUPPORT_INTERNAL_TEST;
		m_PluginInfo.pFormatInfo[index].lpName = info->Name;
		m_PluginInfo.pFormatInfo[index].lpDefaultExtention = info->DefaultExtention;

		index++;
	}
}

WcxModules::~WcxModules ()
{
	if ( m_PluginInfo.pFormatInfo )
		free (m_PluginInfo.pFormatInfo);

	m_Modules.free ();
}

WcxModule *WcxModules::IsArchive (QueryArchiveStruct *pQAS, int *nModuleNum)
{
	WcxModule *TrueArc = NULL;

	*nModuleNum = -1;

	for (int i=0; i<m_Modules.count(); i++)
	{
		if ( m_Modules[i]->m_pfnCanYouHandleThisFile )
		{
			if (m_Modules[i]->m_pfnCanYouHandleThisFile (pQAS->lpFileName))
			{
				TrueArc = m_Modules[i];
				*nModuleNum = i;
				break;
			}
		}
		else
		{
			tOpenArchiveData OpenArchiveData = {0};

			OpenArchiveData.ArcName = (char *)pQAS->lpFileName;
			OpenArchiveData.OpenMode = PK_OM_LIST;

			HANDLE hArchive = m_Modules[i]->m_pfnOpenArchive (&OpenArchiveData);

			if (hArchive)
			{
				m_Modules[i]->m_pfnCloseArchive (hArchive);
				TrueArc = m_Modules[i];
				*nModuleNum = i;
				break;
			}
		}
	}

	return TrueArc;
}

int WINAPI WcxModules::LoadWcxModules (const WIN32_FIND_DATA *pFindData,
		const char *lpFullName,
		WcxModules *pModules
		)
{
	if ( pFindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
		return TRUE;

	WcxModule *pModule = new WcxModule (lpFullName);

	if ( !pModule )
		return TRUE;

	if ( !pModule->LoadedOK () )
	{
		delete pModule;
		return TRUE;
	}

	WcxPluginInfo *info = &pModule->m_Info;

	strcpy(info->Name,FSF.PointToName(lpFullName));
	*strrchr(info->Name,'.') = 0;
	strcpy(info->DefaultExtention, info->Name);
	strcat(info->Name," [wcx]");

	pModules->m_PluginInfo.nFormats++;
	pModules->m_Modules.add (pModule);

	return TRUE;
}

void WcxModules::GetArchivePluginInfo (ArchivePluginInfo *ai)
{
	ai->nFormats = m_PluginInfo.nFormats;
	ai->pFormatInfo = m_PluginInfo.pFormatInfo;
}

bool WcxModules::GetDefaultCommand (const GUID &uid, int nCommand, char *lpCommand)
{
	return false;
}

int __stdcall WcxArchive::ProcessDataProc (char *FileName, int Size)
{
	if ( m_pfnCallback && bProcessDataProc )
		return (int)m_pfnCallback (AM_PROCESS_DATA, 0, (LONG_PTR)Size);

	return 1;
}

int __stdcall WcxArchive::SetChangeVolProc (char *ArcName, int Mode)
{
	return 1;
}


int __stdcall SetChangeVolProcThunk (char *ArcName, int Mode)
{
	WcxArchive *p = (WcxArchive*)THUNK_MAGIC;
	return p->SetChangeVolProc(ArcName, Mode);
}


int __stdcall ProcessDataProcThunk (char *FileName, int Size)
{
	WcxArchive *p = (WcxArchive*)THUNK_MAGIC;
	return p->ProcessDataProc(FileName, Size);
}


WcxArchive::WcxArchive (WcxModule *pModule, int nModuleNum, const char *lpFileName)
{
	m_pModule = pModule;

	m_lpFileName = StrDuplicate (lpFileName);
	OemToChar (m_lpFileName, m_lpFileName);

	m_nModuleNum = nModuleNum;
	m_hArchive = NULL;

	bProcessDataProc = false;

	m_pfnProcessDataProc = CreateThunkFastEx(this, (void *)ProcessDataProcThunk);
	m_pfnSetChangeVolProc = CreateThunkFastEx(this, (void *)SetChangeVolProcThunk);

	m_pfnCallback = NULL;
}

WcxArchive::~WcxArchive ()
{
	StrFree (m_lpFileName);

	ReleaseThunkEx (m_pfnProcessDataProc);
	ReleaseThunkEx (m_pfnSetChangeVolProc);
}

int WcxArchive::ConvertResult (int nResult)
{
	switch (nResult)
	{
		case E_END_ARCHIVE:
			return E_EOF;
		case E_BAD_ARCHIVE:
			return E_BROKEN;
		case E_BAD_DATA:
			return E_UNEXPECTED_EOF;
		case E_EREAD:
			return E_READ_ERROR;
	}

	return E_UNEXPECTED_EOF;
}

bool __stdcall WcxArchive::pOpenArchive (int nOpMode, ARCHIVECALLBACK pfnCallback)
{
	m_pfnCallback = pfnCallback;

	m_hArchive = NULL;

	tOpenArchiveData OpenArchiveData;

	memset (&OpenArchiveData, 0, sizeof (OpenArchiveData));

	OpenArchiveData.ArcName = m_lpFileName;
	OpenArchiveData.OpenMode = (nOpMode == OM_LIST)?PK_OM_LIST:PK_OM_EXTRACT;

	SetFileApisToANSI();
	m_hArchive = m_pModule->m_pfnOpenArchive (&OpenArchiveData);
	SetFileApisToOEM();

	if ( m_hArchive )
	{
		if (m_pModule->m_pfnSetProcessDataProc)
			m_pModule->m_pfnSetProcessDataProc (m_hArchive, (tProcessDataProc)m_pfnProcessDataProc);

		if (m_pModule->m_pfnSetChangeVolProc)
			m_pModule->m_pfnSetChangeVolProc (m_hArchive, (tChangeVolProc)m_pfnSetChangeVolProc);

		return true;
	}

	return false;
}

void __stdcall WcxArchive::pCloseArchive ()
{
	m_pModule->m_pfnCloseArchive (m_hArchive);
}

int __stdcall WcxArchive::pGetArchiveItem (ArchiveItemInfo *pItem)
{
	int nResult = 0;

	tHeaderData HeaderData;
	memset (&HeaderData, 0, sizeof (HeaderData));

	nResult = m_pModule->m_pfnReadHeader (m_hArchive, &HeaderData);

	if ( !nResult )
	{
		CharToOem(HeaderData.FileName,pItem->pi.FindData.cFileName);

		pItem->pi.FindData.dwFileAttributes = HeaderData.FileAttr;
		pItem->pi.FindData.nFileSizeLow = HeaderData.UnpSize;

		FILETIME filetime;
		DosDateTimeToFileTime ((WORD)((DWORD)HeaderData.FileTime >> 16), (WORD)HeaderData.FileTime, &filetime);
		LocalFileTimeToFileTime (&filetime, &pItem->pi.FindData.ftLastWriteTime);
		pItem->pi.FindData.ftCreationTime = pItem->pi.FindData.ftLastAccessTime = pItem->pi.FindData.ftLastWriteTime;

		pItem->pi.PackSize = HeaderData.PackSize;
		pItem->pi.CRC32 = HeaderData.FileCRC;

		m_pModule->m_pfnProcessFile (m_hArchive, PK_SKIP, NULL, NULL);

		return E_SUCCESS;
	}

	return ConvertResult (nResult);
}

void  __stdcall WcxArchive::pGetArchiveType (GUID *puid)
{
	GetGUIDFromModule (m_pModule, 0, puid); //??? 0????
}

void CreateDirectoryEx (char *FullPath) //$ 16.05.2002 AA
{
//	if( !FileExists (FullPath) )
	{
		for (char *c = FullPath; *c; c++)
		{
			if(*c!=' ')
			{
				for(; *c; c++)
					if(*c=='\\')
						{
							*c=0;
							CreateDirectory(FullPath, NULL);
							*c='\\';
						}
				CreateDirectory(FullPath, NULL);
				break;
			}
		}
	}
}

void CreateDirs (const char *lpFileName)
{
	char *lpNameCopy = StrDuplicate (lpFileName);

	CutToSlash (lpNameCopy);

	CreateDirectoryEx (lpNameCopy);

	StrFree (lpNameCopy);
}



bool __stdcall WcxArchive::pExtract (
		PluginPanelItem *pItems,
		int nItemsNumber,
		const char *lpDestPath,
		const char *lpCurrentFolder
		)
{
	int nProcessed = 0;
	int nResult = 0;

	bool bFound;

	char *lpDestName  = StrCreate (260);
	char *lpDestNameA = StrCreate (260);
	char *lpCurrentFileName = StrCreate (260);

	Callback (AM_START_OPERATION, OPERATION_EXTRACT, 0);

	while ( /*m_hArchive &&*/ nResult == 0 )
	{
		tHeaderData HeaderData;
		memset (&HeaderData, 0, sizeof (HeaderData));

		nResult = m_pModule->m_pfnReadHeader (m_hArchive, &HeaderData);

		if ( nResult == 0 )
		{
			CharToOem (HeaderData.FileName, lpCurrentFileName);

			bFound = false;

			int i = 0;

			for (; i < nItemsNumber; i++)
			{
				if ( !strcmp (pItems[i].FindData.cFileName, lpCurrentFileName) )
				{
					OemToChar (lpDestPath, lpDestNameA);
					strcpy (lpDestName, lpDestPath);

					FSF.AddEndSlash (lpDestName);
					FSF.AddEndSlash (lpDestNameA);

					char *lpName  = lpCurrentFileName;
					char *lpNameA = HeaderData.FileName;

					if ( *lpCurrentFolder /*&& !strncmp (
							lpName,
							lpCurrentFolder,
							strlen (lpCurrentFolder)
							)*/ )
					{
						lpName  += strlen (lpCurrentFolder);
						lpNameA += strlen (lpCurrentFolder);
				  }

					if ( *lpName == '\\' )
					{
						lpName++;
						lpNameA++;
					}

					strcat (lpDestName,  lpName);
					strcat (lpDestNameA, lpNameA);

					ProcessFileStruct pfs;

					pfs.pItem = &pItems[i];
					pfs.lpDestFileName = lpDestName;

					Callback (AM_PROCESS_FILE, 0, (LONG_PTR)&pfs);

					int nProcessResult = 0;

					bProcessDataProc = true;

					if ( (HeaderData.FileAttr & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY )
					{
						CreateDirectoryEx (lpDestName);
						nProcessResult = m_pModule->m_pfnProcessFile (m_hArchive, PK_SKIP, NULL, NULL);
					}
					else
					{
						CreateDirs(lpDestName);
						SetFileApisToANSI();
						nProcessResult = m_pModule->m_pfnProcessFile (m_hArchive, PK_EXTRACT, NULL, lpDestNameA);
						SetFileApisToOEM();
					}

					bProcessDataProc = false;

					if ( !nProcessResult  )
						nProcessed++;

					if ( /*m_bAborted ||*/ nProcessResult || (nProcessed == nItemsNumber) )
						goto l_1;

					bFound = true;

					break;
				}
			}

			if ( !bFound )
				m_pModule->m_pfnProcessFile (m_hArchive, PK_SKIP, NULL, NULL);
		}
	}

l_1:

	StrFree (lpDestName);
	StrFree (lpDestNameA);
	StrFree (lpCurrentFileName);

	return nProcessed!=0;
}

LONG_PTR WcxArchive::Callback (int nMsg, int nParam1, LONG_PTR nParam2)
{
	if ( m_pfnCallback )
		return m_pfnCallback (nMsg, nParam1, nParam2);

	return FALSE;
}
