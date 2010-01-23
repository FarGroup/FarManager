struct Options
{
  TCHAR FileMasks[512];
  TCHAR ExcludeFileMasks[512];
  int RightMargin;
  int Wrap;
} Opt;

void SetRegKey(HKEY hRoot,const TCHAR *Key,const TCHAR *ValueName,DWORD ValueData);
void SetRegKey(HKEY hRoot,const TCHAR *Key,const TCHAR *ValueName,TCHAR *ValueData);
int GetRegKey(HKEY hRoot,const TCHAR *Key,const TCHAR *ValueName,int &ValueData,DWORD Default);
int GetRegKey(HKEY hRoot,const TCHAR *Key,const TCHAR *ValueName,DWORD Default);
int GetRegKey(HKEY hRoot,const TCHAR *Key,const TCHAR *ValueName,TCHAR *ValueData,const TCHAR *Default,DWORD DataSize);
const TCHAR *GetMsg(int MsgId);
TCHAR *GetCommaWord(const TCHAR *Src,TCHAR *Word);


static struct PluginStartupInfo Info;
static FARSTANDARDFUNCTIONS FSF;
TCHAR PluginRootKey[80];
