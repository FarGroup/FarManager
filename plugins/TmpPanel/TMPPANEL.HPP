extern "C"
{
  void WINAPI _export SetStartupInfo(struct PluginStartupInfo *Info);
  HANDLE WINAPI _export OpenPlugin(int OpenFrom,int Item);
  void WINAPI _export ClosePlugin(HANDLE hPlugin);
  void WINAPI _export ExitFAR();
  int WINAPI _export GetFindData(HANDLE hPlugin,struct PluginPanelItem **pPanelItem,int *pItemsNumber,int OpMode);
  void WINAPI _export GetPluginInfo(struct PluginInfo *Info);
  void WINAPI _export GetOpenPluginInfo(HANDLE hPlugin,struct OpenPluginInfo *Info);
  int WINAPI _export SetDirectory(HANDLE hPlugin,char *Dir,int OpMode);
  int WINAPI _export PutFiles(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int Move,int OpMode);
  int WINAPI _export SetFindList(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber);
  int WINAPI _export ProcessEvent(HANDLE hPlugin,int Event,void *Param);
  int WINAPI _export ProcessKey(HANDLE hPlugin,int Key,unsigned int ControlState);
  int WINAPI _export Configure(int ItemNumber);
};

#define REMOVE_FLAG 1

class TmpPanel
{
  private:
    void SortList();
    void RemoveDups();
    void RemoveEmptyItems();
    void UpdateItems(int ShowOwners,int ShowLinks);
    int IsOwnersDisplayed();
    int IsLinksDisplayed();
    PluginPanelItem *TmpPanelItem;
    int TmpItemsNumber;
    int LastOwnersRead;
    int LastLinksRead;
    int UpdateNotNeeded;
  public:
    TmpPanel();
    ~TmpPanel();
    int GetFindData(PluginPanelItem **pPanelItem,int *pItemsNumber,int OpMode);
    void GetOpenPluginInfo(struct OpenPluginInfo *Info);
    int SetDirectory(char *Dir,int OpMode);
    int PutFiles(struct PluginPanelItem *PanelItem,int ItemsNumber,
                 int Move,int OpMode);
    int SetFindList(struct PluginPanelItem *PanelItem,int ItemsNumber);
    int ProcessEvent(int Event,void *Param);
    int ProcessKey(int Key,unsigned int ControlState);
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
  int CommonPanel;
} Opt;


static PluginPanelItem *CommonTmpPanelItem;
static int CommonTmpItemsNumber;

static struct PluginStartupInfo Info;

char PluginRootKey[80];

char *GetMsg(int MsgId);
void InitDialogItems(struct InitDialogItem *Init,struct FarDialogItem *Item,
                     int ItemsNumber);
void AddEndSlash(char *Path);
char* PointToName(char *Path);
int GetFileOwner(char *Name,char *Owner);
int GetNumberOfLinks(char *Name);

int Config();

void SetRegKey(HKEY hRoot,char *Key,char *ValueName,char *ValueData);
void SetRegKey(HKEY hRoot,char *Key,char *ValueName,DWORD ValueData);
void SetRegKey(HKEY hRoot,char *Key,char *ValueName,BYTE *ValueData,DWORD ValueSize);
int GetRegKey(HKEY hRoot,char *Key,char *ValueName,char *ValueData,char *Default,DWORD DataSize);
int GetRegKey(HKEY hRoot,char *Key,char *ValueName,int &ValueData,DWORD Default);
int GetRegKey(HKEY hRoot,char *Key,char *ValueName,DWORD Default);
int GetRegKey(HKEY hRoot,char *Key,char *ValueName,BYTE *ValueData,BYTE *Default,DWORD DataSize);
void DeleteRegKey(HKEY hRoot,char *Key);

