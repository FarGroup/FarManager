#include "Proclist.hpp"
#include "Proclng.hpp"
#include "perfthread.hpp"

//#ifdef _WIN64
extern "C"
{
  int _fltused = 0;
}
//#endif

// obtained from PSAPI.DLL
/*
struct ModuleData {
    void* p1,*p2;
    TCHAR* pNext;
    void* p3;
    DWORD dw1,dw2;
    HMODULE hModule;
    DWORD dwEntryPoint; //28
    DWORD dwSizeOfImage; //32
    DWORD dw3; //36
    TCHAR* lpModuleFileName; //40
    TCHAR* lpModuleFileName1;
    TCHAR* lpModuleBaseName; //48
    DWORD unknown[5];
};
*/

struct UNICODE_STRING {
  USHORT  Length; USHORT  MaximumLength; PWSTR  Buffer;
};

#if 0
typedef struct _LIST_ENTRY {
  struct _LIST_ENTRY *Flink;
  struct _LIST_ENTRY *Blink;
} LIST_ENTRY, *PLIST_ENTRY;
#endif

#if 0
struct PROCESS_PARAMETERS
{
  ULONG          AllocationSize;
  ULONG          ActualSize;
  ULONG          Flags;
  ULONG          Unknown1;
  UNICODE_STRING Unknown2;
  HANDLE         InputHandle;
  HANDLE         OutputHandle;
  HANDLE         ErrorHandle;
  UNICODE_STRING CurrentDirectory;
  HANDLE         CurrentDirectoryHandle;
  UNICODE_STRING SearchPaths;
  UNICODE_STRING ApplicationName;
  UNICODE_STRING CommandLine;
  PVOID          EnvironmentBlock;
  ULONG          Unknown[9];
  UNICODE_STRING Unknown3;
  UNICODE_STRING Unknown4;
  UNICODE_STRING Unknown5;
  UNICODE_STRING Unknown6;
} ;
#endif

typedef struct _RTL_DRIVE_LETTER_CURDIR {


  USHORT                  Flags;
  USHORT                  Length;
  ULONG                   TimeStamp;
  UNICODE_STRING          DosPath;



} RTL_DRIVE_LETTER_CURDIR, *PRTL_DRIVE_LETTER_CURDIR;


typedef struct _RTL_USER_PROCESS_PARAMETERS {


  ULONG                   MaximumLength;
  ULONG                   Length;
  ULONG                   Flags;
  ULONG                   DebugFlags;
  PVOID                   ConsoleHandle;
  ULONG                   ConsoleFlags;
  HANDLE                  StdInputHandle;
  HANDLE                  StdOutputHandle;
  HANDLE                  StdErrorHandle;
  UNICODE_STRING          CurrentDirectoryPath;
  HANDLE                  CurrentDirectoryHandle;
  UNICODE_STRING          DllPath;
  UNICODE_STRING          ImagePathName;
  UNICODE_STRING          CommandLine;
  PVOID                   EnvironmentBlock;
  ULONG                   StartingPositionLeft;
  ULONG                   StartingPositionTop;
  ULONG                   Width;
  ULONG                   Height;
  ULONG                   CharWidth;
  ULONG                   CharHeight;
  ULONG                   ConsoleTextAttributes;
  ULONG                   WindowFlags;
  ULONG                   ShowWindowFlags;
  UNICODE_STRING          WindowTitle;
  UNICODE_STRING          DesktopName;
  UNICODE_STRING          ShellInfo;
  UNICODE_STRING          RuntimeData;
  RTL_DRIVE_LETTER_CURDIR DLCurrentDirectory[0x20];

} RTL_USER_PROCESS_PARAMETERS, *PRTL_USER_PROCESS_PARAMETERS, PROCESS_PARAMETERS;



