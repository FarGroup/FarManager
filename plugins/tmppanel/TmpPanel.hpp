/*
TMPPANEL.HPP

Temporary panel header file

*/

#ifndef __TMPPANEL_HPP__
#define __TMPPANEL_HPP__

#include "plugin.hpp"
#include "CRT/crt.hpp"
#include <shellapi.h>

#include "TmpLng.hpp"
#include "TmpClass.hpp"
#include "TmpCfg.hpp"

#define COMMONPANELSNUMBER 10

typedef struct _MyInitDialogItem
{
  unsigned char Type;
  unsigned char X1,Y1,X2,Y2;
  DWORD Flags;
  signed char Data;
} MyInitDialogItem;

typedef struct _PluginPanels
{
  PluginPanelItem *Items;
  unsigned int ItemsNumber;
  unsigned int OpenFrom;
} PluginPanels;

extern PluginPanels CommonPanels[COMMONPANELSNUMBER];

extern unsigned int CurrentCommonPanel;

extern struct PluginStartupInfo Info;
extern struct FarStandardFunctions FSF;

extern int StartupOptFullScreenPanel,StartupOptCommonPanel,StartupOpenFrom;
extern TCHAR PluginRootKey[80];

const TCHAR *GetMsg(int MsgId);
void InitDialogItems(const MyInitDialogItem *Init,struct FarDialogItem *Item,int ItemsNumber);

int Config();
void GoToFile(const TCHAR *Target, BOOL AnotherPanel);
void FreePanelItems(PluginPanelItem *Items, DWORD Total);

TCHAR *ParseParam(TCHAR *& str);
void GetOptions(void);
void WFD2FFD(WIN32_FIND_DATA &wfd, FAR_FIND_DATA &ffd);
#ifdef UNICODE
wchar_t* NtPath(const wchar_t* path, wchar_t* buf);
#else
#define NtPath(path, buf) path
#endif

#ifndef UNICODE
#define ExpandEnvStrs   FSF.ExpandEnvironmentStr
#else
#define ExpandEnvStrs   ExpandEnvironmentStrings
#endif

#ifdef UNICODE
#define BOM_UCS2     0xFEFF
#define BOM_UCS2_BE  0xFFFE
#define BOM_UTF8     0xBFBBEF
#endif

#define NT_MAX_PATH 32768

#endif /* __TMPPANEL_HPP__ */
