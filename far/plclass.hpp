#ifndef __PLCLASS_HPP__
#define __PLCLASS_HPP__

class PluginManager;

#include "language.hpp"
#include "bitflags.hpp"
#include "unicodestring.hpp"
#include "struct.hpp"
#include "plugin.hpp"

typedef void (WINAPI *PLUGINCLOSEPLUGIN)(HANDLE hPlugin);
typedef int (WINAPI *PLUGINCOMPARE)(HANDLE hPlugin,const PluginPanelItemW *Item1,const PluginPanelItemW *Item2,unsigned int Mode);
typedef int (WINAPI *PLUGINCONFIGURE)(int ItemNumber);
typedef int (WINAPI *PLUGINDELETEFILES)(HANDLE hPlugin,PluginPanelItemW *PanelItem,int ItemsNumber,int OpMode);
typedef void (WINAPI *PLUGINEXITFAR)();
typedef void (WINAPI *PLUGINFREEFINDDATA)(HANDLE hPlugin,PluginPanelItemW *PanelItem,int ItemsNumber);
typedef void (WINAPI *PLUGINFREEVIRTUALFINDDATA)(HANDLE hPlugin,PluginPanelItemW *PanelItem,int ItemsNumber);
typedef int (WINAPI *PLUGINGETFILES)(HANDLE hPlugin,PluginPanelItemW *PanelItem,int ItemsNumber,int Move,const wchar_t *DestPath,int OpMode);
typedef int (WINAPI *PLUGINGETFINDDATA)(HANDLE hPlugin,PluginPanelItemW **pPanelItem,int *pItemsNumber,int OpMode);
typedef int (WINAPI *PLUGINMINFARVERSION)();
typedef void (WINAPI *PLUGINGETOPENPLUGININFO)(HANDLE hPlugin,OpenPluginInfoW *Info);
typedef void (WINAPI *PLUGINGETPLUGININFO)(PluginInfoW *Info);
typedef int (WINAPI *PLUGINGETVIRTUALFINDDATA)(HANDLE hPlugin,PluginPanelItemW **pPanelItem,int *pItemsNumber,const wchar_t *Path);
typedef int (WINAPI *PLUGINMAKEDIRECTORY)(HANDLE hPlugin,const wchar_t *Name,int OpMode);
typedef HANDLE (WINAPI *PLUGINOPENFILEPLUGIN)(const wchar_t *Name,const unsigned char *Data,int DataSize,int OpMode);
typedef HANDLE (WINAPI *PLUGINOPENPLUGIN)(int OpenFrom,INT_PTR Item);
typedef int (WINAPI *PLUGINPROCESSEDITOREVENT)(int Event,void *Param);
typedef int (WINAPI *PLUGINPROCESSEDITORINPUT)(const INPUT_RECORD *Rec);
typedef int (WINAPI *PLUGINPROCESSEVENT)(HANDLE hPlugin,int Event,void *Param);
typedef int (WINAPI *PLUGINPROCESSHOSTFILE)(HANDLE hPlugin,PluginPanelItemW *PanelItem,int ItemsNumber,int OpMode);
typedef int (WINAPI *PLUGINPROCESSKEY)(HANDLE hPlugin,int Key,unsigned int ControlState);
typedef int (WINAPI *PLUGINPUTFILES)(HANDLE hPlugin,PluginPanelItemW *PanelItem,int ItemsNumber,int Move,int OpMode);
typedef int (WINAPI *PLUGINSETDIRECTORY)(HANDLE hPlugin,const wchar_t *Dir,int OpMode);
typedef int (WINAPI *PLUGINSETFINDLIST)(HANDLE hPlugin,const PluginPanelItemW *PanelItem,int ItemsNumber);
typedef void (WINAPI *PLUGINSETSTARTUPINFO)(const PluginStartupInfo *Info);
typedef int (WINAPI *PLUGINPROCESSVIEWEREVENT)(int Event,void *Param); //* $ 27.09.2000 SVS -  События во вьювере


class Plugin
{
public:

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

