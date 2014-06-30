struct OptionsName
{
	const wchar_t *ShowCmdOutput;
	const wchar_t *CatchMode;
	const wchar_t *ViewZeroFiles;
	const wchar_t *EditNewFiles;
	const wchar_t *MaxDataSize;
	const wchar_t *Separator;
};

inline int IsSpace(int x) { return x==L' ' || x==L'\t'; }
inline int IsEol(int x)  { return x==L'\r' || x==L'\n'; }

struct Options
{
	int ShowCmdOutput;
	int CatchMode;
	int ViewZeroFiles;
	int EditNewFiles;
	DWORD MaxDataSize;
	wchar_t Separator[4];
} Opt;

extern struct PluginStartupInfo Info;
extern struct FarStandardFunctions FSF;


bool StrToGuid(const wchar_t *Value,GUID *Guid);
int GetInt(wchar_t *Start, wchar_t *End);
const wchar_t *GetMsg(int MsgId);
BOOL FileExists(const wchar_t *Name);
wchar_t *GetCommaWord(wchar_t *Src,wchar_t *Word,wchar_t Separator);
int ReplaceStrings(wchar_t *Str,const wchar_t *FindStr,const wchar_t *ReplStr,int Count,BOOL IgnoreCase);
int PartCmdLine(const wchar_t *CmdStr,wchar_t *NewCmdStr,int SizeNewCmdStr,wchar_t *NewCmdPar,int SizeNewCmdPar);
BOOL ProcessOSAliases(wchar_t *Str,int SizeStr);
wchar_t *GetShellLinkPath(const wchar_t *LinkFile);

