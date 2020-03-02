#include "multiex.h"

// {BEDAF2F1-09EC-4F9E-A276-46C9E5C00266}
MY_DEFINE_GUID(CLSID_ModuleMultiEx, 0xbedaf2f1, 0x9ec, 0x4f9e, 0xa2, 0x76, 0x46, 0xc9, 0xe5, 0xc0, 0x2, 0x66);


MultiExModule::MultiExModule()
{
	m_nCurrentQuery = 0;
	m_pPluginInfo = NULL;
}

const GUID& MultiExModule::GetUID()
{
	return CLSID_ModuleMultiEx;
}

bool MultiExModule::Load()
{
	string strPluginsPath = Info.ModuleName;

	CutToSlash(strPluginsPath);
	strPluginsPath += _T("Formats");

	FSF.FarRecursiveSearch(strPluginsPath, _T("*.*"), (FRSUSERFUNC)LoadMultiExPlugins, FRS_RECUR, this);

	m_pPluginInfo = new ArchivePluginInfo[m_Plugins.count()];

	for (int i = 0; i < m_Plugins.count(); i++)
	{
		MultiExPlugin *pPlugin = m_Plugins[i];
		ArchivePluginInfo *info = &m_pPluginInfo[i];

		info->uid = pPlugin->GetUID();
		info->pFormats = pPlugin->GetFormats();
		info->uFormats = pPlugin->GetNumberOfFormats();
		info->lpModuleName = StrDuplicate(pPlugin->GetModuleName());
		info->dwFlags = APF_SUPPORT_SINGLE_FORMAT_QUERY;
	}

	return m_Plugins.count() > 0;
}

MultiExModule::~MultiExModule()
{
	if ( m_pPluginInfo )
	{
		for (int i = 0; i < m_Plugins.count(); i++)
			StrFree((void*)m_pPluginInfo[i].lpModuleName);

		delete m_pPluginInfo;
	}
}

unsigned int MultiExModule::GetNumberOfPlugins()
{
	return m_Plugins.count();
}

const ArchivePluginInfo* MultiExModule::GetPlugins()
{
	return m_pPluginInfo;
}


const ArchiveQueryResult* MultiExModule::QueryArchive(const QueryArchiveStruct* pQAS, bool& bMoreArchives)
{
	if ( m_nCurrentQuery == 0 ) //first time
	{
		m_QueryPool.reset();

		if ( (pQAS->dwFlags & QUERY_FLAG_PLUGIN_UID_READY) == QUERY_FLAG_PLUGIN_UID_READY )
		{
			MultiExPlugin* pPlugin = GetPlugin(pQAS->uidPlugin);

			if ( pPlugin )
			{
				if ( (pQAS->dwFlags & QUERY_FLAG_FORMAT_UID_READY) == QUERY_FLAG_FORMAT_UID_READY )
					pPlugin->QueryArchive(pQAS->uidFormat, pQAS->lpFileName, m_QueryPool);
				else
					pPlugin->QueryArchives(pQAS->lpFileName, m_QueryPool);
			}
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

MultiExArchive* MultiExModule::OpenCreateArchive(
		const GUID& uidPlugin, 
		const GUID& uidFormat,
		const TCHAR* lpFileName,
		HANDLE hCallback,
		ARCHIVECALLBACK pfnCallback,
		bool bCreate
		)
{
	MultiExPlugin* pPlugin = GetPlugin(uidPlugin);

	if ( pPlugin )
		return pPlugin->OpenCreateArchive(uidFormat, lpFileName, hCallback, pfnCallback, bCreate);

	return NULL;
}

void MultiExModule::CloseArchive(const GUID& uidPlugin, MultiExArchive* pArchive)
{
	MultiExPlugin* pPlugin = GetPlugin(uidPlugin);

	if ( pPlugin )
		return pPlugin->CloseArchive(pArchive);
}


int __stdcall MultiExModule::LoadMultiExPlugins(
		const FAR_FIND_DATA* pFindData,
		const TCHAR* lpFullName,
		MultiExModule *pModule
		)
{
	if ( pFindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
		return TRUE;

	GUID uid = CreatePluginUID(pModule->GetUID(), FSF.PointToName(lpFullName));

	MultiExPlugin *pPlugin = new MultiExPlugin(uid);

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


MultiExPlugin* MultiExModule::GetPlugin(const GUID& uidPlugin)
{
	for (int i = 0; i < m_Plugins.count(); i++)
	{
		if ( m_Plugins[i]->GetUID() == uidPlugin )
			return m_Plugins[i];
	}

	return NULL;
}