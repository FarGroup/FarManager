#define STRICT

#include <windows.h>
#include <CRT/crt.hpp>
#include <stdio.h>
#include <plugin.hpp>
#include <initguid.h>
#include "guid.hpp"

#define WCONST const
#define WTYPE wchar_t**
#define WDEREF *
#define WADDR &

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

#define MAX_CMDLINE     8192    // Max length of displayed process's cmd line
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
	int AddToPluginsMenu;
	int ExportEnvironment;
	int ExportModuleInfo;
	int ExportModuleVersion;
	int ExportPerformance;
	int ExportHandles;
	int ExportHandlesUnnamed;
	int EnableWMI;

	static void Read();
	static void Write();
} Opt;


inline int Message(unsigned Flags, wchar_t *HelpTopic, LPCTSTR*Items, int nItems, int nButtons=1)
{
	return Info.Message(&MainGuid, Flags, HelpTopic, Items, nItems, nButtons);
}

extern class ui64Table
{
		unsigned __int64 Table[21];
	public:
		ui64Table();
		unsigned __int64 tenpow(unsigned n);
} *_ui64Table;

class Plist
{
	private:
		void PrintVersionInfo(HANDLE InfoFile, wchar_t* FullPath);
		void Reread();
		void FileTimeToText(FILETIME *CurFileTime,FILETIME *SrcTime,wchar_t *TimeText);
		void PutToCmdLine(wchar_t* tmp);

		DWORD LastUpdateTime;
		wchar_t HostName[64];
		PerfThread* pPerfThread;
		unsigned StartPanelMode;
		unsigned SortMode;

		WMIConnection* pWMI;
		DWORD dwPluginThread;

		void GeneratePanelModes();
		int Menu(unsigned int Flags, const wchar_t *Title, const wchar_t *Bottom, wchar_t *HelpTopic, const struct FarKey *BreakKeys, FarMenuItem *Item, int ItemsNumber)
		{
			return (*Info.Menu)(&MainGuid, -1, -1, 0, Flags, Title, Bottom, HelpTopic, BreakKeys, 0, Item, ItemsNumber);
		}
		static bool TranslateMode(LPCTSTR src, LPTSTR dest);
		void PrintOwnerInfo(HANDLE InfoFile, DWORD dwPid);

		bool ConnectWMI();
		void DisconnectWMI();
		void WmiError();

	public:
		Plist();
		~Plist();
		bool Connect(LPCTSTR pMachine, LPCTSTR pUser=0, LPCTSTR pPasw=0);
		int GetFindData(PluginPanelItem* &pPanelItem,int &pItemsNumber,OPERATION_MODES OpMode);
		void FreeFindData(PluginPanelItem *PanelItem,int ItemsNumber);
		void GetOpenPanelInfo(struct OpenPanelInfo *Info);
		int SetDirectory(wchar_t *Dir,int OpMode);
		int GetFiles(PluginPanelItem *PanelItem,int ItemsNumber,
		             int Move,WCONST WTYPE DestPath,OPERATION_MODES OpMode, _Opt& opt=::Opt);
		int DeleteFiles(PluginPanelItem *PanelItem,int ItemsNumber,OPERATION_MODES OpMode);
		int ProcessEvent(int Event,void *Param);
		int Compare(const PluginPanelItem *Item1, const PluginPanelItem *Item2, unsigned int Mode);
		int ProcessKey(const INPUT_RECORD *Rec);
		PanelMode *PanelModes(size_t& nModes);

		static wchar_t* PrintTitle(int MsgId);

		static bool GetVersionInfo(wchar_t* pFullPath, LPBYTE &pBuffer, wchar_t* &pVersion, wchar_t* &pDesc);
		static bool bInit;
		static PanelMode PanelModesLocal[NPANELMODES], PanelModesRemote[NPANELMODES];
		static wchar_t ProcPanelModesLocal[NPANELMODES][MAX_MODE_STR], ProcPanelModesRemote[NPANELMODES][MAX_MODE_STR];
		static wchar_t PanelModeBuffer[NPANELMODES*MAX_MODE_STR*4*2];
		static void SavePanelModes();
		static void InitializePanelModes();
		static bool PanelModesInitialized() { return PanelModesLocal[0].ColumnTypes!=0; }
};

struct InitDialogItem
{
	unsigned char Type;
	unsigned char X1,Y1,X2,Y2;
	unsigned char Focus;
	DWORD_PTR Selected;
	unsigned int Flags;
	unsigned char DefaultButton;
	const wchar_t *Data;
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
	wchar_t FullPath[MAX_PATH];
};

struct ProcessDataNT : ProcessData
{
	DWORD dwElapsedTime;
	wchar_t CommandLine[MAX_CMDLINE];
};