typedef struct _LDR_MODULE {
  LIST_ENTRY              InLoadOrderModuleList;
  LIST_ENTRY              InMemoryOrderModuleList;
  LIST_ENTRY              InInitializationOrderModuleList;
  PVOID                   BaseAddress;
  PVOID                   EntryPoint;
  ULONG                   SizeOfImage;
  UNICODE_STRING          FullDllName;
  UNICODE_STRING          BaseDllName;
  ULONG                   Flags;
  SHORT                   LoadCount;
  SHORT                   TlsIndex;
  LIST_ENTRY              HashTableEntry;
  ULONG                   TimeDateStamp;
} LDR_MODULE, ModuleData, *PLDR_MODULE;


typedef struct _PEB_LDR_DATA {
  ULONG                   Length;
  BOOLEAN                 Initialized;
  PVOID                   SsHandle;
  LIST_ENTRY              InLoadOrderModuleList;
  LIST_ENTRY              InMemoryOrderModuleList;
  LIST_ENTRY              InInitializationOrderModuleList;
} PEB_LDR_DATA, *PPEB_LDR_DATA;

typedef struct _PEB {
  BOOLEAN                 InheritedAddressSpace;
  BOOLEAN                 ReadImageFileExecOptions;
  BOOLEAN                 BeingDebugged;
  BOOLEAN                 Spare;
  HANDLE                  Mutant;
  PVOID                   ImageBaseAddress;
  PPEB_LDR_DATA           LoaderData;
  PROCESS_PARAMETERS      *ProcessParameters;
  PVOID                   SubSystemData;
  PVOID                   ProcessHeap;
  PVOID                   FastPebLock;
//  PPEBLOCKROUTINE         FastPebLockRoutine;
  PVOID         FastPebLockRoutine;
//  PPEBLOCKROUTINE         FastPebUnlockRoutine;
  PVOID         FastPebUnlockRoutine;
  ULONG                   EnvironmentUpdateCount;
  PVOID                  *KernelCallbackTable;
  PVOID                   EventLogSection;
  PVOID                   EventLog;
//  PPEB_FREE_BLOCK         FreeList;
  PVOID         FreeList;
  ULONG                   TlsExpansionCounter;
  PVOID                   TlsBitmap;
  ULONG                   TlsBitmapBits[0x2];
  PVOID                   ReadOnlySharedMemoryBase;
  PVOID                   ReadOnlySharedMemoryHeap;
  PVOID                   *ReadOnlyStaticServerData;
  PVOID                   AnsiCodePageData;
  PVOID                   OemCodePageData;
  PVOID                   UnicodeCaseTableData;
  ULONG                   NumberOfProcessors;
  ULONG                   NtGlobalFlag;
  BYTE                    Spare2[0x4];
  LARGE_INTEGER           CriticalSectionTimeout;
  ULONG                   HeapSegmentReserve;
  ULONG                   HeapSegmentCommit;
  ULONG                   HeapDeCommitTotalFreeThreshold;
  ULONG                   HeapDeCommitFreeBlockThreshold;
  ULONG                   NumberOfHeaps;
  ULONG                   MaximumNumberOfHeaps;
  PVOID                  **ProcessHeaps;
  PVOID                   GdiSharedHandleTable;
  PVOID                   ProcessStarterHelper;
  PVOID                   GdiDCAttributeList;
  PVOID                   LoaderLock;
  ULONG                   OSMajorVersion;
  ULONG                   OSMinorVersion;
  ULONG                   OSBuildNumber;
  ULONG                   OSPlatformId;
  ULONG                   ImageSubSystem;
  ULONG                   ImageSubSystemMajorVersion;
  ULONG                   ImageSubSystemMinorVersion;
  ULONG                   GdiHandleBuffer[0x22];
  ULONG                   PostProcessInitRoutine;
  ULONG                   TlsExpansionBitmap;
  BYTE                    TlsExpansionBitmapBits[0x80];
  ULONG                   SessionId;
} PEB, *PPEB;

