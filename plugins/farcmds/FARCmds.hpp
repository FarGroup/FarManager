struct RegistryStr {TCHAR *Add2PlugMenu; TCHAR *Add2DisksMenu; TCHAR *sss;
                    TCHAR *Separator; TCHAR *DisksMenuDigit; TCHAR *ShowCmdOutput;
                    TCHAR *CatchMode; TCHAR *ViewZeroFiles; TCHAR *EditNewFiles;};

struct HELPIDS {TCHAR *CMD; TCHAR *Config;};

void SetRegKey(HKEY hRoot,const TCHAR *Key,const TCHAR *ValueName,DWORD ValueData);
void SetRegKey(HKEY hRoot,const TCHAR *Key,const TCHAR *ValueName,TCHAR *ValueData);
int GetRegKey(HKEY hRoot,const TCHAR *Key,const TCHAR *ValueName,int &ValueData,DWORD Default);
int GetRegKey(HKEY hRoot,const TCHAR *Key,const TCHAR *ValueName,DWORD Default);
int GetRegKey(HKEY hRoot,const TCHAR *Key,const TCHAR *ValueName,TCHAR *ValueData,TCHAR *Default,DWORD DataSize);
int GetRegKey(HKEY hRoot,const TCHAR *Key,const TCHAR *ValueName,BYTE *ValueData,BYTE *Default,DWORD DataSize);
void SetRegKey(HKEY hRoot,const TCHAR *Key,const TCHAR *ValueName,BYTE *ValueData,DWORD ValueSize);


inline int IsSpace(int x) { return x==_T(' ') || x==_T('\t'); }
inline int IsEol(int x)  { return x==_T('\r') || x==_T('\n'); }

struct InitDialogItem
{
  int Type;
  int X1,Y1,X2,Y2;
  int Focus;
  DWORD_PTR Selected;
  DWORD Flags;
  int DefaultButton;
  TCHAR *Data;
};

struct Options{
  int Add2PlugMenu;
  int Add2DisksMenu;
  int DisksMenuDigit;
  int ShowCmdOutput;
  int CatchMode;
  int ViewZeroFiles;
  int EditNewFiles;
  TCHAR Separator[4];
} Opt={
 0,
 0,
 0,
 0,
 0,
 1,
 1,
 _T(" ")
};
