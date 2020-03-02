#ifndef __FARFMT_HPP__
#define __FARFMT_HPP__
/*
  FMT.HPP

  Archive Support API for FAR Manager 1.70 and MultiArc plugin

  Copyright (c) 1996-2000 Eugene Roshal
  Copyrigth (c) 2000-<%YEAR%> FAR group
*/

/* Revision: 1.10 06.05.2003 $ */

#if defined(__BORLANDC__)
  #pragma option -a2
#elif defined(__GNUC__) || (defined(__WATCOMC__) && (__WATCOMC__ < 1100)) || defined(__LCC__)
  #pragma pack(2)
  #if defined(__LCC__)
    #define _export __declspec(dllexport)
  #endif
#else
  #pragma pack(push,2)
  #if _MSC_VER
    #define _export
  #endif
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


#if defined(__BORLANDC__) || defined(_MSC_VER) || defined(__GNUC__) || defined(__WATCOMC__)
#ifdef __cplusplus
extern "C"{
#endif

DWORD WINAPI _export LoadFormatModule(const char *ModuleName);
void  WINAPI _export SetFarInfo(const struct PluginStartupInfo *Info);

BOOL  WINAPI _export IsArchive(const char *Name,const unsigned char *Data,int DataSize);
DWORD WINAPI _export GetSFXPos(void);
BOOL  WINAPI _export OpenArchive(const char *Name,int *TypeArc);
int   WINAPI _export GetArcItem(struct PluginPanelItem *Item,struct ArcItemInfo *Info);
BOOL  WINAPI _export CloseArchive(struct ArcInfo *Info);
BOOL  WINAPI _export GetFormatName(int TypeArc,char *FormatName,char *DefaultExt);
BOOL  WINAPI _export GetDefaultCommands(int TypeArc,int Command,char *Dest);

#ifdef __cplusplus
};
#endif
#endif

#if defined(__BORLANDC__)
  #pragma option -a.
#elif defined(__GNUC__) || (defined(__WATCOMC__) && (__WATCOMC__ < 1100)) || defined(__LCC__)
  #pragma pack()
#else
  #pragma pack(pop)
#endif

#endif /* __FARFMT_HPP__ */
