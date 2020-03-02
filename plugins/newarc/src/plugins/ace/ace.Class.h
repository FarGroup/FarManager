#include <FarPluginBase.hpp>
#include <Rtl.Base.h>
#include "../../module.hpp"
#include "ace.h"

extern "C" const GUID CLSID_FormatACE;

typedef int (__stdcall *ACEINITDLL) (pACEInitDllStruc DllData);
typedef int (__stdcall *ACEREADARCHIVEDATA) (LPSTR ArchiveName, pACEReadArchiveDataStruc ArchiveData);
typedef int (__stdcall *ACELIST) (LPSTR ArchiveName, pACEListStruc List);
typedef int (__stdcall *ACETEST) (LPSTR ArchiveName, pACETestStruc Test);
typedef int (__stdcall *ACEEXTRACT) (LPSTR ArchiveName, pACEExtractStruc Extract);
typedef int (__stdcall *ACEADD) (LPSTR ArchiveName, pACEAddStruc Add);
typedef int (__stdcall *ACEDELETE) (LPSTR ArchiveName, pACEDeleteStruc Delete);

class AceModule {

public:

	HMODULE m_hModule;

	ACEINITDLL m_pfnInitDll;
	ACEREADARCHIVEDATA m_pfnReadArchiveData;
	ACELIST m_pfnList;
	ACETEST m_pfnTest;
	ACEEXTRACT m_pfnExtract;
	ACEADD m_pfnAdd;
	ACEDELETE m_pfnDelete;

	bool m_bSupportUpdate;

public:

	AceModule ();
	~AceModule ();

	void GetArchiveFormatInfo (ArchiveFormatInfo *pInfo);
};

class AceArchive {

public:

	AceModule *m_pModule;
	char *m_lpFileName;

	ARCHIVECALLBACK m_pfnCallback;

	char m_CommentBuf[8192];

	HANDLE m_hListThread;
	HANDLE m_hListEvent;
	HANDLE m_hListEventComplete;

	ArchiveItemInfo *m_item;
	unsigned __int64 m_dwLastProcessed;

	int m_nMode;
	bool m_bIsArchive;

	bool m_bNewFile;

public:

	AceArchive (AceModule *pModule, const char *lpFileName, bool bNewFile);
	virtual ~AceArchive ();

	bool IsArchive ();

	virtual bool __stdcall pOpenArchive (int nOpMode, ARCHIVECALLBACK pfnCallback);
	virtual void __stdcall pCloseArchive ();

	virtual int __stdcall pGetArchiveItem (ArchiveItemInfo *pItem);
	virtual bool __stdcall pExtract (PluginPanelItem *pItems, int nItemsNumber, const char *lpDestPath, const char *lpCurrentFolder);
	virtual bool __stdcall pAddFiles (const char *lpSourcePath, const char *lpCurrentPath, PluginPanelItem *pItems, int nItemsNumber);
	virtual bool __stdcall pDelete (PluginPanelItem *pItems, int nItemsNumber);

public:

	int __stdcall OnInfo (pACEInfoCallbackProcStruc Info);
	int __stdcall OnState (pACEStateCallbackProcStruc State);
	int __stdcall OnRequest (pACERequestCallbackProcStruc Request);
	int __stdcall OnError (pACEErrorCallbackProcStruc Error);

	LONG_PTR __stdcall Callback (int nMsg, int nParam1, LONG_PTR nParam2);
};
