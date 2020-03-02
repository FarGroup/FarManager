#include <Rtl.Base.h>

HANDLE RtlCreateEvent(
		const TCHAR* EventName
		)
{
	return CreateEvent(
			NULL, 
			false, 
			false, 
			EventName
			);
}

HANDLE RtlCreateThread(
		PVOID ThreadProc, 
		PVOID Param
		)
{
	DWORD dwThreadId;

	return CreateThread(
			NULL,
			0,
			(LPTHREAD_START_ROUTINE)ThreadProc,
			Param,
			0,
			&dwThreadId
			);
}


VOID RtlWaitAndCloseThread(
		HANDLE hThread,
		int WaitInterval
		)
{
	WaitForSingleObject(hThread, WaitInterval);
	CloseHandle(hThread);
}


