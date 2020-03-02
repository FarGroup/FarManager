#include "7z.h"

// {4329AE19-DA56-4D1B-8DB9-D985929160BA}
MY_DEFINE_GUID(CLSID_ModuleSevenZip, 0x4329ae19, 0xda56, 0x4d1b, 0x8d, 0xb9, 0xd9, 0x85, 0x92, 0x91, 0x60, 0xba);

SevenZipModule::SevenZipModule()
{
	m_nCurrentQuery = 0;
	m_pPluginInfo = NULL;
}

bool SevenZipModule::Load()
{
	string strPluginsPath = Info.ModuleName;
	
	CutToSlash(strPluginsPath);
	strPluginsPath += _T("Formats");

	FSF.FarRecursiveSearch(
			strPluginsPath, 
			_T("*.dll"), 
			(FRSUSERFUNC)LoadSevenZipPlugins, 
			FRS_RECUR, 
			this
			);

	if ( m_Plugins.count() == 0 ) //load from 7-zip directory, if any
	{
		HKEY hKey;

		if ( RegOpenKey(
				HKEY_CURRENT_USER,
				_T("Software\\7-zip"),
				&hKey
				) == ERROR_SUCCESS )
		{
			strPluginsPath = _T("");
			
			strPluginsPath = apiRegQueryStringValue(hKey, _T("Path"), strPluginsPath);

			FSF.FarRecursiveSearch(
					strPluginsPath, 
					_T("*.dll"), 
					(FRSUSERFUNC)LoadSevenZipPlugins, 
					FRS_RECUR, 
					this
					);

			RegCloseKey(hKey);
		}
	}

	m_pPluginInfo = new ArchivePluginInfo[m_Plugins.count()];

	for (unsigned int i = 0; i < m_Plugins.count(); i++)
	{
		SevenZipPlugin* pPlugin = m_Plugins[i];
		ArchivePluginInfo* info = &m_pPluginInfo[i];

		memset(info, 0, sizeof(ArchivePluginInfo));

		info->uid = pPlugin->GetUID();
		info->dwFlags = APF_SUPPORT_SINGLE_FORMAT_QUERY;
		info->lpModuleName = StrDuplicate(pPlugin->GetModuleName());
		info->uFormats = pPlugin->GetNumberOfFormats();
		info->pFormats = pPlugin->GetFormats();
	}

	return m_Plugins.count() > 0;
}

SevenZipModule::~SevenZipModule()
{
	if ( m_pPluginInfo )
	{
		for (unsigned int i = 0; i < m_Plugins.count(); i++)
			StrFree((void*)m_pPluginInfo[i].lpModuleName);

		delete m_pPluginInfo;
	}
}

const GUID& SevenZipModule::GetUID()
{
	return CLSID_ModuleSevenZip;
}

unsigned int SevenZipModule::GetNumberOfPlugins()
{
	return m_Plugins.count();
}

ArchivePluginInfo* SevenZipModule::GetPlugins()
{
	return m_pPluginInfo;
}

int __stdcall SevenZipModule::LoadSevenZipPlugins(
		const FAR_FIND_DATA* pFindData,
		const TCHAR* lpFullName,
		SevenZipModule *pModule
		)
{
	if ( pFindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
		return TRUE;

	GUID uid = CreatePluginUID(CLSID_ModuleSevenZip, FSF.PointToName(lpFullName));

	SevenZipPlugin *pPlugin = new SevenZipPlugin(uid, lpFullName);

	if ( pPlugin )
	{
		if ( pPlugin->Load() )
		{
			pModule->m_Plugins.add(pPlugin);
			return TRUE;
		}

		delete pPlugin;
	}

	return TRUE;
}


const ArchiveQueryResult* SevenZipModule::QueryArchive(const QueryArchiveStruct* pQAS, bool& bMoreArchives)
{
	if ( m_nCurrentQuery == 0 ) //first time
	{
		m_QueryPool.reset();

		if ( (pQAS->dwFlags & QUERY_FLAG_PLUGIN_UID_READY) == QUERY_FLAG_PLUGIN_UID_READY )
		{
			SevenZipPlugin* pPlugin = GetPlugin(pQAS->uidPlugin);

			if ( pPlugin )
			{
				if ( (pQAS->dwFlags & QUERY_FLAG_FORMAT_UID_READY) == QUERY_FLAG_FORMAT_UID_READY )
					pPlugin->QueryArchive(pQAS->pBuffer, pQAS->dwBufferSize, pQAS->uidFormat, pQAS->lpFileName, m_QueryPool);
				else
					pPlugin->QueryArchives(pQAS->pBuffer, pQAS->dwBufferSize, pQAS->lpFileName, m_QueryPool);
			}
		}
		else
		{
			for (unsigned int i = 0; i < m_Plugins.count(); i++)
				m_Plugins[i]->QueryArchives(pQAS->pBuffer, pQAS->dwBufferSize, pQAS->lpFileName, m_QueryPool);
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


SevenZipPlugin* SevenZipModule::GetPlugin(const GUID& uid)
{
	for (unsigned int i = 0; i < m_Plugins.count(); i++)
	{
		SevenZipPlugin* pPlugin = m_Plugins[i];

		if ( pPlugin->GetUID() == uid )
			return pPlugin;
	}

	return NULL;
}


SevenZipArchive* SevenZipModule::OpenCreateArchive(
			const GUID& uidPlugin, 
			const GUID& uidFormat, 
			const TCHAR* lpFileName,
			HANDLE hCallback,
			ARCHIVECALLBACK pfnCallback,
			bool bCreate
			)
{
	SevenZipPlugin* pPlugin = GetPlugin(uidPlugin);

	if ( pPlugin )
		return pPlugin->OpenCreateArchive(uidFormat, lpFileName, hCallback, pfnCallback, bCreate);

	return NULL;
}

void SevenZipModule::CloseArchive(const GUID& uidPlugin, SevenZipArchive* pArchive)
{
	SevenZipPlugin* pPlugin = GetPlugin(uidPlugin);

	if ( pPlugin )
		pPlugin->CloseArchive(pArchive);
}
