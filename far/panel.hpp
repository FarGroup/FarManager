#ifndef __PANEL_HPP__
#define __PANEL_HPP__
/*
panel.hpp

Parent class для панелей

*/

/* Revision: 1.00 25.06.2000 $ */

/*
Modify:
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/


class Panel:public ScreenObject
{
  private:
    int ChangeDiskMenu(int Pos,int FirstCall);
    void FastFindShow(int FindX,int FindY);
    void DragMessage(int X,int Y,int Move);
  protected:
    void FastFind(int FirstKey);
    void DrawSeparator(int Y);
    void ShowScreensCount();
    int IsDragging();

    char CurDir[NM];
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
    int DisableOut;
    int ModalMode;
    int PluginCommand;
    BYTE PluginParam[1024];
  public:
    Panel();
    virtual ~Panel();
    virtual void SetCurDir(char *NewDir,int ClosePlugin);
    virtual void ChangeDirToCurrent();
    virtual void GetCurDir(char *CurDir);
    virtual int GetSelCount() {return(0);};
    virtual int GetSelName(char *Name,int &FileAttr,char *ShortName=NULL) {return(FALSE);};
    virtual void UngetSelName() {};
    virtual void ClearLastGetSelection() {};
    virtual long GetLastSelectedSize(int64 *Size) {return(-1);};
    virtual int GetLastSelectedItem(struct FileListItem *LastItem) {return(0);};
    virtual int GetCurName(char *Name,char *ShortName);
    virtual int GetFileName(char *Name,int Pos,int &FileAttr) {return(FALSE);};
    virtual int GetCurrentPos() {return(0);};
    virtual void SetFocus();
    virtual void KillFocus();
    virtual void Update(int Mode) {};
    virtual int UpdateIfChanged() {return(0);};
    virtual void CloseChangeNotification() {};
    virtual int FindPartName(char *Name,int Next) {return(FALSE);}
    virtual int GoToFile(char *Name) {return(TRUE);};
    virtual int IsSelected(char *Name) {return(FALSE);};
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
    virtual void SetSortMode(int SortMode) {Panel::SortMode=SortMode;};
    int GetSortOrder() {return(SortOrder);};
    void SetSortOrder(int SortOrder) {Panel::SortOrder=SortOrder;};
    int GetSortGroups() {return(SortGroups);};
    void SetSortGroups(int SortGroups) {Panel::SortGroups=SortGroups;};
    int GetShowShortNamesMode() {return(ShowShortNames);};
    void SetShowShortNamesMode(int Mode) {ShowShortNames=Mode;};
    void InitCurDir(char *CurDir);
    virtual void CloseFile() {};
    virtual void UpdateViewPanel() {};
    virtual void CompareDir() {};
    virtual void MoveToMouse(MOUSE_EVENT_RECORD *MouseEvent) {};
    virtual void ClearSelection() {};
    virtual void SaveSelection() {};
    virtual void RestoreSelection() {};
    virtual void SortFileList(int KeepPosition) {};
    virtual void EditFilter() {};
    virtual void ReadDiz(struct PluginPanelItem *ItemList=NULL,int ItemLength=0) {};
    virtual void DeleteDiz(char *Name,char *ShortName) {};
    virtual void GetDizName(char *DizName) {};
    virtual void FlushDiz() {};
    virtual void CopyDiz(char *Name,char *ShortName,char *DestName,
                 char *DestShortName,DizList *DestDiz) {};
    virtual int IsFullScreen() {return(FALSE);};
    virtual int IsDizDisplayed() {return(FALSE);};
    virtual int IsColumnDisplayed(int Type) {return(FALSE);};
    virtual void SetReturnCurrentFile(int Mode) {};
    virtual void QViewDelTempName() {};
    virtual void GetPluginInfo(struct PluginInfo *Info) {};
    virtual void GetOpenPluginInfo(struct OpenPluginInfo *Info) {};
    virtual void SetPluginMode(HANDLE hPlugin,char *PluginFile) {};
    virtual void SetPluginModified() {};
    virtual int ProcessPluginEvent(int Event,void *Param) {return(FALSE);};
    virtual HANDLE GetPluginHandle() {return(INVALID_HANDLE_VALUE);};
    virtual void SetTitle();

    static void EndDrag();
    void Hide();
    void Show();
    void SetPluginCommand(int Command,void *Param);
    int PanelProcessMouse(MOUSE_EVENT_RECORD *MouseEvent,int &RetCode);
    void ChangeDisk();
    int GetFocus() {return(Focus);};
    int GetType() {return(Type);};
    void SetUpdateMode(int Mode) {EnableUpdate=Mode;};
    int MakeListFile(char *ListFileName,int ShortNames);
    int SetCurPath();

    struct PanelViewSettings ViewSettings;
    int ProcessingPluginCommand;
};

#endif	// __PANEL_HPP__
