#include "wcx.h"

class WcxArchive {

	GUID m_uid;

	WcxPlugin* m_pPlugin;

	HANDLE m_hArchive;
	string m_strFileName;

	bool bProcessDataProc;

	//to check thunks
	PBYTE m_pfnProcessData;
	PBYTE m_pfnChangeVol;

	PBYTE m_pfnProcessDataW;
	PBYTE m_pfnChangeVolW;

	ARCHIVECALLBACK m_pfnCallback;
	HANDLE m_hCallback;

	bool m_bUserAbort;
	int m_nSuccessCount;

public:

	WcxArchive(
			WcxPlugin *pPlugin, 
			const GUID& uid, 
			const TCHAR *lpFileName, 
			HANDLE hCallback, 
			ARCHIVECALLBACK pfnCallback,
			bool bCreate
			);

	virtual ~WcxArchive();

	const GUID& GetUID();

	LONG_PTR Callback(int nMsg, int nParam1, LONG_PTR nParam2);

	bool StartOperation(int nOperation, bool bInternal);
	bool EndOperation(int nOperation, bool bInternal);

	int GetArchiveItem (ArchiveItem* pItem);
	void FreeArchiveItem(ArchiveItem* pItem);

	int Extract(const ArchiveItem* pItems, int nItemsNumber, const TCHAR *lpDestDiskPath, const TCHAR* lpPathInArchive);
	//virtual bool __stdcall pTest (PluginPanelItem *pItems, int nItemsNumber);
	int Delete(const ArchiveItem *pItems, int nItemsNumber);
	int AddFiles(const ArchiveItem *pItems, int nItemsNumber, const TCHAR* lpSourceDiskPath, const TCHAR* lpPathInArchive);


private: 

	int GetResult(int nItemsNumber);

	int __stdcall OnProcessData(const char *FileName, int Size);
	int __stdcall OnChangeVol(const char *ArcName, int Mode);

	int __stdcall OnProcessDataW(const wchar_t *FileName, int Size);
	int __stdcall OnChangeVolW(const wchar_t *ArcName, int Mode);

	static int __stdcall OnChangeVolThunk(const char* ArcName, int Mode);
	static int __stdcall OnProcessDataThunk(const char* FileName, int Size);
	static int __stdcall OnChangeVolThunkW(const wchar_t *ArcName, int Mode);
	static int __stdcall OnProcessDataThunkW(const wchar_t *FileName, int Size);
};
