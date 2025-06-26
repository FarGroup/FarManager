#include <algorithm>
#include <mutex>
#include <cassert>
#include <cstddef>
#include <cassert>

#include "Proclist.hpp"
#include "Proclng.hpp"
#include "perfthread.hpp"
#include "ipc.hpp"
#include "guid.hpp"

#include <algorithm.hpp>
#include <smart_ptr.hpp>
#include <utility.hpp>

#include <winperf.h>

const counters Counters[]
{
	{ L"% Processor Time",        MProcessorTime,     MColProcessorTime,     counter_type::duration, },
	{ L"% Privileged Time",       MPrivilegedTime,    MColPrivilegedTime,    counter_type::duration, },
	{ L"% User Time",             MUserTime,          MColUserTime,          counter_type::duration, },
	{ L"Handle Count",            MHandleCount,       MColHandleCount,       counter_type::number,   },
	{ L"Page File Bytes",         MPageFileBytes,     MColPageFileBytes,     counter_type::bytes,    },
	{ L"Page File Bytes Peak",    MPageFileBytesPeak, MColPageFileBytesPeak, counter_type::bytes,    },
	{ L"Working Set",             MWorkingSet,        MColWorkingSet,        counter_type::bytes,    },
	{ L"Working Set Peak",        MWorkingSetPeak,    MColWorkingSetPeak,    counter_type::bytes,    },
	{ L"Pool Nonpaged Bytes",     MPoolNonpagedBytes, MColPoolNonpagedBytes, counter_type::bytes,    },
	{ L"Pool Paged Bytes",        MPoolPagedBytes,    MColPoolPagedBytes,    counter_type::bytes,    },
	{ L"Private Bytes",           MPrivateBytes,      MColPrivateBytes,      counter_type::bytes,    },
	{ L"Page Faults/sec",         MPageFaults,        MColPageFaults,        counter_type::number,   },
	{ L"Virtual Bytes",           MVirtualBytes,      MColVirtualBytes,      counter_type::bytes,    },
	{ L"Virtual Bytes Peak",      MVirtualBytesPeak,  MColVirtualBytesPeak,  counter_type::bytes,    },
	{ L"IO Data Bytes/sec",       MIODataBytes,       MColIODataBytes,       counter_type::bytes,    },
	{ L"IO Read Bytes/sec",       MIOReadBytes,       MColIOReadBytes,       counter_type::bytes,    },
	{ L"IO Write Bytes/sec",      MIOWriteBytes,      MColIOWriteBytes,      counter_type::bytes,    },
	{ L"IO Other Bytes/sec",      MIOOtherBytes,      MColIOOtherBytes,      counter_type::bytes,    },
	{ L"IO Data Operations/sec",  MIODataOperations,  MColIODataOperations,  counter_type::number,   },
	{ L"IO Read Operations/sec",  MIOReadOperations,  MColIOReadOperations,  counter_type::number,   },
	{ L"IO Write Operations/sec", MIOWriteOperations, MColIOWriteOperations, counter_type::number,   },
	{ L"IO Other Operations/sec", MIOOtherOperations, MColIOOtherOperations, counter_type::number,   },
	{ L"Working Set - Private",   MWorkingSetPrivate, MColWorkingSetPrivate, counter_type::bytes,    },
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
	return is_wow64_itself();
#endif
}

