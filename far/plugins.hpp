#ifndef __PLUGINS_HPP__
#define __PLUGINS_HPP__
/*
plugins.hpp

Работа с плагинами (низкий уровень, кое-что повыше в flplugin.cpp)

*/

/* Revision: 1.00 25.06.2000 $ */

/*
Modify:
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

class SaveScreen;
class Editor;

typedef void (WINAPI *PLUGINSETSTARTUPINFO)(struct PluginStartupInfo *Info);
typedef HANDLE (WINAPI *PLUGINOPENPLUGIN)(int OpenFrom,int Item);
typedef HANDLE (WINAPI *PLUGINOPENFILEPLUGIN)(char *Name,const unsigned char *Data,int DataSize);
typedef void (WINAPI *PLUGINCLOSEPLUGIN)(HANDLE hPlugin);
typedef void (WINAPI *PLUGINGETPLUGININFO)(struct PluginInfo *Info);
typedef void (WINAPI *PLUGINGETOPENPLUGININFO)(HANDLE hPlugin,struct OpenPluginInfo *Info);
typedef int (WINAPI *PLUGINGETFINDDATA)(HANDLE hPlugin,struct PluginPanelItem **pPanelItem,int *pItemsNumber,int OpMode);
typedef void (WINAPI *PLUGINFREEFINDDATA)(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber);
typedef int (WINAPI *PLUGINGETVIRTUALFINDDATA)(HANDLE hPlugin,struct PluginPanelItem **pPanelItem,int *pItemsNumber,char *Path);
typedef void (WINAPI *PLUGINFREEVIRTUALFINDDATA)(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber);
typedef int (WINAPI *PLUGINSETDIRECTORY)(HANDLE hPlugin,char *Dir,int OpMode);
typedef int (WINAPI *PLUGINGETFILES)(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int Move,char *DestPath,int OpMode);
typedef int (WINAPI *PLUGINPUTFILES)(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int Move,int OpMode);
typedef int (WINAPI *PLUGINDELETEFILES)(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int OpMode);
typedef int (WINAPI *PLUGINMAKEDIRECTORY)(HANDLE hPlugin,char *Name,int OpMode);
typedef int (WINAPI *PLUGINPROCESSHOSTFILE)(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int OpMode);
typedef int (WINAPI *PLUGINSETFINDLIST)(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber);
typedef int (WINAPI *PLUGINPROCESSKEY)(HANDLE hPlugin,int Key,unsigned int ControlState);
typedef int (WINAPI *PLUGINPROCESSEVENT)(HANDLE hPlugin,int Event,void *Param);
typedef int (WINAPI *PLUGINPROCESSEDITOREVENT)(int Event,void *Param);
typedef int (WINAPI *PLUGINCONFIGURE)(int ItemNumber);
typedef void (WINAPI *PLUGINEXITFAR)();
typedef int (WINAPI *PLUGINCOMPARE)(HANDLE hPlugin,struct PluginPanelItem *Item1,struct PluginPanelItem *Item2,unsigned int Mode);
typedef int (WINAPI *PLUGINPROCESSEDITORINPUT)(INPUT_RECORD *Rec);

struct PluginItem
{
  char ModuleName[NM];
  HMODULE hModule;
  WIN32_FIND_DATA FindData;
  Language Lang;
  int Cached;
  int CachePos;
  int EditorPlugin;
  char RootKey[512];
  PLUGINSETSTARTUPINFO pSetStartupInfo;
  PLUGINOPENPLUGIN pOpenPlugin;
  PLUGINOPENFILEPLUGIN pOpenFilePlugin;
  PLUGINCLOSEPLUGIN pClosePlugin;
  PLUGINGETPLUGININFO pGetPluginInfo;
  PLUGINGETOPENPLUGININFO pGetOpenPluginInfo;
  PLUGINGETFINDDATA pGetFindData;
  PLUGINFREEFINDDATA pFreeFindData;
  PLUGINGETVIRTUALFINDDATA pGetVirtualFindData;
  PLUGINFREEVIRTUALFINDDATA pFreeVirtualFindData;
  PLUGINSETDIRECTORY pSetDirectory;
  PLUGINGETFILES pGetFiles;
  PLUGINPUTFILES pPutFiles;
  PLUGINDELETEFILES pDeleteFiles;
  PLUGINMAKEDIRECTORY pMakeDirectory;
  PLUGINPROCESSHOSTFILE pProcessHostFile;
  PLUGINSETFINDLIST pSetFindList;
  PLUGINCONFIGURE pConfigure;
  PLUGINEXITFAR pExitFAR;
  PLUGINPROCESSKEY pProcessKey;
  PLUGINPROCESSEVENT pProcessEvent;
  PLUGINPROCESSEDITOREVENT pProcessEditorEvent;
  PLUGINCOMPARE pCompare;
  PLUGINPROCESSEDITORINPUT pProcessEditorInput;
};

class PluginsSet
{
  private:
    int LoadPlugin(struct PluginItem &CurPlugin,int ModuleNumber,int Init);
    void SetPluginStartupInfo(struct PluginItem &CurPlugin,int ModuleNumber);
    int PreparePlugin(int PluginNumber);
    int GetCacheNumber(char *FullName,WIN32_FIND_DATA *FindData,int CachePos);
    int SavePluginSettings(struct PluginItem &CurPlugin,WIN32_FIND_DATA &FindData);
    void LoadIfCacheAbsent();
    void ReadUserBackgound(SaveScreen *SaveScr);
    int GetHotKeyRegKey(int PluginNumber,int ItemNumber,char *RegKey);
  public:
    PluginsSet();
    ~PluginsSet();
    void LoadPlugins();
    HANDLE OpenPlugin(int PluginNumber,int OpenFrom,int Item);
    HANDLE OpenFilePlugin(char *Name,const unsigned char *Data,int DataSize);
    HANDLE OpenFindListPlugin(PluginPanelItem *PanelItem,int ItemsNumber);
    void ClosePlugin(HANDLE hPlugin);
    int GetPluginInfo(int PluginNumber,struct PluginInfo *Info);
    void GetOpenPluginInfo(HANDLE hPlugin,struct OpenPluginInfo *Info);
    int GetFindData(HANDLE hPlugin,PluginPanelItem **pPanelItem,int *pItemsNumber,int Silent);
    void FreeFindData(HANDLE hPlugin,PluginPanelItem *PanelItem,int ItemsNumber);
    int GetVirtualFindData(HANDLE hPlugin,PluginPanelItem **pPanelItem,int *pItemsNumber,char *Path);
    void FreeVirtualFindData(HANDLE hPlugin,PluginPanelItem *PanelItem,int ItemsNumber);
    int SetDirectory(HANDLE hPlugin,char *Dir,int OpMode);
    int GetFile(HANDLE hPlugin,struct PluginPanelItem *PanelItem,char *DestPath,char *ResultName,int OpMode);
    int GetFiles(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int Move,char *DestPath,int OpMode);
    int PutFiles(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int Move,int OpMode);
    int DeleteFiles(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int OpMode);
    int MakeDirectory(HANDLE hPlugin,char *Name,int OpMode);
    int ProcessHostFile(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int OpMode);
    int ProcessKey(HANDLE hPlugin,int Key,unsigned int ControlState);
    int ProcessEvent(HANDLE hPlugin,int Event,void *Param);
    int Compare(HANDLE hPlugin,struct PluginPanelItem *Item1,struct PluginPanelItem *Item2,unsigned int Mode);
    int ProcessEditorInput(INPUT_RECORD *Rec);
    void ProcessEditorEvent(int Event,void *Param);
    void SendExit();
    char* FarGetMsg(int PluginNumber,int MsgId);
    void Configure();
    int CommandsMenu(int Editor,int Viewer,int StartPos);
    int GetDiskMenuItem(int PluginNumber,int PluginItem,int &ItemPresent,int &PluginTextNumber,char *PluginText);
    int UseFarCommand(HANDLE hPlugin,int CommandType);
    void ReloadLanguage();
    void DiscardCache();
    int ProcessCommandLine(char *Command);

    struct PluginItem *PluginsData;
    int PluginsCount;

    Editor *CurEditor;
};

#endif	// __PLUGINS_HPP__
