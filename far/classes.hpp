class Panel;
class Modal;
class FileEditor;
class Viewer;
class VMenu;
class History;

#define MSG(ID) FarMSG(ID)

class int64
{
  public:
    int64();
    int64(DWORD n);
    int64(DWORD HighPart,DWORD LowPart);

    int64 operator = (int64 n);
    int64 operator << (int n);
    int64 operator >> (int n);

    friend int64 operator / (int64 n1,int64 n2);
    friend int64 operator * (int64 n1,int64 n2);
    friend int64 operator % (int64 n1,int64 n2);
    friend int64 operator += (int64 &n1,int64 n2);
    friend int64 operator -= (int64 &n1,int64 n2);
    friend int64 operator + (int64 n1,int64 n2);
    friend int64 operator - (int64 n1,int64 n2);
    friend int64 operator ++ (int64 &n);
    friend int64 operator -- (int64 &n);
    friend bool operator == (int64 n1,int64 n2);
    friend bool operator > (int64 n1,int64 n2);
    friend bool operator < (int64 n1,int64 n2);
    friend bool operator != (int64 n1,int64 n2);
    friend bool operator >= (int64 n1,int64 n2);
    friend bool operator <= (int64 n1,int64 n2);

    void Set(DWORD HighPart,DWORD LowPart);
    void itoa(char *Str);

    DWORD LowPart,HighPart;
};


class Language
{
  private:
    void ConvertString(char *Src,char *Dest);
    char **MsgAddr;
    char *MsgList;
    long MsgSize;
    int MsgCount;
    char MessageFile[NM];
  public:
    Language();
    int Init(char *Path);
    void Close();
    char* GetMsg(int MsgId);
    static FILE* OpenLangFile(char *Path,char *Mask,char *Language,char *FileName);
    static int GetLangParam(FILE *SrcFile,char *ParamName,char *Param1,char *Param2);
    static int Select(int HelpLanguage,VMenu **MenuPtr);
};

#include "plugins.hpp"

class SaveScreen
{
  private:
    void SaveArea(int X1,int Y1,int X2,int Y2);
    char *ScreenBuf;
    int CurPosX,CurPosY,CurVisible,CurSize;
    int X1,Y1,X2,Y2;
    int RealScreen;
  public:
    SaveScreen();
    SaveScreen(int RealScreen);
    SaveScreen(int X1,int Y1,int X2,int Y2,int RealScreen=FALSE);
    ~SaveScreen();
    void RestoreArea(int RestoreCursor=TRUE);
    void Discard();
    void AppendArea(SaveScreen *NewArea);
    CHAR_INFO* GetBufferAddress() {return((CHAR_INFO *)ScreenBuf);};
};


class BaseInput
{
  public:
    virtual int ProcessKey(int Key) { return(0); };
    virtual int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent) { return(0); };
};


class ScreenObject:public BaseInput
{
  private:
    virtual void DisplayObject() {};
    SaveScreen *ShadowSaveScr;
    int Visible;
    int Type;
    int EnableRestoreScreen;
    int SetPositionDone;
  protected:
    int X1,Y1,X2,Y2;
    int ObjWidth,ObjHeight;
  public:
    SaveScreen *SaveScr;
    ScreenObject();
    virtual ~ScreenObject();
    virtual void Hide();
    virtual void Show();
    virtual void ShowConsoleTitle() {};
    void SavePrevScreen();
    void Redraw();
    void SetPosition(int X1,int Y1,int X2,int Y2);
    void GetPosition(int& X1,int& Y1,int& X2,int& Y2);
    int IsVisible() { return(Visible); };
    void SetVisible(int Visible) {ScreenObject::Visible=Visible;};
    void SetRestoreScreenMode(int Mode) {EnableRestoreScreen=Mode;};
    void Shadow();
};


struct MacroRecord
{
  int Key;
  int *Buffer;
  int BufferSize;
  int Mode;
  int DisableOutput;
  int EmptyCommandLine;
  int NotEmptyCommandLine;
  int RunAfterStart;
};


enum {MACRO_SHELL,MACRO_VIEWER,MACRO_EDITOR,MACRO_DIALOG,MACRO_SEARCH,
      MACRO_DISKS,MACRO_MAINMENU,MACRO_HELP,MACRO_OTHER};

class KeyMacro
{
  private:
    void ReadMacros(int ReadMode);
    void KeyToText(int Key,char *KeyName);
    int GetMacroSettings(int &DisableOutput,int &RunAfterStart,
                         int &EmptyCommandLine,int &NotEmptyCommandLine);

    class LockScreen *LockScr;

    struct MacroRecord *Macros;
    int MacrosNumber;
    int Recording;
    int *RecBuffer;
    int RecBufferSize;
    int Executing;
    int ExecMacroPos;
    int ExecKeyPos;
    int InternalInput;
    int Mode;
    int StartMode;
    int StartMacroPos;
  public:
    KeyMacro();
    ~KeyMacro();
    int ProcessKey(int Key);
    int GetKey();
    int PeekKey();
    int IsRecording() {return(Recording);};
    int IsExecuting() {return(Executing);};
    void SaveMacros();
    void SetMode(int Mode) {KeyMacro::Mode=Mode;};
    int GetMode() {return(Mode);};
    void RunStartMacro();
};


struct DizRecord
{
  char *DizText;
  int Deleted;
};

