#pragma once
#include "FarPluginBase.hpp"
#include "StringBase.hpp"

void AddEndSlash(string& str);
void CutToSlash(string &strStr, bool bInclude = false);
void CutTo (string& str, TCHAR symbol, bool bInclude);
void ConvertSlashes(string& strStr);


DWORD apiExpandEnvironmentStrings(const TCHAR* src, string& strDest);
DWORD apiGetEnvironmentVariable(const TCHAR* lpwszName, string& strBuffer);
string& apiRegQueryStringValue(HKEY hKey, const TCHAR *lpValueName, string& strDefaultValue);
DWORD apiGetFullPathName(const TCHAR* lpFileName, string& strResult);

void apiCreateDirectoryEx(const TCHAR* lpDirectory);


wchar_t* AnsiToUnicode(const char* lpSrc, int CodePage = CP_OEMCP);
char* UnicodeToAnsi(const wchar_t* lpSrc, int CodePage = CP_OEMCP);