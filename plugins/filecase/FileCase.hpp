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
  char WordDiv[512];
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
  const char *Data;
};

const char *GetMsg(int MsgId);
void InitDialogItems(const struct InitDialogItem *Init,struct FarDialogItem *Item,int ItemsNumber);
int IsCaseMixed(const char *Str);
char *GetOnlyName(char *FullName);
char *GetFullName(char *Dest,const char *Dir,char *Name);
void CaseWord( char *nm, int Type );
void ProcessName(char *OldFullName, DWORD FileAttributes);


void SetRegKey(HKEY hRoot,const char *Key,const char *ValueName,DWORD ValueData);
void SetRegKey(HKEY hRoot,const char *Key,const char *ValueName,char *ValueData);
int GetRegKey(HKEY hRoot,const char *Key,const char *ValueName,int &ValueData,DWORD Default);
int GetRegKey(HKEY hRoot,const char *Key,const char *ValueName,DWORD Default);
int GetRegKey(HKEY hRoot,const char *Key,const char *ValueName,char *ValueData,const char *Default,DWORD DataSize);

static struct PluginStartupInfo Info;
static struct FarStandardFunctions FSF;
char PluginRootKey[80];
BOOL IsOldFar;