class DizList
{
  private:
    int GetDizPos(char *Name,char *ShortName,int *TextPos);
    int GetDizPosEx(char *Name,char *ShortName,int *TextPos);
    void AddRecord(char *DizText);
    void BuildIndex();
    char DizFileName[NM];
    struct DizRecord *DizData;
    int DizCount;
    int *IndexData;
    int IndexCount;
  public:
    DizList();
    ~DizList();
    void Read(char *Path,char *DizName=NULL);
    void Reset();
    char* GetDizTextAddr(char *Name,char *ShortName,DWORD FileSize);
    int DeleteDiz(char *Name,char *ShortName);
    int Flush(char *Path,char *DizName=NULL);
    void AddDiz(char *Name,char *ShortName,char *DizText);
    int CopyDiz(char *Name,char *ShortName,char *DestName,
                 char *DestShortName,DizList *DestDiz);
    void GetDizName(char *DizName);
};


struct GroupSortData
{
  char Masks[256];
  int Group;
};


class GroupSort
{
  private:
    int EditGroupsMenu(int Pos);
    struct GroupSortData *GroupData;
    int GroupCount;
  public:
    GroupSort();
    ~GroupSort();
    int GetGroup(char *Path);
    void EditGroups();
};


struct FilterDataRecord
{
  char Title[128];
  char Masks[256];
  int LeftPanelInclude;
  int LeftPanelExclude;
  int RightPanelInclude;
  int RightPanelExclude;
};


class PanelFilter
{
  private:
    void SaveFilterFile();
    int SaveFilterData();
    int EditRecord(char *Title,char *Masks);
    int ShowFilterMenu(int Pos,int FirstCall);
    void AddMasks(char *Masks,int Exclude);
    void ProcessSelection(VMenu &FilterList);
    void SaveFilters();
    Panel *HostPanel;
    char *FilterMask;
    int FilterMaskCount;
    char *ExcludeFilterMask;
    int ExcludeFilterMaskCount;
  public:
    PanelFilter(Panel *HostPanel);
    ~PanelFilter();
    static void InitFilter();
    static void CloseFilter();
    static void SwapFilter();
    static void SaveSelection();
    void FilterEdit();
    int CheckName(char *Name);
    bool IsEnabled();
};

enum {
  COLUMN_MARK        =  0x80000000,
  COLUMN_NAMEONLY    =  0x40000000,
  COLUMN_RIGHTALIGN  =  0x20000000,
  COLUMN_FORMATTED   =  0x10000000,
  COLUMN_COMMAS      =  0x08000000,
  COLUMN_THOUSAND    =  0x04000000,
  COLUMN_BRIEF       =  0x02000000,
  COLUMN_MONTH       =  0x01000000,
};

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
  int FolderUpperCase;
  int FileLowerCase;
  int FileUpperToLowerCase;
  int CaseSensitiveSort;
};

enum {MODALTREE_ACTIVE=1,MODALTREE_PASSIVE=2};
enum {NORMAL_PANEL,PLUGIN_PANEL};

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


enum {ARCHIVE_NONE,ARCHIVE_RAR,ARCHIVE_ZIP,ARCHIVE_ARJ,ARCHIVE_LZH};

struct FileListItem
{
  char Selected;
  char PrevSelected;
  char ShowFolderSize;
  char ShortNamePresent;
  unsigned char Color,SelColor,CursorColor,CursorSelColor;
  unsigned char MarkChar;
  DWORD UnpSizeHigh;
  DWORD UnpSize;
  DWORD PackSizeHigh;
  DWORD PackSize;
  DWORD NumberOfLinks;
  DWORD UserFlags;
  DWORD UserData;

  FILETIME WriteTime;
  FILETIME CreationTime;
  FILETIME AccessTime;

  DWORD FileAttr;
  int Position;
  int SortGroup;
  char *DizText;
  char DeleteDiz;
  char Owner[40];
  char Name[NM];
  char ShortName[80];
  char **CustomColumnData;
  int CustomColumnNumber;
};


struct PluginsStackItem
{
  HANDLE hPlugin;
  char HostFile[NM];
  int Modified;
  int PrevViewMode;
  int PrevSortMode;
  int PrevSortOrder;
};


struct PrevDataItem
{
  struct FileListItem *PrevListData;
  long PrevFileCount;
  char PrevName[NM];
};


class FileList:public Panel
{
  private:
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
    static void TextToViewSettings(char *ColumnTitles,char *ColumnWidths,
           unsigned int *ViewColumnTypes,int *ViewColumnWidths,int &ColumnCount);
    static void ViewSettingsToText(unsigned int *ViewColumnTypes,
           int *ViewColumnWidths,int ColumnCount,char *ColumnTitles,
           char *ColumnWidths);

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
    int TranslateKeyToVK(int Key,int &VirtKey,int &ControlState);
    void ProcessCopyKeys(int Key);
    void ReadSortGroups();

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
    int AccessTimeUpdateRequired;

    PluginPanelItem *DataToDelete[32];
    int DataSizeToDelete[32];
    int DataToDeleteCount;
    int UpdateRequired,UpdateRequiredMode;
    int SortGroupsRead;
    int InternalProcessKey;
  public:
    FileList();
    ~FileList();
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
    static int IsModeFullScreen(int Mode);
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
};


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
};


