#ifndef __PANEL_HPP__
#define __PANEL_HPP__
/*
panel.hpp

Parent class для панелей

*/
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
      BY_OWNER,BY_COMPRESSEDSIZE,BY_NUMLINKS
     };

enum {VIEW_0=0,VIEW_1,VIEW_2,VIEW_3,VIEW_4,VIEW_5,VIEW_6,VIEW_7,VIEW_8,VIEW_9};

enum
{
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
		   обработка Del в меню дисков
		*/
		int ProcessDelDisk(char Drive, int DriveType,VMenu *ChDiskMenu);
		/* DJ $ */
		void FastFindShow(int FindX,int FindY);
		void FastFindProcessName(Edit *FindEdit,const char *Src,char *LastName,char *Name);
		void DragMessage(int X,int Y,int Move);

	protected:
		void FastFind(int FirstKey);
		void DrawSeparator(int Y);
		void ShowScreensCount();
		int  IsDragging();
		int  ProcessShortcutFolder(int Key,BOOL ProcTreePanel=FALSE);

	public:
		Panel();
		virtual ~Panel();

	public:
		virtual int SendKeyToPlugin(DWORD Key,BOOL Pred=FALSE) {return FALSE;};
		virtual BOOL SetCurDir(const char *NewDir,int ClosePlugin);
		virtual void ChangeDirToCurrent();
		virtual int GetCurDir(char *CurDir);
		virtual int GetSelCount() {return(0);};
		virtual int GetRealSelCount() {return(0);};
		virtual int GetSelName(char *Name,int &FileAttr,char *ShortName=NULL,WIN32_FIND_DATA *fd=NULL) {return(FALSE);};
		virtual void UngetSelName() {};
		virtual void ClearLastGetSelection() {};
		virtual unsigned __int64 GetLastSelectedSize() {return (unsigned __int64)(-1);};
		virtual int GetLastSelectedItem(struct FileListItem *LastItem) {return(0);};
		virtual int GetCurName(char *Name,char *ShortName);
		virtual int GetCurBaseName(char *Name,char *ShortName);
		virtual int GetFileName(char *Name,int Pos,int &FileAttr) {return(FALSE);};
		virtual int GetCurrentPos() {return(0);};
		virtual void SetFocus();
		virtual void KillFocus();
		virtual void Update(int Mode) {};
		/*$ 22.06.2001 SKV
		  Параметр для игнорирования времени последнего Update.
		  Используется для Update после исполнения команды.
		*/
		virtual int UpdateIfChanged(int UpdateMode) {return(0);};
		/* SKV$*/
		/* $ 19.03.2002 DJ
		   UpdateIfRequired() - обновить, если апдейт был пропущен из-за того,
		   что панель невидима
		*/
		virtual void UpdateIfRequired() {};
		/* DJ $ */
		virtual void CloseChangeNotification() {};
		virtual int FindPartName(char *Name,int Next,int Direct=1,int ExcludeSets=0) {return(FALSE);}
		virtual int GoToFile(long idxItem) {return(TRUE);};
		virtual int GoToFile(const char *Name,BOOL OnlyPartName=FALSE) {return(TRUE);};
		virtual long FindFile(const char *Name,BOOL OnlyPartName=FALSE) {return -1L;};
		virtual int IsSelected(char *Name) {return(FALSE);};

		virtual long FindFirst(const char *Name) {return -1;}
		virtual long FindNext(int StartPos, const char *Name) {return -1;}

		/* $ 09.02.2001 IS
		   Функции установления/считывания состояния режима
		   "Помеченные файлы вперед"
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
		  Изменить порядок сортировки на панели */
		virtual void ChangeSortOrder(int NewOrder) {SetSortOrder(NewOrder);};
		/* VVM $ */
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
		virtual void ReadDiz(struct PluginPanelItem *ItemList=NULL,int ItemLength=0, DWORD dwFlags=0) {};
		virtual void DeleteDiz(char *Name,char *ShortName) {};
		virtual void GetDizName(char *DizName) {};
		virtual void FlushDiz() {};
		virtual void CopyDiz(char *Name,char *ShortName,char *DestName,
		                     char *DestShortName,DizList *DestDiz) {};
		virtual int IsFullScreen() {return ViewSettings.FullScreen;};
		virtual int IsDizDisplayed() {return(FALSE);};
		virtual int IsColumnDisplayed(int Type) {return(FALSE);};
		virtual int GetColumnsCount() { return 1;};
		virtual void SetReturnCurrentFile(int Mode) {};
		virtual void QViewDelTempName() {};
		virtual void GetPluginInfo(struct PluginInfo *Info) {};
		virtual void GetOpenPluginInfo(struct OpenPluginInfo *Info) {};
		virtual void SetPluginMode(HANDLE hPlugin,char *PluginFile,bool SendOnFocus=false) {};
		virtual void SetPluginModified() {};
		virtual int ProcessPluginEvent(int Event,void *Param) {return(FALSE);};
		virtual HANDLE GetPluginHandle() {return(INVALID_HANDLE_VALUE);};
		virtual void SetTitle();
		virtual const char *GetTitle(char *Title,int LenTitle,int TruncSize=0);

		virtual __int64 VMProcess(int OpCode,void *vParam=NULL,__int64 iParam=0);

		virtual void IfGoHome(char Drive) {};

		/* $ 30.04.2001 DJ
		   функция вызывается для обновления кейбара; если возвращает FALSE,
		   используется стандартный кейбар
		*/
		virtual BOOL UpdateKeyBar() { return FALSE; };
		/* DJ $ */
		virtual long GetFileCount() {return 0;}
		virtual BOOL GetItem(int,void *) {return FALSE;};

		static void EndDrag();
		virtual void Hide();
		virtual void Show();
		int SetPluginCommand(int Command,void *Param);
		int PanelProcessMouse(MOUSE_EVENT_RECORD *MouseEvent,int &RetCode);
		void ChangeDisk();
		int GetFocus() {return(Focus);};
		int GetType() {return(Type);};
		void SetUpdateMode(int Mode) {EnableUpdate=Mode;};
		int MakeListFile(char *ListFileName,int ShortNames,char *Modifers=NULL);
		int SetCurPath();

		BOOL NeedUpdatePanel(Panel *AnotherPanel);
};

#endif  // __PANEL_HPP__
