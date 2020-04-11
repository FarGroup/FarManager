// Based on Zoltan Csizmadia's TaskManagerEx source, zoltan_csizmadia@yahoo.com

#include <algorithm>
#include <memory>
#include <mutex>

#include "Proclist.hpp"
#include "perfthread.hpp" // fot GetProcessData
#include "Proclng.hpp"


typedef struct _SYSTEM_HANDLE_TABLE_ENTRY_INFO
{
	USHORT UniqueProcessId;
	USHORT CreatorBackTraceIndex;
	UCHAR ObjectTypeIndex;
	UCHAR HandleAttributes;
	USHORT HandleValue;
	PVOID Object;
	ULONG GrantedAccess;
} SYSTEM_HANDLE_TABLE_ENTRY_INFO, * PSYSTEM_HANDLE_TABLE_ENTRY_INFO;

typedef struct _SYSTEM_HANDLE_INFORMATION
{
	ULONG NumberOfHandles;
	SYSTEM_HANDLE_TABLE_ENTRY_INFO Handles[1];
} SYSTEM_HANDLE_INFORMATION, * PSYSTEM_HANDLE_INFORMATION;

typedef struct _CLIENT_ID
{
	HANDLE UniqueProcess;
	HANDLE UniqueThread;
} CLIENT_ID, * PCLIENT_ID;

typedef ULONG_PTR KAFFINITY;
typedef LONG  KPRIORITY;

typedef struct _THREAD_BASIC_INFORMATION
{
	LONG  ExitStatus;
	PVOID  TebBaseAddress;
	CLIENT_ID  ClientId;
	KAFFINITY  AffinityMask;
	KPRIORITY  Priority;
	KPRIORITY  BasePriority;
} BASIC_THREAD_INFORMATION, THREAD_BASIC_INFORMATION, * PTHREAD_BASIC_INFORMATION;

typedef struct _PROCESS_BASIC_INFORMATION
{
	PVOID Reserved1;
	PVOID PebBaseAddress;
	PVOID Reserved2[2];
	ULONG_PTR UniqueProcessId;
	PVOID Reserved3;
} PROCESS_BASIC_INFORMATION;

struct UNICODE_STRING
{
	WORD  Length;
	WORD  MaximumLength;
	PWSTR Buffer;
};

typedef enum _OBJECT_INFORMATION_CLASS
{
	ObjectBasicInformation,
	ObjectNameInformation,
	ObjectTypeInformation,
}
OBJECT_INFORMATION_CLASS;

static const wchar_t* GetUserAccountID();
static const wchar_t* pUserAccountID;

static bool GetProcessId(HANDLE Handle, DWORD& Pid)
{
	PROCESS_BASIC_INFORMATION pi{};

	if (pNtQueryInformationProcess(Handle, ProcessBasicInformation, &pi, sizeof(pi), {}) != STATUS_SUCCESS)
		return false;

	Pid = static_cast<DWORD>(pi.UniqueProcessId);
	return true;
}

static bool GetThreadId(HANDLE Handle, DWORD& Tid)
{
	BASIC_THREAD_INFORMATION ti;
	if (pNtQueryInformationThread(Handle, 0, &ti, sizeof(ti), {}) != STATUS_SUCCESS)
		return false;

	Tid = static_cast<DWORD>(reinterpret_cast<uintptr_t>(ti.ClientId.UniqueThread));
	return true;
}

static std::wstring to_string(const UNICODE_STRING& Str)
{
	return std::wstring(Str.Buffer, Str.Length / sizeof(wchar_t));
}

static std::unique_ptr<char[]> query_object(HANDLE Handle, OBJECT_INFORMATION_CLASS const Class)
{
	ULONG Size = 8192;
	for (;;)
	{
		auto Buffer = std::make_unique<char[]>(Size);

		switch (pNtQueryObject(Handle, Class, Buffer.get(), Size, &Size))
		{
		case STATUS_SUCCESS:
			return Buffer;

		case STATUS_INFO_LENGTH_MISMATCH:
			continue;

		default:
			return {};
		}
	}
}

