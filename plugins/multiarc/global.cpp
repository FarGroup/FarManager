#include "plugin.hpp"
#include "fmt.hpp"
#include "multiarc.hpp"

char PluginRootKey[80];
struct FarStandardFunctions FSF;
struct Options Opt;
struct PluginStartupInfo Info;
int FarVER;

class ArcPlugins *ArcPlugin=NULL;

char *CmdNames[]={"Extract","ExtractWithoutPath","Test","Delete",
                  "Comment","CommentFiles","SFX","Lock","Protect","Recover",
                  "Add","Move","AddRecurse","MoveRecurse","AllFilesMask",
                  "DefExt"};

#ifdef _NEW_ARC_SORT_
char IniFile[MAX_PATH];
char *SortModes[]={"None", "Name", "RunRate", "ChoiceRate", "User"};
#endif //_NEW_ARC_SORT_
