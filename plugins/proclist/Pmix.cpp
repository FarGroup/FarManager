#include "Proclist.hpp"
#include "Proclng.hpp"

const wchar_t *GetMsg(int MsgId)
{
  return Info.GetMsg(&MainGuid,MsgId);
}

void ConvertDate(const FILETIME& ft,wchar_t *DateText,wchar_t *TimeText)
{
	if (ft.dwHighDateTime==0 && ft.dwLowDateTime==0)
	{
		if (DateText!=NULL)
			*DateText=0;

		if (TimeText!=NULL)
			*TimeText=0;

		return;
	}

	SYSTEMTIME st;
	FILETIME ct;
	FileTimeToLocalFileTime(&ft,&ct);
	FileTimeToSystemTime(&ct,&st);

	if (TimeText!=NULL)
		GetTimeFormat(LOCALE_USER_DEFAULT, 0, &st, 0, TimeText, MAX_DATETIME);

	if (DateText!=NULL)
		GetDateFormat(LOCALE_USER_DEFAULT, 0, &st, 0, DateText, MAX_DATETIME);
}

int WinError(wchar_t* pSourceModule)
{
	wchar_t* lpMsgBuf; BOOL bAllocated = FALSE;
	DWORD dwLastErr = GetLastError();
	FormatMessage(pSourceModule ?
	              FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE :
	              FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
	              pSourceModule ? GetModuleHandle(pSourceModule): NULL, dwLastErr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
	              (LPTSTR)&lpMsgBuf, 0, NULL);
	wchar_t ErrBuf[32];

	if (lpMsgBuf)
		bAllocated = TRUE;
	else
	{
		FSF.sprintf(ErrBuf, L"Error 0x%x", dwLastErr);
		lpMsgBuf = ErrBuf;
	}

	static const wchar_t* items[]={0,0,0,0};
	items[0] = GetMsg(MError); items[3] = GetMsg(MOk);

	if (lstrlen(lpMsgBuf) > 64)
		for (int i=lstrlen(lpMsgBuf)/2; i<lstrlen(lpMsgBuf); i++)
			if (lpMsgBuf[i]==L' ')
			{
				lpMsgBuf[i] = L'\n';
				break;
			}

	items[1] = _tcstok(lpMsgBuf,L"\r\n");

	items[2] = _tcstok(NULL,L"\r\n");

	if (!items[2])
		items[2] = items[3];

	int rc = Message(FMSG_WARNING,0,items,(int)(ARRAYSIZE(items) - (items[2]==items[3])));

	if (bAllocated)
		LocalFree(lpMsgBuf);

	return rc;
}
