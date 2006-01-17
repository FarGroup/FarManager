#ifndef __NETCLASS_HPP__
#define __NETCLASS_HPP__

#include "plugin.hpp"
#ifdef NETWORK_LOGGING
#include <stdio.h>
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
  static char *CopyText (const char *Text);

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

class NetBrowser
{
  private:
	  void RemoveItems();
#ifdef NETWORK_LOGGING
	void LogData(char * Data);
#endif
	static void DisconnectFromServer(NETRESOURCE *nr);
    BOOL ChangeToDirectory (const char *Dir, int IsFind, int IsExplicit);
    void ManualConnect();
    BOOL CancelConnection (char *RemoteName);
    BOOL GetDriveToDisconnect (const char *RemoteName, char *LocalName);
    BOOL ConfirmCancelConnection (char *LocalName, char *RemoteName, int &UpdateProfile);
    BOOL NeedConfirmCancelConnection();
    BOOL HandsOffDisconnectDrive (const char *LocalName);
    void GetLocalName(char *RemoteName,char *LocalName);
    static int GetNameAndPassword(char *Title,char *Name,char *Password,BOOL *pRemember=NULL);
    void GetRemoteName(NETRESOURCE *NetRes,char *RemoteName);
    BOOL EnumerateNetList();
    void GetHideShareNT();
    void GetHideShare95();
    void GetFreeLetter(DWORD &DriveMask,char *DiskName);
    BOOL IsReadable(const char *Remote);
    int GotoComputer (const char *Dir);
    void SetCursorToShare (char *Share);
    BOOL MapNetworkDrive (char *RemoteName, BOOL AskDrive, BOOL Permanent);
    BOOL AskMapDrive (char *NewLocalName, BOOL &Permanent);
    void PutCurrentFileName (BOOL ToCommandLine);
    NetResourceList NetList;               // list of resources in the current folder
    char NetListRemoteName [NM];           // remote name of the resource stored in NetList
    NetResourceList ConnectedList;         // list of resources mapped to local drives
    NetResourceList RootResources;         // stack of resources above the current level
                                           // (used in non-MS Windows networks only)
    NETRESOURCE CurResource;               // NETRESOURCE describing the current location
    NETRESOURCE *PCurResource;             // points to CurResource or NULL (if at root)

    BOOL ChangeDirSuccess;
    BOOL OpenFromFilePanel;
    int ReenterGetFindData;
    char CmdLinePath [NM];                 // path passed when invoking us from command line
    char PanelMode[4];                     // current start panel mode

#ifdef NETWORK_LOGGING
    static FILE *LogFile;
	static int LogFileRef;
    static void LogNetResource (NETRESOURCE &Res);
	static void OpenLogFile(char *lpFileName);
	static void CloseLogfile();
#endif

  public:
	  void CreateFavSubFolder();
    NetBrowser();
    ~NetBrowser();
    int GetFindData(PluginPanelItem **pPanelItem,int *pItemsNumber,int OpMode);
    void FreeFindData(PluginPanelItem *PanelItem,int ItemsNumber);
    void GetOpenPluginInfo(struct OpenPluginInfo *Info);
    int SetDirectory(const char *Dir,int OpMode);
    int DeleteFiles(struct PluginPanelItem *PanelItem,int ItemsNumber,int OpMode);
    int ProcessKey(int Key,unsigned int ControlState);
    int ProcessEvent(int Event, void *Param);
    void SetOpenFromCommandLine (char *ShareName);
    BOOL SetOpenFromFilePanel (char *ShareName);
    void GotoLocalNetwork();

	BOOL GotoFavorite(char *lpPath);
	BOOL EditFavorites();

	static int AddConnection(NETRESOURCE *nr,int Remember=TRUE);
    static int AddConnectionExplicit(NETRESOURCE *nr,int Remember=TRUE);
	static int AddConnectionWithLogon(NETRESOURCE *nr, char *Name, char *Password, int Remember=TRUE);
	static int AddConnectionFromFavorites(NETRESOURCE *nr,int Remember=TRUE);

	static BOOL GetResourceInfo (char *SrcName,LPNETRESOURCE DstNetResource);
    static BOOL GetResourceParent (NETRESOURCE &SrcRes, LPNETRESOURCE DstNetResource);
    static BOOL IsMSNetResource (const NETRESOURCE &Res);
    static BOOL IsResourceReadable (NETRESOURCE &Res);
	//static BOOL GetDfsParent(const NETRESOURCE &SrcRes, NETRESOURCE &Parent);
};

extern NetResourceList CommonRootResources;
extern BOOL SavedCommonRootResources;

#endif // __NETCLASS_HPP__