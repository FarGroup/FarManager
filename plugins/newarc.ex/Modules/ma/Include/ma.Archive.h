#pragma once
#include "ma.h"

class MaArchive {

	GUID m_uid;

	MaPlugin* m_pPlugin;

	HANDLE m_hArchive;
	string m_strFileName;

	ARCHIVECALLBACK m_pfnCallback;
	HANDLE m_hCallback;

	Array<ArchiveInfoItem> m_pArchiveInfo;

	bool m_bItemInfoAdded;
	bool m_bArchiveInfoAdded;

public:

	MaArchive (MaPlugin *pPlugin, const GUID& uid, const TCHAR *lpFileName, HANDLE hCallback, ARCHIVECALLBACK pfnCallback);
	virtual ~MaArchive ();

	const GUID& GetUID();

	LONG_PTR Callback(int nMsg, int nParam1, LONG_PTR nParam2);

	bool StartOperation(int nOperation, bool bInternal);
	bool EndOperation(int nOperation, bool bInternal);

	int GetArchiveItem(ArchiveItem *pItem);
	void FreeArchiveItem(ArchiveItem* pItem);

	int GetArchiveInfo(const ArchiveInfoItem** pItems);
};
