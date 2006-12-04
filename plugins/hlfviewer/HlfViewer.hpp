struct RegistryStr
{
 char *sss;
 char *EditorKey;
 char *ProcessEditorInput;
 char *Style;
} REGStr= {
 "%s%s%s",
 "EditorKey",
 "ProcessEditorInput",
 "Style"
};

struct HELPIDS
{
 char *Contents;
 char *cmd;
 char *Config;
} HlfId=
{
 "Contents",
 "cmd",
 "Config"
};

void SetRegKey(HKEY hRoot,const char *Key,const char *ValueName,DWORD ValueData);
void SetRegKey(HKEY hRoot,const char *Key,const char *ValueName,char *ValueData);
int GetRegKey(HKEY hRoot,const char *Key,const char *ValueName,int &ValueData,DWORD Default);
int GetRegKey(HKEY hRoot,const char *Key,const char *ValueName,DWORD Default);
int GetRegKey(HKEY hRoot,const char *Key,const char *ValueName,char *ValueData,char *Default,DWORD DataSize);
int GetRegKey(HKEY hRoot,const char *Key,const char *ValueName,BYTE *ValueData,BYTE *Default,DWORD DataSize);
void SetRegKey(HKEY hRoot,const char *Key,const char *ValueName,BYTE *ValueData,DWORD ValueSize);

const char *GetMsg(int MsgId);
BOOL FileExists(const char* Name);
void InitDialogItems(const struct InitDialogItem *Init,struct FarDialogItem *Item, int ItemsNumber);
BOOL CheckExtension(const char *ptrName);
void ShowHelp(const char *fullfilename,const char *topic, bool CmdLine=false);
void RestorePosition(void);
BOOL IsHlf(void);
const char *FindTopic(void);
void ShowCurrentHelpTopic();
void ShowHelpFromTempFile();

struct InitDialogItem
{
  int Type;
  int X1,Y1,X2,Y2;
  int Focus;
  int Selected;
  DWORD Flags;
  int DefaultButton;
  char *Data;
};

struct Options
{
 BOOL ProcessEditorInput;
 int Key;
 DWORD Style;
} Opt;

FARSTDPOINTTONAME PointToName;
FARSTDGETPATHROOT GetPathRoot;
FARSTDADDENDSLASH AddEndSlash;
FARSTDSPRINTF FarSprintf;

static struct PluginStartupInfo Info;
static struct FarStandardFunctions FSF;

struct PanelInfo PInfo;
char PluginRootKey[80];
char FullFileName[NM];
struct EditorInfo ei;
struct EditorGetString egs;
struct EditorSetPosition esp;

static char KeyNameFromReg[34];
BOOL IsOldFar;
