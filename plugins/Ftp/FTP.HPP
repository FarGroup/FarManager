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
  int WINAPI _export GetFiles(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int Move,char *DestPath,int OpMode);
  int WINAPI _export PutFiles(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int Move,int OpMode);
  int WINAPI _export DeleteFiles(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int OpMode);
  int WINAPI _export MakeDirectory(HANDLE hPlugin,char *Name,int OpMode);
  int WINAPI _export ProcessKey(HANDLE hPlugin,int Key,unsigned int ControlState);
  int WINAPI _export ProcessEvent(HANDLE hPlugin,int Event,void *Param);
  int WINAPI _export Configure(int ItemNumber);
  int WINAPI _export Compare(HANDLE hPlugin,struct PluginPanelItem *Item1,struct PluginPanelItem *Item2,unsigned int Mode);
};


class FTP
{
  private:
    int Connect();
    int GetHost(char *HostName,char *Password,char *HostDescr,int &AskLogin,
                int &AsciiMode,int &PassiveMode,int &UseFirewall,
                char *HostTable);
    int ExpandList(struct PluginPanelItem *PanelItem,int ItemsNumber,
                   struct PluginPanelItem **pFullItemsList,
                   int *pFullItemsNumber,int FromPlugin);
    void ShellDeleteMsg(char *Name,int Flags);
    void GetNewKeyName(char *FullKeyName);
    int GetFreeKeyNumber();
    void MakeKeyName(char *FullKeyName,int Number);
    void GetFullKey(char *FullKeyName,char *Name);
    int GetHostFiles(struct PluginPanelItem *PanelItem,int ItemsNumber,
                     int Move,char *DestPath,int OpMode);
    int PutHostsFiles(struct PluginPanelItem *PanelItem,int ItemsNumber,
                      int Move,int OpMode);
    void SaveURL();
    int GetLoginData(char *UserName,char *Password);
    void GetFullFileName(char *FullName,char *Name);
    void CopyNamesToClipboard();
    void PasswordToHex(char *Password,char *HexStr);
    void HexToPassword(char *HexStr,char *Password);
    int HexToNum(int Hex);
    void SelectTable();
    void SelectHostTable(char *TableName);
    int TableNameToValue(char *TableName);
    void SetAttributes();
    void ShellChmodMsg(char *Name,int Flags);

    char HostName[NM],Password[256],HostDescr[256];
    int AskLogin;
    int PassiveMode;
    int UseFirewall;
    int AsciiMode;
    char HostTable[100];
    HINTERNET hConnect;
    int ResetCache;
    char PanelTitle[512];
    int ShowHosts;
    int SwitchingToFTP;
    char HostsPath[1024];
    int StartViewMode;
    char DirToRestore[NM];
    int LastUsedTableNum;
    int RereadRequired;
  public:
    FTP();
    ~FTP();
    int GetFindData(PluginPanelItem **pPanelItem,int *pItemsNumber,int OpMode);
    void FreeFindData(PluginPanelItem *PanelItem,int ItemsNumber);
    void GetOpenPluginInfo(struct OpenPluginInfo *Info);
    int SetDirectory(char *Dir,int OpMode);
    int GetFiles(struct PluginPanelItem *PanelItem,int ItemsNumber,
                 int Move,char *DestPath,int OpMode);
    int PutFiles(struct PluginPanelItem *PanelItem,int ItemsNumber,
                 int Move,int OpMode);
    int DeleteFiles(struct PluginPanelItem *PanelItem,int ItemsNumber,
                    int OpMode);
    int MakeDirectory(char *Name,int OpMode);
    int ProcessKey(int Key,unsigned int ControlState);
    int ProcessEvent(int Event,void *Param);
    int ProcessCommandLine(char *CommandLine);
    int ProcessShortcutLine(char *Line);
    int Compare(struct PluginPanelItem *Item1,struct PluginPanelItem *Item2,
                unsigned int Mode);
    static void EncryptPassword(char *Src,char *Dest);
    static void DecryptPassword(char *Src,char *Dest);
};


class EnumHost
{
  private:
    HKEY hEnum;
    char RootKey[1024];
    int HostPos;
  public:
    EnumHost(HKEY hRoot,char *HostsPath);
    ~EnumHost();
    BOOL GetNextHost(char *EnumKey,char *HostName=NULL,FILETIME *LastWrite=NULL);
};


class SaveConsoleTitle
{
  private:
    char SaveTitle[512];
  public:
    SaveConsoleTitle() {GetConsoleTitle(SaveTitle,sizeof(SaveTitle));};
    ~SaveConsoleTitle() {SetConsoleTitle(SaveTitle);};
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

struct Options Opt;
struct PluginStartupInfo Info;

char PluginRootKey[80];

char *GetMsg(int MsgId);
void InitDialogItems(struct InitDialogItem *Init,struct FarDialogItem *Item,
                     int ItemsNumber);
void AddEndSlash(char *Path);
char* TruncStr(char *Str,int MaxLength);
char* PointToName(char *Path);
int CheckForEsc();
int IsCaseMixed(char *Str);
void LocalLower(char *Str);
int LocalStricmp(char *Str1,char *Str2);
void CenterStr(char *Src,char *Dest,int Length);
int Config();
void ConnectMessage(int Msg,char *HostName);
int CopyToClipboard(char *Data);
int ToPercent(long N1,long N2);
long filelen(FILE *FPtr);

void SetRegKey(HKEY hRoot,char *Key,char *ValueName,char *ValueData);
void SetRegKey(HKEY hRoot,char *Key,char *ValueName,DWORD ValueData);
void SetRegKey(HKEY hRoot,char *Key,char *ValueName,BYTE *ValueData,DWORD ValueSize);
int GetRegKey(HKEY hRoot,char *Key,char *ValueName,char *ValueData,char *Default,DWORD DataSize);
int GetRegKey(HKEY hRoot,char *Key,char *ValueName,int &ValueData,DWORD Default);
int GetRegKey(HKEY hRoot,char *Key,char *ValueName,DWORD Default);
int GetRegKey(HKEY hRoot,char *Key,char *ValueName,BYTE *ValueData,BYTE *Default,DWORD DataSize);
void DeleteRegKey(HKEY hRoot,char *Key);
BOOL CheckRegKey(HKEY hRoot,char *Key);
HKEY OpenRegKey(HKEY hRoot,char *Key);

