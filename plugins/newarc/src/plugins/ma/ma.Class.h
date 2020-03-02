#include <FarPluginBase.hpp>
#include <Rtl.Base.h>
#include <array.hpp>
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


struct MaPluginInfo {
	char Name[100];
	char DefaultExtention[260];
};


class MaModule {
public:

	DWORD m_dwCRC;
	pointer_array<MaPluginInfo*> m_Info;

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

	char *m_lpModuleName;

	MaModule (const char *lpFileName);
	bool LoadedOK();
	~MaModule ();
};

class MaModules {

public:

	pointer_array<MaModule*> m_Modules;
	ArchivePluginInfo m_PluginInfo;

public:

	MaModules();
	~MaModules();

	MaModule *IsArchive(QueryArchiveStruct *pQAS, int *nModuleNum);

	static int WINAPI LoadFmtModules(const WIN32_FIND_DATA *pFindData,const char *lpFullName,MaModules *pModules);

	void GetArchivePluginInfo (ArchivePluginInfo *ai);
	bool GetDefaultCommand (const GUID &uid, int nCommand, char *lpCommand);
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

	virtual void __stdcall pGetArchiveType (GUID *puid);
};