typedef struct _PROCESS_BASIC_INFORMATION {
    PVOID Reserved1;
    PPEB PebBaseAddress;
    PVOID Reserved2[2];
    ULONG_PTR UniqueProcessId;
    PVOID Reserved3;
} PROCESS_BASIC_INFORMATION;

BOOL GetInternalProcessData( HANDLE hProcess, ModuleData* Data, PROCESS_PARAMETERS* &pProcessParams, char*&pEnd, bool bFirstModule=false)
{
    DWORD ret;

    // From ntddk.h
    PROCESS_BASIC_INFORMATION processInfo;

    if (pNtQueryInformationProcess(hProcess, ProcessBasicInformation, &processInfo, sizeof(processInfo), &ret))
      return FALSE;

    char *p4;

    //FindModule, obtained from PSAPI.DLL

    PVOID hModule;
    PEB peb;
    PEB_LDR_DATA pld;

    if(ReadProcessMemory(hProcess, processInfo.PebBaseAddress, &peb, sizeof(peb), 0) &&
       ReadProcessMemory(hProcess, peb.LoaderData, &pld, sizeof(pld), 0) ) {
       //pEnd = (void *)((void *)peb.LoaderData+((void *)&pld.InMemoryOrderModuleList-(void *)&pld));
       hModule = peb.ImageBaseAddress;
       pProcessParams = peb.ProcessParameters;
       pEnd = (char *)peb.LoaderData+sizeof(pld)-sizeof(LIST_ENTRY)*2;
       p4 = (char *)pld.InMemoryOrderModuleList.Flink;
       while(p4) {
           if(p4==pEnd || !ReadProcessMemory(hProcess, p4-sizeof(PVOID)*2, Data, sizeof(*Data), 0) )
              return FALSE;
           if(bFirstModule)
              return TRUE;
           if(Data->BaseAddress==hModule) break;
           p4 = (char *)Data->InMemoryOrderModuleList.Flink;
       }
    }
    return TRUE;
}


static size_t mwcslen( const wchar_t *str, size_t maxsize=0 )
{
    size_t sz=0;
    for(const wchar_t *p = str; !maxsize || sz<maxsize-1; p++,sz++)
    if(*(long*)p==0)
        break;
    return sz+1;
}

HANDLE OpenProcessForced(DebugToken* token, DWORD dwFlags, DWORD dwProcessId, BOOL bInh)
{
    HANDLE hProcess = OpenProcess(dwFlags, bInh, dwProcessId);
    if(hProcess==NULL && GetLastError()==ERROR_ACCESS_DENIED)
    {
      if (token->Enable())
      {
        hProcess = OpenProcess(dwFlags, bInh, dwProcessId);
      }
    }
    return hProcess;
}

bool GetPDataNT(ProcessDataNT& DATA, ProcessPerfData& pd)
{
    DATA.Size = sizeof(ProcessDataNT);
    DATA.dwPID = pd.dwProcessId;
    DATA.dwPrBase = pd.dwProcessPriority;
    DATA.dwParentPID = pd.dwCreatingPID;
    DATA.dwElapsedTime = pd.dwElapsedTime;
    TCHAR* pFullPath = pd.FullPath;
#ifndef UNICODE
    if(*(DWORD*)pFullPath==0x5C3F3F5C) // "\??\"
#else
    if(*(DWORD*)pFullPath==0x3F005C && ((DWORD*)pFullPath)[1]==0x5C003F) // "\??\"
#endif
      pFullPath += 4;
    lstrcpyn(DATA.FullPath, pFullPath, ArraySize(DATA.FullPath));
    lstrcpyn(DATA.CommandLine, pd.CommandLine, ArraySize(DATA.CommandLine));
    return true;
}

