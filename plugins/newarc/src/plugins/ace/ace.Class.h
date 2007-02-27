#include <FarPluginBase.hpp>
#include <Rtl.Base.h>
#include "../../module.hpp"
#include "unace.h"


typedef int (__stdcall *ACEINITDLL) (pACEInitDllStruc DllData);
typedef int (__stdcall *ACEREADARCHIVEDATA) (LPSTR ArchiveName, pACEReadArchiveDataStruc ArchiveData);
typedef int (__stdcall *ACELIST) (LPSTR ArchiveName, pACEListStruc List);
typedef int (__stdcall *ACETEST) (LPSTR ArchiveName, pACETestStruc Test);
typedef int (__stdcall *ACEEXTRACT) (LPSTR ArchiveName, pACEExtractStruc Extract);

class AceModule {

public:

	HMODULE m_hModule;

	ACEINITDLL m_pfnInitDll;
	ACEREADARCHIVEDATA m_pfnReadArchiveData;
	ACELIST m_pfnList;
	ACETEST m_pfnTest;
	ACEEXTRACT m_pfnExtract;
public:

	AceModule ();
	~AceModule ();
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
	int m_nLastProcessed;

	int m_nMode;
	bool m_bIsArchive;

public:

	AceArchive (const char *lpFileName);
	virtual ~AceArchive ();

	bool IsArchive ();

	virtual bool __stdcall pOpenArchive (int nOpMode, ARCHIVECALLBACK pfnCallback);
	virtual void __stdcall pCloseArchive ();

	virtual int __stdcall pGetArchiveItem (ArchiveItemInfo *pItem);
	virtual bool __stdcall pExtract (PluginPanelItem *pItems, int nItemsNumber, const char *lpDestPath, const char *lpCurrentFolder);

public:

	int __stdcall OnInfo (pACEInfoCallbackProcStruc Info);
	int __stdcall OnState (pACEStateCallbackProcStruc State);
	int __stdcall OnRequest (pACERequestCallbackProcStruc Request);
	int __stdcall OnError (pACEErrorCallbackProcStruc Error);
};
