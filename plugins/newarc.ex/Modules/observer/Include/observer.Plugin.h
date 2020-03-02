#include "observer.h"

class ObserverPlugin{

private:

	GUID m_uid;

	string m_strModuleName;

	LoadSubModuleFunc m_pfnLoadSubModule;
	UnloadSubModuleFunc m_pfnUnloadSubModule;

	OpenStorageFunc m_pfnOpenStorage;
	CloseStorageFunc m_pfnCloseStorage;
	GetItemFunc m_pfnGetStorageItem;
	ExtractFunc m_pfnExtract;

	HMODULE m_hModule;

	ArchiveFormatInfo* m_pFormatInfo;

public:

	ObserverPlugin(const GUID& uid);
	virtual ~ObserverPlugin();

	bool Load(const TCHAR* lpModuleName);

	const GUID& GetUID();
	const TCHAR* GetModuleName();

	unsigned int GetNumberOfFormats();
	const ArchiveFormatInfo* GetFormats();


	int QueryArchives(const TCHAR* lpFileName, Array<ArchiveQueryResult*>& result); 

	ObserverArchive* OpenArchive(const GUID& uid, const TCHAR* lpFileName, HANDLE hCallback, ARCHIVECALLBACK pfnCallback);
	void CloseArchive(ObserverArchive* pArchive);

//observer
	HANDLE OpenStorage(const TCHAR* lpFileName, StorageGeneralInfo* pInfo);
	
	void CloseStorage(HANDLE hArchive);
	int GetStorageItem(HANDLE hArchive, int nIndex, ArchiveItem* pItem, unsigned int& uNumberOfFiles);
	
	int ExtractItem(
			HANDLE hArchive, 
			int nIndex, 
			const TCHAR* lpDestPath, 
			ExtractProcessCallbacks* pCallbacks
			);

	int ConvertResult(int nResult);
};
