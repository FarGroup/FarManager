#include "newarc.h"

ArchiveFilter::ArchiveFilter(ArchiveModuleManager* pManager, bool bUseRemaining)
{
	m_bUseRemaining = bUseRemaining;
	m_pManager = pManager;
}


bool ArchiveFilter::UseRemaining()
{
	return m_bUseRemaining;
}

void ArchiveFilter::SetRemaining(bool bUseRemaining)
{
	m_bUseRemaining = bUseRemaining;
}

ArchiveFilter::~ArchiveFilter()
{
}

void ArchiveFilter::Clear()
{
	m_pFilters.reset();
	m_pStopFilters.reset();
}

void ArchiveFilter::Reset()
{
	m_pStopFilters.reset();
}

void ArchiveFilter::AddFilter(ArchiveFilterEntry* pFE)
{
	m_pFilters.add(pFE);
}


int ArchiveFilter::GetFilters(Array<ArchiveFilterEntry*>& filters)
{
	for (int i = 0; i < m_pFilters.count(); i++)
		filters.add(m_pFilters[i]);

	return 0;
}

int ArchiveFilter::QueryFilters(const TCHAR* lpFileName, ArchiveFilterArray& filters, bool& bStopped)
{
	bStopped = false;

	for (int i = 0; i < m_pFilters.count(); i++)
	{
		ArchiveFilterEntry* pFE = m_pFilters[i];

		if ( !pFE->bEnabled || pFE->bInvalid )
			continue;

#ifdef UNICODE
		if ( FSF.ProcessName(pFE->strMask, (TCHAR*)lpFileName, 0, PN_CMPNAME|PN_SKIPPATH) )
#else
		if ( FSF.ProcessName(pFE->strMask, (TCHAR*)lpFileName, PN_CMPNAME|PN_SKIPPATH) )
#endif
		{
			if ( !pFE->bExcludeFilter )
				filters.add(pFE);
			else
				m_pStopFilters.add(pFE);

			if ( !pFE->bContinue )
			{
				bStopped = true;
				break;
			}
		}
	}

	return 0;
}


bool ArchiveFilter::Filtered(const GUID* puidModule, const GUID* puidPlugin, const GUID* puidFormat)
{
	for (int i = 0; i < m_pStopFilters.count(); i++)
	{
		ArchiveFilterEntry* pFE = m_pStopFilters[i];

		if ( !pFE->bEnabled || pFE->bInvalid )
			continue;

		bool bFormatFiltered = (pFE->bAllFormats || (puidFormat && (pFE->uidFormat == *puidFormat)));
		bool bPluginFiltered = bFormatFiltered && (pFE->bAllPlugins || (puidPlugin && (pFE->uidPlugin == *puidPlugin)));
		bool bModuleFiltered = bPluginFiltered && (pFE->bAllModules || (puidModule && (pFE->uidModule == *puidModule)));

		if ( bModuleFiltered )
			return true;
	}

	return false;
}