class QuickView:public Panel
{
  private:
    void DisplayObject();
    void PrintText(char *Str);
    Viewer *QView;
    char CurFileName[NM];
    char CurFileType[80];
    char TempName[NM];
    int Directory;
    unsigned long DirCount,FileCount,ClusterSize;
    int64 FileSize,CompressedFileSize,RealFileSize;
  public:
    QuickView();
    ~QuickView();
    int ProcessKey(int Key);
    int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
    void Update(int Mode);
    void ShowFile(char *FileName,int TempFile,HANDLE hDirPlugin);
    void CloseFile();
    void QViewDelTempName();
    int UpdateIfChanged();
};


class InfoList:public Panel
{
  private:
    void DisplayObject();
    void ShowDirDescription();
    void ShowPluginDescription();
    void PrintText(char *Str);
    void PrintText(int MsgID);
    void PrintInfo(char *Str);
    void PrintInfo(int MsgID);
    char DizFileName[NM];
  public:
    InfoList();
    int ProcessKey(int Key);
    int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
    void Update(int Mode) {Redraw();};
};


struct ColorItem
{
  int StartPos;
  int EndPos;
  int Color;
};

class Edit:public ScreenObject
{
  private:
    void DisplayObject();
    void ShowString(char *ShowStr,int TabSelStart,int TabSelEnd);
    int InsertKey(int Key);
    int RecurseProcessKey(int Key);
    void DeleteBlock();
    void ApplyColor();
    char *Str;
    struct CharTableSet *TableSet;
    struct ColorItem *ColorList;
    int ColorCount;
    int StrSize;
    int CurPos;
    int Color,SelColor;
    int LeftPos;
    int ConvertTabs;
    int CursorPos;
    int EndType;
    int MaxLength;
    int SelStart,SelEnd;
    char MarkingBlock;
    char ClearFlag;
    char PasswordMode;
    char EditBeyondEnd;
    char Overtype;
    char EditorMode;
  public:
    Edit();
    ~Edit();
    void Show() {DisplayObject();}
    void FastShow();
    int ProcessKey(int Key);
    int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
    void SetObjectColor(int Color,int SelColor=0xf);
    void GetString(char *Str,int MaxSize);
    char* GetStringAddr();
    void SetString(char *Str);
    void SetBinaryString(char *Str,int Length);
    void GetBinaryString(char **Str,char **EOL,int &Length);
    void SetEOL(char *EOL);
    int GetSelString(char *Str,int MaxSize);
    int GetLength();
    void InsertString(char *Str);
    void InsertBinaryString(char *Str,int Length);
    int Search(char *Str,int Position,int Case,int Reverse);
    void SetClearFlag(int Flag) {ClearFlag=Flag;}
    void SetCurPos(int NewPos) {CurPos=NewPos;}
    int GetCurPos() {return(CurPos);}
    int GetTabCurPos();
    int GetLeftPos() {return(LeftPos);}
    void SetLeftPos(int NewPos) {LeftPos=NewPos;}
    void SetTabCurPos(int NewPos);
    void SetPasswordMode(int Mode) {PasswordMode=Mode;};
    void SetMaxLength(int Length) {MaxLength=Length;};
    void SetOvertypeMode(int Mode) {Overtype=Mode;};
    int GetOvertypeMode() {return(Overtype);};
    void SetConvertTabs(int Mode) {ConvertTabs=Mode;};
    int RealPosToTab(int Pos);
    int TabPosToReal(int Pos);
    void SetTables(struct CharTableSet *TableSet);
    void Select(int Start,int End);
    void AddSelect(int Start,int End);
    void GetSelection(int &Start,int &End);
    void GetRealSelection(int &Start,int &End);
    void SetEditBeyondEnd(int Mode) {EditBeyondEnd=Mode;};
    void SetEditorMode(int Mode) {EditorMode=Mode;};
    void ReplaceTabs();
    void AddColor(struct ColorItem *col);
    int DeleteColor(int ColorPos);
    int GetColor(struct ColorItem *col,int Item);
    static void DisableEditOut(int Disable);
    static void DisableEncode(int Disable);
};


class CommandLine:public ScreenObject
{
  private:
    void DisplayObject();
    int CmdExecute(char *CmdLine,int AlwaysWaitFinish,int SeparateWindow,
                   int DirectRun);
    int ProcessOSCommands(char *CmdLine);
    void GetPrompt(char *DestStr);
    Edit CmdStr;
    char CurDir[NM];
    char LastCmdStr[256];
    int LastCmdPartLength;
  public:
    CommandLine();
    int ProcessKey(int Key);
    int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
    void GetCurDir(char *CurDir);
    void SetCurDir(char *CurDir);
    void GetString(char *Str,int MaxSize);
    void SetString(char *Str);
    int GetLength() {return(CmdStr.GetLength());};
    void ExecString(char *Str,int AlwaysWaitFinish,int SeparateWindow=FALSE,
                    int DirectRun=FALSE);
    void InsertString(char *Str);
    void SetCurPos(int Pos);
    int GetCurPos();
};


class KeyBar:public ScreenObject
{
  private:
    void DisplayObject();
    BaseInput *Owner;
    char KeyName[12][10];
    char ShiftKeyName[12][10],AltKeyName[12][10],CtrlKeyName[12][10];
    int KeyCount,ShiftKeyCount,AltKeyCount,CtrlKeyCount;
    int AltState,CtrlState,ShiftState;
    int DisableMask;
  public:
    KeyBar();
    void SetOwner(BaseInput *Owner);
    int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
    void Set(char **Key,int KeyCount);
    void SetShift(char **Key,int KeyCount);
    void SetAlt(char **Key,int KeyCount);
    void SetCtrl(char **Key,int KeyCount);
    void SetDisableMask(int Mask);
    void Change(char *NewStr,int Pos);
    void RedrawIfChanged();
};


