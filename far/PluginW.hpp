#ifndef __PLUGINW_HPP__
#define __PLUGINW_HPP__

class PluginManager;

#include "language.hpp"
#include "bitflags.hpp"
#include "unicodestring.hpp"
#include "struct.hpp"
#include "plugin.hpp"
#include "plclass.hpp"

typedef void (WINAPI *PLUGINCLOSEPLUGINW)(HANDLE hPlugin);
typedef int (WINAPI *PLUGINCOMPAREW)(HANDLE hPlugin,const PluginPanelItem *Item1,const PluginPanelItem *Item2,unsigned int Mode);
typedef int (WINAPI *PLUGINCONFIGUREW)(int ItemNumber);
typedef int (WINAPI *PLUGINDELETEFILESW)(HANDLE hPlugin,PluginPanelItem *PanelItem,int ItemsNumber,int OpMode);
typedef void (WINAPI *PLUGINEXITFARW)();
typedef void (WINAPI *PLUGINFREEFINDDATAW)(HANDLE hPlugin,PluginPanelItem *PanelItem,int ItemsNumber);
typedef void (WINAPI *PLUGINFREEVIRTUALFINDDATAW)(HANDLE hPlugin,PluginPanelItem *PanelItem,int ItemsNumber);
typedef int (WINAPI *PLUGINGETFILESW)(HANDLE hPlugin,PluginPanelItem *PanelItem,int ItemsNumber,int Move,const wchar_t *DestPath,int OpMode);
typedef int (WINAPI *PLUGINGETFINDDATAW)(HANDLE hPlugin,PluginPanelItem **pPanelItem,int *pItemsNumber,int OpMode);
typedef int (WINAPI *PLUGINMINFARVERSIONW)();
typedef void (WINAPI *PLUGINGETOPENPLUGININFOW)(HANDLE hPlugin,OpenPluginInfo *Info);
typedef void (WINAPI *PLUGINGETPLUGININFOW)(PluginInfo *Info);
typedef int (WINAPI *PLUGINGETVIRTUALFINDDATAW)(HANDLE hPlugin,PluginPanelItem **pPanelItem,int *pItemsNumber,const wchar_t *Path);
typedef int (WINAPI *PLUGINMAKEDIRECTORYW)(HANDLE hPlugin,const wchar_t *Name,int OpMode);
typedef HANDLE (WINAPI *PLUGINOPENFILEPLUGINW)(const wchar_t *Name,const unsigned char *Data,int DataSize,int OpMode);
typedef HANDLE (WINAPI *PLUGINOPENPLUGINW)(int OpenFrom,INT_PTR Item);
typedef int (WINAPI *PLUGINPROCESSEDITOREVENTW)(int Event,void *Param);
typedef int (WINAPI *PLUGINPROCESSEDITORINPUTW)(const INPUT_RECORD *Rec);
typedef int (WINAPI *PLUGINPROCESSEVENTW)(HANDLE hPlugin,int Event,void *Param);
typedef int (WINAPI *PLUGINPROCESSHOSTFILEW)(HANDLE hPlugin,PluginPanelItem *PanelItem,int ItemsNumber,int OpMode);
typedef int (WINAPI *PLUGINPROCESSKEYW)(HANDLE hPlugin,int Key,unsigned int ControlState);
typedef int (WINAPI *PLUGINPUTFILESW)(HANDLE hPlugin,PluginPanelItem *PanelItem,int ItemsNumber,int Move,int OpMode);
typedef int (WINAPI *PLUGINSETDIRECTORYW)(HANDLE hPlugin,const wchar_t *Dir,int OpMode);
typedef int (WINAPI *PLUGINSETFINDLISTW)(HANDLE hPlugin,const PluginPanelItem *PanelItem,int ItemsNumber);
typedef void (WINAPI *PLUGINSETSTARTUPINFOW)(const PluginStartupInfo *Info);
typedef int (WINAPI *PLUGINPROCESSVIEWEREVENTW)(int Event,void *Param); //* $ 27.09.2000 SVS -  События во вьювере


class PluginW: public Plugin
{
private:

	PluginManager *m_owner; //BUGBUG

