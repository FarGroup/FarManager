#pragma once
#include <FarPluginBase.hpp>
#include <Rtl.Base.h>

extern int  RegQueryStringArray (HKEY hKey, const char *lpPrefix, char ***Strings);
extern void RegSaveStringArray (HKEY hKey, const char *lpPrefix, char **Strings, int nStringCount);
extern void RegQueryLongValue (HKEY hKey, const char *lpValueName, void *pValueData);
extern char *RegQueryStringValueEx (HKEY hKey, const char *lpValueName,	char *lpCurrentValue = NULL);

#define RegSetStringValue(key, name, data) \
	if ( data ) RegSetValueEx (key, name, 0, REG_SZ, (PBYTE)data, (DWORD)strlen(data)+1);

#define RegSetLongValue(key, name, data) \
	RegSetValueEx (key, name, 0, REG_DWORD, (PBYTE)&data, 4);
