#include "d5d.h"

class D5DArchive {

	GUID m_uid;

	D5DPlugin* m_pPlugin;

	HANDLE m_hArchive;
	string m_strFileName;

	ARCHIVECALLBACK m_pfnCallback;
	HANDLE m_hCallback;

	int m_nItemsNumber;
	int m_nIndex;

	bool m_bOpened;

public:

	D5DArchive (D5DPlugin *pPlugin, const GUID& uid, const TCHAR *lpFileName, HANDLE hCallback, ARCHIVECALLBACK pfnCallback);
	virtual ~D5DArchive ();

	const GUID& GetUID();

	LONG_PTR Callback(int nMsg, int nParam1, LONG_PTR nParam2);

	bool StartOperation(int nOperation, bool bInternal);
	bool EndOperation(int nOperation, bool bInternal);

	int Extract(const ArchiveItem *pItems, int nItemsNumber, const TCHAR *lpDestPath, const TCHAR *lpCurrentFolder);

	int GetArchiveItem(ArchiveItem *pItem);
	void FreeArchiveItem(ArchiveItem* pItem);

private:

	int GetResult(int nSuccessCount, int nItemsNumber, bool bUserAbort);

	bool Open();
	void Close();
};
