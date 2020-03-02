#pragma once
#include "multiex.h"

class MultiExPlugin {

private:

	GUID m_uid;
	HMODULE m_hModule;

	Array<ArchiveFormatInfo> m_pFormatInfo;
	
	string m_strModuleName;

	MPGETINTERFACEVERSION m_pfnMpGetInterfaceVersion;
	MPGETPLUGININFO m_pfnMpGetPluginInfo;
	MPGETFORMATCOUNT m_pfnMpGetFormatCount;
	MPGETFORMATINFO m_pfnMpGetFormatInfo;
	MPGETOPTIONS m_pfnMpGetOptions;
	MPSETOPTION m_pfnMpSetOption;
	MPOPENARCHIVE m_pfnMpOpenArchive;
	MPOPENARCHIVEBINDSTREAM m_pfnMpOpenArchiveBindStream;
	MPCLOSEARCHIVE m_pfnMpCloseArchive;
	MPINDEXCOUNT m_pfnMpIndexCount;
	MPINDEXEDINFO m_pfnMpIndexedInfo;
	MPFINDINFO m_pfnMpFindInfo;
	MPFINDFIRSTFILE m_pfnMpFindFirstFile;
	MPFINDNEXTFILE m_pfnMpFindNextFile;
	MPFINDCLOSE m_pfnMpFindClose;
	MPISFILEANARCHIVE m_pfnMpIsFileAnArchive;
	MPISSTREAMANARCHIVE m_pfnMpIsStreamAnArchive;
	MPEXPORTFILEBYNAMETOFILE m_pfnMpExportFileByNameToFile;
	MPEXPORTFILEBYINDEXTOFILE m_pfnMpExportFileByIndexToFile;
	MPEXPORTFILEBYNAMETOSTREAM m_pfnMpExportFileByNameToStream;
	MPEXPORTFILEBYINDEXTOSTREAM m_pfnMpExportFileByIndexToStream;
	MPIMPORTFILEFROMFILE m_pfnMpImportFileFromFile;
	MPIMPORTFILEFROMSTREAM m_pfnMpImportFileFromStream;
	MPREMOVEFILEBYNAME m_pfnMpRemoveFileByName;
	MPREMOVEFILEBYINDEX m_pfnMpRemoveFileByIndex;
	MPGETLASTERROR m_pfnMpGetLastError;
	MPGETERRORTEXT m_pfnMpGetErrorText;

public:

	MultiExPlugin(const GUID& uid);
	~MultiExPlugin();

	bool Load(const TCHAR* lpModuleName);

	const GUID& GetUID();
	const TCHAR* GetModuleName();

	const ArchiveFormatInfo* GetFormats();
	unsigned int GetNumberOfFormats();

	int QueryArchives(const TCHAR* lpFileName, Array<ArchiveQueryResult*>& result);
	int QueryArchive(const GUID& uidFormat, const TCHAR* lpFileName, Array<ArchiveQueryResult*>& result);

	MultiExArchive* OpenCreateArchive(
			const GUID& uidFormat, 
			const TCHAR* lpFileName, 
			HANDLE hCallback, 
			ARCHIVECALLBACK pfnCallback, 
			bool bCreate
			);

	void CloseArchive(MultiExArchive* pArchive);

	int GetFormatIndex(const GUID& uidFormat);

//multiex

	int OpenArchive(int nFormatIndex, const TCHAR* lpFileName);
	void CloseArchive(int hArchive);

	int GetArchiveItem(int hArchive, int& hSearch, ArchiveItem* pItem);
	void FreeArchiveItem(int hArchive, ArchiveItem* pItem);

	bool Delete(int hArchive, const ArchiveItem* pItem, int nItemsNumber);
	bool Extract(int hArchive, const ArchiveItem* pItem, int nItemsNumber, const TCHAR* lpDestDiskPath, const TCHAR* lpPathInArchive);
	bool AddFiles(int hArchive, const ArchiveItem* pItems, int nItemsNumber, const TCHAR* lpSourceDiskFile, const TCHAR* lpPathInArchive);

private:

	int ConvertResult(int nResult);
};
