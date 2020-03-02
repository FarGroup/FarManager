#pragma once
#include "Rtl.Base.h"
#include "StringBase.hpp"
#include "strmix.hpp"

DWORD apiExpandEnvironmentStrings(const TCHAR* src, string& strDest);

DWORD apiGetEnvironmentVariable(const TCHAR* lpwszName, string& strBuffer);

string& apiRegQueryStringValue(HKEY hKey, const TCHAR *lpValueName, string& strDefaultValue);

DWORD apiGetFullPathName(const TCHAR* lpFileName, string& strResult);

void apiCreateDirectoryEx(const TCHAR* lpDirectory);
void apiCreateDirectoryForFile(const TCHAR* lpFileName);

bool apiSetFilePointer(HANDLE hFile, __int64 nDistanceToMove, __int64* pNewFilePointer, DWORD dwMoveMethod);

bool apiGetFileSize(HANDLE hFile, unsigned __int64 *pSize);

bool apiGetFindData(const TCHAR* lpFileName, WIN32_FIND_DATA& fData);
