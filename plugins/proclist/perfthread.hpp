#define NCOUNTERS 22
#define MAX_USERNAME_LENGTH 128

template <class T> class Array
{
	protected:
		T *data;
		DWORD size;
		void Assign(T* src, DWORD cb)
		{
			//Clear();
			data = new T[size=cb];
			memcpy(data, src, cb*sizeof(T));
		}
	public:
		Array() { data = 0; size = 0; }
		Array(DWORD cbData) { data = new T[size=cbData]; memset(data,0,cbData*sizeof(T)); }
		~Array() { delete data; }
		void reserve(DWORD cNew)
		{
			if (cNew<=size) return;

			T* newdata = new T[cNew];
			memcpy(newdata, data, sizeof(T)*size);
			memset(newdata+size, 0, sizeof(T)*(cNew-size));
			delete data;
			data = newdata;
			size = cNew;
		}
		operator T*() const { return data; }
		DWORD length() const { return size; }
};

extern struct _Counters
{
	const wchar_t* Name; DWORD idName; DWORD idCol;
} Counters[NCOUNTERS];

struct ProcessPerfData
{
	DWORD       dwProcessId;
	DWORD       dwProcessPriority;
	DWORD       dwThreads;
	DWORD       dwCreatingPID;
	DWORD       dwElapsedTime;
	LONGLONG    qwCounters[NCOUNTERS];
	LONGLONG    qwResults[NCOUNTERS];

	wchar_t       ProcessName[MAX_PATH];
	wchar_t       FullPath[MAX_PATH];
	wchar_t       Owner[MAX_USERNAME_LENGTH];
	wchar_t       CommandLine[MAX_CMDLINE];
	FILETIME    ftCreation;
	DWORD       dwGDIObjects, dwUSERObjects;
	BOOL        bProcessIsWow64;
};

struct PerfLib
{
	wchar_t szSubKey[1024];
	DWORD dwProcessIdTitle;
	DWORD dwPriorityTitle;
	DWORD dwThreadTitle;
	DWORD dwCreatingPIDTitle;
	DWORD dwElapsedTitle;
	DWORD dwCounterTitles[ARRAYSIZE(Counters)];
	DWORD CounterTypes[ARRAYSIZE(Counters)];
};

struct IWbemServices;

class WMIConnection
{

		DebugToken token;
		IWbemServices *pIWbemServices;
		int ExecMethod(DWORD dwPID, PCWSTR wsMethod, PCWSTR wsParamName=0, DWORD dwParam=0);
		HRESULT hrLast;

	public:

		WMIConnection();
		~WMIConnection();
		operator bool () { return pIWbemServices!=0; }
		bool Connect(LPCTSTR pMachineName, LPCTSTR pUser=0, LPCTSTR pPassword=0);
		void Disconnect();
		IWbemServices *GetIWbemServices() { return pIWbemServices; }
		DWORD GetProcessPriority(DWORD dwPID);
		int SetProcessPriority(DWORD dwPID, DWORD dwPri);
		int TerminateProcess(DWORD dwPID);
		void GetProcessOwner(DWORD dwPID, wchar_t* pUser, wchar_t* pDomain=0);
		void GetProcessUserSid(DWORD dwPID, wchar_t* pUserSid);
		int GetProcessSessionId(DWORD dwPID);
		void GetProcessExecutablePath(DWORD dwPID, wchar_t* pPath);
		int AttachDebuggerToProcess(DWORD dwPID) { return ExecMethod(dwPID, L"AttachDebugger"); }
		HRESULT GetLastHResult() { return hrLast; }
};

class PerfThread
{
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
		wchar_t HostName[64];
		HANDLE hMutex;
		WMIConnection WMI;

		PerfLib pf;
		bool bUpdated;
		bool bConnectAttempted;

		void Refresh();
		void RefreshWMIData();

		Plist& PlistPlugin;

	public:
		PerfThread(Plist& plist, LPCTSTR hostname=0, LPCTSTR pUser=0, LPCTSTR pPasw=0);
		~PerfThread();
		void GetProcessData(ProcessPerfData* &pd, DWORD &nProc) const
		{
			if (!pData) { nProc=0; pd=0; return; }

			pd = *pData; nProc = pData->length();
		}
		ProcessPerfData* GetProcessData(DWORD dwPid, DWORD dwThreads) const;
		const PerfLib* GetPerfLib() const { return &pf; }
		void AsyncReread() { SetEvent(hEvtRefresh); }
		void SyncReread();
		void SmartReread() { if (dwLastRefreshTicks>1000) AsyncReread(); else SyncReread(); }
		bool IsOK() const { return bOK; }
		LPCTSTR GetHostName() const { return HostName; }
		bool Updated() { bool bRet=bUpdated; bUpdated=false; return bRet; }
		bool IsWMIConnected() { return WMI; }
		static void GetProcessOwnerInfo(DWORD dwPid, wchar_t* pUser, wchar_t* UserSid, wchar_t* pDomain, int& nSession);
		wchar_t UserName[64];
		wchar_t Password[64];
};
class Lock
{
		HANDLE h;
	public:
		Lock(PerfThread* pth):h(pth?pth->hMutex:0) { if (h) WaitForSingleObject(h, INFINITE); }
		~Lock() { if (h) ReleaseMutex(h); }
};

enum {IDX_PAGEFILE=4, IDX_WORKINGSET=6};
