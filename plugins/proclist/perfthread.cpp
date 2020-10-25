#include <algorithm>
#include <mutex>
#include <cstddef>
#include <cassert>

#include "Proclist.hpp"
#include "Proclng.hpp"
#include "perfthread.hpp"

#define INITIAL_SIZE        51200
#define INITIAL_SIZE        51200
#define EXTEND_SIZE         25600

const counters Counters[]
{
	{L"% Processor Time",        MProcessorTime     , MColProcessorTime     },
	{L"% Privileged Time",       MPrivilegedTime    , MColPrivilegedTime    },
	{L"% User Time",             MUserTime          , MColUserTime          },
	{L"Handle Count",            MHandleCount       , MColHandleCount       },
	{L"Page File Bytes",         MPageFileBytes     , MColPageFileBytes     },
	{L"Page File Bytes Peak",    MPageFileBytesPeak , MColPageFileBytesPeak },
	{L"Working Set",             MWorkingSet        , MColWorkingSet        },
	{L"Working Set Peak",        MWorkingSetPeak    , MColWorkingSetPeak    },
	{L"Pool Nonpaged Bytes",     MPoolNonpagedBytes , MColPoolNonpagedBytes },
	{L"Pool Paged Bytes",        MPoolPagedBytes    , MColPoolPagedBytes    },
	{L"Private Bytes",           MPrivateBytes      , MColPrivateBytes      },
	{L"Page Faults/sec",         MPageFaults        , MColPageFaults        },
	{L"Virtual Bytes",           MVirtualBytes      , MColVirtualBytes      },
	{L"Virtual Bytes Peak",      MVirtualBytesPeak  , MColVirtualBytesPeak  },
	{L"IO Data Bytes/sec",       MIODataBytes       , MColIODataBytes       },
	{L"IO Read Bytes/sec",       MIOReadBytes       , MColIOReadBytes       },
	{L"IO Write Bytes/sec",      MIOWriteBytes      , MColIOWriteBytes      },
	{L"IO Other Bytes/sec",      MIOOtherBytes      , MColIOOtherBytes      },
	{L"IO Data Operations/sec",  MIODataOperations  , MColIODataOperations  },
	{L"IO Read Operations/sec",  MIOReadOperations  , MColIOReadOperations  },
	{L"IO Write Operations/sec", MIOWriteOperations , MColIOWriteOperations },
	{L"IO Other Operations/sec", MIOOtherOperations , MColIOOtherOperations },
};

// A wrapper class to provide auto-closing of registry key
class RegKey
{
	HKEY hKey;
public:
	RegKey(HKEY hParent, const wchar_t* pKey, DWORD flags = KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS)
	{
		if (const auto rc = RegOpenKeyEx(hParent, pKey, 0, flags, &hKey); rc != ERROR_SUCCESS)
		{
			SetLastError(rc);
			hKey = {};
		}
	}
	~RegKey() { if (hKey) RegCloseKey(hKey); }
	operator HKEY() const { return hKey; }
};

// look backwards for the counter number
static int getcounter(wchar_t* p)
{
	wchar_t* p2;

	for (p2 = p - 2; iswdigit(*p2); p2--)
		;

	return FSF.atoi(p2 + 1);
}

static bool Is64BitWindows()
{
#if defined(_WIN64)
	return true;  // 64-bit programs run only on Win64
#else
	// 32-bit programs run on both 32-bit and 64-bit Windows
	// so must sniff
	static const auto IsWow64 = is_wow64_process(GetCurrentProcess());
	return IsWow64;
#endif
}

