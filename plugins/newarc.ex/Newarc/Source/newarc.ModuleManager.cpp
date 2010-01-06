#include "newarc.h"

ArchiveModuleManager::ArchiveModuleManager(const TCHAR* lpCurrentLanguage)
{
	m_bLoaded = false;
	m_pFilter = new ArchiveFilter(this, true);
	m_strCurrentLanguage = lpCurrentLanguage;
}

bool ArchiveModuleManager::LoadIfNeeded()
{
	if ( !m_bLoaded )
	{
		string strModulesPath = Info.ModuleName;

		CutToSlash(strModulesPath);
		strModulesPath += _T("Plugins"); //BUGBUG, to change

		FSF.FarRecursiveSearch(strModulesPath, _T("*.module"), (FRSUSERFUNC)LoadModules, FRS_RECUR, this);

		CutToSlash(strModulesPath);
		strModulesPath += _T("filters.ini");

		m_pFilter->Load(strModulesPath);

		CutToSlash(strModulesPath);
		strModulesPath += _T("templates.ini");
		
		LoadTemplates(strModulesPath);
	/*	ArchiveFormatInfoArray formats;

		GetFormats(formats);

		__debug(_T("formats - %d"), formats.count());  
		*/
		m_bLoaded = true;
	}

	return m_bLoaded;
}