static void PrintFileName(HANDLE Handle, HANDLE file)
{
	if (GetFileType(Handle) == FILE_TYPE_PIPE)
	{
		// Check if it's possible to get the file name info
		struct test
		{
			static DWORD WINAPI GetFileNameThread(PVOID Param)
			{
				DWORD iob[2];
				BYTE info[256];
				const auto FileBasicInformation = 4;
				pNtQueryInformationFile(Param, iob, info, sizeof(info), FileBasicInformation);
				return 0;
			}
		};

		const handle Thread(CreateThread({}, 0, test::GetFileNameThread, Handle, 0, {}));

		// Wait for finishing the thread
		const auto Timeout = WaitForSingleObject(Thread.get(), 100) == WAIT_TIMEOUT;
		if (Timeout)
		{
			TerminateThread(Thread.get(), 0);
			PrintToFile(file, L"<pipe>");
			return;
		}
	}

	const auto Data = query_object(Handle, ObjectNameInformation);
	if (!Data)
		return;

	if (const auto Str = to_string(*reinterpret_cast<const UNICODE_STRING*>(Data.get())); !Str.empty())
		PrintToFile(file, L"%ls", Str.c_str());
}

static std::wstring GetTypeToken(HANDLE Handle)
{
	const auto Data = query_object(Handle, ObjectTypeInformation);
	if (!Data)
		return {};

	return to_string(*reinterpret_cast<const UNICODE_STRING*>(Data.get()));
}

enum
{
	OB_TYPE_UNKNOWN,
	/*OB_TYPE_TYPE,
	OB_TYPE_DIRECTORY,
	OB_TYPE_SYMBOLIC_LINK,
	OB_TYPE_TOKEN,*/
	OB_TYPE_PROCESS,
	OB_TYPE_THREAD,
	/*OB_TYPE_JOB,
	OB_TYPE_DEBUG_OBJECT,
	OB_TYPE_EVENT,
	OB_TYPE_EVENT_PAIR,
	OB_TYPE_MUTANT,
	OB_TYPE_CALLBACK,
	OB_TYPE_SEMAPHORE,
	OB_TYPE_TIMER,
	OB_TYPE_PROFILE,
	OB_TYPE_KEYED_EVENT,
	OB_TYPE_WINDOW_STATION,
	OB_TYPE_DESKTOP,
	OB_TYPE_SECTION,*/
	OB_TYPE_KEY,
	/*OB_TYPE_PORT,
	OB_TYPE_WAITABLE_PORT,
	OB_TYPE_ADAPTER,
	OB_TYPE_CONTROLLER,
	OB_TYPE_DEVICE,
	OB_TYPE_DRIVER,
	OB_TYPE_IOCOMPLETION,*/
	OB_TYPE_FILE,
	/*OB_TYPE_WMI_GUID,*/

	OB_OTHER
};

static wchar_t const* const constStrTypes[]
{
	L"",
	/*L"Type",
	L"Directory",
	L"SymbolicLink",
	L"Token",*/
	L"Process",
	L"Thread",
	/*L"Job",
	L"OB_TYPE_DEBUG_OBJECT",
	L"Event",
	L"EventPair",
	L"Mutant",
	L"Callback",
	L"Semaphore",
	L"Timer",
	L"Profile",
	L"OB_TYPE_KEYED_EVENT",
	L"WindowStation",
	L"Desktop",
	L"Section",*/
	L"Key",
	/*L"Port",
	L"OB_TYPE_WAITABLE_PORT",
	L"Adapter",
	L"Controller",
	L"Device",
	L"Driver",
	L"IoCompletion",*/
	L"File",
	/*L"WmiGuid",*/
};

static_assert(std::size(constStrTypes) == OB_OTHER);

static WORD GetTypeFromTypeToken(const wchar_t* const TypeToken)
{
	const auto It = std::find_if(std::cbegin(constStrTypes), std::cend(constStrTypes), [&](const wchar_t* i)
	{
		return !FSF.LStricmp(i, TypeToken);
	});

	return It == std::cend(constStrTypes)? OB_OTHER : static_cast<WORD>(It - std::cbegin(constStrTypes));
}

