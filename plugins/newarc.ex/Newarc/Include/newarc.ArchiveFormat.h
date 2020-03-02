#pragma once
#include "newarc.h"

class ArchiveFormat {

private:

	ArchivePlugin* m_pPlugin;

	string m_strDefaultExtention;
	string m_strName;

	DWORD m_dwFlags;

	GUID m_uid;

public:

	ArchiveFormat(ArchivePlugin* pPlugin, const ArchiveFormatInfo* pInfo);
	~ArchiveFormat();

	const GUID& GetUID() const;

	const TCHAR* GetDefaultExtention() const;
	const TCHAR* GetName() const;

	ArchivePlugin* GetPlugin();
	ArchiveModule* GetModule();

	bool QueryCapability(DWORD dwFlags) const;

	//CHECK USAGE!!! do not use outside manager & config
	bool GetDefaultCommand(int nCommand, string& strCommand, bool& bEnabled);
	bool Configure(const TCHAR* lpInitialConfig, string& strResultConfig);
};


