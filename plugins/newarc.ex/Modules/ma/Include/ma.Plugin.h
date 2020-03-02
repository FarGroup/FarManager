#pragma once
#include "ma.h"

typedef DWORD (WINAPI *PLUGINLOADFORMATMODULE)(const char *ModuleName);
typedef BOOL (WINAPI *PLUGINISARCHIVE)(const char *Name,const unsigned char *Data,int DataSize);
typedef BOOL (WINAPI *PLUGINOPENARCHIVE)(const char *Name,int *Type);
typedef int (WINAPI *PLUGINGETARCITEM)(struct oldfar::PluginPanelItem *Item,struct ArcItemInfo *Info);
typedef BOOL (WINAPI *PLUGINCLOSEARCHIVE)(struct ArcInfo *Info);
typedef BOOL (WINAPI *PLUGINGETFORMATNAME)(int Type,char *FormatName,char *DefaultExt);
typedef BOOL (WINAPI *PLUGINGETDEFAULTCOMMANDS)(int Type,int Command,char *Dest);
typedef void (WINAPI *PLUGINSETFARINFO)(const struct oldfar::PluginStartupInfo *plg);
typedef DWORD (WINAPI *PLUGINGETSFXPOS)(void);

class MaPlugin {

private:

	GUID m_uid;
	HMODULE m_hModule;

	Array<ArchiveFormatInfo> m_pFormatInfo;
	
	string m_strModuleName;
	string m_strCommandBuffer;

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

	MaPlugin(const GUID& uid);
	~MaPlugin();

	bool Load(const TCHAR* lpModuleName, oldfar::PluginStartupInfo* pInfo, oldfar::FARSTANDARDFUNCTIONS* pFSF);

	const GUID& GetUID();
	const TCHAR* GetModuleName();

	const ArchiveFormatInfo* GetFormats();
	unsigned int GetNumberOfFormats();

	int QueryArchives(const unsigned char* pBuffer, DWORD dwBufferSize, const TCHAR* lpFileName, Array<ArchiveQueryResult*>& result);
	
	MaArchive* OpenArchive(const GUID& uid, const TCHAR* lpFileName, HANDLE hCallback, ARCHIVECALLBACK pfnCallback);
	void CloseArchive(MaArchive* pArchive);

	bool GetDefaultCommand(const GUID& uid, int nCommand, const TCHAR** ppCommand);

//fmt

	bool OpenArchive(const TCHAR* lpFileName);
	void CloseArchive(ArcInfo* pInfo);

	int GetArchiveItem(ArchiveItem* pItem, ArcItemInfo* pInfo);
	void FreeArchiveItem(ArchiveItem* pItem);

private:

	int ConvertResult(int nResult);
};
