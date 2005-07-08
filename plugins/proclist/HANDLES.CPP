// Based on Zoltan Csizmadia's TaskManagerEx source, zoltan_csizmadia@yahoo.com
/*
$Id$

$Log$
*/

#include "proclist.hpp"
#include "perfthread.hpp" // fot GetProcessData
#include "proclng.hpp"
#include <stdio.h>

struct SYSTEM_HANDLE {
	DWORD	ProcessID;
	WORD	HandleType;
	WORD	HandleNumber;
	DWORD	KernelAddress;
	DWORD	Flags;
};
struct SYSTEM_HANDLE_INFORMATION
{
	DWORD		Count;
	SYSTEM_HANDLE	Handles[1];
};
struct BASIC_THREAD_INFORMATION {
	DWORD u1;
	DWORD u2;
	DWORD u3;
	DWORD ThreadId;
	DWORD u5;
	DWORD u6;
	DWORD u7;
};
struct PROCESS_BASIC_INFORMATION {
    DWORD ExitStatus;
    PVOID PebBaseAddress;
    DWORD AffinityMask;
    DWORD BasePriority;
    DWORD UniqueProcessId;
    DWORD InheritedFromUniqueProcessId;
};
struct UNICODE_STRING
{
    WORD  Length;
    WORD  MaximumLength;
    PWSTR Buffer;
};

enum { OB_TYPE_UNKNOWN = 0, OB_TYPE_TYPE = 1, OB_TYPE_DIRECTORY,
	OB_TYPE_SYMBOLIC_LINK,	OB_TYPE_TOKEN,	OB_TYPE_PROCESS,
	OB_TYPE_THREAD,	/*OB_TYPE_UNKNOWN_7,*/ OB_TYPE_EVENT,
	OB_TYPE_EVENT_PAIR, OB_TYPE_MUTANT, //OB_TYPE_UNKNOWN_11,
	OB_TYPE_SEMAPHORE, OB_TYPE_TIMER, OB_TYPE_PROFILE,
	OB_TYPE_WINDOW_STATION, OB_TYPE_DESKTOP, OB_TYPE_SECTION,
	OB_TYPE_KEY, OB_TYPE_PORT, OB_TYPE_WAITABLE_PORT,
	/*OB_TYPE_UNKNOWN_21, OB_TYPE_UNKNOWN_22, OB_TYPE_UNKNOWN_23,
	OB_TYPE_UNKNOWN_24,*/ OB_TYPE_JOB, //22 on Whistler
	//OB_TYPE_CONTROLLER, OB_TYPE_DEVICE, OB_TYPE_DRIVER,
	OB_TYPE_IO_COMPLETION, OB_TYPE_FILE, OB_TYPE_WMI_GUID,
};

wchar_t* GetUserAccountID();
static wchar_t* pUserAccountID;

BOOL GetProcessId( HANDLE handle, DWORD& dwPID)
{
    BOOL ret = FALSE;
    bool remote = false;
    PROCESS_BASIC_INFORMATION pi;
    
    ZeroMemory( &pi, sizeof(pi) );
    dwPID = 0;
    
    // Get the process information
    typedef LONG (WINAPI *PNtQueryInformationProcess)(HANDLE,UINT,PVOID,ULONG,PULONG);
    DYNAMIC_ENTRY(NtQueryInformationProcess,GetModuleHandle("ntdll"))

    if ( pNtQueryInformationProcess( handle, 0, &pi, sizeof(pi), NULL) == 0 )
    {
    	dwPID = pi.UniqueProcessId;
    	ret = TRUE;
    }

    return ret;
}
BOOL GetThreadId( HANDLE h, DWORD& threadID)
{
    BOOL ret = FALSE;
    BASIC_THREAD_INFORMATION ti;
    HANDLE handle, hRemoteProcess = NULL;
    bool remote = false;
    
    handle = h;
    
    typedef DWORD (WINAPI *PNtQueryInformationThread)(HANDLE, ULONG, PVOID, DWORD, DWORD* );
    DYNAMIC_ENTRY(NtQueryInformationThread, GetModuleHandle("ntdll"))
    // Get the thread information
    if ( pNtQueryInformationThread( handle, 0, &ti, sizeof ti, NULL ) == 0 )
    {
    	threadID = ti.ThreadId;
    	ret = TRUE;
    }

    if ( remote )
    {
    	if ( hRemoteProcess ) CloseHandle( hRemoteProcess );
    	if ( handle ) CloseHandle( handle );
    }
    return ret;
}

