#ifndef __TREELIST_HPP__
#define __TREELIST_HPP__
/*
treelist.hpp

Tree panel

*/

/* Revision: 1.01 16.10.2000 $ */

/*
Modify:
  16.10.2000 tran
    + MustBeCached(Root) - функция, определяющая необходимость кеширования 
      дерева
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

class TreeList:public Panel
{
  private:
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
    void KillFocus();
    void Update(int Mode);
    void ReadTree();
    void SetCurDir(char *NewDir,int ClosePlugin);
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
};

#endif  // __TREELIST_HPP__
