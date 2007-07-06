#include "proclist.hpp"
#include "proclng.hpp"
#include "perfthread.hpp"

#ifdef _WIN64
extern "C"
{
  int _fltused = 0;
}
#endif

// obtained from PSAPI.DLL
/*
struct ModuleData {
    void* p1,*p2;
    char* pNext;
    void* p3;
    DWORD dw1,dw2;
    HMODULE hModule;
    DWORD dwEntryPoint; //28
    DWORD dwSizeOfImage; //32
    DWORD dw3; //36
    char* lpModuleFileName; //40
    char* lpModuleFileName1;
    char* lpModuleBaseName; //48
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

size_t mwcslen( const wchar_t *str, size_t maxsize=0 )
{
    size_t sz=0;
    for(const wchar_t *p = str; !maxsize || sz<maxsize-1; p++,sz++)
    if(*(long*)p==0)
        break;
    return sz+1;
}

HANDLE OpenProcessForced(DWORD dwFlags, DWORD dwProcessId, BOOL bInh)
{
    SetLastError(0);
    HANDLE hProcess = OpenProcess(dwFlags, bInh, dwProcessId);
    if(GetLastError()==ERROR_ACCESS_DENIED && ChangePrivileges(TRUE,FALSE))
      hProcess = OpenProcess(dwFlags, bInh, dwProcessId);
    return hProcess;
}

bool GetPDataNT(ProcessDataNT& DATA, ProcessPerfData& pd)
{
    DATA.Size = sizeof(ProcessDataNT);
    DATA.dwPID = pd.dwProcessId;
    DATA.dwPrBase = pd.dwProcessPriority;
    DATA.dwParentPID = pd.dwCreatingPID;
    DATA.dwElapsedTime = pd.dwElapsedTime;
    char* pFullPath = pd.FullPath;
    if(*(DWORD*)pFullPath==0x5C3F3F5C) // "\??\"
      pFullPath += 4;
    lstrcpyn(DATA.FullPath, pFullPath, sizeof(DATA.FullPath));
    lstrcpyn(DATA.CommandLine, pd.CommandLine, sizeof(DATA.CommandLine));
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
      lstrcpyn(CurItem.FindData.cFileName,pd.ProcessName,sizeof(CurItem.FindData.cFileName));
      if(*pd.Owner) {
        CurItem.Owner = new char[lstrlen(pd.Owner)+1];
        lstrcpy(CurItem.Owner, pd.Owner);
      }
      CharToOem(CurItem.FindData.cFileName, CurItem.FindData.cFileName);

      CurItem.UserData = (DWORD_PTR) new ProcessDataNT;
      memset((void*)CurItem.UserData, 0, sizeof(ProcessDataNT));
      if(!pd.ftCreation.dwHighDateTime && pd.dwElapsedTime)
        *(ULONGLONG*)&pd.ftCreation = *(ULONGLONG*)&ftSystemTime - (ULONGLONG)pd.dwElapsedTime * 10000000;

      CurItem.FindData.ftCreationTime = CurItem.FindData.ftLastWriteTime = CurItem.FindData.ftLastAccessTime = pd.ftCreation;
      ULONGLONG ullSize = pd.qwCounters[IDX_WORKINGSET] + pd.qwCounters[IDX_PAGEFILE];
      CurItem.FindData.nFileSizeLow  = ((DWORD*)&ullSize)[0];
      CurItem.FindData.nFileSizeHigh = ((DWORD*)&ullSize)[1];
      CurItem.PackSize     = ((DWORD*)&pd.qwResults[IDX_PAGEFILE])[0];
      CurItem.PackSizeHigh = ((DWORD*)&pd.qwResults[IDX_PAGEFILE])[1];

      CurItem.PackSize = pd.dwProcessId;
      if(pd.dwProcessId)
        FSF.itoa(pd.dwProcessId, CurItem.FindData.cAlternateFileName, 10);
      CurItem.NumberOfLinks = pd.dwThreads;

      GetPDataNT( *(ProcessDataNT*)CurItem.UserData, pd);
      if(pd.dwProcessId==0 && pd.dwThreads >5)  //_Total
        CurItem.FindData.dwFileAttributes |=
#ifndef _WIN64
                                             FILE_ATTRIBUTE_HIDDEN;
#else
                   FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN;  // Far64 unmark :)
#endif
      if(pd.bProcessIs64bit)
        CurItem.FindData.dwFileAttributes |= FILE_ATTRIBUTE_READONLY;
    }//for
    return TRUE;
}

void GetOpenProcessDataNT(HANDLE hProcess, char* pProcessName, DWORD cbProcessName,
    char* pFullPath, DWORD cbFullPath, char* pCommandLine, DWORD cbCommandLine,
    char** ppEnvStrings, OemString** psCurDir)
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
                WideCharToMultiByte( CP_ACP, 0, szProcessName, -1,
            pProcessName, cbProcessName, NULL, NULL);
            else
                *pProcessName = 0;
        }
        if(pFullPath) {
            SIZE_T sz = sizeof(szProcessName);//min(sizeof(szProcessName), Data.FullDllName.MaximumLength*2);
            if(ReadProcessMemory(hProcess, Data.FullDllName.Buffer, szProcessName, sz,0))
                WideCharToMultiByte( CP_ACP, 0, szProcessName, -1,
                pFullPath, cbFullPath, NULL, NULL);
            else
                *pFullPath = 0;
        }
        if(pCommandLine) {
            UNICODE_STRING pCmd;
            if(ReadProcessMemory(hProcess, &pProcessParams->CommandLine, &pCmd, sizeof(pCmd), 0)) {
                SIZE_T sz = min(cbCommandLine, (ULONG)pCmd.Length + 1);
                Array<WCHAR> sCommandLine(sz);
                *pCommandLine = 0;
                if(ReadProcessMemory(hProcess, pCmd.Buffer, sCommandLine, sz-1,0)) {
                    sCommandLine[sz-1] = 0;
                    WideCharToMultiByte(CP_ACP, 0, sCommandLine, -1,
                                        pCommandLine, cbCommandLine, NULL, NULL);
                }
            }
        }

        if(ppEnvStrings) {
            char *pEnv;
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
                    dwSize = mwcslen(pwEnvStrings) + 1;
                    dwSize = WideCharToMultiByte( CP_ACP, 0, pwEnvStrings, dwSize, 0,0,0,0);
                    if(dwSize) {
                        *ppEnvStrings = new char[dwSize];
                        WideCharToMultiByte( CP_ACP, 0, pwEnvStrings, dwSize, *ppEnvStrings,dwSize,0,0);
                    }
                }
            }
        }
        if(psCurDir) {
            UNICODE_STRING CurDir;
            if(ReadProcessMemory(hProcess, &pProcessParams->CurrentDirectoryPath, &CurDir, sizeof(CurDir), 0)) {
                Array<WCHAR> wsCurDir(CurDir.Length + 1);
                if(ReadProcessMemory(hProcess, CurDir.Buffer, (WCHAR*)wsCurDir, CurDir.Length,0))
                *psCurDir = new OemString((WCHAR*)wsCurDir);
            }
        }
    }
}

BOOL ChangePrivileges(BOOL bAdd, BOOL bAsk)
{
    static BOOL bPrivChanged = FALSE;

    if(bAdd==bPrivChanged)
      return TRUE;

    if(bAdd && bAsk)
    {
      const char *MsgItems[]={ GetMsg(MDeleteTitle),
        GetMsg(MCannotDeleteProc), GetMsg(MRetryWithDebug),
        GetMsg(MDangerous), GetMsg(MYes), GetMsg(MNo)};
      if(Message(FMSG_WARNING,NULL,MsgItems,sizeof(MsgItems)/sizeof(*MsgItems),2)!=0)
        return FALSE;
    }
    static HANDLE hToken=0;
    static CRITICAL_SECTION cs;
    if(!*(DWORD*)&cs)
      InitializeCriticalSection(&cs);
    EnterCriticalSection(&cs);

    BOOL rc = FALSE;
    if (hToken || OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken))
    {
       TOKEN_PRIVILEGES tp;
       tp.PrivilegeCount = 1;
       SetLastError(0);
       LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &tp.Privileges[0].Luid);
       tp.Privileges[0].Attributes = bAdd ? SE_PRIVILEGE_ENABLED : 0;
       SetLastError(0);
       AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), NULL, NULL);
       rc = GetLastError()==0;
       if(rc) bPrivChanged = bAdd;
    }

   LeaveCriticalSection(&cs);
   return rc;
}

BOOL KillProcessNT(DWORD pid,HWND hwnd)
{
  HANDLE hProcess;
  BOOL bPrivilegesAdded = FALSE;

  while(1)
  {
      SetLastError(0);
      hProcess = OpenProcess( PROCESS_TERMINATE, FALSE, pid );
      if(hProcess || bPrivilegesAdded)
      break;
      // If access denied, try to assign debug privileges
      if(GetLastError()!=ERROR_ACCESS_DENIED ||
      !ChangePrivileges(TRUE,TRUE))
      break;
      bPrivilegesAdded = TRUE;
  }

  BOOL bRet = FALSE;

  if (hProcess)
  {
    bRet = TerminateProcess( hProcess, 1 );
    CloseHandle( hProcess );
  }
  if(bPrivilegesAdded)
      ChangePrivileges(FALSE,FALSE);

  return bRet;
}

char* PrintTime(ULONG s, bool bDays=true)
{
    ULONG m = s/60;
    s %= 60;
    ULONG h = m / 60;
    m %= 60;
    static char buf[32];
    if(!bDays || h<24)
      FSF.sprintf(buf, "%02d:%02d:%02d", h, m, s);
    else
      FSF.sprintf(buf, "%d %02d:%02d:%02d", h/24, h%24, m, s);
    return buf;
}

char* PrintTime(ULONGLONG ul100ns, bool bDays)
{
    char* buf = PrintTime((ULONG)(ul100ns/10000000), bDays);
    FSF.sprintf(buf+lstrlen(buf), ".%03d", (ul100ns/1000000)%1000 );
    return buf;
}

char* PrintNTUptime(void*p)
{
    return PrintTime((ULONG)((ProcessDataNT*)p)->dwElapsedTime);
}

void DumpNTCounters(HANDLE InfoFile, PerfThread& Thread, DWORD dwPid, DWORD dwThreads)
{
    char tmp[100];
    fputc('\n',InfoFile);

    Lock l(&Thread);
    ProcessPerfData* pdata = Thread.GetProcessData(dwPid, dwThreads);
    if(!pdata)
      return;
    const PerfLib* pf = Thread.GetPerfLib();

    for(size_t i=0; i<sizeof(Counters)/sizeof(*Counters); i++)
    {
      if(!pf->dwCounterTitles[i]) // counter is absent
          continue;
      char buf[28];
      lstrcpyn(buf,GetMsg(Counters[i].idName),sizeof(buf)-2);
      /*if(Opt.AnsiOutput)
          OemToChar(buf,buf);*/
      lstrcat(buf,":");
      fprintf(InfoFile, "%-24s ", buf);

      switch(pf->CounterTypes[i]) {
          case PERF_COUNTER_RAWCOUNT:
          // Display as is.  No Display Suffix.
              FSF.sprintf(tmp, "%10d\n", *(DWORD*)&pdata->qwResults[i]);
              fprintf(InfoFile, "%s", tmp);
          break;
          case PERF_COUNTER_LARGE_RAWCOUNT: //  same, large int
              FSF.sprintf(tmp, "%10.0f\n", (FLOAT)pdata->qwResults[i]);
              fprintf(InfoFile, "%s", tmp);
          break;
          case PERF_100NSEC_TIMER:
          // 64-bit Timer in 100 nsec units. Display delta divided by
          // delta time.  Display suffix: "%"
              //fprintf(InfoFile, "%10.0f%%\n", (FLOAT)pdata->qwResults[i]);
              FSF.sprintf(tmp, "%s %7.0f%%\n", PrintTime((ULONGLONG)pdata->qwCounters[i]), (FLOAT)pdata->qwResults[i]);
              fprintf(InfoFile, "%s", tmp);
          break;
          case PERF_COUNTER_COUNTER:
          // 32-bit Counter.  Divide delta by delta time.  Display suffix: "/sec"
              fprintf(InfoFile, "%10d  %5d%s\n", *(DWORD*)&pdata->qwCounters[i], *(DWORD*)&pdata->qwResults[i], GetMsg(MperSec));//Opt.AnsiOutput
          break;
          case PERF_COUNTER_BULK_COUNT: //PERF_COUNTER_BULK_COUNT
          // 64-bit Counter.  Divide delta by delta time. Display Suffix: "/sec"
              FSF.sprintf(tmp, "%10.0f  %5.0f%s\n", (FLOAT)pdata->qwCounters[i], (FLOAT)pdata->qwResults[i], GetMsg(MperSec));//Opt.AnsiOutput
              fprintf(InfoFile, "%s", tmp);
          break;
          default:
              fputc('\n',InfoFile);
      }
    }
}

