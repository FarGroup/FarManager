#pragma once
#include "ace.h"

class AceModule {

public:

	AcePlugin* m_pPlugin;
	ArchivePluginInfo* m_pPluginInfo;

public:

	AceModule();
	~AceModule();

	bool Load();

	const GUID& GetUID();

	unsigned int GetNumberOfPlugins();
	const ArchivePluginInfo* GetPlugins();

	bool QueryArchive(const QueryArchiveStruct* pQAS, ArchiveQueryResult* pResult);

	AceArchive* OpenCreateArchive(
			const TCHAR* lpFileName,
			HANDLE hCallback,
			ARCHIVECALLBACK pfnCallback,
			bool bCreate
			);

	void CloseArchive(AceArchive* pArchive);
};

