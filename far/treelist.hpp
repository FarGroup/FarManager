#ifndef __TREELIST_HPP__
#define __TREELIST_HPP__
/*
treelist.hpp

Tree panel

*/

/* Revision: 1.07 24.10.2001 $ */

/*
Modify:
  24.10.2001 VVM
    + TreeIsPrepared - устанавливается после чтения дерева с диска/файла.
  22.10.2001 SVS
    ! ReadTree() возвращает TRUE/FALSE
  21.10.2001 SVS
    + PR_MsgReadTree
  06.05.2001 DJ
    ! перетрях #include
  25.04.2001 SVS
    + SetRootDir()
  05.04.2001 VVM
    + Переключение макросов в режим MACRO_TREEPANEL
  16.10.2000 tran
    + MustBeCached(Root) - функция, определяющая необходимость кеширования
      дерева
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#include "panel.hpp"

struct TreeItem
{
  char Name[NM];
  int Last[NM/2];
  int Depth;
};

class TreeList: public Panel
{
  private:
    int TreeIsPrepared;
    int PrevMacroMode;
    void SetMacroMode(int Restore = FALSE);
    void DisplayObject();
    void DisplayTree(int Fast);
    void DisplayTreeName(char *Name,int Pos);
    void Up(int Count);
    void Down(int Count);
    void CorrectPosition();
    static int MsgReadTree(int TreeCount,int &FirstCall);
    void FillLastData();
    int CountSlash(char *Str);
    int SetDirPosition(char *NewDir);
    void GetRoot();
    Panel* GetRootPanel();
    void SyncDir();
    void SaveTreeFile();
    int ReadTreeFile();
    static int GetCacheTreeName(char *Root,char *Name,int CreateDir);
    int GetSelCount();
    int GetSelName(char *Name,int &FileAttr,char *ShortName=NULL);

    struct TreeItem *ListData;
    char Root[NM];
    long TreeCount;
    long WorkDir;
    long GetSelPosition;
    int UpdateRequired;
    int CaseSensitiveSort;
  public:
    TreeList();
    ~TreeList();
    int ProcessKey(int Key);
    int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
//    void KillFocus();
    void Update(int Mode);
    int  ReadTree();
    void SetCurDir(char *NewDir,int ClosePlugin);
    void SetRootDir(char *NewRootDir);
    void GetCurDir(char *CurDir);
    int GetCurName(char *Name,char *ShortName);
    void UpdateViewPanel();
    void MoveToMouse(MOUSE_EVENT_RECORD *MouseEvent);
    int FindPartName(char *Name,int Next);
    int GoToFile(char *Name);
    void ProcessEnter();
    static void AddTreeName(char *Name);
    static void DelTreeName(char *Name);
    static void RenTreeName(char *SrcName,char *DestName);
    static void ReadSubTree(char *Path);
    static void ClearCache(int EnableFreeMem);
    static void ReadCache(char *TreeRoot);
    static void FlushCache();

    /* $ 16.10.2000 tran
       функция, определяющаяя необходимость кеширования
       файла */
    static int MustBeCached(char *Root);
    /* tran $ */
    virtual void SetFocus();
    virtual void KillFocus();
    static void PR_MsgReadTree(void);
};

#endif  // __TREELIST_HPP__
