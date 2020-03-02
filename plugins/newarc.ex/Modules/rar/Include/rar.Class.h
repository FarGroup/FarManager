#include "rar.h"

typedef HANDLE (__stdcall *RAROPENARCHIVEEX) (struct RAROpenArchiveDataEx *ArchiveData);
typedef int    (__stdcall *RARCLOSEARCHIVE) (HANDLE hArcData);
typedef int    (__stdcall *RARREADHEADER) (HANDLE hArcData, struct RARHeaderData *HeaderData);
typedef int    (__stdcall *RARREADHEADEREX) (HANDLE hArcData, struct RARHeaderDataEx *HeaderData);
typedef int    (__stdcall *RARPROCESSFILE)(HANDLE hArcData, int Operation, const char *DestPath, const char *DestName);
typedef int    (__stdcall *RARPROCESSFILEW)(HANDLE hArcData, int Operation, const wchar_t *DestPath, const wchar_t *DestName);
typedef void   (__stdcall *RARSETCALLBACK)(HANDLE hArcData, UNRARCALLBACK Callback,LONG UserData);
typedef void   (__stdcall *RARSETCHANGEVOLPROC)(HANDLE hArcData, CHANGEVOLPROC ChangeVolProc);
typedef void   (__stdcall *RARSETPROCESSDATAPROC)(HANDLE hArcData, PROCESSDATAPROC ProcessDataProc);
typedef void   (__stdcall *RARSETPASSWORD)(HANDLE hArcData, const char *Password);
typedef int    (__stdcall *RARGETDLLVERSION)();


class RarModule {

public:

	HMODULE m_hModule;

	RAROPENARCHIVEEX m_pfnOpenArchiveEx;
	RARCLOSEARCHIVE m_pfnCloseArchive;
	RARREADHEADER m_pfnReadHeader;
	RARREADHEADEREX m_pfnReadHeaderEx;
	RARPROCESSFILE m_pfnProcessFile;
	RARPROCESSFILEW m_pfnProcessFileW;
	RARSETCALLBACK m_pfnSetCallback;
	RARSETCHANGEVOLPROC m_pfnSetChangeVolProc;
	RARSETPROCESSDATAPROC m_pfnSetProcessDataProc;
	RARSETPASSWORD m_pfnSetPassword;
	RARGETDLLVERSION m_pfnGetDllVersion;

public:

	RarModule (const TCHAR *lpFileName);
	~RarModule ();
};

class RarArchive {

public:

	RarModule *m_pModule;
	HANDLE m_hArchive;
	
	ARCHIVECALLBACK m_pfnCallback;
	bool m_bAborted;

	string m_strFileName;
	
	int m_nOpMode;

	HANDLE m_hCallback;

public:

	RarArchive (RarModule *pModule, const TCHAR *lpFileName);
	virtual ~RarArchive ();

	LONG_PTR Callback (int nMsg, int nParam1, LONG_PTR nParam2);

	bool OpenArchive (int nOpMode, HANDLE hCallback, ARCHIVECALLBACK pfnCallback);
	void CloseArchive ();

	int GetArchiveItem (ArchiveItem *pItem);
	bool FreeArchiveItem(ArchiveItem* pItem);
	
	bool Extract(const ArchiveItem *pItems, int nItemsNumber, const TCHAR *lpDestPath, const TCHAR *lpCurrentFolder);
	bool Test(const ArchiveItem *pItems, int nItemsNumber);

//	virtual int __stdcall pGetArchiveType () {	return 0; }

public:

	static int __stdcall RarCallback (int nMsg, LONG UserData, int nParam1, int nParam2);
};
