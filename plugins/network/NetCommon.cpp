#include "NetCommon.hpp"
#include "Network.hpp"
#include "guid.hpp"

Options Opt;

NETRESOURCE CommonCurResource;
NETRESOURCE* PCommonCurResource{};


BOOL DlgCreateFolder(wchar_t* lpBuffer, size_t nBufferSize)
{
	BOOL res = static_cast<BOOL>(PsInfo.InputBox(
		&MainGuid,
		nullptr,
		L"Make Folder",
		L"Folder name:",
		L"NewFolder",
		{},
		lpBuffer,
		nBufferSize,
		{},
		FIB_BUTTONS
	));
	return res;
}

const wchar_t* GetMsg(int MsgId)
{
	return PsInfo.GetMsg(&MainGuid, MsgId);
}

