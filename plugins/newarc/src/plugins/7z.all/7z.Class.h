#include "7z.h"

struct FormatPosition {
	const GUID *puid;
	int position;
};


typedef unsigned int (__stdcall *CREATEOBJECT) (const GUID *, const GUID *, void **);
typedef HRESULT (__stdcall *GETHANDLERPROPERTY) (PROPID propID, PROPVARIANT *value);

bool GetFormatCommand(const GUID guid, int nCommand, char *lpCommand);


class SevenZipModule {

public:
	HMODULE m_hModule;

	CREATEOBJECT m_pfnCreateObject;
	GETHANDLERPROPERTY m_pfnGetHandlerProperty;

	GUID m_uid;

public:

	bool Initialize (const char *lpFileName);
	~SevenZipModule ();

	void GetArchiveFormatInfo (ArchiveFormatInfo *pInfo);
	bool IsSplitModule ();
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

	bool m_bListPassword;

	bool m_bForcedUpdate;
	bool m_bOpened;
	bool m_bNewArchive;

public:

	SevenZipArchive (SevenZipModule *pModule, const char *lpFileName, bool bNewArchive);
	virtual ~SevenZipArchive ();

	virtual bool __stdcall pOpenArchive (int nOpMode, ARCHIVECALLBACK pfnCallback);
	virtual void __stdcall pCloseArchive ();

	virtual int __stdcall pGetArchiveItem (ArchiveItemInfo *pItem);
	virtual bool __stdcall pExtract (PluginPanelItem *pItems, int nItemsNumber, const char *lpDestPath, const char *lpCurrentFolder);
	virtual bool __stdcall pTest (PluginPanelItem *pItems, int nItemsNumber);
	virtual bool __stdcall pDelete (PluginPanelItem *pItems, int nItemsNumber);
	virtual bool __stdcall pAddFiles (const char *lpSourcePath, const char *lpCurrentPath, PluginPanelItem *pItems, int nItemsNumber);
	virtual void __stdcall pNotify (int nEvent, void *pEventData);

	virtual int __stdcall pGetArchiveType () { return 0; }
};
