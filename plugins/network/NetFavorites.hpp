#ifndef __NETFAVORITES_HPP__
#define __NETFAVORITES_HPP__

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4121)
#endif

#include <windows.h>

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include "NetClass.hpp"

typedef struct _FAVORITEITEM {
  TCHAR *lpRemoteName;
  int ccRemoteName;
  TCHAR *lpUserName;
  int ccUserName;
  TCHAR *lpPassword;
  int ccPassword;
} FAVORITEITEM, *LPFAVORITEITEM;

#define FAVORITES_UPBROWSE_TO_FAVORITES 0x01L
#define FAVORITES_CHECK_RESOURCES       0x02L

#define FAVORITES_DEFAULTS FAVORITES_UPBROWSE_TO_FAVORITES|FAVORITES_CHECK_RESOURCES

BOOL GetFavorites(LPNETRESOURCE pNR, NetResourceList *pList);
BOOL CheckFavoriteItem(const LPNETRESOURCE pNR);
void WriteFavoriteItem(LPFAVORITEITEM lpFavItem, TCHAR* szFolderName=NULL);
BOOL ReadFavoriteItem(LPFAVORITEITEM lpFavItem);
BOOL GetFavoritesParent(NETRESOURCE& SrcRes, LPNETRESOURCE lpParent);
BOOL GetFavoriteResource(TCHAR *SrcName, LPNETRESOURCE DstNetResource);

typedef DWORD (WINAPI * LPREMOVEFROMFAVCB)(TCHAR *SrcName, LPVOID pUserData);

BOOL RemoveFromFavorites(TCHAR *SrcName, LPREMOVEFROMFAVCB pUserCallBack, LPVOID pUserData);
BOOL ValidatePath(const TCHAR *szPath);
BOOL CreateSubFolder(TCHAR *szRoot, TCHAR *szSubFolder);


#endif //__NETFAVORITES_HPP__
