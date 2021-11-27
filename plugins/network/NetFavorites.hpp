#ifndef __NETFAVORITES_HPP__
#define __NETFAVORITES_HPP__

#include <windows.h>
#include "NetClass.hpp"

struct FAVORITEITEM
{
	wchar_t* lpRemoteName;
	wchar_t* lpUserName;
	wchar_t* lpPassword;
};

#define FAVORITES_UPBROWSE_TO_FAVORITES 0x01L
#define FAVORITES_CHECK_RESOURCES       0x02L

#define FAVORITES_DEFAULTS (FAVORITES_UPBROWSE_TO_FAVORITES|FAVORITES_CHECK_RESOURCES)

bool EnumFavorites(LPNETRESOURCE pNR, NetResourceList* pList);
bool CheckFavoriteItem(LPNETRESOURCE pNR);
bool InFavoriteExists(const wchar_t *lpRemoteName);
void WriteFavoriteItem(FAVORITEITEM* lpFavItem);
bool ReadFavoriteItem(FAVORITEITEM* lpFavItem);
bool GetFavoritesParent(NETRESOURCE& SrcRes, LPNETRESOURCE lpParent);
bool GetFavoriteResource(const wchar_t* SrcName, LPNETRESOURCE DstNetResource);
bool RemoveFromFavorites(const wchar_t *SrcName);

#endif //__NETFAVORITES_HPP__
