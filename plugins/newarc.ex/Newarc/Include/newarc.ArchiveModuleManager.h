#pragma once
#include "newarc.h"

class ArchiveModuleManager {

private:

	ObjectArray<ArchiveTemplate*> m_pTemplates;
	ObjectArray<ArchiveModule*> m_pModules;

	string m_strCurrentLanguage;

	ArchiveFilter* m_pFilter;

	bool m_bLoaded;

public:

	ArchiveModuleManager(const TCHAR* lpCurrentLanguage);
	~ArchiveModuleManager();

	bool LoadIfNeeded();

	ArchiveFilter* GetFilter();
	ArchiveModule* GetModule(const GUID& uid);

	Array<ArchiveModule*>& GetModules();
	Array<ArchiveTemplate*>& GetTemplates();
	
	int GetPlugins(Array<ArchivePlugin*>& plugins);
	int GetFormats(Array<ArchiveFormat*>& formats);

	ArchiveFormat* GetFormat(const GUID& uidModule, const GUID& uidPlugin, const GUID& uidFormat);
	ArchivePlugin* GetPlugin(const GUID& uidModule, const GUID& uidPlugin);

	int QueryArchives(const TCHAR *lpFileName, const unsigned char *pBuffer, DWORD dwBufferSize, Array<ArchiveFormat*>& result);

	Archive* OpenCreateArchive(
			ArchiveFormat* pFormat, 
			const TCHAR* lpFileName, 
			HANDLE hCallback, 
			ARCHIVECALLBACK pfnCallback,
			bool bCreate
			);

	void CloseArchive(Archive* pArchive);

	bool GetDefaultCommand(ArchiveFormat* pFormat, int nCommand, string& strCommand);

	void SetCurrentLanguage(const TCHAR* lpCurrentLanguage, bool bForce = false);

	static int __stdcall LoadModules(const FAR_FIND_DATA* FData, const TCHAR* lpFullName, ArchiveModuleManager* pManager);

	bool LoadTemplates(const TCHAR* lpFileName);
	bool SaveTemplates(const TCHAR* lpFileName);

	bool LoadCommands(const TCHAR* lpFileName);
	bool SaveCommands(const TCHAR* lpFileName);

};