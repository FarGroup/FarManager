#ifdef DEBUG
static void SysLog(TCHAR *fmt,...)
{
  const TCHAR *Log = _T("\\FarCmds.LOG.TMP");
  TCHAR temp[4096];
  SYSTEMTIME SysDate;
  GetLocalTime(&SysDate);
  GetTimeFormat(LOCALE_SYSTEM_DEFAULT, 0, &SysDate, NULL, temp, sizeof(temp));
  lstrcat(temp, _T("> "));
  DWORD dwBytesRead = lstrlen(temp);
  va_list argptr;
  va_start(argptr, fmt);
  wvsprintf(temp+dwBytesRead, fmt, argptr);
  va_end(argptr);
  lstrcat(temp, _T("\n"));
  HANDLE f = CreateFile(Log, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  if ( f != INVALID_HANDLE_VALUE )
  {
    DWORD dwBytesWritten = 0;
    DWORD dwPos = SetFilePointer(f, 0, NULL, FILE_END);
    dwBytesRead = lstrlen(temp);
    LockFile(f, dwPos, 0, dwPos+dwBytesRead, 0);
    WriteFile(f, temp, dwBytesRead, &dwBytesWritten, NULL);
    UnlockFile(f, dwPos, 0, dwPos+dwBytesRead, 0);
  }
  CloseHandle(f);
}
#else
#define SysLog //
#endif
