#ifndef __HISTORY_HPP__
#define __HISTORY_HPP__
/*
history.hpp

История (Alt-F8, Alt-F11, Alt-F12)

*/

#include "TList.hpp"

#include "farconst.hpp"

enum{
  HISTORYTYPE_CMD,
  HISTORYTYPE_FOLDER,
  HISTORYTYPE_VIEW,
};

struct HistoryRecord
{
  bool  Lock;
  int   Type;
  char *Name;
  FILETIME Timestamp;

  HistoryRecord()
  {
    Lock = false;
    Type = 0;
    Name = NULL;
    Timestamp.dwLowDateTime=0;
    Timestamp.dwHighDateTime=0;
  }

  const HistoryRecord& operator=(const HistoryRecord &rhs);
};

class History: protected TList<HistoryRecord>
{
  private:
    char RegKey[256];
    bool EnableAdd, KeepSelectedPos, SaveType;
    int RemoveDups;
    int TypeHistory;
    int HistoryCount;
    const int *EnableSave;

  private:
    void AddToHistoryLocal(const char *Str,const char *Prefix,int Type);
    const char *GetTitle(int Type);
    bool EqualType(int Type1, int Type2);

  public:
    History(int TypeHistory,int HistoryCount,const char *RegKey,const int *EnableSave,bool SaveType);
   ~History();

  public:
    void AddToHistory(const char *Str,int Type=0,const char *Prefix=NULL,bool SaveForbid=false);
    bool ReadHistory();
    bool SaveHistory();
    int  Select(const char *Title,const char *HelpTopic,char *Str,int StrLength,int &Type);
    void GetPrev(char *Str,int StrLength);
    void GetNext(char *Str,int StrLength);
    void GetSimilar(char *Str,int StrLength,int LastCmdPartLength);
    void SetAddMode(bool EnableAdd,int RemoveDups,bool KeepSelectedPos);
    void ResetPosition();
};

#endif  // __HISTORY_HPP__