inline bool GOODSTATUS(DWORD st) { return !(st) || (st)==0xC0000004L; }

typedef DWORD (WINAPI *PNtQueryObject)( HANDLE, DWORD, VOID*, DWORD, VOID* );
PNtQueryObject NtQueryObject = (PNtQueryObject)GetProcAddress(GetModuleHandle("ntdll"),
            "NtQueryObject");

DWORD WINAPI GetFileNameThread(PVOID Param)
{
    typedef DWORD (WINAPI *PNtQueryInformationFile)( HANDLE, PVOID, PVOID, DWORD, DWORD );
    DYNAMIC_ENTRY(NtQueryInformationFile, GetModuleHandle("ntdll"))

    DWORD iob[2];
    char info[256];
    pNtQueryInformationFile((HANDLE)Param, &iob, &info,sizeof info, 22);
    return 0;
}

bool PrintFileName(HANDLE handle, FILE* file)
{
    bool ret = false;

    // Check if it's possible to get the file name info
    DWORD dwThreadId;    
    HANDLE hThread = CreateThread(0, 0, GetFileNameThread, handle, 0, &dwThreadId);

    // Wait for finishing the thread
    if ( WaitForSingleObject( hThread, 100 ) == WAIT_TIMEOUT )
    {	
    	// Access denied, terminate the thread
    	TerminateThread( hThread, 0 );
    	ret = true;
    }
    else {
	// it is safe to call NtQueryObject
	ULONG size = 0x2000;
	if(GOODSTATUS(NtQueryObject ( handle, 1, NULL, 0, &size ))) {
	// let's try to use the default
	    if ( size == 0 )
    		size = 0x2000;
    
	    UCHAR* lpBuffer = new UCHAR[size];
	    if ( NtQueryObject( handle, 1, lpBuffer, size, 0 ) == 0 &&
		 ((UNICODE_STRING*)lpBuffer)->Length)
		    fprintf(file, "%S", ((UNICODE_STRING*)lpBuffer)->Buffer);
	    else
    		ret = true;
	    delete lpBuffer;
	}
    }

    CloseHandle(hThread);
    return ret;
}

bool GetTypeToken(HANDLE handle, char* str, DWORD dwSize)
{
    ULONG size = 0x2000;
    bool ret = false;

    // Query the info size
    if(GOODSTATUS(NtQueryObject( handle, 2, NULL, 0, &size ))) {

	UCHAR* lpBuffer = new UCHAR[size];

	// Query the info size ( type )
	if ( NtQueryObject( handle, 2, lpBuffer, size, NULL ) == 0 )
	{
	    WideCharToMultiByte( CP_ACP, 0, (LPCWSTR)(lpBuffer+0x60), -1,
		    str, dwSize, NULL, NULL);
    	    ret = true;
	}
	delete lpBuffer;
    }
    return ret;
}

char* constStrTypes[] = { 
    "", "", "Directory", "SymbolicLink", "Token",
    "Process", "Thread", /*"Unknown7",*/ "Event", "EventPair", "Mutant", 
    /*"Unknown11", */"Semaphore", "Timer", "Profile", "WindowStation",
    "Desktop", "Section", "Key", "Port", "WaitablePort", 
    //"Unknown21", "Unknown22", "Unknown23", "Unknown24",
    "Job",
    "IoCompletion", "File", /*"Unknown27",*/ "WmiGuid", };

WORD GetTypeFromTypeToken( LPCSTR typeToken)
{
    for ( WORD i = 1; i < sizeof constStrTypes/sizeof *constStrTypes; i++ )
	if ( !stricmp(constStrTypes[i], typeToken) )
	    return i;
    return OB_TYPE_UNKNOWN;
}

WORD GetType( HANDLE h)
{
    char strType[256];
    return GetTypeToken( h, strType, sizeof strType) ? 
	GetTypeFromTypeToken(strType) : OB_TYPE_UNKNOWN;
}

