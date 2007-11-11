#ifndef __EditCase_H
#define __EditCase_H

static struct PluginStartupInfo Info;
static FARSTANDARDFUNCTIONS FSF;

BOOL IsOldFAR;

// Menu item numbers
#define CCLower     0
#define CCTitle     1
#define CCUpper     2
#define CCToggle    3
#define CCCyclic    4

// Plugin Registry root
static TCHAR PluginRootKey[80];
// This chars aren't letters
static TCHAR WordDiv[80];
static int WordDivLen;

const TCHAR *GetMsg(int MsgId);

BOOL FindBounds(TCHAR *Str, int Len, int Pos, int &Start, int &End);
int FindEnd(TCHAR *Str, int Len, int Pos);
int FindStart(TCHAR *Str, int Len, int Pos);
BOOL MyIsAlpha(int c);
int GetNextCCType(TCHAR *Str, int StrLen, int Start, int End);
int ChangeCase(TCHAR *NewString, int Start, int End, int CCType);

#endif
