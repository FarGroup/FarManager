#include "observer.h"

/*
struct ObserverHandle {
	INT_PTR* hArchive;
	ObserverArchive pArchive;
};
*/

class ObserverArchive {

public:

	GUID m_uid;

	ObserverPlugin* m_pPlugin;
	StorageGeneralInfo m_Info;

	HANDLE m_hArchive;
	string m_strFileName;

	ARCHIVECALLBACK m_pfnCallback;
	HANDLE m_hCallback;

	unsigned int m_uNumberOfFiles;

	int m_nIndex;
	bool m_bOpened;

	Array<ArchiveInfoItem> m_pArchiveInfo;
	bool m_bArchiveInfoAdded;

	unsigned __int64 m_uProcessedBytes;

public:

	ObserverArchive(
			ObserverPlugin* pPlugin, 
			const GUID& uid, 
			const TCHAR* lpFileName, 
			HANDLE hCallback, 
			ARCHIVECALLBACK pfnCallback
			);

	~ObserverArchive();

	const GUID& GetUID();

	bool StartOperation(int nOperation, bool bInternal);
	bool EndOperation(int nOperation, bool bInternal);

	int GetArchiveItem(ArchiveItem *pItem);
	bool FreeArchiveItem(ArchiveItem* pItem);
	
	int Extract(const ArchiveItem *pItems, int nItemsNumber, const TCHAR *lpDestPath, const TCHAR *lpCurrentFolder);

	LONG_PTR Callback(int nMsg, int nParam1, LONG_PTR nParam2);

	int GetArchiveInfo(const ArchiveInfoItem** pItems);

private:

	int GetResult(int nSuccessCount, int nItemsNumber, bool bUserAbort);

	bool Open();
	void Close();

	static int __stdcall OnExtractProgress(ProgressContextEx* pContext, unsigned __int64 uProcessedBytes);
};