void PrintNTCurDirAndEnv(HANDLE InfoFile, HANDLE hProcess, BOOL bExportEnvironment)
{
    OemString* sCurDir=0;
    char *pEnvStrings = 0;
    GetOpenProcessDataNT(hProcess, 0,0,0,0,0,0, bExportEnvironment ? &pEnvStrings : 0, &sCurDir);

    fputc('\n',InfoFile);

    if(sCurDir)
    fprintf(InfoFile,"%s %s\n\n",Plist::PrintTitle(MCurDir),(char*)*sCurDir);

    if(bExportEnvironment) {
    if(pEnvStrings)
    {
        fprintf(InfoFile, "%s\n\n", GetMsg(MEnvironment));//Opt.AnsiOutput
        for(char* p = pEnvStrings; *p; p+=lstrlen(p)+1)
        {
        //if(!Opt.AnsiOutput)
            CharToOem(p,p);
        fprintf(InfoFile, "%s\n", p);
        }
        delete pEnvStrings;
    }
    }
}

void PrintModuleVersion(HANDLE InfoFile, char* pVersion, char* pDesc, int len)
{
    //Changes pVersion and pDesc contents!

    //if(!Opt.AnsiOutput) {
    CharToOem(pVersion,pVersion);
    CharToOem(pDesc,pDesc);
    //}

    do
        fputc('\t', InfoFile);
    while((len=(len|7)+1) < 56);
    len += fprintf(InfoFile, pVersion?pVersion:"");
    if(pDesc) {
    do
       fputc(' ', InfoFile);
    while(len++ < 72);
    fprintf(InfoFile, pDesc);
    }
}

