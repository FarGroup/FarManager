#ifndef __FILELIST_HPP__
#define __FILELIST_HPP__
/*
filelist.hpp

Файловая панель - общие функции

*/

/* Revision: 1.42 08.11.2004 $ */

/*
Modify:
  08.11.2004 WARP
    + GetShowColor()
  01.11.2004 SVS
    + ColumnsInGlobal
  24.05.2004 SVS
    + PluginsStackItem.PrevNumericSort
    + GetPrevNumericSort()
  18.05.2004 SVS
    ! IsNumeric() переехала из filelist.hpp в panel.hpp
  06.10.2003 SVS
    ! FileList::CreatePluginItemList() имеет доп.параметр - "добавлять '..'?"
      по умолчанию - "добавлять"
      В FileList::PluginGetPanelInfo() этот параметр = FALSE ("не добавлять")
  31.08.2003 SVS
    ! В FileList::CountDirSize() передается 1 параметр - DWORD, флаги плагина
  11.07.2003 SVS
    + FileList::IsNumeric()
  13.01.2003 SVS
    ! Доп.параметр у ConvertName() - файловые атрибуты, для того, чтобы
      обечпечить работу опции "Выравнивать расширения файлов"
  21.12.2002 SVS
    ! Добавляем третий параметр в FileList::ReadFileNames()
  18.06.2002 SVS
    + IfGoHome()
  14.05.2002 VVM
    + ResetLastUpdateTime() - сбросить время последнего обновления панели.
  12.04.2002 SVS
    ! Увеличим размер шортнэйма до NM
  12.04.2002 IS
    ! PluginPutFilesToAnother теперь int - возвращает то, что возвращает
      PutFiles
  11.04.2002 SVS
    ! Доп.Параметр у PluginGetPanelInfo - получать полную инфу или не полную
  10.04.2002 SVS
    + ProcessOneHostFile - обработка одного хост-файла
  08.04.2002 IS
    ! внедрение const
  05.04.2002 SVS
    ! CheckShortcutFolder стала самостоятельной и уехала в mix.cpp
  21.03.2002 SVS
    + CheckShortcutFolder()
  19.03.2002 DJ
    + UpdateIfRequired()
    + параметр IgnoreVisible
  19.02.2002 SVS
    ! ChangeDir() имеет доп.параметр.
  14.02.2002 VVM
    ! UpdateIfChanged принимает не булевый Force, а варианты из UIC_*
  27.11.2001 SVS
    + GetCurBaseName() выдает на гора имя файлового объекта под курсором
      с учетом вложенности панельного плагина, т.е. имя самого верхнего
      хост-файла в стеке.
  09.11.2001 IS
    + openBracket, closeBracket
  25.10.2001 SVS
    ! У функции CopyNames() 2 параметра:
      FillPathName - при копировании вставлять полный путь
      UNC          - учитывать так же UNC-путь (а так же с учетом symlink)
    + Функция CreateFullPathName() - конструирует на основе некоторых сведений
      полное имя файлового объекта.
  21.10.2001 SVS
    + AddPluginPrefix()
  01.10.2001 SVS
    + AddParentPoint() - общий код по добавлению ".."
    + UpdateColorItems() - колоризация итемов
  05.09.2001 SVS
    ! Вместо полей Color* в структе FileListItem используется
      структура HighlightDataColor
  17.08.2001 VVM
    + FileListItem.CRC32
  09.08.2001 SVS
    + virtual long GetFileCount() для нужд макросов :-)
  20.07.2001 SVS
    ! PluginPanelHelp переехала из help.hpp
  22.06.2001 SKV
    + Параметр Force у UpdateIfChanged.
  06.05.2001 DJ
    + перетрях #include
  30.04.2001 DJ
    + UpdateKeyBar()
  26.04.2001 VVM
    + Scroll() - прокрутить файлы, не двигая курсор
  25.04.2001 SVS
    + GetRealSelCount() - сейчас используется для макросов.
  24.04.2001 VVM
    + Функция для смены порядка сортировки.
  09.04.2001 SVS
    ! ChangeDir() возвращает FALSE, если файловая панель была закрыта
  25.02.2001 VVM
    + Доп. параметр у ReadDiz - dwFlags
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

#include "panel.hpp"
#include "dizlist.hpp"

class PanelFilter;

struct FileListItem
{
  char Selected;
  char PrevSelected;
  char ShowFolderSize;
  char ShortNamePresent;
  struct HighlightDataColor Colors;

  DWORD NumberOfLinks;
  DWORD UserFlags;
  DWORD UserData;

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
    PanelFilter *Filter;
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
    int64 SelFileSize;
    int64 TotalFileSize;
    int64 FreeDiskSize;
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
    void Scroll(int Count);
    void CorrectPosition();
    void ShowFileList(int Fast);
    void ShowList(int ShowStatus,int StartColumn);
    void SetShowColor(int Position);
    int GetShowColor(int Position);
    void ShowSelectedSize();
    void ShowTotalSize(struct OpenPluginInfo &Info);
    int ConvertName(char *SrcName,char *DestName,int MaxLength,int RightAlign,int ShowStatus,DWORD FileAttr);
    void Select(struct FileListItem *SelPtr,int Selection);
    void SelectFiles(int Mode);
    void ProcessEnter(int EnableExec,int SeparateWindow);
    /* $ 09.04.2001 SVS
       ChangeDir возвращает FALSE, eсли файловая панель была закрыта
    */
    BOOL ChangeDir(char *NewDir,BOOL IsUpdated=TRUE);
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
    int GetSelCount();
    int GetSelName(char *Name,int &FileAttr,char *ShortName=NULL);
    void UngetSelName();
    void ClearLastGetSelection();
    long GetLastSelectedSize(int64 *Size);
    int GetLastSelectedItem(struct FileListItem *LastItem);
    int GetCurName(char *Name,char *ShortName);
    int GetCurBaseName(char *Name,char *ShortName);
    void PushPlugin(HANDLE hPlugin,char *HostFile);
    int PopPlugin(int EnableRestoreViewMode);
    int FindFile(const char *Name);
    void CopyNames(int FillPathName=FALSE,int UNC=FALSE);
    void SelectSortMode();
    void ApplyCommand();
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
    void ReadSortGroups();
    void AddParentPoint(struct FileListItem *CurPtr,long CurFilePos);
    int  ProcessOneHostFile(int Idx);

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

    void CreateChangeNotification(int CheckTree);
    void CloseChangeNotification();
    void SortFileList(int KeepPosition);
    void SetViewMode(int ViewMode);
    void SetSortMode(int SortMode);
    virtual void ChangeSortOrder(int NewOrder);
    void SetCurDir(char *NewDir,int ClosePlugin);
    int GetPrevSortMode();
    int GetPrevSortOrder();
    int GetPrevViewMode();
    int GetPrevNumericSort();
    HANDLE OpenFilePlugin(char *FileName,int PushPrev);
    int GetFileName(char *Name,int Pos,int &FileAttr);
    int GetCurrentPos();
    int FindPartName(char *Name,int Next);
    int GoToFile(const char *Name);
    int IsSelected(char *Name);
    void ProcessHostFile();
    void UpdateViewPanel();
    void CompareDir();
    void ClearSelection();
    void SaveSelection();
    void RestoreSelection();
    void EditFilter();
    void ReadDiz(struct PluginPanelItem *ItemList=NULL,int ItemLength=0, DWORD dwFlags=0);
    void DeleteDiz(char *Name,char *ShortName);
    void FlushDiz();
    void GetDizName(char *DizName);
    void CopyDiz(char *Name,char *ShortName,char *DestName,
                 char *DestShortName,DizList *DestDiz);
    int IsFullScreen();
    int IsCaseSensitive();
    int IsDizDisplayed();
    int IsColumnDisplayed(int Type);
    void SetReturnCurrentFile(int Mode);
    void GetPluginInfo(struct PluginInfo *Info);
    void GetOpenPluginInfo(struct OpenPluginInfo *Info);
    void SetPluginMode(HANDLE hPlugin,char *PluginFile);
    void PluginGetPanelInfo(struct PanelInfo *Info,int FullInfo=TRUE);
    void PluginSetSelection(struct PanelInfo *Info);
    void SetPluginModified();
    int ProcessPluginEvent(int Event,void *Param);
    void SetTitle();
    int PluginPanelHelp(HANDLE hPlugin);
    long GetFileCount() {return FileCount;}
    char *CreateFullPathName(char *Name,char *ShortName,DWORD FileAttr,
                            char *Dest,int SizeDest,int UNC);

    /* $ 30.04.2001 DJ
       добавлен UpdateKeyBar()
    */
    virtual BOOL UpdateKeyBar();
    /* DJ $ */
    void UpdateColorItems(void);

    virtual void IfGoHome(char Drive);

    /* 14.05.2002 VVM
      + Сбросить время последнего обновления панели */
    void ResetLastUpdateTime() {LastUpdateTime = 0;}
    /* VVM $ */
    HANDLE GetPluginHandle();
    int GetRealSelCount();
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