bool ArchiveFilter::Load(const TCHAR* lpFileName)
{
	TCHAR szNames[4096]; //бля, пора, пора уходить в XML!!!

	if ( !GetPrivateProfileSectionNames(szNames, 4096, lpFileName) )
		return false;

	Array<const TCHAR*> names;

	Clear();

	const TCHAR *lpName = szNames;

	while ( *lpName )
	{
		names.add(lpName);			
		lpName += _tcslen (lpName)+1;
	}

	for (unsigned int i = 0; i < names.count(); i++)
	{
		ArchiveFilterEntry *pFE = new ArchiveFilterEntry;

		pFE->bAllModules = true;
		pFE->bAllPlugins = true;
		pFE->bAllFormats = true;

		pFE->strName = names[i];

		TCHAR* pBuffer = pFE->strMask.GetBuffer(260); //херня

		GetPrivateProfileString (pFE->strName, _T("Mask"), _T(""), pBuffer, 260, lpFileName);

		pFE->strMask.ReleaseBuffer();

		pFE->bExcludeFilter = GetPrivateProfileInt(pFE->strName, _T("ExcludeFilter"), 0, lpFileName);
		pFE->bEnabled = GetPrivateProfileInt(pFE->strName, _T("Enabled"), 0, lpFileName);
		pFE->bAllModules = GetPrivateProfileInt(pFE->strName, _T("AllModules"), 0, lpFileName);
		pFE->bContinue = GetPrivateProfileInt(pFE->strName, _T("Continue"), 0, lpFileName);

		if ( !pFE->bAllModules )
		{
			TCHAR szGUID[64];

			GetPrivateProfileString (pFE->strName, _T("ModuleUID"), _T(""), szGUID, 64, lpFileName);

			pFE->uidModule = STR2GUID(szGUID);
			pFE->pModule = m_pManager->GetModule(pFE->uidModule);

			if ( !pFE->pModule )
				pFE->bInvalid = true;
			
			pFE->bAllPlugins = GetPrivateProfileInt(pFE->strName, _T("AllPlugins"), 0, lpFileName);

			if ( !pFE->bAllPlugins )
			{
				GetPrivateProfileString(pFE->strName, _T("PluginUID"), _T(""), szGUID, 64, lpFileName);

				pFE->uidPlugin = STR2GUID(szGUID);
			
			    if ( !pFE->bInvalid )
					pFE->pPlugin = pFE->pModule->GetPlugin(pFE->uidPlugin);

				if ( !pFE->pPlugin )
					pFE->bInvalid = true;
					
				pFE->bAllFormats = GetPrivateProfileInt(pFE->strName, _T("AllFormats"), 0, lpFileName);
		
				if ( !pFE->bAllFormats )
				{
					GetPrivateProfileString(pFE->strName, _T("FormatUID"), _T(""), szGUID, 64, lpFileName);

					pFE->uidFormat = STR2GUID(szGUID);

					if ( !pFE->bInvalid )
						pFE->pFormat = pFE->pPlugin->GetFormat(pFE->uidFormat);

					if ( !pFE->pFormat )
						pFE->bInvalid = true;
				}
			}
		}

		m_pFilters.add(pFE);
	}

	return true;
}

bool ArchiveFilter::Save(const TCHAR* lpFileName)
{
	DeleteFile(lpFileName);

	for (unsigned int i = 0; i < m_pFilters.count(); i++)
	{
		TCHAR sz[32];
		ArchiveFilterEntry* pFE = m_pFilters[i];

		WritePrivateProfileString(pFE->strName, _T("Mask"), pFE->strMask, lpFileName);

		_itot(pFE->bEnabled, sz, 10);
		WritePrivateProfileString(pFE->strName, _T("Enabled"), sz, lpFileName);

		_itot(pFE->bExcludeFilter, sz, 10);
		WritePrivateProfileString(pFE->strName, _T("ExcludeFilter"), sz, lpFileName);

		_itot(pFE->bContinue, sz, 10);
		WritePrivateProfileString(pFE->strName, _T("Continue"), sz, lpFileName);

		if ( pFE->bAllModules )
		{
			_itot(pFE->bAllModules, sz, 10);
			WritePrivateProfileString(pFE->strName, _T("AllModules"), sz, lpFileName);
		}
		else
		{
			WritePrivateProfileString(pFE->strName, _T("ModuleUID"), GUID2STR(pFE->uidModule), lpFileName);
		
			if ( pFE->bAllPlugins )
			{
				_itot(pFE->bAllPlugins, sz, 10);
				WritePrivateProfileString(pFE->strName, _T("AllPlugins"), sz, lpFileName);
			}
			else
			{
				WritePrivateProfileString(pFE->strName, _T("PluginUID"), GUID2STR(pFE->uidPlugin), lpFileName);
				
				if ( pFE->bAllFormats )
				{
					_itot(pFE->bAllFormats, sz, 10);
					WritePrivateProfileString(pFE->strName, _T("AllFormats"), sz, lpFileName);
				}
				else
					WritePrivateProfileString(pFE->strName, _T("FormatUID"), GUID2STR(pFE->uidFormat), lpFileName);
			}
		}
	}

	return true;

}

void ArchiveFilter::AddStopFilter(ArchiveFilterEntry* pFE)
{
	m_pStopFilters.add(pFE);
}