#ifndef __PLUGINS_HPP__
#define __PLUGINS_HPP__
/*
plugins.hpp

Работа с плагинами (низкий уровень, кое-что повыше в flplugin.cpp)

*/

/* Revision: 1.22 28.01.2002 $ */

/*
Modify:
  28.01.2002 SVS
    + CreatePluginStartupInfo() - создание/заполнение PSI & FSF
  25.01.2002 SVS
    + PIWF_PRELOADED
  23.01.2002 SVS
    + PICFF_PANELPLUGIN - первая попытка определиться с понятием "это панель"
  22.01.2002 SVS
    ! Удалено поле PluginItem.EditorPlugin - нигде не встретил.
    + PluginItem.*Flags - рабочие флаги пункта и флаги вызова эксп.функций
  15.01.2002 SVS
    ! CurEditor теперь - FileEditor, а не Editor (первая стадия отучения
      класса Editor от слова "Файл")
  20.09.2001 SVS
    + Новое поле Flags у класса PluginsSet.
  25.06.2001 IS
    ! Внедрение const
  07.06.2001 SVS
    ! Configure() имеет параметр, дабы не скакал курсор все время в начало.
  03.06.2001 SVS
    + ConfigureCurrent() - вызов конфига конкретного плагина
  22.05.2001 DJ
    ! SetPluginStartupInfo() возвращает TRUE при удачной загрузке
  16.05.2001 SVS
    ! Метод DumpPluginsInfo - в морг. Есть "штатные" средства записи
      информации о плагинах :-)
  16.05.2001 SVS
    ! enum ExceptFunctionsType переехала в farexcpt.hpp
  06.05.2001 DJ
    ! перетрях #include
  04.05.2001 OT
    + Неверно формировалось меню плагинов по F11 (NWZ)
      Изменился PluginSet::CommandsMenu()
  26.03.2001 SVS
    + дополнительный параметр у CommandsMenu() - HistoryName
  31.10.2000 SVS
    + Функция TestOpenPluginInfo - проверка на вшивость переданных плагином
      данных
  23.10.2000 SVS
    + Функция TestPluginInfo - проверка на вшивость переданных плагином данных
  12.10.2000 tran 1.05
    + DumpPluginsInfo()
  27.09.2000 SVS
    + Функция CallPlugin - найти плагин по ID и запустить
    + CurViewer
    + pProcessViewerEvent
  21.09.2000 SVS
    + поле SysID - системный идентификатор плагина
  01.09.2000 tran 1.02
    + PluginsSet::LoadPluginsFromCache()
  03.08.2000 tran 1.01
    + GetFarMinVersion
      и три новых метода
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#include "language.hpp"
#include "bitflags.hpp"

class SaveScreen;
class FileEditor;
class Viewer;
class Frame;

typedef void (WINAPI *PLUGINCLOSEPLUGIN)(HANDLE hPlugin);
typedef int (WINAPI *PLUGINCOMPARE)(HANDLE hPlugin,const struct PluginPanelItem *Item1,const struct PluginPanelItem *Item2,unsigned int Mode);
typedef int (WINAPI *PLUGINCONFIGURE)(int ItemNumber);
typedef int (WINAPI *PLUGINDELETEFILES)(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int OpMode);
typedef void (WINAPI *PLUGINEXITFAR)();
typedef void (WINAPI *PLUGINFREEFINDDATA)(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber);
typedef void (WINAPI *PLUGINFREEVIRTUALFINDDATA)(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber);
typedef int (WINAPI *PLUGINGETFILES)(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int Move,char *DestPath,int OpMode);
typedef int (WINAPI *PLUGINGETFINDDATA)(HANDLE hPlugin,struct PluginPanelItem **pPanelItem,int *pItemsNumber,int OpMode);
typedef int (WINAPI *PLUGINMINFARVERSION)();
typedef void (WINAPI *PLUGINGETOPENPLUGININFO)(HANDLE hPlugin,struct OpenPluginInfo *Info);
typedef void (WINAPI *PLUGINGETPLUGININFO)(struct PluginInfo *Info);
typedef int (WINAPI *PLUGINGETVIRTUALFINDDATA)(HANDLE hPlugin,struct PluginPanelItem **pPanelItem,int *pItemsNumber,const char *Path);
typedef int (WINAPI *PLUGINMAKEDIRECTORY)(HANDLE hPlugin,char *Name,int OpMode);
typedef HANDLE (WINAPI *PLUGINOPENFILEPLUGIN)(char *Name,const unsigned char *Data,int DataSize);
typedef HANDLE (WINAPI *PLUGINOPENPLUGIN)(int OpenFrom,int Item);
typedef int (WINAPI *PLUGINPROCESSEDITOREVENT)(int Event,void *Param);
typedef int (WINAPI *PLUGINPROCESSEDITORINPUT)(const INPUT_RECORD *Rec);
typedef int (WINAPI *PLUGINPROCESSEVENT)(HANDLE hPlugin,int Event,void *Param);
typedef int (WINAPI *PLUGINPROCESSHOSTFILE)(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int OpMode);
typedef int (WINAPI *PLUGINPROCESSKEY)(HANDLE hPlugin,int Key,unsigned int ControlState);
typedef int (WINAPI *PLUGINPUTFILES)(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int Move,int OpMode);
typedef int (WINAPI *PLUGINSETDIRECTORY)(HANDLE hPlugin,const char *Dir,int OpMode);
typedef int (WINAPI *PLUGINSETFINDLIST)(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber);
typedef void (WINAPI *PLUGINSETSTARTUPINFO)(const struct PluginStartupInfo *Info);
typedef int (WINAPI *PLUGINPROCESSVIEWEREVENT)(int Event,void *Param); //* $ 27.09.2000 SVS -  События во вьювере

// флаги для поля PluginItem.WorkFlags
enum PLUGINITEMWORKFLAGS{
  PIWF_CACHED        = 0x00000001, // кешируется
  PIWF_PRELOADED     = 0x00000002, //
  PIWF_DONTLOADAGAIN = 0x00000004, // не загружать плагин снова, ставится в
                                   //   результате проверки требуемой версии фара
};

// флаги для поля PluginItem.FuncFlags - активности функций
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

struct PluginItem
{
  char ModuleName[NM];
  BitFlags WorkFlags;      // рабочие флаги текущего плагина
  BitFlags FuncFlags;      // битовые маски вызова эксп.функций плагина

  HMODULE hModule;
  WIN32_FIND_DATA FindData;
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
  char RootKey[512];

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
};

// флаги для поля PluginsSet.Flags
enum PLUGINSETFLAGS{
  PSIF_ENTERTOOPENPLUGIN        = 0x00000001, // ввалились в плагин OpenPlugin
  PSIF_DIALOG                   = 0x00000002, // была бадяга с диалогом
};

class PluginsSet
{
  public:
    BitFlags Flags;        // флаги манагера плагинов
    DWORD Reserved;        // в будущем это может быть второй порцией флагов

    struct PluginItem *PluginsData;
    int PluginsCount;

    FileEditor *CurEditor;
    Viewer *CurViewer;     // 27.09.2000 SVS: Указатель на текущий Viewer

  private:
    int LoadPlugin(struct PluginItem &CurPlugin,int ModuleNumber,int Init);
    /* $ 22.05.2001 DJ
       возвращает TRUE при успешной загрузке или FALSE, если не прошло
       GetMinFarVersion()
    */
    int SetPluginStartupInfo(struct PluginItem &CurPlugin,int ModuleNumber);
    /* DJ $ */
    int PreparePlugin(int PluginNumber);
    int GetCacheNumber(char *FullName,WIN32_FIND_DATA *FindData,int CachePos);
    int SavePluginSettings(struct PluginItem &CurPlugin,WIN32_FIND_DATA &FindData);
    void LoadIfCacheAbsent();
    void ReadUserBackgound(SaveScreen *SaveScr);
    int GetHotKeyRegKey(int PluginNumber,int ItemNumber,char *RegKey);
    BOOL TestPluginInfo(struct PluginItem& Item,struct PluginInfo *Info);
    BOOL TestOpenPluginInfo(struct PluginItem& Item,struct OpenPluginInfo *Info);

    /* $ 03.08.2000 tran
       новые методы для проверки минимальной версии */
    int  CheckMinVersion(struct PluginItem &CurPlg);
    void ShowMessageAboutIllegalPluginVersion(char* plg,int required);
    /* tran 03.08.2000 $ */

  public:
    PluginsSet();
    ~PluginsSet();

  public:
    void LoadPlugins();
    void LoadPluginsFromCache();
    HANDLE OpenPlugin(int PluginNumber,int OpenFrom,int Item);
    HANDLE OpenFilePlugin(char *Name,const unsigned char *Data,int DataSize);
    HANDLE OpenFindListPlugin(PluginPanelItem *PanelItem,int ItemsNumber);
    void ClosePlugin(HANDLE hPlugin);
    int GetPluginInfo(int PluginNumber,struct PluginInfo *Info);
    void GetOpenPluginInfo(HANDLE hPlugin,struct OpenPluginInfo *Info);
    int GetFindData(HANDLE hPlugin,PluginPanelItem **pPanelItem,int *pItemsNumber,int Silent);
    void FreeFindData(HANDLE hPlugin,PluginPanelItem *PanelItem,int ItemsNumber);
    int GetVirtualFindData(HANDLE hPlugin,PluginPanelItem **pPanelItem,int *pItemsNumber,const char *Path);
    void FreeVirtualFindData(HANDLE hPlugin,PluginPanelItem *PanelItem,int ItemsNumber);
    int SetDirectory(HANDLE hPlugin,const char *Dir,int OpMode);
    int GetFile(HANDLE hPlugin,struct PluginPanelItem *PanelItem,char *DestPath,char *ResultName,int OpMode);
    int GetFiles(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int Move,char *DestPath,int OpMode);
    int PutFiles(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int Move,int OpMode);
    int DeleteFiles(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int OpMode);
    int MakeDirectory(HANDLE hPlugin,char *Name,int OpMode);
    int ProcessHostFile(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int OpMode);
    int ProcessKey(HANDLE hPlugin,int Key,unsigned int ControlState);
    int ProcessEvent(HANDLE hPlugin,int Event,void *Param);
    int Compare(HANDLE hPlugin,const struct PluginPanelItem *Item1,const struct PluginPanelItem *Item2,unsigned int Mode);
    int ProcessEditorInput(INPUT_RECORD *Rec);
    void ProcessEditorEvent(int Event,void *Param);
    void ProcessViewerEvent(int Event,void *Param);
    void SendExit();
    char* FarGetMsg(int PluginNumber,int MsgId);
    void Configure(int StartPos=0);
    void ConfigureCurrent(int PluginNumber,int INum);
    int CommandsMenu(int ModalType,int StartPos,char *HistoryName=NULL);
    int GetDiskMenuItem(int PluginNumber,int PluginItem,int &ItemPresent,int &PluginTextNumber,char *PluginText);
    int UseFarCommand(HANDLE hPlugin,int CommandType);
    void ReloadLanguage();
    void DiscardCache();
    int ProcessCommandLine(char *Command);

    void UnloadPlugin(struct PluginItem &CurPlg,DWORD Exception);

    /* $ .09.2000 SVS
      Функция CallPlugin - найти плагин по ID и запустить
      OpenFrom = OPEN_*
    */
    int CallPlugin(DWORD SysID,int OpenFrom, void *Data);
    int FindPlugin(DWORD SysID);
    /* SVS $ */

    void CreatePluginStartupInfo(struct PluginStartupInfo *PSI,
                                 struct FarStandardFunctions *FSF,
                                 const char *ModuleName,
                                 int ModuleNumber);

    void SetFlags(DWORD NewFlags) { Flags.Set(NewFlags); }
    void SkipFlags(DWORD NewFlags) { Flags.Skip(NewFlags); }
    BOOL CheckFlags(DWORD NewFlags) { return Flags.Check(NewFlags); }
};

#endif  // __PLUGINS_HPP__
