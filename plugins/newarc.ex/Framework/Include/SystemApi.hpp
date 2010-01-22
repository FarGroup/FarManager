#pragma once
#include "StringBase.hpp"

DWORD apiExpandEnvironmentStrings(const TCHAR* src, string& strDest);

DWORD apiGetEnvironmentVariable(const TCHAR* lpwszName, string& strBuffer);

string& apiRegQueryStringValue(HKEY hKey, const TCHAR *lpValueName, string& strDefaultValue);

DWORD apiGetFullPathName(const TCHAR* lpFileName, string& strResult);

void apiCreateDirectoryEx(const TCHAR* lpDirectory);
