struct RegistryStr
{
 const TCHAR *sss;
 const TCHAR *EditorKey;
 const TCHAR *ProcessEditorInput;
 const TCHAR *Style;
} REGStr= {
 _T("%s%s%s"),
 _T("EditorKey"),
 _T("ProcessEditorInput"),
 _T("Style")
};

struct HELPIDS
{
 const TCHAR *Contents;
 const TCHAR *cmd;
 const TCHAR *Config;
} HlfId=
{
 _T("Contents"),
 _T("cmd"),
 _T("Config")
};

void SetRegKey(HKEY hRoot,const TCHAR *Key,const TCHAR *ValueName,DWORD ValueData);
void SetRegKey(HKEY hRoot,const TCHAR *Key,const TCHAR *ValueName,TCHAR *ValueData);
int GetRegKey(HKEY hRoot,const TCHAR *Key,const TCHAR *ValueName,int &ValueData,DWORD Default);
int GetRegKey(HKEY hRoot,const TCHAR *Key,const TCHAR *ValueName,DWORD Default);
int GetRegKey(HKEY hRoot,const TCHAR *Key,const TCHAR *ValueName,TCHAR *ValueData,const TCHAR *Default,DWORD DataSize);
int GetRegKey(HKEY hRoot,const TCHAR *Key,const TCHAR *ValueName,BYTE *ValueData,BYTE *Default,DWORD DataSize);
void SetRegKey(HKEY hRoot,const TCHAR *Key,const TCHAR *ValueName,BYTE *ValueData,DWORD ValueSize);

const TCHAR *GetMsg(int MsgId);
BOOL FileExists(const TCHAR* Name);
void InitDialogItems(const struct InitDialogItem *Init,struct FarDialogItem *Item, int ItemsNumber);
BOOL CheckExtension(const TCHAR *ptrName);
void ShowHelp(const TCHAR *fullfilename,const TCHAR *topic, bool CmdLine=false);
void RestorePosition(void);
BOOL IsHlf(void);
const TCHAR *FindTopic(void);
void ShowCurrentHelpTopic();
void ShowHelpFromTempFile();

struct InitDialogItem
{
  int Type;
  int X1,Y1,X2,Y2;
  int Focus;
  DWORD_PTR Selected;
  DWORD Flags;
  int DefaultButton;
  const TCHAR *Data;
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
TCHAR PluginRootKey[80];
TCHAR FullFileName[MAX_PATH];
struct EditorInfo ei;
struct EditorGetString egs;
struct EditorSetPosition esp;

static TCHAR KeyNameFromReg[34];
