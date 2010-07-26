struct RegistryStr
{
	const TCHAR *Add2PlugMenu;
	const TCHAR *Add2DisksMenu;
	const TCHAR *Separator;
	const TCHAR *DisksMenuDigit;
	const TCHAR *ShowCmdOutput;
	const TCHAR *CatchMode;
	const TCHAR *ViewZeroFiles;
	const TCHAR *EditNewFiles;
	const TCHAR *MaxDataSize;
};

void SetRegKey(HKEY hRoot,const TCHAR *Key,const TCHAR *ValueName,DWORD ValueData);
void SetRegKey(HKEY hRoot,const TCHAR *Key,const TCHAR *ValueName,TCHAR *ValueData);
int GetRegKey(HKEY hRoot,const TCHAR *Key,const TCHAR *ValueName,int &ValueData,DWORD Default);
int GetRegKey(HKEY hRoot,const TCHAR *Key,const TCHAR *ValueName,DWORD Default);
int GetRegKey(HKEY hRoot,const TCHAR *Key,const TCHAR *ValueName,TCHAR *ValueData,const TCHAR *Default,DWORD DataSize);
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
	const TCHAR *Data;
};

struct Options
{
	int Add2PlugMenu;
	int Add2DisksMenu;
	int DisksMenuDigit;
	int ShowCmdOutput;
	int CatchMode;
	int ViewZeroFiles;
	int EditNewFiles;
	DWORD MaxDataSize;
	TCHAR Separator[4];
} Opt=
{
	0,
	0,
	0,
	0,
	0,
	1,
	1,
	1048576,
	_T(" ")
};