	string m_strModuleName;

	BitFlags WorkFlags;      // рабочие флаги текущего плагина
	BitFlags FuncFlags;      // битовые маски вызова эксп.функций плагина

	HMODULE m_hModule;
	FAR_FIND_DATA_EX FindData;
	Language Lang;

  /* $ 21.09.2000 SVS
     поле - системный идентификатор плагина
     Плагин должен сам задавать, например для
     Network      = 0x5774654E (NetW)
     PrintManager = 0x6E614D50 (PMan)  SYSID_PRINTMANAGER
  */
	DWORD SysID;

	int CachePos;
	string strRootKey;

	PLUGINSETSTARTUPINFOW        pSetStartupInfoW;
	PLUGINOPENPLUGINW            pOpenPluginW;
	PLUGINOPENFILEPLUGINW        pOpenFilePluginW;
	PLUGINCLOSEPLUGINW           pClosePluginW;
	PLUGINGETPLUGININFOW         pGetPluginInfoW;
	PLUGINGETOPENPLUGININFOW     pGetOpenPluginInfoW;
	PLUGINGETFINDDATAW           pGetFindDataW;
	PLUGINFREEFINDDATAW          pFreeFindDataW;
	PLUGINGETVIRTUALFINDDATAW    pGetVirtualFindDataW;
	PLUGINFREEVIRTUALFINDDATAW   pFreeVirtualFindDataW;
	PLUGINSETDIRECTORYW          pSetDirectoryW;
	PLUGINGETFILESW              pGetFilesW;
	PLUGINPUTFILESW              pPutFilesW;
	PLUGINDELETEFILESW           pDeleteFilesW;
	PLUGINMAKEDIRECTORYW         pMakeDirectoryW;
	PLUGINPROCESSHOSTFILEW       pProcessHostFileW;
	PLUGINSETFINDLISTW           pSetFindListW;
	PLUGINCONFIGUREW             pConfigureW;
	PLUGINEXITFARW               pExitFARW;
	PLUGINPROCESSKEYW            pProcessKeyW;
	PLUGINPROCESSEVENTW          pProcessEventW;
	PLUGINPROCESSEDITOREVENTW    pProcessEditorEventW;
	PLUGINCOMPAREW               pCompareW;
	PLUGINPROCESSEDITORINPUTW    pProcessEditorInputW;
	PLUGINMINFARVERSIONW         pMinFarVersionW;
	PLUGINPROCESSVIEWEREVENTW    pProcessViewerEventW;

public:

	PluginW (PluginManager *owner, const wchar_t *lpwzModuleName, const FAR_FIND_DATA_EX *fdata = NULL);
	~PluginW ();

	int Load();
	int LoadFromCache();

	int SaveToCache ();

	int Unload (bool bExitFAR = false);

	int GetCacheNumber ();
	bool IsPanelPlugin ();

	bool HasOpenPlugin() { return pOpenPluginW!=NULL; }
	bool HasMakeDirectory() { return pMakeDirectoryW!=NULL; }
	bool HasDeleteFiles() { return pDeleteFilesW!=NULL; }
	bool HasPutFiles() { return pPutFilesW!=NULL; }
	bool HasGetFiles() { return pGetFilesW!=NULL; }
	bool HasSetStartupInfo() { return pSetStartupInfoW!=NULL; }
	bool HasOpenFilePlugin() { return pOpenFilePluginW!=NULL; }
	bool HasClosePlugin() { return pClosePluginW!=NULL; }
	bool HasGetPluginInfo() { return pGetPluginInfoW!=NULL; }
	bool HasGetOpenPluginInfo() { return pGetOpenPluginInfoW!=NULL; }
	bool HasGetFindData() { return pGetFindDataW!=NULL; }
	bool HasFreeFindData() { return pFreeFindDataW!=NULL; }
	bool HasGetVirtualFindData() { return pGetVirtualFindDataW!=NULL; }
	bool HasFreeVirtualFindData() { return pFreeVirtualFindDataW!=NULL; }
	bool HasSetDirectory() { return pSetDirectoryW!=NULL; }
	bool HasProcessHostFile() { return pProcessHostFileW!=NULL; }
	bool HasSetFindList() { return pSetFindListW!=NULL; }
	bool HasConfigure() { return pConfigureW!=NULL; }
	bool HasExitFAR() { return pExitFARW!=NULL; }
	bool HasProcessKey() { return pProcessKeyW!=NULL; }
	bool HasProcessEvent() { return pProcessEventW!=NULL; }
	bool HasProcessEditorEvent() { return pProcessEditorEventW!=NULL; }
	bool HasCompare() { return pCompareW!=NULL; }
	bool HasProcessEditorInput() { return pProcessEditorInputW!=NULL; }
	bool HasMinFarVersion() { return pMinFarVersionW!=NULL; }
	bool HasProcessViewerEvent() { return pProcessViewerEventW!=NULL; }

