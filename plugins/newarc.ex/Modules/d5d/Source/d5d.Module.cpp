#include "d5d.h"

D5DLanguage lng; //bugbug

// {AD5154E7-4837-4AF8-AAE4-69E2D9ED0545}
DEFINE_GUID(CLSID_ModuleD5D, 0xad5154e7, 0x4837, 0x4af8, 0xaa, 0xe4, 0x69, 0xe2, 0xd9, 0xed, 0x5, 0x45);

D5DModule::D5DModule()
{
	m_nCurrentQuery = 0;
	m_pPluginInfo = NULL;
}

bool D5DModule::Load()
{
	string strPluginsPath = Info.ModuleName;

	CutToSlash(strPluginsPath);
	strPluginsPath += _T("Formats");

	FSF.FarRecursiveSearch(strPluginsPath, _T("*.d5d"), (FRSUSERFUNC)LoadD5DPlugins, FRS_RECUR, this);

	m_pPluginInfo = new ArchivePluginInfo[m_Plugins.count()];

	for (int i = 0; i < m_Plugins.count(); i++)
	{
		D5DPlugin* pPlugin = m_Plugins[i];
		ArchivePluginInfo* info = &m_pPluginInfo[i];

		info->dwFlags = 0;
		info->uid = pPlugin->GetUID();
		info->lpModuleName = StrDuplicate(pPlugin->GetModuleName());
		info->uFormats = pPlugin->GetNumberOfFormats();
		info->pFormats = pPlugin->GetFormats();
	}

	string strLanguageFilePath = Info.ModuleName;
	CutToSlash(strLanguageFilePath);

	string strLanguageFileName = strLanguageFilePath;

	string strLanguage;
	apiGetEnvironmentVariable(_T("FARLANG"), strLanguage);

	strLanguageFileName += strLanguage;
	strLanguageFileName += _T(".lng");

	if ( !lng.Load(strLanguageFileName) )
	{
		strLanguageFileName = strLanguageFilePath;
		strLanguageFileName += _T("english.lng");

		lng.Load(strLanguageFileName);
	}

	return (m_Plugins.count() > 0);
}


D5DModule::~D5DModule()
{
	if ( m_pPluginInfo )
	{
		for (int i = 0; i < m_Plugins.count(); i++)
			StrFree((void*)m_pPluginInfo[i].lpModuleName);

		delete [] m_pPluginInfo;
	}
}

const GUID& D5DModule::GetUID()
{
	return CLSID_ModuleD5D;
}

const ArchivePluginInfo* D5DModule::GetPlugins()
{
	return m_pPluginInfo;
}

unsigned int D5DModule::GetNumberOfPlugins()
{
	return m_Plugins.count();
}

const ArchiveQueryResult* D5DModule::QueryArchive(const QueryArchiveStruct* pQAS, bool& bMoreArchives)
{
	if ( m_nCurrentQuery == 0 ) //first time
	{
		m_QueryPool.reset();

		if ( (pQAS->dwFlags & QUERY_FLAG_PLUGIN_UID_READY) == QUERY_FLAG_PLUGIN_UID_READY )
		{
			D5DPlugin* pPlugin = GetPlugin(pQAS->uidPlugin);

			if ( pPlugin )
				pPlugin->QueryArchives(pQAS->lpFileName, m_QueryPool);
		}
		else
		{
			for (int i = 0; i < m_Plugins.count(); i++)
				m_Plugins[i]->QueryArchives(pQAS->lpFileName, m_QueryPool);
		}
	}

	int nQueryCount = m_QueryPool.count();

	const ArchiveQueryResult* pResult = NULL;
	bMoreArchives = false;
	              
	if ( m_nCurrentQuery < nQueryCount )
	{
		pResult = m_QueryPool[m_nCurrentQuery];

		m_nCurrentQuery++;

		bMoreArchives = (m_nCurrentQuery < nQueryCount);
	}

	if ( !bMoreArchives )
		m_nCurrentQuery = 0;

	return pResult;
}

D5DArchive* D5DModule::OpenArchive(
			const GUID& uidPlugin, 
			const GUID& uidFormat, 
			const TCHAR* lpFileName,
			HANDLE hCallback,
			ARCHIVECALLBACK pfnCallback
			)
{
	D5DPlugin* pPlugin = GetPlugin(uidPlugin);

	if ( pPlugin )
		return pPlugin->OpenArchive(uidFormat, lpFileName, hCallback, pfnCallback);

	return NULL;
}

void D5DModule::CloseArchive(const GUID& uidPlugin, D5DArchive* pArchive)
{
	D5DPlugin* pPlugin = GetPlugin(uidPlugin);

	if ( pPlugin )
		pPlugin->CloseArchive(pArchive);
}



int __stdcall D5DModule::LoadD5DPlugins(
		const FAR_FIND_DATA* pFindData,
		const TCHAR* lpFullName,
		D5DModule *pModule
		)
{
	if ( pFindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
		return TRUE;

	GUID uid = CreatePluginUID(pModule->GetUID(), FSF.PointToName(lpFullName));

	D5DPlugin *pPlugin = new D5DPlugin(uid);

	if ( pPlugin )
	{
		if ( pPlugin->Load(lpFullName) )
		{
			pModule->m_Plugins.add(pPlugin);
			return TRUE;
		}

		delete pPlugin;
	}

	return TRUE;
}


D5DPlugin* D5DModule::GetPlugin(const GUID& uid)
{
	for (int i = 0; i < m_Plugins.count(); i++)
	{
		if ( m_Plugins[i]->m_uid == uid )
			return m_Plugins[i];
	}

	return NULL;
}