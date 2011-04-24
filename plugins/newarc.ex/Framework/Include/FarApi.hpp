#pragma once
#include "FarPluginBase.hpp"
#include "StringBase.hpp"

DWORD farGetFullPathName(const TCHAR* lpFileName, string& strResult);

void farUnquote(string& strStr);

void farQuoteSpaceOnly(string& strStr);

void farTrim(string& strStr);

void farPrepareFileName(string& strFileName);

void farTruncPathStr(string& strFileName, int nLength);
