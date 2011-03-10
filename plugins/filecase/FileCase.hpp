// TYPES OF CONVERSIONS

#define MODE_LOWER    0
#define MODE_UPPER    1
#define MODE_N_WORD   2
#define MODE_LN_WORD  3
#define MODE_NONE     4

const wchar_t *GetMsg(int MsgId);
int IsCaseMixed(const wchar_t *Str);
const wchar_t *GetOnlyName(const wchar_t *FullName);
wchar_t *GetFullName(wchar_t *Dest,const wchar_t *Dir,const wchar_t *Name);
void CaseWord( wchar_t *nm, int Type );
void ProcessName(const wchar_t *OldFullName, DWORD FileAttributes);
