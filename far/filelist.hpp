#ifndef __FILELIST_HPP__
#define __FILELIST_HPP__
/*
filelist.hpp

Файловая панель - общие функции

*/

#include "panel.hpp"
#include "dizlist.hpp"
#include "filefilter.hpp"
#include "TList.hpp"

struct FileListItem
{
  char Selected;
  char PrevSelected;
  char ShowFolderSize;
  char ShortNamePresent;
  struct HighlightDataColor Colors;

  DWORD NumberOfLinks;
  DWORD UserFlags;
  DWORD_PTR UserData;

  int Position;
  int SortGroup;
  char *DizText;
  char DeleteDiz;
  char Owner[40];
  char **CustomColumnData;
  int CustomColumnNumber;
  DWORD CRC32;

  // Аля  WIN32_FIND_DATA - эту часть желательно держать в таком виде
  DWORD FileAttr;
  FILETIME CreationTime;
  FILETIME AccessTime;
  FILETIME WriteTime;
  DWORD UnpSizeHigh;
  DWORD UnpSize;
  DWORD PackSizeHigh;     // WIN32_FIND_DATA.dwReserved0
  DWORD PackSize;         // WIN32_FIND_DATA.dwReserved1
  char Name[NM];
  char ShortName[NM];

  DWORD ReparseTag;
};

struct PluginsStackItem
{
  HANDLE hPlugin;
  char HostFile[NM];
  int Modified;
  int PrevViewMode;
  int PrevSortMode;
  int PrevSortOrder;
  int PrevNumericSort;
  struct PanelViewSettings PrevViewSettings;
};

struct PrevDataItem
{
  struct FileListItem *PrevListData;
  long PrevFileCount;
  char PrevName[NM];
  long PrevTopFile;
};

enum {NAME_COLUMN=0,SIZE_COLUMN,PACKED_COLUMN,DATE_COLUMN,TIME_COLUMN,
      MDATE_COLUMN,CDATE_COLUMN,ADATE_COLUMN,ATTR_COLUMN,DIZ_COLUMN,
      OWNER_COLUMN,NUMLINK_COLUMN,
      CUSTOM_COLUMN0,CUSTOM_COLUMN1,CUSTOM_COLUMN2,CUSTOM_COLUMN3,
      CUSTOM_COLUMN4,CUSTOM_COLUMN5,CUSTOM_COLUMN6,CUSTOM_COLUMN7,
      CUSTOM_COLUMN8,CUSTOM_COLUMN9};

class FileList:public Panel
{
  private:
    FileFilter *Filter;
    DizList Diz;
    int DizRead;
    /* $ 09.11.2001 IS
         Открывающий и закрывающий символ, которые используются для показа
         имени, которое не помещается в панели. По умолчанию - фигурные скобки.
    */
    char openBracket[2], closeBracket[2];
    /* IS $ */
    char PluginDizName[NM];
    struct FileListItem *ListData;
    long FileCount;
    HANDLE hPlugin;
    struct PrevDataItem *PrevDataStack;
    int PrevDataStackSize;
    struct PluginsStackItem *PluginsStack;
    int PluginsStackSize;
    HANDLE hListChange;
    long UpperFolderTopFile,LastCurFile;
    long ReturnCurrentFile;
    long SelFileCount;
    long GetSelPosition,LastSelPosition;
    long TotalFileCount;
    __int64 SelFileSize;
    __int64 TotalFileSize;
    __int64 FreeDiskSize;
    clock_t LastUpdateTime;
    int Height,Columns;

    int ColumnsInGlobal;

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

    struct DataToDeleteItem
    {
      PluginPanelItem* Item;
      int Size;
    };
    TList<DataToDeleteItem> DataToDelete;

    int UpdateRequired,UpdateRequiredMode;
    int SortGroupsRead;
    int InternalProcessKey;

    BOOL Is_FS_NTFS;

