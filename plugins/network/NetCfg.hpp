#ifndef __NETCFG_HPP__
#define __NETCFG_HPP__

#include <plugin.hpp>

extern const wchar_t* StrPanelMode;
extern const wchar_t* StrHelpNetBrowse;
extern const wchar_t* StrDisconnectMode;

int Config();
__int64 GetSetting(FARSETTINGS_SUBFOLDERS Root, const wchar_t* Name);

#endif // __NETCFG_HPP__
