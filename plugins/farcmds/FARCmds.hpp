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