BOOL GetListNT(PluginPanelItem* &pPanelItem,int &ItemsNumber,PerfThread& Thread)
{
    DWORD numTasks;
    ProcessPerfData* pData=0;
//    Lock l(&Thread); // it's already locked in Plist::GetFindData
    FILETIME ftSystemTime;
    //Prepare system time to subtract dwElapsedTime
    GetSystemTimeAsFileTime(&ftSystemTime);

    Thread.GetProcessData(pData, numTasks);
    if(numTasks && !Thread.IsOK())
      return FALSE;

    pPanelItem = new PluginPanelItem[numTasks];
    memset(pPanelItem, 0, numTasks*sizeof(*pPanelItem));
    ItemsNumber=numTasks;

    for (DWORD i=0; i<numTasks; i++)
    {
      PluginPanelItem& CurItem = pPanelItem[i];
      ProcessPerfData& pd = pData[i];

      CurItem.Flags|=PPIF_USERDATA;
#ifndef UNICODE
      lstrcpyn(CurItem.FindData.cFileName,pd.ProcessName,ArraySize(CurItem.FindData.cFileName));
#else
      delete CurItem.FindData.lpwszFileName;  // ???
      CurItem.FindData.lpwszFileName = wcsdup(pd.ProcessName);
#endif
      if(*pd.Owner) {
        CurItem.Owner = new TCHAR[lstrlen(pd.Owner)+1];
        lstrcpy((TCHAR*)CurItem.Owner, pd.Owner);
      }
#ifndef UNICODE
      if(Thread.IsLocal())
        CharToOem(CurItem.FindData.cFileName, CurItem.FindData.cFileName);
#endif

      CurItem.UserData = (DWORD_PTR) new ProcessDataNT;
      memset((void*)CurItem.UserData, 0, sizeof(ProcessDataNT));
      if(!pd.ftCreation.dwHighDateTime && pd.dwElapsedTime)
        *(ULONGLONG*)&pd.ftCreation = *(ULONGLONG*)&ftSystemTime - (ULONGLONG)pd.dwElapsedTime * 10000000;

      CurItem.FindData.ftCreationTime = CurItem.FindData.ftLastWriteTime = CurItem.FindData.ftLastAccessTime = pd.ftCreation;
      ULONGLONG ullSize = pd.qwCounters[IDX_WORKINGSET] + pd.qwCounters[IDX_PAGEFILE];
#ifndef UNICODE
      CurItem.FindData.nFileSizeLow  = ((DWORD*)&ullSize)[0];
      CurItem.FindData.nFileSizeHigh = ((DWORD*)&ullSize)[1];
      CurItem.PackSize     = ((DWORD*)&pd.qwResults[IDX_PAGEFILE])[0];
      CurItem.PackSizeHigh = ((DWORD*)&pd.qwResults[IDX_PAGEFILE])[1];
#else
      CurItem.FindData.nFileSize = ullSize;
      CurItem.FindData.nPackSize = pd.qwResults[IDX_PAGEFILE];
#endif

//yjh:???      CurItem.PackSize = pd.dwProcessId;
#ifdef UNICODE
#define cAlternateFileName  lpwszAlternateFileName
#endif
      if(pd.dwProcessId)
        FSF.itoa(pd.dwProcessId, (TCHAR*)CurItem.FindData.cAlternateFileName, 10);
#undef cAlternateFileName
      CurItem.NumberOfLinks = pd.dwThreads;

      GetPDataNT( *(ProcessDataNT*)CurItem.UserData, pd);
      if(pd.dwProcessId==0 && pd.dwThreads >5)  //_Total
        CurItem.FindData.dwFileAttributes |= FILE_ATTRIBUTE_HIDDEN;
      if(pd.bProcessIsWow64)
        CurItem.FindData.dwFileAttributes |= FILE_ATTRIBUTE_READONLY;
    }//for
    return TRUE;
}

