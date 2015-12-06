// Based on Zoltan Csizmadia's TaskManagerEx source, zoltan_csizmadia@yahoo.com

#include <stdio.h>
#include "Proclist.hpp"
#include "perfthread.hpp" // fot GetProcessData
#include "Proclng.hpp"

/*
struct SYSTEM_HANDLE {
        DWORD   ProcessID;
        WORD    HandleType;
        WORD    HandleNumber;
        DWORD   KernelAddress;
        DWORD   Flags;
};
struct SYSTEM_HANDLE_INFORMATION
{
        DWORD           Count;
        SYSTEM_HANDLE   Handles[1];
};
*/

typedef struct _SYSTEM_HANDLE_TABLE_ENTRY_INFO
{
	USHORT UniqueProcessId;
	USHORT CreatorBackTraceIndex;
	UCHAR ObjectTypeIndex;
	UCHAR HandleAttributes;
	USHORT HandleValue;
	PVOID Object;
	ULONG GrantedAccess;
} SYSTEM_HANDLE_TABLE_ENTRY_INFO, *PSYSTEM_HANDLE_TABLE_ENTRY_INFO;

typedef struct _SYSTEM_HANDLE_INFORMATION
{
	ULONG NumberOfHandles;
	SYSTEM_HANDLE_TABLE_ENTRY_INFO Handles[1];
} SYSTEM_HANDLE_INFORMATION, *PSYSTEM_HANDLE_INFORMATION;


/*
struct BASIC_THREAD_INFORMATION {
        DWORD u1;
        DWORD u2;
        DWORD u3;
        DWORD ThreadId;
        DWORD u5;
        DWORD u6;
        DWORD u7;
};
*/

typedef struct _CLIENT_ID
{
	HANDLE UniqueProcess;
	HANDLE UniqueThread;
} CLIENT_ID, *PCLIENT_ID;

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
} BASIC_THREAD_INFORMATION, THREAD_BASIC_INFORMATION, *PTHREAD_BASIC_INFORMATION;

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

enum { OB_TYPE_UNKNOWN = 0, OB_TYPE_TYPE = 1, OB_TYPE_DIRECTORY,
       OB_TYPE_SYMBOLIC_LINK,  OB_TYPE_TOKEN,  OB_TYPE_PROCESS,
       OB_TYPE_THREAD, /*OB_TYPE_UNKNOWN_7,*/ OB_TYPE_EVENT,
       OB_TYPE_EVENT_PAIR, OB_TYPE_MUTANT, //OB_TYPE_UNKNOWN_11,
       OB_TYPE_SEMAPHORE, OB_TYPE_TIMER, OB_TYPE_PROFILE,
       OB_TYPE_WINDOW_STATION, OB_TYPE_DESKTOP, OB_TYPE_SECTION,
       OB_TYPE_KEY, OB_TYPE_PORT, OB_TYPE_WAITABLE_PORT,
       /*OB_TYPE_UNKNOWN_21, OB_TYPE_UNKNOWN_22, OB_TYPE_UNKNOWN_23,
       OB_TYPE_UNKNOWN_24,*/ OB_TYPE_JOB, //22 on Whistler
       //OB_TYPE_CONTROLLER, OB_TYPE_DEVICE, OB_TYPE_DRIVER,
       OB_TYPE_IO_COMPLETION, OB_TYPE_FILE, OB_TYPE_WMI_GUID,
     };

static const wchar_t* GetUserAccountID();
static const wchar_t* pUserAccountID;

static BOOL GetProcessId(HANDLE handle, DWORD& dwPID)
{
	BOOL ret = FALSE;
	//bool remote = false;
	PROCESS_BASIC_INFORMATION pi;
	ZeroMemory(&pi, sizeof(pi));
	dwPID = 0;

	// Get the process information
	if (pNtQueryInformationProcess(handle, ProcessBasicInformation, &pi, sizeof(pi), NULL) == 0)
	{
		dwPID = (DWORD)pi.UniqueProcessId;
		ret = TRUE;
	}

	return ret;
}
static BOOL GetThreadId(HANDLE h, DWORD& threadID)
{
	BOOL ret = FALSE;
	BASIC_THREAD_INFORMATION ti;
	HANDLE handle, hRemoteProcess = NULL;
	bool remote = false;
	handle = h;

	// Get the thread information
	if (pNtQueryInformationThread(handle, 0, &ti, sizeof(ti), NULL) == 0)
	{
		threadID = (DWORD)(SIZE_T)ti.ClientId.UniqueThread;
		ret = TRUE;
	}

	if (remote)
	{
		if (hRemoteProcess) CloseHandle(hRemoteProcess);

		if (handle) CloseHandle(handle);
	}

	return ret;
}