	PLUGINSETSTARTUPINFO        pSetStartupInfo;
	PLUGINOPENPLUGIN            pOpenPlugin;
	PLUGINOPENFILEPLUGIN        pOpenFilePlugin;
	PLUGINCLOSEPLUGIN           pClosePlugin;
	PLUGINGETPLUGININFO         pGetPluginInfo;
	PLUGINGETOPENPLUGININFO     pGetOpenPluginInfo;
	PLUGINGETFINDDATA           pGetFindData;
	PLUGINFREEFINDDATA          pFreeFindData;
	PLUGINGETVIRTUALFINDDATA    pGetVirtualFindData;
	PLUGINFREEVIRTUALFINDDATA   pFreeVirtualFindData;
	PLUGINSETDIRECTORY          pSetDirectory;
	PLUGINGETFILES              pGetFiles;
	PLUGINPUTFILES              pPutFiles;
	PLUGINDELETEFILES           pDeleteFiles;
	PLUGINMAKEDIRECTORY         pMakeDirectory;
	PLUGINPROCESSHOSTFILE       pProcessHostFile;
	PLUGINSETFINDLIST           pSetFindList;
	PLUGINCONFIGURE             pConfigure;
	PLUGINEXITFAR               pExitFAR;
	PLUGINPROCESSKEY            pProcessKey;
	PLUGINPROCESSEVENT          pProcessEvent;
	PLUGINPROCESSEDITOREVENT    pProcessEditorEvent;
	PLUGINCOMPARE               pCompare;
	PLUGINPROCESSEDITORINPUT    pProcessEditorInput;
	PLUGINMINFARVERSION         pMinFarVersion;
	PLUGINPROCESSVIEWEREVENT    pProcessViewerEvent;

public:

	Plugin (PluginManager *owner, const wchar_t *lpwzModuleName, const FAR_FIND_DATA_EX *fdata = NULL);
	~Plugin ();

	int Load();
	int LoadFromCache();

	int SaveToCache ();

	int Unload (bool bExitFAR = false);

	int GetCacheNumber ();
	bool IsPanelPlugin ();

public:

	int SetStartupInfo (bool &bUnloaded);
	int CheckMinFarVersion (bool &bUnloaded);

	HANDLE OpenPlugin (int OpenFrom, INT_PTR Item);
	HANDLE OpenFilePlugin (const wchar_t *Name, const unsigned char *Data, int DataSize, int OpMode);

	int SetFindList (HANDLE hPlugin, const PluginPanelItemW *PanelItem, int ItemsNumber);
	int GetFindData (HANDLE hPlugin, PluginPanelItemW **pPanelItem, int *pItemsNumber, int OpMode);
	int GetVirtualFindData (HANDLE hPlugin, PluginPanelItemW **pPanelItem, int *pItemsNumber, const wchar_t *Path);
	int SetDirectory (HANDLE hPlugin, const wchar_t *Dir, int OpMode);
	int GetFiles (HANDLE hPlugin, PluginPanelItemW *PanelItem, int ItemsNumber, int Move, const wchar_t *DestPath, int OpMode);
	int PutFiles (HANDLE hPlugin, PluginPanelItemW *PanelItem, int ItemsNumber, int Move, int OpMode);
	int DeleteFiles (HANDLE hPlugin, PluginPanelItemW *PanelItem, int ItemsNumber, int OpMode);
	int MakeDirectory (HANDLE hPlugin, const wchar_t *Name, int OpMode);
	int ProcessHostFile (HANDLE hPlugin, PluginPanelItemW *PanelItem, int ItemsNumber, int OpMode);
	int ProcessKey (HANDLE hPlugin, int Key, unsigned int dwControlState);
	int ProcessEvent (HANDLE hPlugin, int Event, PVOID Param);
	int Compare (HANDLE hPlugin, const PluginPanelItemW *Item1, const PluginPanelItemW *Item2, unsigned long Mode);

	void GetOpenPluginInfo (HANDLE hPlugin, OpenPluginInfoW *Info);
	void FreeFindData (HANDLE hPlugin, PluginPanelItemW *PanelItem, int ItemsNumber);
	void FreeVirtualFindData (HANDLE hPlugin, PluginPanelItemW *PanelItem, int ItemsNumber);
	void ClosePlugin (HANDLE hPlugin);

	int ProcessEditorInput (const INPUT_RECORD *D);
	int ProcessEditorEvent (int Event, PVOID Param);
	int ProcessViewerEvent (int Event, PVOID Param);

	void GetPluginInfo (PluginInfoW *pi);
	int Configure (int MenuItem);

	void ExitFAR();

private:

	void ClearExports ();
};

#endif  // __PLUGINS_HPP__
