#include <FarPluginBase.h>
#include <Rtl.Base.h>
#include <Collections.h>
#ifdef _DEBUG
#include <debug.h>
#endif
#include "../../module.hpp"
#include "fmt.hpp"

typedef DWORD (WINAPI *PLUGINLOADFORMATMODULE)(const char *ModuleName);
typedef BOOL (WINAPI *PLUGINISARCHIVE)(const char *Name,const unsigned char *Data,int DataSize);
typedef BOOL (WINAPI *PLUGINOPENARCHIVE)(const char *Name,int *Type);
typedef int (WINAPI *PLUGINGETARCITEM)(struct PluginPanelItem *Item,struct ArcItemInfo *Info);
typedef BOOL (WINAPI *PLUGINCLOSEARCHIVE)(struct ArcInfo *Info);
typedef BOOL (WINAPI *PLUGINGETFORMATNAME)(int Type,char *FormatName,char *DefaultExt);
typedef BOOL (WINAPI *PLUGINGETDEFAULTCOMMANDS)(int Type,int Command,char *Dest);
typedef void (WINAPI *PLUGINSETFARINFO)(const struct PluginStartupInfo *plg);
typedef DWORD (WINAPI *PLUGINGETSFXPOS)(void);

class MaModule {

public:

	HMODULE m_hModule;

	PLUGINLOADFORMATMODULE m_pfnLoadFormatModule;
	PLUGINISARCHIVE m_pfnIsArchive;
	PLUGINOPENARCHIVE m_pfnOpenArchive;
	PLUGINGETARCITEM m_pfnGetArcItem;
	PLUGINCLOSEARCHIVE m_pfnCloseArchive;
	PLUGINGETFORMATNAME m_pfnGetFormatName;
	PLUGINGETDEFAULTCOMMANDS m_pfnGetDefaultCommands;
	PLUGINSETFARINFO m_pfnSetFarInfo;
	PLUGINGETSFXPOS m_pfnGetSFXPos;

public:

	MaModule (const char *lpFileName);
	bool LoadedOK();
	~MaModule ();
};

class MaModules {

	Collection <MaModule*> m_Modules;

	ArchivePluginInfo m_PluginInfo;

	struct ExtraPluginInfo {
		char Name[100];
		char DefaultExtention[260];
		int FormatNumber;
	};

	struct ExtraPluginInfoArray {
		int BaseNumber;
		int Count;
		ExtraPluginInfo *pExtraPluginInfo;
	};

	Collection <ExtraPluginInfoArray*> m_ExtraPluginInfo;

public:

	MaModules();
	~MaModules();

	MaModule *IsArchive(QueryArchiveStruct *pQAS, int *nModuleNum);

	static int WINAPI LoadFmtModules(const WIN32_FIND_DATA *pFindData,const char *lpFullName,MaModules *pModules);

	void GetArchivePluginInfo (ArchivePluginInfo *ai);
	bool GetDefaultCommand (int nFormat, int nCommand, char *lpCommand);
};

class MaArchive {

public:

	MaModule* m_pModule;

	int m_nModuleNum;

	HANDLE m_hArchive;

	char *m_lpFileName;

	int m_nArcType;

public:

	MaArchive (MaModule *pModule, int nModuleNum, const char *lpFileName);
	virtual ~MaArchive ();

	virtual int ConvertResult (int nResult);

	virtual bool __stdcall pOpenArchive ();
	virtual void __stdcall pCloseArchive ();

	virtual int __stdcall pGetArchiveItem (ArchiveItemInfo *pItem);

	virtual int __stdcall pGetArchiveType ();
};