PerfThread::PerfThread(const wchar_t* hostname, const wchar_t* pUser, const wchar_t* pPasw) :
	DefaultBitness(Is64BitWindows()? 64 : 32)
{
	if (pUser && *pUser)
	{
		m_UserName = pUser;

		if (pPasw)
			m_Password = pPasw;
	}

	hMutex.reset(CreateMutex({}, false, {}));
	const std::scoped_lock l(*this);

	if (hostname)
	{
		m_HostName = hostname;

		if (const auto rc = RegConnectRegistry(hostname, HKEY_PERFORMANCE_TEXT, &hPerf); rc != ERROR_SUCCESS)
		{
			SetLastError(rc);
			return;
		}
	}
	else
	{
		hPerf = HKEY_PERFORMANCE_TEXT;
	}

	// Get the buffer size for the counter names
	DWORD dwType, dwSize;

	if (const auto rc = RegQueryValueEx(hPerf, L"Counter", {}, &dwType, {}, &dwSize); rc != ERROR_SUCCESS)
	{
		SetLastError(rc);
		return;
	}

	// Allocate the counter names buffer
	std::vector<wchar_t> buf(dwSize);

	// read the counter names from the registry
	if (const auto rc = RegQueryValueEx(hPerf, L"Counter", {}, &dwType, reinterpret_cast<BYTE*>(buf.data()), &dwSize); rc != ERROR_SUCCESS)
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

	for (auto p = buf.data(); *p; p += std::wcslen(p) + 1)
	{
		if (FSF.LStricmp(p, L"Process") == 0)
			pf.SubKey = str(getcounter(p));
		else if (!pf.dwProcessIdTitle && FSF.LStricmp(p, L"ID Process") == 0)
			pf.dwProcessIdTitle = getcounter(p);
		else if (!pf.dwPriorityTitle && FSF.LStricmp(p, L"Priority Base") == 0)
			pf.dwPriorityTitle = getcounter(p);
		else if (!pf.dwThreadTitle && FSF.LStricmp(p, L"Thread Count") == 0)
			pf.dwThreadTitle = getcounter(p);
		else if (!pf.dwCreatingPIDTitle && FSF.LStricmp(p, L"Creating Process ID") == 0)
			pf.dwCreatingPIDTitle = getcounter(p);
		else if (!pf.dwElapsedTitle && FSF.LStricmp(p, L"Elapsed Time") == 0)
			pf.dwElapsedTitle = getcounter(p);
		else
			for (int i = 0; i < NCOUNTERS; i++)
				if (!pf.dwCounterTitles[i] && FSF.LStricmp(p, Counters[i].Name) == 0)
					pf.dwCounterTitles[i] = getcounter(p);
	}

	hEvtBreak.reset(CreateEvent({}, TRUE, FALSE, {}));
	hEvtRefresh.reset(CreateEvent({}, 0, 0, {}));
	hEvtRefreshDone.reset(CreateEvent({}, 0, 0, {}));
	Refresh();
	hThread.reset(CreateThread({}, 0, ThreadProc, this, 0, &dwThreadId));
	bOK = true;
}
PerfThread::~PerfThread()
{
	SetEvent(hEvtBreak.get());
	WaitForSingleObject(hThread.get(), INFINITE);

	if (hHKLM)
		RegCloseKey(hHKLM);

	if (hPerf)
		RegCloseKey(hPerf);
}

void PerfThread::lock()
{
	WaitForSingleObject(hMutex.get(), INFINITE);
}

void PerfThread::unlock()
{
	ReleaseMutex(hMutex.get());
}

ProcessPerfData* PerfThread::GetProcessData(DWORD dwPid, DWORD dwThreads)
{
	std::pair<ProcessPerfData*, size_t> ZeroPid[10];
	auto ZeroPidIterator = std::begin(ZeroPid);

	for (auto& i: pData)
	{
		if (i.dwProcessId == dwPid)
		{
			if (dwPid)
				return &i;

			if (ZeroPidIterator == std::end(ZeroPid))
			{
				assert(false);
				continue;
			}

			ZeroPidIterator->first = &i;
			++ZeroPidIterator;
		}
	}

	if (dwPid)
		return {};

	if (ZeroPidIterator == ZeroPid + 1)
		return ZeroPidIterator->first;

	const auto threads_delta = [dwThreads](ProcessPerfData* Data)
	{
		return dwThreads > Data->dwThreads?
			dwThreads - Data->dwThreads :
			Data->dwThreads - dwThreads;
	};

	return std::min_element(std::begin(ZeroPid), ZeroPidIterator, [&](const auto& a, const auto& b)
	{
		return threads_delta(a.first) < threads_delta(b.first);
	})->first;
}

template<typename T>
static const T* view_as(const void* const Address, size_t const Offset)
{
	return static_cast<const T*>(static_cast<const void*>(static_cast<const char*>(Address) + Offset));
}

