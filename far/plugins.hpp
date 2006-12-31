#ifndef __PLUGINS_HPP__
#define __PLUGINS_HPP__
/*
plugins.hpp

Работа с плагинами (низкий уровень, кое-что повыше в flplugin.cpp)

*/

/* Revision: 1.43 06.06.2006 $ */

#include "language.hpp"
#include "bitflags.hpp"
#include "unicodestring.hpp"
#include "struct.hpp"

class SaveScreen;
class FileEditor;
class Viewer;
class Frame;
class Panel;

typedef void (WINAPI *PLUGINCLOSEPLUGIN)(HANDLE hPlugin);
typedef int (WINAPI *PLUGINCOMPARE)(HANDLE hPlugin,const struct PluginPanelItemW *Item1,const struct PluginPanelItemW *Item2,unsigned int Mode);
typedef int (WINAPI *PLUGINCONFIGURE)(int ItemNumber);
typedef int (WINAPI *PLUGINDELETEFILES)(HANDLE hPlugin,struct PluginPanelItemW *PanelItem,int ItemsNumber,int OpMode);
typedef void (WINAPI *PLUGINEXITFAR)();
typedef void (WINAPI *PLUGINFREEFINDDATA)(HANDLE hPlugin,struct PluginPanelItemW *PanelItem,int ItemsNumber);
typedef void (WINAPI *PLUGINFREEVIRTUALFINDDATA)(HANDLE hPlugin,struct PluginPanelItemW *PanelItem,int ItemsNumber);
typedef int (WINAPI *PLUGINGETFILES)(HANDLE hPlugin,struct PluginPanelItemW *PanelItem,int ItemsNumber,int Move,const wchar_t *DestPath,int OpMode);
typedef int (WINAPI *PLUGINGETFINDDATA)(HANDLE hPlugin,struct PluginPanelItemW **pPanelItem,int *pItemsNumber,int OpMode);
typedef int (WINAPI *PLUGINMINFARVERSION)();
typedef void (WINAPI *PLUGINGETOPENPLUGININFO)(HANDLE hPlugin,struct OpenPluginInfoW *Info);
typedef void (WINAPI *PLUGINGETPLUGININFO)(struct PluginInfoW *Info);
typedef int (WINAPI *PLUGINGETVIRTUALFINDDATA)(HANDLE hPlugin,struct PluginPanelItemW **pPanelItem,int *pItemsNumber,const wchar_t *Path);
typedef int (WINAPI *PLUGINMAKEDIRECTORY)(HANDLE hPlugin,const wchar_t *Name,int OpMode);
typedef HANDLE (WINAPI *PLUGINOPENFILEPLUGIN)(const wchar_t *Name,const unsigned char *Data,int DataSize);
typedef HANDLE (WINAPI *PLUGINOPENPLUGIN)(int OpenFrom,int Item);
typedef int (WINAPI *PLUGINPROCESSEDITOREVENT)(int Event,void *Param);
typedef int (WINAPI *PLUGINPROCESSEDITORINPUT)(const INPUT_RECORD *Rec);
typedef int (WINAPI *PLUGINPROCESSEVENT)(HANDLE hPlugin,int Event,void *Param);
typedef int (WINAPI *PLUGINPROCESSHOSTFILE)(HANDLE hPlugin,struct PluginPanelItemW *PanelItem,int ItemsNumber,int OpMode);
typedef int (WINAPI *PLUGINPROCESSKEY)(HANDLE hPlugin,int Key,unsigned int ControlState);
typedef int (WINAPI *PLUGINPUTFILES)(HANDLE hPlugin,struct PluginPanelItemW *PanelItem,int ItemsNumber,int Move,int OpMode);
typedef int (WINAPI *PLUGINSETDIRECTORY)(HANDLE hPlugin,const wchar_t *Dir,int OpMode);
typedef int (WINAPI *PLUGINSETFINDLIST)(HANDLE hPlugin,const struct PluginPanelItemW *PanelItem,int ItemsNumber);
typedef void (WINAPI *PLUGINSETSTARTUPINFO)(const struct PluginStartupInfo *Info);
typedef int (WINAPI *PLUGINPROCESSVIEWEREVENT)(int Event,void *Param); //* $ 27.09.2000 SVS -  События во вьювере

// флаги для поля Plugin.WorkFlags
enum PLUGINITEMWORKFLAGS{
  PIWF_CACHED        = 0x00000001, // кешируется
  PIWF_PRELOADED     = 0x00000002, //
  PIWF_DONTLOADAGAIN = 0x00000004, // не загружать плагин снова, ставится в
                                   //   результате проверки требуемой версии фара
};