class MenuBar:public ScreenObject
{
  private:
    void DisplayObject();
};


class NamesList
{
  private:
    char *Names;
    char *CurName;
    int CurNamePos;
    int NamesNumber;
    int NamesSize;

    char CurDir[NM];
  public:
    NamesList();
    ~NamesList();
    void AddName(char *Name);
    bool GetNextName(char *Name);
    bool GetPrevName(char *Name);
    void SetCurName(char *Name);
    void MoveData(NamesList *Dest);
    void GetCurDir(char *Dir);
    void SetCurDir(char *Dir);
};


class Viewer:public ScreenObject
{
  private:
    void DisplayObject();
    void Up();
    void ShowHex();
    void ShowUp();
    void ShowStatus();
    void ReadString(char *Str,int MaxSize,int StrSize,int &SelPos,int &SelSize);
    int CalcStrSize(char *Str,int Length);
    void ChangeViewKeyBar();
    void SetCRSym();
    void Search(int Next,int FirstChar);
    void ConvertToHex(char *SearchStr,int &SearchLength);
    int HexToNum(int Hex);
    int vread(char *Buf,int Size,FILE *SrcFile);
    int vseek(FILE *SrcFile,long Offset,int Whence);
    unsigned long vtell(FILE *SrcFile);
    int vgetc(FILE *SrcFile);
    void GoTo();
    void SetFileSize();

    NamesList ViewNamesList;
    KeyBar *ViewKeyBar;
    char OutStr[MAXSCRY+1][528];
    int StrFilePos[MAXSCRY+1];
    char FileName[NM];
    char FullFileName[NM];
    FILE *ViewFile;
    WIN32_FIND_DATA ViewFindData;

    unsigned char LastSearchStr[256];
    int LastSearchCase,LastSearchReverse,LastSearchHex;

    struct CharTableSet TableSet;
    int UseDecodeTable,TableNum,AnsiText;
    int Unicode;

    int Wrap,Hex;

    unsigned long FilePos;
    unsigned long SecondPos;
    unsigned long LastScrPos;
    unsigned long FileSize;
    unsigned long LastSelPos;
    int LeftPos;
    int LastPage;
    int CRSym;
    int SelectPos,SelectSize;
    int ViewY1;
    int ShowStatusLine,HideCursor;
    char TempViewName[NM];
    char Title[512];
    char PluginData[NM*2];
    int TableChangedByUser;
    int ReadStdin;
    int InternalKey;

    unsigned long SavePosAddr[10];
    int SavePosLeft[10];

    unsigned long UndoAddr[128];
    int UndoLeft[128];
    int LastKeyUndo;
  public:
    Viewer();
    ~Viewer();
    int OpenFile(char *Name);
    void SetViewKeyBar(KeyBar *ViewKeyBar);
    int ProcessKey(int Key);
    int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
    void SetStatusMode(int Mode);
    void EnableHideCursor(int HideCursor);
    int GetWrapMode();
    void SetWrapMode(int Wrap);
    void KeepInitParameters();
    void GetFileName(char *Name);
    void ShowConsoleTitle();
    void SetTempViewName(char *Name);
    void SetTitle(char *Title);
    long GetFilePos();
    void SetFilePos(long Pos);
    void SetPluginData(char *PluginData);
    void SetNamesList(NamesList *List);
};


struct HighlightData
{
  char Masks[256];
  unsigned int IncludeAttr;
  unsigned int ExcludeAttr;
  unsigned int Color,SelColor,CursorColor,CursorSelColor,MarkChar;
};


class HighlightFiles
{
  private:
    void SaveHiData();
    int EditRecord(int RecPos,int New);
    struct HighlightData *HiData;
    int HiDataCount;
    int StartHiDataCount;
  public:
    HighlightFiles();
    ~HighlightFiles();
    void GetHiColor(char *Path,int Attr,unsigned char &Color,
                    unsigned char &SelColor,unsigned char &CursorColor,
                    unsigned char &CursorSelColor,unsigned char &MarkChar);
    void HiEdit(int MenuPos);
};


#define MAX_POSITIONS 64

class FilePositionCache
{
  private:
    char Names[MAX_POSITIONS][3*NM];
    unsigned int Positions1[MAX_POSITIONS],Positions2[MAX_POSITIONS];
    unsigned int Positions3[MAX_POSITIONS],Positions4[MAX_POSITIONS];
    unsigned int Positions5[MAX_POSITIONS];
    int CurPos;
  public:
    FilePositionCache();
    void AddPosition(char *Name,unsigned int Position1,unsigned int Position2,
                     unsigned int Position3,unsigned int Position4,
                     unsigned int Position5);
    void GetPosition(char *Name,unsigned int &Position1,unsigned int &Position2,
                     unsigned int &Position3,unsigned int &Position4,
                     unsigned int &Position5);
    void Read(char *Key);
    void Save(char *Key);
};


class Manager
{
  private:
    void ActivateNextWindow();

