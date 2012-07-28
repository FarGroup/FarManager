#pragma once

/*
panel.hpp

Parent class для панелей
*/
/*
Copyright © 1996 Eugene Roshal
Copyright © 2000 Far Group
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "scrobj.hpp"
#include "panelctype.hpp"

class DizList;

struct PanelViewSettings
{
	unsigned __int64 ColumnType[PANEL_COLUMNCOUNT];
	int ColumnWidth[PANEL_COLUMNCOUNT];
	int ColumnCount;
	unsigned __int64 StatusColumnType[PANEL_COLUMNCOUNT];
	int StatusColumnWidth[PANEL_COLUMNCOUNT];
	int StatusColumnCount;
	DWORD Flags;
	int ColumnWidthType[PANEL_COLUMNCOUNT];
	int StatusColumnWidthType[PANEL_COLUMNCOUNT];
};

enum
{
	PVS_FULLSCREEN            = 0x00000001,
	PVS_ALIGNEXTENSIONS       = 0x00000002,
	PVS_FOLDERALIGNEXTENSIONS = 0x00000004,
	PVS_FOLDERUPPERCASE       = 0x00000008,
	PVS_FILELOWERCASE         = 0x00000010,
	PVS_FILEUPPERTOLOWERCASE  = 0x00000020,
};

enum
{
	FILE_PANEL,
	TREE_PANEL,
	QVIEW_PANEL,
	INFO_PANEL
};

enum
{
	UNSORTED,
	BY_NAME,
	BY_EXT,
	BY_MTIME,
	BY_CTIME,
	BY_ATIME,
	BY_SIZE,
	BY_DIZ,
	BY_OWNER,
	BY_COMPRESSEDSIZE,
	BY_NUMLINKS,
	BY_NUMSTREAMS,
	BY_STREAMSSIZE,
	BY_FULLNAME,
	BY_CHTIME,
	BY_CUSTOMDATA,
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
	DRIVE_SORT_PLUGINS_BY_HOTKEY = 0x00000400,
};

enum {UPDATE_KEEP_SELECTION=1,UPDATE_SECONDARY=2,UPDATE_IGNORE_VISIBLE=4,UPDATE_DRAW_MESSAGE=8};

enum {NORMAL_PANEL,PLUGIN_PANEL};

enum {DRIVE_DEL_FAIL, DRIVE_DEL_SUCCESS, DRIVE_DEL_EJECT, DRIVE_DEL_NONE};

enum {UIC_UPDATE_NORMAL, UIC_UPDATE_FORCE, UIC_UPDATE_FORCE_NOTIFICATION};

class VMenu;
class Edit;
struct PanelMenuItem;

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
		int CurTopFile;
		int CurFile;
		int ShowShortNames;
		int NumericSort;
		int CaseSensitiveSort;
		int DirectoriesFirst;
		int ModalMode;
		int PluginCommand;
		string strPluginParam;

	public:
		struct PanelViewSettings ViewSettings;
		int ProcessingPluginCommand;

	private:
		int ChangeDiskMenu(int Pos,int FirstCall);
		int DisconnectDrive(PanelMenuItem *item, VMenu &ChDisk);
		void RemoveHotplugDevice(PanelMenuItem *item, VMenu &ChDisk);
		int ProcessDelDisk(wchar_t Drive, int DriveType,VMenu *ChDiskMenu);
		void FastFindShow(int FindX,int FindY);
		void FastFindProcessName(Edit *FindEdit,const wchar_t *Src,string &strLastName, string &strName);
		void DragMessage(int X,int Y,int Move);

	private:
		struct ShortcutInfo
		{
			string ShortcutFolder;
			string PluginFile;
			string PluginData;
			GUID PluginGuid;
		};
		bool GetShortcutInfo(ShortcutInfo& Info);

	protected:
		void FastFind(int FirstKey);
		void DrawSeparator(int Y);
		void ShowScreensCount();
		int  IsDragging();
		virtual void ClearAllItem(){}

	public:
		Panel();
		virtual ~Panel();

	public:
		virtual int SendKeyToPlugin(DWORD Key,bool Pred=false) {return FALSE;}
		virtual BOOL SetCurDir(const string& NewDir,int ClosePanel,BOOL IsUpdated=TRUE);
		virtual void ChangeDirToCurrent();

		virtual int GetCurDir(string &strCurDir);

		virtual size_t GetSelCount() {return 0;}
		virtual size_t GetRealSelCount() {return 0;}
		virtual int GetSelName(string *strName,DWORD &FileAttr,string *ShortName=nullptr,FAR_FIND_DATA_EX *fd=nullptr) {return FALSE;}
		virtual void UngetSelName() {}
		virtual void ClearLastGetSelection() {}
		virtual unsigned __int64 GetLastSelectedSize() {return (unsigned __int64)(-1);}
		virtual int GetLastSelectedItem(struct FileListItem *LastItem) {return 0;}

		virtual int GetCurName(string &strName, string &strShortName);
		virtual int GetCurBaseName(string &strName, string &strShortName);
		virtual int GetFileName(string &strName,int Pos,DWORD &FileAttr) {return FALSE;}

		virtual int GetCurrentPos() {return 0;}
		virtual void SetFocus();
		virtual void KillFocus();
		virtual void Update(int Mode) {}
		/*$ 22.06.2001 SKV
		  Параметр для игнорирования времени последнего Update.
		  Используется для Update после исполнения команды.
		*/
		virtual int UpdateIfChanged(int UpdateMode) {return 0;}
		/* $ 19.03.2002 DJ
		   UpdateIfRequired() - обновить, если апдейт был пропущен из-за того,
		   что панель невидима
		*/
		virtual void UpdateIfRequired() {}

		virtual void StartFSWatcher(bool got_focus=false) {}
		virtual void StopFSWatcher() {}
		virtual int FindPartName(const wchar_t *Name,int Next,int Direct=1,int ExcludeSets=0) {return FALSE;}
		virtual bool GetPlainString(string& Dest,int ListPos){return false;}


		virtual int GoToFile(long idxItem) {return TRUE;}
		virtual int GoToFile(const wchar_t *Name,BOOL OnlyPartName=FALSE) {return TRUE;}
		virtual long FindFile(const wchar_t *Name,BOOL OnlyPartName=FALSE) {return -1;}

		virtual int IsSelected(const wchar_t *Name) {return FALSE;}
		virtual int IsSelected(long indItem) {return FALSE;}

		virtual long FindFirst(const wchar_t *Name) {return -1;}
		virtual long FindNext(int StartPos, const wchar_t *Name) {return -1;}

		virtual void SetSelectedFirstMode(int) {}
		virtual int GetSelectedFirstMode() {return 0;}
		int GetMode() {return(PanelMode);}
		void SetMode(int Mode) {PanelMode=Mode;}
		int GetModalMode() {return(ModalMode);}
		void SetModalMode(int ModalMode) {Panel::ModalMode=ModalMode;}
		int GetViewMode() {return(ViewMode);}
		virtual void SetViewMode(int ViewMode);
		virtual int GetPrevViewMode() {return(PrevViewMode);}
		void SetPrevViewMode(int PrevViewMode) {Panel::PrevViewMode=PrevViewMode;}
		virtual int GetPrevSortMode() {return(SortMode);}
		virtual int GetPrevSortOrder() {return(SortOrder);}
		int GetSortMode() {return(SortMode);}
		virtual int GetPrevNumericSort() {return NumericSort;}
		int GetNumericSort() { return NumericSort; }
		void SetNumericSort(int Mode) { NumericSort=Mode; }
		virtual void ChangeNumericSort(int Mode) { SetNumericSort(Mode); }
		virtual int GetPrevCaseSensitiveSort() {return CaseSensitiveSort;}
		int GetCaseSensitiveSort() {return CaseSensitiveSort;}
		void SetCaseSensitiveSort(int Mode) {CaseSensitiveSort = Mode;}
		virtual void ChangeCaseSensitiveSort(int Mode) {SetCaseSensitiveSort(Mode);}
		virtual int GetPrevDirectoriesFirst() {return DirectoriesFirst;}
		int GetDirectoriesFirst() { return DirectoriesFirst; }
		void SetDirectoriesFirst(int Mode) { DirectoriesFirst=Mode; }
		virtual void ChangeDirectoriesFirst(int Mode) { SetDirectoriesFirst(Mode); }
		virtual void SetSortMode(int SortMode) {Panel::SortMode=SortMode;}
		int GetSortOrder() {return(SortOrder);}
		void SetSortOrder(int SortOrder) {Panel::SortOrder=SortOrder;}
		virtual void ChangeSortOrder(int NewOrder) {SetSortOrder(NewOrder);}
		int GetSortGroups() {return(SortGroups);}
		void SetSortGroups(int SortGroups) {Panel::SortGroups=SortGroups;}
		int GetShowShortNamesMode() {return(ShowShortNames);}
		void SetShowShortNamesMode(int Mode) {ShowShortNames=Mode;}
		void InitCurDir(const wchar_t *CurDir);
		virtual void CloseFile() {}
		virtual void UpdateViewPanel() {}
		virtual void CompareDir() {}
		virtual void MoveToMouse(MOUSE_EVENT_RECORD *MouseEvent) {}
		virtual void ClearSelection() {}
		virtual void SaveSelection() {}
		virtual void RestoreSelection() {}
		virtual void SortFileList(int KeepPosition) {}
		virtual void EditFilter() {}
		virtual bool FileInFilter(long idxItem) {return true;}
		virtual bool FilterIsEnabled() {return false;}
		virtual void ReadDiz(struct PluginPanelItem *ItemList=nullptr,int ItemLength=0, DWORD dwFlags=0) {}
		virtual void DeleteDiz(const string& Name,const string& ShortName) {}
		virtual void GetDizName(string &strDizName) {}
		virtual void FlushDiz() {}
		virtual void CopyDiz(const string& Name,const string& ShortName,const string& DestName, const string& DestShortName,DizList *DestDiz) {}
		virtual bool IsFullScreen() {return (ViewSettings.Flags&PVS_FULLSCREEN)==PVS_FULLSCREEN;}
		virtual int IsDizDisplayed() {return FALSE;}
		virtual int IsColumnDisplayed(int Type) {return FALSE;}
		virtual int GetColumnsCount() { return 1;}
		virtual void SetReturnCurrentFile(int Mode) {}
		virtual void QViewDelTempName() {}
		virtual void GetOpenPanelInfo(struct OpenPanelInfo *Info) {}
		virtual void SetPluginMode(HANDLE hPlugin,const wchar_t *PluginFile,bool SendOnFocus=false) {}
		virtual void SetPluginModified() {}
		virtual int ProcessPluginEvent(int Event,void *Param) {return FALSE;}
		virtual HANDLE GetPluginHandle() {return nullptr;}
		virtual void SetTitle();
		virtual string &GetTitle(string &Title,int SubLen=-1,int TruncSize=0);