extern int NT, W2K;

extern wchar_t *PluginRootKey;

class PerfThread;

BOOL GetList95(PluginPanelItem **pPanelItem,int *pItemsNumber);
BOOL GetListNT(PluginPanelItem **pPanelItem,int *pItemsNumber,PerfThread& PThread);
BOOL KillProcess(DWORD pid);
BOOL KillProcessNT(DWORD pid,HWND hwnd);

const wchar_t *GetMsg(int MsgId);
//void InitDialogItems(InitDialogItem *Init,FarDialogItem *Item, int ItemsNumber);
//int LocalStricmp(wchar_t *Str1,wchar_t *Str2);
void ConvertDate(const FILETIME &ft,wchar_t *DateText,wchar_t *TimeText);
int Config();

void SetRegKey(LPCTSTR Key,LPCTSTR ValueName,LPCTSTR ValueData);
void SetRegKey(LPCTSTR Key,LPCTSTR ValueName,DWORD ValueData);
void SetRegKey(LPCTSTR Key,LPCTSTR ValueName,LPBYTE ValueData,DWORD ValueSize);
int GetRegKey(LPCTSTR Key,LPCTSTR ValueName,LPTSTR ValueData,LPCTSTR Default,DWORD DataSize);
int GetRegKey(LPCTSTR Key,LPCTSTR ValueName,LPBYTE ValueData,LPBYTE Default,DWORD DataSize);
int GetRegKey(LPCTSTR Key,LPCTSTR ValueName,int &ValueData,DWORD Default);
int GetRegKey(LPCTSTR Key,LPCTSTR ValueName,DWORD Default);
void DeleteRegKey(LPCTSTR Key);

int WinError(wchar_t* pSourceModule=0);

class DebugToken
{
		// Debug thread token
		static volatile HANDLE hDebugToken;

		// Saved impersonation token
		HANDLE hSavedToken;

		bool saved;
		bool enabled;

	public:
		DebugToken(): hSavedToken(NULL), saved(false), enabled(false) {}
		~DebugToken() { Revert(); }

		bool Enable();
		bool Revert();

		static bool CreateToken();
		static void CloseToken();
};

static inline const wchar_t *__chk_wca(const wchar_t *arg) { return arg; }
#define OUT_STRING(p) __chk_wca(p)
#define CURDIR_STR_TYPE wchar_t
#define OUT_CVT(pp)   __chk_wca(pp)

void GetOpenProcessDataNT(HANDLE hProcess, wchar_t* pProcessName=0, DWORD cbProcessName=0,
                          wchar_t* pFullPath=0, DWORD cbFullPath=0, wchar_t* pCommandLine=0, DWORD cbCommandLine=0,
                          wchar_t** ppEnvStrings=0, CURDIR_STR_TYPE** pCurDir=0);

HANDLE OpenProcessForced(DebugToken* token, DWORD dwFlags, DWORD dwProcessId, BOOL bInh = FALSE);

enum { SM_CUSTOM=64, SM_PID, SM_PARENTPID, SM_PRIOR, SM_PERFCOUNTER,  SM_PERSEC=128 };

extern wchar_t CustomColumns[10][10];

BOOL GetListNT(PluginPanelItem* &pPanelItem,int &ItemsNumber,PerfThread& PThread);
BOOL GetList95(PluginPanelItem*& pPanelItem,int &ItemsNumber);
DWORD GetHeapSize(DWORD dwPID);
wchar_t* PrintNTUptime(void*p);
wchar_t* PrintTime(ULONGLONG ul100ns, bool bDays=true);
void DumpNTCounters(HANDLE InfoFile, PerfThread& PThread, DWORD dwPid, DWORD dwThreads);
void PrintNTCurDirAndEnv(HANDLE InfoFile, HANDLE hProcess, BOOL bExportEnvironment);
void PrintModules95(HANDLE InfoFile, DWORD dwPID, _Opt& opt);
void PrintModulesNT(HANDLE InfoFile, DWORD dwPID, _Opt& opt);
bool PrintHandleInfo(DWORD dwPID, HANDLE file, bool bIncludeUnnamed, PerfThread* pThread=0);
extern bool GetPData95(ProcessData& pdata);
struct ProcessPerfData;
extern bool GetPDataNT(ProcessDataNT& pdata, ProcessPerfData& pd);

