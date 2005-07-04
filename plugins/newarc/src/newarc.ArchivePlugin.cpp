#include "newarc.h"

bool ArchivePlugin::Initialize (
		const char *lpModuleName,
		const char *lpLanguage
		)
{
	int nResult;

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
			PluginStartupInfo _Info;
			FARSTANDARDFUNCTIONS _FSF;

			_Info = Info;
			_FSF  = FSF;
			_Info.FSF = &_FSF;

			CreateClassThunk (ArchivePlugin, pGetMsg, m_pfnGetMsgThunk);

			strcpy (_Info.ModuleName, lpModuleName);

			memcpy (&_Info.GetMsg, &m_pfnGetMsgThunk, 4);
			ReloadLanguage (lpLanguage);

            m_pfnPluginEntry (FID_INITIALIZE, (void*)&_Info);

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

	free (m_pfnGetMsgThunk);

	FinalizeLanguageStrings (m_pLanguageStrings, m_nStringsCount);
}

Archive *ArchivePlugin::QueryArchive (
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
		return new Archive (this, lpFileName, QAS.hResult);

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
		int nFormat,
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
			pPlugin->m_ArchivePluginInfo.pFormatInfo[nFormat].lpName
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
		int nFormat,
		int nCommand,
		char *lpCommand
		)
{
	char *lpCommand2 = NULL;

	lpCommand2 = LoadDefaultCommand (this, nFormat, nCommand, lpCommand2);

	if ( !lpCommand2 )
	{
		GetDefaultCommandStruct GDC;

		GDC.nFormat = nFormat;
		GDC.nCommand = nCommand;
		GDC.lpCommand = lpCommand;

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
