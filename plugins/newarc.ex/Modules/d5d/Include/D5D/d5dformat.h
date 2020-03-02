#pragma once

struct ShortString {
	unsigned char Length;
	char Data[255];
};

struct CurrentDriverInfo {
	ShortString Sch;
	ShortString ID;
	HANDLE FileHandle; //HANDLE or Integer??
	bool ExtractInternal;
};

struct FormatInfo {
	ShortString Extensions;
	ShortString Name;
};

struct DriverInfo {
	ShortString Name;
	ShortString Author;
	ShortString Version;
	ShortString Comment;
	unsigned char NumFormats;
	FormatInfo Formats[255];
};

struct ErrorInfo {
	ShortString Format;
	ShortString Games;
};

struct FormatEntry {
	ShortString FileName;
	__int64 Offset;
	__int64 Size;
	int DataX;
	int DataY;
};

/*
typedef void (__stdcall *PERCENTCALLBACK)(unsigned char p);
typedef ShortString (__stdcall *LANGUAGECALLBACK)(ShortString x);
*/

//эти callback'и не stdcall, нет, идиоты. реальные прототипы выше.
typedef void (*PERCENTCALLBACK)();
typedef void (*LANGUAGECALLBACK)();
typedef void (*MESSAGEBOXCALLBACK)();

	
typedef unsigned char (__stdcall* DUDIVERSION)();
typedef unsigned int (__stdcall* GETNUMVERSION)();
typedef void (__stdcall* GETDRIVERINFO)(DriverInfo* info);
typedef bool (__stdcall* ISFORMAT)(ShortString& fil, bool Deeper);
typedef void (__stdcall* CLOSEFORMAT)();
typedef void (__stdcall* GETCURRENTDRIVERINFO)(CurrentDriverInfo* info);
typedef void (__stdcall* GETENTRY)(FormatEntry* entry);
typedef void (__stdcall* GETERRORINFO)(ErrorInfo* info);

typedef unsigned char (__stdcall* DUDIVERSIONEX)(unsigned char supported);
typedef bool (__stdcall *EXTRACTFILETOSTREAM)(void* outstream, ShortString& EntryNam, __int64 Offset, __int64 Size, int DataX, int DataY, bool bSilent);
typedef void (__stdcall *INITPLUGINEX5)(MESSAGEBOXCALLBACK MsgBox);

typedef int (__stdcall* INITPLUGIN)(PERCENTCALLBACK per, LANGUAGECALLBACK lngid, ShortString& DUP5Path);
typedef int (__stdcall* INITPLUGIN3)(PERCENTCALLBACK per, LANGUAGECALLBACK lngid, ShortString& DUP5Path, HANDLE AppHandle, void* AppOwner);

typedef bool (__stdcall* EXTRACTFILE)(ShortString& OutPutFile, ShortString& EntryNam, __int64 Offset, __int64 Size, int DataX, int DataY);
typedef bool (__stdcall* EXTRACTFILE2)(ShortString& OutPutFile, ShortString& EntryNam, __int64 Offset, __int64 Size, int DataX, int DataY, bool bSilent);

typedef int (__stdcall* READFORMAT)(ShortString& fil, PERCENTCALLBACK per, bool Deeper);
typedef int (__stdcall* READFORMAT2)(ShortString& fil, bool Deeper);

typedef int (__stdcall* ABOUTBOX)(HWND hwnd, LANGUAGECALLBACK lngstr);
typedef int (__stdcall* ABOUTBOX2)(HWND hwnd);
typedef int (__stdcall* ABOUTBOX3)();

typedef int (__stdcall* CONFIGUREBOX)(HWND hwnd, LANGUAGECALLBACK lngstr);
typedef int (__stdcall* CONFIGUREBOX2)(HWND hwnd);
typedef int (__stdcall* CONFIGUREBOX3)();
