#ifndef __FILELIST_HPP__
#define __FILELIST_HPP__
/*
filelist.hpp

Файловая панель - общие функции

*/

/* Revision: 1.05 09.02.2001 $ */

/*
Modify:
  09.02.2001 IS
    + Get(Set)SelectedFirstMode
  04.01.2001 SVS
    ! TranslateKeyToVK() -> keyboard.cpp
  27.09.2000 SVS
    ! FileList::CallPlugin() перенесен в PluginsSet
  21.09.2000 SVS
    + Функция CallPlugin - найти плагин по ID и запустить
  11.09.2000 SVS
    + Переменная IsEmpty, указывающая на полностью пустую колонку
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#ifndef __TIME_H
#include <time.h>
#endif

class FileList:public Panel
{
  private:
    PanelFilter *Filter;
    DizList Diz;
    int DizRead;
    char PluginDizName[NM];
    struct FileListItem *ListData;
    long FileCount;
    struct PrevDataItem *PrevDataStack;
    int PrevDataStackSize;
    HANDLE hListChange;
    struct PluginsStackItem *PluginsStack;
    int PluginsStackSize;
    HANDLE hPlugin;
    long UpperFolderTopFile,LastCurFile;
    long ReturnCurrentFile;
    long SelFileCount;
    int64 SelFileSize;
    long GetSelPosition,LastSelPosition;
    long TotalFileCount;
    int64 TotalFileSize;
    int64 FreeDiskSize;
    clock_t LastUpdateTime;
    int Height,Columns;
    int LeftPos;
    int ShiftSelection;
    int MouseSelection;
    int SelectedFirst;
    /* $ 11.09.2000 SVS
       Переменная IsEmpty, указывающая на полностью пустую колонку
    */
    int IsEmpty;
    /* SVS $ */
    int AccessTimeUpdateRequired;

    PluginPanelItem *DataToDelete[32];
    int DataSizeToDelete[32];
    int DataToDeleteCount;
    int UpdateRequired,UpdateRequiredMode;
    int SortGroupsRead;
    int InternalProcessKey;

  private:
    /* $ 09.02.2001 IS
       Функции установления/считывания состояния режима
       "Помеченные файлы вперед"
    */
    void SetSelectedFirstMode(int Mode);
    int GetSelectedFirstMode(void) {return SelectedFirst;};
    /* IS $ */
    void DisplayObject();
    void DeleteListData(struct FileListItem *(&ListData),long &FileCount);
    void DeleteAllDataToDelete();
    void Up(int Count);
    void Down(int Count);
    void CorrectPosition();
    void ShowFileList(int Fast);
    void ShowList(int ShowStatus,int StartColumn);
    void SetShowColor(int Position);
    void ShowSelectedSize();
    void ShowTotalSize(struct OpenPluginInfo &Info);
    int ConvertName(char *SrcName,char *DestName,int MaxLength,int RightAlign,int ShowStatus);
    void Select(struct FileListItem *SelPtr,int Selection);
    void SelectFiles(int Mode);
    void ProcessEnter(int EnableExec,int SeparateWindow);
    void ChangeDir(char *NewDir);
    void CountDirSize();
    void ReadFileNames(int KeepSelection);
    void UpdatePlugin(int KeepSelection);
    void MoveSelection(struct FileListItem *FileList,long FileCount,
                       struct FileListItem *OldList,long OldFileCount);
    int GetSelCount();
    int GetSelName(char *Name,int &FileAttr,char *ShortName=NULL);
    void UngetSelName();
    void ClearLastGetSelection();
    long GetLastSelectedSize(int64 *Size);
    int GetLastSelectedItem(struct FileListItem *LastItem);
    int GetCurName(char *Name,char *ShortName);
    void PushPlugin(HANDLE hPlugin,char *HostFile);
    int PopPlugin(int EnableRestoreViewMode);
    int FindFile(char *Name);
    void CopyNames();
    void SelectSortMode();
    void ApplyCommand();
    void DescribeFiles();
    void CreatePluginItemList(struct PluginPanelItem *(&ItemList),int &ItemNumber);
    void DeletePluginItemList(struct PluginPanelItem *(&ItemList),int &ItemNumber);
    HANDLE OpenPluginForFile(char *FileName);
    int PreparePanelView(struct PanelViewSettings *PanelView);
    int PrepareColumnWidths(unsigned int *ColumnTypes,int *ColumnWidths,
                            int &ColumnCount,int FullScreen);
    void PrepareViewSettings(int ViewMode,struct OpenPluginInfo *PlugInfo);

    void PluginDelete();
    void PutDizToPlugin(FileList *DestPanel,struct PluginPanelItem *ItemList,
                        int ItemNumber,int Delete,int Move,DizList *SrcDiz,
                        DizList *DestDiz);
    void PluginGetFiles(char *DestPath,int Move);
    void PluginToPluginFiles(int Move);
    void PluginHostGetFiles();
    void PluginPutFilesToNew();
    void PluginPutFilesToAnother(int Move,Panel *AnotherPanel);
    void ProcessPluginCommand();
    void PluginClearSelection(struct PluginPanelItem *ItemList,int ItemNumber);
    void ProcessCopyKeys(int Key);
    void ReadSortGroups();

    static void TextToViewSettings(char *ColumnTitles,char *ColumnWidths,
           unsigned int *ViewColumnTypes,int *ViewColumnWidths,int &ColumnCount);
    static void ViewSettingsToText(unsigned int *ViewColumnTypes,
           int *ViewColumnWidths,int ColumnCount,char *ColumnTitles,
           char *ColumnWidths);

  public:
    FileList();
    ~FileList();

  public:
    int ProcessKey(int Key);
    int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
    void MoveToMouse(MOUSE_EVENT_RECORD *MouseEvent);
    void SetFocus();
    void Update(int Mode);
    int UpdateIfChanged();
    void CreateChangeNotification(int CheckTree);
    void CloseChangeNotification();
    void SortFileList(int KeepPosition);
    void SetViewMode(int ViewMode);
    void SetSortMode(int SortMode);
    void SetCurDir(char *NewDir,int ClosePlugin);
    int GetPrevSortMode();
    int GetPrevSortOrder();
    HANDLE OpenFilePlugin(char *FileName,int PushPrev);
    int GetFileName(char *Name,int Pos,int &FileAttr);
    int GetCurrentPos();
    int FindPartName(char *Name,int Next);
    int GoToFile(char *Name);
    int IsSelected(char *Name);
    void ProcessHostFile();
    void UpdateViewPanel();
    void CompareDir();
    void ClearSelection();
    void SaveSelection();
    void RestoreSelection();
    void EditFilter();
    void ReadDiz(struct PluginPanelItem *ItemList=NULL,int ItemLength=0);
    void DeleteDiz(char *Name,char *ShortName);
    void FlushDiz();
    void GetDizName(char *DizName);
    void CopyDiz(char *Name,char *ShortName,char *DestName,
                 char *DestShortName,DizList *DestDiz);
    int IsFullScreen();
    int IsCaseSensitive();
    int IsDizDisplayed();
    int IsColumnDisplayed(int Type);
    int GetPrevViewMode();
    void SetReturnCurrentFile(int Mode);
    void GetPluginInfo(struct PluginInfo *Info);
    void GetOpenPluginInfo(struct OpenPluginInfo *Info);
    void SetPluginMode(HANDLE hPlugin,char *PluginFile);
    void PluginGetPanelInfo(struct PanelInfo *Info);
    void PluginSetSelection(struct PanelInfo *Info);
    void SetPluginModified();
    int ProcessPluginEvent(int Event,void *Param);
    void SetTitle();
    HANDLE GetPluginHandle();
    static void SetFilePanelModes();
    static void SavePanelModes();
    static void ReadPanelModes();
    static int FileNameToPluginItem(char *Name,PluginPanelItem *pi);
    static void FileListToPluginItem(struct FileListItem *fi,struct PluginPanelItem *pi);
    static void PluginToFileListItem(struct PluginPanelItem *pi,struct FileListItem *fi);
    static int IsModeFullScreen(int Mode);
};

#endif	// __FILELIST_HPP__
