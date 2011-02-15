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
  wchar_t WordDiv[512];
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
  const wchar_t *Data;
};

const wchar_t *GetMsg(int MsgId);
void InitDialogItems(const struct InitDialogItem *Init,struct FarDialogItem *Item,int ItemsNumber);
int IsCaseMixed(const wchar_t *Str);
const wchar_t *GetOnlyName(const wchar_t *FullName);
wchar_t *GetFullName(wchar_t *Dest,const wchar_t *Dir,const wchar_t *Name);
void CaseWord( wchar_t *nm, int Type );
void ProcessName(const wchar_t *OldFullName, DWORD FileAttributes);

static struct PluginStartupInfo Info;
static struct FarStandardFunctions FSF;
wchar_t *PluginRootKey;
