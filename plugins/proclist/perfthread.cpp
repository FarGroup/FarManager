#include "Proclist.hpp"
#include "Proclng.hpp"
#include "perfthread.hpp"
#include <stddef.h>
#include <stdlib.h>

#define INITIAL_SIZE        51200
#define INITIAL_SIZE        51200
#define EXTEND_SIZE         25600

_Counters Counters[] =
{
	{_T("% Processor Time"),        MProcessorTime     , MColProcessorTime     },
	{_T("% Privileged Time"),       MPrivilegedTime    , MColPrivilegedTime    },
	{_T("% User Time"),             MUserTime          , MColUserTime          },
	{_T("Handle Count"),            MHandleCount       , MColHandleCount       },
	{_T("Page File Bytes"),         MPageFileBytes     , MColPageFileBytes     },
	{_T("Page File Bytes Peak"),    MPageFileBytesPeak , MColPageFileBytesPeak },
	{_T("Working Set"),             MWorkingSet        , MColWorkingSet        },
	{_T("Working Set Peak"),        MWorkingSetPeak    , MColWorkingSetPeak    },
	{_T("Pool Nonpaged Bytes"),     MPoolNonpagedBytes , MColPoolNonpagedBytes },
	{_T("Pool Paged Bytes"),        MPoolPagedBytes    , MColPoolPagedBytes    },
	{_T("Private Bytes"),           MPrivateBytes      , MColPrivateBytes      },
	{_T("Page Faults/sec"),         MPageFaults        , MColPageFaults        },
	{_T("Virtual Bytes"),           MVirtualBytes      , MColVirtualBytes      },
	{_T("Virtual Bytes Peak"),      MVirtualBytesPeak  , MColVirtualBytesPeak  },
	{_T("IO Data Bytes/sec"),       MIODataBytes       , MColIODataBytes       },
	{_T("IO Read Bytes/sec"),       MIOReadBytes       , MColIOReadBytes       },
	{_T("IO Write Bytes/sec"),      MIOWriteBytes      , MColIOWriteBytes      },
	{_T("IO Other Bytes/sec"),      MIOOtherBytes      , MColIOOtherBytes      },
	{_T("IO Data Operations/sec"),  MIODataOperations  , MColIODataOperations  },
	{_T("IO Read Operations/sec"),  MIOReadOperations  , MColIOReadOperations  },
	{_T("IO Write Operations/sec"), MIOWriteOperations , MColIOWriteOperations },
	{_T("IO Other Operations/sec"), MIOOtherOperations , MColIOOtherOperations },
};

// A wrapper class to provide auto-closing of registry key
class RegKey
{
		HKEY hKey;
	public:
		RegKey(HKEY hParent, LPCTSTR pKey, DWORD flags=KEY_QUERY_VALUE|KEY_ENUMERATE_SUB_KEYS)
		{
			DWORD rc;

			if ((rc=RegOpenKeyEx(hParent, pKey, 0, flags, &hKey))!=ERROR_SUCCESS)
			{
				SetLastError(rc);
				hKey = 0;
			}
		}
		~RegKey() { if (hKey) RegCloseKey(hKey); }
		operator HKEY() { return hKey; }
};

// look backwards for the counter number
static int getcounter(TCHAR*p)
{
	TCHAR* p2;

	for (p2=p-2; _istdigit(*p2); p2--) ;

	return FSF.atoi(p2+1);
}

