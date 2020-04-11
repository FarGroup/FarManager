#include <all_far.h>
#pragma hdrstop

#include "fstdlib.h"

const char *__cdecl __WINError(void)
{
	static char *WinEBuff = NULL;
	DWORD err = GetLastError();

	if(WinEBuff) LocalFree(WinEBuff);

	if(FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
	                 NULL,err,
	                 MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
	                 (LPTSTR) &WinEBuff,0,NULL) == 0)
		return "Unknown error";

	char *m;

	if((m=strchr(WinEBuff,'\n')) != 0) *m = 0;

	if((m=strchr(WinEBuff,'\r')) != 0) *m = 0;

	CharToOem(WinEBuff, WinEBuff);
	return WinEBuff;
}
