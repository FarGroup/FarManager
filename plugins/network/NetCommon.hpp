#ifndef __NETCOMMON_HPP__
#define __NETCOMMON_HPP__

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4121)
#endif // _MSC_VER

#include <windows.h>
#include <lm.h>
#include <plugin.hpp>
#include "NetLng.hpp"

extern struct Options
{
	int AddToDisksMenu;
	int AddToPluginsMenu;
	int HiddenShares;
	int ShowPrinters;
	int LocalNetwork;
	int DisconnectMode;
	int ConfirmRemoveConnection;
	int HiddenSharesAsHidden;
	int FullPathShares;
	int FavoritesFlags;
	int RootDoublePoint;
	int NavigateToDomains;
} Opt;

extern PluginStartupInfo PsInfo;
extern FarStandardFunctions FSF;
extern NETRESOURCE CommonCurResource;
extern LPNETRESOURCE PCommonCurResource;
extern BOOL IsFirstRun;

class TSaveScreen
{
private:
	HANDLE hScreen;

public:
	TSaveScreen();
	~TSaveScreen();
};

const wchar_t* GetMsg(int MsgId);

BOOL DlgCreateFolder(wchar_t* lpBuffer, int nBufferSize);

#define ShowMessage(x) PsInfo.Message(&MainGuid, nullptr, FMSG_ALLINONE|FMSG_MB_OK, L"", reinterpret_cast<const wchar_t* const*>(x), 0,{})
/* NO NEED THIS
char* NextToken(char *szSource, char *szToken, int nBuff);
*/

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif // __NETCOMMON_HPP__
