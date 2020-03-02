#include <FarPluginBase.hpp>
#include <Rtl.Base.h>
#include "../../module.hpp"
#include "dll.hpp"

typedef HANDLE (__stdcall *RAROPENARCHIVEEX) (struct RAROpenArchiveDataEx *ArchiveData);
typedef int    (__stdcall *RARCLOSEARCHIVE) (HANDLE hArcData);
typedef int    (__stdcall *RARREADHEADER) (HANDLE hArcData,struct RARHeaderData *HeaderData);
typedef int    (__stdcall *RARREADHEADEREX) (HANDLE hArcData,struct RARHeaderDataEx *HeaderData);
typedef int    (__stdcall *RARPROCESSFILE)(HANDLE hArcData,int Operation,char *DestPath,char *DestName);
typedef int    (__stdcall *RARPROCESSFILEW)(HANDLE hArcData,int Operation,wchar_t *DestPath, wchar_t *DestName);
typedef void   (__stdcall *RARSETCALLBACK)(HANDLE hArcData,UNRARCALLBACK Callback,LONG UserData);
typedef void   (__stdcall *RARSETCHANGEVOLPROC)(HANDLE hArcData,CHANGEVOLPROC ChangeVolProc);
typedef void   (__stdcall *RARSETPROCESSDATAPROC)(HANDLE hArcData,PROCESSDATAPROC ProcessDataProc);
typedef void   (__stdcall *RARSETPASSWORD)(HANDLE hArcData,char *Password);
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

	RarModule (const char *lpFileName);
	~RarModule ();
};

class RarArchive {

public:

	RarModule *m_pModule;
	HANDLE m_hArchive;
	ARCHIVECALLBACK m_pfnCallback;
	PBYTE m_pfnRarCallback;
	bool m_bAborted;
	char *m_lpFileName;
	int m_nOpMode;

public:

	RarArchive (RarModule *pModule, const char *lpFileName);
	virtual ~RarArchive ();

	LONG_PTR Callback (int nMsg, int nParam1, LONG_PTR nParam2);

	virtual bool __stdcall pOpenArchive (int nOpMode, ARCHIVECALLBACK pfnCallback);
	virtual void __stdcall pCloseArchive ();

	virtual int __stdcall pGetArchiveItem (ArchiveItemInfo *pItem);
	virtual bool __stdcall pExtract (PluginPanelItem *pItems, int nItemsNumber, const char *lpDestPath, const char *lpCurrentFolder);
	virtual bool __stdcall pTest (PluginPanelItem *pItems, int nItemsNumber);

//	virtual int __stdcall pGetArchiveType () {	return 0; }

public:

	virtual int __stdcall RarCallback (int nMsg, void *pParam, int nParam1, int nParam2);
};