    Modal **ModalList;
    int ModalCount;
    int ModalPos;
    int NextViewer;
    char NextName[NM];
    int NextPos;
    int UpdateRequired;
  public:
    Manager();
    void AddModal(Modal *NewModal);
    void NextModal(int Increment);
    void CloseAll();
    BOOL IsAnyModalModified(int Activate);
    void SelectModal();
    int GetModalCount() {return(ModalCount);};
    void GetModalTypesCount(int &Viewers,int &Editors);
    void SetModalPos(int NewPos);
    int FindModalByFile(int ModalType,char *FileName);
    void ShowBackground();
    void SetNextWindow(int Viewer,char *Name,long Pos);
};


class ControlObject:public BaseInput
{
  private:
    Panel* ControlObject::CreatePanel(int Type);
    void DeletePanel(Panel *Deleted);
    void ShowCopyright();
    int EndLoop;
    int LastLeftType,LastRightType;
    int LeftStateBeforeHide,RightStateBeforeHide,HideState;
    Panel *LastLeftFilePanel,*LastRightFilePanel;
  public:
    ControlObject();
    ~ControlObject();
    void Init();
    int ProcessKey(int Key);
    int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
    void EnterMainLoop();
    void ExitMainLoop(int Ask);
    Panel* GetAnotherPanel(Panel *Current);
    Panel* ChangePanelToFilled(Panel *Current,int NewType);
    Panel* ChangePanel(Panel *Current,int NewType,int CreateNew,int Force);
    void SetPanelPositions(int LeftFullScreen,int RightFullScreen);
    void SetScreenPositions();
    void RedrawKeyBar();

    Panel *LeftPanel,*RightPanel,*ActivePanel;

    Manager ModalManager;
    CommandLine CmdLine;
    History *CmdHistory,*FolderHistory,*ViewHistory;
    KeyBar MainKeyBar;
    MenuBar TopMenuBar;
    HighlightFiles HiFiles;
    GroupSort GrpSort;
    FilePositionCache ViewerPosCache,EditorPosCache;
    KeyMacro Macro;
    PluginsSet Plugins;
};


class Modal:public ScreenObject
{
  private:
    int ReadKey,WriteKey;
    KeyBar *ModalKeyBar;
  protected:
    INPUT_RECORD ReadRec;
    char HelpTopic[512];
    int ExitCode;
    int EndLoop;
    int EnableSwitch;
  public:
    Modal();
    virtual void GetDialogObjectsData() {};
    int Done();
    void ClearDone();
    int GetExitCode();
    void SetExitCode(int Code);
    int GetEnableSwitch() {return(EnableSwitch);};
    void SetEnableSwitch(int Mode) {EnableSwitch=Mode;};
    virtual void Process();
    int ReadInput();
    void WriteInput(int Key);
    void ProcessInput();
    void SetHelp(char *Topic);
    void ShowHelp();
    void SetKeyBar(KeyBar *ModalKeyBar);
    virtual int GetTypeAndName(char *Type,char *Name) {return(0);};
    virtual int IsFileModified() {return(FALSE);};
};


class FolderTree:public Modal
{
  private:
    void DrawEdit();
    TreeList *Tree;
    Edit *FindEdit;
    char NewFolder[NM];
    char LastName[NM];
  public:
    FolderTree(char *ResultFolder,int ModalMode,int TX1,int TY1,int TX2,int TY2);
    int ProcessKey(int Key);
    int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
};


class FileViewer:public Modal
{
  private:
    void Process();
    void Show();
    void DisplayObject();
    Viewer View;
    KeyBar ViewKeyBar;
    char NewTitle[NM];
    int F3KeyOnly;
    int FullScreen;
    int ExitCode;
    int DisableEdit;
  public:
    FileViewer(char *Name,int EnableSwitch=FALSE,int DisableHistory=FALSE,
               int DisableEdit=FALSE,long ViewStartPos=-1,char *PluginData=NULL,
               NamesList *ViewNamesList=NULL);
    FileViewer(char *Name,int EnableSwitch,char *Title,
               int X1,int Y1,int X2,int Y2);
    void Init(char *Name,int EnableSwitch,int DisableHistory,
              long ViewStartPos,char *PluginData,NamesList *ViewNamesList);
    int ProcessKey(int Key);
    int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
    int GetTypeAndName(char *Type,char *Name);
    void ShowConsoleTitle();
    void SetTempViewName(char *Name);
    int GetExitCode();
};


enum {MENU_SHOWAMPERSAND=1,MENU_WRAPMODE=2,MENU_DISABLEDRAWBACKGROUND=4};

class VMenu:public Modal
{
  private:
    void DisplayObject();
    void ShowMenu();
    SaveScreen *SaveScr;
    struct MenuItem *Item;
    int ItemCount;
    char Title[100];
    char BottomTitle[100];
    int SelectPos,TopPos;
    int MaxHeight;
    int MaxLength;
    int DrawBackground,BoxType,WrapMode,ShowAmpersand;
    int UpdateRequired;
    int DialogStyle;
    int AutoHighlight;
    int CallCount;
    int PrevMacroMode;
  public:
    VMenu(char *Title,struct MenuData *Data,int ItemCount,int MaxHeight=0);
    ~VMenu();
    void DeleteItems();
    void FastShow() {ShowMenu();}
    void Show();
    void Hide();
    int ProcessKey(int Key);
    int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
    int AddItem(struct MenuItem *NewItem);
    void SetBottomTitle(char *BottomTitle);
    void SetDialogStyle(int Style) {DialogStyle=Style;};
    void SetUpdateRequired(int SetUpdate) {UpdateRequired=SetUpdate;};
    void SetBoxType(int BoxType);
    void SetFlags(unsigned int Flags);
    int GetUserData(void *Data,int Size,int Position=-1);
    int GetSelection(int Position=-1);
    void SetSelection(int Selection,int Position=-1);
    int GetSelectPos();
    int GetItemCount() {return(ItemCount);};
    void AssignHighlights(int Reverse);
};


