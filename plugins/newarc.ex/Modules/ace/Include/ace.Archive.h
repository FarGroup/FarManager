#pragma once
#include "ace.h"

class AceArchive {

private:

	string m_strFileName;

	AcePlugin* m_pPlugin;

	ARCHIVECALLBACK m_pfnCallback;
	HANDLE m_hCallback;

	HANDLE m_hArchive;

public:

	AceArchive(AcePlugin* pPlugin, const TCHAR *lpFileName, HANDLE hCallback, ARCHIVECALLBACK pfnCallback, bool bCreate);
	virtual ~AceArchive ();

	const GUID& GetUID();

	bool StartOperation(int nOperation, bool bInternal);
	void EndOperation(int nOperation, bool bInternal);

	int GetArchiveItem(ArchiveItem* pItem);
	void FreeArchiveItem(ArchiveItem* pItem);
	
	bool Extract(const ArchiveItem* pItems, int nItemsNumber, const TCHAR* lpDestDiskPath, const TCHAR* lpPathInArchive);
	bool AddFiles(const ArchiveItem* pItems, int nItemsNumber, const TCHAR* lpSourceDiskPath, const TCHAR* lpPathInArchive);
	bool Delete(const ArchiveItem* pItems, int nItemsNumber);

public:

	LONG_PTR Callback(int nMsg, int nParam1, LONG_PTR nParam2);
};
