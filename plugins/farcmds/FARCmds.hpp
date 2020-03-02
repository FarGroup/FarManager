struct OptionsName
{
	const wchar_t *ShowCmdOutput;
	const wchar_t *CatchMode;
	const wchar_t *ViewZeroFiles;
	const wchar_t *EditNewFiles;
	const wchar_t *MaxDataSize;
	const wchar_t *Separator;
};

struct Options
{
	int ShowCmdOutput;
	int CatchMode;
	int ViewZeroFiles;
	int EditNewFiles;
	DWORD MaxDataSize;
	wchar_t Separator[4];
};

class ConsoleTitle{
	private:
		wchar_t *OldTitle;

	public:
		ConsoleTitle(const wchar_t *NewTitle):OldTitle(nullptr)
		{
			int Length = GetWindowTextLength(GetConsoleWindow());
			if (Length)
			{
				OldTitle=new wchar_t[Length+1];
				if (OldTitle)
				{
					GetWindowText(GetConsoleWindow(), OldTitle, Length + 1);
					SetConsoleTitle(NewTitle);
				}
			}
		}
		~ConsoleTitle()
		{
			if (OldTitle)
			{
				SetConsoleTitle(OldTitle);
				delete[] OldTitle;
			}
		}
};


class MakeVETitle{
	private:
		wchar_t *Title;

	public:
		MakeVETitle(const wchar_t *Pref, const wchar_t *Cmd)
		{
			Title=new wchar_t[lstrlen(Pref)+(Cmd?lstrlen(Cmd):0)+1];
			if (Title)
			{
				lstrcpy(Title,Pref);
				if (Cmd) lstrcat(Title,Cmd);
			}
		}
		~MakeVETitle()
		{
			if (Title)
				delete[] Title;
		}
		const wchar_t *Get() {return Title;}
};


#define THREADSLEEP  200
#define THREADREDRAW 10

struct TShowOutputStreamData
{
	HANDLE hRead;
	HANDLE hWrite;
	HANDLE hConsole;
};

enum enStream { enStreamOut, enStreamErr, enStreamMAX };
enum enThreadType { enThreadHideOutput, enThreadShowOutput };

struct TThreadData
{
	enThreadType type;
	HANDLE process;
	TShowOutputStreamData stream[enStreamMAX];
	wchar_t title[80], cmd[1024];
};


enum enShowCmdOutput {
	scoHide,          // <>
	scoShow,          // <<
	scoShowAll        // <+
};

enum CatchModeType {
	cmtAllInOne  = 0,  // <* - redirect #stderr# and #stdout# as one stream
	cmtStdOut    = 1,  // <1 - redirect only standard output stream #stdout#
	cmtStdErr    = 2,  // <2 - redirect only standard output stream #stderr#
	cmtDiff      = 3,  // <? - redirect #stderr# and #stdout# as different streams
};

extern struct PluginStartupInfo Info;
extern struct FarStandardFunctions FSF;
extern struct Options Opt;


inline int IsSpace(int x) { return x==L' ' || x==L'\t'; }
inline int IsEol(int x)  { return x==L'\r' || x==L'\n'; }

inline bool FileExists(const wchar_t *Name)  { return GetFileAttributes(Name)!=0xFFFFFFFF; }


bool StrToGuid(const wchar_t *Value,GUID *Guid);
int GetInt(const wchar_t *Start, wchar_t *End);
const wchar_t *GetMsg(int MsgId);
int ReplaceStrings(wchar_t *Str,const wchar_t *FindStr,const wchar_t *ReplStr,int Count,BOOL IgnoreCase);
int PartCmdLine(const wchar_t *CmdStr,wchar_t **NewCmdStr,wchar_t **NewCmdPar);
wchar_t *ProcessOSAliases(const wchar_t *Str);
wchar_t *GetShellLinkPath(const wchar_t *LinkFile);
wchar_t *OpenFromCommandLine(const wchar_t *_farcmd);
wchar_t *ExpandEnv(const wchar_t* Src, DWORD* Length);

wchar_t* __proc_Load(int outputtofile,wchar_t *pCmd);
wchar_t* __proc_Unload(int outputtofile,wchar_t *pCmd);
wchar_t* __proc_Goto(int outputtofile,wchar_t *pCmd);
wchar_t* __proc_WhereIs(int outputtofile,wchar_t *pCmd,bool Dir=true);

bool __proc_Link(int outputtofile,wchar_t *pCmd);

bool IsTextUTF8(const char* Buffer,size_t Length);
wchar_t *ConvertBuffer(wchar_t* Ptr,size_t PtrSize,BOOL outputtofile, size_t& shift,bool *unicode);

#define CP_UNICODE    ((uintptr_t)1200)
#define CP_REVERSEBOM ((uintptr_t)1201)

UINT GetCPBuffer(const void* data, size_t size, size_t* off);
