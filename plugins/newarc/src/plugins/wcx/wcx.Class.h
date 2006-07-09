#include <FarPluginBase.h>
#include <Rtl.Base.h>
#include <Collections.h>
#ifdef _DEBUG
#include <debug.h>
#endif
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

class WcxModule {

public:

	HMODULE m_hModule;

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

public:

	WcxModule (const char *lpFileName);
	bool LoadedOK();
	~WcxModule ();
};

class WcxModules {

	Collection <WcxModule*> m_Modules;

	ArchivePluginInfo m_PluginInfo;

	struct ExtraPluginInfo {
		char Name[100];
		char DefaultExtention[260];
	};

	Collection <ExtraPluginInfo*> m_ExtraPluginInfo;

public:

	WcxModules();
	~WcxModules();

	WcxModule *IsArchive(QueryArchiveStruct *pQAS, int *nModuleNum);

	static int WINAPI LoadWcxModules(const WIN32_FIND_DATA *pFindData,const char *lpFullName,WcxModules *pModules);

	void GetArchivePluginInfo (ArchivePluginInfo *ai);
	bool GetDefaultCommand (int nFormat, int nCommand, char *lpCommand);
};

class WcxArchive {

public:

	WcxModule* m_pModule;

	int m_nModuleNum;

	HANDLE m_hArchive;

	char *m_lpFileName;

public:

	WcxArchive (WcxModule *pModule, int nModuleNum, const char *lpFileName);
	virtual ~WcxArchive ();

	virtual int ConvertResult (int nResult);

	virtual bool __stdcall pOpenArchive ();
	virtual void __stdcall pCloseArchive ();

	virtual int __stdcall pGetArchiveItem (ArchiveItemInfo *pItem);

	virtual int __stdcall pGetArchiveType ();
};
