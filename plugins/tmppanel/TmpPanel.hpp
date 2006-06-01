/*
TMPPANEL.HPP

Temporary panel header file

*/

#define REMOVE_FLAG 1

class TmpPanel
{
  private:
    void SortList();
    void RemoveDups();
    void RemoveEmptyItems();
    void UpdateItems(int ShowOwners,int ShowLinks);
    int IsOwnersDisplayed (const struct PanelInfo &PInfo);
    int IsLinksDisplayed (const struct PanelInfo &PInfo);
    void ProcessRemoveKey();
    void ProcessSaveListKey();
    void ProcessPanelSwitchMenu();
    void SwitchToPanel (int NewPanelIndex);
    void FindSearchResultsPanel();
    void SaveListFile (const char *Path);
    int IsCurrentFileCorrect (char *pCurFileName);

    PluginPanelItem *TmpPanelItem;
    int TmpItemsNumber;
    int LastOwnersRead;
    int LastLinksRead;
    int UpdateNotNeeded;
  public:
    TmpPanel();
    ~TmpPanel();
    int PanelIndex;
//    int OpenFrom;
    int GetFindData(PluginPanelItem **pPanelItem,int *pItemsNumber,int OpMode);
    void GetOpenPluginInfo(struct OpenPluginInfo *Info);
    int SetDirectory(const char *Dir,int OpMode);

    int PutFiles(struct PluginPanelItem *PanelItem,int ItemsNumber,int Move,int OpMode);
    HANDLE BeginPutFiles();
    void CommitPutFiles (HANDLE hRestoreScreen, int Success);
    int PutOneFile (PluginPanelItem &PanelItem);

    int SetFindList(const struct PluginPanelItem *PanelItem,int ItemsNumber);
    int ProcessEvent(int Event,void *Param);
    int ProcessKey(int Key,unsigned int ControlState);
    static int CheckForCorrect(const char *Dir,FAR_FIND_DATA *FindData,int OpenFrom);
    void IfOptCommonPanel(void);

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
  int CommonPanel;
  int SafeModePanel;
  int AnyInPanel;
  int CopyContents;
  int Mode;
  int MenuForFilelist;
  int NewPanelForSearchResults;
  int FullScreenPanel;
  int LastSearchResultsPanel;
  int SelectedCopyContents;
  char ColumnTypes[64];
  char ColumnWidths[64];
  char StatusColumnTypes[64];
  char StatusColumnWidths[64];
  char DisksMenuDigit[1];
  char Mask[512];
  char Prefix[16];
} Opt;

#define COMMONPANELSNUMBER 10
struct PluginPanels
{
  PluginPanelItem *Items;
  unsigned int ItemsNumber;
  unsigned int OpenFrom;
} CommonPanels[COMMONPANELSNUMBER];
unsigned int CurrentCommonPanel;

static struct PluginStartupInfo Info;
struct FarStandardFunctions FSF;
int StartupOptFullScreenPanel,StartupOptCommonPanel,StartupOpenFrom;

static char PluginRootKey[80];

const char *GetMsg(int MsgId);
void InitDialogItems(const struct InitDialogItem *Init,struct FarDialogItem *Item,
                     int ItemsNumber);

int Config();
void GoToFile(const char *Target, BOOL AnotherPanel);
void FreePanelItems(PluginPanelItem *Items, DWORD Total);

#if defined(__BORLANDC__)
char* __cdecl strchr(char * string,int ch);
char* __cdecl strrchr(char * string,int ch);
#elif !defined(_MSC_VER)
char* __cdecl strchr(const char * string,int ch);
char* __cdecl strrchr(const char * string,int ch);
#endif
char *ParseParam(char *& str);
