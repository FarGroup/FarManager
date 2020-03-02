#include "observer.h"

class ObserverModule {

public:

	int m_nCurrentQuery;

	ObjectArray<ObserverPlugin*> m_Plugins;
	Array<ArchiveQueryResult*> m_QueryPool;

	ArchivePluginInfo* m_pPluginInfo;

public:

	ObserverModule();
	~ObserverModule();

	bool Load();

	const GUID& GetUID();

	unsigned int GetNumberOfPlugins();
	const ArchivePluginInfo* GetPlugins();

	const ArchiveQueryResult* QueryArchive(const QueryArchiveStruct* pQAS, bool& bMoreArchives);

	ObserverArchive* OpenArchive(
				const GUID& uidPlugin, 
				const GUID& uidFormat, 
				const TCHAR* lpFileName, 
				HANDLE hCallback,
				ARCHIVECALLBACK pfnCallback
				);

	void CloseArchive(const GUID& uidPlugin, ObserverArchive* pArchive);

	void GetArchiveModuleInfo(ArchiveModuleInfo *ai);
	ObserverPlugin* GetPlugin(const GUID& uid);

	static int WINAPI LoadObserverPlugins(const FAR_FIND_DATA *pFindData, const TCHAR *lpFullName, ObserverModule* pModule);
};
