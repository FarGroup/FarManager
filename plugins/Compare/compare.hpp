extern "C"
{
  void WINAPI _export SetStartupInfo(struct PluginStartupInfo *Info);
  HANDLE WINAPI _export OpenPlugin(int OpenFrom,int Item);
  void WINAPI _export GetPluginInfo(struct PluginInfo *Info);
};


struct Options
{
  int ProcessSubfolders;
  int CompareTime;
  int LowPrecisionTime;
  int CompareSize;
  int CompareContents;
} Opt;

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

void Compare();
int CompareContents(char *AName,char *PName);
int CompareDirectories(char *ADir,char *PDir);
void ShowMessage(char *Name1,char *Name2);
void SetRegKey(HKEY hRoot,char *Key,char *ValueName,DWORD ValueData);
int GetRegKey(HKEY hRoot,char *Key,char *ValueName,int &ValueData,DWORD Default);
int GetRegKey(HKEY hRoot,char *Key,char *ValueName,DWORD Default);
int LocalStricmp(char *Str1,char *Str2);
int CheckForEsc();
void AddEndSlash(char *Path);
char* TruncStr(char *Str,int MaxLength);

static struct PluginStartupInfo Info;
char PluginRootKey[80];