// флаги для поля Plugin.FuncFlags - активности функций
enum PLUGINITEMCALLFUNCFLAGS{
  PICFF_LOADED               = 0x00000001, // DLL загружен ;-)
  PICFF_SETSTARTUPINFO       = 0x00000002, //
  PICFF_OPENPLUGIN           = 0x00000004, //
  PICFF_OPENFILEPLUGIN       = 0x00000008, //
  PICFF_CLOSEPLUGIN          = 0x00000010, //
  PICFF_GETPLUGININFO        = 0x00000020, //
  PICFF_GETOPENPLUGININFO    = 0x00000040, //
  PICFF_GETFINDDATA          = 0x00000080, //
  PICFF_FREEFINDDATA         = 0x00000100, //
  PICFF_GETVIRTUALFINDDATA   = 0x00000200, //
  PICFF_FREEVIRTUALFINDDATA  = 0x00000400, //
  PICFF_SETDIRECTORY         = 0x00000800, //
  PICFF_GETFILES             = 0x00001000, //
  PICFF_PUTFILES             = 0x00002000, //
  PICFF_DELETEFILES          = 0x00004000, //
  PICFF_MAKEDIRECTORY        = 0x00008000, //
  PICFF_PROCESSHOSTFILE      = 0x00010000, //
  PICFF_SETFINDLIST          = 0x00020000, //
  PICFF_CONFIGURE            = 0x00040000, //
  PICFF_EXITFAR              = 0x00080000, //
  PICFF_PROCESSKEY           = 0x00100000, //
  PICFF_PROCESSEVENT         = 0x00200000, //
  PICFF_PROCESSEDITOREVENT   = 0x00400000, //
  PICFF_COMPARE              = 0x00800000, //
  PICFF_PROCESSEDITORINPUT   = 0x01000000, //
  PICFF_MINFARVERSION        = 0x02000000, //
  PICFF_PROCESSVIEWEREVENT   = 0x04000000, //

  // PICFF_PANELPLUGIN - первая попытка определиться с понятием "это панель"
  PICFF_PANELPLUGIN          = PICFF_OPENFILEPLUGIN|
                               PICFF_GETFINDDATA|
                               PICFF_FREEFINDDATA|
                               PICFF_GETVIRTUALFINDDATA|
                               PICFF_FREEVIRTUALFINDDATA|
                               PICFF_SETDIRECTORY|
                               PICFF_GETFILES|
                               PICFF_PUTFILES|
                               PICFF_DELETEFILES|
                               PICFF_MAKEDIRECTORY|
                               PICFF_PROCESSHOSTFILE|
                               PICFF_SETFINDLIST|
                               PICFF_PROCESSKEY|
                               PICFF_PROCESSEVENT|
                               PICFF_COMPARE|
                               PICFF_GETOPENPLUGININFO,
};

class Plugin
{
public:

  string strModuleName;
  BitFlags WorkFlags;      // рабочие флаги текущего плагина
  BitFlags FuncFlags;      // битовые маски вызова эксп.функций плагина

  HMODULE hModule;
  FAR_FIND_DATA_EX FindData;
  Language Lang;
  Frame* LinkedFrame;
  /* $ 21.09.2000 SVS
     поле - системный идентификатор плагина
     Плагин должен сам задавать, например для
     Network      = 0x5774654E (NetW)
     PrintManager = 0x6E614D50 (PMan)  SYSID_PRINTMANAGER
  */
  DWORD SysID;
  /* SVS $ */
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
  Plugin ();
};

// флаги для поля PluginsSet.Flags
enum PLUGINSETFLAGS{
  PSIF_ENTERTOOPENPLUGIN        = 0x00000001, // ввалились в плагин OpenPlugin
  PSIF_DIALOG                   = 0x00000002, // была бадяга с диалогом
  PSIF_PLUGINSLOADDED           = 0x80000000, // пагины загружены
};

class PluginsSet
{
public:

    BitFlags Flags;        // флаги манагера плагинов
    DWORD Reserved;        // в будущем это может быть второй порцией флагов

    Plugin **PluginsData;
    int    PluginsCount;
    Plugin *CurPluginItem;

    FileEditor *CurEditor;
    Viewer *CurViewer;     // 27.09.2000 SVS: Указатель на текущий Viewer

private:

    int LoadPlugin(Plugin *CurPlugin,int ModuleNumber,int Init);
    // $ 22.05.2001 DJ возвращает TRUE при успешной загрузке или FALSE, если не прошло GetMinFarVersion()
    int SetPluginStartupInfo(Plugin *CurPlugin,int ModuleNumber);
    int PreparePlugin(int PluginNumber);
    int GetCacheNumber(const wchar_t *FullName,FAR_FIND_DATA_EX *FindData,int CachePos);
    int SavePluginSettings(Plugin *CurPlugin,FAR_FIND_DATA_EX &FindData);
    void LoadIfCacheAbsent();
    void ReadUserBackgound(SaveScreen *SaveScr);
    int GetHotKeyRegKey(int PluginNumber,int ItemNumber,string &strRegKey);
    BOOL TestPluginInfo(Plugin *Item,struct PluginInfoW *Info);
    BOOL TestOpenPluginInfo(Plugin *Item,struct OpenPluginInfoW *Info);

