#pragma once
#include <FarPluginBase.h>

struct FarVersion {
     union {
          struct {
               BYTE MinorVersion;
               BYTE MajorVersion;
               WORD Build;
          };
          DWORD Version;
     };
};

#define FarGetHWND() (HWND)Info.AdvControl (Info.ModuleNumber, ACTL_GETFARHWND, NULL)
#define FarGetVersion() Info.AdvControl (Info.ModuleNumber, ACTL_GETFARVERSION, NULL)
#define FarGetColor(Index) Info.AdvControl (Info.ModuleNumber, ACTL_GETCOLOR, (void*)Index)
//extern int FarGetState ();
