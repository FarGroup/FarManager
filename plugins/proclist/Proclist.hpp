#define STRICT

#include <windows.h>
#include <stdio.h>
#define _FAR_USE_FARFINDDATA
#include "plugin.hpp"
#ifdef __GNUC__
#include "crt.hpp"
#endif

#ifdef _MSC_VER
#pragma hdrstop
#  pragma comment( lib, "version.lib" )
#endif

#ifndef BELOW_NORMAL_PRIORITY_CLASS
#  define BELOW_NORMAL_PRIORITY_CLASS 0x00004000
#  define ABOVE_NORMAL_PRIORITY_CLASS 0x00008000
typedef unsigned long ULONG_PTR, *PULONG_PTR;
#endif

#ifndef ACTL_GETPOLICIES
#  define ACTL_GETPOLICIES (ACTL_SETARRAYCOLOR+2)
#  define FFPOL_KILLTASK   0x00200000
#endif

#define  Min(x,y) (((x)<(y)) ? (x):(y))
#define  Max(x,y) (((x)>(y)) ? (x):(y))

#define MAX_CMDLINE     512     // Max length of displayed process's cmd line
#define NPANELMODES     10      // Number of panel modes
#define MAX_MODE_STR    80      // Max length of panel mode string and width string
#define MAX_CUSTOM_COLS 20      // Max number of custom cols in any panel mode
#define MAX_DATETIME    50

extern PluginStartupInfo Info;
extern FarStandardFunctions FSF;

class PerfThread;
class WMIConnection;

extern struct _Opt
{
  int AddToDisksMenu;
  int DisksMenuDigit;
  int AddToPluginsMenu;
  int ExportEnvironment;
  int ExportModuleInfo;
  int ExportModuleVersion;
  int ExportPerformance;
  int ExportHandles;
  int EnableWMI;
  //int AnsiOutput;
  static void Read();
  static void Write();
} Opt;


inline int Message(unsigned Flags, char *HelpTopic, PCSTR*Items, int nItems, int nButtons=1) {
    return Info.Message(Info.ModuleNumber, Flags, HelpTopic, Items, nItems, nButtons);
}


class Plist
{
  private:
    void PrintVersionInfo(FILE* InfoFile, char* FullPath);
    void Reread();
    void FileTimeToText(FILETIME *CurFileTime,FILETIME *SrcTime,char *TimeText);
    void PutToCmdLine(char* tmp);

    DWORD LastUpdateTime;
    char HostName[64];
    PerfThread* pPerfThread;
    unsigned StartPanelMode;
    unsigned SortMode;

    WMIConnection* pWMI;
    DWORD dwPluginThread;

    void GeneratePanelModes();
    int Control(int Command, void *Param) { return (*Info.Control)(this, Command, Param); }
    int Menu(unsigned int Flags, char *Title, char *Bottom, char *HelpTopic, int *BreakKeys, FarMenuItem *Item, int ItemsNumber)
    {
        return (*Info.Menu)(Info.ModuleNumber, -1, -1, 0, Flags, Title, Bottom, HelpTopic, BreakKeys,0, Item, ItemsNumber);
    }
    static bool TranslateMode(char* src, char* dest);
    void PrintOwnerInfo(FILE* InfoFile, DWORD dwPid);

    bool ConnectWMI();
    void DisconnectWMI();
    void WmiError();

  public:
    Plist();
    ~Plist();
    bool Connect(LPCSTR pMachine, LPCSTR pUser=0, LPCSTR pPasw=0);
    int GetFindData(PluginPanelItem* &pPanelItem,int &pItemsNumber,int OpMode);
    void FreeFindData(PluginPanelItem *PanelItem,int ItemsNumber);
    void GetOpenPluginInfo(OpenPluginInfo *Info);
    int SetDirectory(char *Dir,int OpMode);
    int GetFiles(PluginPanelItem *PanelItem,int ItemsNumber,
        int Move,char *DestPath,int OpMode, _Opt& opt=::Opt);
    int DeleteFiles(PluginPanelItem *PanelItem,int ItemsNumber,
                    int OpMode);
    int ProcessEvent(int Event,void *Param);
    int Compare(const PluginPanelItem *Item1, const PluginPanelItem *Item2, unsigned int Mode);
    int ProcessKey(int Key,unsigned int ControlState);
    PanelMode* Plist::PanelModes(int& nModes);

    static char* PrintTitle(int MsgId);

  static bool GetVersionInfo(char* pFullPath, char* &pBuffer, char* &pVersion, char* &pDesc);
  static bool bInit;
  static PanelMode PanelModesLocal[NPANELMODES], PanelModesRemote[NPANELMODES];
  static char ProcPanelModesLocal[NPANELMODES][MAX_MODE_STR], ProcPanelModesRemote[NPANELMODES][MAX_MODE_STR];
  static char PanelModeBuffer[NPANELMODES*MAX_MODE_STR*4*2];
  static void SavePanelModes();
  static void InitializePanelModes();
  static bool PanelModesInitialized() { return PanelModesLocal[0].ColumnTypes!=0; }
};

