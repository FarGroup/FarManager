#include "wcx.h"

class WcxModule {

	ObjectArray<WcxPlugin*> m_Plugins;
	Array<ArchiveQueryResult*> m_QueryPool;

	int m_nCurrentQuery;
	ArchivePluginInfo* m_pPluginInfo;

public:

	WcxModule();
	~WcxModule();

	bool Load();

	const GUID& GetUID();

	unsigned int GetNumberOfPlugins();
	const ArchivePluginInfo* GetPlugins();

	const ArchiveQueryResult* QueryArchive(const QueryArchiveStruct* pQAS, bool& bMoreArchives);

	WcxArchive* OpenCreateArchive(
			const GUID& uidPlugin, 
			const GUID& uidFormat, 
			const TCHAR* lpFileName,
			HANDLE hCallback,
			ARCHIVECALLBACK pfnCallback,
			bool bCreate
			);

	void CloseArchive(const GUID& uidPlugin, WcxArchive* pArchive);

	bool GetDefaultCommand(const GUID &uid, int nCommand, TCHAR *lpCommand);

	WcxPlugin* GetPlugin(const GUID& uid);

	static int WINAPI LoadWcxPlugins(const FAR_FIND_DATA *pFindData, const TCHAR *lpFullName, WcxModule *pModule);
};

