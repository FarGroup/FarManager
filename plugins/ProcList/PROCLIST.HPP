#define  Min(x,y) (((x)<(y)) ? (x):(y))
#define  Max(x,y) (((x)>(y)) ? (x):(y))

extern "C"
{
  void WINAPI _export SetStartupInfo(struct PluginStartupInfo *Info);
  HANDLE WINAPI _export OpenPlugin(int OpenFrom,int Item);
  void WINAPI _export ClosePlugin(HANDLE hPlugin);
  int WINAPI _export GetFindData(HANDLE hPlugin,struct PluginPanelItem **pPanelItem,int *pItemsNumber,int OpMode);
  void WINAPI _export FreeFindData(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber);
  void WINAPI _export GetPluginInfo(struct PluginInfo *Info);
  void WINAPI _export GetOpenPluginInfo(HANDLE hPlugin,struct OpenPluginInfo *Info);
  int WINAPI _export GetFiles(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int Move,char *DestPath,int OpMode);
  int WINAPI _export DeleteFiles(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int OpMode);
  int WINAPI _export ProcessEvent(HANDLE hPlugin,int Event,void *Param);
  int WINAPI _export ProcessKey(HANDLE hPlugin,int Key,unsigned int ControlState);
};


class Plist
{
  private:
    char* PrintTitle(int MsgId);
    void FileTimeToText(FILETIME *CurFileTime,FILETIME *SrcTime,char *TimeText);
    clock_t LastUpdateTime;
  public:
    Plist();
    int GetFindData(PluginPanelItem **pPanelItem,int *pItemsNumber,int OpMode);
    void FreeFindData(PluginPanelItem *PanelItem,int ItemsNumber);
    void GetOpenPluginInfo(struct OpenPluginInfo *Info);
    int SetDirectory(char *Dir,int OpMode);
    int GetFiles(struct PluginPanelItem *PanelItem,int ItemsNumber,
                 int Move,char *DestPath,int OpMode);
    int DeleteFiles(struct PluginPanelItem *PanelItem,int ItemsNumber,
                    int OpMode);
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


struct ProcessData
{
  DWORD Size;
  HWND hwnd;
  DWORD Threads;
  UINT  uAppType;
  char FullPath[NM];
};


static struct PluginStartupInfo Info;

int NT;

char PluginRootKey[80];

int GetList95(PluginPanelItem **pPanelItem,int *pItemsNumber);
int GetListNT(PluginPanelItem **pPanelItem,int *pItemsNumber);
BOOL KillProcess(DWORD pid);
BOOL KillProcessNT(DWORD pid,HWND hwnd);

char *GetMsg(int MsgId);
void InitDialogItems(struct InitDialogItem *Init,struct FarDialogItem *Item,
                     int ItemsNumber);
int LocalStricmp(char *Str1,char *Str2);
void AddEndSlash(char *Path);
char *NullToEmpty(char *Str);
void ConvertDate(FILETIME *ft,char *DateText,char *TimeText,int TimeLength);
int GetDateFormat();
int GetDateSeparator();
int GetTimeSeparator();

void SetRegKey(HKEY hRoot,char *Key,char *ValueName,char *ValueData);
void SetRegKey(HKEY hRoot,char *Key,char *ValueName,DWORD ValueData);
void SetRegKey(HKEY hRoot,char *Key,char *ValueName,BYTE *ValueData,DWORD ValueSize);
int GetRegKey(HKEY hRoot,char *Key,char *ValueName,char *ValueData,char *Default,DWORD DataSize);
int GetRegKey(HKEY hRoot,char *Key,char *ValueName,int &ValueData,DWORD Default);
int GetRegKey(HKEY hRoot,char *Key,char *ValueName,DWORD Default);
int GetRegKey(HKEY hRoot,char *Key,char *ValueName,BYTE *ValueData,BYTE *Default,DWORD DataSize);
void DeleteRegKey(HKEY hRoot,char *Key);

