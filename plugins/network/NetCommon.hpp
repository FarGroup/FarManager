#ifndef __NETCOMMON_HPP__
#define __NETCOMMON_HPP__

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4121)
#endif // _MSC_VER

#include <windows.h>
#include <lm.h>
#include <CRT/crt.hpp>
#include <plugin.hpp>
#include "netlng.hpp"
#include "NetMacros.hpp"

#ifndef UNICODE
#define GetCheck(i) DialogItems[i].Selected
#define GetDataPtr(i) DialogItems[i].Data
#else
#define GetCheck(i) (int)Info.SendDlgMessage(hDlg,DM_GETCHECK,i,0)
#define GetDataPtr(i) ((const TCHAR *)Info.SendDlgMessage(hDlg,DM_GETCONSTTEXTPTR,i,0))
#endif

struct InitDialogItem
{
  unsigned char Type;
  unsigned char X1,Y1,X2,Y2;
  unsigned char Focus;
  DWORD_PTR Selected;
  unsigned int Flags;
  unsigned char DefaultButton;
  const TCHAR *Data;
};

extern struct Options
{
  int AddToDisksMenu;
  int AddToPluginsMenu;
  int DisksMenuDigit;
  int NTGetHideShare;
  BOOL ShowPrinters;
  BOOL LocalNetwork;
  BOOL DisconnectMode;
  BOOL ConfirmRemoveConnection;
  BOOL HiddenSharesAsHidden;
  BOOL FullPathShares;
  int FavoritesFlags;
  BOOL NoRootDoublePoint;
  BOOL NavigateToDomains;
} Opt;

extern BOOL IsOldFAR;

extern struct PluginStartupInfo Info;
extern struct FarStandardFunctions FSF;
extern NETRESOURCE CommonCurResource;
extern LPNETRESOURCE PCommonCurResource;
extern BOOL IsFirstRun;
extern OSVERSIONINFO WinVer;

extern TCHAR PluginRootKey[80];
extern TCHAR FarRootKey [NM];

class TSaveScreen{
  private:
    HANDLE hScreen;
//    struct PluginStartupInfo *Info;

  public:
    //TSaveScreen(struct PluginStartupInfo *Info);
    TSaveScreen();
   ~TSaveScreen();
};

TCHAR *GetMsg(int MsgId);
void InitDialogItems(struct InitDialogItem *Init,struct FarDialogItem *Item,
                     int ItemsNumber);

BOOL DlgCreateFolder(TCHAR* lpBuffer, int nBufferSize);

typedef DWORD (APIENTRY *PWNetGetResourceInformation) (
                                                       LPNETRESOURCE lpNetResource,
                                                       LPVOID lpBuffer,
                                                       LPDWORD cbBuffer,
                                                       LPTSTR *lplpSystem);

typedef DWORD (APIENTRY *PWNetGetResourceParent)(
                                                 LPNETRESOURCE lpNetResource,
                                                 LPVOID lpBuffer,
                                                 LPDWORD lpcbBuffer
                                                 );

typedef NET_API_STATUS (NET_API_FUNCTION *PNetShareEnum)(
                                                         LPWSTR servername,
                                                         DWORD level,
                                                         LPBYTE *bufptr,
                                                         DWORD prefmaxlen,
                                                         LPDWORD entriesread,
                                                         LPDWORD totalentries,
                                                         LPDWORD resume_handle);

typedef NET_API_STATUS (NET_API_FUNCTION *PNetApiBufferFree)(LPVOID Buffer);

typedef API_RET_TYPE (APIENTRY *PNetShareEnum95)(const TCHAR *    pszServer,
                                                 short            sLevel,
                                                 TCHAR *          pbBuffer,
                                                 unsigned short   cbBuffer,
                                                 unsigned short * pcEntriesRead,
                                                 unsigned short * pcTotalAvail );

typedef NET_API_STATUS (NET_API_FUNCTION *PNetDfsGetInfo)(
    IN  LPWSTR  DfsEntryPath,       // DFS entry path for the volume
    IN  LPWSTR  ServerName OPTIONAL,// Name of server hosting a storage
    IN  LPWSTR  ShareName OPTIONAL, // Name of share on server serving the volume
    IN  DWORD   Level,              // Level of information requested
    OUT LPBYTE* Buffer              // API allocates and returns buffer with requested info
);

typedef struct _DFS_STORAGE_INFO {
    ULONG   State;                  // State of this storage, one of DFS_STORAGE_STATE_*
                                    // possibly OR'd with DFS_STORAGE_STATE_ACTIVE
    LPWSTR  ServerName;             // Name of server hosting this storage
    LPWSTR  ShareName;              // Name of share hosting this storage
} DFS_STORAGE_INFO, *PDFS_STORAGE_INFO, *LPDFS_STORAGE_INFO;

typedef struct _DFS_INFO_3 {
    LPWSTR  EntryPath;              // Dfs name for the top of this volume
    LPWSTR  Comment;                // Comment for this volume
    DWORD   State;                  // State of this volume, one of DFS_VOLUME_STATE_*
    DWORD   NumberOfStorages;       // Number of storage servers for this volume
    LPDFS_STORAGE_INFO   Storage;   // An array (of NumberOfStorages elements) of storage-specific information.
} DFS_INFO_3, *PDFS_INFO_3, *LPDFS_INFO_3;

struct share_info_1 {
  TCHAR   shi1_netname[LM20_NNLEN+1];
  TCHAR   shi1_pad1;
  unsigned short  shi1_type;
  TCHAR * shi1_remark;
};  /* share_info_1 */

extern PWNetGetResourceInformation FWNetGetResourceInformation;
extern PNetApiBufferFree FNetApiBufferFree;
extern PNetShareEnum FNetShareEnum;
extern PNetShareEnum95 FNetShareEnum95;
extern PWNetGetResourceParent FWNetGetResourceParent;
extern PNetDfsGetInfo FNetDfsGetInfo;
extern BOOL UsedNetFunctions;

void InitializeNetFunction(void);
void DeinitializeNetFunctions(void);

#define ShowMessage(x) Info.Message(Info.ModuleNumber, FMSG_ALLINONE|FMSG_MB_OK, _T(""), (const TCHAR * const *) x, 0,0)
/* NO NEED THIS
char* NextToken(char *szSource, char *szToken, int nBuff);
*/

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif // __NETCOMMON_HPP__
