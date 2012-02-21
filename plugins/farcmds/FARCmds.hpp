struct OptionsName
{
	const wchar_t *Add2PlugMenu;
	const wchar_t *Add2DisksMenu;
	const wchar_t *Separator;
	const wchar_t *ShowCmdOutput;
	const wchar_t *CatchMode;
	const wchar_t *ViewZeroFiles;
	const wchar_t *EditNewFiles;
	const wchar_t *MaxDataSize;
};

inline int IsSpace(int x) { return x==L' ' || x==L'\t'; }
inline int IsEol(int x)  { return x==L'\r' || x==L'\n'; }

struct Options
{
	int Add2PlugMenu;
	int Add2DisksMenu;
	int ShowCmdOutput;
	int CatchMode;
	int ViewZeroFiles;
	int EditNewFiles;
	DWORD MaxDataSize;
	wchar_t Separator[4];
} Opt;
