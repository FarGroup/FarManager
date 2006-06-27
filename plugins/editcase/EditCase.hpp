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
static char PluginRootKey[80];
// This chars aren't letters
static char WordDiv[80];
static int WordDivLen;

const char *GetMsg(int MsgId);

BOOL FindBounds(char *Str, int Len, int Pos, int &Start, int &End);
int FindEnd(char *Str, int Len, int Pos);
int FindStart(char *Str, int Len, int Pos);
BOOL MyIsAlpha(unsigned char c);
int GetNextCCType(char *Str, int StrLen, int Start, int End);
int ChangeCase(char *NewString, int Start, int End, int CCType);

#endif
