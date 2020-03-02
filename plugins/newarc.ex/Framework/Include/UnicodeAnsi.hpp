#pragma once
#include <windows.h>

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


wchar_t* AnsiToUnicode(const char* lpSrc, int CodePage = CP_OEMCP);
char* UnicodeToAnsi(const wchar_t* lpSrc, int CodePage = CP_OEMCP);

char* UnicodeToUTF8(const wchar_t* lpSrc);
char* AnsiToUTF8(const char* lpSrc, int CodePage = CP_OEMCP);

wchar_t* UTF8ToUnicode(const char* lpSrc);
char* UTF8ToAnsi(const char* lpSrc, int CodePage = CP_OEMCP);

