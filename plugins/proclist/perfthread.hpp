#ifndef PERFTHREAD_HPP_319B828C_E001_4BB5_93EE_3A505C9A5ABF
#define PERFTHREAD_HPP_319B828C_E001_4BB5_93EE_3A505C9A5ABF

#pragma once

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
	FILETIME    ftCreation{};
	DWORD       dwGDIObjects{}, dwUSERObjects{};
	int         Bitness{};
};

struct PerfLib
{
	wchar_t szSubKey[1024]{};
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
	DWORD GetProcessPriority(DWORD dwPID);
	int SetProcessPriority(DWORD dwPID, DWORD dwPri);
	int TerminateProcess(DWORD dwPID);
	std::wstring GetProcessOwner(DWORD dwPID, std::wstring* pDomain = {});
	std::wstring GetProcessUserSid(DWORD dwPID);
	int GetProcessSessionId(DWORD dwPID);
	std::wstring GetProcessExecutablePath(DWORD dwPID);
	int AttachDebuggerToProcess(DWORD dwPID) { return ExecMethod(dwPID, L"AttachDebugger"); }
	HRESULT GetLastHResult() const { return hrLast; }

private:
	int ExecMethod(DWORD dwPID, const wchar_t* wsMethod, const wchar_t* wsParamName = {}, DWORD dwParam = 0);

	DebugToken token;
	IWbemServices* pIWbemServices{};
	HRESULT hrLast{};
};

class PerfThread
{
public:
	PerfThread(const wchar_t* hostname = {}, const wchar_t* pUser = {}, const wchar_t* pPasw = {});
	~PerfThread();

	void lock();
	void unlock();

	auto& ProcessData() { return pData; }
	ProcessPerfData* GetProcessData(DWORD dwPid, DWORD dwThreads);
	const PerfLib* GetPerfLib() const { return &pf; }
	void AsyncReread() const { SetEvent(hEvtRefresh.get()); }
	void SyncReread();
	void SmartReread() { if (dwLastRefreshTicks > 1000) AsyncReread(); else SyncReread(); }
	bool IsOK() const { return bOK; }
	const auto& HostName() const { return m_HostName; }
	bool Updated() { const auto Ret = bUpdated; bUpdated = false; return Ret; }
	bool IsWMIConnected() const { return WMI.operator bool(); }
	int GetDefaultBitness() const { return DefaultBitness; }
	const auto& UserName() const { return m_UserName; }
	const auto& Password() const { return m_Password; }
private:
	static DWORD WINAPI ThreadProc(void* Param);
	void ThreadProc();
	void Refresh();
	void RefreshWMIData();

	int DefaultBitness;
	handle hThread;
	handle hEvtBreak, hEvtRefresh, hEvtRefreshDone;
	DWORD dwThreadId{};
	std::vector<ProcessPerfData> pData;

	DWORD dwLastTickCount{};
	bool bOK{};
	HKEY hHKLM{}, hPerf{};
	DWORD dwRefreshMsec{ 500 }, dwLastRefreshTicks{};
	std::wstring m_HostName;
	handle hMutex;
	WMIConnection WMI;
	PerfLib pf;
	bool bUpdated{};
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
