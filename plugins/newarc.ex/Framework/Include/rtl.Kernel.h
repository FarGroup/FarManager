#pragma once
#include "Rtl.Base.h"

extern HANDLE RtlCreateEvent  (const TCHAR *EventName = NULL);


#define RtlExitThread(i) \
		ExitThread (i); \
		return i; 

extern HANDLE RtlCreateThread (PVOID ThreadProc,
                               PVOID Param = NULL);

extern VOID   RtlWaitAndCloseThread  (HANDLE hThread, 
                                      int WaitInterval = INFINITE);