void GetOpenProcessDataNT(HANDLE hProcess, TCHAR* pProcessName, DWORD cbProcessName,
    TCHAR* pFullPath, DWORD cbFullPath, TCHAR* pCommandLine, DWORD cbCommandLine,
    TCHAR** ppEnvStrings, CURDIR_STR_TYPE** psCurDir)
{
    ModuleData Data={0};
    char *pEnd;
    PROCESS_PARAMETERS* pProcessParams = 0;
    if(GetInternalProcessData(hProcess, &Data, pProcessParams, pEnd))
    {
        WCHAR szProcessName[MAX_PATH];
        memset(szProcessName, 0, sizeof(szProcessName));
        if(pProcessName) {
            SIZE_T sz = sizeof(szProcessName);//min(sizeof(szProcessName), Data.BaseDllName.MaximumLength*2);
            if(ReadProcessMemory(hProcess, Data.BaseDllName.Buffer, szProcessName, sz,0))
#ifndef UNICODE
                WideCharToMultiByte( CP_ACP, 0, szProcessName, -1,
                pProcessName, cbProcessName, NULL, NULL);
#else
                lstrcpyn(pProcessName, szProcessName, cbProcessName);
#endif
            else
                *pProcessName = 0;
        }
        if(pFullPath) {
            SIZE_T sz = sizeof(szProcessName);//min(sizeof(szProcessName), Data.FullDllName.MaximumLength*2);
            if(ReadProcessMemory(hProcess, Data.FullDllName.Buffer, szProcessName, sz,0))
#ifndef UNICODE
                WideCharToMultiByte( CP_ACP, 0, szProcessName, -1,
                pFullPath, cbFullPath, NULL, NULL);
#else
               lstrcpyn(pFullPath, szProcessName, cbFullPath);
#endif
            else
                *pFullPath = 0;
        }
        if(pCommandLine) {
            UNICODE_STRING pCmd;
            if(ReadProcessMemory(hProcess, &pProcessParams->CommandLine, &pCmd, sizeof(pCmd), 0)) {
                SIZE_T sz = min(cbCommandLine, (ULONG)pCmd.Length/sizeof(WCHAR) + 1);
                Array<WCHAR> sCommandLine((DWORD)sz);
                *pCommandLine = 0;
                if(ReadProcessMemory(hProcess, pCmd.Buffer, sCommandLine, (sz-1)*sizeof(WCHAR),0)) {
                    sCommandLine[sz-1] = 0;
#ifndef UNICODE
                    WideCharToMultiByte(CP_ACP, 0, sCommandLine, -1,
                                        pCommandLine, cbCommandLine, NULL, NULL);
#else
                    lstrcpyn(pCommandLine, sCommandLine, cbCommandLine);
#endif
                }
            }
        }

        if(ppEnvStrings) {
            TCHAR *pEnv;
            *ppEnvStrings = 0;
            if(ReadProcessMemory(hProcess, &pProcessParams->EnvironmentBlock, &pEnv, sizeof(pEnv), 0)) {
                WCHAR* pwEnvStrings = 0;
                DWORD dwSize = 0;
                while(1) {
                    pwEnvStrings = new WCHAR[dwSize+=1024];
                    if(!ReadProcessMemory(hProcess, pEnv, pwEnvStrings, dwSize*2,0)) {
                        delete pwEnvStrings;
                        pwEnvStrings = 0;
                        break;
                    }
                    if(mwcslen(pwEnvStrings, dwSize)<dwSize)
                        break;
                    delete pwEnvStrings;
                }
                if(pwEnvStrings) {
#ifndef UNICODE
                    dwSize = (DWORD)(mwcslen(pwEnvStrings) + 1);
                    dwSize = WideCharToMultiByte( CP_ACP, 0, pwEnvStrings, dwSize, 0,0,0,0);
                    if(dwSize) {
                        *ppEnvStrings = new char[dwSize];
                        WideCharToMultiByte( CP_ACP, 0, pwEnvStrings, dwSize, *ppEnvStrings,dwSize,0,0);
                        delete pwEnvStrings;
                    }
#else
                    if(pwEnvStrings && *pwEnvStrings)
                      *ppEnvStrings = pwEnvStrings;
                    else
                      delete pwEnvStrings;
#endif
                }
            }
        }
        if(psCurDir) {
            *psCurDir = 0;
            UNICODE_STRING CurDir;
            if(ReadProcessMemory(hProcess, &pProcessParams->CurrentDirectoryPath, &CurDir, sizeof(CurDir), 0)) {
                wchar_t *wsCurDir = new wchar_t[(CurDir.Length+1)/2 + 1];
                if(ReadProcessMemory(hProcess, CurDir.Buffer, wsCurDir, CurDir.Length,0)) {
                  wsCurDir[(CurDir.Length+1)/2] = 0;
#ifndef UNICODE
                  *psCurDir = new OemString(wsCurDir);
                  delete wsCurDir;
#else
                  *psCurDir = wsCurDir;
#endif
                }
            }
        }
    }
}

