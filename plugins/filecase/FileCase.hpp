#include <plugin.hpp>

// TYPES OF CONVERSIONS

#define MODE_LOWER    0
#define MODE_UPPER    1
#define MODE_N_WORD   2
#define MODE_LN_WORD  3
#define MODE_NONE     4

extern PluginStartupInfo Info;
extern FarStandardFunctions FSF;

extern struct Options
{
	int ConvertMode;
	int ConvertModeExt;
	int SkipMixedCase;
	int ProcessSubDir;
	int ProcessDir;
	wchar_t WordDiv[512];
	int WordDivLen;
} Opt;

const wchar_t *GetMsg(int MsgId);
int IsCaseMixed(const wchar_t *Str);
void CaseWord( wchar_t *nm, int Type );
void ProcessName(const wchar_t *OldFullName, DWORD FileAttributes);
void CaseConvertion();
