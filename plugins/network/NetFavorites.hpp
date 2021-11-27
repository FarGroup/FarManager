#ifndef __NETFAVORITES_HPP__
#define __NETFAVORITES_HPP__

#include <windows.h>
#include "NetClass.hpp"

struct FAVORITEITEM
{
	wchar_t* lpRemoteName;
	int ccRemoteName;
	wchar_t* lpUserName;
	int ccUserName;
	wchar_t* lpPassword;
	int ccPassword;
};

#define FAVORITES_UPBROWSE_TO_FAVORITES 0x01L
#define FAVORITES_CHECK_RESOURCES       0x02L

#define FAVORITES_DEFAULTS FAVORITES_UPBROWSE_TO_FAVORITES|FAVORITES_CHECK_RESOURCES

BOOL GetFavorites(LPNETRESOURCE pNR, NetResourceList* pList);
BOOL CheckFavoriteItem(const LPNETRESOURCE pNR);
void WriteFavoriteItem(FAVORITEITEM* lpFavItem, wchar_t* szFolderName = {});
BOOL ReadFavoriteItem(FAVORITEITEM* lpFavItem);
BOOL GetFavoritesParent(NETRESOURCE& SrcRes, LPNETRESOURCE lpParent);
BOOL GetFavoriteResource(const wchar_t* SrcName, LPNETRESOURCE DstNetResource);

typedef DWORD (WINAPI * LPREMOVEFROMFAVCB)(wchar_t* SrcName, LPVOID pUserData);

BOOL RemoveFromFavorites(wchar_t* SrcName, LPREMOVEFROMFAVCB pUserCallBack, LPVOID pUserData);
BOOL ValidatePath(const wchar_t* szPath);
BOOL CreateSubFolder(wchar_t* szRoot, wchar_t* szSubFolder);


#endif //__NETFAVORITES_HPP__