volatile HANDLE DebugToken::hDebugToken = NULL;

bool DebugToken::Enable()
{
  if (enabled || hDebugToken == NULL)
    return true;

  BOOL rc = OpenThreadToken(GetCurrentThread(), TOKEN_IMPERSONATE, TRUE, &hSavedToken);
  if (!rc)
  {
    hSavedToken = NULL;
    if (GetLastError()==ERROR_NO_TOKEN)
      rc = ERROR_SUCCESS;
    else
      return false;
  }

  rc = SetThreadToken(NULL, hDebugToken);
  if (!rc)
  {
    if (hSavedToken != NULL)
    {
      CloseHandle(hSavedToken);
      hSavedToken = NULL;
    }
    return false;
  }

  enabled = true;
  return true;
}

bool DebugToken::Revert()
{
  if (!enabled)
    return true;

  BOOL rc = SetThreadToken(NULL, hSavedToken);
  if (!rc)
    return false;

  if (hSavedToken != NULL)
  {
    CloseHandle(hSavedToken);
    hSavedToken = NULL;
  }

  enabled = false;
  return true;
}

bool DebugToken::CreateToken()
{
  HANDLE hProcessToken = NULL;
  BOOL rc = OpenProcessToken(GetCurrentProcess(), TOKEN_DUPLICATE, &hProcessToken);
  if (!rc)
    return false;

  HANDLE hToken = NULL;
  rc = DuplicateTokenEx(
         hProcessToken,
         TOKEN_IMPERSONATE | TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES,
         NULL,
         SecurityImpersonation,
         TokenImpersonation,
         &hToken);

  if (!rc)
  {
    CloseHandle(hProcessToken);
    return false;
  }

  TOKEN_PRIVILEGES tp;
  tp.PrivilegeCount = 1;
  tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
  rc = LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &tp.Privileges[0].Luid);
  if (!rc)
  {
    CloseHandle(hToken);
    CloseHandle(hProcessToken);
    return false;
  }

  rc = AdjustTokenPrivileges(
         hToken,
         FALSE,
         &tp,
         sizeof(tp),
         NULL,
         NULL);

  if (!rc)
  {
    CloseHandle(hToken);
    CloseHandle(hProcessToken);
    return false;
  }

  hDebugToken = hToken;
  return true;
}

void DebugToken::CloseToken()
{
  CloseHandle(hDebugToken);
  hDebugToken = NULL;
}

BOOL KillProcessNT(DWORD pid,HWND hwnd)
{
  DebugToken token;
  HANDLE hProcess = OpenProcess( PROCESS_TERMINATE, FALSE, pid );

  // If access denied, try to assign debug privileges
  if (hProcess==NULL && GetLastError()==ERROR_ACCESS_DENIED)
  {
    const TCHAR *MsgItems[]=
    {
      GetMsg(MDeleteTitle),
      GetMsg(MCannotDeleteProc),
      GetMsg(MRetryWithDebug),
      GetMsg(MDangerous),
      GetMsg(MYes),
      GetMsg(MNo)
    };

    if(Message(FMSG_WARNING,NULL,MsgItems,ArraySize(MsgItems),2)==0)
    {
      if (token.Enable())
        hProcess = OpenProcess( PROCESS_TERMINATE, FALSE, pid );
    }
  }

  BOOL bRet = FALSE;

  if (hProcess)
  {
    bRet = TerminateProcess( hProcess, 1 );
    CloseHandle( hProcess );
  }

  return bRet;
}

