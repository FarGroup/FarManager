#include <debug.h>
#include <stdio.h>

void __cdecl __debug (const char *format, ...)
{
	char szBuff[1024];
	va_list argptr;

	va_start( argptr, format );
	wvsprintf( szBuff, format, argptr );
	va_end( argptr );

	OemToChar (szBuff, szBuff);

	MessageBox (0, szBuff, "debug", MB_OK|MB_SYSTEMMODAL);
}

int __cdecl dprintf (const char * format, ...)
{
	char szBuff[1024];
	int retValue;
	va_list argptr;

	va_start( argptr, format );
	retValue = wvsprintf( szBuff, format, argptr );
	va_end( argptr );

	OutputDebugString (szBuff);

	return retValue;
}

void __cdecl __fdebug (const char *format, ...)
{
	char szBuff[1024];
	va_list argptr;

	va_start( argptr, format );
	wvsprintf( szBuff, format, argptr );
	va_end( argptr );

	OemToChar (szBuff, szBuff);

	HANDLE hFile = CreateFile (
	       "c:\\debug.log",
	       GENERIC_WRITE,
	       FILE_SHARE_READ|FILE_SHARE_WRITE,
	       NULL,
	       OPEN_ALWAYS,
	       0,
	       NULL
	       );

		if ( hFile != INVALID_HANDLE_VALUE )
		{
			dword dwWritten;
			SetFilePointer (hFile, 0, NULL, FILE_END);

			const char *lpCRLF = "\n\r";

			WriteFile (hFile, szBuff, StrLength(szBuff), &dwWritten, NULL);
			WriteFile (hFile, lpCRLF, 2, &dwWritten, NULL);

			CloseHandle (hFile);
		}
}
