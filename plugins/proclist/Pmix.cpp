#include "Proclist.hpp"
#include "Proclng.hpp"
#include "guid.hpp"

#include <smart_ptr.hpp>

const wchar_t* GetMsg(int MsgId)
{
	return PsInfo.GetMsg(&MainGuid, MsgId);
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

	FILETIME ct;
	FileTimeToLocalFileTime(&ft, &ct);

	SYSTEMTIME st;
	FileTimeToSystemTime(&ct, &st);

	if (TimeText)
		GetTimeFormat(LOCALE_USER_DEFAULT, 0, &st, {}, TimeText, MAX_DATETIME);

	if (DateText)
		GetDateFormat(LOCALE_USER_DEFAULT, 0, &st, {}, DateText, MAX_DATETIME);
}

int WinError(const wchar_t* pSourceModule)
{
	local_ptr<wchar_t> SysBuffer;
	const auto LastError = GetLastError();

	std::wstring Buffer;

	if (const auto Size = FormatMessage(
		(pSourceModule? FORMAT_MESSAGE_FROM_HMODULE : FORMAT_MESSAGE_FROM_SYSTEM) | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS,
		pSourceModule? GetModuleHandle(pSourceModule) : nullptr,
		LastError,
		0,
		reinterpret_cast<wchar_t*>(&ptr_setter(SysBuffer)), 0, {}))
	{
		Buffer = { SysBuffer.get(), Size };
	}
	else
	{
		Buffer = far::format(L"Error 0x{0:08X}"sv, LastError);
	}

	Buffer.resize(Buffer.find_last_not_of(L"\r\n") + 1);

	const wchar_t* items[]
	{
		GetMsg(MError),
		Buffer.c_str(),
		GetMsg(MOk),
	};

	return Message(FMSG_WARNING, {}, items, std::size(items));
}
