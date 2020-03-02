#include "newarc.h"

void* __stdcall Allocate (DWORD dwBytes)
{
	return malloc(dwBytes);
}

void __stdcall Free (void *pBlock)
{
	free (pBlock);
}

bool ArchivePlugin::Initialize (
		const char *lpModuleName,
		const char *lpLanguage
		)
{
	m_hModule = LoadLibrary (lpModuleName);

	m_pfnGetMsgThunk = NULL;

	m_pLanguageStrings = NULL;
	m_nStringsCount = 0;

	if ( m_hModule )
	{
		m_lpModuleName = StrDuplicate (lpModuleName);

		// global

		m_pfnPluginEntry = (PLUGINENTRY)GetProcAddress (m_hModule, "PluginEntry");

		if ( m_pfnPluginEntry )
		{
			StartupInfo _si;
			FARSTANDARDFUNCTIONS _FSF;
			Helpers _HF;

			_si.Info = Info;

			_HF.Allocate = Allocate;
			_HF.Free = Free;

			_FSF  = FSF;

			_si.Info.FSF = &_FSF;
			_si.HF = _HF;

			m_pfnGetMsgThunk = CreateThunkFastEx(this, (void *)GetMsgThunk);

			strcpy (_si.Info.ModuleName, lpModuleName);

			memcpy (&_si.Info.GetMsg, &m_pfnGetMsgThunk, sizeof(void*));
			ReloadLanguage (lpLanguage);

            m_pfnPluginEntry (FID_INITIALIZE, (void*)&_si);

			memset (&m_ArchivePluginInfo, 0, sizeof (ArchivePluginInfo));

			m_pfnPluginEntry (FID_GETARCHIVEPLUGININFO, (void*)&m_ArchivePluginInfo);

			return true;
		}
	}

	return false;
}

void ArchivePlugin::Finalize ()
{
	m_pfnPluginEntry (FID_FINALIZE, NULL);

	if ( m_hModule )
		FreeLibrary (m_hModule);

	StrFree (m_lpModuleName);

	ReleaseThunkEx (m_pfnGetMsgThunk);

	FinalizeLanguageStrings (m_pLanguageStrings, m_nStringsCount);
}

pointer_array<Archive*> *ArchivePlugin::QueryArchive (
		const char *lpFileName,
		const char *lpBuffer,
		dword dwBufferSize
		)
{
	QueryArchiveStruct QAS;

	QAS.lpFileName = lpFileName;
	QAS.lpBuffer = lpBuffer;
	QAS.dwBufferSize = dwBufferSize;

	if ( m_pfnPluginEntry (FID_QUERYARCHIVE, &QAS) == NAERROR_SUCCESS )
	{
		int nFormats = QAS.nFormats==-1 ? 1 : QAS.nFormats;
		pointer_array<Archive*> *pArchives = new pointer_array<Archive*>(ARRAY_OPTIONS_DELETE, nFormats);

		if (pArchives)
		{
			if ( QAS.nFormats != -1 )
			{
				QueryArchiveFormatStruct QAFS;

				for (int i=0; i<nFormats; i++)
				{
					QAFS.nFormat = i;
					if (m_pfnPluginEntry (FID_QUERYARCHIVEFORMAT, &QAFS) == NAERROR_SUCCESS)
						pArchives->add(new Archive (this, lpFileName, QAFS.hResult));
				}
			}
			else //if nFormats == -1 then the plugin detected only one archive type and passed it in QAS
			{
				pArchives->add(new Archive (this, lpFileName, QAS.hResult));
			}

			m_pfnPluginEntry (FID_QUERYARCHIVEEND, NULL);

			return pArchives;
		}

		m_pfnPluginEntry (FID_QUERYARCHIVEEND, NULL);
	}

	return NULL;
}

Archive *ArchivePlugin::CreateArchive (
		const GUID &uid,
		const char *lpFileName
		)
{
	CreateArchiveStruct CAS;

	CAS.lpFileName = lpFileName;
	CAS.uid = uid;

	if ( m_pfnPluginEntry (FID_CREATEARCHIVE, &CAS) == NAERROR_SUCCESS )
		return new Archive (this, lpFileName, CAS.hResult);

	return NULL;
}

void ArchivePlugin::FinalizeArchive (
		Archive *pArchive
		)
{
	m_pfnPluginEntry (FID_FINALIZEARCHIVE, (void*)pArchive->m_hArchive);

	delete pArchive;
}


char *LoadDefaultCommand (
		ArchivePlugin *pPlugin,
		const GUID &uid,
		int nCommand,
		char *lpCommand
		)
{
	HKEY hKey;

	char *lpRegKey = StrCreate (260);

	FSF.sprintf (
			lpRegKey,
			"%s\\newarc\\Formats\\%s",
			Info.RootKey,
			GUID2STR (uid)
			);

	if ( RegOpenKeyEx (
			HKEY_CURRENT_USER,
			lpRegKey,
			0,
			KEY_READ,
			&hKey
			) == ERROR_SUCCESS )
	{
		lpCommand = RegQueryStringValueEx (
				hKey,
				pCommandNames[nCommand],
				lpCommand
				);

		RegCloseKey (hKey);
	}

	StrFree (lpRegKey);

	return lpCommand;
}



bool ArchivePlugin::pGetDefaultCommand (
		const GUID &uid,
		int nCommand,
		char *lpCommand
		)
{
	char *lpCommand2 = NULL;

	lpCommand2 = LoadDefaultCommand (this, uid, nCommand, lpCommand2);

	if ( !lpCommand2 )
	{
		GetDefaultCommandStruct GDC;

		GDC.nCommand = nCommand;
		GDC.lpCommand = lpCommand;
		GDC.uid = uid;

		if ( m_pfnPluginEntry (FID_GETDEFAULTCOMMAND, (void*)&GDC) == NAERROR_SUCCESS )
			return GDC.bResult;

		return false;
	}

	strcpy (lpCommand, lpCommand2);

	return true;
}


void ArchivePlugin::ReloadLanguage (
		const char *lpLanguage
		)
{
	FinalizeLanguageStrings (m_pLanguageStrings, m_nStringsCount);

	char *lpPath = StrDuplicate (m_lpModuleName);

	CutToSlash (lpPath);

	if ( !SearchAndLoadLanguageFile (lpPath, lpLanguage, m_pLanguageStrings, m_nStringsCount) )
		if ( !SearchAndLoadLanguageFile(lpPath, "English", m_pLanguageStrings, m_nStringsCount) )
			SearchAndLoadLanguageFile (lpPath, NULL, m_pLanguageStrings, m_nStringsCount);

	StrFree (lpPath);
}


const char * __stdcall ArchivePlugin::pGetMsg (
		int nModuleNumber,
		int nID
		)
{
	if ( nID < m_nStringsCount)
		return m_pLanguageStrings[nID];

	return "NO LNG STRING";
}

const ArchiveFormatInfo* ArchivePlugin::GetArchiveFormatInfo (const GUID &uid)
{
	for (int i = 0; i < m_ArchivePluginInfo.nFormats; i++)
		if ( m_ArchivePluginInfo.pFormatInfo[i].uid == uid )
			return &m_ArchivePluginInfo.pFormatInfo[i];

	return NULL;
}
