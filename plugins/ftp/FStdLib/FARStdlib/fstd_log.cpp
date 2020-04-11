#include <all_far.h>
#pragma hdrstop

#include "fstdlib.h"

//------------------------------------------------------------------------
//[fstd_mklog.cpp]
extern BOOL WINAPI LOGInit(void);
extern CRITICAL_SECTION PLOG_cs;

/** @brief Length of text with std error writen to log file.

    Length of left part log string with description of __WINError() in moment of write log call.
    Default value 30 characters.
    Set this value to 0 to disable write error description.
*/
int FP_LogErrorStringLength = 30;

//------------------------------------------------------------------------
void __cdecl FP_FILELog(LPCSTR msg,...)
{
	FILE   *f;
	BOOL    first;
	va_list argptr;
	char    str[3000],*m;
	DWORD   err = GetLastError();

	if(!msg) return;

	first = LOGInit();
	EnterCriticalSection(&PLOG_cs);
	m = (char*)FP_GetLogFullFileName();
	f = (m && *m) ? fopen(m,first?"w":"a") : NULL;

	if(f)
	{
		//Time
		static SYSTEMTIME stOld = { 0 };
		SYSTEMTIME st;
		GetLocalTime(&st);
		fprintf(f,"%4d.%02d.%02d %02d:%02d:%02d:%04d ",
		        st.wYear, st.wMonth,  st.wDay,
		        st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

		if(!stOld.wYear)
			fprintf(f,"---- ");
		else
			fprintf(f,"%04d ",
			        (st.wSecond-stOld.wSecond)*1000 + (st.wMilliseconds-stOld.wMilliseconds));

		stOld = st;

		//Error
		if(FP_LogErrorStringLength)
		{
			SetLastError(err);
			StrCpy(str,__WINError(),FP_LogErrorStringLength);

			if((m=strchr(str,'\n')) != NULL) *m = 0;

			if((m=strchr(str,'\r')) != NULL) *m = 0;

			fprintf(f,"%-*s->",FP_LogErrorStringLength,str);
		}
		else
			fprintf(f,"->");

		//Message
		StrCpy(str,msg,sizeof(str));

		if((m=strchr(str,'\n')) != NULL) *m = 0;

		if((m=strchr(str,'\r')) != NULL) *m = 0;

		va_start(argptr, msg);
		vfprintf(f,str,argptr);
		va_end(argptr);
		//EOL
		fprintf(f,"\n");
		fclose(f);
	}

	LeaveCriticalSection(&PLOG_cs);
	SetLastError(err);
}
