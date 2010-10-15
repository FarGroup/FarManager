#ifndef __NETCFG_HPP__
#define __NETCFG_HPP__
#include <CRT/crt.hpp>

extern const TCHAR *StrAddToDisksMenu;
extern const TCHAR *StrAddToPluginsMenu;
#ifndef UNICODE
extern const TCHAR *StrDisksMenuDigit;
#endif
extern const TCHAR *StrHelpNetBrowse;
extern const TCHAR *StrNTHiddenShare;
extern const TCHAR *StrShowPrinters;
extern const TCHAR *StrLocalNetwork;
extern const TCHAR *StrDisconnectMode;
extern const TCHAR *StrRemoveConnection;
extern const TCHAR *StrHiddenSharesAsHidden;
extern const TCHAR *StrFullPathShares;
extern const TCHAR *StrFavoritesFlags;
extern const TCHAR *StrNoRootDoublePoint;
extern const TCHAR *StrNavigateToDomains;
extern const TCHAR *StrPanelMode;

int Config(void);

#endif // __NETCFG_HPP__
