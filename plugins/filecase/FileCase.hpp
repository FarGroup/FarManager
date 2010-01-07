// TYPES OF CONVERSIONS

#define MODE_LOWER    0
#define MODE_UPPER    1
#define MODE_N_WORD   2
#define MODE_LN_WORD  3
#define MODE_NONE     4

struct Options
{
  int ConvertMode;
  int ConvertModeExt;
  int SkipMixedCase;
  int ProcessSubDir;
  int ProcessDir;
  TCHAR WordDiv[512];
  int WordDivLen;
} Opt;

struct InitDialogItem
{
  unsigned char Type;
  unsigned char X1,Y1,X2,Y2;
  unsigned char Focus;
  DWORD_PTR Selected;
  unsigned int Flags;
  unsigned char DefaultButton;
  const TCHAR *Data;
};

const TCHAR *GetMsg(int MsgId);
void InitDialogItems(const struct InitDialogItem *Init,struct FarDialogItem *Item,int ItemsNumber);
int IsCaseMixed(const TCHAR *Str);
const TCHAR *GetOnlyName(const TCHAR *FullName);
TCHAR *GetFullName(TCHAR *Dest,const TCHAR *Dir,const TCHAR *Name);
void CaseWord( TCHAR *nm, int Type );
void ProcessName(TCHAR *OldFullName, DWORD FileAttributes);


void SetRegKey(HKEY hRoot,const TCHAR *Key,const TCHAR *ValueName,DWORD ValueData);
void SetRegKey(HKEY hRoot,const TCHAR *Key,const TCHAR *ValueName,TCHAR *ValueData);
int GetRegKey(HKEY hRoot,const TCHAR *Key,const TCHAR *ValueName,int &ValueData,DWORD Default);
int GetRegKey(HKEY hRoot,const TCHAR *Key,const TCHAR *ValueName,DWORD Default);
int GetRegKey(HKEY hRoot,const TCHAR *Key,const TCHAR *ValueName,TCHAR *ValueData,const TCHAR *Default,DWORD DataSize);

static struct PluginStartupInfo Info;
static struct FarStandardFunctions FSF;
TCHAR PluginRootKey[80];
