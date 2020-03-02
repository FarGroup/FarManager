#pragma once
#include "ace.h"

typedef int (__stdcall *ACEINITDLL) (pACEInitDllStruc DllData);
typedef int (__stdcall *ACEREADARCHIVEDATA) (const char* ArchiveName, pACEReadArchiveDataStruc ArchiveData);
typedef int (__stdcall *ACELIST) (const char* ArchiveName, pACEListStruc List);
typedef int (__stdcall *ACETEST) (const char* ArchiveName, pACETestStruc Test);
typedef int (__stdcall *ACEEXTRACT) (const char* ArchiveName, pACEExtractStruc Extract);
typedef int (__stdcall *ACEADD) (const char* ArchiveName, pACEAddStruc Add);
typedef int (__stdcall *ACEDELETE) (const char* ArchiveName, pACEDeleteStruc Delete);

struct AceArchiveHandle {
	const char* lpFileName;
	AcePlugin* pPlugin;
	bool bList;
};

class AcePlugin {

private:

	HMODULE m_hModule;

	string m_strModuleName;
	bool m_bIsArchive;

	ArchiveFormatInfo* m_pFormatInfo;

	ACEINITDLL m_pfnInitDll;
	ACEREADARCHIVEDATA m_pfnReadArchiveData;
	ACELIST m_pfnList;
	ACETEST m_pfnTest;
	ACEEXTRACT m_pfnExtract;
	ACEADD m_pfnAdd;
	ACEDELETE m_pfnDelete;

	char m_CommentBuf[8192];

	HANDLE m_hListThread;
	HANDLE m_hListEvent;
	HANDLE m_hListEventComplete;

	unsigned __int64 m_dwLastProcessed;

	int m_nMode;

	ArchiveItem* m_pItem;


public:

	AcePlugin();
	~AcePlugin();

	const GUID& GetUID();
	const TCHAR* GetModuleName();

	bool Load(const TCHAR* lpFileName);

	unsigned int GetNumberOfFormats();
	const ArchiveFormatInfo* GetFormats();

	bool QueryArchive(
			const TCHAR* lpFileName, 
			const unsigned char* pBuffer,
			DWORD dwBufferSize, 
			ArchiveQueryResult* pResult
			);
	
	AceArchive* OpenCreateArchive(const TCHAR* lpFileName, HANDLE hCallback, ARCHIVECALLBACK pfnCallback, bool bCreate);
	void CloseArchive(AceArchive* pArchive);


/// ACE
	bool Initialize(); //REMOVE!!!

	HANDLE OpenArchive(const TCHAR* lpFileName, bool bList);
	void CloseArchive(HANDLE hArchive);

	int GetArchiveItem(ArchiveItem* pItem);
	void FreeArchiveItem(ArchiveItem* pItem);

	bool Extract(HANDLE hArchive, const ArchiveItem* pItems, int nItemsNumber, const TCHAR* lpDeskDiskPath, const TCHAR* lpPathInArchive);
	bool Delete(HANDLE hArchive, const ArchiveItem* pItems, int nItemsNumber);
	bool AddFiles(HANDLE hArchive, const ArchiveItem* pItems, int nItemsNumber, const TCHAR* lpSourceDiskPath, const TCHAR* lpPathInArchive);

private:

	int OnInfo(pACEInfoCallbackProcStruc Info);
	int OnState(pACEStateCallbackProcStruc State);
	int OnRequest(pACERequestCallbackProcStruc Request);
	int OnError(pACEErrorCallbackProcStruc Error);

	static int __stdcall InfoProc(pACEInfoCallbackProcStruc Info);
	static int __stdcall StateProc(pACEStateCallbackProcStruc State);
	static int __stdcall RequestProc(pACERequestCallbackProcStruc Request);
	static int __stdcall ErrorProc(pACEErrorCallbackProcStruc Error);

	static int __stdcall AceListThread(void* pParam);
};