  private:
    /* $ 09.02.2001 IS
       Функции установления/считывания состояния режима
       "Помеченные файлы вперед"
    */
    virtual void SetSelectedFirstMode(int Mode);
    virtual int GetSelectedFirstMode(void) {return SelectedFirst;};
    /* IS $ */
    virtual void DisplayObject();
    void DeleteListData(struct FileListItem *(&ListData),long &FileCount);
    void DeleteAllDataToDelete();
    void Up(int Count);
    void Down(int Count);
    void Scroll(int Count);
    void CorrectPosition();
    void ShowFileList(int Fast);
    void ShowList(int ShowStatus,int StartColumn);
    void SetShowColor(int Position, int ColorType=HIGHLIGHTCOLORTYPE_FILE);
    int  GetShowColor(int Position, int ColorType);
    void ShowSelectedSize();
    void ShowTotalSize(struct OpenPluginInfo &Info);
    int ConvertName(char *SrcName,char *DestName,int MaxLength,int RightAlign,int ShowStatus,DWORD FileAttr);
    void Select(struct FileListItem *SelPtr,int Selection);
    void SelectFiles(int Mode);
    void ProcessEnter(int EnableExec,int SeparateWindow);
    /* $ 09.04.2001 SVS
       ChangeDir возвращает FALSE, eсли файловая панель была закрыта
    */
    BOOL ChangeDir(const char *NewDir,BOOL IsUpdated=TRUE);
    /* SVS $ */
    void CountDirSize(DWORD PluginFlags);
    /* $ 19.03.2002 DJ
       IgnoreVisible - обновить, даже если панель невидима
    */
    void ReadFileNames(int KeepSelection, int IgnoreVisible, int DrawMessage);
    void UpdatePlugin(int KeepSelection, int IgnoreVisible);
    /* DJ $ */
    void MoveSelection(struct FileListItem *FileList,long FileCount,
                       struct FileListItem *OldList,long OldFileCount);
    virtual int GetSelCount();
    virtual int GetSelName(char *Name,int &FileAttr,char *ShortName=NULL,WIN32_FIND_DATA *fd=NULL);
    virtual void UngetSelName();
    virtual void ClearLastGetSelection();
    virtual unsigned __int64 GetLastSelectedSize();
    virtual int GetLastSelectedItem(struct FileListItem *LastItem);
    virtual int GetCurName(char *Name,char *ShortName);
    virtual int GetCurBaseName(char *Name,char *ShortName);
    void PushPlugin(HANDLE hPlugin,char *HostFile);
    int PopPlugin(int EnableRestoreViewMode);
    void CopyNames(int FillPathName=FALSE,int UNC=FALSE);
    void SelectSortMode();
    bool ApplyCommand();
    void DescribeFiles();
    void CreatePluginItemList(struct PluginPanelItem *(&ItemList),int &ItemNumber,BOOL AddTwoDot=TRUE);
    void DeletePluginItemList(struct PluginPanelItem *(&ItemList),int &ItemNumber);
    HANDLE OpenPluginForFile(char *FileName,DWORD FileAttr=0);
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
    // возвращает то, что возвращает PutFiles
    int PluginPutFilesToAnother(int Move,Panel *AnotherPanel);
    void ProcessPluginCommand();
    void PluginClearSelection(struct PluginPanelItem *ItemList,int ItemNumber);
    void ProcessCopyKeys(int Key);
    void ReadSortGroups(bool UpdateFilterCurrentTime=true);
    void AddParentPoint(struct FileListItem *CurPtr,long CurFilePos);
    int  ProcessOneHostFile(int Idx);

    static void TextToViewSettings(char *ColumnTitles,char *ColumnWidths,
           unsigned int *ViewColumnTypes,int *ViewColumnWidths,int &ColumnCount);
    static void ViewSettingsToText(unsigned int *ViewColumnTypes,
           int *ViewColumnWidths,int ColumnCount,char *ColumnTitles,
           char *ColumnWidths=NULL);

  public:
    FileList();
    virtual ~FileList();

  public:
    virtual int ProcessKey(int Key);
    virtual int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
    virtual void MoveToMouse(MOUSE_EVENT_RECORD *MouseEvent);
    virtual __int64 VMProcess(int OpCode,void *vParam=NULL,__int64 iParam=0);
    virtual void SetFocus();
    virtual void Update(int Mode);
    /*$ 22.06.2001 SKV
      Параметр для игнорирования времени последнего Update.
      Используется для Update после исполнения команды.
    */
    virtual int UpdateIfChanged(int UpdateMode);
    /* SKV$*/

