#ifndef PERFTHREAD_HPP_319B828C_E001_4BB5_93EE_3A505C9A5ABF
#define PERFTHREAD_HPP_319B828C_E001_4BB5_93EE_3A505C9A5ABF

#pragma once

#include <functional>
#include <optional>
#include <string>

#include <expected.hpp>

constexpr inline auto NCOUNTERS = 23;
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

	std::optional<std::wstring> FullPath;
	std::optional<std::wstring> Owner;
	std::optional<std::wstring> Domain;
	std::optional<std::wstring> Sid;
	std::optional<std::wstring> CommandLine;
	std::optional<int> SessionId;

	uint64_t    CreationTime{};
	DWORD       dwGDIObjects{}, dwUSERObjects{};
	int         Bitness{-1};
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

template<typename T>
using wmi_result = expected<T, HRESULT>;

class WMIConnection
{
public:
	WMIConnection();
	~WMIConnection();
	explicit operator bool() const { return pIWbemServices != nullptr; }
	HRESULT Connect(const wchar_t* pMachineName, const wchar_t* pUser = {}, const wchar_t* pPassword = {});
	void Disconnect();
	wmi_result<DWORD> GetProcessPriority(DWORD Pid) const;
	HRESULT SetProcessPriority(DWORD Pid, DWORD Priority) const;
	HRESULT TerminateProcess(DWORD Pid) const;
	std::pair<wmi_result<std::wstring>, wmi_result<std::wstring>> GetProcessOwner(DWORD Pid) const;
	wmi_result<std::wstring> GetProcessUserSid(DWORD Pid) const;
	wmi_result<DWORD> GetProcessSessionId(DWORD Pid) const;
	wmi_result<std::wstring> GetProcessExecutablePath(DWORD Pid) const;
	wmi_result<std::wstring> GetProcessCommandLine(DWORD Pid) const;
	HRESULT AttachDebuggerToProcess(DWORD Pid) const;

private:
	HRESULT ExecMethod(DWORD Pid, const wchar_t* wsMethod, const wchar_t* wsParamName = {}, DWORD dwParam = 0) const;

	HRESULT GetProcessProperty(DWORD Pid, const wchar_t* Name, const std::function<void(const VARIANT&)>& Getter) const;
	wmi_result<DWORD> GetProcessInt(DWORD Pid, const wchar_t* Name) const;
	wmi_result<std::wstring> GetProcessString(DWORD Pid, const wchar_t* Name) const;

	DebugToken token;
	IWbemServices* pIWbemServices{};
};

class PerfThread
{
public:
	PerfThread(Plist* Owner, const wchar_t* hostname = {}, const wchar_t* pUser = {}, const wchar_t* pPasw = {});
	~PerfThread();

	void lock();
	void unlock();

	auto& ProcessData() { return m_ProcessesData; }
	ProcessPerfData* GetProcessData(DWORD Pid);
	const PerfLib* GetPerfLib() const { return &pf; }
	void AsyncReread() const { SetEvent(hEvtRefresh.get()); }
	void SyncReread() const;
	void SmartReread() { if (dwLastRefreshTicks > 1000) AsyncReread(); else SyncReread(); }
	bool IsOK() const { return bOK; }
	const auto& HostName() const { return m_HostName; }
	HRESULT GetWMIStatus() const;
	int GetDefaultBitness() const { return DefaultBitness; }
	const auto& UserName() const { return m_UserName; }
	const auto& Password() const { return m_Password; }
	void RefreshWMI(DWORD Pid) { return RefreshWMIData(Pid); }
	void RunMTA(std::function<void(WMIConnection const& WMI)> Callable);
private:
	static DWORD WINAPI ThreadProc(void* Param);
	static DWORD WINAPI WmiThreadProc(void* Param);
	static DWORD WINAPI MTAThreadProc(void* Param);
	void ThreadProc();
	void WmiThreadProc();
	void MTAThreadProc() const;
	bool RefreshImpl();
	void Refresh();
	void RefreshWMIData(std::optional<DWORD> Pid = {});

	Plist* m_Owner;
	int DefaultBitness;
	handle hThread, hWmiThread, MTAThread;
	handle
		hEvtBreak,
		EventWMIReady,
		hEvtRefresh, hEvtRefreshDone,
		EventMTARefresh, EventMTARefreshDone;

	std::unordered_map<DWORD, ProcessPerfData> m_ProcessesData;

	DWORD dwLastTickCount{};
	bool bOK{};
	HKEY hHKLM{}, hPerf{};
	DWORD dwRefreshMsec{ 1000 }, dwLastRefreshTicks{};
	std::wstring m_HostName;
	handle hMutex;
	WMIConnection WMI;
	HRESULT WMIConnectionStatus{};
	PerfLib pf;
	std::wstring m_UserName;
	std::wstring m_Password;
	std::function<void(WMIConnection const& WMI)> MTACallable;
};

enum
{
	IDX_PAGEFILE = 4,
	IDX_WORKINGSET = 6,
};

#endif // PERFTHREAD_HPP_319B828C_E001_4BB5_93EE_3A505C9A5ABF
