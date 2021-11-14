#include "NetCommon.hpp"
#include "guid.hpp"

const wchar_t* GetMsg(int MsgId)
{
	return PsInfo.GetMsg(&MainGuid, MsgId);
}

TSaveScreen::TSaveScreen()
{
	hScreen = PsInfo.SaveScreen(0, 0, -1, -1);
	const wchar_t* MsgItems[] = {GetMsg(MWaitForNetworkBrowse1), GetMsg(MWaitForNetworkBrowse2)};
	PsInfo.Message(&MainGuid, nullptr, 0, {}, MsgItems,ARRAYSIZE(MsgItems), 0);
}

TSaveScreen::~TSaveScreen()
{
	if (hScreen)
	{
		PsInfo.RestoreScreen({});
		PsInfo.RestoreScreen(hScreen);
	}
}
