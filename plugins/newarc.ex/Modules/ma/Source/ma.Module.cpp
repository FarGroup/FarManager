#include "ma.h"

// {BEDAF2F1-09EC-4F9E-A276-46C9E5C00266}
MY_DEFINE_GUID(CLSID_ModuleMA, 0xbedaf2f1, 0x9ec, 0x4f9d, 0xa2, 0x76, 0x46, 0xc9, 0xe5, 0xc0, 0x2, 0x66);


MaModule::MaModule()
{
	m_nCurrentQuery = 0;
	m_pPluginInfo = NULL;

	m_pInfo = NULL;
	m_pFSF = NULL;
}

const GUID& MaModule::GetUID()
{
	return CLSID_ModuleMA;
}

const ArchivePluginInfo* MaModule::GetPlugins()
{
	return m_pPluginInfo;
}

unsigned int MaModule::GetNumberOfPlugins()
{
	return m_Plugins.count();
}

bool MaModule::Load()
{
	string strPluginsPath = Info.ModuleName;

#ifdef UNICODE
	CutToSlash(strPluginsPath);

	strPluginsPath += _T("ansi.dll");

	bool bAnsiModuleLoaded = false;
	GETPLUGINSSTARTUPINFO pfnGetPluginStartupInfo = nullptr;



	if ( Info.PluginsControl(
			INVALID_HANDLE_VALUE, 
			2/*PCTL_FORCEDLOADPLUGIN*/,  //BUGBUG
			PLT_PATH, 
			(LONG_PTR)strPluginsPath.GetString()
			) )
	{
		HMODULE hAnsiModule = LoadLibraryEx(
				strPluginsPath, 
				NULL, 
				LOAD_WITH_ALTERED_SEARCH_PATH
				);

		if ( hAnsiModule )
		{
			pfnGetPluginStartupInfo = (GETPLUGINSSTARTUPINFO)GetProcAddress(hAnsiModule, "GetPluginStartupInfo");

			if ( pfnGetPluginStartupInfo && pfnGetPluginStartupInfo(&m_pInfo, &m_pFSF) )
				bAnsiModuleLoaded = true;

			FreeLibrary(hAnsiModule);
		}
	}

	if ( !bAnsiModuleLoaded )
	{
		__debug(pfnGetPluginStartupInfo ? _T("ansi.dll was not loaded before ma.module") : _T("error with ansi.dll"));
		return false;
	}

#else
	m_pInfo = (oldfar::PluginStartupInfo*)&Info;
	m_pFSF = (oldfar::FARSTANDARDFUNCTIONS*)&FSF;
#endif


	CutToSlash(strPluginsPath);
	strPluginsPath += _T("Formats");

	FSF.FarRecursiveSearch(strPluginsPath, _T("*.fmt"), (FRSUSERFUNC)LoadMaPlugins, FRS_RECUR, this);

	m_pPluginInfo = new ArchivePluginInfo[m_Plugins.count()];

	for (unsigned int i = 0; i < m_Plugins.count(); i++)
	{
		MaPlugin *pPlugin = m_Plugins[i];
		ArchivePluginInfo *info = &m_pPluginInfo[i];

		info->uid = pPlugin->GetUID();
		info->uFormats = pPlugin->GetNumberOfFormats();
		info->pFormats = pPlugin->GetFormats();
		info->lpModuleName = StrDuplicate(pPlugin->GetModuleName());
		info->dwFlags = 0; //не поддерживаем запрос отдельных форматов
	}

	return m_Plugins.count() > 0;
}

MaModule::~MaModule()
{
	if ( m_pPluginInfo )
	{
		for (unsigned int i = 0; i < m_Plugins.count(); i++)
			StrFree((void*)m_pPluginInfo[i].lpModuleName);

		delete m_pPluginInfo;
	}
}



const ArchiveQueryResult* MaModule::QueryArchive(const QueryArchiveStruct* pQAS, bool& bMoreArchives)
{
	if ( m_nCurrentQuery == 0 ) //first time
	{
		m_QueryPool.reset();

		if ( (pQAS->dwFlags & QUERY_FLAG_PLUGIN_UID_READY) == QUERY_FLAG_PLUGIN_UID_READY )
		{
			MaPlugin* pPlugin = GetPlugin(pQAS->uidPlugin);

			if ( pPlugin )
				pPlugin->QueryArchives(pQAS->pBuffer, pQAS->dwBufferSize, pQAS->lpFileName, m_QueryPool);
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

MaArchive* MaModule::OpenArchive(
		const GUID& uidPlugin, 
		const GUID& uidFormat, 
		const TCHAR* lpFileName,
		HANDLE hCallback,
		ARCHIVECALLBACK pfnCallback
		)
{
	MaPlugin* pPlugin = GetPlugin(uidPlugin);

	if ( pPlugin )
		return pPlugin->OpenArchive(uidFormat, lpFileName, hCallback, pfnCallback);

	return NULL;
}

void MaModule::CloseArchive(const GUID& uidPlugin, MaArchive* pArchive)
{
	MaPlugin* pPlugin = GetPlugin(uidPlugin);

	if ( pPlugin )
		pPlugin->CloseArchive(pArchive);
}

int __stdcall MaModule::LoadMaPlugins(
		const FAR_FIND_DATA* pFindData,
		const TCHAR* lpFullName,
		MaModule *pModule
		)
{
	if ( pFindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
		return TRUE;

	GUID uid = CreatePluginUID(pModule->GetUID(), FSF.PointToName(lpFullName));

	MaPlugin *pPlugin = new MaPlugin(uid);

	if ( pPlugin )
	{
		if ( pPlugin->Load(lpFullName, pModule->m_pInfo, pModule->m_pFSF) )
		{
			pModule->m_Plugins.add(pPlugin);
			return TRUE;
		}

		delete pPlugin;
	}

	return TRUE;
}


MaPlugin* MaModule::GetPlugin(const GUID& uid)
{
	for (unsigned int i = 0; i < m_Plugins.count(); i++)
	{
		if ( m_Plugins[i]->GetUID() == uid )
			return m_Plugins[i];
	}

	return NULL;
}