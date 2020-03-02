#include "wcx.h"

// {71EF7A40-DB57-47BD-81B7-02FC43F7DC9D}
MY_DEFINE_GUID(CLSID_ModuleWCX, 0x71ef7a40, 0xdb57, 0x47bd, 0x81, 0xb7, 0x2, 0xfc, 0x43, 0xf7, 0xdc, 0x9d);


WcxModule::WcxModule()
{
	m_nCurrentQuery = 0;
	m_pPluginInfo = NULL;
}

bool WcxModule::Load()
{
	string strPluginsPath = Info.ModuleName;

	CutToSlash(strPluginsPath);
	strPluginsPath += _T("Formats");

	FSF.FarRecursiveSearch(strPluginsPath, _T("*.wcx"), (FRSUSERFUNC)LoadWcxPlugins, FRS_RECUR, this);

	m_pPluginInfo = new ArchivePluginInfo[m_Plugins.count()];

	for (unsigned int i = 0; i < m_Plugins.count(); i++)
	{
		ArchivePluginInfo* info = &m_pPluginInfo[i];
		memset(info, 0, sizeof(ArchivePluginInfo));

		WcxPlugin* pPlugin = m_Plugins[i];

		info->uid = pPlugin->GetUID();
		info->lpModuleName = StrDuplicate(pPlugin->GetModuleName());
		info->uFormats = pPlugin->GetNumberOfFormats();
		info->pFormats = pPlugin->GetFormats();
		info->dwFlags = 0;
	}

	return m_Plugins.count() > 0;
}

WcxModule::~WcxModule()
{
	if ( m_pPluginInfo )
	{
		for (unsigned int i = 0; i < m_Plugins.count(); i++)
			StrFree((void*)m_pPluginInfo[i].lpModuleName);

		delete [] m_pPluginInfo;			
	}
}

const GUID& WcxModule::GetUID()
{
	return CLSID_ModuleWCX;
}

unsigned int WcxModule::GetNumberOfPlugins()
{
	return m_Plugins.count();
}

const ArchivePluginInfo* WcxModule::GetPlugins()
{
	return m_pPluginInfo;
}


WcxArchive* WcxModule::OpenCreateArchive(
		const GUID& uidPlugin, 
		const GUID& uidFormat, 
		const TCHAR* lpFileName,
		HANDLE hCallback,
		ARCHIVECALLBACK pfnCallback,
		bool bCreate
		)
{
	WcxPlugin* pPlugin = GetPlugin(uidPlugin);

	if ( pPlugin )
		return pPlugin->OpenCreateArchive(uidFormat, lpFileName, hCallback, pfnCallback, bCreate);

	return NULL;
}


void WcxModule::CloseArchive(const GUID& uidPlugin, WcxArchive* pArchive)
{
	WcxPlugin* pPlugin = GetPlugin(uidPlugin);

	if ( pPlugin )
		pPlugin->CloseArchive(pArchive);
}


const ArchiveQueryResult* WcxModule::QueryArchive(const QueryArchiveStruct* pQAS, bool& bMoreArchives)
{
	if ( m_nCurrentQuery == 0 ) //first time
	{
		m_QueryPool.reset();

		if ( (pQAS->dwFlags & QUERY_FLAG_PLUGIN_UID_READY) == QUERY_FLAG_PLUGIN_UID_READY )
		{
			WcxPlugin* pPlugin = GetPlugin(pQAS->uidPlugin);

			if ( pPlugin )
				pPlugin->QueryArchive(pQAS->lpFileName, m_QueryPool);
		}
		else
		{
			for (unsigned int i = 0; i < m_Plugins.count(); i++)
				m_Plugins[i]->QueryArchive(pQAS->lpFileName, m_QueryPool);
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


int __stdcall WcxModule::LoadWcxPlugins(
		const FAR_FIND_DATA* pFindData,
		const TCHAR* lpFullName,
		WcxModule *pModule
		)
{
	if ( pFindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
		return TRUE;

	GUID uid = CreatePluginUID(pModule->GetUID(), FSF.PointToName(lpFullName));

	WcxPlugin *pPlugin = new WcxPlugin(uid);

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


bool WcxModule::GetDefaultCommand(const GUID &uid, int nCommand, TCHAR* lpCommand)
{
	return false;
}

WcxPlugin* WcxModule::GetPlugin(const GUID& uid)
{
	for (unsigned int i = 0; i < m_Plugins.count(); i++)
	{
		if ( m_Plugins[i]->GetUID() == uid )
			return m_Plugins[i];
	}

	return NULL;
}