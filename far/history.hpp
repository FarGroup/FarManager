#ifndef __HISTORY_HPP__
#define __HISTORY_HPP__
/*
history.hpp

История (Alt-F8, Alt-F11, Alt-F12)

*/

#include "farconst.hpp"

enum{
  HISTORYTYPE_CMD,
  HISTORYTYPE_FOLDER,
  HISTORYTYPE_VIEW,
};

#define HISTORY_TITLESIZE 32

struct HistoryRecord
{
  int   Type;
  char  Title[HISTORY_TITLESIZE];
  char *Name;
};

class History
{
  private:
    char RegKey[256];
    unsigned int LastPtr,CurLastPtr;
    unsigned int LastPtr0,CurLastPtr0;
    int EnableAdd,RemoveDups,KeepSelectedPos;
    int TypeHistory;
    int HistoryCount;
    const int *EnableSave;
    int SaveTitle,SaveType;
    int LastSimilar;
    int ReturnSimilarTemplate;
    struct HistoryRecord *LastStr;

  private:
    void AddToHistoryLocal(const char *Str,const char *Title,int Type);
    void FreeHistory();
    BOOL EqualType(int Type1, int Type2);

  public:
    History(int TypeHistory,int HistoryCount,const char *RegKey,const int *EnableSave,int SaveTitle,int SaveType);
   ~History();

  public:
    void AddToHistory(const char *Str,const char *Title=NULL,int Type=0,int SaveForbid=0);
    BOOL ReadHistory();
    BOOL SaveHistory();
    int  Select(const char *Title,const char *HelpTopic,char *Str,int StrLength,int &Type,char *ItemTitle=NULL);
    void GetPrev(char *Str,int StrLength);
    void GetNext(char *Str,int StrLength);
    void SetFirst() {LastPtr=LastPtr0;CurLastPtr=CurLastPtr0;}
    void GetSimilar(char *Str,int LastCmdPartLength);
    void SetAddMode(int EnableAdd,int RemoveDups,int KeepSelectedPos);
    void ReloadTitle();
};

#endif  // __HISTORY_HPP__