class HMenu:public Modal
{
  private:
    void DisplayObject();
    void ShowMenu();
    void ProcessSubMenu(struct MenuData *Data,int DataCount,char *SubMenuHelp,
                        int X,int Y,int &Position);
    VMenu *SubMenu;
    struct HMenuData *Item;
    int SelectPos;
    int ItemCount;
    int VExitCode;
    int ItemX[16];
  public:
    HMenu(struct HMenuData *Item,int ItemCount);
    int ProcessKey(int Key);
    int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
    void GetExitCode(int &ExitCode,int &VExitCode);
};


class Dialog:public Modal
{
  private:
    void DisplayObject();
    void DeleteDialogObjects();
    void ShowDialog();
    int ChangeFocus(int FocusPos,int Step,int SkipGroup);
    int IsEdit(int Type);
    void SelectFromEditHistory(Edit *EditLine,char *HistoryName);
    void AddToEditHistory(char *AddStr,char *HistoryName);
    int ProcessHighlighting(int Key,int FocusPos,int Translate);

    struct DialogItem *Item;
    char OldConsoleTitle[512];
    int ItemCount;
    int InitObjects;
    int CreateObjects;
    int WarningStyle;
    int DialogTooLong;
    int PrevMacroMode;
  public:
    Dialog(struct DialogItem *Item,int ItemCount);
    ~Dialog();
    int ProcessKey(int Key);
    int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
    void Show();
    void FastShow() {ShowDialog();}
    void InitDialogObjects();
    void GetDialogObjectsData();
    void SetWarningStyle(int Style) {WarningStyle=Style;};
    static void DataToItem(struct DialogData *Data,struct DialogItem *Item,
                           int Count);
    static int IsKeyHighlighted(char *Str,int Key,int Translate);
};


struct MenuItem
{
  char Name[128];
  unsigned char Selected;
  unsigned char Checked;
  unsigned char Separator;
  char UserData[sizeof(WIN32_FIND_DATA)+NM+10];
  int UserDataSize;
};


struct MenuData
{
  char *Name;
  unsigned char Selected;
  unsigned char Checked;
  unsigned char Separator;
};


struct HMenuData
{
  char *Name;
  int Selected;
  struct MenuData *SubMenu;
  int SubMenuSize;
  char *SubMenuHelp;
};


class ScanTree
{
  private:
    void Init();

    HANDLE FindHandle[NM/2];
    int SecondPass[NM/2];
    int FindHandleCount;
    int RetUpDir;
    int Recurse;
    int SecondDirName;
    char FindPath[2*NM];
    char FindMask[NM];
  public:
    ScanTree(int RetUpDir,int Recurse=1);
    ScanTree::~ScanTree();
    void SetFindPath(char *Path,char *Mask);
    int GetNextName(WIN32_FIND_DATA *fdata,char *FullName);
    void SkipDir();
    int IsDirSearchDone() {return(SecondDirName);};
};


struct TreeItem
{
  char Name[NM];
  int Last[NM/2];
  int Depth;
};

/*
enum DialogItemTypes {DI_TEXT,DI_VTEXT,DI_SINGLEBOX,DI_DOUBLEBOX,
                      DI_EDIT,DI_PSWEDIT,DI_FIXEDIT,
                      DI_BUTTON,DI_CHECKBOX,DI_RADIOBUTTON};


enum DialogItemFlags {
  DIF_COLORMASK=0xff,DIF_SETCOLOR=0x100,DIF_BOXCOLOR=0x200,DIF_GROUP=0x400,
  DIF_LEFTTEXT=0x800,DIF_MOVESELECT=0x1000,DIF_SHOWAMPERSAND=0x2000,
  DIF_CENTERGROUP=0x4000,DIF_NOBRACKETS=0x8000,DIF_SEPARATOR=0x10000,
  DIF_EDITOR=0x20000,DIF_HISTORY=0x40000
};
*/

struct DialogItem
{
  unsigned char Type;
  unsigned char X1,Y1,X2,Y2;
  unsigned char Focus;
  unsigned int Selected;
  unsigned int Flags;
  unsigned char DefaultButton;
  char Data[512];
  void *ObjPtr;
};


struct DialogData
{
  unsigned char Type;
  unsigned char X1,Y1,X2,Y2;
  unsigned char Focus;
  unsigned int Selected;
  int Flags;
  unsigned char DefaultButton;
  char *Data;
};


#define MakeDialogItems(Data,Item) \
  struct DialogItem Item[sizeof(Data)/sizeof(Data[0])]; \
  Dialog::DataToItem(Data,Item,sizeof(Data)/sizeof(Data[0]));



enum {UNDO_NONE=0,UNDO_EDIT,UNDO_INSSTR,UNDO_DELSTR};

struct EditorUndoData
{
  int Type;
  int UndoNext;
  int StrPos;
  int StrNum;
  char *Str;
};


