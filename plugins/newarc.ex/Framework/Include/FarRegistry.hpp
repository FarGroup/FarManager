#pragma once
#include "FarPluginBase.hpp"

extern int  RegQueryStringArray (HKEY hKey, const TCHAR *lpPrefix, TCHAR ***Strings);
extern void RegSaveStringArray (HKEY hKey, const TCHAR *lpPrefix, TCHAR **Strings, int nStringCount);
extern void RegQueryLongValue (HKEY hKey, const TCHAR *lpValueName, void *pValueData);
extern TCHAR *RegQueryStringValueEx (HKEY hKey, const TCHAR *lpValueName,	TCHAR *lpCurrentValue = NULL);

#define RegSetStringValue(key, name, data) \
	if ( data ) RegSetValueEx (key, name, 0, REG_SZ, (PBYTE)data, (DWORD)((_tcslen(data)+1)*sizeof(TCHAR)));

#define RegSetLongValue(key, name, data) \
	RegSetValueEx (key, name, 0, REG_DWORD, (PBYTE)&data, 4);
