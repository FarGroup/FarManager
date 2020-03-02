#pragma once
#include "ma.h"

#ifdef UNICODE
typedef bool (__stdcall *GETPLUGINSSTARTUPINFO)(oldfar::PluginStartupInfo** ppInfo, oldfar::FARSTANDARDFUNCTIONS** pFSF);
#endif

class MaModule {

	ObjectArray<MaPlugin*> m_Plugins;
	Array<ArchiveQueryResult*> m_QueryPool;

	int m_nCurrentQuery;

	ArchivePluginInfo* m_pPluginInfo;

	oldfar::PluginStartupInfo* m_pInfo;
	oldfar::FARSTANDARDFUNCTIONS* m_pFSF;

public:

	MaModule();
	~MaModule();

	bool Load();

	const GUID& GetUID();

	unsigned int GetNumberOfPlugins();
	const ArchivePluginInfo* GetPlugins();

	const ArchiveQueryResult* QueryArchive(const QueryArchiveStruct* pQAS, bool& bMoreArchives);
	
	MaArchive* OpenArchive(
			const GUID& uidPlugin, 
			const GUID& uidFormat, 
			const TCHAR* lpFileName, 
			HANDLE hCallback,
			ARCHIVECALLBACK pfnCallback
			);

	void CloseArchive(const GUID& uidPlugin, MaArchive* pArchive);	
	MaPlugin* GetPlugin(const GUID& uid);

	static int WINAPI LoadMaPlugins(const FAR_FIND_DATA *pFindData, const TCHAR *lpFullName, MaModule *pModule);
};