class Editor:public ScreenObject
{
  private:
    void DisplayObject();
    void ShowEditor(int CurLineOnly);
    void ShowStatus();
    void DeleteString(struct EditList *DelPtr,int DeleteLast,int UndoLine);
    void InsertString();
    void Up();
    void Down();
    void ScrollDown();
    void ScrollUp();
    void Search(int Next);
    void GoToLine(int Line);
    int CalcDistance(struct EditList *From,struct EditList *To,int MaxDist);
    void Paste();
    void Copy(int Append);
    void DeleteBlock();
    void UnmarkBlock();
    void ChangeEditKeyBar();
    void AddUndoData(char *Str,int StrNum,int StrPos,int Type);
    void Undo();
    void SelectAll();
    void SetStringsTable();
    void BlockLeft();
    void BlockRight();
    void DeleteVBlock();
    void VCopy(int Append);
    void VPaste(char *ClipText);
    void VBlockShift(int Left);
    struct EditList * GetStringByNumber(int DestLine);

    KeyBar *EditKeyBar;
    struct EditList *TopList,*EndList,*TopScreen,*CurLine;
    struct EditorUndoData UndoData[64];
    int UndoDataPos;
    int UndoOverflow;
    int UndoSavePos;
    int LastChangeStrPos;
    char FileName[NM];
    int NumLastLine,NumLine;
    int Modified;
    int WasChanged;
    int Overtype;
    int DisableOut;
    int Pasting;
    int MarkingBlock;
    char GlobalEOL[10];
    struct EditList *BlockStart;
    int BlockStartLine;

    struct EditList *VBlockStart;
    int VBlockX;
    int VBlockSizeX;
    int VBlockY;
    int VBlockSizeY;
    int MarkingVBlock;

    int DisableUndo;
    int NewUndo;
    int LockMode;
    int BlockUndo;

    int MaxRightPos;

    unsigned char LastSearchStr[256];
    int LastSearchCase,LastSearchReverse;

    struct CharTableSet TableSet;
    int UseDecodeTable,TableNum,AnsiText;
    int StartLine,StartChar;

    int TableChangedByUser;

    char Title[512];
    char PluginData[NM*2];

    char PluginTitle[512];

    long SavePosLine[10];
    long SavePosCursor[10];
    long SavePosScreenLine[10];
    long SavePosLeftPos[10];

    int EditorID;
    bool OpenFailed;

    FileEditor *HostFileEditor;
  public:
    Editor();
    ~Editor();
    int ReadFile(char *Name,int &UserBreak);
    int SaveFile(char *Name,int Ask,int TextFormat);
    int ProcessKey(int Key);
    int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
    void SetEditKeyBar(KeyBar *EditKeyBar);
    void KeepInitParameters();
    void SetStartPos(int LineNum,int CharNum);
    int IsFileModified();
    int IsFileChanged();
    void SetTitle(char *Title);
    long GetCurPos();
    void SetPluginData(char *PluginData);
    int EditorControl(int Command,void *Param);
    int ProcessEditorInput(INPUT_RECORD *Rec);
    void SetHostFileEditor(FileEditor *Editor) {HostFileEditor=Editor;};
    static int IsShiftKey(int Key);
    static void SetReplaceMode(int Mode);
};


struct EditList
{
  struct EditList *Prev;
  struct EditList *Next;
  Edit EditLine;
};


class FileEditor:public Modal
{
  private:
    void Process();
    void Show();
    void DisplayObject();

    Editor FEdit;
    KeyBar EditKeyBar;
    char FileName[NM];
    char FullFileName[NM];
    char StartDir[NM];
    char NewTitle[NM];
    int FullScreen;
    int ExitCode;
  public:
    FileEditor(char *Name,int CreateNewFile,int EnableSwitch,
               int StartLine=-1,int StartChar=-1,int DisableHistory=FALSE,
               char *PluginData=NULL);
    FileEditor(char *Name,int CreateNewFile,int EnableSwitch,
               int StartLine,int StartChar,char *Title,
               int X1,int Y1,int X2,int Y2);
    void Init(char *Name,int CreateNewFile,int EnableSwitch,
              int StartLine,int StartChar,int DisableHistory,char *PluginData);
    int ProcessKey(int Key);
    int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
    int GetTypeAndName(char *Type,char *Name);
    void ShowConsoleTitle();
    int IsFileChanged() {return(FEdit.IsFileChanged());};
    int IsFileModified() {return(FEdit.IsFileModified());};
    int GetExitCode();
};


class Help:public Modal
{
  private:
    void DisplayObject();
    void ReadHelp();
    void AddLine(char *Line);
    void HighlightsCorrection(char *Str);
    void FastShow();
    void OutString(char *Str);
    int StringLen(char *Str);
    void CorrectPosition();
    int IsReferencePresent();
    void MoveToReference(int Forward,int CurScreen);
    void ReadPluginsHelp();
    char *HelpData;
    char HelpTopic[512];
    char SelTopic[512];
    char HelpPath[NM];
    int TopLevel;
    int PrevFullScreen;
    int StrCount,FixCount,FixSize;
    int TopStr;
    int CurX,CurY;
    int ShowPrev;
    int DisableOut;
    int TopicFound;
    int PrevMacroMode;
  public:
    Help(char *Topic);
    Help(char *Topic,int &ShowPrev,int PrevFullScreen);
    ~Help();
    void Hide();
    int ProcessKey(int Key);
    int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
    static int GetFullScreenMode();
    static void SetFullScreenMode(int Mode);
    static int PluginPanelHelp(HANDLE hPlugin);
};


enum COPY_CODES {COPY_CANCEL,COPY_NEXT,COPY_FAILURE,COPY_SUCCESS,
                 COPY_SUCCESS_MOVE};

