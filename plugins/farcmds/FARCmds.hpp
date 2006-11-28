struct RegistryStr {char *Add2PlugMenu; char *Add2DisksMenu; char *sss;
                    char *Separator; char *DisksMenuDigit; char *ShowCmdOutput;
                    char *CatchMode; char *ViewZeroFiles; char *EditNewFiles;};

struct HELPIDS {char *CMD; char *Config;};

void SetRegKey(HKEY hRoot,const char *Key,const char *ValueName,DWORD ValueData);
void SetRegKey(HKEY hRoot,const char *Key,const char *ValueName,char *ValueData);
int GetRegKey(HKEY hRoot,const char *Key,const char *ValueName,int &ValueData,DWORD Default);
int GetRegKey(HKEY hRoot,const char *Key,const char *ValueName,DWORD Default);
int GetRegKey(HKEY hRoot,const char *Key,const char *ValueName,char *ValueData,char *Default,DWORD DataSize);
int GetRegKey(HKEY hRoot,const char *Key,const char *ValueName,BYTE *ValueData,BYTE *Default,DWORD DataSize);
void SetRegKey(HKEY hRoot,const char *Key,const char *ValueName,BYTE *ValueData,DWORD ValueSize);


inline int IsSpace(int x) { return x==' ' || x=='\t'; }
inline int IsEol(int x)  { return x=='\r' || x=='\n'; }

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

struct Options{
  int Add2PlugMenu;
  int Add2DisksMenu;
  int DisksMenuDigit;
  int ShowCmdOutput;
  int CatchMode;
  int ViewZeroFiles;
  int EditNewFiles;
  char Separator[4];
} Opt={
 0,
 0,
 0,
 0,
 0,
 1,
 1,
 " "
};