void PerfThread::Refresh()
{
	DebugToken token;
	const auto dwTicksBeforeRefresh = GetTickCount();
	// allocate the initial buffer for the performance data
	std::vector<BYTE> buf(INITIAL_SIZE);
	const PERF_DATA_BLOCK* pPerf;
	DWORD dwDeltaTickCount;

	for (;;)
	{
		auto dwSize = static_cast<DWORD>(buf.size());
		DWORD dwType;
		dwDeltaTickCount = GetTickCount() - dwLastTickCount;
		DWORD rc;

		while ((rc = RegQueryValueEx(hPerf, pf.SubKey.c_str(), {}, &dwType, buf.data(), &dwSize)) == ERROR_LOCK_FAILED)
			; //Just retry

		pPerf = view_as<PERF_DATA_BLOCK>(buf.data(), 0);

		// check for success and valid perf data block signature
		if (rc == ERROR_SUCCESS && !std::wmemcmp(pPerf->Signature, L"PERF", 4))
		{
			break;
		}

		if (rc == ERROR_MORE_DATA)
			buf.resize(buf.size() + EXTEND_SIZE);
		else if (rc < 0x100000)  // ??? sometimes we receive garbage in rc
		{
			bOK = false;
			return;
		}
	}

	const auto bDeltaValid = dwLastTickCount && dwDeltaTickCount;
	// set the perf_object_type pointer
	const auto pObj = view_as<PERF_OBJECT_TYPE>(pPerf, pPerf->HeaderLength);
	// loop thru the performance counter definition records looking
	// for the process id counter and then save its offset
	const auto pCounterDef = view_as<PERF_COUNTER_DEFINITION>(pObj, pObj->HeaderLength);

	if (!pf.CounterTypes[0] && !pf.CounterTypes[1])
	{
		for (DWORD i = 0; i < (DWORD)pObj->NumCounters; i++)
			for (DWORD ii = 0; ii < NCOUNTERS; ii++)
				if (pf.dwCounterTitles[ii] && pCounterDef[i].CounterNameTitleIndex == pf.dwCounterTitles[ii])
					pf.CounterTypes[ii] = pCounterDef[i].CounterType;
	}

	DWORD dwProcessIdCounter = 0, dwPriorityCounter = 0, dwThreadCounter = 0, dwElapsedCounter = 0,
		dwCreatingPIDCounter = 0, dwCounterOffsets[NCOUNTERS]{};

	for (size_t i = 0; i != pObj->NumCounters; ++i)
	{
		const auto& Def = pCounterDef[i];

		if (Def.CounterNameTitleIndex == pf.dwProcessIdTitle)
			dwProcessIdCounter = Def.CounterOffset;
		else if (Def.CounterNameTitleIndex == pf.dwPriorityTitle)
			dwPriorityCounter = Def.CounterOffset;
		else if (Def.CounterNameTitleIndex == pf.dwThreadTitle)
			dwThreadCounter = Def.CounterOffset;
		else if (Def.CounterNameTitleIndex == pf.dwCreatingPIDTitle)
			dwCreatingPIDCounter = Def.CounterOffset;
		else if (Def.CounterNameTitleIndex == pf.dwElapsedTitle)
			dwElapsedCounter = Def.CounterOffset;
		else
			for (int ii = 0; ii < NCOUNTERS; ii++)
				if (pf.dwCounterTitles[ii] && Def.CounterNameTitleIndex == pf.dwCounterTitles[ii])
					dwCounterOffsets[ii] = Def.CounterOffset;
	}

	std::vector<ProcessPerfData> NewPData(pObj->NumInstances);
	auto pInst = view_as<PERF_INSTANCE_DEFINITION>(pObj, pObj->DefinitionLength);

	// loop thru the performance instance data extracting each process name
	// and process id
	//
	for (size_t i = 0; i != static_cast<size_t>(pObj->NumInstances); ++i)
	{
		auto& Task = NewPData[i];
		// get the process id
		const auto pCounter = view_as<PERF_COUNTER_BLOCK>(pInst, pInst->ByteLength);

		Task.Bitness = DefaultBitness;

		Task.dwProcessId = *view_as<DWORD>(pCounter, dwProcessIdCounter);
		if (dwThreadCounter)
			Task.dwThreads = *view_as<DWORD>(pCounter, dwThreadCounter);

		ProcessPerfData* pOldTask = {};
		if (!pData.empty())  // Use prev data if any
		{
			//Get the pointer to the previous instance of this process
			pOldTask = GetProcessData(Task.dwProcessId, Task.dwThreads);
			if (pOldTask)  // copy process' data from pOldTask to Task
			{
				Task = *pOldTask;
			}
		}

		Task.dwProcessPriority = *view_as<DWORD>(pCounter, dwPriorityCounter);
		if (dwCreatingPIDCounter)
			Task.dwCreatingPID = *view_as<DWORD>(pCounter, dwCreatingPIDCounter);

		if (const auto Value = *view_as<LONGLONG>(pCounter, dwElapsedCounter); Value && pObj->PerfFreq.QuadPart)
			Task.dwElapsedTime = ((pObj->PerfTime.QuadPart - Value) / pObj->PerfFreq.QuadPart);

		// Store new qwCounters
		for (int ii = 0; ii < NCOUNTERS; ii++)
		{
			if (!dwCounterOffsets[ii])
				continue;

			Task.qwCounters[ii] = (pf.CounterTypes[ii] & 0x300) == PERF_SIZE_LARGE?
				*view_as<LONGLONG>(pCounter, dwCounterOffsets[ii]) :
				*view_as<DWORD>(pCounter, dwCounterOffsets[ii]); // PERF_SIZE_DWORD
		}

		//get the rest of the counters
		for (int ii = 0; ii < NCOUNTERS; ii++)
		{
			if (!pf.dwCounterTitles[ii])
				continue;

			// Fill qwResults
			if (!bDeltaValid)
				continue;

			switch (pf.CounterTypes[ii])
			{
			case PERF_COUNTER_RAWCOUNT:
			case PERF_COUNTER_LARGE_RAWCOUNT:
				Task.qwResults[ii] = Task.qwCounters[ii];
				break;

			case PERF_100NSEC_TIMER:
				// 64-bit Timer in 100 nsec units. Display suffix: "%"
				if (pOldTask)
					Task.qwResults[ii] = (*view_as<LONGLONG>(pCounter, dwCounterOffsets[ii]) - pOldTask->qwCounters[ii]) / (dwDeltaTickCount * 100);
				break;

			case PERF_COUNTER_COUNTER:
			case PERF_COUNTER_BULK_COUNT:
				if (pOldTask)
					Task.qwResults[ii] = (Task.qwCounters[ii] - pOldTask->qwCounters[ii]) * 1000 / dwDeltaTickCount;
				break;
			}
		}

		if (!pOldTask)
		{
			if (const auto hProcess = !m_HostName.empty() || Task.dwProcessId <= 8? nullptr :
				handle(OpenProcessForced(&token, PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | READ_CONTROL, Task.dwProcessId)))
			{
				GetOpenProcessData(hProcess.get(), &Task.ProcessName, &Task.FullPath, &Task.CommandLine);
				FILETIME ftExit, ftKernel, ftUser;
				GetProcessTimes(hProcess.get(), &Task.ftCreation, &ftExit, &ftKernel, &ftUser);
				SetLastError(ERROR_SUCCESS);
				Task.dwGDIObjects = pGetGuiResources(hProcess.get(), 0/*GR_GDIOBJECTS*/);
				Task.dwUSERObjects = pGetGuiResources(hProcess.get(), 1/*GR_USEROBJECTS*/);

				if (is_wow64_process(hProcess.get()))
					Task.Bitness = 32;
			}
		}

		if (Task.ProcessName.empty())  // if after all this it's still unfilled...
		{
			// pointer to the process name
			// convert it to ascii
			Task.ProcessName.assign(reinterpret_cast<const wchar_t*>(reinterpret_cast<DWORD_PTR>(pInst) + pInst->NameOffset), pInst->NameLength / sizeof(wchar_t) - 1);

			if (Task.dwProcessId > 8)
				Task.ProcessName += L".exe";
		}

		pInst = view_as<PERF_INSTANCE_DEFINITION>(pCounter, pCounter->ByteLength);
	}

	dwLastTickCount += dwDeltaTickCount;
	{
		const std::scoped_lock l(*this);
		pData = std::move(NewPData);
	}

	bUpdated = true;
	dwLastRefreshTicks = GetTickCount() - dwTicksBeforeRefresh;
	SetEvent(hEvtRefreshDone.get());
}