inline bool GOODSTATUS(LONG st) { return !(st) || (st)==STATUS_INFO_LENGTH_MISMATCH; }

static DWORD WINAPI GetFileNameThread(PVOID Param)
{
	DWORD iob[2];
	BYTE info[256];

	if (pNtQueryInformationFile((HANDLE)Param, &iob, &info, sizeof(info), 22)
	        == STATUS_NOT_IMPLEMENTED) Sleep(200);

	return 0;
}

static bool PrintFileName(HANDLE handle, HANDLE file)
{
	bool ret = false;
	// Check if it's possible to get the file name info
	DWORD dwThreadId;
	HANDLE hThread = CreateThread(0, 0, GetFileNameThread, handle, 0, &dwThreadId);

	// Wait for finishing the thread
	if (WaitForSingleObject(hThread, 100) == WAIT_TIMEOUT)
	{
		// Access denied, terminate the thread
		TerminateThread(hThread, 0);
		ret = true;
	}
	else
	{
		// it is safe to call NtQueryObject
		ULONG size = 0x2000;

		if (GOODSTATUS(pNtQueryObject(handle, 1, NULL, 0, &size)))
		{
			// let's try to use the default
			if (size == 0)
				size = 0x2000;

			UCHAR* lpBuffer = new UCHAR[size];

			if (pNtQueryObject(handle, 1, lpBuffer, size, 0) == 0 &&
			        ((UNICODE_STRING*)lpBuffer)->Length)
			{
				fprintf(file, L"%ls", ((UNICODE_STRING*)lpBuffer)->Buffer);
			}
			else
			{
				ret = true;
			}

			delete[] lpBuffer;
		}
	}

	CloseHandle(hThread);
	return ret;
}

static bool GetTypeToken(HANDLE handle, wchar_t* str, DWORD dwSize)
{
	ULONG size = 0x2000;
	bool ret = false;

	// Query the info size
	if (GOODSTATUS(pNtQueryObject(handle, 2, NULL, 0, &size)))
	{
		UCHAR* lpBuffer = new UCHAR[size];

		// Query the info size ( type )
		if (pNtQueryObject(handle, 2, lpBuffer, size, NULL) == 0)
		{
			lstrcpynW(str,(LPCWSTR)(lpBuffer+0x60) , dwSize);
			ret = true;
		}

		delete[] lpBuffer;
	}

	return ret;
}

static wchar_t const * const constStrTypes[] =
{
	L"",             L"",           L"Directory",    L"SymbolicLink",
	L"Token",        L"Process",    L"Thread",       /*L"Unk7",*/
	L"Event",        L"EventPair",  L"Mutant",       /*L"Unk11", */
	L"Semaphore",    L"Timer",      L"Profile",      L"WindowStation",
	L"Desktop",      L"Section",    L"Key",          L"Port",
	L"WaitablePort", /*L"Unk21",*/  /*L"Unk22",*/    /*L"Unk23",*/
	/*L"Unk24",*/     L"Job",        L"IoCompletion", L"File",
	/*L"Unk27",*/     L"WmiGuid",
};

static WORD GetTypeFromTypeToken(LPCTSTR typeToken)
{
	for (WORD i = 1; i < ARRAYSIZE(constStrTypes); i++)
		if (!FSF.LStricmp(constStrTypes[i], typeToken))
			return i;

	return OB_TYPE_UNKNOWN;
}

static WORD GetType(HANDLE h)
{
	wchar_t strType[256];
	return GetTypeToken(h, strType, ARRAYSIZE(strType)) ? GetTypeFromTypeToken(strType) : OB_TYPE_UNKNOWN;
}

