#ifndef FARFMT_HPP
#define FARFMT_HPP
/*
  FMT.HPP

  Archive Support API for FAR Manager 1.75 and MultiArc plugin

  Copyright (c) 1996-2000 Eugene Roshal
  Copyrigth (c) 2000-2008 FAR group
*/

#ifdef __GNUC__
#define STR_PRAGMA(x) _Pragma(#x)
#endif

#ifdef __GNUC__
#define PACK_PUSH(n) STR_PRAGMA(pack(push ,n))
#define PACK_POP() STR_PRAGMA(pack(pop))
#else
#define PACK_PUSH(n) __pragma(pack(push, n))
#define PACK_POP() __pragma(pack(pop))
#endif

#ifdef __cplusplus
#define PACK_CHECK(type, n) static_assert(alignof(type) == n, "Wrong alignment")
#endif

#if defined(_MSC_VER) && _MSC_VER < 1800
#undef PACK_CHECK
#define PACK_CHECK(t,n)
#endif

#if !defined(_WIN64)
PACK_PUSH(2)
#endif

enum GETARC_CODE
{
  GETARC_EOF               =0,
  GETARC_SUCCESS           =1,
  GETARC_BROKEN            =2,
  GETARC_UNEXPEOF          =3,
  GETARC_READERROR         =4,
};

struct ArcItemInfo
{
  char HostOS[32];
  char Description[256];
  int Solid;
  int Comment;
  int Encrypted;
  int DictSize;
  int UnpVer;
  int Chapter;
};

enum ARCINFO_FLAGS
{
  AF_AVPRESENT      =0x00000001,
  AF_IGNOREERRORS   =0x00000002,
  AF_HDRENCRYPTED   =0x00000080,
};

struct ArcInfo
{
  int SFXSize;
  int Volume;
  int Comment;
  int Recovery;
  int Lock;
  DWORD Flags;
  DWORD Reserved;
  int Chapters;
};


#ifdef __cplusplus
extern "C"{
#endif

DWORD WINAPI LoadFormatModule(const char *ModuleName);
void  WINAPI SetFarInfo(const struct PluginStartupInfo *Info);

BOOL  WINAPI IsArchive(const char *Name,const unsigned char *Data,int DataSize);
DWORD WINAPI GetSFXPos(void);
BOOL  WINAPI OpenArchive(const char *Name,int *TypeArc);
int   WINAPI GetArcItem(struct PluginPanelItem *Item,struct ArcItemInfo *Info);
BOOL  WINAPI CloseArchive(struct ArcInfo *Info);
BOOL  WINAPI GetFormatName(int TypeArc,char *FormatName,char *DefaultExt);
BOOL  WINAPI GetDefaultCommands(int TypeArc,int Command,char *Dest);

#ifdef __cplusplus
};
#endif

#if !defined(_WIN64)
PACK_POP()
#endif

#endif // FARFMT_HPP