void PrintModulesNT(HANDLE InfoFile, DWORD dwPID, _Opt& Opt)
{
    ModuleData Data;
    HANDLE hProcess =
    OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ|READ_CONTROL, FALSE, dwPID);
    if(GetLastError()==ERROR_ACCESS_DENIED && ChangePrivileges(TRUE,FALSE))
    hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ|READ_CONTROL , FALSE, dwPID);
    PROCESS_PARAMETERS* pProcessParams;
    char *pEnd;
    if(hProcess && GetInternalProcessData( hProcess, &Data, pProcessParams, pEnd, true)) {
    char *p4;
    do {
        int len = fprintf(InfoFile, "  %p  %6X", Data.BaseAddress, Data.SizeOfImage);
        WCHAR wszModuleName[MAX_PATH];
        SIZE_T sz = sizeof(wszModuleName);//min(sizeof(wszModuleName), Data.BaseDllName.MaximumLength*2);

        if(ReadProcessMemory(hProcess, Data.FullDllName.Buffer, wszModuleName, sz,0)) {
            len += fprintf(InfoFile, " %s", (const char *) OemString(wszModuleName));
        char *pBuf, *pVersion, *pDesc;
        if(Opt.ExportModuleVersion && Plist::GetVersionInfo((char*)wszModuleName, pBuf, pVersion, pDesc)) {
            PrintModuleVersion(InfoFile, pVersion, pDesc, len);
            delete pBuf;
        }
        }
        fputc('\n', InfoFile);

        p4 = (char *)Data.InMemoryOrderModuleList.Flink;
    } while(p4 && p4!=pEnd && ReadProcessMemory(hProcess, p4-sizeof(PVOID)*2, &Data, sizeof(Data), 0));
    }
    fputc('\n', InfoFile);
    if(hProcess)
    CloseHandle(hProcess);
}
