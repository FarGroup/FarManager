#include "multiex.h"

class MultiExModule {

	ObjectArray<MultiExPlugin*> m_Plugins;
	Array<ArchiveQueryResult*> m_QueryPool;

	int m_nCurrentQuery;

	ArchivePluginInfo* m_pPluginInfo;

public:

	MultiExModule();
	~MultiExModule();

	bool Load();

	const GUID& GetUID();

	unsigned int GetNumberOfPlugins();
	const ArchivePluginInfo* GetPlugins();

	const ArchiveQueryResult* QueryArchive(const QueryArchiveStruct* pQAS, bool& bMoreArchives);

	MultiExArchive* OpenCreateArchive(
			const GUID& uidPlugin,
			const GUID& uidFormat,
			const TCHAR* lpFileName,
			HANDLE hCallback,
			ARCHIVECALLBACK pfnCallback,
			bool bCreate
			);

	void CloseArchive(const GUID& uidPlugin, MultiExArchive* pArchive);
	MultiExPlugin* GetPlugin(const GUID& uidPlugin);

	static int WINAPI LoadMultiExPlugins(const FAR_FIND_DATA *pFindData, const TCHAR *lpFullName, MultiExModule *pModule);
};