PerfThread::PerfThread(Plist& plist, LPCTSTR hostname, LPCTSTR pUser, LPCTSTR pPasw) : PlistPlugin(plist)
{
	memset(this, 0, sizeof(*this));
	dwRefreshMsec = 500;

	if (pUser && *pUser)
	{
		lstrcpyn(UserName, pUser, ARRAYSIZE(UserName));

		if (pPasw)
			lstrcpyn(Password, pPasw, ARRAYSIZE(Password));
	}

	hMutex = CreateMutex(0, FALSE, 0);
	Lock l(this);
	DWORD rc;

	if (hostname)
	{
		lstrcpyn(HostName, hostname, ARRAYSIZE(HostName));

		if ((rc=RegConnectRegistry(hostname, HKEY_LOCAL_MACHINE, &hHKLM))!=ERROR_SUCCESS ||
		        (rc=RegConnectRegistry(hostname, HKEY_PERFORMANCE_DATA, &hPerf))!=ERROR_SUCCESS)
		{
			SetLastError(rc);
			return;
		}
	}
	else
	{
		hHKLM = HKEY_LOCAL_MACHINE;
		hPerf = HKEY_PERFORMANCE_DATA;
	}

	LANGID lid = MAKELANGID(LANG_ENGLISH, SUBLANG_NEUTRAL);
	FSF.sprintf(pf.szSubKey, _T("Software\\Microsoft\\Windows NT\\CurrentVersion\\Perflib\\%03X"), lid);
	RegKey hKeyNames(hHKLM, pf.szSubKey, KEY_READ);

	if (!hKeyNames)
		return;

	// Get the buffer size for the counter names
	DWORD dwType, dwSize;

	if ((rc=RegQueryValueEx(hKeyNames, _T("Counters"), 0, &dwType, NULL, &dwSize))
	        != ERROR_SUCCESS)
	{
		SetLastError(rc);
		return;
	}

	// Allocate the counter names buffer
	TCHAR* buf = new TCHAR [dwSize];

	// read the counter names from the registry
	if ((rc=RegQueryValueEx(hKeyNames, _T("Counters"), 0, &dwType,
	                        (BYTE*)buf, &dwSize)))
	{
		SetLastError(rc);
		return;
	}

	// now loop thru the counter names looking for the following counters:
	//      1.  "Process"           process name
	//      2.  "ID Process"        process id
	// the buffer contains multiple null terminated strings and then
	// finally null terminated at the end.  the strings are in pairs of
	// counter number and counter name.

	for (TCHAR* p = buf; *p; p += lstrlen(p)+1)
	{
		if (FSF.LStricmp(p, _T("Process"))==0)
			FSF.itoa(getcounter(p), pf.szSubKey, 10);
		else if (!pf.dwProcessIdTitle && FSF.LStricmp(p, _T("ID Process"))==0)
			pf.dwProcessIdTitle = getcounter(p);
		else if (!pf.dwPriorityTitle && FSF.LStricmp(p, _T("Priority Base"))== 0)
			pf.dwPriorityTitle = getcounter(p);
		else if (!pf.dwThreadTitle && FSF.LStricmp(p, _T("Thread Count")) == 0)
			pf.dwThreadTitle = getcounter(p);
		else if (!pf.dwCreatingPIDTitle && FSF.LStricmp(p, _T("Creating Process ID")) == 0)
			pf.dwCreatingPIDTitle = getcounter(p);
		else if (!pf.dwElapsedTitle && FSF.LStricmp(p, _T("Elapsed Time")) == 0)
			pf.dwElapsedTitle = getcounter(p);
		else
			for (int i=0; i<NCOUNTERS; i++)
				if (!pf.dwCounterTitles[i] && FSF.LStricmp(p,Counters[i].Name)==0)
					pf.dwCounterTitles[i] = getcounter(p);
	}

	delete buf;
	hEvtBreak = CreateEvent(0, 0, 0, 0);
	hEvtRefresh = CreateEvent(0, 0, 0, 0);
	hEvtRefreshDone = CreateEvent(0, 0, 0, 0);
	Refresh();
	hThread = CreateThread(0, 0, ThreadProc, this, 0, &dwThreadId);
	bOK = true;
}
PerfThread::~PerfThread()
{
	SetEvent(hEvtBreak);
	WaitForSingleObject(hThread, INFINITE);
	delete pData;
	CloseHandle(hEvtRefreshDone);
	CloseHandle(hEvtRefresh);
	CloseHandle(hEvtBreak);
	CloseHandle(hMutex);
	CloseHandle(hThread);

	if (hHKLM) RegCloseKey(hHKLM);

	if (hPerf) RegCloseKey(hPerf);
}

ProcessPerfData* PerfThread::GetProcessData(DWORD dwPid, DWORD dwThreads) const
{
	for (DWORD i=0; i< pData->length(); i++)
		if ((*pData)[i].dwProcessId==dwPid &&
		        (dwPid ||((dwThreads >5 && (*pData)[i].dwThreads >5) ||
		                  (dwThreads<=5 && (*pData)[i].dwThreads<=5)))
		   )
			return &(*pData)[i];

	return 0;
}

