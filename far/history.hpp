#ifndef __HISTORY_HPP__
#define __HISTORY_HPP__
/*
history.hpp

История (Alt-F8, Alt-F11, Alt-F12)

*/

#include "farconst.hpp"
#include "UnicodeString.hpp"

enum{
  HISTORYTYPE_CMD,
  HISTORYTYPE_FOLDER,
  HISTORYTYPE_VIEW,
};

#define HISTORY_TITLESIZE 32

struct HistoryRecordW
{
  int   Type;
  wchar_t  Title[HISTORY_TITLESIZE]; //BUGBUG
  wchar_t *Name;
};

class HistoryW
{
  private:
    string strRegKey;
    unsigned int LastPtr,CurLastPtr;
    unsigned int LastPtr0,CurLastPtr0;
    int EnableAdd,RemoveDups,KeepSelectedPos;
    int TypeHistory;
    int HistoryCount;
    const int *EnableSave;
    int SaveTitle,SaveType;
    int LastSimilar;
    int ReturnSimilarTemplate;
    struct HistoryRecordW *LastStr;

  private:
    void AddToHistoryLocal(const wchar_t *Str,const wchar_t *Title,int Type);
    void FreeHistory();
    BOOL EqualType(int Type1, int Type2);

  public:
    HistoryW(int TypeHistory,int HistoryCount,const wchar_t *RegKey,const int *EnableSave,int SaveTitle,int SaveType);
   ~HistoryW();

  public:
    void AddToHistory(const wchar_t *Str,const wchar_t *Title=NULL,int Type=0,int SaveForbid=0);
    BOOL ReadHistory();
    BOOL SaveHistory();
    int  Select(const wchar_t *Title,const wchar_t *HelpTopic, string &strStr,int &Type, string *strItemTitle = NULL);
    void GetPrev(string &strStr);
    void GetNext(string &strStr);
    void SetFirst() {LastPtr=LastPtr0;CurLastPtr=CurLastPtr0;}
    void GetSimilar(string &strStr,int LastCmdPartLength);
    void SetAddMode(int EnableAdd,int RemoveDups,int KeepSelectedPos);
    void ReloadTitle();
};


#endif  // __HISTORY_HPP__
