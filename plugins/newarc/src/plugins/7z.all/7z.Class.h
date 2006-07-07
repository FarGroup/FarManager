#include "7z.h"

struct FormatPosition {
	const GUID *puid;
	int position;
};


typedef unsigned int (__stdcall *CREATEOBJECT) (const GUID *, const GUID *, void **);
typedef HRESULT (__stdcall *GETHANDLERPROPERTY) (PROPID propID, PROPVARIANT *value);


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
	bool HasSignature ();
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

	bool m_bPasswordDefined;
	char *m_lpPassword;
	int m_nPasswordLength;

public:

	SevenZipArchive (SevenZipModule *pModule, const char *lpFileName);
	virtual ~SevenZipArchive ();

	virtual bool __stdcall pOpenArchive (int nOpMode, ARCHIVECALLBACK pfnCallback, bool bAllowModifier);
	virtual void __stdcall pCloseArchive ();

	virtual int __stdcall pGetArchiveItem (ArchiveItemInfo *pItem);
	virtual bool __stdcall pExtract (PluginPanelItem *pItems, int nItemsNumber, const char *lpDestPath, const char *lpCurrentFolder);
	virtual bool __stdcall pTest (PluginPanelItem *pItems, int nItemsNumber);

	virtual int __stdcall pGetArchiveType () { return 0; }

	void SetPassword (const char *lpPassword, int nLength);
	int GetPasswordLength ();
	void GetPassword (char *lpBuffer);
};
