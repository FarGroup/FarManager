#include "NetCommon.hpp"

const wchar_t *GetMsg(int MsgId)
{
	return Info.GetMsg(&MainGuid,MsgId);
}

TSaveScreen::TSaveScreen()
{
	hScreen=Info.SaveScreen(0,0,-1,-1);
	const wchar_t *MsgItems[]={GetMsg(MWaitForNetworkBrowse1),GetMsg(MWaitForNetworkBrowse2)};
	Info.Message(&MainGuid, nullptr,0,NULL,MsgItems,ARRAYSIZE(MsgItems),0);
}

TSaveScreen::~TSaveScreen()
{
	if (hScreen)
	{
		Info.RestoreScreen(NULL);
		Info.RestoreScreen(hScreen);
	}
}
