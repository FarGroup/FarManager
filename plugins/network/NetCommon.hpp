#ifndef __NETCOMMON_HPP__
#define __NETCOMMON_HPP__

#include <plugin.hpp>

extern struct Options
{
	int AddToDisksMenu;
	int AddToPluginsMenu;
	int HiddenShares;
	int ShowPrinters;
	int LocalNetwork;
	int DisconnectMode; // not configurable
	int HiddenSharesAsHidden;
	int FullPathShares;
	int FavoritesFlags; // not configurable
	int RootDoublePoint;
	int NavigateToDomains; // not configurable
	int ScanNetwork;

	static void Read();
	static void Write();
} Opt;


extern NETRESOURCE CommonCurResource;
extern LPNETRESOURCE PCommonCurResource;

const wchar_t* GetMsg(int MsgId);

class TSaveScreen
{
private:
	HANDLE hScreen;

public:
	TSaveScreen();
	~TSaveScreen();
};

BOOL DlgCreateFolder(wchar_t* lpBuffer, size_t nBufferSize);

#endif // __NETCOMMON_HPP__
