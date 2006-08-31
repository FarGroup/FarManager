#ifndef __TREELIST_HPP__
#define __TREELIST_HPP__
/*
treelist.hpp

Tree panel

*/

/* Revision: 1.34 01.09.2006 $ */

#include "panel.hpp"
#include "UnicodeString.hpp"

struct TreeItem
{
  string strName;
  int Last[NM/2];        // ?
  int Depth;             // ������� �����������
};

enum TREELIST_FLAGS{
  FTREELIST_TREEISPREPARED          = 0x00010000,
  FTREELIST_UPDATEREQUIRED          = 0x00020000,
  FTREELIST_ISPANEL                 = 0x00040000,
};

class TreeList: public Panel
{
  private:
    int PrevMacroMode;
    struct TreeItem *ListData;
    string strRoot;
    long TreeCount;
    long WorkDir;
    long GetSelPosition;
    int CaseSensitiveSort;
    int NumericSort;
    int ExitCode; // ��������� ������ ��� ������, ���������� �� ������!

  private:
    void SetMacroMode(int Restore = FALSE);
    void DisplayObject();
    void DisplayTree(int Fast);
    void DisplayTreeName(const wchar_t *Name,int Pos);
    void Up(int Count);
    void Down(int Count);
    void Scroll(int Count);
    void CorrectPosition();
    void FillLastData();
    int CountSlash(const wchar_t *Str);
    int SetDirPosition(const wchar_t *NewDir);
    void GetRoot();
    Panel* GetRootPanel();
    void SyncDir();
    void SaveTreeFile();
    int ReadTreeFile();
    int GetSelCount();
    void DynamicUpdateKeyBar();
    int GetNextNavPos();
    int GetPrevNavPos();
    static string &MkTreeFileName(const wchar_t *RootDir,string &strDest);
    static string &MkTreeCacheFolderName(const wchar_t *RootDir,string &strDest);
    static string &CreateTreeFileName(const wchar_t *Path,string &strDest);

  private:
    static int MsgReadTree(int TreeCount,int &FirstCall);
    static int GetCacheTreeName(const wchar_t *Root, string &strName,int CreateDir);

  public:
    TreeList(int IsPanel=TRUE);
    ~TreeList();

  public:
    int ProcessKey(int Key);
    int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
//    void KillFocus();
    void Update(int Mode);
    int  ReadTree();

    void SetCurDirW(const wchar_t *NewDir,int ClosePlugin);

    void SetRootDirW(const wchar_t *NewRootDir);

    int GetCurDirW(string &strCurDir);

    virtual int GetCurNameW(string &strName, string &strShortName);

    void UpdateViewPanel();
    void MoveToMouse(MOUSE_EVENT_RECORD *MouseEvent);
    int FindPartName(const wchar_t *Name,int Next,int Direct=1);

    int GoToFileW(const wchar_t *Name,BOOL OnlyPartName=FALSE);
    int FindFileW(const wchar_t *Name,BOOL OnlyPartName=FALSE);

    void ProcessEnter();

    virtual int FindFirstW(const wchar_t *Name);
    virtual int FindNextW(int StartPos, const wchar_t *Name);

    int GetExitCode() {return ExitCode;}
    virtual long GetFileCount() {return TreeCount;}
    virtual int GetFileNameW(string &strName,int Pos,int &FileAttr);

    virtual void SetTitle();
    virtual void GetTitle(string &Title,int SubLen=-1,int TruncSize=0);
    virtual void SetFocus();
    virtual void KillFocus();
    virtual BOOL UpdateKeyBar();
    virtual BOOL GetItem(int Index,void *Dest);
    virtual int GetCurrentPos();

    virtual int GetSelNameW(string *strName,int &FileAttr,string *ShortName=NULL,FAR_FIND_DATA_EX *fd=NULL);

  public:
    static void AddTreeName(const wchar_t *Name);
    static void DelTreeName(const wchar_t *Name);
    static void RenTreeName(const wchar_t *SrcName, const wchar_t *DestName);
    static void ReadSubTree(const wchar_t *Path);
    static void ClearCache(int EnableFreeMem);
    static void ReadCache(const wchar_t *TreeRoot);
    static void FlushCache();

    static int MustBeCached(const wchar_t *Root); // $ 16.10.2000 tran - �������, ������������� ������������� ����������� �����
    static void PR_MsgReadTree(void);
};

#endif  // __TREELIST_HPP__
