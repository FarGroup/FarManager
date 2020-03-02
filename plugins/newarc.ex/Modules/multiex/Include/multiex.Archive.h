#include "multiex.h"

class MultiExArchive {

	GUID m_uid;

	MultiExPlugin* m_pPlugin;

	int m_hArchive;
	int m_hSearch;

	string m_strFileName;

	ARCHIVECALLBACK m_pfnCallback;
	HANDLE m_hCallback;

	bool m_bOpened;
	int m_nFormatIndex;

public:

	MultiExArchive (MultiExPlugin *pPlugin, const GUID& uid, const TCHAR *lpFileName, HANDLE hCallback, ARCHIVECALLBACK pfnCallback);
	virtual ~MultiExArchive ();

	const GUID& GetUID();

	LONG_PTR Callback(int nMsg, int nParam1, LONG_PTR nParam2);

	bool StartOperation(int nOperation, bool bInternal);
	void EndOperation(int nOperation, bool bInternal);

	int GetArchiveItem(ArchiveItem *pItem);
	void FreeArchiveItem(ArchiveItem* pItem);
	
	bool Extract(const ArchiveItem *pItems, int nItemsNumber, const TCHAR *lpDestPath, const TCHAR *lpCurrentFolder);
	//virtual bool __stdcall pTest (PluginPanelItem *pItems, int nItemsNumber);
	bool Delete(const ArchiveItem *pItems, int nItemsNumber);
	bool AddFiles(const ArchiveItem *pItems, int nItemsNumber, const TCHAR *lpSourcePath, const TCHAR *lpCurrentPath);

private:

	bool Open();
	void Close();
};