void PerfThread::Refresh()
{
	DebugToken token;
	DWORD dwTicksBeforeRefresh = GetTickCount();
	// allocate the initial buffer for the performance data
	Array<BYTE> buf(INITIAL_SIZE);
	PPERF_DATA_BLOCK pPerf;
	DWORD dwDeltaTickCount;

	while (1)
	{
		DWORD dwSize = buf.length(), dwType;
		dwDeltaTickCount = GetTickCount() - dwLastTickCount;
		DWORD rc;

		while ((rc = RegQueryValueEx(hPerf, pf.szSubKey, 0, &dwType, buf, &dwSize))
		        ==ERROR_LOCK_FAILED)  ; //Just retry

		pPerf = (PPERF_DATA_BLOCK)(BYTE*) buf;

		// check for success and valid perf data block signature
		if (rc==ERROR_SUCCESS &&
		        !memcmp(pPerf->Signature, L"PERF", 8))
		{
			break;
		}

		if (rc==ERROR_MORE_DATA)
			buf.reserve(buf.length()+EXTEND_SIZE);
		else if (rc<0x100000)  // ??? sometimes we receive garbage in rc
		{
			bOK = false;
			return;
		}
	}

	bool bDeltaValid = dwLastTickCount && dwDeltaTickCount;
	// set the perf_object_type pointer
	PPERF_OBJECT_TYPE pObj =
	    (PPERF_OBJECT_TYPE)((DWORD_PTR)pPerf + pPerf->HeaderLength);
	// loop thru the performance counter definition records looking
	// for the process id counter and then save its offset
	PPERF_COUNTER_DEFINITION pCounterDef =
	    (PPERF_COUNTER_DEFINITION)((DWORD_PTR)pObj + pObj->HeaderLength);

	if (!pf.CounterTypes[0] && !pf.CounterTypes[1])
	{
		for (DWORD i=0; i<(DWORD)pObj->NumCounters; i++)
			for (DWORD ii=0; ii<NCOUNTERS; ii++)
				if (pf.dwCounterTitles[ii] && pCounterDef[i].CounterNameTitleIndex==pf.dwCounterTitles[ii])
					pf.CounterTypes[ii] = pCounterDef[i].CounterType;
	}

	DWORD dwProcessIdCounter=0, dwPriorityCounter=0, dwThreadCounter=0, dwElapsedCounter=0,
	                         dwCreatingPIDCounter=0, dwCounterOffsets[NCOUNTERS];
	memset(dwCounterOffsets,0,sizeof(dwCounterOffsets));
	DWORD i;

	for (i=0; i<(DWORD)pObj->NumCounters; i++)
	{
		if (pCounterDef->CounterNameTitleIndex == pf.dwProcessIdTitle)
			dwProcessIdCounter = pCounterDef->CounterOffset;
		else if (pCounterDef->CounterNameTitleIndex == pf.dwPriorityTitle)
			dwPriorityCounter = pCounterDef->CounterOffset;
		else if (pCounterDef->CounterNameTitleIndex == pf.dwThreadTitle)
			dwThreadCounter = pCounterDef->CounterOffset;
		else if (pCounterDef->CounterNameTitleIndex == pf.dwCreatingPIDTitle)
			dwCreatingPIDCounter = pCounterDef->CounterOffset;
		else if (pCounterDef->CounterNameTitleIndex == pf.dwElapsedTitle)
			dwElapsedCounter = pCounterDef->CounterOffset;
		else
			for (int ii=0; ii<NCOUNTERS; ii++)
				if (pf.dwCounterTitles[ii] && pCounterDef->CounterNameTitleIndex==pf.dwCounterTitles[ii])
					dwCounterOffsets[ii] = pCounterDef->CounterOffset;

		pCounterDef++;
	}

	Array<ProcessPerfData> *pNewPData = new Array<ProcessPerfData>((DWORD)pObj->NumInstances);
	PPERF_INSTANCE_DEFINITION pInst =
	    (PPERF_INSTANCE_DEFINITION)((DWORD_PTR)pObj + pObj->DefinitionLength);

	// loop thru the performance instance data extracting each process name
	// and process id
	//
	for (i=0; i<(DWORD)pObj->NumInstances; i++)
	{
		ProcessPerfData& Task = (*pNewPData)[i];
		// get the process id
		PPERF_COUNTER_BLOCK pCounter = (PPERF_COUNTER_BLOCK)((DWORD_PTR)pInst + pInst->ByteLength);
		Task.bProcessIsWow64 = FALSE;
		Task.dwProcessId = *((LPDWORD)((DWORD_PTR)pCounter + dwProcessIdCounter));
		Task.dwProcessPriority = *((LPDWORD)((DWORD_PTR)pCounter + dwPriorityCounter));
		Task.dwThreads = dwThreadCounter ? *((LPDWORD)((DWORD_PTR)pCounter + dwThreadCounter)) : 0;
		Task.dwCreatingPID = dwCreatingPIDCounter ? *((LPDWORD)((DWORD_PTR)pCounter + dwCreatingPIDCounter)) : 0;

		if (pObj->PerfFreq.QuadPart && *((LONGLONG*)((DWORD_PTR)pCounter + dwElapsedCounter)))
			Task.dwElapsedTime = (DWORD)((pObj->PerfTime.QuadPart - *((LONGLONG*)((DWORD_PTR)pCounter + dwElapsedCounter))
			                             ) / pObj->PerfFreq.QuadPart);
		else
			Task.dwElapsedTime = 0;

// Store new qwCounters
		for (int ii=0; ii<NCOUNTERS; ii++)
			if (dwCounterOffsets[ii])
			{
				if ((pf.CounterTypes[ii]&0x300)==PERF_SIZE_LARGE)
					Task.qwCounters[ii] = *((LONGLONG*)((DWORD_PTR)pCounter + dwCounterOffsets[ii]));
				else // PERF_SIZE_DWORD
					Task.qwCounters[ii] = *((DWORD*)((DWORD_PTR)pCounter + dwCounterOffsets[ii]));
			}

		//memcpy(Task.qwResults, Task.qwCounters, sizeof(Task.qwResults));
		memset(Task.qwResults, 0, sizeof(Task.qwResults));
		ProcessPerfData* pOldTask = 0;

		if (pData)  // Use prev data if any
		{
			//Get the pointer to the previous instance of this process
			pOldTask = GetProcessData(Task.dwProcessId, Task.dwThreads);
			//get the rest of the counters

			for (int ii=0; ii<NCOUNTERS; ii++)
			{
				if (!pf.dwCounterTitles[ii])
					continue;

// Fill qwResults
				if (bDeltaValid)
					switch (pf.CounterTypes[ii])
					{
						case PERF_COUNTER_RAWCOUNT:
						case PERF_COUNTER_LARGE_RAWCOUNT:
							Task.qwResults[ii] = Task.qwCounters[ii];
							break;
						case PERF_100NSEC_TIMER:
							// 64-bit Timer in 100 nsec units. Display suffix: "%"
							Task.qwResults[ii] = !pOldTask ? 0 :
							                     (*(LONGLONG*)((DWORD_PTR)pCounter + dwCounterOffsets[ii]) - pOldTask->qwCounters[ii])
							                     / (dwDeltaTickCount*100);
							break;
						case PERF_COUNTER_COUNTER:
						case PERF_COUNTER_BULK_COUNT:
							Task.qwResults[ii] = !pOldTask ? 0 :
							                     (Task.qwCounters[ii] - pOldTask->qwCounters[ii])
							                     * 1000 / dwDeltaTickCount;
							break;
					}
			}
		}//if pData

		if (pOldTask)  // copy process' data from pOldTask to Task
		{
			/*
			    lstrcpy(Task.ProcessName, pOldTask->ProcessName);
			    lstrcpy(Task.FullPath, pOldTask->FullPath);
			    lstrcpy(Task.CommandLine, pOldTask->CommandLine);
			    Task.ftCreation = pOldTask->ftCreation;
			    */
			memcpy(Task.ProcessName, pOldTask->ProcessName, sizeof(Task) - offsetof(ProcessPerfData, ProcessName));
		}
		else
		{
			HANDLE hProcess = *HostName || Task.dwProcessId<=8 ? 0 :
			                  OpenProcessForced(&token, PROCESS_QUERY_INFORMATION|PROCESS_VM_READ|READ_CONTROL, Task.dwProcessId);

			if (hProcess)
			{
				GetOpenProcessDataNT(hProcess, Task.ProcessName, ARRAYSIZE(Task.ProcessName),
				                     Task.FullPath, ARRAYSIZE(Task.FullPath), Task.CommandLine, ARRAYSIZE(Task.CommandLine));
				FILETIME ftExit,ftKernel,ftUser;
				GetProcessTimes(hProcess,&Task.ftCreation,&ftExit,&ftKernel,&ftUser);
				SetLastError(0);
				Task.dwGDIObjects = pGetGuiResources(hProcess, 0/*GR_GDIOBJECTS*/);
				Task.dwUSERObjects = pGetGuiResources(hProcess, 1/*GR_USEROBJECTS*/);
				BOOL wow64;

				if (pIsWow64Process(hProcess, &wow64) && wow64)
					Task.bProcessIsWow64 = TRUE;

				CloseHandle(hProcess);
			}
		}

		if (!*Task.ProcessName)  // if after all this it's still unfilled...
		{
			// pointer to the process name
			// convert it to ascii
#ifndef UNICODE

			if (WideCharToMultiByte(CP_OEMCP, 0, (LPCWSTR)((DWORD_PTR)pInst + pInst->NameOffset),
			                        -1, Task.ProcessName, sizeof(Task.ProcessName) - 5, 0, 0) == 0)
				lstrcpy(Task.ProcessName, _T("unknown"));
			else
#else
			lstrcpyn(Task.ProcessName, (LPCWSTR)((DWORD_PTR)pInst + pInst->NameOffset),
			         ARRAYSIZE(Task.ProcessName) - 5);

#endif

				if (Task.dwProcessId>8)
					lstrcat(Task.ProcessName, _T(".exe"));
		}

		pInst = (PPERF_INSTANCE_DEFINITION)((DWORD_PTR)pCounter + pCounter->ByteLength);
	}

	dwLastTickCount += dwDeltaTickCount;
	{
		Lock l(this);
		delete pData;
		pData = pNewPData;
	}
//    if(!PlistPlugin.PostUpdate())
	bUpdated = true;
	dwLastRefreshTicks = GetTickCount() - dwTicksBeforeRefresh;
	SetEvent(hEvtRefreshDone);
}