//------
// dynamic binding
typedef enum _PROCESSINFOCLASS
{
	ProcessBasicInformation = 0,
	ProcessWow64Information = 26
} PROCESSINFOCLASS;
//
#ifndef STATUS_NOT_IMPLEMENTED
#define STATUS_NOT_IMPLEMENTED           ((LONG)0xC0000002L)
#endif
#ifndef STATUS_INFO_LENGTH_MISMATCH
#define STATUS_INFO_LENGTH_MISMATCH      ((LONG)0xC0000004L)
#endif
//
typedef LONG(WINAPI *PNtQueryInformationProcess)(HANDLE,PROCESSINFOCLASS,PVOID,ULONG,PULONG);
extern PNtQueryInformationProcess pNtQueryInformationProcess;
typedef LONG(WINAPI *PNtQueryInformationThread)(HANDLE, ULONG, PVOID, DWORD, DWORD*);
extern PNtQueryInformationThread pNtQueryInformationThread;
typedef LONG(WINAPI *PNtQueryObject)(HANDLE, DWORD, VOID*, DWORD, VOID*);
extern PNtQueryObject pNtQueryObject;
typedef LONG(WINAPI *PNtQuerySystemInformation)(DWORD, VOID*, DWORD, ULONG*);
extern PNtQuerySystemInformation pNtQuerySystemInformation;
typedef LONG(WINAPI *PNtQueryInformationFile)(HANDLE, PVOID, PVOID, DWORD, DWORD);
extern PNtQueryInformationFile pNtQueryInformationFile;
typedef LONG(WINAPI *PNtQueryInformationFile)(HANDLE, PVOID, PVOID, DWORD, DWORD);
extern PNtQueryInformationFile pNtQueryInformationFile;
typedef BOOL (WINAPI *PIsValidSid)(PSID);
extern PIsValidSid pIsValidSid;
typedef PSID_IDENTIFIER_AUTHORITY(WINAPI *PGetSidIdentifierAuthority)(PSID);
extern PGetSidIdentifierAuthority pGetSidIdentifierAuthority;
typedef PUCHAR(WINAPI *PGetSidSubAuthorityCount)(PSID);
extern PGetSidSubAuthorityCount pGetSidSubAuthorityCount;
typedef PDWORD(WINAPI *PGetSidSubAuthority)(PSID, DWORD);
extern PGetSidSubAuthority pGetSidSubAuthority;
typedef BOOL (WINAPI *PLookupAccountName)(LPCTSTR,LPCTSTR,PSID,LPDWORD,LPTSTR,LPDWORD,PSID_NAME_USE);
extern PLookupAccountName pLookupAccountName;
typedef BOOL (WINAPI *PIsWow64Process)(IN HANDLE hProcess, IN PBOOL Wow64Process);
extern PIsWow64Process pIsWow64Process;
typedef DWORD (WINAPI *PGetGuiResources)(IN HANDLE hProcess, IN DWORD uiFlags);
extern PGetGuiResources pGetGuiResources;
typedef HRESULT(WINAPI *PCoSetProxyBlanket)(IUnknown*, DWORD, DWORD, OLECHAR*, DWORD, DWORD, RPC_AUTH_IDENTITY_HANDLE, DWORD);
extern PCoSetProxyBlanket pCoSetProxyBlanket;
//------

extern wchar_t FPRINTFbuffer[];
#define FPRINTFbufferLen (64*1024)

#define fprintf(FPRINTFstream, FPRINTFformat, ...) \
	{ \
		int FPRINTFret=0; \
		HANDLE FPRINTFhFile = (HANDLE)(FPRINTFstream); \
		DWORD FPRINTFtmp; \
		if (FPRINTFformat) \
		{ \
			FPRINTFret=FSF.snprintf(FPRINTFbuffer,FPRINTFbufferLen,FPRINTFformat, __VA_ARGS__); \
			if (WriteFile(FPRINTFhFile,FPRINTFbuffer,FPRINTFret*sizeof(wchar_t),&FPRINTFtmp,NULL)) \
				FPRINTFret = (FPRINTFtmp + sizeof(wchar_t)-1) / sizeof(wchar_t); \
		} \
	}

#define fprintf2(FPRINTFret, FPRINTFstream, FPRINTFformat, ...) \
	{ \
		FPRINTFret=0; \
		HANDLE FPRINTFhFile = (HANDLE)(FPRINTFstream); \
		DWORD FPRINTFtmp; \
		if (FPRINTFformat) \
		{ \
			FPRINTFret=FSF.snprintf(FPRINTFbuffer,FPRINTFbufferLen,FPRINTFformat, __VA_ARGS__); \
			if (WriteFile(FPRINTFhFile,FPRINTFbuffer,FPRINTFret*sizeof(wchar_t),&FPRINTFtmp,NULL)) \
				FPRINTFret = (FPRINTFtmp + sizeof(wchar_t)-1) / sizeof(wchar_t); \
		} \
	}

int fputc(int c, HANDLE stream);

//-------
#define NORM_M_PREFIX(m)  (*(LPDWORD)m==0x5c005c)
#define REV_M_PREFIX(m)   (*(LPDWORD)m==0x2f002f)
