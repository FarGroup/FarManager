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
    wchar_t* pNext;
    void* p3;
    DWORD dw1,dw2;
    HMODULE hModule;
    DWORD dwEntryPoint; //28
    DWORD dwSizeOfImage; //32
    DWORD dw3; //36
    wchar_t* lpModuleFileName; //40
    wchar_t* lpModuleFileName1;
    wchar_t* lpModuleBaseName; //48
    DWORD unknown[5];
};
*/

struct UNICODE_STRING
{
	USHORT  Length; USHORT  MaximumLength; PWSTR  Buffer;
};

#if 0
typedef struct _LIST_ENTRY
{
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

typedef struct _RTL_DRIVE_LETTER_CURDIR
{


	USHORT                  Flags;
	USHORT                  Length;
	ULONG                   TimeStamp;
	UNICODE_STRING          DosPath;



} RTL_DRIVE_LETTER_CURDIR, *PRTL_DRIVE_LETTER_CURDIR;


typedef struct _RTL_USER_PROCESS_PARAMETERS
{


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



typedef struct _LDR_MODULE
{
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


typedef struct _PEB_LDR_DATA
{
	ULONG                   Length;
	BOOLEAN                 Initialized;
	PVOID                   SsHandle;
	LIST_ENTRY              InLoadOrderModuleList;
	LIST_ENTRY              InMemoryOrderModuleList;
	LIST_ENTRY              InInitializationOrderModuleList;
} PEB_LDR_DATA, *PPEB_LDR_DATA;

typedef struct _PEB
{
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

typedef struct _PROCESS_BASIC_INFORMATION
{
	PVOID Reserved1;
	PPEB PebBaseAddress;
	PVOID Reserved2[2];
	ULONG_PTR UniqueProcessId;
	PVOID Reserved3;
} PROCESS_BASIC_INFORMATION;

BOOL GetInternalProcessData(HANDLE hProcess, ModuleData* Data, PROCESS_PARAMETERS* &pProcessParams, char*&pEnd, bool bFirstModule=false)
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

	if (ReadProcessMemory(hProcess, processInfo.PebBaseAddress, &peb, sizeof(peb), 0) &&
	        ReadProcessMemory(hProcess, peb.LoaderData, &pld, sizeof(pld), 0))
	{
		//pEnd = (void *)((void *)peb.LoaderData+((void *)&pld.InMemoryOrderModuleList-(void *)&pld));
		hModule = peb.ImageBaseAddress;
		pProcessParams = peb.ProcessParameters;
		pEnd = (char *)peb.LoaderData+sizeof(pld)-sizeof(LIST_ENTRY)*2;
		p4 = (char *)pld.InMemoryOrderModuleList.Flink;

		while (p4)
		{
			if (p4==pEnd || !ReadProcessMemory(hProcess, p4-sizeof(PVOID)*2, Data, sizeof(*Data), 0))
				return FALSE;

			if (bFirstModule)
				return TRUE;

			if (Data->BaseAddress==hModule) break;

			p4 = (char *)Data->InMemoryOrderModuleList.Flink;
		}
	}

	return TRUE;
}


static size_t mwcslen(const wchar_t *str, size_t maxsize=0)
{
	size_t sz=0;

	for (const wchar_t *p = str; !maxsize || sz<maxsize-1; p++,sz++)
		if (*(long*)p==0)
			break;

	return sz+1;
}

HANDLE OpenProcessForced(DebugToken* token, DWORD dwFlags, DWORD dwProcessId, BOOL bInh)
{
	HANDLE hProcess = OpenProcess(dwFlags, bInh, dwProcessId);

	if (hProcess==NULL && GetLastError()==ERROR_ACCESS_DENIED)
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
	wchar_t* pFullPath = pd.FullPath;
	if (*(DWORD*)pFullPath==0x3F005C && ((DWORD*)pFullPath)[1]==0x5C003F) // "\??\"
		pFullPath += 4;

	lstrcpyn(DATA.FullPath, pFullPath, ARRAYSIZE(DATA.FullPath));
	lstrcpyn(DATA.CommandLine, pd.CommandLine, ARRAYSIZE(DATA.CommandLine));
	return true;
}

BOOL GetList(PluginPanelItem* &pPanelItem,size_t &ItemsNumber,PerfThread& Thread)
{
	DWORD numTasks;
	ProcessPerfData* pData=0;
//    Lock l(&Thread); // it's already locked in Plist::GetFindData
	FILETIME ftSystemTime;
	//Prepare system time to subtract dwElapsedTime
	GetSystemTimeAsFileTime(&ftSystemTime);
	Thread.GetProcessData(pData, numTasks);

	if (numTasks && !Thread.IsOK())
		return FALSE;

	pPanelItem = new PluginPanelItem[numTasks];
	memset(pPanelItem, 0, numTasks*sizeof(*pPanelItem));
	ItemsNumber=numTasks;

	for (DWORD i=0; i<numTasks; i++)
	{
		PluginPanelItem& CurItem = pPanelItem[i];
		ProcessPerfData& pd = pData[i];
		CurItem.Flags|=PPIF_USERDATA;
 		//delete CurItem.FileName;  // ???
 		CurItem.FileName = new wchar_t[lstrlen(pd.ProcessName)+1];
 		lstrcpy((wchar_t*)CurItem.FileName, pd.ProcessName);

		if (*pd.Owner)
		{
			CurItem.Owner = new wchar_t[lstrlen(pd.Owner)+1];
			lstrcpy((wchar_t*)CurItem.Owner, pd.Owner);
		}

		CurItem.UserData = (DWORD_PTR) new ProcessDataNT;
		memset((void*)CurItem.UserData, 0, sizeof(ProcessDataNT));

		if (!pd.ftCreation.dwHighDateTime && pd.dwElapsedTime)
		{
			ULARGE_INTEGER St;
			St.LowPart = ftSystemTime.dwLowDateTime;
			St.HighPart = ftSystemTime.dwHighDateTime;
			ULARGE_INTEGER Cr;
			Cr.QuadPart = St.QuadPart - (UINT64)pd.dwElapsedTime * 10000000;
			pd.ftCreation.dwLowDateTime = Cr.LowPart;
			pd.ftCreation.dwHighDateTime = Cr.HighPart;
		}

		CurItem.CreationTime = CurItem.LastWriteTime = CurItem.LastAccessTime = pd.ftCreation;
		ULONGLONG ullSize = pd.qwCounters[IDX_WORKINGSET] + pd.qwCounters[IDX_PAGEFILE];
		CurItem.FileSize = ullSize;
		CurItem.AllocationSize = pd.qwResults[IDX_PAGEFILE];
//yjh:???      CurItem.AllocationSize = pd.dwProcessId;

		CurItem.AlternateFileName=new wchar_t[16];
		FSF.itoa(pd.dwProcessId, (wchar_t*)CurItem.AlternateFileName, 10);

		CurItem.NumberOfLinks = pd.dwThreads;
		GetPDataNT(*(ProcessDataNT*)CurItem.UserData, pd);

		if (pd.dwProcessId==0 && pd.dwThreads >5) //_Total
			CurItem.FileAttributes |= FILE_ATTRIBUTE_HIDDEN;

		if (pd.bProcessIsWow64)
			CurItem.FileAttributes |= FILE_ATTRIBUTE_READONLY;
	}//for

	return TRUE;
}

void GetOpenProcessData(HANDLE hProcess, wchar_t* pProcessName, DWORD cbProcessName,
                          wchar_t* pFullPath, DWORD cbFullPath, wchar_t* pCommandLine, DWORD cbCommandLine,
                          wchar_t** ppEnvStrings, CURDIR_STR_TYPE** psCurDir)
{
	ModuleData Data={};
	char *pEnd;
	PROCESS_PARAMETERS* pProcessParams = 0;

	if (GetInternalProcessData(hProcess, &Data, pProcessParams, pEnd))
	{
		WCHAR szProcessName[MAX_PATH];
		memset(szProcessName, 0, sizeof(szProcessName));

		if (pProcessName)
		{
			SIZE_T sz = sizeof(szProcessName);//min(sizeof(szProcessName), Data.BaseDllName.MaximumLength*2);

			if (ReadProcessMemory(hProcess, Data.BaseDllName.Buffer, szProcessName, sz,0))
				lstrcpyn(pProcessName, szProcessName, cbProcessName);
			else
				*pProcessName = 0;
		}

		if (pFullPath)
		{
			SIZE_T sz = sizeof(szProcessName);//min(sizeof(szProcessName), Data.FullDllName.MaximumLength*2);

			if (ReadProcessMemory(hProcess, Data.FullDllName.Buffer, szProcessName, sz,0))
				lstrcpyn(pFullPath, szProcessName, cbFullPath);
			else
				*pFullPath = 0;
		}

		if (pCommandLine)
		{
			UNICODE_STRING pCmd;

			if (ReadProcessMemory(hProcess, &pProcessParams->CommandLine, &pCmd, sizeof(pCmd), 0))
			{
				SIZE_T sz = Min(cbCommandLine, (ULONG)pCmd.Length/sizeof(WCHAR) + 1);
				Array<WCHAR> sCommandLine((DWORD)sz);
				*pCommandLine = 0;

				if (ReadProcessMemory(hProcess, pCmd.Buffer, sCommandLine, (sz-1)*sizeof(WCHAR),0))
				{
					sCommandLine[sz-1] = 0;
					lstrcpyn(pCommandLine, sCommandLine, cbCommandLine);
				}
			}
		}

		if (ppEnvStrings)
		{
			wchar_t *pEnv;
			*ppEnvStrings = 0;

			if (ReadProcessMemory(hProcess, &pProcessParams->EnvironmentBlock, &pEnv, sizeof(pEnv), 0))
			{
				WCHAR* pwEnvStrings = 0;
				DWORD dwSize = 0;

				while (1)
				{
					pwEnvStrings = new WCHAR[dwSize+=1024];

					if (!ReadProcessMemory(hProcess, pEnv, pwEnvStrings, dwSize*2,0))
					{
						delete[] pwEnvStrings;
						pwEnvStrings = 0;
						break;
					}

					if (mwcslen(pwEnvStrings, dwSize)<dwSize)
						break;

					delete[] pwEnvStrings;
				}

				if (pwEnvStrings)
				{
					if (pwEnvStrings && *pwEnvStrings)
						*ppEnvStrings = pwEnvStrings;
					else
						delete[] pwEnvStrings;
				}
			}
		}

		if (psCurDir)
		{
			*psCurDir = 0;
			UNICODE_STRING CurDir;

			if (ReadProcessMemory(hProcess, &pProcessParams->CurrentDirectoryPath, &CurDir, sizeof(CurDir), 0))
			{
				wchar_t *wsCurDir = new wchar_t[(CurDir.Length+1)/2 + 1];

				if (ReadProcessMemory(hProcess, CurDir.Buffer, wsCurDir, CurDir.Length,0))
				{
					wsCurDir[(CurDir.Length+1)/2] = 0;
					*psCurDir = wsCurDir;
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

BOOL KillProcess(DWORD pid,HWND hwnd)
{
	DebugToken token;
	HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);

	// If access denied, try to assign debug privileges
	if (hProcess==NULL && GetLastError()==ERROR_ACCESS_DENIED)
	{
		const wchar_t *MsgItems[]=
		{
			GetMsg(MDeleteTitle),
			GetMsg(MCannotDeleteProc),
			GetMsg(MRetryWithDebug),
			GetMsg(MDangerous),
			GetMsg(MYes),
			GetMsg(MNo)
		};

		if (Message(FMSG_WARNING,NULL,MsgItems,ARRAYSIZE(MsgItems),2)==0)
		{
			if (token.Enable())
				hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
		}
	}

	BOOL bRet = FALSE;

	if (hProcess)
	{
		bRet = TerminateProcess(hProcess, 1);
		CloseHandle(hProcess);
	}

	return bRet;
}

wchar_t* PrintTime(ULONG s, bool bDays=true)
{
	ULONG m = s/60;
	s %= 60;
	ULONG h = m / 60;
	m %= 60;
	static wchar_t buf[32];

	if (!bDays || h<24)
		FSF.sprintf(buf, L"%02d:%02d:%02d", h, m, s);
	else
		FSF.sprintf(buf, L"%d %02d:%02d:%02d", h/24, h%24, m, s);

	return buf;
}

wchar_t* PrintTime(ULONGLONG ul100ns, bool bDays)
{
	wchar_t* buf = PrintTime((ULONG)(ul100ns/10000000), bDays);
	FSF.sprintf(buf+lstrlen(buf), L".%03d", (ul100ns/10000)%1000);
	return buf;
}

wchar_t* PrintNTUptime(void*p)
{
	return PrintTime((ULONG)((ProcessDataNT*)p)->dwElapsedTime);
}

void DumpNTCounters(HANDLE InfoFile, PerfThread& Thread, DWORD dwPid, DWORD dwThreads)
{
	fputc(L'\n',InfoFile);
	Lock l(&Thread);
	ProcessPerfData* pdata = Thread.GetProcessData(dwPid, dwThreads);

	if (!pdata)
		return;

	const PerfLib* pf = Thread.GetPerfLib();

	for (size_t i=0; i<ARRAYSIZE(Counters); i++)
	{
		if (!pf->dwCounterTitles[i]) // counter is absent
			continue;

		wchar_t buf[28];
		lstrcpyn(buf,GetMsg(Counters[i].idName),ARRAYSIZE(buf)-2);
		lstrcat(buf,L":");
		fprintf(InfoFile, L"%-24s ", buf);

		switch (pf->CounterTypes[i])
		{
			case PERF_COUNTER_RAWCOUNT:
			{
				// Display as is.  No Display Suffix.
				fprintf(InfoFile, L"%10I64u\n", pdata->qwResults[i]);
			}
			break;
			case PERF_COUNTER_LARGE_RAWCOUNT: //  same, large int
			{
				fprintf(InfoFile, L"%10.0f\n", (FLOAT)pdata->qwResults[i]);
			}
			break;
			case PERF_100NSEC_TIMER:
			{
				// 64-bit Timer in 100 nsec units. Display delta divided by
				// delta time.  Display suffix: "%"
				//fprintf(InfoFile, L"%10.0f%%\n", (FLOAT)pdata->qwResults[i]);
				fprintf(InfoFile, L"%s %7.0f%%\n", PrintTime((ULONGLONG)pdata->qwCounters[i]), (FLOAT)pdata->qwResults[i]);
			}
			break;
			case PERF_COUNTER_COUNTER:
			{
				// 32-bit Counter.  Divide delta by delta time.  Display suffix: "/sec"
				fprintf(InfoFile, L"%10I64u  %5I64u%s\n", pdata->qwCounters[i], pdata->qwResults[i], GetMsg(MperSec));
			}
			break;
			case PERF_COUNTER_BULK_COUNT: //PERF_COUNTER_BULK_COUNT
			{
				// 64-bit Counter.  Divide delta by delta time. Display Suffix: "/sec"
				fprintf(InfoFile, L"%10.0f  %5.0f%s\n", (FLOAT)pdata->qwCounters[i], (FLOAT)pdata->qwResults[i], GetMsg(MperSec));
			}
			break;
			default:
				fputc(L'\n',InfoFile);
		}
	}
}

void PrintNTCurDirAndEnv(HANDLE InfoFile, HANDLE hProcess, BOOL bExportEnvironment)
{
	CURDIR_STR_TYPE* sCurDir=0;
	wchar_t *pEnvStrings = 0;
	GetOpenProcessData(hProcess, 0,0,0,0,0,0, bExportEnvironment ? &pEnvStrings : 0, &sCurDir);
	fputc(L'\n',InfoFile);

	if (sCurDir)
	{
		fprintf(InfoFile,L"%s %s\n\n",Plist::PrintTitle(MCurDir),OUT_CVT(sCurDir));
		delete[] sCurDir;
	}

	if (bExportEnvironment && pEnvStrings)
	{
		fprintf(InfoFile, L"%s\n\n", GetMsg(MEnvironment));

		for (wchar_t* p = pEnvStrings; *p; p+=lstrlen(p)+1)
		{
			fprintf(InfoFile, L"%s\n", p);
		}

		delete[] pEnvStrings;
	}
}

void PrintModuleVersion(HANDLE InfoFile, wchar_t* pVersion, wchar_t* pDesc, int len)
{
	//Changes pVersion and pDesc contents!

	do
	{
		fputc(L'\t', InfoFile);
	}
	while ((len=(len|7)+1) < 56);

	int len2=0;
	fprintf2(len2, InfoFile, L"%s", pVersion?pVersion:L"");
	len += len2;

	if (pDesc)
	{
		do
		{
			fputc(L' ', InfoFile);
		}
		while (len++ < 72);

		fprintf(InfoFile, L"%s", pDesc);
	}
}

void PrintModules(HANDLE InfoFile, DWORD dwPID, _Opt& Opt)
{
	ModuleData Data;
	DebugToken token;
	HANDLE hProcess = OpenProcessForced(&token, PROCESS_QUERY_INFORMATION|PROCESS_VM_READ|READ_CONTROL, dwPID);
	PROCESS_PARAMETERS* pProcessParams;
	char *pEnd;

	if (hProcess && GetInternalProcessData(hProcess, &Data, pProcessParams, pEnd, true))
	{
		char *p4;

		do
		{
			int len = 0;
			fprintf2(len, InfoFile, L"  %p  %6X", Data.BaseAddress, Data.SizeOfImage);
			WCHAR wszModuleName[MAX_PATH];
			SIZE_T sz = sizeof(wszModuleName);//min(sizeof(wszModuleName), Data.BaseDllName.MaximumLength*2);

			if (ReadProcessMemory(hProcess, Data.FullDllName.Buffer, wszModuleName, sz,0))
			{
				int len2=0;
				fprintf2(len2, InfoFile, L" %s", OUT_STRING(wszModuleName));
				len += len2;
				wchar_t   *pVersion, *pDesc;
				LPBYTE  pBuf;

				if (Opt.ExportModuleVersion && Plist::GetVersionInfo((wchar_t*)wszModuleName, pBuf, pVersion, pDesc))
				{
					PrintModuleVersion(InfoFile, pVersion, pDesc, len);
					delete[] pBuf;
				}
			}

			fputc(L'\n', InfoFile);
			p4 = (char *)Data.InMemoryOrderModuleList.Flink;
		}
		while (p4 && p4!=pEnd && ReadProcessMemory(hProcess, p4-sizeof(PVOID)*2, &Data, sizeof(Data), 0));
	}

	fputc(L'\n', InfoFile);

	if (hProcess)
		CloseHandle(hProcess);
}