void PerfThread::RefreshWMIData()
{
	for (auto& i: pData)
	{
		if (WaitForSingleObject(hEvtBreak.get(), 0) == WAIT_OBJECT_0)
			break;

		if (!m_HostName.empty() && !i.FullPathRead)
		{
			i.FullPath = WMI.GetProcessExecutablePath(i.dwProcessId);
			i.FullPathRead = true;
		}

		if (!i.OwnerRead)
		{
			i.Owner = WMI.GetProcessOwner(i.dwProcessId);

			if (const auto SessionId = WMI.GetProcessSessionId(i.dwProcessId); SessionId)
			{
				i.Owner += L':';
				i.Owner += str(SessionId);
			}

			i.OwnerRead = true;
		}

		if (!i.CommandLineRead)
		{
			i.CommandLine = WMI.GetProcessCommandLine(i.dwProcessId);
			i.CommandLineRead = true;
		}
	}
}

void PerfThread::ThreadProc()
{
	const HANDLE handles[]
	{
		hEvtBreak.get(), hEvtRefresh.get()
	};

	const auto CoInited = SUCCEEDED(CoInitialize({}));

	for (;;)
	{
		Refresh();

		if (!bConnectAttempted && Opt.EnableWMI)
		{
			WMI.Connect(
				m_HostName.c_str(),
				m_UserName.empty()? nullptr : m_UserName.c_str(),
				m_UserName.empty()? nullptr : m_Password.c_str()
			);
			bConnectAttempted = true;
		}

		if (WMI)
			RefreshWMIData();

		if (WaitForMultipleObjects(static_cast<DWORD>(std::size(handles)), handles, 0, dwRefreshMsec) == WAIT_OBJECT_0)
			break;
	}

	WMI.Disconnect();

	if (CoInited)
		CoUninitialize();
}

DWORD WINAPI PerfThread::ThreadProc(void* Parm)
{
	static_cast<PerfThread*>(Parm)->ThreadProc();
	return 0;
}

void PerfThread::SyncReread()
{
	ResetEvent(hEvtRefreshDone.get());
	AsyncReread();
	WaitForSingleObject(hEvtRefreshDone.get(), INFINITE);
}
