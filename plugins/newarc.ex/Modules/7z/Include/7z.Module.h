#include "7z.h"

class SevenZipModule {
private:

	int m_nCurrentQuery;

	ObjectArray<SevenZipPlugin*> m_Plugins;
	Array<ArchiveQueryResult*> m_QueryPool;

	ArchivePluginInfo* m_pPluginInfo;

public:

	SevenZipModule();
	virtual ~SevenZipModule();

	bool Load();

	const GUID& GetUID();
	SevenZipPlugin* GetPlugin(const GUID& uid);

	const ArchiveQueryResult* QueryArchive(const QueryArchiveStruct* pQAS, bool& bMoreArchives);

	SevenZipArchive* OpenCreateArchive(
				const GUID& uidPlugin, 
				const GUID& uidFormat, 
				const TCHAR* lpFileName, 
				HANDLE hCallback,
				ARCHIVECALLBACK pfnCallback,
				bool bCreate
				);

	void CloseArchive(const GUID& uidPlugin, SevenZipArchive* pArchive);

	unsigned int GetNumberOfPlugins();
	ArchivePluginInfo* GetPlugins();


	static int __stdcall LoadSevenZipPlugins(const FAR_FIND_DATA* pFindData, const TCHAR* lpFullName, SevenZipModule *pModule);
};