PerfThread::PerfThread(Plist* const Owner, const wchar_t* hostname, const wchar_t* pUser, const wchar_t* pPasw) :
	m_Owner(Owner),
	DefaultBitness(hostname? -1 : Is64BitWindows()? 64 : 32)
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

	if (Opt.EnableWMI)
	{
		EventWMIReady.reset(CreateEvent({}, TRUE, FALSE, {}));
		EventMTARefresh.reset(CreateEvent({}, 0, 0, {}));
		EventMTARefreshDone.reset(CreateEvent({}, 0, 0, {}));

		hWmiThread.reset(CreateThread({}, 0, WmiThreadProc, this, 0, {}));
		MTAThread.reset(CreateThread({}, 0, MTAThreadProc, this, 0, {}));
	}

	bOK = true;
}
PerfThread::~PerfThread()
{
	SetEvent(hEvtBreak.get());

	if (Opt.EnableWMI)
	{
		WaitForSingleObject(MTAThread.get(), INFINITE);
		WaitForSingleObject(hWmiThread.get(), INFINITE);
	}

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

ProcessPerfData* PerfThread::GetProcessData(DWORD const Pid)
{
	if (const auto Iterator = m_ProcessesData.find(Pid); Iterator != m_ProcessesData.end())
		return &Iterator->second;

	return {};
}

static size_t get_logical_processor_count()
{
	block_ptr<SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX> Buffer(1024);
	DWORD Size{};

	for (;;)
	{
		Size = static_cast<DWORD>(Buffer.size());
		if (pGetLogicalProcessorInformationEx(RelationProcessorCore, Buffer.data(), &Size))
			break;

		if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
		{
			SYSTEM_INFO sysInfo;
			GetSystemInfo(&sysInfo);
			return sysInfo.dwNumberOfProcessors;
		}

		Buffer.reset(Size);
	}

	size_t LogicalProcessorCount{};

	for (size_t Offset{}; Offset != Size;)
	{
		const auto& Info = *static_cast<SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX const*>(static_cast<void const*>(Buffer.bytes().data() + Offset));

		Offset += Info.Size;

		if (Info.Relationship != RelationProcessorCore)
			continue;

		for (const auto& i: std::span{ Info.Processor.GroupMask, Info.Processor.GroupCount })
		{
			LogicalProcessorCount += std::popcount(i.Mask);
		}
	}

	return LogicalProcessorCount;
}

struct PROCLIST_SYSTEM_PROCESS_INFORMATION
{
	ULONG NextEntryOffset;
	ULONG NumberOfThreads;
	LARGE_INTEGER WorkingSetPrivateSize;
	ULONG HardFaultCount;
	ULONG NumberOfThreadsHighWatermark;
	ULONGLONG CycleTime;
	LARGE_INTEGER CreateTime;
	LARGE_INTEGER UserTime;
	LARGE_INTEGER KernelTime;
	UNICODE_STRING ImageName;
	KPRIORITY BasePriority;
	HANDLE UniqueProcessId;
	HANDLE InheritedFromUniqueProcessId;
	ULONG HandleCount;
	ULONG SessionId;
	ULONG_PTR UniqueProcessKey;
	SIZE_T PeakVirtualSize;
	SIZE_T VirtualSize;
	ULONG PageFaultCount;
	SIZE_T PeakWorkingSetSize;
	SIZE_T WorkingSetSize;
	SIZE_T QuotaPeakPagedPoolUsage;
	SIZE_T QuotaPagedPoolUsage;
	SIZE_T QuotaPeakNonPagedPoolUsage;
	SIZE_T QuotaNonPagedPoolUsage;
	SIZE_T PagefileUsage;
	SIZE_T PeakPagefileUsage;
	SIZE_T PrivatePageCount;
	LARGE_INTEGER ReadOperationCount;
	LARGE_INTEGER WriteOperationCount;
	LARGE_INTEGER OtherOperationCount;
	LARGE_INTEGER ReadTransferCount;
	LARGE_INTEGER WriteTransferCount;
	LARGE_INTEGER OtherTransferCount;
	SYSTEM_THREAD_INFORMATION Threads[1];
};

bool PerfThread::RefreshImpl()
{
	block_ptr<PROCLIST_SYSTEM_PROCESS_INFORMATION> Info;
	std::unordered_map<uintptr_t, PROCLIST_SYSTEM_PROCESS_INFORMATION const*> ProcessMap;

	if (m_HostName.empty())
	{
		Info.reset(sizeof(*Info));

		for (;;)
		{
			ULONG ReturnSize{};
			const auto Result = pNtQuerySystemInformation(SystemProcessInformation, Info.data(), static_cast<ULONG>(Info.size()), &ReturnSize);
			if (NT_SUCCESS(Result))
				break;

			if (any_of(Result, STATUS_INFO_LENGTH_MISMATCH, STATUS_BUFFER_OVERFLOW, STATUS_BUFFER_TOO_SMALL))
			{
				Info.reset(ReturnSize? ReturnSize : grow_exp(Info.size(), {}));
				continue;
			}

			Info.reset();
		}

		if (!Info.empty())
		{
			for (size_t Offset = 0;;)
			{
				const auto& ProcessInfo = view_as<PROCLIST_SYSTEM_PROCESS_INFORMATION>(Info.data(), Offset);
				ProcessMap.emplace(std::bit_cast<uintptr_t>(ProcessInfo.UniqueProcessId), &ProcessInfo);

				if (ProcessInfo.NextEntryOffset)
					Offset += ProcessInfo.NextEntryOffset;
				else
					break;
			}
		}
	}

	// TODO: call NtQuerySystemInformation for fallbacks?

	DebugToken token;
	const auto dwTicksBeforeRefresh = GetTickCount();
	std::vector<BYTE> buf(512 * 1024);
	DWORD dwDeltaTickCount{};

	FILETIME SystemTime;

	for (bool Read = false; !Read;)
	{
		auto dwSize = static_cast<DWORD>(buf.size());
		dwDeltaTickCount = GetTickCount() - dwLastTickCount;

		switch (RegQueryValueEx(hPerf, pf.SubKey.c_str(), {}, {}, buf.data(), &dwSize))
		{
		case ERROR_SUCCESS:
			Read = true;

			// To subtract dwElapsedTime. As close to the read as possible for best accuracy.
			GetSystemTimeAsFileTime(&SystemTime);

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

		const auto IsTotal = *pProcessId == 0 && ProcessName == L"_Total"sv;

		// Real process ids can't be odd, so it's fine
		auto& Task = NewPData.emplace(IsTotal? static_cast<DWORD>(-1) : *pProcessId, ProcessPerfData{}).first->second;

		if (IsTotal)
		{
			Task.dwProcessId = static_cast<DWORD>(-1);
		}
		else
		{
			Task.dwProcessId = *pProcessId;
			Task.Bitness = DefaultBitness;
		}

		const auto ProcessInfo = [&]() -> PROCLIST_SYSTEM_PROCESS_INFORMATION const*
		{
			if (IsTotal)
				return nullptr;

			const auto ProcesInfoIterator = ProcessMap.find(Task.dwProcessId);
			return ProcesInfoIterator != ProcessMap.cend()? ProcesInfoIterator->second : nullptr;
		}();

		ProcessPerfData* pOldTask = {};
		if (!m_ProcessesData.empty())  // Use prev data if any
		{
			//Get the pointer to the previous instance of this process
			pOldTask = GetProcessData(Task.dwProcessId);
			if (pOldTask)  // copy process' data from pOldTask to Task
			{
				Task = *pOldTask;
			}
		}

		if (const auto Ptr = dwThreadCounter? view_as_opt<DWORD>(pCounter, DataEnd, dwThreadCounter) : nullptr)
			Task.dwThreads = *Ptr;
		else if (ProcessInfo)
			Task.dwThreads = ProcessInfo->NumberOfThreads;
		else
			return false;


		if (const auto Ptr = dwPriorityCounter? view_as_opt<DWORD>(pCounter, DataEnd, dwPriorityCounter) : nullptr)
			Task.dwProcessPriority = *Ptr;
		else if (ProcessInfo)
			Task.dwProcessPriority = ProcessInfo->BasePriority;
		else
			return false;


		if (const auto Ptr = dwCreatingPIDCounter? view_as_opt<DWORD>(pCounter, DataEnd, dwCreatingPIDCounter) : nullptr)
			Task.dwCreatingPID = *Ptr;
		else if (ProcessInfo)
			Task.dwCreatingPID = static_cast<DWORD>(std::bit_cast<uintptr_t>(ProcessInfo->InheritedFromUniqueProcessId));
		else
			return false;


		if (const auto Ptr = dwElapsedCounter? view_as_opt<LONGLONG>(pCounter, DataEnd, dwElapsedCounter) : nullptr)
		{
			if (*Ptr && pObj->PerfFreq.QuadPart)
			{
				assert(pObj->PerfFreq.QuadPart == 10'000'000); // 100 ns

				Task.dwElapsedTime = pObj->PerfTime.QuadPart - *Ptr;
			}
		}
		else if (ProcessInfo)
			Task.dwElapsedTime = ProcessInfo->CreateTime.QuadPart;
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
					static const auto LogicalProcessorCount = get_logical_processor_count();
					const auto Factor = m_HostName.empty()? LogicalProcessorCount : 1;

					// For _Total this can get negative, e.g. if the process that was in the previsous snapshot is gone.
					if (const auto Ptr = view_as_opt<LONGLONG>(pCounter, DataEnd, dwCounterOffsets[ii]))
						Task.qwResults[ii] = *Ptr > pOldTask->qwCounters[ii]?
							(*Ptr - pOldTask->qwCounters[ii]) / (dwDeltaTickCount * 100) / Factor :
							0;
					else
						return false;
				}
				break;

			case PERF_COUNTER_COUNTER:
			case PERF_COUNTER_BULK_COUNT:
				if (pOldTask)
					// For _Total this can get negative, e.g. if the process that was in the previsous snapshot is gone.
					Task.qwResults[ii] = Task.qwCounters[ii] > pOldTask->qwCounters[ii]?
						(Task.qwCounters[ii] - pOldTask->qwCounters[ii]) * 1000 / dwDeltaTickCount :
						0;
				break;
			}
		}

		if (ProcessInfo)
		{
			Task.CreationTime = ProcessInfo->CreateTime.QuadPart;
			Task.ProcessName = { ProcessInfo->ImageName.Buffer, ProcessInfo->ImageName.Length / sizeof(wchar_t) };
			Task.SessionId = ProcessInfo->SessionId;
		}

		if (!pOldTask && m_HostName.empty() && Task.dwProcessId > 8)
		{
			auto hProcess = handle(OpenProcessForced(&token, PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, Task.dwProcessId));

			// Try limited to at least get the times etc.
			if (!hProcess)
				hProcess = handle(OpenProcessForced(&token, PROCESS_QUERY_LIMITED_INFORMATION, Task.dwProcessId));

			if (hProcess)
			{
				Task.FullPath.emplace();
				Task.CommandLine.emplace();

				get_open_process_data(hProcess.get(), &Task.ProcessName, &*Task.FullPath, &*Task.CommandLine, {}, {});

				if (Task.FullPath->empty())
					Task.FullPath.reset();

				if (Task.CommandLine->empty())
					Task.CommandLine.reset();

				if (!Task.CreationTime)
				{
					FILETIME ftCreation, ftExit, ftKernel, ftUser;
					GetProcessTimes(hProcess.get(), &ftCreation, &ftExit, &ftKernel, &ftUser);
					Task.CreationTime = ULARGE_INTEGER
					{
						.LowPart = ftCreation.dwLowDateTime,
						.HighPart = ftCreation.dwHighDateTime,
					}
					.QuadPart;
				}

				Task.dwGDIObjects = GetGuiResources(hProcess.get(), GR_GDIOBJECTS);
				Task.dwUSERObjects = GetGuiResources(hProcess.get(), GR_USEROBJECTS);

				if (is_wow64_process(hProcess.get()))
					Task.Bitness = 32;
			}
		}

		if (!Task.CreationTime && Task.dwElapsedTime)
		{
			Task.CreationTime = ULARGE_INTEGER
			{
				.LowPart = SystemTime.dwLowDateTime,
				.HighPart = SystemTime.dwHighDateTime,
			}
			.QuadPart - Task.dwElapsedTime;
		}

		if (Task.ProcessName.empty() && !ProcessName.empty())  // if after all this it's still unfilled...
		{
			// pointer to the process name
			Task.ProcessName.assign(ProcessName);

			if (!IsTotal && Task.dwProcessId > 8)
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

void PerfThread::RefreshWMIData(std::optional<DWORD> const Pid)
{
	std::vector<ProcessPerfData> DataCopy;
	DataCopy.reserve(Pid? 1 : m_ProcessesData.size());

	{
		const std::scoped_lock l(*this);
		if (Pid)
		{
			const auto* Data = GetProcessData(*Pid);
			if (!Data)
				return;

			DataCopy = { *Data };
		}
		else
		{
			std::ranges::transform(m_ProcessesData, std::back_inserter(DataCopy), [](const auto& i) { return i.second; });
		}
	}

	for (auto& i: DataCopy)
	{
		if (WaitForSingleObject(hEvtBreak.get(), 0) == WAIT_OBJECT_0)
			break;

		auto AnythingRead = false;

		if (!i.FullPath)
		{
			if (const auto Result = WMI.GetProcessExecutablePath(i.dwProcessId))
			{
				i.FullPath = *Result;
				AnythingRead = true;
			}
		}

		if (!i.Owner)
		{
			const auto [Owner, Domain] = WMI.GetProcessOwner(i.dwProcessId);

			if (Owner)
			{
				i.Owner = *Owner;
				AnythingRead = true;
			}

			if (Domain)
			{
				i.Domain = *Domain;
				AnythingRead = true;
			}
		}

		if (!i.SessionId)
		{
			if (const auto Result = WMI.GetProcessSessionId(i.dwProcessId))
			{
				i.SessionId = *Result;
				AnythingRead = true;
			}
		}

		if (!i.Sid)
		{
			if (const auto Result = WMI.GetProcessUserSid(i.dwProcessId))
			{
				i.Sid = *Result;
				AnythingRead = true;
			}
		}

		if (!i.CommandLine)
		{
			if (const auto Result = WMI.GetProcessCommandLine(i.dwProcessId))
			{
				i.CommandLine = *Result;
				AnythingRead = true;
			}
		}

		if (AnythingRead)
		{
			const std::scoped_lock l(*this);

			if (auto* Data = GetProcessData(i.dwProcessId))
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

	const auto CoInited = SUCCEEDED(CoInitializeEx({}, COINIT_DISABLE_OLE1DDE | COINIT_MULTITHREADED));

	WMIConnectionStatus = WMI.Connect(
		m_HostName.c_str(),
		m_UserName.empty()? nullptr : m_UserName.c_str(),
		m_UserName.empty()? nullptr : m_Password.c_str()
	);

	SetEvent(EventWMIReady.get());

	for (;;)
	{
		if (WMI)
			RefreshWMIData();

		if (WaitForMultipleObjects(static_cast<DWORD>(std::size(handles)), handles, 0, dwRefreshMsec) == WAIT_OBJECT_0)
			break;
	}

	WMI.Disconnect();

	if (CoInited)
		CoUninitialize();
}

void PerfThread::MTAThreadProc() const
{
	const HANDLE RefreshHandles[]
	{
		hEvtBreak.get(), EventMTARefresh.get()
	};

	const HANDLE ReadyHandles[]
	{
		hEvtBreak.get(), EventWMIReady.get()
	};

	const auto CoInited = SUCCEEDED(CoInitializeEx({}, COINIT_DISABLE_OLE1DDE | COINIT_MULTITHREADED));

	for (;;)
	{
		if (WaitForMultipleObjects(static_cast<DWORD>(std::size(RefreshHandles)), RefreshHandles, 0, INFINITE) == WAIT_OBJECT_0)
			break;

		if (WaitForMultipleObjects(static_cast<DWORD>(std::size(ReadyHandles)), ReadyHandles, 0, INFINITE) == WAIT_OBJECT_0)
			break;

		MTACallable(WMI);
		SetEvent(EventMTARefreshDone.get());
	}

	if (CoInited)
		CoUninitialize();
}

void PerfThread::RunMTA(std::function<void(WMIConnection const& WMI)> Callable)
{
	MTACallable = Callable;
	SetEvent(EventMTARefresh.get());
	WaitForSingleObject(EventMTARefreshDone.get(), INFINITE);
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

DWORD WINAPI PerfThread::MTAThreadProc(void* Param)
{
	static_cast<PerfThread*>(Param)->MTAThreadProc();
	return 0;
}

void PerfThread::SyncReread() const
{
	ResetEvent(hEvtRefreshDone.get());
	AsyncReread();
	WaitForSingleObject(hEvtRefreshDone.get(), INFINITE);
}

HRESULT PerfThread::GetWMIStatus() const
{
	const auto Result = WaitForSingleObject(EventWMIReady.get(), INFINITE);
	if (Result != WAIT_OBJECT_0)
		return HRESULT_FROM_WIN32(Result);

	return WMIConnectionStatus;
}
