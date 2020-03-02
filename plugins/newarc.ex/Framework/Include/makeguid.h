#pragma once
#include "windows.h"
#include "UnicodeAnsi.hpp"

GUID CreatePluginUID(const GUID& ModuleUID, const TCHAR* lpFileName);
GUID CreateFormatUID(const GUID& PluginUID, const TCHAR* lpFormatName);
GUID CreateFormatUID(const GUID& PluginUID, int nModuleNumber);

unsigned long CRC32(unsigned long crc, const char *buf, unsigned int len);