#ifdef FAR_LUA_TEMP
#else
		virtual __int64 VMProcess(int OpCode,void *vParam=nullptr,__int64 iParam=0);
#endif

		virtual void IfGoHome(wchar_t Drive) {}

		/* $ 30.04.2001 DJ
		   функция вызывается для обновления кейбара; если возвращает FALSE,
		   используется стандартный кейбар
		*/
		virtual BOOL UpdateKeyBar() { return FALSE; }

		virtual long GetFileCount() {return 0;}
		virtual BOOL GetItem(int,void *) {return FALSE;}

		bool ExecShortcutFolder(int Pos);
		bool ExecShortcutFolder(string& strShortcutFolder,const GUID& PluginGuid,string& strPluginFile,const string& strPluginData,bool CheckType);
		bool SaveShortcutFolder(int Pos, bool Add);

		static void EndDrag();
		virtual void Hide();
		virtual void Show();
		int SetPluginCommand(int Command,int Param1,void* Param2);
		int PanelProcessMouse(MOUSE_EVENT_RECORD *MouseEvent,int &RetCode);
		void ChangeDisk();
		int GetFocus() {return(Focus);}
		int GetType() {return(Type);}
		void SetUpdateMode(int Mode) {EnableUpdate=Mode;}
		bool MakeListFile(string &strListFileName,bool ShortNames,const wchar_t *Modifers=nullptr);
		int SetCurPath();

		BOOL NeedUpdatePanel(Panel *AnotherPanel);
};
