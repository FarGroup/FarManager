#ifndef __HISTORY_HPP__
#define __HISTORY_HPP__
/*
history.hpp

История (Alt-F8, Alt-F11, Alt-F12)

*/

/* Revision: 1.04 04.06.2001 $ */

/*
Modify:
  04.06.2001 SVS
    ! 64 -> HISTORY_COUNT
  06.05.2001 DJ
    ! перетрях #include
  09.01.2001 SVS
    - Бага с CmdHistoryRule=1
  24.09.2000 SVS
    + SetFirst() - для CmdHistoryRule
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#include "farconst.hpp"

struct HistoryRecord
{
  char Name[512];
  char Title[32];
  int Type;
};

class History
{
  private:
    void AddToHistoryLocal(char *Str,char *Title,int Type);
    struct HistoryRecord LastStr[HISTORY_COUNT];
    char RegKey[256];
    unsigned int LastPtr,CurLastPtr;
    unsigned int LastPtr0,CurLastPtr0;
    int EnableAdd,RemoveDups,KeepSelectedPos;
    int *EnableSave,SaveTitle,SaveType;
    int LastSimilar;
    int ReturnSimilarTemplate;
  public:
    History(char *RegKey,int *EnableSave,int SaveTitle,int SaveType);
    void AddToHistory(char *Str,char *Title=NULL,int Type=0);
    void ReadHistory();
    void SaveHistory();
    int Select(char *Title,char *HelpTopic,char *Str,int &Type,char *ItemTitle=NULL);
    void GetPrev(char *Str);
    void GetNext(char *Str);
    void SetFirst() {LastPtr=LastPtr0;CurLastPtr=CurLastPtr0;}
    void GetSimilar(char *Str,int LastCmdPartLength);
    void SetAddMode(int EnableAdd,int RemoveDups,int KeepSelectedPos);
};

#endif	// __HISTORY_HPP__
