#include "MultiArc.hpp"

char PluginRootKey[NM];
struct FarStandardFunctions FSF;
struct Options Opt;
struct PluginStartupInfo Info;

class ArcPlugins *ArcPlugin=NULL;

const char *CmdNames[]={"Extract","ExtractWithoutPath","Test","Delete",
                  "Comment","CommentFiles","SFX","Lock","Protect","Recover",
                  "Add","Move","AddRecurse","MoveRecurse","AllFilesMask",
                  "DefExt"};

#ifdef _NEW_ARC_SORT_
char IniFile[MAX_PATH];
const char *SortModes[]={"None", "Name", "RunRate", "ChoiceRate", "User"};
#endif //_NEW_ARC_SORT_

#ifndef BELOW_NORMAL_PRIORITY_CLASS
#define BELOW_NORMAL_PRIORITY_CLASS 0x00004000
#define ABOVE_NORMAL_PRIORITY_CLASS 0x00008000
#endif

DWORD PriorityProcessCode[]={
  IDLE_PRIORITY_CLASS,
  BELOW_NORMAL_PRIORITY_CLASS,
  NORMAL_PRIORITY_CLASS,
  ABOVE_NORMAL_PRIORITY_CLASS,
  HIGH_PRIORITY_CLASS,
};

OSVERSIONINFO WinVer;
