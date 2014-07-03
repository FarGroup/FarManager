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
	bool processDone;
	TShowOutputStreamData stream[enStreamMAX];
	wchar_t title[80], cmd[1024];
};


enum enShowCmdOutput {
	scoHide,          // <>
	scoShow,          // <<
	scoShowAll        // <+
};

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
