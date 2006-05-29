#include "7z.h"

typedef unsigned int (__stdcall *CREATEOBJECT) (const GUID *, const GUID *, void **);


class SevenZipModule {

public:
	HMODULE m_hModule;

	CREATEOBJECT m_pfnCreateObject;

public:

	SevenZipModule (const char *lpFileName);
	~SevenZipModule ();
};

class SevenZipArchive {

public:

	SevenZipModule *m_pModule;

	HANDLE m_hArchive;
	ARCHIVECALLBACK m_pfnCallback;

	IInArchive *m_pArchive;
	CInFile *m_pInFile;

	char *m_lpFileName;

	DWORD m_nItemsNumber;

public:

	SevenZipArchive (SevenZipModule *pModule, const char *lpFileName);
	virtual ~SevenZipArchive ();

	virtual bool __stdcall pOpenArchive (int nOpMode, ARCHIVECALLBACK pfnCallback);
	virtual void __stdcall pCloseArchive ();

	virtual int __stdcall pGetArchiveItem (ArchiveItemInfo *pItem);
	virtual bool __stdcall pExtract (PluginPanelItem *pItems, int nItemsNumber, const char *lpDestPath, const char *lpCurrentFolder);
	virtual bool __stdcall pTest (PluginPanelItem *pItems, int nItemsNumber);

	virtual int __stdcall pGetArchiveType () { return 0; }
};
