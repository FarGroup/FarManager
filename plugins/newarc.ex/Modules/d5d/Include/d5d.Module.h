#include "d5d.h"

class D5DModule {

	ObjectArray<D5DPlugin*> m_Plugins;

	Array<ArchiveQueryResult*> m_QueryPool;

	int m_nCurrentQuery;

	ArchivePluginInfo* m_pPluginInfo;

public:

	D5DModule();
	~D5DModule();

	bool Load();

	const GUID& GetUID();

	unsigned int GetNumberOfPlugins();
	const ArchivePluginInfo* GetPlugins();

	const ArchiveQueryResult* QueryArchive(const QueryArchiveStruct* pQAS, bool& bMoreArchives);

	D5DArchive* OpenArchive(
			const GUID& uidPlugin, 
			const GUID& uidFormat, 
			const TCHAR *lpFileName,
			HANDLE hCallback,
			ARCHIVECALLBACK pfnCallback
			);

	void CloseArchive(const GUID& uidPlugin, D5DArchive* pArchive);

	D5DPlugin* GetPlugin(const GUID& uid);

	static int WINAPI LoadD5DPlugins(const FAR_FIND_DATA *pFindData, const TCHAR *lpFullName, D5DModule *pModule);
};

