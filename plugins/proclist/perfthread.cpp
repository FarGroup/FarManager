#include <algorithm>
#include <mutex>
#include <cstddef>
#include <cassert>

#include "Proclist.hpp"
#include "Proclng.hpp"
#include "perfthread.hpp"
#include "guid.hpp"

#include <utility.hpp>

#include <winperf.h>

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

PerfThread::PerfThread(Plist* const Owner, const wchar_t* hostname, const wchar_t* pUser, const wchar_t* pPasw) :
	m_Owner(Owner),
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
	hThread.reset(CreateThread({}, 0, ThreadProc, this, 0, {}));
	hWmiThread.reset(CreateThread({}, 0, WmiThreadProc, this, 0, {}));
	bOK = true;
}
PerfThread::~PerfThread()
{
	SetEvent(hEvtBreak.get());
	WaitForSingleObject(hThread.get(), INFINITE);
	WaitForSingleObject(hWmiThread.get(), INFINITE);

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

	if (dwPid)
	{
		if (const auto Iterator = m_ProcessesData.find(dwPid); Iterator != m_ProcessesData.end())
		{
			return &Iterator->second;
		}

		return {};
	}

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

bool PerfThread::RefreshImpl()
{
	DebugToken token;
	const auto dwTicksBeforeRefresh = GetTickCount();
	std::vector<BYTE> buf(512 * 1024);
	DWORD dwDeltaTickCount{};

	for (bool Read = false; !Read;)
	{
		auto dwSize = static_cast<DWORD>(buf.size());
		dwDeltaTickCount = GetTickCount() - dwLastTickCount;

		switch (RegQueryValueEx(hPerf, pf.SubKey.c_str(), {}, {}, buf.data(), &dwSize))
		{
		case ERROR_SUCCESS:
			Read = true;
			break;

		case ERROR_LOCK_FAILED:
			continue;

		case ERROR_MORE_DATA:
			/*
			https://learn.microsoft.com/en-us/windows/win32/api/winreg/nf-winreg-regqueryvalueexw
			If hKey specifies HKEY_PERFORMANCE_DATA and the lpData buffer is not large enough
			to contain all of the returned data, RegQueryValueEx returns ERROR_MORE_DATA
			and the value returned through the lpcbData parameter is undefined.
			This is because the size of the performance data can change from one call to the next.
			In this case, you must increase the buffer size and call RegQueryValueEx again
			passing the updated buffer size in the lpcbData parameter.
			Repeat this until the function succeeds.
			*/
			buf.resize(buf.size() + (buf.size() + 2) / 2);
			continue;

		default:
			return false;
		}
	}

	const auto DataEnd = buf.data() + buf.size();
	const auto pPerf = view_as_opt<PERF_DATA_BLOCK>(buf);
	if (!pPerf)
		return false;

	// check for a valid perf data block signature
	if (std::wmemcmp(pPerf->Signature, L"PERF", std::size(pPerf->Signature)))
		return false;

	const auto bDeltaValid = dwLastTickCount && dwDeltaTickCount;

	// set the perf_object_type pointer
	const auto pObj = view_as_opt<PERF_OBJECT_TYPE>(pPerf, DataEnd, pPerf->HeaderLength);
	if (!pObj)
		return false;

	// loop thru the performance counter definition records looking
	// for the process id counter and then save its offset
	const auto pCounterDef = view_as_opt<PERF_COUNTER_DEFINITION>(pObj, DataEnd, pObj->HeaderLength);
	if (!pCounterDef)
		return false;

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

	decltype(m_ProcessesData) NewPData;

	const auto process_counter = [&](PERF_COUNTER_BLOCK const* pCounter, std::wstring_view const ProcessName)
	{
		// get the process id
		const auto pProcessId = view_as_opt<DWORD>(pCounter, DataEnd, dwProcessIdCounter);
		if (!pProcessId)
			return false;

		auto& Task = NewPData.emplace(*pProcessId, ProcessPerfData{})->second;

		Task.dwProcessId = *pProcessId;
		Task.Bitness = DefaultBitness;

		if (const auto Ptr = view_as_opt<DWORD>(pCounter, DataEnd, dwProcessIdCounter))
			Task.dwProcessId = *Ptr;
		else
			return false;

		if (dwThreadCounter)
		{
			if (const auto Ptr = view_as_opt<DWORD>(pCounter, DataEnd, dwThreadCounter))
				Task.dwThreads = *Ptr;
			else
				return false;
		}

		ProcessPerfData* pOldTask = {};
		if (!m_ProcessesData.empty())  // Use prev data if any
		{
			//Get the pointer to the previous instance of this process
			pOldTask = GetProcessData(Task.dwProcessId, Task.dwThreads);
			if (pOldTask)  // copy process' data from pOldTask to Task
			{
				Task = *pOldTask;
			}
		}

		if (const auto Ptr = view_as_opt<DWORD>(pCounter, DataEnd, dwPriorityCounter))
			Task.dwProcessPriority = *Ptr;
		else
			return false;

		if (dwCreatingPIDCounter)
		{
			if (const auto Ptr = view_as_opt<DWORD>(pCounter, DataEnd, dwCreatingPIDCounter))
				Task.dwCreatingPID = *Ptr;
			else
				return false;
		}

		if (const auto Ptr = view_as_opt<LONGLONG>(pCounter, DataEnd, dwElapsedCounter))
		{
			if (*Ptr && pObj->PerfFreq.QuadPart)
				Task.dwElapsedTime = ((pObj->PerfTime.QuadPart - *Ptr) / pObj->PerfFreq.QuadPart);
		}
		else
			return false;

		// Store new qwCounters
		for (int ii = 0; ii < NCOUNTERS; ii++)
		{
			if (!dwCounterOffsets[ii])
				continue;

			if ((pf.CounterTypes[ii] & 0x300) == PERF_SIZE_LARGE)
			{
				if (const auto Ptr = view_as_opt<LONGLONG>(pCounter, DataEnd, dwCounterOffsets[ii]))
					Task.qwCounters[ii] = *Ptr;
				else
					return false;
			}
			else
			{
				if (const auto Ptr = view_as_opt<DWORD>(pCounter, DataEnd, dwCounterOffsets[ii])) // PERF_SIZE_DWORD
					Task.qwCounters[ii] = *Ptr;
				else
					return false;
			}
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
				{
					if (const auto Ptr = view_as_opt<LONGLONG>(pCounter, DataEnd, dwCounterOffsets[ii]))
						Task.qwResults[ii] = (*Ptr - pOldTask->qwCounters[ii]) / (dwDeltaTickCount * 100);
					else
						return false;
				}
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

		if (Task.ProcessName.empty() && !ProcessName.empty())  // if after all this it's still unfilled...
		{
			// pointer to the process name
			Task.ProcessName.assign(ProcessName);

			if (Task.dwProcessId > 8)
				Task.ProcessName += L".exe";
		}

		return true;
	};

	if (pObj->NumInstances >= 0)
	{
		// If set to a value from 0 to 0x7fffffff, indicates that this is data collected from an object that supports 0 or more named instances.
		// The PERF_COUNTER_DEFINITION block should be followed by the specified number of PERF_INSTANCE_DEFINITION blocks.

		NewPData.reserve(pObj->NumInstances);

		auto pInst = view_as_opt<PERF_INSTANCE_DEFINITION>(pObj, DataEnd, pObj->DefinitionLength);
		if (!pInst)
			return false;

		for (size_t i = 0; i != static_cast<size_t>(pObj->NumInstances); ++i)
		{
			if (!pInst)
				return false;

			const auto pCounter = view_as_opt<PERF_COUNTER_BLOCK>(pInst, DataEnd, pInst->ByteLength);
			if (!pCounter)
				return false;

			const auto NamePtr = view_as_opt<wchar_t>(pInst, DataEnd, pInst->NameOffset);
			if (!NamePtr)
				return false;

			const auto NameSize = pInst->NameLength / sizeof(wchar_t) - 1;
			if (static_cast<void const*>(NamePtr + NameSize) > DataEnd)
				return false;

			if (!process_counter(pCounter, { NamePtr, NameSize }))
				return false;

			pInst = view_as_opt<PERF_INSTANCE_DEFINITION>(pCounter, DataEnd, pCounter->ByteLength);
		}
	}
	else if (pObj->NumInstances == PERF_NO_INSTANCES)
	{
		// If set to the value PERF_NO_INSTANCES, indicates that this is data collected from an object that always has exactly one unnamed instance.
		// The PERF_COUNTER_DEFINITION block should be followed by exactly one PERF_COUNTER_BLOCK block.

		// Supposedly it should not happen, but just in case: there were reports about NumInstances set to some unholy value.
		const auto pCounter = view_as_opt<PERF_COUNTER_BLOCK>(pObj, DataEnd, pObj->DefinitionLength);
		if (!pCounter)
			return false;

		if (!process_counter(pCounter, {}))
			return false;
	}
	else
	{
		// If set to the value PERF_METADATA_MULTIPLE_INSTANCES, indicates that this is metadata for an object that supports 0 or more named instances.
		// The result contains no PERF_INSTANCE_DEFINITION blocks.

		// If set to the value PERF_METADATA_NO_INSTANCES, indicates that this is metadata for an object that always has exactly one unnamed instance.
		// The result contains no PERF_COUNTER_BLOCK.

		return false;
	}

	dwLastTickCount += dwDeltaTickCount;
	{
		const std::scoped_lock l(*this);
		m_ProcessesData = std::move(NewPData);
	}

	dwLastRefreshTicks = GetTickCount() - dwTicksBeforeRefresh;

	return true;
}

void PerfThread::Refresh()
{
	bOK = RefreshImpl();
	SetEvent(hEvtRefreshDone.get());
}

void PerfThread::RefreshWMIData()
{
	std::vector<ProcessPerfData> DataCopy;
	DataCopy.reserve(m_ProcessesData.size());

	{
		const std::scoped_lock l(*this);
		std::transform(m_ProcessesData.cbegin(), m_ProcessesData.cend(), std::back_inserter(DataCopy), [](const auto& i) { return i.second; });
	}

	for (auto& i: DataCopy)
	{
		if (WaitForSingleObject(hEvtBreak.get(), 0) == WAIT_OBJECT_0)
			break;

		auto AnythingRead = false;

		if (!m_HostName.empty() && !i.FullPathRead)
		{
			i.FullPath = WMI.GetProcessExecutablePath(i.dwProcessId);
			i.FullPathRead = true;
			AnythingRead = true;
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
			AnythingRead = true;
		}

		if (!i.CommandLineRead)
		{
			i.CommandLine = WMI.GetProcessCommandLine(i.dwProcessId);
			i.CommandLineRead = true;
			AnythingRead = true;
		}

		if (AnythingRead)
		{
			const std::scoped_lock l(*this);

			if (auto* Data = GetProcessData(i.dwProcessId, i.dwThreads))
				*Data = i;
		}
	}
}

void PerfThread::ThreadProc()
{
	const HANDLE handles[]
	{
		hEvtBreak.get(), hEvtRefresh.get()
	};

	for (;;)
	{
		Refresh();

		if (WaitForMultipleObjects(static_cast<DWORD>(std::size(handles)), handles, 0, dwRefreshMsec) == WAIT_OBJECT_0)
			break;

		PsInfo.AdvControl(&MainGuid, ACTL_SYNCHRO, 0, m_Owner);
	}
}

void PerfThread::WmiThreadProc()
{
	const HANDLE handles[]
	{
		hEvtBreak.get(), hEvtRefresh.get()
	};

	const auto CoInited = SUCCEEDED(CoInitialize({}));

	for (;;)
	{
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

		PsInfo.AdvControl(&MainGuid, ACTL_SYNCHRO, 0, m_Owner);
	}

	WMI.Disconnect();

	if (CoInited)
		CoUninitialize();
}

DWORD WINAPI PerfThread::ThreadProc(void* Param)
{
	static_cast<PerfThread*>(Param)->ThreadProc();
	return 0;
}

DWORD WINAPI PerfThread::WmiThreadProc(void* Param)
{
	static_cast<PerfThread*>(Param)->WmiThreadProc();
	return 0;
}

void PerfThread::SyncReread()
{
	ResetEvent(hEvtRefreshDone.get());
	AsyncReread();
	WaitForSingleObject(hEvtRefreshDone.get(), INFINITE);
}