static void PrintNameByType(HANDLE handle, WORD type, HANDLE file, PerfThread* pThread)
{

	switch (type)
	{
	case OB_TYPE_UNKNOWN:
		return;

	case OB_TYPE_PROCESS:
		if (DWORD dwId = 0; GetProcessId(handle, dwId))
		{
			const std::scoped_lock l(*pThread);
			const auto pd = pThread->GetProcessData(dwId, 0);
			const auto pName = pd? pd->ProcessName.c_str() : L"<unknown>";
			PrintToFile(file, L"%s (%d)", pName, dwId);
		}
		return;

	case OB_TYPE_THREAD:
		if (DWORD dwId = 0; GetThreadId(handle, dwId))
			PrintToFile(file, L"TID: %d", dwId);
		return;

	case OB_TYPE_FILE:
		PrintFileName(handle, file);
		return;

	default:
		const auto Data = query_object(handle, ObjectNameInformation);
		if (!Data)
			return;

		const auto& Str = *reinterpret_cast<const UNICODE_STRING*>(Data.get());
		if (!Str.Length)
			return;

		auto ws = Str.Buffer;

		const wchar_t
			REGISTRY[] = L"\\REGISTRY\\",
			USER[] = L"USER",
			CLASSES[] = L"MACHINE\\SOFTWARE\\CLASSES",
			MACHINE[] = L"MACHINE",
			CLASSES_[] = L"_Classes";

		if (type == OB_TYPE_KEY && !_memicmp(ws, REGISTRY, sizeof(REGISTRY) - 2))
		{
			wchar_t* ws1 = ws + std::size(REGISTRY) - 1;
			const wchar_t* s0 = {};

			if (!_memicmp(ws1, USER, sizeof(USER) - 2))
			{
				ws1 += std::size(USER) - 1;
				const auto l = std::wcslen(pUserAccountID);

				if (l && !_memicmp(ws1, pUserAccountID, l * 2))
				{
					s0 = L"HKCU";
					ws1 += l;

					if (!_memicmp(ws1, CLASSES_, sizeof(CLASSES_) - 2))
					{
						s0 = L"HKCU\\Classes";
						ws1 += std::size(CLASSES_) - 1;
					}
				}
				else
					s0 = L"HKU";
			}
			else if (!_memicmp(ws1, CLASSES, sizeof(CLASSES) - 2)) { s0 = L"HKCR"; ws1 += std::size(CLASSES) - 1; }
			else if (!_memicmp(ws1, MACHINE, sizeof(MACHINE) - 2)) { s0 = L"HKLM"; ws1 += std::size(MACHINE) - 1; }

			if (s0)
			{
				PrintToFile(file, L"%s", s0);
				ws = ws1;
			}
		}

		PrintToFile(file, L"%ls", ws);
	}
}

static void PrintNameAndType(HANDLE h, DWORD dwPID, HANDLE file, PerfThread* pThread)
{
	HANDLE Handle = h;
	handle DuplicatedHandle, RemoteProcess;
	const auto remote = dwPID != GetCurrentProcessId();

	if (remote)
	{
		DebugToken token;
		RemoteProcess.reset(OpenProcessForced(&token, PROCESS_DUP_HANDLE, dwPID, TRUE));
		if (!RemoteProcess)
			return;

		if (!DuplicateHandle(RemoteProcess.get(), h, GetCurrentProcess(), &Handle, 0, 0, DUPLICATE_SAME_ACCESS))
			return;

		DuplicatedHandle.reset(Handle);
	}

	const auto TypeToken = GetTypeToken(Handle);
	PrintToFile(file, L"%-13s ", TypeToken.c_str());

	const auto type = GetTypeFromTypeToken(TypeToken.c_str());
	PrintNameByType(Handle, type, file, pThread);
}

bool PrintHandleInfo(DWORD dwPID, HANDLE file, bool bIncludeUnnamed, PerfThread* pThread)
{
	bool ret = true;
	DWORD size = 0x2000, needed = 0;
	auto pSysHandleInformation = static_cast<SYSTEM_HANDLE_INFORMATION*>(VirtualAlloc({}, size, MEM_COMMIT, PAGE_READWRITE));
	if (!pSysHandleInformation)
		return false;

	if (pNtQuerySystemInformation(16, pSysHandleInformation, size, &needed))
	{
		if (needed == 0)
		{
			ret = false;
			goto cleanup;
		}

		// The size was not enough
		VirtualFree(pSysHandleInformation, 0, MEM_RELEASE);
		pSysHandleInformation = static_cast<SYSTEM_HANDLE_INFORMATION*>(VirtualAlloc({}, size = needed + 256, MEM_COMMIT, PAGE_READWRITE));
	}

	if (!pSysHandleInformation)
		return false;

	// Query the objects ( system wide )
	if (pNtQuerySystemInformation(16, pSysHandleInformation, size, {}))
	{
		ret = false;
		goto cleanup;
	}

	PrintToFile(file, L"%s\n%s\n", GetMsg(MTitleHandleInfo), GetMsg(MHandleInfoHdr));

	if (!pUserAccountID)
		pUserAccountID = GetUserAccountID(); // init once

	// Iterating through the objects
	for (DWORD i = 0; i < pSysHandleInformation->NumberOfHandles; i++)
	{
		// ProcessId filtering check
		if (pSysHandleInformation->Handles[i].UniqueProcessId == dwPID || dwPID == (DWORD)-1)
		{
			pSysHandleInformation->Handles[i].HandleAttributes = (UCHAR)(pSysHandleInformation->Handles[i].HandleAttributes & 0xff);
			PrintToFile(file, L"%5X  %08X ",
				pSysHandleInformation->Handles[i].HandleValue,
				/*dwType< std::size(constStrTypes) ?
					constStrTypes[dwType] : _L"(Unknown)",*/
					//              pSysHandleInformation->Handles[i].KernelAddress,
				pSysHandleInformation->Handles[i].GrantedAccess);
			PrintNameAndType((HANDLE)(SIZE_T)(UINT)pSysHandleInformation->Handles[i].HandleValue, dwPID, file, pThread);
			PrintToFile(file, L'\n');
		}
	}

	PrintToFile(file, L'\n');
cleanup:

	VirtualFree(pSysHandleInformation, 0, MEM_RELEASE);

	return ret;
}

