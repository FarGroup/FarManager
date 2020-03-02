#pragma once
#include "7z.h"

class SevenZipArchive {

private:

	unsigned int m_uItemsCount;

	SevenZipPlugin* m_pPlugin;

	HANDLE m_hCallback;
	ARCHIVECALLBACK m_pfnCallback;

	string m_strFileName;

	GUID m_uid;
	
	IInArchive* m_pArchive;
	CInFile* m_pFile;

	bool m_bCreated;
	bool m_bOpened;

	Array<ArchiveInfoItem> m_pArchiveInfo;
	bool m_bMultiVolume;

	bool m_bSolid;
	int m_nNumberOfVolumes;

public:

	SevenZipArchive(
			SevenZipPlugin* pPlugin, 
			const GUID& uid, 
			const TCHAR *lpFileName,
			HANDLE hCallback,
			ARCHIVECALLBACK pfnCallback,
			bool bCreated
			);

	IInArchive* GetArchive();
	CInFile* GetFile();

	int GetNumberOfVolumes();
	bool IsSolid();

	const TCHAR* GetFileName();

	void SetItemsCount(unsigned int uItemsCount);
	unsigned int GetItemsCount();

	virtual ~SevenZipArchive();

	const GUID& GetUID();

	bool StartOperation(int nOperation, bool bInternal);
	bool EndOperation(int nOperation, bool bInternal);

	int GetArchiveItem(ArchiveItem* pItem);
	bool GetArchiveItem(unsigned int uIndex, ArchiveItem* pItem);

	bool FreeArchiveItem(ArchiveItem* pItem);

	int GetArchiveInfo(bool& bMultiVolume, const ArchiveInfoItem** pItems);

	int Test(const ArchiveItem* pItems, int nItemsNumber);
	int Delete(const ArchiveItem *pItems, int nItemsNumber);
	int Extract(const ArchiveItem *pItems, int nItemsNumber, const TCHAR* lpDestDiskPath, const TCHAR* lpPathInArchive);
	int AddFiles(const ArchiveItem* pItems, int nItemsNumber, const TCHAR* lpSourceDiskPath, const TCHAR* lpPathInArchive, const TCHAR* lpConfig);

	LONG_PTR OnStartOperation(int nOperation, unsigned __int64 uTotalSize, unsigned __int64 uTotalFiles);
	LONG_PTR OnEnterStage(int nStage);

	LONG_PTR OnProcessFile(const ArchiveItem* pItem, const TCHAR* lpDestName);

	LONG_PTR OnProcessData(
			unsigned __int64 uProcessedBytesFile, 
			unsigned __int64 uTotalBytesFile,
			unsigned __int64 uProcessedBytesTotal,
			unsigned __int64 uTotalBytes
			);

	LONG_PTR OnNeedVolume(const TCHAR* lpSuggestedName, DWORD dwBufferSize, TCHAR* lpBuffer);
	LONG_PTR OnPasswordOperation(int nType, TCHAR* lpBuffer, DWORD dwBufferSize);
	LONG_PTR OnReportError(const ArchiveItem* pItem, int nError);

private:

	bool Open();
	void Close();

	void QueryArchiveInfo();

	LONG_PTR Callback(int nMsg, int nParam1, LONG_PTR nParam2);
};
