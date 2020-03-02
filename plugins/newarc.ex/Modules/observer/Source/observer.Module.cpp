#include "observer.h"

// {B9B29522-3CA7-42BD-A2B1-A06294A5197F}
MY_DEFINE_GUID(CLSID_ModuleObserver, 0xb9b29522, 0x3ca7, 0x42bd, 0xa2, 0xb1, 0xa0, 0x62, 0x94, 0xa5, 0x19, 0x7f);


ObserverModule::ObserverModule()
{
	m_nCurrentQuery = 0;
	m_pPluginInfo = NULL;
}

bool ObserverModule::Load()
{
	string strPluginsPath = Info.ModuleName;

	CutToSlash(strPluginsPath);
	strPluginsPath += _T("Formats");

	FSF.FarRecursiveSearch(strPluginsPath, _T("*.so"), (FRSUSERFUNC)LoadObserverPlugins, FRS_RECUR, this);

	m_pPluginInfo = new ArchivePluginInfo[m_Plugins.count()];

	for (unsigned int i = 0; i < m_Plugins.count(); i++)
	{
		ArchivePluginInfo* info = &m_pPluginInfo[i];
		memset(info, 0, sizeof(ArchivePluginInfo));

		ObserverPlugin* pPlugin = m_Plugins[i];

		info->uid = pPlugin->GetUID();
		info->lpModuleName = StrDuplicate(pPlugin->GetModuleName());
		info->uFormats = pPlugin->GetNumberOfFormats();
		info->pFormats = pPlugin->GetFormats();
		info->dwFlags = 0; //нет смысла поддерживать "запрос одного UID", все равно один плагин == один формат
	}

	return m_Plugins.count() > 0;
}

ObserverModule::~ObserverModule()
{
	if ( m_pPluginInfo )
	{
		for (unsigned int i = 0; i < m_Plugins.count(); i++)
			StrFree((void*)m_pPluginInfo[i].lpModuleName);

		delete [] m_pPluginInfo;
	}
}

const GUID& ObserverModule::GetUID()
{
	return CLSID_ModuleObserver;
}

unsigned int ObserverModule::GetNumberOfPlugins()
{
	return m_Plugins.count();
}

const ArchivePluginInfo* ObserverModule::GetPlugins()
{
	return m_pPluginInfo;
}



const ArchiveQueryResult* ObserverModule::QueryArchive(const QueryArchiveStruct* pQAS, bool& bMoreArchives)
{
	if ( m_nCurrentQuery == 0 ) //first time
	{
		m_QueryPool.reset();

		if ( (pQAS->dwFlags & QUERY_FLAG_PLUGIN_UID_READY) == QUERY_FLAG_PLUGIN_UID_READY )
		{
			ObserverPlugin* pPlugin = GetPlugin(pQAS->uidPlugin);

			if ( pPlugin )
				pPlugin->QueryArchives(pQAS->lpFileName, m_QueryPool);
		}
		else
		{
			for (unsigned int i = 0; i < m_Plugins.count(); i++)
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

ObserverArchive* ObserverModule::OpenArchive(
			const GUID& uidPlugin, 
			const GUID& uidFormat, 
			const TCHAR* lpFileName,
			HANDLE hCallback,
			ARCHIVECALLBACK pfnCallback
			)
{
	ObserverPlugin* pPlugin = GetPlugin(uidPlugin);

	if ( pPlugin )
		return pPlugin->OpenArchive(uidFormat, lpFileName, hCallback, pfnCallback);

	return NULL;
}

void ObserverModule::CloseArchive(const GUID& uidPlugin, ObserverArchive* pArchive)
{
	ObserverPlugin* pPlugin = GetPlugin(uidPlugin);

	if ( pPlugin )
		pPlugin->CloseArchive(pArchive);
}


int __stdcall ObserverModule::LoadObserverPlugins(
		const FAR_FIND_DATA* pFindData,
		const TCHAR* lpFullName,
		ObserverModule *pModule
		)
{
	if ( pFindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
		return TRUE;

	GUID uid = CreatePluginUID(pModule->GetUID(), FSF.PointToName(lpFullName));

	ObserverPlugin *pPlugin = new ObserverPlugin(uid);

	if ( pPlugin )
	{
		if ( pPlugin->Load(lpFullName) )
		{
			pModule->m_Plugins.add(pPlugin);
			return TRUE;
		}

		delete pPlugin;
	}

	return TRUE; //anyway!
}


ObserverPlugin* ObserverModule::GetPlugin(const GUID& uid)
{
	for (unsigned int i = 0; i < m_Plugins.count(); i++)
	{
		if ( m_Plugins[i]->GetUID() == uid )
			return m_Plugins[i];
	}

	return NULL;
}