    // $ 03.08.2000 tran - новые методы для проверки минимальной версии
    int  CheckMinVersion(Plugin *CurPlg);
    void ShowMessageAboutIllegalPluginVersion(const wchar_t* plg,int required);

public:

    PluginsSet();
    ~PluginsSet();

public:

    void LoadPlugins();
    void LoadPluginsFromCache();
    BOOL IsPluginsLoaded() {return Flags.Check(PSIF_PLUGINSLOADDED);}

    void SendExit();

    char* FarGetMsg(int PluginNumber,int MsgId);
    void Configure(int StartPos=0);
    void ConfigureCurrent(int PluginNumber,int INum);
    int CommandsMenu(int ModalType,int StartPos,const wchar_t *HistoryName=NULL);
    // $ 21.08.2002 IS - Параметр PluginTextSize, чтобы знать, сколько брать
    int GetDiskMenuItem(int PluginNumber,int PluginItem,int &ItemPresent,
      int &PluginTextNumber, string &strPluginText);

    int UseFarCommand(HANDLE hPlugin,int CommandType);
    void ReloadLanguage();
    void DiscardCache();
    int ProcessCommandLine(const wchar_t *Command,Panel *Target=NULL);

    void UnloadPlugin(Plugin *CurPlg,DWORD Exception);

    // $ .09.2000 SVS - Функция CallPlugin - найти плагин по ID и запустить OpenFrom = OPEN_*
    int CallPlugin(DWORD SysID,int OpenFrom, void *Data);
    int FindPlugin(DWORD SysID);

    void CreatePluginStartupInfo(struct PluginStartupInfo *PSI,
                                 struct FarStandardFunctions *FSF,
                                 const wchar_t *ModuleName,
                                 int ModuleNumber);

    void SetFlags(DWORD NewFlags) { Flags.Set(NewFlags); }
    void SkipFlags(DWORD NewFlags) { Flags.Clear(NewFlags); }
    BOOL CheckFlags(DWORD NewFlags) { return Flags.Check(NewFlags); }

//api functions

public:

	HANDLE OpenPlugin(int PluginNumber,int OpenFrom,int Item);
	HANDLE OpenFilePlugin(const wchar_t *Name,const unsigned char *Data,int DataSize);
	HANDLE OpenFindListPlugin(const PluginPanelItemW *PanelItem,int ItemsNumber);
	void ClosePlugin(HANDLE hPlugin);
	int GetPluginInfo(int PluginNumber,struct PluginInfoW *Info);
	void GetOpenPluginInfo(HANDLE hPlugin,struct OpenPluginInfoW *Info);
	int GetFindData(HANDLE hPlugin,PluginPanelItemW **pPanelItem,int *pItemsNumber,int Silent);
	void FreeFindData(HANDLE hPlugin,PluginPanelItemW *PanelItem,int ItemsNumber);
	int GetVirtualFindData(HANDLE hPlugin,PluginPanelItemW **pPanelItem,int *pItemsNumber,const wchar_t *Path);
	void FreeVirtualFindData(HANDLE hPlugin,PluginPanelItemW *PanelItem,int ItemsNumber);
	int SetDirectory(HANDLE hPlugin,const wchar_t *Dir,int OpMode);
	int GetFile(HANDLE hPlugin,struct PluginPanelItemW *PanelItem,const wchar_t *DestPath,string &strResultName,int OpMode);
	int GetFiles(HANDLE hPlugin,struct PluginPanelItemW *PanelItem,int ItemsNumber,int Move,const wchar_t *DestPath,int OpMode);
	int PutFiles(HANDLE hPlugin,struct PluginPanelItemW *PanelItem,int ItemsNumber,int Move,int OpMode);
	int DeleteFiles(HANDLE hPlugin,struct PluginPanelItemW *PanelItem,int ItemsNumber,int OpMode);
	int MakeDirectory(HANDLE hPlugin,const wchar_t *Name,int OpMode);
	int ProcessHostFile(HANDLE hPlugin,struct PluginPanelItemW *PanelItem,int ItemsNumber,int OpMode);
	int ProcessKey(HANDLE hPlugin,int Key,unsigned int ControlState);
	int ProcessEvent(HANDLE hPlugin,int Event,void *Param);
	int Compare(HANDLE hPlugin,const struct PluginPanelItemW *Item1,const struct PluginPanelItemW *Item2,unsigned int Mode);
	int ProcessEditorInput(INPUT_RECORD *Rec);
	int ProcessEditorEvent(int Event,void *Param);
	int ProcessViewerEvent(int Event,void *Param);
};

#endif  // __PLUGINS_HPP__