struct InitDialogItem
{
  unsigned char Type;
  unsigned char X1,Y1,X2,Y2;
  unsigned char Focus;
  unsigned int Selected;
  unsigned int Flags;
  unsigned char DefaultButton;
  char *Data;
};

struct ProcessData
{
  DWORD Size;
  HWND hwnd;
//  DWORD Threads;
  DWORD dwPID;
  DWORD dwParentPID;
  DWORD dwPrBase;
  UINT  uAppType;
  char FullPath[NM];
};

struct ProcessDataNT : ProcessData {
    DWORD dwElapsedTime;
    char CommandLine[MAX_CMDLINE];
};

extern int NT, W2K;

extern char PluginRootKey[80];

class PerfThread;

BOOL GetList95(PluginPanelItem **pPanelItem,int *pItemsNumber);
BOOL GetListNT(PluginPanelItem **pPanelItem,int *pItemsNumber,PerfThread& PThread);
BOOL KillProcess(DWORD pid);
BOOL KillProcessNT(DWORD pid,HWND hwnd);

char *GetMsg(int MsgId);
void InitDialogItems(InitDialogItem *Init,FarDialogItem *Item,
                     int ItemsNumber);
int LocalStricmp(char *Str1,char *Str2);
void ConvertDate(const FILETIME &ft,char *DateText,char *TimeText);
int Config();

void SetRegKey(const char *Key,const char *ValueName,char *ValueData);
void SetRegKey(const char *Key,const char *ValueName,DWORD ValueData);
void SetRegKey(const char *Key,const char *ValueName,BYTE *ValueData,DWORD ValueSize);
int GetRegKey(const char *Key,const char *ValueName,char *ValueData,char *Default,DWORD DataSize);
int GetRegKey(const char *Key,const char *ValueName,int &ValueData,DWORD Default);
int GetRegKey(const char *Key,const char *ValueName,DWORD Default);
int GetRegKey(const char *Key,const char *ValueName,BYTE *ValueData,BYTE *Default,DWORD DataSize);
void DeleteRegKey(const char *Key);

int WinError(char* pSourceModule=0, BOOL bDown=FALSE);
BOOL ChangePrivileges(BOOL bAdd, BOOL bAsk);

class OemString {
        char* pStr;
    public:
        OemString(LPCSTR  pAnsi);
        OemString(LPCWSTR pWide);
        ~OemString() { delete pStr; }
        operator char*() { return pStr; }
        operator unsigned char*() { return (unsigned char*)pStr; }
};

void GetOpenProcessDataNT(HANDLE hProcess, char* pProcessName=0, DWORD cbProcessName=0,
        char* pFullPath=0, DWORD cbFullPath=0, char* pCommandLine=0, DWORD cbCommandLine=0,
        char** ppEnvStrings=0, OemString** pCurDir=0);

HANDLE OpenProcessForced(DWORD dwFlags, DWORD dwProcessId, BOOL bInh = FALSE);

enum { SM_CUSTOM=64, SM_PID, SM_PARENTPID, SM_PRIOR, SM_PERFCOUNTER,  SM_PERSEC=128 };

extern char CustomColumns[10][10];

BOOL GetListNT(PluginPanelItem* &pPanelItem,int &ItemsNumber,PerfThread& PThread);
BOOL GetList95(PluginPanelItem*& pPanelItem,int &ItemsNumber);
DWORD GetHeapSize(DWORD dwPID);
char* PrintNTUptime(void*p);
char* PrintTime(ULONGLONG ul100ns, bool bDays=true);
void DumpNTCounters(FILE* InfoFile, PerfThread& PThread, DWORD dwPid, DWORD dwThreads);
void PrintNTCurDirAndEnv(FILE* InfoFile, HANDLE hProcess, BOOL bExportEnvironment);
void PrintModules95(FILE* InfoFile, DWORD dwPID, _Opt& opt);
void PrintModulesNT(FILE* InfoFile, DWORD dwPID, _Opt& opt);
bool PrintHandleInfo(DWORD dwPID, FILE* file, bool bIncludeUnnamed, PerfThread* pThread=0);
extern bool GetPData95(ProcessData& pdata);
struct ProcessPerfData;
extern bool GetPDataNT(ProcessDataNT& pdata, ProcessPerfData& pd);

void MakeViewOptions(FarDialogItem* Items, _Opt& Opt, int offset);
void GetViewOptions(FarDialogItem* Items, _Opt& Opt);
#define NVIEWITEMS 7


#define DYNAMIC_ENTRY(_name, _module)  \
    static P##_name p##_name;   \
    if(!p##_name)               \
       p##_name = (P##_name)GetProcAddress((_module), #_name);
