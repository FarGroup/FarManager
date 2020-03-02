#include <debug.h>
#include <stdio.h>

void __cdecl __debug (const TCHAR *format, ...)
{
	TCHAR szBuff[1024];
	va_list argptr;

	va_start( argptr, format );
	wvsprintf( szBuff, format, argptr );
	va_end( argptr );

#ifndef _UNICODE
	OemToChar (szBuff, szBuff);
#endif

	MessageBox (0, szBuff, _T("debug"), MB_OK|MB_SYSTEMMODAL);
}

int __cdecl dprintf (const TCHAR * format, ...)
{
	TCHAR szBuff[1024];
	int retValue;
	va_list argptr;

	va_start( argptr, format );
	retValue = wvsprintf( szBuff, format, argptr );
	va_end( argptr );

	OutputDebugString (szBuff);

	return retValue;
}

void __cdecl __fdebug (const TCHAR *format, ...)
{
	TCHAR szBuff[1024];
	va_list argptr;

	va_start( argptr, format );
	wvsprintf( szBuff, format, argptr );
	va_end( argptr );

#ifndef _UNICODE
	OemToChar (szBuff, szBuff);
#endif

	HANDLE hFile = CreateFile (
	       _T("c:\\debug.log"),
	       GENERIC_WRITE,
	       FILE_SHARE_READ|FILE_SHARE_WRITE,
	       NULL,
	       OPEN_ALWAYS,
	       0,
	       NULL
	       );

    if ( hFile != INVALID_HANDLE_VALUE )
    {
		DWORD dwWritten;
    	SetFilePointer (hFile, 0, NULL, FILE_END);

    	TCHAR *lpCRLF = _T("\n\r");

    	WriteFile (hFile, szBuff, _tcslen (szBuff)*sizeof(TCHAR), &dwWritten, NULL);
    	WriteFile (hFile, lpCRLF, 2, &dwWritten, NULL);

    	CloseHandle (hFile);
    }
}
