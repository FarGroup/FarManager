#include "Proclist.hpp"
#include "Proclng.hpp"
#include "guid.hpp"

const wchar_t* GetMsg(int MsgId)
{
	return Info.GetMsg(&MainGuid, MsgId);
}

void ConvertDate(const FILETIME& ft, wchar_t* DateText, wchar_t* TimeText)
{
	if (ft.dwHighDateTime == 0 && ft.dwLowDateTime == 0)
	{
		if (DateText)
			*DateText = 0;

		if (TimeText)
			*TimeText = 0;

		return;
	}

	SYSTEMTIME st;
	FILETIME ct;
	FileTimeToLocalFileTime(&ft, &ct);
	FileTimeToSystemTime(&ct, &st);

	if (TimeText)
		GetTimeFormat(LOCALE_USER_DEFAULT, 0, &st, {}, TimeText, MAX_DATETIME);

	if (DateText)
		GetDateFormat(LOCALE_USER_DEFAULT, 0, &st, {}, DateText, MAX_DATETIME);
}

int WinError(const wchar_t* pSourceModule)
{
	wchar_t* lpMsgBuf;
	const auto dwLastErr = GetLastError();
	const size_t Size = FormatMessage(
	(pSourceModule? FORMAT_MESSAGE_FROM_HMODULE : FORMAT_MESSAGE_FROM_SYSTEM) | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS,
		pSourceModule? GetModuleHandle(pSourceModule) : nullptr,
		dwLastErr,
		0,
		reinterpret_cast<wchar_t*>(&lpMsgBuf), 0, {});

	wchar_t ErrBuf[32];

	if (!Size)
	{
		FSF.sprintf(ErrBuf, L"Error 0x%x", dwLastErr);
		lpMsgBuf = ErrBuf;
	}

	static const wchar_t* items[]
	{
		{},
		{},
		{},
		{},
	};

	items[0] = GetMsg(MError); items[3] = GetMsg(MOk);

	if (std::wcslen(lpMsgBuf) > 64)
	{
		for (auto i = std::wcslen(lpMsgBuf) / 2; i < std::wcslen(lpMsgBuf); ++i)
		{
			if (lpMsgBuf[i] == L' ')
			{
				lpMsgBuf[i] = L'\n';
				break;
			}
		}
	}

	items[1] = std::wcstok(lpMsgBuf, L"\r\n");
	items[2] = std::wcstok({}, L"\r\n");

	if (!items[2])
		items[2] = items[3];

	const auto rc = Message(FMSG_WARNING, {}, items, std::size(items) - (items[2] == items[3]));

	if (Size)
		LocalFree(lpMsgBuf);

	return rc;
}
