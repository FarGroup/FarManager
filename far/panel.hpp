#ifndef __PANEL_HPP__
#define __PANEL_HPP__
/*
panel.hpp

Parent class ��� �������

*/

/* Revision: 1.50 01.09.2006 $ */

#include "scrobj.hpp"
#include "farconst.hpp"
#include "struct.hpp"

class DizList;

struct PanelViewSettings
{
  unsigned int ColumnType[20];
  int ColumnWidth[20];
  int ColumnCount;
  unsigned int StatusColumnType[20];
  int StatusColumnWidth[20];
  int StatusColumnCount;
  int FullScreen;
  int AlignExtensions;
  int FolderAlignExtensions;
  int FolderUpperCase;
  int FileLowerCase;
  int FileUpperToLowerCase;
  int CaseSensitiveSort;
};

enum {FILE_PANEL,TREE_PANEL,QVIEW_PANEL,INFO_PANEL};

enum {UNSORTED,BY_NAME,BY_EXT,BY_MTIME,BY_CTIME,BY_ATIME,BY_SIZE,BY_DIZ,
      BY_OWNER,BY_COMPRESSEDSIZE,BY_NUMLINKS};

enum {VIEW_0=0,VIEW_1,VIEW_2,VIEW_3,VIEW_4,VIEW_5,VIEW_6,VIEW_7,VIEW_8,VIEW_9};

enum {
  DRIVE_SHOW_TYPE       = 0x00000001,
  DRIVE_SHOW_NETNAME    = 0x00000002,
  DRIVE_SHOW_LABEL      = 0x00000004,
  DRIVE_SHOW_FILESYSTEM = 0x00000008,
  DRIVE_SHOW_SIZE       = 0x00000010,
  DRIVE_SHOW_REMOVABLE  = 0x00000020,
  DRIVE_SHOW_PLUGINS    = 0x00000040,
  DRIVE_SHOW_CDROM      = 0x00000080,
  DRIVE_SHOW_SIZE_FLOAT = 0x00000100,
  DRIVE_SHOW_REMOTE     = 0x00000200,
};

enum {UPDATE_KEEP_SELECTION=1,UPDATE_SECONDARY=2,UPDATE_IGNORE_VISIBLE=4,UPDATE_DRAW_MESSAGE=8};

enum {NORMAL_PANEL,PLUGIN_PANEL};

enum {DRIVE_DEL_FAIL, DRIVE_DEL_SUCCESS, DRIVE_DEL_EJECT};

enum {UIC_UPDATE_NORMAL, UIC_UPDATE_FORCE, UIC_UPDATE_FORCE_NOTIFICATION};

class VMenu;
class Edit;
class Panel:public ScreenObject
{
  protected:
    string strCurDir;
    int Focus;
    int Type;
    int EnableUpdate;
    int PanelMode;
    int SortMode;
    int SortOrder;
    int SortGroups;
    int PrevViewMode,ViewMode;
    long CurTopFile;
    long CurFile;
    int ShowShortNames;
    int NumericSort;
    int ModalMode;
    int PluginCommand;
    BYTE PluginParam[1024];

  public:
    struct PanelViewSettings ViewSettings;
    int ProcessingPluginCommand;

  private:
    int ChangeDiskMenu(int Pos,int FirstCall);
    /* $ 28.12.2001 DJ
       ��������� Del � ���� ������
    */
    int ProcessDelDisk (wchar_t Drive, int DriveType,VMenu *ChDiskMenu);
    /* DJ $ */
    void FastFindShow(int FindX,int FindY);
    void FastFindProcessName(Edit *FindEdit,const wchar_t *Src,string &strLastName, string &strName);
    void DragMessage(int X,int Y,int Move);

  protected:
    void FastFind(int FirstKey);
    void DrawSeparator(int Y);
    void ShowScreensCount();
    int IsDragging();
    int  ProcessShortcutFolder(int Key,BOOL ProcTreePanel=FALSE);

  public:
    Panel();
    virtual ~Panel();

  public:
    virtual int SendKeyToPlugin(DWORD Key,BOOL Pred=FALSE){return FALSE;};
    virtual void SetCurDirW(const wchar_t *NewDir,int ClosePlugin);
    virtual void ChangeDirToCurrent();

    virtual int GetCurDirW(string &strCurDir);

    virtual int GetSelCount() {return(0);};
    virtual int GetRealSelCount() {return(0);};
    virtual int GetSelNameW(string *strName,int &FileAttr,string *ShortName=NULL,FAR_FIND_DATA_EX *fd=NULL) {return(FALSE);};
    virtual void UngetSelName() {};
    virtual void ClearLastGetSelection() {};
    virtual unsigned __int64 GetLastSelectedSize () {return (unsigned __int64)(-1);};
    virtual int GetLastSelectedItem(struct FileListItem *LastItem) {return(0);};

    virtual int GetCurNameW(string &strName, string &strShortName);
    virtual int GetCurBaseNameW(string &strName, string &strShortName);
    virtual int GetFileNameW(string strName,int Pos,int &FileAttr) {return(FALSE);};

    virtual int GetCurrentPos() {return(0);};
    virtual void SetFocus();
    virtual void KillFocus();
    virtual void Update(int Mode) {};
    /*$ 22.06.2001 SKV
      �������� ��� ������������� ������� ���������� Update.
      ������������ ��� Update ����� ���������� �������.
    */
    virtual int UpdateIfChanged(int UpdateMode) {return(0);};
    /* SKV$*/
    /* $ 19.03.2002 DJ
       UpdateIfRequired() - ��������, ���� ������ ��� �������� ��-�� ����,
       ��� ������ ��������
    */
    virtual void UpdateIfRequired() {};
    /* DJ $ */
    virtual void CloseChangeNotification() {};
    virtual int FindPartName(const wchar_t *Name,int Next,int Direct=1) {return(FALSE);}

