#pragma once
#include "newarc.h"

class ArchiveModuleManager {

private:

	ObjectArray<ArchiveModule*> m_pModules;
	string m_strCurrentLanguage;

	bool m_bLoaded;

	ArchiveManagerConfig* m_pConfig;

public:

	ArchiveModuleManager(const TCHAR* lpCurrentLanguage);
	~ArchiveModuleManager();

	bool LoadIfNeeded();

//	ArchiveFilter* GetFilter();
	ArchiveManagerConfig* GetConfig();

	unsigned int GetModules(Array<ArchiveModule*>& modules);
	unsigned int GetPlugins(Array<ArchivePlugin*>& plugins);
	unsigned int GetFormats(Array<ArchiveFormat*>& formats);

	ArchiveModule* GetModule(const GUID& uid);
	ArchivePlugin* GetPlugin(const GUID& uidModule, const GUID& uidPlugin);
	ArchiveFormat* GetFormat(const GUID& uidModule, const GUID& uidPlugin, const GUID& uidFormat);

	int QueryArchives(const TCHAR *lpFileName, const unsigned char *pBuffer, DWORD dwBufferSize, Array<ArchiveFormat*>& result);

	Archive* OpenCreateArchive(
			ArchiveFormat* pFormat, 
			const TCHAR* lpFileName, 
			HANDLE hCallback, 
			ARCHIVECALLBACK pfnCallback,
			bool bCreate
			);

	void CloseArchive(Archive* pArchive);

	bool GetCommand(ArchiveFormat* pFormat, int nCommand, string& strCommand);

	void SetCurrentLanguage(const TCHAR* lpCurrentLanguage, bool bForce = false);

	static int __stdcall LoadModules(const FAR_FIND_DATA* FData, const TCHAR* lpFullName, ArchiveModuleManager* pManager);


};