	const string &GetModuleName() { return m_strModuleName; }
	const FAR_FIND_DATA_EX &GetFindData() {return FindData; }
	DWORD GetSysID() { return SysID; }
	int GetCachePos() { return CachePos; }
	bool CheckWorkFlags(DWORD flags) { return WorkFlags.Check(flags)==TRUE; }
	DWORD GetWorkFlags() { return WorkFlags.Flags; }
	DWORD GetFuncFlags() { return FuncFlags.Flags; }

	int InitLang(const wchar_t *Path,int CountNeed=-1) { return Lang.Init(Path,CountNeed); }
	void CloseLang() { Lang.Close(); }
	const wchar_t *GetMsg (int nID) { return Lang.GetMsg(nID); }

public:

	int SetStartupInfo (bool &bUnloaded);
	int CheckMinFarVersion (bool &bUnloaded);

	HANDLE OpenPlugin (int OpenFrom, INT_PTR Item);
	HANDLE OpenFilePlugin (const wchar_t *Name, const unsigned char *Data, int DataSize, int OpMode);

	int SetFindList (HANDLE hPlugin, const PluginPanelItem *PanelItem, int ItemsNumber);
	int GetFindData (HANDLE hPlugin, PluginPanelItem **pPanelItem, int *pItemsNumber, int OpMode);
	int GetVirtualFindData (HANDLE hPlugin, PluginPanelItem **pPanelItem, int *pItemsNumber, const wchar_t *Path);
	int SetDirectory (HANDLE hPlugin, const wchar_t *Dir, int OpMode);
	int GetFiles (HANDLE hPlugin, PluginPanelItem *PanelItem, int ItemsNumber, int Move, const wchar_t *DestPath, int OpMode);
	int PutFiles (HANDLE hPlugin, PluginPanelItem *PanelItem, int ItemsNumber, int Move, int OpMode);
	int DeleteFiles (HANDLE hPlugin, PluginPanelItem *PanelItem, int ItemsNumber, int OpMode);
	int MakeDirectory (HANDLE hPlugin, const wchar_t *Name, int OpMode);
	int ProcessHostFile (HANDLE hPlugin, PluginPanelItem *PanelItem, int ItemsNumber, int OpMode);
	int ProcessKey (HANDLE hPlugin, int Key, unsigned int dwControlState);
	int ProcessEvent (HANDLE hPlugin, int Event, PVOID Param);
	int Compare (HANDLE hPlugin, const PluginPanelItem *Item1, const PluginPanelItem *Item2, unsigned long Mode);

	void GetOpenPluginInfo (HANDLE hPlugin, OpenPluginInfo *Info);
	void FreeFindData (HANDLE hPlugin, PluginPanelItem *PanelItem, int ItemsNumber);
	void FreeVirtualFindData (HANDLE hPlugin, PluginPanelItem *PanelItem, int ItemsNumber);
	void ClosePlugin (HANDLE hPlugin);

	int ProcessEditorInput (const INPUT_RECORD *D);
	int ProcessEditorEvent (int Event, PVOID Param);
	int ProcessViewerEvent (int Event, PVOID Param);

	void GetPluginInfo (PluginInfo *pi);
	int Configure (int MenuItem);

	void ExitFAR();

private:

	void ClearExports ();
};

#endif  // __PLUGINW_HPP__