TCHAR* PrintTime(ULONG s, bool bDays=true)
{
    ULONG m = s/60;
    s %= 60;
    ULONG h = m / 60;
    m %= 60;
    static TCHAR buf[32];
    if(!bDays || h<24)
      FSF.sprintf(buf, _T("%02d:%02d:%02d"), h, m, s);
    else
      FSF.sprintf(buf, _T("%d %02d:%02d:%02d"), h/24, h%24, m, s);
    return buf;
}

TCHAR* PrintTime(ULONGLONG ul100ns, bool bDays)
{
    TCHAR* buf = PrintTime((ULONG)(ul100ns/10000000), bDays);
    FSF.sprintf(buf+lstrlen(buf), _T(".%03d"), (ul100ns/10000)%1000 );
    return buf;
}

TCHAR* PrintNTUptime(void*p)
{
    return PrintTime((ULONG)((ProcessDataNT*)p)->dwElapsedTime);
}

void DumpNTCounters(HANDLE InfoFile, PerfThread& Thread, DWORD dwPid, DWORD dwThreads)
{
    TCHAR tmp[100];
    fputc(_T('\n'),InfoFile);

    Lock l(&Thread);
    ProcessPerfData* pdata = Thread.GetProcessData(dwPid, dwThreads);
    if(!pdata)
      return;
    const PerfLib* pf = Thread.GetPerfLib();

    for(size_t i=0; i<ArraySize(Counters); i++)
    {
      if(!pf->dwCounterTitles[i]) // counter is absent
          continue;
      TCHAR buf[28];
      lstrcpyn(buf,GetMsg(Counters[i].idName),ArraySize(buf)-2);
      lstrcat(buf,_T(":"));
      fprintf(InfoFile, _T("%-24s "), buf);

      switch(pf->CounterTypes[i]) {
          case PERF_COUNTER_RAWCOUNT:
          {
          // Display as is.  No Display Suffix.
              FSF.sprintf(tmp, _T("%10d\n"), *(DWORD*)&pdata->qwResults[i]);
              fprintf(InfoFile, _T("%s"), tmp);
          }
          break;
          case PERF_COUNTER_LARGE_RAWCOUNT: //  same, large int
          {
              FSF.sprintf(tmp, _T("%10.0f\n"), (FLOAT)pdata->qwResults[i]);
              fprintf(InfoFile, _T("%s"), tmp);
          }
          break;
          case PERF_100NSEC_TIMER:
          {
          // 64-bit Timer in 100 nsec units. Display delta divided by
          // delta time.  Display suffix: "%"
              //fprintf(InfoFile, _T("%10.0f%%\n"), (FLOAT)pdata->qwResults[i]);
              FSF.sprintf(tmp, _T("%s %7.0f%%\n"), PrintTime((ULONGLONG)pdata->qwCounters[i]), (FLOAT)pdata->qwResults[i]);
              fprintf(InfoFile, _T("%s"), tmp);
          }
          break;
          case PERF_COUNTER_COUNTER:
          {
          // 32-bit Counter.  Divide delta by delta time.  Display suffix: "/sec"
              fprintf(InfoFile, _T("%10d  %5d%s\n"), *(DWORD*)&pdata->qwCounters[i], *(DWORD*)&pdata->qwResults[i], GetMsg(MperSec));
          }
          break;
          case PERF_COUNTER_BULK_COUNT: //PERF_COUNTER_BULK_COUNT
          {
          // 64-bit Counter.  Divide delta by delta time. Display Suffix: "/sec"
              FSF.sprintf(tmp, _T("%10.0f  %5.0f%s\n"), (FLOAT)pdata->qwCounters[i], (FLOAT)pdata->qwResults[i], GetMsg(MperSec));
              fprintf(InfoFile, _T("%s"), tmp);
          }
          break;
          default:
              fputc(_T('\n'),InfoFile);
      }
    }
}