static BOOL ConvertSid(PSID pSid, LPWSTR pszSidText, LPDWORD dwBufferLen)
{
	// test if SID passed in is valid
	if (!pIsValidSid(pSid))
		return FALSE;

	// obtain SidIdentifierAuthority
	const auto psia = pGetSidIdentifierAuthority(pSid);
	if (!psia)
		return FALSE;

	// obtain sidsubauthority count
	const auto pscnt = pGetSidSubAuthorityCount(pSid);
	if (!pscnt)
		return FALSE;

	const auto dwSubAuthorities = *pscnt;

	// compute buffer length
	// S-SID_REVISION- + identifierauthority- + subauthorities- + {}
	auto dwSidSize = (15 + 12 + (12 * dwSubAuthorities) + 1) * sizeof(wchar_t);

	// check provided buffer length.
	// If not large enough, indicate proper size and setlasterror
	if (*dwBufferLen < dwSidSize)
	{
		*dwBufferLen = static_cast<DWORD>(dwSidSize);
		SetLastError(ERROR_INSUFFICIENT_BUFFER);
		return FALSE;
	}

	// prepare S-SID_REVISION-
	dwSidSize = wsprintfW(pszSidText, L"\\S-%lu-", SID_REVISION);
	// prepare SidIdentifierAuthority
	dwSidSize += psia->Value[0] || psia->Value[1] ?
		wsprintfW(pszSidText + std::wcslen(pszSidText), L"0x%02hx%02hx%02hx%02hx%02hx%02hx",
			(USHORT)psia->Value[0], (USHORT)psia->Value[1],
			(USHORT)psia->Value[2], (USHORT)psia->Value[3],
			(USHORT)psia->Value[4], (USHORT)psia->Value[5]) :
		wsprintfW(pszSidText + std::wcslen(pszSidText), L"%lu",
			(ULONG)(psia->Value[5]) + (ULONG)(psia->Value[4] << 8) +
		(ULONG)(psia->Value[3] << 16) + (ULONG)(psia->Value[2] << 24));

	// loop through SidSubAuthorities
	// obtain sidsubauthority count
	for (DWORD dwCounter = 0; dwCounter < dwSubAuthorities; dwCounter++)
	{
		DWORD rc = 0, * prc = pGetSidSubAuthority(pSid, dwCounter);

		if (prc) rc = *prc;

		dwSidSize += wsprintfW(pszSidText + dwSidSize, L"-%lu", rc);
	}

	return TRUE;
}

static const wchar_t* GetUserAccountID()
{
	static wchar_t UserAccountID[256];
	auto size = static_cast<DWORD>(std::size(UserAccountID));
	if (!GetUserName(UserAccountID, &size))
	{
		return L"";
	}

	SID_NAME_USE eUse;
	DWORD cbSid = 0, cbDomainName = 0;
	if (!pLookupAccountNameW({}, UserAccountID, {}, &cbSid, {}, &cbDomainName, &eUse))
	{
		return L"";
	}

	const auto Sid = make_malloc<void>(cbSid);
	const auto DomainName = std::make_unique<wchar_t[]>(cbDomainName + 1);
	pLookupAccountNameW({}, UserAccountID, Sid.get(), &cbSid, DomainName.get(), &cbDomainName, &eUse);
	size = static_cast<DWORD>(std::size(UserAccountID));

	if (!ConvertSid(Sid.get(), static_cast<wchar_t*>(UserAccountID), &size))
		*UserAccountID = 0;

	return static_cast<const wchar_t*>(UserAccountID);
}