    /* $ 19.03.2002 DJ
       UpdateIfRequired() - обновить, если апдейт был пропущен из-за того,
       что панель невидима
    */
    virtual void UpdateIfRequired();
    /* DJ $ */

    virtual int SendKeyToPlugin(DWORD Key,BOOL Pred=FALSE);
    void CreateChangeNotification(int CheckTree);
    virtual void CloseChangeNotification();
    virtual void SortFileList(int KeepPosition);
    virtual void SetViewMode(int ViewMode);
    virtual void SetSortMode(int SortMode);
    virtual void ChangeSortOrder(int NewOrder);
    virtual BOOL SetCurDir(const char *NewDir,int ClosePlugin);
    virtual int GetPrevSortMode();
    virtual int GetPrevSortOrder();
    virtual int GetPrevViewMode();
    virtual int GetPrevNumericSort();
    HANDLE OpenFilePlugin(char *FileName,int PushPrev);
    virtual int GetFileName(char *Name,int Pos,int &FileAttr);
    virtual int GetCurrentPos();
    virtual int FindPartName(char *Name,int Next,int Direct=1,int ExcludeSets=0);
    virtual int GoToFile(long idxItem);
    virtual int GoToFile(const char *Name,BOOL OnlyPartName=FALSE);
    virtual long FindFile(const char *Name,BOOL OnlyPartName=FALSE);
    virtual int IsSelected(char *Name);

    virtual long FindFirst(const char *Name);
    virtual long FindNext(int StartPos, const char *Name);

    void ProcessHostFile();
    virtual void UpdateViewPanel();
    virtual void CompareDir();
    virtual void ClearSelection();
    virtual void SaveSelection();
    virtual void RestoreSelection();
    virtual void EditFilter();
    virtual void ReadDiz(struct PluginPanelItem *ItemList=NULL,int ItemLength=0, DWORD dwFlags=0);
    virtual void DeleteDiz(char *Name,char *ShortName);
    virtual void FlushDiz();
    virtual void GetDizName(char *DizName);
    virtual void CopyDiz(char *Name,char *ShortName,char *DestName,
                         char *DestShortName,DizList *DestDiz);
    virtual int IsFullScreen();
    int IsCaseSensitive();
    virtual int IsDizDisplayed();
    virtual int IsColumnDisplayed(int Type);
    virtual int GetColumnsCount(){ return Columns;};
    virtual void SetReturnCurrentFile(int Mode);
    virtual void GetPluginInfo(struct PluginInfo *Info);
    virtual void GetOpenPluginInfo(struct OpenPluginInfo *Info);
    virtual void SetPluginMode(HANDLE hPlugin,char *PluginFile,bool SendOnFocus=false);
    void PluginGetPanelInfo(struct PanelInfo *Info,int FullInfo=TRUE);
    void PluginSetSelection(struct PanelInfo *Info);
    virtual void SetPluginModified();
    virtual int ProcessPluginEvent(int Event,void *Param);
    virtual void SetTitle();
    //virtual const char *GetTitle(char *Title,int LenTitle,int TruncSize);
    int PluginPanelHelp(HANDLE hPlugin);
    virtual long GetFileCount() {return FileCount;}
    char *CreateFullPathName(char *Name,char *ShortName,DWORD FileAttr,
                            char *Dest,int SizeDest,int UNC,int ShortNameAsIs=TRUE);

    virtual BOOL GetItem(int Index,void *Dest);
    virtual BOOL UpdateKeyBar();

    virtual void IfGoHome(char Drive);

    /* 14.05.2002 VVM
      + Сбросить время последнего обновления панели */
    void ResetLastUpdateTime() {LastUpdateTime = 0;}
    /* VVM $ */
    virtual HANDLE GetPluginHandle();
    virtual int GetRealSelCount();
    static void SetFilePanelModes();
    static void SavePanelModes();
    static void ReadPanelModes();
    static int FileNameToPluginItem(char *Name,PluginPanelItem *pi);
    static void FileListToPluginItem(struct FileListItem *fi,struct PluginPanelItem *pi);
    static void PluginToFileListItem(struct PluginPanelItem *pi,struct FileListItem *fi);
    static int IsModeFullScreen(int Mode);
    static char* AddPluginPrefix(FileList *SrcPanel,char *Prefix);
};

#endif  // __FILELIST_HPP__