    virtual int GoToFileW(const wchar_t *Name,BOOL OnlyPartName=FALSE) {return(TRUE);};
    virtual int FindFileW(const wchar_t *Name,BOOL OnlyPartName=FALSE) {return -1;};

    virtual int IsSelectedW(const wchar_t *Name) {return(FALSE);};

    virtual int FindFirstW(const wchar_t *Name) {return -1;}
    virtual int FindNextW(int StartPos, const wchar_t *Name) {return -1;}

    /* $ 09.02.2001 IS
       ������� ������������/���������� ��������� ������
       "���������� ����� ������"
    */
    virtual void SetSelectedFirstMode(int) {};
    virtual int GetSelectedFirstMode(void) {return 0;};
    /* IS $ */
    int GetMode() {return(PanelMode);};
    void SetMode(int Mode) {PanelMode=Mode;};
    int GetModalMode() {return(ModalMode);};
    void SetModalMode(int ModalMode) {Panel::ModalMode=ModalMode;};
    int GetViewMode() {return(ViewMode);};
    virtual void SetViewMode(int ViewMode);
    virtual int GetPrevViewMode() {return(PrevViewMode);};
    void SetPrevViewMode(int PrevViewMode) {Panel::PrevViewMode=PrevViewMode;};
    virtual int GetPrevSortMode() {return(SortMode);};
    virtual int GetPrevSortOrder() {return(SortOrder);};
    int GetSortMode() {return(SortMode);};
    virtual int GetPrevNumericSort() {return NumericSort;};
    int GetNumericSort() { return NumericSort; }
    void SetNumericSort(int Mode) { Panel::NumericSort=Mode; }
    virtual void SetSortMode(int SortMode) {Panel::SortMode=SortMode;};
    int GetSortOrder() {return(SortOrder);};
    void SetSortOrder(int SortOrder) {Panel::SortOrder=SortOrder;};
    /* $ 24.04.2001 VVM
      �������� ������� ���������� �� ������ */
    virtual void ChangeSortOrder(int NewOrder) {SetSortOrder(NewOrder);};
    /* VVM $ */
    int GetSortGroups() {return(SortGroups);};
    void SetSortGroups(int SortGroups) {Panel::SortGroups=SortGroups;};
    int GetShowShortNamesMode() {return(ShowShortNames);};
    void SetShowShortNamesMode(int Mode) {ShowShortNames=Mode;};
    void InitCurDirW(const wchar_t *CurDir);
    virtual void CloseFile() {};
    virtual void UpdateViewPanel() {};
    virtual void CompareDir() {};
    virtual void MoveToMouse(MOUSE_EVENT_RECORD *MouseEvent) {};
    virtual void ClearSelection() {};
    virtual void SaveSelection() {};
    virtual void RestoreSelection() {};
    virtual void SortFileList(int KeepPosition) {};
    virtual void EditFilter() {};
    virtual void ReadDiz(struct PluginPanelItemW *ItemList=NULL,int ItemLength=0, DWORD dwFlags=0) {};
    virtual void DeleteDiz(const wchar_t *Name,const wchar_t *ShortName) {};
    virtual void GetDizName(const wchar_t *DizName) {};
    virtual void FlushDiz() {};
    virtual void CopyDiz(const wchar_t *Name,const wchar_t *ShortName,const wchar_t *DestName,
                 const wchar_t *DestShortName,DizList *DestDiz) {};
    virtual int IsFullScreen() {return(FALSE);};
    virtual int IsDizDisplayed() {return(FALSE);};
    virtual int IsColumnDisplayed(int Type) {return(FALSE);};
    virtual void SetReturnCurrentFile(int Mode) {};
    virtual void QViewDelTempName() {};
    virtual void GetPluginInfo(struct PluginInfoW *Info) {};
    virtual void GetOpenPluginInfo(struct OpenPluginInfoW *Info) {};
    virtual void SetPluginMode(HANDLE hPlugin,char *PluginFile) {};
    virtual void SetPluginModified() {};
    virtual int ProcessPluginEvent(int Event,void *Param) {return(FALSE);};
    virtual HANDLE GetPluginHandle() {return(INVALID_HANDLE_VALUE);};
    virtual void SetTitle();
    virtual void GetTitle(string &Title,int SubLen=-1,int TruncSize=0);

    virtual void IfGoHomeW(wchar_t Drive){};

    /* $ 30.04.2001 DJ
       ������� ���������� ��� ���������� �������; ���� ���������� FALSE,
       ������������ ����������� ������
    */
    virtual BOOL UpdateKeyBar() { return FALSE; };
    /* DJ $ */
    virtual long GetFileCount() {return 0;}
    virtual BOOL GetItem(int,void *){return FALSE;};

    static void EndDrag();
    void Hide();
    void Show();
    int SetPluginCommand(int Command,void *Param);
    int PanelProcessMouse(MOUSE_EVENT_RECORD *MouseEvent,int &RetCode);
    void ChangeDisk();
    int GetFocus() {return(Focus);};
    int GetType() {return(Type);};
    void SetUpdateMode(int Mode) {EnableUpdate=Mode;};
    int MakeListFile(string &strListFileName,int ShortNames,const wchar_t *Modifers=NULL);
    int SetCurPath();

    BOOL NeedUpdatePanel(Panel *AnotherPanel);
};

#endif  // __PANEL_HPP__
