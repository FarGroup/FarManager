#pragma once

#define _CRT_SECURE_NO_WARNINGS

#ifdef _WINDOWS_ //windows.h already included, align problems!
	#pragma message("WINDOWS.H already included, that's bad!")
#endif

#ifdef UNICODE
 
#include "../../../common/unicode/plugin.hpp"
#include "../../../common/unicode/farkeys.hpp"
#include "../../../common/unicode/farcolor.hpp"

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
#include "../../../common/ascii/plugin.hpp"
#include "../../../common/ascii/farkeys.hpp"
#include "../../../common/ascii/farcolor.hpp"
};

#else

#include "../../../common/ascii/plugin.hpp"
#include "../../../common/ascii/farkeys.hpp"
#include "../../../common/ascii/farcolor.hpp"

#undef __PLUGIN_HPP__
#undef __FARKEYS_HPP__
#undef __FARCOLOR_HPP__

#undef FARMANAGERVERSION_MAJOR
#undef FARMANAGERVERSION_MINOR
#undef FARMANAGERVERSION_BUILD

namespace oldfar {
#include "../../../common/ascii/plugin.hpp"
#include "../../../common/ascii/farkeys.hpp"
#include "../../../common/ascii/farcolor.hpp"
};

#endif

#include <stdio.h>
#include <string.h>
#include <tchar.h>
#include <Rtl.Base.h>

#ifndef UNICODE
	#define EXP_NAME(p) _export p
	#define UNICODE_NAME(p) p
#else
	#define EXP_NAME(p) _export p ## W
	#define UNICODE_NAME(p) p ## W
#endif

#ifdef UNICODE
	#define UNIVERSAL_ANSI_NAME_CREATE(name) \
		wchar_t* name ## W = StrDuplicate(name); \
		char* name ## A = UnicodeToAnsi(name, CP_ACP);

	#define UNIVERSAL_ANSI_NAME_CREATE_EX(name, alias) \
		wchar_t* alias ## W = StrDuplicate(name); \
		char* alias ## A = UnicodeToAnsi(name, CP_ACP);

	#define UNIVERSAL_NAME_CREATE(name) \
		wchar_t* name ## W = StrDuplicate(name); \
		char* name ## A = UnicodeToAnsi(name);

	#define UNIVERSAL_NAME_CREATE_EX(name, alias) \
		wchar_t* alias ## W = StrDuplicate(name); \
		char* alias ## A = UnicodeToAnsi(name);

	#define ANSI_NAME_CREATE(name) \
		char* name ## A = UnicodeToAnsi(name, CP_ACP);

	#define ANSI_NAME_CREATE_EX(name, alias) \
		char* alias ## A = UnicodeToAnsi(name, CP_ACP); 

	#define OEM_NAME_CREATE(name) \
		char* name ## A = UnicodeToAnsi(name, CP_OEMCP);

	#define OEM_NAME_CREATE_EX(name, alias) \
		char* alias ## A = UnicodeToAnsi(name, CP_OEMCP); 

	#define OEM_NAME_CREATE_CONST(name) \
		char* name ## A = UnicodeToAnsi(name, CP_OEMCP);

#else
	#define UNIVERSAL_ANSI_NAME_CREATE(name) \
		wchar_t* name ## W = AnsiToUnicode(name); \
		char* name ## A = StrDuplicate(name); \
		OemToCharA(name ## A, name ## A);

	#define UNIVERSAL_ANSI_NAME_CREATE_EX(name, alias) \
		wchar_t* alias ## W = AnsiToUnicode(name); \
		char* alias ## A = StrDuplicate(name); \
		OemToCharA(alias ## A, alias ## A); 

	#define UNIVERSAL_NAME_CREATE(name) \
		wchar_t* name ## W = AnsiToUnicode(name); \
		char* name ## A = StrDuplicate(name);

	#define UNIVERSAL_NAME_CREATE_EX(name, alias) \
		wchar_t* alias ## W = AnsiToUnicode(name); \
		char* alias ## A = StrDuplicate(name); 

	#define ANSI_NAME_CREATE(name) \
		char* name ## A = StrDuplicate(name); \
		OemToCharA(name ## A, name ##A);

	#define ANSI_NAME_CREATE_EX(name, alias) \
		char* alias ## A = StrDuplicate(name); \
		OemToCharA(alias ## A, alias ## A);

	#define OEM_NAME_CREATE(name) \
		char* name ## A = StrDuplicate(name, CP_OEMCP);

	#define OEM_NAME_CREATE_EX(name, alias) \
		char* alias ## A = StrDuplicate(name, CP_OEMCP); 

	#define OEM_NAME_CREATE_CONST(name) \
		const char* name ## A = name;

#endif

#define UNIVERSAL_NAME_DELETE(name) \
	free(name ## W); \
	free(name ## A);

#define ANSI_NAME_DELETE(name) \
	free(name ## A);

#define OEM_NAME_DELETE(name) \
	free(name ## A);

#ifdef UNICODE
	#define OEM_NAME_DELETE_CONST(name) \
		free(name ##A); 
#else
	#define OEM_NAME_DELETE_CONST(name)
#endif

extern PluginStartupInfo Info;
extern FARSTANDARDFUNCTIONS FSF;

