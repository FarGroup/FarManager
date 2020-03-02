#pragma once
#include "FarPluginBase.hpp"
#include "StringBase.hpp"

void AddEndSlash(string& str);
void CutToSlash(string &strStr, bool bInclude = false);
void CutTo(string& str, TCHAR symbol, bool bInclude);
void ConvertSlashes(string& strStr);

void Quote(string& strStr);




