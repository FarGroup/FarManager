#include "observer.h"


class ObserverArchive {

public:

	GUID m_uid;

	ObserverPlugin* m_pPlugin;

	INT_PTR* m_hArchive;
	string m_strFileName;

	ARCHIVECALLBACK m_pfnCallback;
	HANDLE m_hCallback;

	int m_nItemsNumber;
	int m_nIndex;

	bool m_bOpened;

	Array<ArchiveInfoItem> m_pArchiveInfo;
	bool m_bArchiveInfoAdded;

	unsigned __int64 m_uProcessedBytes;
	bool m_bUserAbort;

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
	
	bool Extract(const ArchiveItem *pItems, int nItemsNumber, const TCHAR *lpDestPath, const TCHAR *lpCurrentFolder);

	LONG_PTR Callback (int nMsg, int nParam1, LONG_PTR nParam2);

	int GetArchiveInfo(const ArchiveInfoItem** pItems);

private:

	bool Open();
	void Close();

	static int __stdcall OnExtractStart(ProgressContext* pContext);
	static int __stdcall OnExtractProgress(ProgressContext* pContext);
	static void __stdcall OnExtractEnd(ProgressContext* pContext);
};

