#include "newarc.h"

ArchiveModuleManager::ArchiveModuleManager(const TCHAR* lpCurrentLanguage)
{
	m_bLoaded = false;
	m_strCurrentLanguage = lpCurrentLanguage;

	if ( m_strCurrentLanguage.IsEmpty() )
		m_strCurrentLanguage = _T("English");

	m_pConfig = new ArchiveManagerConfig(this);
}

ArchiveManagerConfig* ArchiveModuleManager::GetConfig()
{
	return m_pConfig;
}

bool ArchiveModuleManager::LoadIfNeeded()
{
	if ( !m_bLoaded )
	{
		string strModulesPath = Info.ModuleName;

		CutToSlash(strModulesPath);
		strModulesPath += _T("Modules"); 

		FSF.FarRecursiveSearch(strModulesPath, _T("*.module"), (FRSUSERFUNC)LoadModules, FRS_RECUR, this);

		m_pConfig->Load();

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
	delete m_pConfig;
}


void ArchiveModuleManager::SetCurrentLanguage(const TCHAR* lpLanguage, bool bForce)
{
	if ( bForce || FSF.LStricmp(lpLanguage, m_strCurrentLanguage) )
	{
		for (unsigned int i = 0; i < m_pModules.count(); i++)
			m_pModules[i]->ReloadLanguage(lpLanguage);

		m_strCurrentLanguage = lpLanguage;
	}
}


//бля, это пиздец
int ArchiveModuleManager::QueryArchives(
		const TCHAR* lpFileName,
		const unsigned char* pBuffer,
		DWORD dwBufferSize,
		Array<ArchiveFormat*>& result
		)
{
	bool bStopped = false;

	//BADBAD
	if ( !dwBufferSize )
	{
		result.reset();
		return 0;
	};

	ArchiveFilter* pFilter = m_pConfig->GetFilter();

	Array<ArchiveFilterEntry*> filters;

	pFilter->Reset();
	pFilter->QueryFilters(lpFileName, filters, bStopped);

	for (unsigned int i = 0; i < filters.count(); i++)
	{
		ArchiveFilterEntry* pFE = filters[i];
		ArchiveModule* pModule = pFE->GetModule();

		bool bNoPluginsFiltered = true;
		bool bNoFormatsFiltered = true;

		if ( !pFilter->Filtered(&pModule->GetUID(), NULL, NULL) )
		{
			if ( pFE->IsAllPlugins() )
			{
				Array<ArchivePlugin*>& plugins = pModule->GetPlugins();

				for (unsigned int i = 0; i < plugins.count(); i++)
				{
					ArchivePlugin* pPlugin = plugins[i];

					if ( !pFilter->Filtered(&pModule->GetUID(), &pPlugin->GetUID(), NULL) )
					{
						if ( pModule->QueryCapability(AMF_SUPPORT_SINGLE_PLUGIN_QUERY) )
						{
							Array<ArchiveFormat*>& formats = pPlugin->GetFormats();

							for (unsigned int j = 0; j < formats.count(); j++)
							{
								ArchiveFormat* pFormat = formats[i];

								if ( !pFilter->Filtered(&pModule->GetUID(), &pPlugin->GetUID(), &pFormat->GetUID()) )
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
				if ( pFE->IsAllFormats() )
				{
					ArchivePlugin* pPlugin = pFE->GetPlugin();
				
					if ( !pFilter->Filtered(&pModule->GetUID(), &pPlugin->GetUID(), NULL) )
					{
						Array<ArchiveFormat*>& formats = pPlugin->GetFormats();

						for (unsigned int i = 0; i < formats.count(); i++)
						{
							ArchiveFormat* pFormat = formats[i];

							if ( !pFilter->Filtered(&pModule->GetUID(), &pPlugin->GetUID(), &pFormat->GetUID()) )
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
					ArchiveFormat* pFormat = pFE->GetFormat();
					ArchivePlugin* pPlugin = pFE->GetPlugin();

					if ( !pFilter->Filtered(&pModule->GetUID(), &pPlugin->GetUID(), &pFormat->GetUID()) )
					{
						if ( pPlugin->QueryCapability(APF_SUPPORT_SINGLE_FORMAT_QUERY) )
							pModule->QueryArchives(&pPlugin->GetUID(), &pFormat->GetUID(), lpFileName, pBuffer, dwBufferSize, result);
					}
				}
			}
		}

		pFilter->AddStopFilter(filters[i]);
	}

	if ( !bStopped && pFilter->UseRemaining() )
	{
		for (unsigned int i = 0; i < m_pModules.count(); i++)
		{
			bool bNoPluginsFiltered = true;
			bool bNoFormatsFiltered = true;

			ArchiveModule* pModule = m_pModules[i]; 

			if ( !pFilter->Filtered(&pModule->GetUID(), NULL, NULL) )
			{
				Array<ArchivePlugin*>& plugins = pModule->GetPlugins();

				for (unsigned int j = 0; j < plugins.count(); j++)
				{
					ArchivePlugin* pPlugin = plugins[j];

					if ( !pFilter->Filtered(&pModule->GetUID(), &pPlugin->GetUID(), NULL) )
					{
						Array<ArchiveFormat*>& formats = pPlugin->GetFormats();

						for (unsigned int k = 0; k < formats.count(); k++)
						{
							ArchiveFormat* pFormat = formats[k];

							if ( !pFilter->Filtered(&pModule->GetUID(), &pPlugin->GetUID(), &pFormat->GetUID()) )
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
	for (unsigned int i = 0; i < m_pModules.count(); i++)
	{
		if ( m_pModules[i]->GetUID() == uid )
			return m_pModules[i];
	}

	return NULL;
}

unsigned int ArchiveModuleManager::GetModules(Array<ArchiveModule*>& modules)
{
	for (unsigned int i = 0; i < m_pModules.count(); i++)
		modules.add(m_pModules[i]);

	return 0;
}

unsigned int ArchiveModuleManager::GetPlugins(Array<ArchivePlugin*>& plugins)
{
	for (unsigned int i = 0; i < m_pModules.count(); i++)
		m_pModules[i]->GetPlugins(plugins);

	return 0;
}

ArchivePlugin* ArchiveModuleManager::GetPlugin(const GUID& uidModule, const GUID& uidPlugin)
{
	ArchiveModule* pModule = GetModule(uidModule);

	if ( pModule )
		return pModule->GetPlugin(uidPlugin);

	return NULL;
}


ArchiveFormat* ArchiveModuleManager::GetFormat(const GUID& uidModule, const GUID& uidPlugin, const GUID& uidFormat)
{
	ArchiveModule* pModule = GetModule(uidModule);

	if ( pModule )
		return pModule->GetFormat(uidPlugin, uidFormat);

	return NULL;
}
	

unsigned int ArchiveModuleManager::GetFormats(Array<ArchiveFormat*>& formats)
{
	for (unsigned int i = 0; i < m_pModules.count(); i++)
		m_pModules[i]->GetFormats(formats);

	return 0;
}

/*
Array<ArchiveModule*>& ArchiveModuleManager::GetModules()
{
	return m_pModules;
}
*/

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


bool ArchiveModuleManager::GetCommand(
		ArchiveFormat* pFormat,
		int nCommand,
		string& strCommand
		)
{
	if ( !pFormat )
		return false;

	if ( m_pConfig->GetCommand(pFormat, nCommand, strCommand) )
		return true;

	bool bEnabled;

	if ( pFormat->GetDefaultCommand(
			nCommand, 
			strCommand, 
			bEnabled
			) && bEnabled && !strCommand.IsEmpty() )
		return true;
	
	return false;
}

/*
bool ArchiveModuleManager::SaveConfigs(const TCHAR* lpFileName)
{
	DeleteFile(lpFileName);

	string strFileName = Info.ModuleName;

	CutToSlash(strFileName);
	strFileName += lpFileName;

	TiXmlDocument doc;

	TiXmlElement* root = new TiXmlElement("configs");

	for (unsigned int i = 0; i < m_pConfigs.count(); i++)
	{
		ArchiveFormat* pAC = m_pConfigs[i];
		TiXmlElement* tpl = new TiXmlElement("config");

		pAC->ToXml(*tpl);

		root->LinkEndChild(tpl);
	}

	doc.LinkEndChild(root);
	doc.SaveFile((FakeUtf8String)lpFileName);

	return true;
}



bool ArchiveModuleManager::LoadConfigs(const TCHAR* lpFileName)
{
	m_pConfigs.reset();

	TiXmlDocument doc;

	doc.LoadFile((FakeUtf8String)lpFileName);

	TiXmlHandle handle(&doc);

	TiXmlNode* pNode = handle.FirstChild("configs").ToNode();

	if ( pNode )
	{
		TiXmlNode* pChild = pNode->FirstChild("config");

		while ( pChild )
		{
			ArchiveFormatConfig* pAC = ArchiveFormanConfig::FromXml(this, pChild);
			m_pConfigs.add(pAC);

			pChild = pChild->NextSibling("config");
		}
	}

	return true;
}


bool ArchiveModuleManager::LoadTemplates(const TCHAR* lpFileName)
{
	m_pTemplates.reset();

	TiXmlDocument doc;

	doc.LoadFile((FakeUtf8String)lpFileName);

	TiXmlHandle handle(&doc);

	TiXmlNode* pNode = handle.FirstChild("templates").ToNode();

	if ( pNode )
	{
		TiXmlNode* pChild = pNode->FirstChild("template");

		while ( pChild )
		{
			ArchiveTemplate* pAT = ArchiveTemplate::LoadFromXml(this, pChild);
			m_pTemplates.add(pAT);

			pChild = pChild->NextSibling("template");
		}
	}

	return true;
}

bool ArchiveModuleManager::SaveTemplates(const TCHAR* lpFileName)
{
	DeleteFile(lpFileName);

	string strFileName = Info.ModuleName;

	CutToSlash(strFileName);
	strFileName += lpFileName;

	TiXmlDocument doc;

	TiXmlElement* root = new TiXmlElement("templates");

	for (unsigned int i = 0; i < m_pTemplates.count(); i++)
	{
		ArchiveTemplate* pAT = m_pTemplates[i];
		TiXmlElement* tpl = new TiXmlElement("template");

		pAT->ToXml(*tpl);

		root->LinkEndChild(tpl);
	}

	doc.LinkEndChild(root);
	doc.SaveFile((FakeUtf8String)lpFileName);

	return true;
}

*/