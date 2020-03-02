#pragma once
#include "newarc.h"

class ArchivePlugin {

private:

	ArchiveModule* m_pModule;
	ObjectArray<ArchiveFormat*> m_pFormats;

	string m_strModuleName;
	DWORD m_dwFlags;

	GUID m_uid;

public:

	ArchivePlugin(ArchiveModule* pModule, const ArchivePluginInfo* pInfo);
	~ArchivePlugin();

	const TCHAR* GetModuleName() const;
	const GUID& GetUID() const;

	bool QueryCapability(DWORD dwFlags) const;
	
	ArchiveModule* GetModule();
	ArchiveFormat* GetFormat(const GUID& uid);
	
	int GetFormats(Array<ArchiveFormat*>& formats);
	Array<ArchiveFormat*>& GetFormats();

};