void PrintNTCurDirAndEnv(HANDLE InfoFile, HANDLE hProcess, BOOL bExportEnvironment)
{
    CURDIR_STR_TYPE* sCurDir=0;
    TCHAR *pEnvStrings = 0;
    GetOpenProcessDataNT(hProcess, 0,0,0,0,0,0, bExportEnvironment ? &pEnvStrings : 0, &sCurDir);

    fputc(_T('\n'),InfoFile);

    if(sCurDir) {
      fprintf(InfoFile,_T("%s %s\n\n"),Plist::PrintTitle(MCurDir),OUT_CVT(sCurDir));
      delete sCurDir;
    }

    if(bExportEnvironment && pEnvStrings)
    {
        fprintf(InfoFile, _T("%s\n\n"), GetMsg(MEnvironment));
        for(TCHAR* p = pEnvStrings; *p; p+=lstrlen(p)+1)
        {
#ifndef UNICODE
          CharToOem(p,p);
#endif
          fprintf(InfoFile, _T("%s\n"), p);
        }
        delete pEnvStrings;
    }
}

void PrintModuleVersion(HANDLE InfoFile, TCHAR* pVersion, TCHAR* pDesc, int len)
{
    //Changes pVersion and pDesc contents!
#ifndef UNICODE
    CharToOem(pVersion,pVersion);
    CharToOem(pDesc,pDesc);
#endif

    do {
        fputc(_T('\t'), InfoFile);
    } while((len=(len|7)+1) < 56);

    int len2=0;
    fprintf2(len2, InfoFile, _T("%s"), pVersion?pVersion:_T(""));
    len += len2;

    if(pDesc) {
      do {
        fputc(_T(' '), InfoFile);
      } while(len++ < 72);
      fprintf(InfoFile, _T("%s"), pDesc);
    }
}

void PrintModulesNT(HANDLE InfoFile, DWORD dwPID, _Opt& Opt)
{
    ModuleData Data;
    DebugToken token;
    HANDLE hProcess = OpenProcessForced(&token, PROCESS_QUERY_INFORMATION|PROCESS_VM_READ|READ_CONTROL, dwPID);
    PROCESS_PARAMETERS* pProcessParams;
    char *pEnd;
    if(hProcess && GetInternalProcessData( hProcess, &Data, pProcessParams, pEnd, true)) {
    char *p4;
    do {
        int len = 0;
        fprintf2(len, InfoFile, _T("  %p  %6X"), Data.BaseAddress, Data.SizeOfImage);
        WCHAR wszModuleName[MAX_PATH];
        SIZE_T sz = sizeof(wszModuleName);//min(sizeof(wszModuleName), Data.BaseDllName.MaximumLength*2);

        if(ReadProcessMemory(hProcess, Data.FullDllName.Buffer, wszModuleName, sz,0)) {
            int len2=0;
            fprintf2(len2, InfoFile, _T(" %s"), OUT_STRING(wszModuleName));
            len += len2;
            TCHAR   *pVersion, *pDesc;
            LPBYTE  pBuf;
            if(Opt.ExportModuleVersion && Plist::GetVersionInfo((TCHAR*)wszModuleName, pBuf, pVersion, pDesc)) {
              PrintModuleVersion(InfoFile, pVersion, pDesc, len);
              delete pBuf;
            }
        }
        fputc(_T('\n'), InfoFile);

        p4 = (char *)Data.InMemoryOrderModuleList.Flink;
    } while(p4 && p4!=pEnd && ReadProcessMemory(hProcess, p4-sizeof(PVOID)*2, &Data, sizeof(Data), 0));
    }
    fputc(_T('\n'), InfoFile);
    if(hProcess)
    CloseHandle(hProcess);
}