bool PrintNameByType(HANDLE handle, WORD type, FILE* file, PerfThread* pThread=0)
{
    bool ret = false;

    // let's be happy, handle is in our process space, so query the infos :)
    DWORD dwId = 0;
    switch( type )
    {
        case OB_TYPE_PROCESS:
	    if(GetProcessId( handle, dwId ))
	    {
		Lock l(pThread);
		ProcessPerfData* pd = pThread ? pThread->GetProcessData(dwId, 0) : 0;
		char* pName = pd ? pd->ProcessName : "<unknown>";
		fprintf(file, "%s (%d)", pName, dwId );
	    }
	    return true;

        case OB_TYPE_THREAD:
	    if(GetThreadId( handle, dwId ))
		fprintf(file, "TID: %d" , dwId );    			
	    return true;

        case OB_TYPE_FILE:
            return PrintFileName( handle, file);
    }

    ULONG size = 0x2000;

    if(GOODSTATUS(NtQueryObject ( handle, 1, NULL, 0, &size ))) {

	// let's try to use the default
	if ( size == 0 )
    	    size = 0x2000;

	UCHAR* lpBuffer = new UCHAR[size];

	if ( NtQueryObject( handle, 1, lpBuffer, size, 0 ) == 0 )
	{
#define REGISTRY L"\\REGISTRY\\"
#define USER L"USER"
#define CLASSES L"MACHINE\\SOFTWARE\\CLASSES"
#define MACHINE L"MACHINE"
#define _CLASSES L"_Classes"

	    if(((UNICODE_STRING*)lpBuffer)->Length) {
		wchar_t *ws = ((UNICODE_STRING*)lpBuffer)->Buffer;
		if(type==OB_TYPE_KEY && !memicmp(ws, REGISTRY, sizeof REGISTRY-2) ) {
		    wchar_t *ws1 = ws + sizeof REGISTRY / 2 - 1;
		    char *s0 = 0;
		    if(!memicmp(ws1, USER, sizeof USER-2)) {
                	    ws1 += sizeof USER / 2 - 1;
			    size_t l  = wcslen(pUserAccountID);
			    if(l>0 && !memicmp(ws1,pUserAccountID,l*2)) {
				s0 = "HKCU";
				ws1 += l;
				if(!memicmp(ws1,_CLASSES,sizeof _CLASSES-2)) {
				    s0 = "HKCU\\Classes";
				    ws1 += sizeof _CLASSES / 2 - 1;
				}
			    }
			    else
				s0 = "HKU";
		    }
		    else if(!memicmp(ws1, CLASSES, sizeof CLASSES-2)) { s0 = "HKCR"; ws1+=(sizeof CLASSES)/2 - 1;}
		    else if(!memicmp(ws1, MACHINE, sizeof MACHINE-2)) { s0 = "HKLM"; ws1+=(sizeof MACHINE)/2 - 1;}
		    if(s0) {
			fprintf(file, "%s", s0);
			ws = ws1;
		    }
		}
		fprintf(file, "%S", ws);
    		ret = true;
	    }
	}
    
	delete [] lpBuffer;    
    }
    return ret;
}

bool PrintNameAndType(HANDLE h, DWORD dwPID, FILE* file, PerfThread* pThread=0)
{
    HANDLE handle, hRemoteProcess=NULL;
    bool remote = dwPID != GetCurrentProcessId();
    if ( remote )
    {
    	hRemoteProcess = OpenProcessForced( PROCESS_DUP_HANDLE, dwPID, TRUE );
    	if( hRemoteProcess == NULL )
    	    return false;
    	if(!DuplicateHandle( hRemoteProcess, h, GetCurrentProcess(), &handle,0,0, DUPLICATE_SAME_ACCESS))
    	    handle = 0;
    }
    else
    	handle = h;

    WORD type = GetType(handle);
    if(type<sizeof constStrTypes/sizeof *constStrTypes)
	fprintf(file, "%-13s ", constStrTypes[type]);
    bool ret = type!=OB_TYPE_UNKNOWN && 
	    PrintNameByType( handle, type, file, pThread);

    if ( remote )
    {
    	if(hRemoteProcess) CloseHandle( hRemoteProcess );
    	if(handle) CloseHandle( handle );
    }
    
    return ret;
}

