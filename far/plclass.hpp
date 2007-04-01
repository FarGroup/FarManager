#ifndef __PLCLASS_HPP__
#define __PLCLASS_HPP__

#include "unicodestring.hpp"
#include "struct.hpp"
#include "plugin.hpp"

class Plugin
{
public:

	virtual ~Plugin () { }

	virtual int Load() = 0;
	virtual int LoadFromCache() = 0;

	virtual int SaveToCache () = 0;

	virtual int Unload (bool bExitFAR = false) = 0;

	virtual int GetCacheNumber () = 0;
	virtual bool IsPanelPlugin () = 0;

	virtual bool HasOpenPlugin() = 0;
	virtual bool HasMakeDirectory() = 0;
	virtual bool HasDeleteFiles() = 0;
	virtual bool HasPutFiles() = 0;
	virtual bool HasGetFiles() = 0;
	virtual bool HasSetStartupInfo() = 0;
	virtual bool HasOpenFilePlugin() = 0;
	virtual bool HasClosePlugin() = 0;
	virtual bool HasGetPluginInfo() = 0;
	virtual bool HasGetOpenPluginInfo() = 0;
	virtual bool HasGetFindData() = 0;
	virtual bool HasFreeFindData() = 0;
	virtual bool HasGetVirtualFindData() = 0;
	virtual bool HasFreeVirtualFindData() = 0;
	virtual bool HasSetDirectory() = 0;
	virtual bool HasProcessHostFile() = 0;
	virtual bool HasSetFindList() = 0;
	virtual bool HasConfigure() = 0;
	virtual bool HasExitFAR() = 0;
	virtual bool HasProcessKey() = 0;
	virtual bool HasProcessEvent() = 0;
	virtual bool HasProcessEditorEvent() = 0;
	virtual bool HasCompare() = 0;
	virtual bool HasProcessEditorInput() = 0;
	virtual bool HasMinFarVersion() = 0;
	virtual bool HasProcessViewerEvent() = 0;

	virtual const string &GetModuleName() = 0;
	virtual const FAR_FIND_DATA_EX &GetFindData() = 0;
	virtual DWORD GetSysID() = 0;
	virtual int GetCachePos() = 0;
	virtual bool CheckWorkFlags(DWORD flags) = 0;
	virtual DWORD GetWorkFlags() = 0;
	virtual DWORD GetFuncFlags() = 0;

	virtual int InitLang(const wchar_t *Path,int CountNeed=-1) = 0;
	virtual void CloseLang() = 0;
	virtual const wchar_t *GetMsg (int nID) = 0;

	virtual int SetStartupInfo (bool &bUnloaded) = 0;
	virtual int CheckMinFarVersion (bool &bUnloaded) = 0;

	virtual HANDLE OpenPlugin (int OpenFrom, INT_PTR Item) = 0;
	virtual HANDLE OpenFilePlugin (const wchar_t *Name, const unsigned char *Data, int DataSize, int OpMode) = 0;

	virtual int SetFindList (HANDLE hPlugin, const PluginPanelItem *PanelItem, int ItemsNumber) = 0;
	virtual int GetFindData (HANDLE hPlugin, PluginPanelItem **pPanelItem, int *pItemsNumber, int OpMode) = 0;
	virtual int GetVirtualFindData (HANDLE hPlugin, PluginPanelItem **pPanelItem, int *pItemsNumber, const wchar_t *Path) = 0;
	virtual int SetDirectory (HANDLE hPlugin, const wchar_t *Dir, int OpMode) = 0;
	virtual int GetFiles (HANDLE hPlugin, PluginPanelItem *PanelItem, int ItemsNumber, int Move, const wchar_t *DestPath, int OpMode) = 0;
	virtual int PutFiles (HANDLE hPlugin, PluginPanelItem *PanelItem, int ItemsNumber, int Move, int OpMode) = 0;
	virtual int DeleteFiles (HANDLE hPlugin, PluginPanelItem *PanelItem, int ItemsNumber, int OpMode) = 0;
	virtual int MakeDirectory (HANDLE hPlugin, const wchar_t *Name, int OpMode) = 0;
	virtual int ProcessHostFile (HANDLE hPlugin, PluginPanelItem *PanelItem, int ItemsNumber, int OpMode) = 0;
	virtual int ProcessKey (HANDLE hPlugin, int Key, unsigned int dwControlState) = 0;
	virtual int ProcessEvent (HANDLE hPlugin, int Event, PVOID Param) = 0;
	virtual int Compare (HANDLE hPlugin, const PluginPanelItem *Item1, const PluginPanelItem *Item2, unsigned long Mode) = 0;

	virtual void GetOpenPluginInfo (HANDLE hPlugin, OpenPluginInfo *Info) = 0;
	virtual void FreeFindData (HANDLE hPlugin, PluginPanelItem *PanelItem, int ItemsNumber) = 0;
	virtual void FreeVirtualFindData (HANDLE hPlugin, PluginPanelItem *PanelItem, int ItemsNumber) = 0;
	virtual void ClosePlugin (HANDLE hPlugin) = 0;

	virtual int ProcessEditorInput (const INPUT_RECORD *D) = 0;
	virtual int ProcessEditorEvent (int Event, PVOID Param) = 0;
	virtual int ProcessViewerEvent (int Event, PVOID Param) = 0;

	virtual void GetPluginInfo (PluginInfo *pi) = 0;
	virtual int Configure (int MenuItem) = 0;

	virtual void ExitFAR() = 0;
};

#endif  // __PLCLASS_HPP__
