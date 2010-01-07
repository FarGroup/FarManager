#ifndef __NETCLASS_HPP__
#define __NETCLASS_HPP__

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4121)
#endif // _MSC_VER

#include <CRT/crt.hpp>
#include <plugin.hpp>
#ifdef NETWORK_LOGGING
#include <stdio.h>
#endif

#ifdef _MSC_VER
#pragma warning(pop)
#endif

// winnt.h
#ifndef FILE_ATTRIBUTE_VIRTUAL
#define FILE_ATTRIBUTE_VIRTUAL 0x00010000
#endif

class NetResourceList
{
private:
  NETRESOURCE *ResList;
  unsigned int ResCount;

public:
  NetResourceList();
  ~NetResourceList();
  NetResourceList &operator= (NetResourceList &other);
  static TCHAR *CopyText (const TCHAR *Text);

  static void InitNetResource (NETRESOURCE &Res);
  static void DeleteNetResource (NETRESOURCE &Res);
  static void CopyNetResource (NETRESOURCE &Dest, const NETRESOURCE &Src);

  void Clear();
  BOOL Enumerate (DWORD dwScope, DWORD dwType, DWORD dwUsage,
    LPNETRESOURCE lpNetResource);

  void Push (NETRESOURCE &Res);
  NETRESOURCE *Top();
  void Pop();

  unsigned int Count()
    { return ResCount; }
  NETRESOURCE &operator[] (int index)
    { return ResList [index]; }
};

typedef struct __NameAndPassInfo
{
    TCHAR* Title;
    TCHAR* Name;
    TCHAR* Password;
    LPBOOL pRemember;
    TCHAR* szFavoritePath;
} NameAndPassInfo;

class NetBrowser
{
  private:
    void RemoveItems();
#ifdef NETWORK_LOGGING
  void LogData(TCHAR* Data);
#endif
  static void DisconnectFromServer(NETRESOURCE *nr);
    BOOL ChangeToDirectory (const TCHAR *Dir, int IsFind, int IsExplicit);
    void ManualConnect();
    BOOL CancelConnection (const TCHAR *RemoteName);
    BOOL GetDriveToDisconnect (const TCHAR *RemoteName, TCHAR *LocalName);
    BOOL ConfirmCancelConnection (TCHAR *LocalName, TCHAR *RemoteName, int &UpdateProfile);
    BOOL NeedConfirmCancelConnection();
    BOOL HandsOffDisconnectDrive (const TCHAR *LocalName);
    void GetLocalName(TCHAR *RemoteName, TCHAR *LocalName);
    static int GetNameAndPassword(NameAndPassInfo* passInfo);
    void GetRemoteName(NETRESOURCE *NetRes, TCHAR *RemoteName);
    BOOL EnumerateNetList();
    void GetHideShareNT();
    void GetHideShare95();
    void GetFreeLetter(DWORD &DriveMask, TCHAR *DiskName);
    BOOL IsReadable(const TCHAR *Remote);
    int GotoComputer (const TCHAR *Dir);
    void SetCursorToShare (TCHAR *Share);
    BOOL MapNetworkDrive (const TCHAR *RemoteName, BOOL AskDrive, BOOL Permanent);
    BOOL AskMapDrive (TCHAR *NewLocalName, BOOL &Permanent);
    void PutCurrentFileName (BOOL ToCommandLine);
    NetResourceList NetList;               // list of resources in the current folder
    TCHAR NetListRemoteName [NM];          // remote name of the resource stored in NetList
    NetResourceList ConnectedList;         // list of resources mapped to local drives
    NetResourceList RootResources;         // stack of resources above the current level
                                           // (used in non-MS Windows networks only)
    NETRESOURCE CurResource;               // NETRESOURCE describing the current location
    NETRESOURCE *PCurResource;             // points to CurResource or NULL (if at root)

    BOOL ChangeDirSuccess;
    BOOL OpenFromFilePanel;
    int ReenterGetFindData;
    TCHAR CmdLinePath [NM];                 // path passed when invoking us from command line
    TCHAR PanelMode[4];                     // current start panel mode

#ifdef NETWORK_LOGGING
    static FILE *LogFile;
  static int LogFileRef;
    static void LogNetResource (NETRESOURCE &Res);
  static void OpenLogFile(TCHAR *lpFileName);
  static void CloseLogfile();
#endif

  public:
    void CreateFavSubFolder();
    NetBrowser();
    ~NetBrowser();
    int GetFindData(PluginPanelItem **pPanelItem,int *pItemsNumber,int OpMode);
    void FreeFindData(PluginPanelItem *PanelItem,int ItemsNumber);
    void GetOpenPluginInfo(struct OpenPluginInfo *Info);
    int SetDirectory(const TCHAR *Dir,int OpMode);
    int DeleteFiles(struct PluginPanelItem *PanelItem,int ItemsNumber,int OpMode);
    int ProcessKey(int Key,unsigned int ControlState);
    int ProcessEvent(int Event, void *Param);
    void SetOpenFromCommandLine (TCHAR *ShareName);
    BOOL SetOpenFromFilePanel (TCHAR *ShareName);
    void GotoLocalNetwork();

  BOOL GotoFavorite(TCHAR *lpPath);
  BOOL EditFavorites();

  static int AddConnection(NETRESOURCE *nr,int Remember=TRUE);
    static int AddConnectionExplicit(NETRESOURCE *nr,int Remember=TRUE);
  static int AddConnectionWithLogon(NETRESOURCE *nr, TCHAR *Name, TCHAR *Password, int Remember=TRUE);
  static int AddConnectionFromFavorites(NETRESOURCE *nr,int Remember=TRUE);

  static BOOL GetResourceInfo (TCHAR *SrcName,LPNETRESOURCE DstNetResource);
    static BOOL GetResourceParent (NETRESOURCE &SrcRes, LPNETRESOURCE DstNetResource);
    static BOOL IsMSNetResource (const NETRESOURCE &Res);
    static BOOL IsResourceReadable (NETRESOURCE &Res);
  //static BOOL GetDfsParent(const NETRESOURCE &SrcRes, NETRESOURCE &Parent);
};

extern NetResourceList *CommonRootResources;
extern BOOL SavedCommonRootResources;

#endif // __NETCLASS_HPP__