inline bool IsSupportedHandle( SYSTEM_HANDLE& handle )
{
	//Here you can filter the handles you don't want in the Handle list

	// Windows 2000 supports everything :)
    extern int W2K;
	if ( W2K )
		return true;

	//NT4 System process doesn't like if we bother his internal security :)
	if ( handle.ProcessID == 2 && handle.HandleType == 16 )
		return false;

	return true;
}
bool PrintHandleInfo(DWORD dwPID, FILE* file, bool bIncludeUnnamed, PerfThread* pThread)
{
    bool ret = true;
    DWORD i;
    typedef DWORD (WINAPI *PNtQuerySystemInformation)( DWORD, VOID*, DWORD, ULONG* );
    DYNAMIC_ENTRY(NtQuerySystemInformation, GetModuleHandle("ntdll"))

    DWORD size = 0x2000, needed = 0;
    SYSTEM_HANDLE_INFORMATION* pSysHandleInformation = (SYSTEM_HANDLE_INFORMATION*)
	VirtualAlloc( NULL, size, MEM_COMMIT, PAGE_READWRITE );
    if( pSysHandleInformation == NULL )
	return false;

    if(pNtQuerySystemInformation( 16, pSysHandleInformation, size, &needed)) {
     	if( needed == 0 )
     	{
     		ret = false;
     		goto cleanup;
     	}
     	// The size was not enough
     	VirtualFree( pSysHandleInformation, 0, MEM_RELEASE );
     	pSysHandleInformation = (SYSTEM_HANDLE_INFORMATION*)
		VirtualAlloc( NULL, size = needed+256, MEM_COMMIT, PAGE_READWRITE );
    }
    if ( pSysHandleInformation == NULL )
	return false;

    // Query the objects ( system wide )
    if(pNtQuerySystemInformation( 16, pSysHandleInformation, size, NULL))
    {
    	ret = false;
    	goto cleanup;
    }
    fprintf(file, "%s\n%s\n", GetMsg(MTitleHandleInfo), GetMsg(MHandleInfoHdr));

    if(!pUserAccountID)
	pUserAccountID = GetUserAccountID(); // init once

    // Iterating through the objects
    for(i = 0; i < pSysHandleInformation->Count; i++)
    {
    	if ( !IsSupportedHandle( pSysHandleInformation->Handles[i] ) )
    		continue;
    	
    	// ProcessId filtering check
    	if(pSysHandleInformation->Handles[i].ProcessID==dwPID || dwPID==(DWORD)-1)
    	{
            pSysHandleInformation->Handles[i].HandleType = (WORD)(pSysHandleInformation->Handles[i].HandleType & 0xff);
            fprintf(file, "%5X  %08X ",
            	pSysHandleInformation->Handles[i].HandleNumber,
            	/*dwType< sizeof constStrTypes/sizeof *constStrTypes ?
		    constStrTypes[dwType] : "(Unknown)",*/
//            	pSysHandleInformation->Handles[i].KernelAddress,
            	pSysHandleInformation->Handles[i].Flags);
            PrintNameAndType((HANDLE)pSysHandleInformation->Handles[i].HandleNumber,
            	dwPID, file, pThread);
            fputc('\n', file);
    	}
    }
    fputc('\n', file);

cleanup:
    if ( pSysHandleInformation != NULL )
	VirtualFree( pSysHandleInformation, 0, MEM_RELEASE );

    return ret;
}
/*
void main(int ac, char** av)
{
    if(ac<2) return;
    PrintHandleInfo(atoi(av[1]),stdout);
}
*/

