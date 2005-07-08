class NetResourceList
{
private:
  NETRESOURCE *ResList;
  unsigned int ResCount;

  static char *CopyText (const char *Text);

public:
  NetResourceList();
  ~NetResourceList();
  NetResourceList &operator= (NetResourceList &other);

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
#ifdef NETWORK_LOGGING
	void LogData(char * Data);
#endif
    int AddConnection(NETRESOURCE *nr,int Remember=TRUE);
    int AddConnectionExplicit(NETRESOURCE *nr,int Remember=TRUE);
	void DisconnectFromServer(NETRESOURCE *nr);
    BOOL ChangeToDirectory (const char *Dir, int IsFind, int IsExplicit);
    void ManualConnect();
    BOOL CancelConnection (char *RemoteName);
    BOOL GetDriveToDisconnect (const char *RemoteName, char *LocalName);
    BOOL ConfirmCancelConnection (char *LocalName, char *RemoteName, int &UpdateProfile);
    BOOL NeedConfirmCancelConnection();
    BOOL HandsOffDisconnectDrive (const char *LocalName);
    void GetLocalName(char *RemoteName,char *LocalName);
    int GetNameAndPassword(char *Title,char *Name,char *Password);
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
    BOOL GetResourceInfo (char *SrcName,LPNETRESOURCE DstNetResource);
    BOOL GetResourceParent (NETRESOURCE &SrcRes, LPNETRESOURCE DstNetResource);
    BOOL IsMSNetResource (const NETRESOURCE &Res);
    BOOL IsResourceReadable (NETRESOURCE &Res);
	BOOL GetDfsParent(const NETRESOURCE &SrcRes, NETRESOURCE &Parent);
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

#ifdef NETWORK_LOGGING
    FILE *LogFile;
    void LogNetResource (NETRESOURCE &Res);
#endif

  public:
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
};

class TSaveScreen{
  private:
    HANDLE hScreen;
//    struct PluginStartupInfo *Info;

  public:
    //TSaveScreen(struct PluginStartupInfo *Info);
    TSaveScreen();
   ~TSaveScreen();
};


struct InitDialogItem
{
  unsigned char Type;
  unsigned char X1,Y1,X2,Y2;
  unsigned char Focus;
  unsigned int Selected;
  unsigned int Flags;
  unsigned char DefaultButton;
  char *Data;
};


struct Options
{
  int AddToDisksMenu;
  int AddToPluginsMenu;
  int DisksMenuDigit;
  int NTGetHideShare;
  BOOL LocalNetwork;
  BOOL DisconnectMode;
  BOOL ConfirmRemoveConnection;
  BOOL HiddenSharesAsHidden;
} Opt;

static struct PluginStartupInfo Info;
struct FarStandardFunctions FSF;
static NETRESOURCE CommonCurResource;
static NETRESOURCE *PCommonCurResource = NULL;
static NetResourceList CommonRootResources;
static BOOL SavedCommonRootResources = FALSE;
static BOOL IsFirstRun = TRUE;
OSVERSIONINFO WinVer;

char PluginRootKey[80];
char FarRootKey [NM];

char *GetMsg(int MsgId);
void InitDialogItems(struct InitDialogItem *Init,struct FarDialogItem *Item,
                     int ItemsNumber);
int Config();

void SetRegKey(HKEY hRoot,const char *Key,const char *ValueName,char *ValueData);
void SetRegKey(HKEY hRoot,const char *Key,const char *ValueName,DWORD ValueData);
void SetRegKey(HKEY hRoot,const char *Key,const char *ValueName,BYTE *ValueData,DWORD ValueSize);
int GetRegKey(HKEY hRoot,const char *Key,const char *ValueName,char *ValueData,char *Default,DWORD DataSize);
int GetRegKey(HKEY hRoot,const char *Key,const char *ValueName,int &ValueData,DWORD Default);
int GetRegKey(HKEY hRoot,const char *Key,const char *ValueName,DWORD Default);
int GetRegKey(HKEY hRoot,const char *Key,const char *ValueName,BYTE *ValueData,BYTE *Default,DWORD DataSize);

const char *StrAddToDisksMenu="AddToDisksMenu";
const char *StrAddToPluginsMenu="AddToPluginsMenu";
const char *StrDisksMenuDigit="DisksMenuDigit";
const char *StrHelpNetBrowse="Contents";
const char *StrNTHiddenShare="NTHiddenShare";
const char *StrLocalNetwork="LocalNetwork";
const char *StrDisconnectMode="DisconnectMode";
const char *StrRemoveConnection="RemoveConnection";
const char *StrHiddenSharesAsHidden="HiddenSharesAsHidden";

HMODULE hMpr32=NULL;
HMODULE hNetApi=NULL;
HMODULE hSvrApi=NULL;

typedef DWORD (APIENTRY *PWNetGetResourceInformation) (
                                                       LPNETRESOURCE lpNetResource,
                                                       LPVOID lpBuffer,
                                                       LPDWORD cbBuffer,
                                                       LPTSTR *lplpSystem);

typedef DWORD (APIENTRY *PWNetGetResourceParent)(
                                                 LPNETRESOURCEA lpNetResource,
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

typedef API_RET_TYPE (APIENTRY *PNetShareEnum95)(const char FAR *     pszServer,
                                                 short                sLevel,
                                                 char FAR *           pbBuffer,
                                                 unsigned short       cbBuffer,
                                                 unsigned short FAR * pcEntriesRead,
                                                 unsigned short FAR * pcTotalAvail );

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
  char		shi1_netname[LM20_NNLEN+1];
  char		shi1_pad1;
  unsigned short	shi1_type;
  char FAR *		shi1_remark;
};  /* share_info_1 */

static PWNetGetResourceInformation FWNetGetResourceInformation=NULL;
static PNetApiBufferFree FNetApiBufferFree=NULL;
static PNetShareEnum FNetShareEnum=NULL;
static PNetShareEnum95 FNetShareEnum95 = NULL;
static PWNetGetResourceParent FWNetGetResourceParent=NULL;
static PNetDfsGetInfo FNetDfsGetInfo;
static BOOL UsedNetFunctions=FALSE;
static void InitializeNetFunction(void);