static bool PrintNameByType(HANDLE handle, WORD type, HANDLE file, PerfThread* pThread=0)
{
	bool ret = false;
	// let's be happy, handle is in our process space, so query the infos :)
	DWORD dwId = 0;

	switch (type)
	{
		case OB_TYPE_PROCESS:

			if (GetProcessId(handle, dwId))
			{
				Lock l(pThread);
				ProcessPerfData* pd = pThread ? pThread->GetProcessData(dwId, 0) : 0;
				const wchar_t* pName = pd ? pd->ProcessName : L"<unknown>";
				fprintf(file, L"%s (%d)", pName, dwId);
			}

			return true;
		case OB_TYPE_THREAD:

			if (GetThreadId(handle, dwId))
				fprintf(file, L"TID: %d", dwId);

			return true;
		case OB_TYPE_FILE:
			return PrintFileName(handle, file);
	}

	ULONG size = 0x2000;

	if (GOODSTATUS(pNtQueryObject(handle, 1, NULL, 0, &size)))
	{
		// let's try to use the default
		if (size == 0)
			size = 0x2000;

		UCHAR* lpBuffer = new UCHAR[size];

		if (pNtQueryObject(handle, 1, lpBuffer, size, 0) == 0)
		{
#define REGISTRY L"\\REGISTRY\\"
#define USER L"USER"
#define CLASSES L"MACHINE\\SOFTWARE\\CLASSES"
#define MACHINE L"MACHINE"
#define _CLASSES L"_Classes"

			if (((UNICODE_STRING*)lpBuffer)->Length)
			{
				wchar_t *ws = ((UNICODE_STRING*)lpBuffer)->Buffer;

				if (type==OB_TYPE_KEY && !_memicmp(ws, REGISTRY, sizeof(REGISTRY)-2))
				{
					wchar_t *ws1 = ws + ARRAYSIZE(REGISTRY) - 1;
					const wchar_t *s0 = 0;

					if (!_memicmp(ws1, USER, sizeof(USER)-2))
					{
						ws1 += ARRAYSIZE(USER) - 1;
						size_t l  = lstrlenW(pUserAccountID);

						if (l>0 && !_memicmp(ws1,pUserAccountID,l*2))
						{
							s0 = L"HKCU";
							ws1 += l;

							if (!_memicmp(ws1,_CLASSES,sizeof(_CLASSES)-2))
							{
								s0 = L"HKCU\\Classes";
								ws1 += ARRAYSIZE(_CLASSES) - 1;
							}
						}
						else
							s0 = L"HKU";
					}
					else if (!_memicmp(ws1, CLASSES, sizeof(CLASSES)-2)) { s0 = L"HKCR"; ws1+=ARRAYSIZE(CLASSES) - 1;}
					else if (!_memicmp(ws1, MACHINE, sizeof(MACHINE)-2)) { s0 = L"HKLM"; ws1+=ARRAYSIZE(MACHINE) - 1;}

					if (s0)
					{
						fprintf(file, L"%s", s0);
						ws = ws1;
					}
				}

				fprintf(file, L"%ls", ws);
				ret = true;
			}
		}

		delete [] lpBuffer;
	}

	return ret;
}

static bool PrintNameAndType(HANDLE h, DWORD dwPID, HANDLE file, PerfThread* pThread=0)
{
	HANDLE handle, hRemoteProcess=NULL;
	bool remote = dwPID != GetCurrentProcessId();

	if (remote)
	{
		DebugToken token;
		hRemoteProcess = OpenProcessForced(&token, PROCESS_DUP_HANDLE, dwPID, TRUE);

		if (hRemoteProcess == NULL)
			return false;

		if (!DuplicateHandle(hRemoteProcess, h, GetCurrentProcess(), &handle,0,0, DUPLICATE_SAME_ACCESS))
			handle = 0;
	}
	else
		handle = h;

	WORD type = GetType(handle);

	if (type < ARRAYSIZE(constStrTypes))
		fprintf(file, L"%-13s ", constStrTypes[type]);

	bool ret = type!=OB_TYPE_UNKNOWN &&
	           PrintNameByType(handle, type, file, pThread);

	if (remote)
	{
		if (hRemoteProcess) CloseHandle(hRemoteProcess);

		if (handle) CloseHandle(handle);
	}

	return ret;
}

bool PrintHandleInfo(DWORD dwPID, HANDLE file, bool bIncludeUnnamed, PerfThread* pThread)
{
	bool ret = true;
	DWORD i;
	DWORD size = 0x2000, needed = 0;
	SYSTEM_HANDLE_INFORMATION* pSysHandleInformation = (SYSTEM_HANDLE_INFORMATION*)
	        VirtualAlloc(NULL, size, MEM_COMMIT, PAGE_READWRITE);

	if (pSysHandleInformation == NULL)
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
		pSysHandleInformation = (SYSTEM_HANDLE_INFORMATION*)
		                        VirtualAlloc(NULL, size = needed+256, MEM_COMMIT, PAGE_READWRITE);
	}

	if (pSysHandleInformation == NULL)
		return false;

	// Query the objects ( system wide )
	if (pNtQuerySystemInformation(16, pSysHandleInformation, size, NULL))
	{
		ret = false;
		goto cleanup;
	}

	fprintf(file, L"%s\n%s\n", GetMsg(MTitleHandleInfo), GetMsg(MHandleInfoHdr));

	if (!pUserAccountID)
		pUserAccountID = GetUserAccountID(); // init once

	// Iterating through the objects
	for (i = 0; i < pSysHandleInformation->NumberOfHandles; i++)
	{
		// ProcessId filtering check
		if (pSysHandleInformation->Handles[i].UniqueProcessId==dwPID || dwPID==(DWORD)-1)
		{
			pSysHandleInformation->Handles[i].HandleAttributes = (UCHAR)(pSysHandleInformation->Handles[i].HandleAttributes & 0xff);
			fprintf(file, L"%5X  %08X ",
			        pSysHandleInformation->Handles[i].HandleValue,
			        /*dwType< ARRAYSIZE(constStrTypes) ?
			            constStrTypes[dwType] : _L"(Unknown)",*/
//              pSysHandleInformation->Handles[i].KernelAddress,
			        pSysHandleInformation->Handles[i].GrantedAccess);
			PrintNameAndType((HANDLE)(SIZE_T)(UINT)pSysHandleInformation->Handles[i].HandleValue, dwPID, file, pThread);
			fputc(L'\n', file);
		}
	}

	fputc(L'\n', file);