class ShellCopy
{
  private:
    void CopyFileTree(char *Dest);
    COPY_CODES ShellCopyOneFile(char *Src,WIN32_FIND_DATA *SrcData,char *Dest,
                                int KeepPathPos,int Rename);
    int ShellCopyFile(char *SrcName,WIN32_FIND_DATA *SrcData,char *DestName,
                      DWORD DestAttr,int Append);
    int ShellSystemCopy(char *SrcName,char *DestName,WIN32_FIND_DATA *SrcData);
    void ShellCopyMsg(char *Src,char *Dest,int Flags);
    int ShellCopyConvertWildcards(char *Src,char *Dest);
    void CreatePath(char *Path);
    int DeleteAfterMove(char *Name,int Attr);
    void SetDestDizPath(char *DestPath);
    int AskOverwrite(WIN32_FIND_DATA *SrcData,char *DestName,
        DWORD DestAttr,int SameName,int Rename,int AskAppend,
        int &Append,int &RetCode);
    int GetSecurity(char *FileName,SECURITY_ATTRIBUTES *sa);
    int SetSecurity(char *FileName,SECURITY_ATTRIBUTES *sa);
    int IsSameDisk(char *SrcPath,char *DestPath);
    bool CalcTotalSize();
    int CmpFullNames(char *Src,char *Dest);

    char sddata[16000];
    DizList DestDiz;
    int DizRead;
    char DestDizPath[2*NM];
    Panel *SrcPanel,*AnotherPanel;
    char *CopyBuffer;
    char RenamedName[NM],CopiedName[NM];
    int PanelMode,SrcPanelMode;
    int OvrMode,ReadOnlyOvrMode,ReadOnlyDelMode;
    int Move;
    int Link;
    int CurrentOnly;
    int CopySecurity;
    long TotalFiles;
    int SrcDriveType;
    char SrcDriveRoot[NM];
    int SelectedFolderNameLength;
    int OverwriteNext;
  public:
    ShellCopy(Panel *SrcPanel,int Move,int Link,int CurrentOnly,int Ask,
              int &ToPlugin,char *PluginDestPath);
    ~ShellCopy();
    static void ShowBar(int64 WrittenSize,int64 TotalSize,bool TotalBar);
    static void ShowTitle(int FirstTime);
};


class ChangePriority
{
  private:
    int SavePriority;
  public:
    ChangePriority(int NewPriority);
    ~ChangePriority();
};


class SaveFilePos
{
  private:
    FILE *SaveFile;
    long SavePos;
  public:
    SaveFilePos(FILE *SaveFile);
    ~SaveFilePos();
};


class LockScreen
{
  public:
    LockScreen();
    ~LockScreen();
};


class RedrawDesktop
{
  private:
    int LeftVisible,RightVisible;
  public:
    RedrawDesktop();
    ~RedrawDesktop();
};


struct GrabberArea
{
  int X1,Y1,X2,Y2;
  int CurX,CurY;
};

class Grabber:Modal
{
  private:
    void DisplayObject();
    int ProcessKey(int Key);
    int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
    void CopyGrabbedArea(int Append);

    SaveScreen *SaveScr;
    struct GrabberArea PrevArea,GArea;
    int ResetArea;
    int PrevMacroMode;
  public:
    Grabber();
    ~Grabber();
};


class FindFiles
{
  private:
    int FindFilesProcess();
    void SetPluginDirectory(char *FileName);
    SaveScreen *FindSaveScr;
  public:
    FindFiles();
    ~FindFiles();
};


class PreserveLongName
{
  private:
    char SaveLongName[NM],SaveShortName[NM];
    int Preserve;
  public:
    PreserveLongName(char *ShortName,int Preserve);
    ~PreserveLongName();
};


class GetFileString
{
  private:
    char ReadBuf[8192];
    int ReadPos,ReadSize;
    char *Str;
    int StrLength;
    FILE *SrcFile;
  public:
    GetFileString(FILE *SrcFile);
    ~GetFileString();
    int GetString(char **DestStr,int &Length);
};


class ScreenBuf
{
  private:
    CHAR_INFO *Buf;
    CHAR_INFO *Shadow;
    CHAR_INFO MacroChar;
    HANDLE hScreen;
    int BufX,BufY;
    int Flushed;
    int LockCount;
    int UseShadow;
    int CurX,CurY,CurVisible,CurSize;
    int FlushedCurPos,FlushedCurType;
  public:
    ScreenBuf();
    ~ScreenBuf();
    void AllocBuf(int X,int Y);
    void FillBuf();
    void Write(int X,int Y,CHAR_INFO *Text,int TextLength);
    void Read(int X1,int Y1,int X2,int Y2,CHAR_INFO *Text);
    void Flush();
    void Lock();
    void Unlock();
    int GetLockCount() {return(LockCount);};
    void SetLockCount(int Count) {LockCount=Count;};
    void SetHandle(HANDLE hScreen);
    void ResetShadow();
    void MoveCursor(int X,int Y);
    void GetCursorPos(int& X,int& Y);
    void SetCursorType(int Visible,int Size);
    void GetCursorType(int &Visible,int &Size);
    void RestoreMacroChar();
};


struct HistoryRecord
{
  char Name[512];
  char Title[32];
  int Type;
};

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


class ChangeMacroMode
{
  private:
    int PrevMacroMode;
  public:
    ChangeMacroMode(int NewMode);
    ~ChangeMacroMode();
};