int __stdcall ArchiveModuleManager::LoadModules(
		const FAR_FIND_DATA* pFindData, 
		const TCHAR* lpFullName, 
		ArchiveModuleManager* pManager
		)
{
	if ( pFindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
		return TRUE;

	ArchiveModule *pModule = new ArchiveModule(pManager);

	if ( pModule )
	{
		if ( pModule->Load(lpFullName, pManager->m_strCurrentLanguage) )
		{
			pManager->m_pModules.add(pModule);
			return TRUE;
		}

		delete pModule;
	}

	return TRUE;
}


ArchiveModuleManager::~ArchiveModuleManager()
{
	delete m_pFilter;
}

ArchiveFilter* ArchiveModuleManager::GetFilter()
{
	return m_pFilter;
}

void ArchiveModuleManager::SetCurrentLanguage(const TCHAR* lpLanguage)
{
	if ( FSF.LStricmp(lpLanguage, m_strCurrentLanguage) )
	{
		for (int i = 0; i < m_pModules.count(); i++)
			m_pModules[i]->ReloadLanguage (lpLanguage);

		m_strCurrentLanguage = lpLanguage;
	}
}


//бля, это пиздец
int ArchiveModuleManager::QueryArchives(
		const TCHAR *lpFileName,
		const unsigned char *pBuffer,
		DWORD dwBufferSize,
		Array<ArchiveFormat*>& result
		)
{
	bool bStopped = false;

	Array<ArchiveFilterEntry*> filters;

	m_pFilter->Reset();
	m_pFilter->QueryFilters(lpFileName, filters, bStopped);

	for (int i = 0; i < filters.count(); i++)
	{
		ArchiveFilterEntry* pFE = filters[i];
		ArchiveModule* pModule = pFE->pModule;

		bool bNoPluginsFiltered = true;
		bool bNoFormatsFiltered = true;
		       
		if ( !m_pFilter->Filtered(&pModule->GetUID(), NULL, NULL) )
		{
			if ( pFE->bAllPlugins )
			{
				Array<ArchivePlugin*>& plugins = pModule->GetPlugins();

				for (int i = 0; i < plugins.count(); i++)
				{
					ArchivePlugin* pPlugin = plugins[i];

					if ( !m_pFilter->Filtered(&pModule->GetUID(), &pPlugin->GetUID(), NULL) )
					{
						if ( pModule->QueryCapability(AMF_SUPPORT_SINGLE_PLUGIN_QUERY) )
						{
							Array<ArchiveFormat*>& formats = pPlugin->GetFormats();

				    		for (int j = 0; j < formats.count(); j++)
				    		{
				    			ArchiveFormat* pFormat = formats[i];
        
	        		    		if ( !m_pFilter->Filtered(&pModule->GetUID(), &pPlugin->GetUID(), &pFormat->GetUID()) )
					    		{
					    			if ( pPlugin->QueryCapability(APF_SUPPORT_SINGLE_FORMAT_QUERY) )
										pModule->QueryArchives(&pPlugin->GetUID(), &pFormat->GetUID(), lpFileName, pBuffer, dwBufferSize, result);
								}
								else
									bNoFormatsFiltered = false;
				    		}

				    		if ( bNoFormatsFiltered && !pPlugin->QueryCapability(APF_SUPPORT_SINGLE_FORMAT_QUERY) )
				    			pModule->QueryArchives(&pPlugin->GetUID(), NULL, lpFileName, pBuffer, dwBufferSize, result);
						}
						else
							bNoPluginsFiltered = false;
					}

					if ( bNoPluginsFiltered && !pModule->QueryCapability(AMF_SUPPORT_SINGLE_PLUGIN_QUERY) )
						pModule->QueryArchives(NULL, NULL, lpFileName, pBuffer, dwBufferSize, result);
				}
			}
			else
			{
				if ( pFE->bAllFormats )
				{
			    	ArchivePlugin* pPlugin = pFE->pPlugin;
				
					if ( !m_pFilter->Filtered(&pModule->GetUID(), &pPlugin->GetUID(), NULL) )
					{
						Array<ArchiveFormat*>& formats = pPlugin->GetFormats();

						for (int i = 0; i < formats.count(); i++)
						{
							ArchiveFormat* pFormat = formats[i];

							if ( !m_pFilter->Filtered(&pModule->GetUID(), &pPlugin->GetUID(), &pFormat->GetUID()) )
							{
								if ( pPlugin->QueryCapability(APF_SUPPORT_SINGLE_FORMAT_QUERY) )
									pModule->QueryArchives(&pPlugin->GetUID(), &pFormat->GetUID(), lpFileName, pBuffer, dwBufferSize, result);
							}
							else
								bNoFormatsFiltered = false;

						}

						if ( bNoFormatsFiltered && !pPlugin->QueryCapability(APF_SUPPORT_SINGLE_FORMAT_QUERY) )
			    			pModule->QueryArchives(&pPlugin->GetUID(), NULL, lpFileName, pBuffer, dwBufferSize, result);
					}
				}
				else
				{
					ArchiveFormat* pFormat = pFE->pFormat;
					ArchivePlugin* pPlugin = pFE->pPlugin;

					if  ( !m_pFilter->Filtered(&pModule->GetUID(), &pPlugin->GetUID(), &pFormat->GetUID()) )
					{
						if ( pPlugin->QueryCapability(APF_SUPPORT_SINGLE_FORMAT_QUERY) )
							pModule->QueryArchives(&pPlugin->GetUID(), &pFormat->GetUID(), lpFileName, pBuffer, dwBufferSize, result);
					}
				}
			}
		}

		m_pFilter->AddStopFilter(filters[i]);						
	}

	if ( !bStopped && m_pFilter->UseRemaining() )
	{
		for (int i = 0; i < m_pModules.count(); i++)
		{
			bool bNoPluginsFiltered = true;
			bool bNoFormatsFiltered = true;

			ArchiveModule* pModule = m_pModules[i]; 

			if ( !m_pFilter->Filtered(&pModule->GetUID(), NULL, NULL) )
			{
				Array<ArchivePlugin*>& plugins = pModule->GetPlugins();

				for (unsigned int j = 0; j < plugins.count(); j++)
				{
					ArchivePlugin* pPlugin = plugins[j];

					if ( !m_pFilter->Filtered(&pModule->GetUID(), &pPlugin->GetUID(), NULL) )
					{
						Array<ArchiveFormat*>& formats = pPlugin->GetFormats();

						for (unsigned int k = 0; k < formats.count(); k++)
						{
							ArchiveFormat* pFormat = formats[k];

							if ( !m_pFilter->Filtered(&pModule->GetUID(), &pPlugin->GetUID(), &pFormat->GetUID()) )
							{
								if ( pPlugin->QueryCapability(APF_SUPPORT_SINGLE_FORMAT_QUERY) )
									pModule->QueryArchives(&pPlugin->GetUID(), &pFormat->GetUID(), lpFileName, pBuffer, dwBufferSize, result);
							}
							else
								bNoFormatsFiltered = false;
						}

						if ( bNoFormatsFiltered && !pPlugin->QueryCapability(APF_SUPPORT_SINGLE_FORMAT_QUERY) )
							pModule->QueryArchives(&pPlugin->GetUID(), NULL, lpFileName, pBuffer, dwBufferSize, result);

					}
					else
						bNoPluginsFiltered = false;
				}

				if ( bNoPluginsFiltered && !pModule->QueryCapability(AMF_SUPPORT_SINGLE_PLUGIN_QUERY) )
					pModule->QueryArchives(NULL, NULL, lpFileName, pBuffer, dwBufferSize, result);
			}
		}
	}

	return 0;
}


ArchiveModule* ArchiveModuleManager::GetModule(const GUID &uid)
{
	for (int i = 0; i < m_pModules.count(); i++)
	{
		if ( m_pModules[i]->GetUID() == uid )
			return m_pModules[i];
	}

	return NULL;
}

int ArchiveModuleManager::GetPlugins(Array<ArchivePlugin*>& plugins)
{
	for (int i = 0; i < m_pModules.count(); i++)
		m_pModules[i]->GetPlugins(plugins);

	return 0;
}

ArchiveFormat* ArchiveModuleManager::GetFormat(const GUID& uidModule, const GUID& uidPlugin, const GUID& uidFormat)
{
	ArchiveModule* pModule = GetModule(uidModule);

	if ( pModule )
		return pModule->GetFormat(uidPlugin, uidFormat);

	return NULL;
}
	

int ArchiveModuleManager::GetFormats(Array<ArchiveFormat*>& formats)
{
	for (int i = 0; i < m_pModules.count(); i++)
		m_pModules[i]->GetFormats(formats);

	return 0;
}

Array<ArchiveModule*>& ArchiveModuleManager::GetModules()
{
	return m_pModules;
}

Archive* ArchiveModuleManager::OpenCreateArchive(
		ArchiveFormat* pFormat, 
		const TCHAR* lpFileName, 
		HANDLE hCallback, 
		ARCHIVECALLBACK pfnCallback,
		bool bCreate
		)
{
	HANDLE hArchive = pFormat->GetModule()->OpenCreateArchive(
				pFormat->GetPlugin()->GetUID(), 
				pFormat->GetUID(),
				lpFileName,
				hCallback,
				pfnCallback,
				bCreate
				);

	if ( hArchive )
		return new Archive(hArchive, pFormat, lpFileName);

	if ( bCreate )
		return new Archive(NULL, pFormat, lpFileName);

	return NULL;
}


void ArchiveModuleManager::CloseArchive(Archive* pArchive)
{
	if ( pArchive->GetHandle() )
		pArchive->GetModule()->CloseArchive(pArchive->GetPlugin()->GetUID(), pArchive->GetHandle());
	
	delete pArchive;
}


bool ArchiveModuleManager::LoadTemplates(const TCHAR* lpFileName)
{
	TCHAR szNames[4096];

	if ( !GetPrivateProfileSectionNames(szNames, 4096, lpFileName) )
		return false;

	m_pTemplates.reset();
	Array<const TCHAR*> names;

	const TCHAR *lpName = szNames;

	while ( *lpName )
	{
		names.add(lpName);
		lpName += _tcslen (lpName)+1;
	}

	for (int i = 0; i < names.count(); i++)
	{
		string strName = names[i];
		string strParams;

		TCHAR* pBuffer = strParams.GetBuffer(260); //BUGBUG
		GetPrivateProfileString(strName, _T("Params"), _T(""), pBuffer, 260, lpFileName);
		strParams.ReleaseBuffer();

		TCHAR szGUID[64];
		
		GetPrivateProfileString(strName, _T("ModuleUID"), _T(""), szGUID, 64, lpFileName);
		GUID uidModule = STR2GUID(szGUID);
		
		GetPrivateProfileString(strName, _T("PluginUID"), _T(""), szGUID, 64, lpFileName);
		GUID uidPlugin = STR2GUID(szGUID);
		
		GetPrivateProfileString(strName, _T("FormatUID"), _T(""), szGUID, 64, lpFileName);
		GUID uidFormat = STR2GUID(szGUID);

		m_pTemplates.add(new ArchiveTemplate(this, strName, strParams, uidModule, uidPlugin, uidFormat));
	}

	return true;
}

bool ArchiveModuleManager::SaveTemplates(const TCHAR* lpFileName)
{
	DeleteFile(lpFileName);

	string strFileName = Info.ModuleName;

	CutToSlash(strFileName);
	strFileName += lpFileName;

	for (int i = 0; i < m_pTemplates.count(); i++)
	{
		ArchiveTemplate* pAT = m_pTemplates[i];

		WritePrivateProfileString(pAT->GetName(), _T("Params"), pAT->GetParams(), strFileName);
		WritePrivateProfileString(pAT->GetName(), _T("ModuleUID"), GUID2STR(pAT->GetModuleUID()), strFileName);
		WritePrivateProfileString(pAT->GetName(), _T("PluginUID"), GUID2STR(pAT->GetPluginUID()), strFileName);
		WritePrivateProfileString(pAT->GetName(), _T("FormatUID"), GUID2STR(pAT->GetFormatUID()), strFileName);
	}

	return true;
}

Array<ArchiveTemplate*>& ArchiveModuleManager::GetTemplates()
{
	return m_pTemplates;	
}