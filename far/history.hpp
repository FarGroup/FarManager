#ifndef __HISTORY_HPP__
#define __HISTORY_HPP__
/*
history.hpp

История (Alt-F8, Alt-F11, Alt-F12)

*/

/* Revision: 1.00 25.06.2000 $ */

/*
Modify:
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

class History
{
  private:
    void AddToHistoryLocal(char *Str,char *Title,int Type);
    struct HistoryRecord LastStr[64];
    char RegKey[256];
    unsigned int LastPtr,CurLastPtr;
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
    void GetSimilar(char *Str,int LastCmdPartLength);
    void SetAddMode(int EnableAdd,int RemoveDups,int KeepSelectedPos);
};

#endif	// __HISTORY_HPP__
