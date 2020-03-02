#include <Rtl.Base.h>
#include <FarPluginBase.hpp>
#include <array.hpp>
#include "../../module.hpp"
#include "wcxhead.h"

typedef HANDLE (__stdcall *PLUGINOPENARCHIVE)( tOpenArchiveData* ArchiveData );
typedef int    (__stdcall *PLUGINCLOSEARCHIVE)( HANDLE hArcData );
typedef int    (__stdcall *PLUGINREADHEADER)( HANDLE hArcData, tHeaderData* HeaderData );
typedef int    (__stdcall *PLUGINPROCESSFILE)( HANDLE hArcData, int Operation, char* DestPath, char* DestName );
typedef int    (__stdcall *PLUGINPACKFILES)( char* PackedFile, char* SubPath, char* SrcPath, char* AddList, int Flags );
typedef int    (__stdcall *PLUGINDELETEFILES)( char* PackedFile, char* DeleteList );
typedef void   (__stdcall *PLUGINSETCHANGEVOLPROC)( HANDLE hArcData, tChangeVolProc pChangeVolProc1 );
typedef void   (__stdcall *PLUGINSETPROCESSDATAPROC)( HANDLE hArcData, tProcessDataProc pProcessDataProc );
typedef void   (__stdcall *PLUGINCONFIGUREPACKER)( HWND Parent, HINSTANCE DllInstance );
typedef int    (__stdcall *PLUGINGETPACKERCAPS)();
typedef BOOL   (__stdcall *PLUGINCANYOUHANDLETHISFILE)( const char* FileName );
typedef void   (__stdcall *PLUGINPACKSETDEFAULTPARAMS)( PackDefaultParamStruct* dps );

struct WcxPluginInfo {
	char Name[100];
	char DefaultExtention[260];
};

class WcxModule {

public:

	HMODULE m_hModule;
	DWORD m_dwCRC;

	WcxPluginInfo m_Info;

public:

	char *m_lpModuleName;

	PLUGINOPENARCHIVE m_pfnOpenArchive;
	PLUGINCLOSEARCHIVE m_pfnCloseArchive;
	PLUGINREADHEADER m_pfnReadHeader;
	PLUGINPROCESSFILE m_pfnProcessFile;
	PLUGINPACKFILES m_pfnPackFiles;
	PLUGINDELETEFILES m_pfnDeleteFiles;
	PLUGINSETCHANGEVOLPROC m_pfnSetChangeVolProc;
	PLUGINSETPROCESSDATAPROC m_pfnSetProcessDataProc;
	PLUGINCONFIGUREPACKER m_pfnConfigurePacker;
	PLUGINGETPACKERCAPS m_pfnGetPackerCaps;
	PLUGINCANYOUHANDLETHISFILE m_pfnCanYouHandleThisFile;
	PLUGINPACKSETDEFAULTPARAMS m_pfnPackSetDefaultParams;

	WcxModule (const char *lpFileName);
	bool LoadedOK();
	~WcxModule ();
};




class WcxModules {

	pointer_array<WcxModule*> m_Modules;

	ArchivePluginInfo m_PluginInfo;

public:

	WcxModules();
	~WcxModules();

	WcxModule *IsArchive(QueryArchiveStruct *pQAS, int *nModuleNum);

	static int WINAPI LoadWcxModules(const WIN32_FIND_DATA *pFindData,const char *lpFullName,WcxModules *pModules);

	void GetArchivePluginInfo (ArchivePluginInfo *ai);
	bool GetDefaultCommand (const GUID &uid, int nCommand, char *lpCommand);
};

class WcxArchive {

	WcxModule* m_pModule;

	int m_nModuleNum;

	HANDLE m_hArchive;
	char *m_lpFileName;

	bool bProcessDataProc;

	PBYTE m_pfnProcessDataProc;
	PBYTE m_pfnSetChangeVolProc;

	ARCHIVECALLBACK m_pfnCallback;

public:

	WcxArchive (WcxModule *pModule, int nModuleNum, const char *lpFileName);
	virtual ~WcxArchive ();

	LONG_PTR Callback (int nMsg, int nParam1, LONG_PTR nParam2);

	virtual int ConvertResult (int nResult);

	virtual bool __stdcall pOpenArchive (int nOpMode, ARCHIVECALLBACK pfnCallback);
	virtual void __stdcall pCloseArchive ();

	virtual bool __stdcall pExtract (PluginPanelItem *pItems, int nItemsNumber, const char *lpDestPath, const char *lpCurrentFolder);
	//virtual bool __stdcall pTest (PluginPanelItem *pItems, int nItemsNumber);

	virtual int __stdcall pGetArchiveItem (ArchiveItemInfo *pItem);
	virtual void __stdcall pGetArchiveType (GUID *puid);

public:
	virtual int __stdcall ProcessDataProc (char *FileName, int Size);
	virtual int __stdcall SetChangeVolProc (char *ArcName, int Mode);
};
