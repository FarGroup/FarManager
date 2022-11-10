#ifndef PERFTHREAD_HPP_319B828C_E001_4BB5_93EE_3A505C9A5ABF
#define PERFTHREAD_HPP_319B828C_E001_4BB5_93EE_3A505C9A5ABF

#pragma once

#include <functional>
#include <string>

constexpr inline auto NCOUNTERS = 22;
constexpr inline auto MAX_USERNAME_LENGTH = 128;

extern const struct counters
{
	const wchar_t* Name;
	DWORD idName;
	DWORD idCol;
}
Counters[NCOUNTERS];

struct ProcessPerfData
{
	DWORD       dwProcessId{};
	DWORD       dwProcessPriority{};
	DWORD       dwThreads{};
	DWORD       dwCreatingPID{};
	uint64_t    dwElapsedTime{};
	LONGLONG    qwCounters[NCOUNTERS]{};
	LONGLONG    qwResults[NCOUNTERS]{};

	std::wstring ProcessName;
	std::wstring FullPath;
	std::wstring Owner;
	std::wstring CommandLine;
	bool FullPathRead{};
	bool OwnerRead{};
	bool CommandLineRead{};
	FILETIME    ftCreation{};
	DWORD       dwGDIObjects{}, dwUSERObjects{};
	int         Bitness{};
};

struct PerfLib
{
	std::wstring SubKey;
	DWORD dwProcessIdTitle{};
	DWORD dwPriorityTitle{};
	DWORD dwThreadTitle{};
	DWORD dwCreatingPIDTitle{};
	DWORD dwElapsedTitle{};
	DWORD dwCounterTitles[std::size(Counters)]{};
	DWORD CounterTypes[std::size(Counters)]{};
};

struct IWbemServices;

class WMIConnection
{
public:
	WMIConnection();
	~WMIConnection();
	explicit operator bool() const { return pIWbemServices != nullptr; }
	bool Connect(const wchar_t* pMachineName, const wchar_t* pUser = {}, const wchar_t* pPassword = {});
	void Disconnect();
	DWORD GetProcessPriority(DWORD Pid);
	int SetProcessPriority(DWORD dwPID, DWORD dwPri);
	int TerminateProcess(DWORD dwPID);
	std::wstring GetProcessOwner(DWORD dwPID, std::wstring* pDomain = {});
	std::wstring GetProcessUserSid(DWORD dwPID);
	DWORD GetProcessSessionId(DWORD Pid);
	std::wstring GetProcessExecutablePath(DWORD Pid);
	std::wstring GetProcessCommandLine(DWORD Pid);
	int AttachDebuggerToProcess(DWORD dwPID) { return ExecMethod(dwPID, L"AttachDebugger"); }
	HRESULT GetLastHResult() const { return hrLast; }

private:
	int ExecMethod(DWORD dwPID, const wchar_t* wsMethod, const wchar_t* wsParamName = {}, DWORD dwParam = 0);

	bool GetProcessProperty(DWORD Pid, const wchar_t* Name, const std::function<void(const VARIANT&)>& Getter);
	DWORD GetProcessInt(DWORD Pid, const wchar_t* Name);
	std::wstring GetProcessString(DWORD Pid, const wchar_t* Name);

	DebugToken token;
	IWbemServices* pIWbemServices{};
	HRESULT hrLast{};
};

class PerfThread
{
public:
	PerfThread(Plist* Owner, const wchar_t* hostname = {}, const wchar_t* pUser = {}, const wchar_t* pPasw = {});
	~PerfThread();

	void lock();
	void unlock();

	auto& ProcessData() { return m_ProcessesData; }
	ProcessPerfData* GetProcessData(DWORD dwPid, DWORD dwThreads);
	const PerfLib* GetPerfLib() const { return &pf; }
	void AsyncReread() const { SetEvent(hEvtRefresh.get()); }
	void SyncReread();
	void SmartReread() { if (dwLastRefreshTicks > 1000) AsyncReread(); else SyncReread(); }
	bool IsOK() const { return bOK; }
	const auto& HostName() const { return m_HostName; }
	bool IsWMIConnected() const { return WMI.operator bool(); }
	int GetDefaultBitness() const { return DefaultBitness; }
	const auto& UserName() const { return m_UserName; }
	const auto& Password() const { return m_Password; }
private:
	static DWORD WINAPI ThreadProc(void* Param);
	static DWORD WINAPI WmiThreadProc(void* Param);
	void ThreadProc();
	void WmiThreadProc();
	bool RefreshImpl();
	void Refresh();
	void RefreshWMIData();

	Plist* m_Owner;
	int DefaultBitness;
	handle hThread, hWmiThread;
	handle hEvtBreak, hEvtRefresh, hEvtRefreshDone;

	std::unordered_multimap<DWORD, ProcessPerfData> m_ProcessesData;

	DWORD dwLastTickCount{};
	bool bOK{};
	HKEY hHKLM{}, hPerf{};
	DWORD dwRefreshMsec{ 1000 }, dwLastRefreshTicks{};
	std::wstring m_HostName;
	handle hMutex;
	WMIConnection WMI;
	PerfLib pf;
	bool bConnectAttempted{};
	std::wstring m_UserName;
	std::wstring m_Password;
};

enum
{
	IDX_PAGEFILE = 4,
	IDX_WORKINGSET = 6,
};

#endif // PERFTHREAD_HPP_319B828C_E001_4BB5_93EE_3A505C9A5ABF
