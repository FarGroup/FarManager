#pragma once

#define _CRT_SECURE_NO_WARNINGS

#ifdef _WINDOWS_ //windows.h already included, align problems!
	#pragma message("WINDOWS.H already included, that's bad!")
#endif

#ifdef UNICODE
 
#ifdef _EXTERNAL
#include "../../common/unicode/plugin.hpp"
#include "../../common/unicode/farkeys.hpp"
#include "../../common/unicode/farcolor.hpp"
#else
#include "../../Far/2.x/plugin.hpp"
#include "../../Far/2.x/farkeys.hpp"
#include "../../Far/2.x/farcolor.hpp"
#endif


#ifndef _EXTERNAL

#define FARMANAGER_MAJOR_SAFE FARMANAGERVERSION_MAJOR
#define FARMANAGER_MINOR_SAFE FARMANAGERVERSION_MINOR
#define FARMANAGER_BUILD_SAFE FARMANAGERVERSION_BUILD

#undef __PLUGIN_HPP__
#undef __FARKEYS_HPP__
#undef __FARCOLOR_HPP__

#undef FARMANAGERVERSION_MAJOR
#undef FARMANAGERVERSION_MINOR
#undef FARMANAGERVERSION_BUILD

namespace oldfar {
#include "../../Far/1.x/plugin.hpp"
#include "../../Far/1.x/farkeys.hpp"
#include "../../Far/1.x/farcolor.hpp"
};
#endif

#else

#ifdef _EXTERNAL
#include "../../common/ascii/plugin.hpp"
#include "../../common/ascii/farkeys.hpp"
#include "../../common/ascii/farcolor.hpp"
#else
#include "../../Far/1.x/plugin.hpp"
#include "../../Far/1.x/farkeys.hpp"
#include "../../Far/1.x/farcolor.hpp"
#endif


#ifndef _EXTERNAL

#undef __PLUGIN_HPP__
#undef __FARKEYS_HPP__
#undef __FARCOLOR_HPP__

#undef FARMANAGERVERSION_MAJOR
#undef FARMANAGERVERSION_MINOR
#undef FARMANAGERVERSION_BUILD

namespace oldfar {
#include "../../Far/1.x/plugin.hpp"
#include "../../Far/1.x/farkeys.hpp"
#include "../../Far/1.x/farcolor.hpp"
};
#endif

#endif

#include <stdio.h>
#include <string.h>
#include <tchar.h>
#include "Rtl.Base.h"
#include "array.hpp"

#include "StringBase.hpp"
#include "UnicodeAnsi.hpp"

extern PluginStartupInfo Info;
extern FARSTANDARDFUNCTIONS FSF;


#include "FarDialog.hpp"
#include "FarMenu.hpp"
#include "FarMessage.hpp"
#include "FarApi.hpp"
#include "FarPanelInfo.hpp"