cleanup:

	if (pSysHandleInformation != NULL)
		VirtualFree(pSysHandleInformation, 0, MEM_RELEASE);

	return ret;
}
/*
void main(int ac, char** av)
{
    if(ac<2) return;
    PrintHandleInfo(FSF.atoi(av[1]),stdout);
}
*/

static BOOL ConvertSid(PSID pSid, LPWSTR pszSidText, LPDWORD dwBufferLen)
{
	PUCHAR pscnt;
	PSID_IDENTIFIER_AUTHORITY psia;
	DWORD dwSubAuthorities;
	DWORD dwSidRev=SID_REVISION;
	DWORD dwCounter;
	DWORD dwSidSize;

	//
	// test if SID passed in is valid
	//
	if (!pIsValidSid(pSid)) return FALSE;

	// obtain SidIdentifierAuthority
	if ((psia = pGetSidIdentifierAuthority(pSid)) == NULL) return FALSE;

	// obtain sidsubauthority count
	if ((pscnt = pGetSidSubAuthorityCount(pSid)) == NULL) return FALSE;

	dwSubAuthorities=*pscnt;
	//
	// compute buffer length
	// S-SID_REVISION- + identifierauthority- + subauthorities- + NULL
	//
	dwSidSize=(15 + 12 + (12 * dwSubAuthorities) + 1) * sizeof(wchar_t);

	// check provided buffer length.
	// If not large enough, indicate proper size and setlasterror
	if (*dwBufferLen < dwSidSize)
	{
		*dwBufferLen = dwSidSize;
		SetLastError(ERROR_INSUFFICIENT_BUFFER);
		return FALSE;
	}

	// prepare S-SID_REVISION-
	dwSidSize=wsprintfW(pszSidText, L"\\S-%lu-", dwSidRev);
	// prepare SidIdentifierAuthority
	dwSidSize += psia->Value[0] || psia->Value[1] ?
	             wsprintfW(pszSidText + lstrlenW(pszSidText),L"0x%02hx%02hx%02hx%02hx%02hx%02hx",
	                       (USHORT)psia->Value[0], (USHORT)psia->Value[1],
	                       (USHORT)psia->Value[2], (USHORT)psia->Value[3],
	                       (USHORT)psia->Value[4], (USHORT)psia->Value[5])  :
	             wsprintfW(pszSidText + lstrlenW(pszSidText),L"%lu",
	                       (ULONG)(psia->Value[5]) + (ULONG)(psia->Value[4] <<  8) +
	                       (ULONG)(psia->Value[3] << 16) + (ULONG)(psia->Value[2] << 24));

	// loop through SidSubAuthorities
	// obtain sidsubauthority count
	for (dwCounter=0 ; dwCounter < dwSubAuthorities ; dwCounter++)
	{
		DWORD rc = 0, *prc = pGetSidSubAuthority(pSid, dwCounter);

		if (prc) rc = *prc;

		dwSidSize += wsprintfW(pszSidText + dwSidSize, L"-%lu", rc);
	}

	return TRUE;
}

static const wchar_t *GetUserAccountID()
{
	static wchar_t UserAccountID[256];
	DWORD size = ARRAYSIZE(UserAccountID);
	SID_NAME_USE eUse;
	DWORD cbSid=0,cbDomainName=0;

	if (!GetUserName(UserAccountID, &size)
	        || !pLookupAccountName(0,UserAccountID,0,&cbSid, 0,&cbDomainName, &eUse))
	{
		return L"";
	}

	PSID pSid = static_cast<PSID>(malloc(cbSid));
	wchar_t* pDomainName = new wchar_t[cbDomainName+1];
	pLookupAccountName(0,UserAccountID,pSid,&cbSid, pDomainName,&cbDomainName, &eUse);
	size = ARRAYSIZE(UserAccountID);

	if (!ConvertSid(pSid, (wchar_t*)UserAccountID, &size)) *UserAccountID = 0;

	free(pSid);
	delete[] pDomainName;
	return (const wchar_t *)UserAccountID;
}
