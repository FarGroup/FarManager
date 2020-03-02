#include "7z.h"

struct FormatPosition {
	const GUID *puid;
	int position;
};


typedef unsigned int (__stdcall *CREATEOBJECT) (const GUID *, const GUID *, void **);
typedef HRESULT (__stdcall *GETHANDLERPROPERTY) (PROPID propID, PROPVARIANT *value);
typedef HRESULT (__stdcall *GETHANDLERPROPERTY2) (unsigned int formatIndex, PROPID propID, PROPVARIANT *value);
typedef HRESULT (__stdcall *GETNUMBEROFFORMATS) (unsigned int *numFormats);

extern bool GetFormatCommand(const GUID &guid, int nCommand, char *lpCommand);


struct FormatInfoInternal {
	GUID uid;
	char *lpSignature;
	int nSignatureLength;
	char *lpDefaultExt;
	bool bUpdate;
	char *lpName;
};


class SevenZipModule {

public:
	HMODULE m_hModule;

	CREATEOBJECT m_pfnCreateObject;
	GETHANDLERPROPERTY m_pfnGetHandlerProperty;
	GETHANDLERPROPERTY2 m_pfnGetHandlerProperty2;
	GETNUMBEROFFORMATS m_pfnGetNumberOfFormats;

	unsigned int m_nNumberOfFormats;
	FormatInfoInternal *m_pInfo;

public:

	bool Initialize (const char *lpFileName);
	~SevenZipModule ();

	void GetArchiveFormatInfo (unsigned int nFormatIndex, ArchiveFormatInfo *pInfo);
};

class SevenZipArchive {

public:

	const SevenZipModule *m_pModule;
	unsigned int m_nFormatIndex;

	HANDLE m_hArchive;
	ARCHIVECALLBACK m_pfnCallback;

	IInArchive *m_pArchive;
	CInFile *m_pInFile;

	char *m_lpFileName;

	DWORD m_nItemsNumber;

	bool m_bListPassword;

//	bool m_bForcedUpdate;
	bool m_bOpened;
	bool m_bNewArchive;

public:

	SevenZipArchive (const SevenZipModule *pModule, unsigned int nFormatIndex, const char *lpFileName, bool bNewArchive);
	virtual ~SevenZipArchive ();

	LONG_PTR __stdcall Callback (int nMsg, int nParam1, LONG_PTR nParam2);

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