BOOL ConvertSid(PSID pSid, LPWSTR pszSidText, LPDWORD dwBufferLen)
{
   PSID_IDENTIFIER_AUTHORITY psia;
   DWORD dwSubAuthorities;
   DWORD dwSidRev=SID_REVISION;
   DWORD dwCounter;
   DWORD dwSidSize;
   HMODULE hApi32 = GetModuleHandle("advapi32");
   // 
   // test if SID passed in is valid
   // 
   typedef BOOL (WINAPI *PIsValidSid)(PSID);
   DYNAMIC_ENTRY(IsValidSid, hApi32)
   
   if(!pIsValidSid || !pIsValidSid(pSid)) return FALSE;

   // obtain SidIdentifierAuthority
   typedef PSID_IDENTIFIER_AUTHORITY (WINAPI *PGetSidIdentifierAuthority)(PSID);
   DYNAMIC_ENTRY(GetSidIdentifierAuthority, hApi32)
   
   if(!pGetSidIdentifierAuthority || !pGetSidIdentifierAuthority(pSid)) return FALSE;

   psia = pGetSidIdentifierAuthority(pSid);

   // obtain sidsubauthority count
   typedef PUCHAR (WINAPI *PGetSidSubAuthorityCount)(PSID);
   DYNAMIC_ENTRY(GetSidSubAuthorityCount, hApi32)
   
   if(!pGetSidIdentifierAuthority || !pGetSidIdentifierAuthority(pSid)) return FALSE;

   dwSubAuthorities=*pGetSidSubAuthorityCount(pSid);

   // 
   // compute buffer length
   // S-SID_REVISION- + identifierauthority- + subauthorities- + NULL
   // 
   dwSidSize=(15 + 12 + (12 * dwSubAuthorities) + 1) * sizeof(TCHAR);

   // check provided buffer length.
   // If not large enough, indicate proper size and setlasterror
   if (*dwBufferLen < dwSidSize){
      *dwBufferLen = dwSidSize;
      SetLastError(ERROR_INSUFFICIENT_BUFFER);
      return FALSE;
   }

   // prepare S-SID_REVISION-
   dwSidSize=wsprintfW(pszSidText, L"\\S-%lu-", dwSidRev );

   // prepare SidIdentifierAuthority
   dwSidSize += psia->Value[0] || psia->Value[1] ?
      wsprintfW(pszSidText + lstrlenW(pszSidText),L"0x%02hx%02hx%02hx%02hx%02hx%02hx",
          (USHORT)psia->Value[0], (USHORT)psia->Value[1],
          (USHORT)psia->Value[2], (USHORT)psia->Value[3],
          (USHORT)psia->Value[4], (USHORT)psia->Value[5])  :
      wsprintfW(pszSidText + lstrlenW(pszSidText),L"%lu",
          (ULONG)(psia->Value[5]      ) + (ULONG)(psia->Value[4] <<  8) +
          (ULONG)(psia->Value[3] << 16) + (ULONG)(psia->Value[2] << 24) );

   // loop through SidSubAuthorities
   // obtain sidsubauthority count
   typedef PDWORD (WINAPI *PGetSidSubAuthority)(PSID, DWORD);
   DYNAMIC_ENTRY(GetSidSubAuthority, hApi32)

   for (dwCounter=0 ; dwCounter < dwSubAuthorities ; dwCounter++){
      dwSidSize += wsprintfW(pszSidText + dwSidSize, L"-%lu",
      *pGetSidSubAuthority(pSid, dwCounter) );
   }

   return TRUE;

}

wchar_t* GetUserAccountID()
{
static char UserAccountID[256];
   DWORD size = sizeof UserAccountID;
   SID_NAME_USE eUse;
   DWORD cbSid=0,cbDomainName=0;

   typedef BOOL (WINAPI *PLookupAccountNameA)(LPCTSTR,LPCTSTR,PSID,LPDWORD,LPTSTR,LPDWORD,PSID_NAME_USE);
   DYNAMIC_ENTRY(LookupAccountNameA, GetModuleHandle("advapi32"))

   if(!GetUserName(UserAccountID,&size) || !pLookupAccountNameA)
       return L"";

   pLookupAccountNameA(0,UserAccountID,0,&cbSid, 0,&cbDomainName, &eUse);

   PSID pSid = (PSID)new char[cbSid];
   char* pDomainName = new char[cbDomainName+1];
   pLookupAccountNameA(0,UserAccountID,pSid,&cbSid, pDomainName,&cbDomainName, &eUse);
   size = sizeof UserAccountID;
   if(!ConvertSid(pSid, (wchar_t*)UserAccountID, &size)) *UserAccountID = 0;
   delete pSid;
   delete pDomainName;
   return (wchar_t*)UserAccountID;
}
