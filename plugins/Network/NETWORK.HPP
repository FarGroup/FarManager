extern "C"
{
  void WINAPI _export SetStartupInfo(struct PluginStartupInfo *Info);
  HANDLE WINAPI _export OpenPlugin(int OpenFrom,int Item);
  void WINAPI _export ClosePlugin(HANDLE hPlugin);
  int WINAPI _export GetFindData(HANDLE hPlugin,struct PluginPanelItem **pPanelItem,int *pItemsNumber,int OpMode);
  void WINAPI _export FreeFindData(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber);
  void WINAPI _export GetPluginInfo(struct PluginInfo *Info);
  void WINAPI _export GetOpenPluginInfo(HANDLE hPlugin,struct OpenPluginInfo *Info);
  int WINAPI _export SetDirectory(HANDLE hPlugin,char *Dir,int OpMode);
  int WINAPI _export DeleteFiles(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int OpMode);
  int WINAPI _export ProcessKey(HANDLE hPlugin,int Key,unsigned int ControlState);
  int WINAPI _export Configure(int ItemNumber);
  void WINAPI _export ExitFAR();
};


class NetBrowser
{
  private:
    int AddConnection(NETRESOURCE *nr);
    void GetLocalName(char *RemoteName,char *LocalName);
    int GetNameAndPassword(char *Title,char *Name,char *Password);
    void GetRemoteName(NETRESOURCE *NetRes,char *RemoteName);
    void GetNetList(HANDLE hEnum,NETRESOURCE *(&Res),DWORD &EnumCount);
    void DeleteNetList(NETRESOURCE *(&Res),DWORD &ResCount);
    void GetFreeLetter(DWORD &DriveMask,char *DiskName);
    char* CopyText(char *Text);
    static void DeleteNetText(NETRESOURCE &Res);
    int IsReadable(char *Remote);
    NETRESOURCE *NetRes;
    DWORD NetCount;
    NETRESOURCE *ConnectedRes;
    DWORD ConnectedCount;
    NETRESOURCE RootResources[16];
    DWORD RootCount;

    int FirstRead;
  public:
    NetBrowser();
    ~NetBrowser();
    int GetFindData(PluginPanelItem **pPanelItem,int *pItemsNumber,int OpMode);
    void FreeFindData(PluginPanelItem *PanelItem,int ItemsNumber);
    void GetOpenPluginInfo(struct OpenPluginInfo *Info);
    int SetDirectory(char *Dir,int OpMode);
    int DeleteFiles(struct PluginPanelItem *PanelItem,int ItemsNumber,int OpMode);
    int ProcessKey(int Key,unsigned int ControlState);
    static void DeleteCommonRootResources();
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
  int DisksMenuDigit;
  int SkipTopLevel;
} Opt;

static struct PluginStartupInfo Info;

char PluginRootKey[80];

char *GetMsg(int MsgId);
void InitDialogItems(struct InitDialogItem *Init,struct FarDialogItem *Item,
                     int ItemsNumber);
int Config();

void SetRegKey(HKEY hRoot,char *Key,char *ValueName,char *ValueData);
void SetRegKey(HKEY hRoot,char *Key,char *ValueName,DWORD ValueData);
void SetRegKey(HKEY hRoot,char *Key,char *ValueName,BYTE *ValueData,DWORD ValueSize);
int GetRegKey(HKEY hRoot,char *Key,char *ValueName,char *ValueData,char *Default,DWORD DataSize);
int GetRegKey(HKEY hRoot,char *Key,char *ValueName,int &ValueData,DWORD Default);
int GetRegKey(HKEY hRoot,char *Key,char *ValueName,DWORD Default);
int GetRegKey(HKEY hRoot,char *Key,char *ValueName,BYTE *ValueData,BYTE *Default,DWORD DataSize);
void DeleteRegKey(HKEY hRoot,char *Key);
