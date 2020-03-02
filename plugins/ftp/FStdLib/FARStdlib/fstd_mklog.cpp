#include <all_far.h>
#pragma hdrstop

#include "fstdlib.h"

//DATA
CRITICAL_SECTION    PLOG_cs;

BOOL WINAPI LOGInit(void)
{
	static BOOL inited = FALSE;

	if(inited) return FALSE;

	InitializeCriticalSection(&PLOG_cs);
	inited=TRUE;
#if defined(__QNX__)
	setbuf(stdout,NULL);
#endif
	return TRUE;
}

LPCSTR WINAPI FP_GetLogFullFileName(void)
{
	static char str[MAX_PATH] = "";
	LPCSTR  m;
	char     *tmp;

	if(!str[0])
	{
		m = FP_GetPluginLogName();

		if(!m || !m[0])
			return "";

		str[ GetModuleFileName(FP_HModule,str,sizeof(str))] = 0;
		tmp = strrchr(str,'\\');

		if(tmp)
		{
			tmp[1] = 0;
			strcat(str,m);
		}
		else
			strcpy(str,m);
	}

	return str;
}
