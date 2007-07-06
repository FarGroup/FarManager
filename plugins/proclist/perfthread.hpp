#define NCOUNTERS 22
#define MAX_USERNAME_LENGTH 128

template <class T> class Array {
  protected:
    T *data;
    DWORD size;
    void Assign(T* src, DWORD cb) {
        //Clear();
        data = new T[size=cb];
        memcpy(data, src, cb*sizeof(T));
    }
  public:
    Array() { data = 0; size = 0; }
    Array(DWORD cbData) { data = new T[size=cbData]; memset(data,0,cbData*sizeof(T)); }
    ~Array() { delete data; }
    void reserve(DWORD cNew) {
        if(cNew<=size) return;
        T* newdata = new T[cNew];
        memcpy(newdata, data, sizeof(T)*size);
        memset(newdata+size, 0, sizeof(T)*(cNew-size));
        delete data;
        data = newdata;
        size = cNew;
    }
    operator T* () const { return data; }
    DWORD length() const { return size; }
};

extern struct _Counters {
   char* Name; DWORD idName; DWORD idCol;
} Counters[NCOUNTERS];

struct ProcessPerfData {
    DWORD       dwProcessId;
    DWORD       dwProcessPriority;
    DWORD       dwThreads;
    DWORD       dwCreatingPID;
    DWORD       dwElapsedTime;
    LONGLONG    qwCounters[NCOUNTERS];
    LONGLONG    qwResults[NCOUNTERS];

    CHAR        ProcessName[MAX_PATH];
    CHAR        FullPath[MAX_PATH];
    CHAR        Owner[MAX_USERNAME_LENGTH];
    CHAR        CommandLine[MAX_CMDLINE];
    FILETIME    ftCreation;
    DWORD       dwGDIObjects, dwUSERObjects;
    BOOL        bProcessIsWow64;
};

struct PerfLib {
    CHAR  szSubKey[1024];
    DWORD dwProcessIdTitle;
    DWORD dwPriorityTitle;
    DWORD dwThreadTitle;
    DWORD dwCreatingPIDTitle;
    DWORD dwElapsedTitle;
    DWORD dwCounterTitles[sizeof(Counters)/sizeof(*Counters)];
    DWORD CounterTypes[sizeof(Counters)/sizeof(*Counters)];
};

struct IWbemServices;

class WMIConnection {

    IWbemServices *pIWbemServices;
    int ExecMethod(DWORD dwPID, PCWSTR wsMethod, PCWSTR wsParamName=0, DWORD dwParam=0);
    HRESULT hrLast;

  public:

    WMIConnection() { pIWbemServices = NULL; hrLast = 0; }
    ~WMIConnection() { Disconnect(); }
    operator bool () { return pIWbemServices!=0; }
    bool Connect(LPCSTR pMachineName, LPCSTR pUser=0, LPCSTR pPassword=0);
    void Disconnect();
    IWbemServices *GetIWbemServices() { return pIWbemServices; }
    DWORD GetProcessPriority(DWORD dwPID);
    int SetProcessPriority(DWORD dwPID, DWORD dwPri);
    int TerminateProcess(DWORD dwPID);
    void GetProcessOwner(DWORD dwPID, char* pUser, char* pDomain=0);
    void GetProcessUserSid(DWORD dwPID, char* pUserSid);
    int GetProcessSessionId(DWORD dwPID);
    void GetProcessExecutablePath(DWORD dwPID, char* pPath);
    int AttachDebuggerToProcess(DWORD dwPID) { return ExecMethod(dwPID, L"AttachDebugger"); }
    HRESULT GetLastHResult() { return hrLast; }
};

class PerfThread {
    friend class Lock;

    static DWORD WINAPI ThreadProc(LPVOID lpParm);
           DWORD WINAPI ThreadProc();

        HANDLE hThread;
        HANDLE hEvtBreak, hEvtRefresh, hEvtRefreshDone;
        DWORD dwThreadId;
        Array<ProcessPerfData> *pData;

        DWORD dwLastTickCount;
        bool bOK;
        HKEY hHKLM, hPerf;
        DWORD dwRefreshMsec, dwLastRefreshTicks;
        char HostName[64];
        HANDLE hMutex;
        WMIConnection WMI;

        PerfLib pf;
        bool bUpdated;
        bool bConnectAttempted;

        void Refresh();
        void RefreshWMIData();

        Plist& PlistPlugin;

    public:
        PerfThread(Plist& plist, LPCSTR hostname=0, LPCSTR pUser=0, LPCSTR pPasw=0);
        ~PerfThread();
        void GetProcessData(ProcessPerfData* &pd, DWORD &nProc) const
        {
            if(!pData) { nProc=0; pd=0; return; }
            pd = *pData; nProc = pData->length();
        }
        ProcessPerfData* GetProcessData(DWORD dwPid, DWORD dwThreads) const;
        const PerfLib* GetPerfLib() const { return &pf; }
        void AsyncReread() { SetEvent(hEvtRefresh); }
        void SyncReread();
        void SmartReread() { if(dwLastRefreshTicks>1000) AsyncReread(); else SyncReread(); }
        bool IsOK() const { return bOK; }
        LPCSTR GetHostName() const { return HostName; }
        bool Updated() { bool bRet=bUpdated; bUpdated=false; return bRet; }
        bool IsWMIConnected() { return WMI; }
        static void GetProcessOwnerInfo(DWORD dwPid, char* pUser, char* UserSid, char* pDomain, int& nSession);

        char UserName[64];
        char Password[64];
};
class Lock {
    HANDLE h;
  public:
      Lock(PerfThread* pth):h(pth?pth->hMutex:0) { if(h) WaitForSingleObject(h, INFINITE); }
    ~Lock(){ if(h) ReleaseMutex(h); }
};

enum {IDX_PAGEFILE=4, IDX_WORKINGSET=6};