#define LASTBYTE(_arr) ((_arr)[sizeof(_arr)-1])

void PerfThread::RefreshWMIData()
{
	for (unsigned i=0; i<pData->length(); i++)
	{
		if (*HostName && LASTBYTE((*pData)[i].FullPath)!='*')
		{
			WMI.GetProcessExecutablePath((*pData)[i].dwProcessId, (*pData)[i].FullPath);
			LASTBYTE((*pData)[i].FullPath) = '*';
		}

		if (LASTBYTE((*pData)[i].Owner)!='*')
		{
			WMI.GetProcessOwner((*pData)[i].dwProcessId, (*pData)[i].Owner);
			int iSessionId = WMI.GetProcessSessionId((*pData)[i].dwProcessId);

			if (iSessionId > 0)
			{
				TCHAR* p = (*pData)[i].Owner + lstrlen((*pData)[i].Owner);
				*p++ = _T(':');
				FSF.itoa(iSessionId, p, 10);
			}

			LASTBYTE((*pData)[i].Owner) = '*';
		}
	}
}

DWORD WINAPI PerfThread::ThreadProc()
{
	HANDLE handles[2] = {hEvtBreak, hEvtRefresh};

	CoInitialize(NULL);
	while (1)
	{
		Refresh();

		if (!bConnectAttempted && Opt.EnableWMI)
		{
			WMI.Connect(HostName, *UserName?UserName:0, *UserName?Password:0);
			bConnectAttempted = true;
		}

		if (WMI)
			RefreshWMIData();

		if (WaitForMultipleObjects(ARRAYSIZE(handles), handles, 0, dwRefreshMsec)==WAIT_OBJECT_0)
			break;
	}

	WMI.Disconnect();
	CoUninitialize();
	return 1;
}

DWORD WINAPI PerfThread::ThreadProc(LPVOID lpParm)
{
	return ((PerfThread*)lpParm)->ThreadProc();
}

void PerfThread::SyncReread()
{
	ResetEvent(hEvtRefreshDone);
	AsyncReread();
	WaitForSingleObject(hEvtRefreshDone, INFINITE);
}
