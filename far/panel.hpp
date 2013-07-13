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

struct column
{
	unsigned __int64 type;
	int width;
	int width_type;
};

struct PanelViewSettings
{
	std::vector<column> PanelColumns;
	std::vector<column> StatusColumns;
	string Name;
	unsigned __int64 Flags;

	PanelViewSettings():Flags(0){}

	void clear()
	{
		PanelColumns.clear();
		StatusColumns.clear();
		Name.clear();
		Flags = 0;
	}
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

enum SORT_MODE
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
	SORTMODE_LAST
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

enum panel_update_mode
{
	UIC_UPDATE_NORMAL,
	UIC_UPDATE_FORCE,
	UIC_UPDATE_FORCE_NOTIFICATION
};

class VMenu2;
class Edit;
struct PanelMenuItem;

class DelayedDestroy
{
public:
	DelayedDestroy():
		destroyed(false),
		prevent_delete_count(0)
	{}

protected:
	virtual ~DelayedDestroy() { assert(destroyed); }

public:
	bool Destroy()
	{
		assert(!destroyed);
		destroyed = true;
		if (prevent_delete_count > 0)
		{
			return false;
		}
		else
		{
			delete this;
			return true;
		}
	}

	bool Destroyed() const { return destroyed; }

	int AddRef() { return ++prevent_delete_count; }

	int Release()
	{
		assert(prevent_delete_count > 0);
		if (--prevent_delete_count > 0 || !destroyed)
			return prevent_delete_count;
		else {
			delete this;
			return 0;
		}
	}

private:
	bool destroyed;
	int prevent_delete_count;

};

class DelayDestroy
{
public:
	DelayDestroy(DelayedDestroy *host):m_host(host) { m_host->AddRef(); }
	~DelayDestroy() { m_host->Release(); }
private:
	DelayedDestroy *m_host;
};

class Panel:public ScreenObject, public DelayedDestroy
{
public:
	Panel();

	struct panelitem
	{
		panelitem() {}
		panelitem(const string& Name):strName(Name) {}
		virtual ~panelitem() {};

		string strName;
	};

	// TODO: make empty methods pure virtual, move empty implementations to dummy_panel class
	virtual void CloseFile() {}
	virtual void UpdateViewPanel() {}
	virtual void CompareDir() {}
	virtual void MoveToMouse(const MOUSE_EVENT_RECORD *MouseEvent) {}
	virtual void ClearSelection() {}
	virtual void SaveSelection() {}
	virtual void RestoreSelection() {}
	virtual void SortFileList(int KeepPosition) {}
	virtual void EditFilter() {}
	virtual bool FileInFilter(size_t idxItem) {return true;}
	virtual bool FilterIsEnabled() {return false;}
	virtual void ReadDiz(struct PluginPanelItem *ItemList=nullptr,int ItemLength=0, DWORD dwFlags=0) {}
	virtual void DeleteDiz(const string& Name,const string& ShortName) {}
	virtual void GetDizName(string &strDizName) {}
	virtual void FlushDiz() {}
	virtual void CopyDiz(const string& Name,const string& ShortName,const string& DestName, const string& DestShortName,DizList *DestDiz) {}
	virtual bool IsDizDisplayed() { return false; }
	virtual bool IsColumnDisplayed(int Type) {return false;}
	virtual int GetColumnsCount() { return 1;}
	virtual void SetReturnCurrentFile(int Mode) {}
	virtual void QViewDelTempName() {}
	virtual void GetOpenPanelInfo(struct OpenPanelInfo *Info) {}
	virtual void SetPluginMode(HANDLE hPlugin,const string& PluginFile,bool SendOnFocus=false) {}
	virtual void SetPluginModified() {}
	virtual int ProcessPluginEvent(int Event,void *Param) {return FALSE;}
	virtual HANDLE GetPluginHandle() {return nullptr;}
	virtual void SetTitle();
	virtual string &GetTitle(string &Title,int SubLen=-1,int TruncSize=0);
	virtual __int64 VMProcess(int OpCode,void *vParam=nullptr,__int64 iParam=0) override;
	virtual int SendKeyToPlugin(DWORD Key,bool Pred=false) {return FALSE;}
	virtual bool SetCurDir(const string& NewDir,bool ClosePanel,bool IsUpdated=true);
	virtual void ChangeDirToCurrent();
	virtual const string& GetCurDir();
	virtual size_t GetSelCount() {return 0;}
	virtual size_t GetRealSelCount() {return 0;}
	virtual int GetSelName(string *strName,DWORD &FileAttr,string *ShortName=nullptr,FAR_FIND_DATA *fd=nullptr) {return FALSE;}
	virtual void UngetSelName() {}
	virtual void ClearLastGetSelection() {}
	virtual unsigned __int64 GetLastSelectedSize() {return (unsigned __int64)(-1);}
	virtual const panelitem* GetLastSelectedItem() const {return nullptr;}
	virtual int GetCurName(string &strName, string &strShortName);
	virtual int GetCurBaseName(string &strName, string &strShortName);
	virtual int GetFileName(string &strName,int Pos,DWORD &FileAttr) {return FALSE;}
	virtual int GetCurrentPos() const {return 0;}
	virtual void SetFocus();
	virtual void KillFocus();
	virtual void Update(int Mode) = 0;
	virtual int UpdateIfChanged(panel_update_mode UpdateMode) {return 0;}
	virtual void UpdateIfRequired() {}
	virtual void StartFSWatcher(bool got_focus=false, bool check_time=true) {}
	virtual void StopFSWatcher() {}
	virtual int FindPartName(const string& Name,int Next,int Direct=1,int ExcludeSets=0) {return FALSE;}
	virtual bool GetPlainString(string& Dest,int ListPos){return false;}
	virtual int GoToFile(long idxItem) {return TRUE;}
	virtual int GoToFile(const string& Name,BOOL OnlyPartName=FALSE) {return TRUE;}
	virtual long FindFile(const string& Name,BOOL OnlyPartName=FALSE) {return -1;}
	virtual int IsSelected(const string& Name) {return FALSE;}
	virtual int IsSelected(size_t indItem) {return FALSE;}
	virtual long FindFirst(const string& Name) {return -1;}
	virtual long FindNext(int StartPos, const string& Name) {return -1;}
	virtual void SetSelectedFirstMode(bool) {}
	virtual bool GetSelectedFirstMode() {return false;}
	virtual void SetViewMode(int ViewMode);
	virtual int GetPrevViewMode() {return(PrevViewMode);}
	virtual int GetPrevSortMode() {return(SortMode);}
	virtual int GetPrevSortOrder() {return(SortOrder);}
	virtual bool GetPrevNumericSort() {return NumericSort;}
	virtual void ChangeNumericSort(bool Mode) { SetNumericSort(Mode); }
	virtual bool GetPrevCaseSensitiveSort() {return CaseSensitiveSort;}
	virtual void ChangeCaseSensitiveSort(bool Mode) {SetCaseSensitiveSort(Mode);}
	virtual bool GetPrevDirectoriesFirst() {return DirectoriesFirst;}
	virtual void ChangeDirectoriesFirst(bool Mode) { SetDirectoriesFirst(Mode); }
	virtual void SetSortMode(int Mode) {SortMode=Mode;}
	virtual void ChangeSortOrder(int NewOrder) {SetSortOrder(NewOrder);}
	virtual void IfGoHome(wchar_t Drive) {}
	virtual void UpdateKeyBar() = 0;
	virtual size_t GetFileCount() {return 0;}
	virtual const panelitem* GetItem(size_t) const {return nullptr;}
	virtual void Hide() override;
	virtual void Show() override;
	virtual void DisplayObject() override {}

	int GetMode() {return(PanelMode);}
	void SetMode(int Mode) {PanelMode=Mode;}
	int GetModalMode() {return(ModalMode);}
	void SetModalMode(int ModalMode) {Panel::ModalMode=ModalMode;}
	int GetViewMode() {return(ViewMode);}
	void SetPrevViewMode(int PrevViewMode) {Panel::PrevViewMode=PrevViewMode;}
	int GetSortMode() {return(SortMode);}
	bool GetNumericSort() { return NumericSort; }
	void SetNumericSort(bool Mode) { NumericSort = Mode; }
	bool GetCaseSensitiveSort() {return CaseSensitiveSort;}
	void SetCaseSensitiveSort(bool Mode) {CaseSensitiveSort = Mode;}
	bool GetDirectoriesFirst() { return DirectoriesFirst; }
	void SetDirectoriesFirst(bool Mode) { DirectoriesFirst = Mode != 0; }
	int GetSortOrder() {return(SortOrder);}
	void SetSortOrder(int Mode) {SortOrder=Mode;}
	bool GetSortGroups() {return(SortGroups);}
	void SetSortGroups(bool Mode) {SortGroups=Mode;}
	bool GetShowShortNamesMode() {return(ShowShortNames);}
	void SetShowShortNamesMode(bool Mode) {ShowShortNames=Mode;}
	void InitCurDir(const string& CurDir);
	bool ExecShortcutFolder(int Pos, bool raw=false);
	bool ExecShortcutFolder(string& strShortcutFolder, const GUID& PluginGuid, const string& strPluginFile, const string& strPluginData, bool CheckType);
	bool SaveShortcutFolder(int Pos, bool Add);
	int SetPluginCommand(int Command,int Param1,void* Param2);
	int PanelProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent,int &RetCode);
	void ChangeDisk();
	bool GetFocus() {return Focus;}
	int GetType() {return(Type);}
	void SetUpdateMode(int Mode) {EnableUpdate=Mode;}
	bool MakeListFile(string &strListFileName,bool ShortNames,const wchar_t *Modifers=nullptr);
	int SetCurPath();
	BOOL NeedUpdatePanel(const Panel *AnotherPanel);
	bool IsFullScreen() const { return (ViewSettings.Flags & PVS_FULLSCREEN) != 0; }
	void SetFullScreen() { ViewSettings.Flags |= PVS_FULLSCREEN; }
	static void EndDrag();

	int ProcessingPluginCommand;

protected:
	virtual ~Panel();
	virtual void ClearAllItem(){}

	void FastFind(int FirstKey);
	void DrawSeparator(int Y);
	void ShowScreensCount();
	int  IsDragging();

private:
	struct ShortcutInfo
	{
		string ShortcutFolder;
		string PluginFile;
		string PluginData;
		GUID PluginGuid;
	};
	bool GetShortcutInfo(ShortcutInfo& Info);
	int ChangeDiskMenu(int Pos,int FirstCall);
	int DisconnectDrive(const PanelMenuItem *item, VMenu2 &ChDisk);
	void RemoveHotplugDevice(const PanelMenuItem *item, VMenu2 &ChDisk);
	int ProcessDelDisk(wchar_t Drive, int DriveType,VMenu2 *ChDiskMenu);
	void FastFindShow(int FindX,int FindY);
	void FastFindProcessName(Edit *FindEdit,const string& Src,string &strLastName, string &strName);
	void DragMessage(int X,int Y,int Move);

protected:
	PanelViewSettings ViewSettings;
	string strCurDir;
	bool Focus;
	int Type;
	int EnableUpdate;
	int PanelMode;
	int SortMode;
	int SortOrder;
	bool SortGroups;
	int PrevViewMode,ViewMode;
	int CurTopFile;
	int CurFile;
	bool ShowShortNames;
	bool NumericSort;
	bool CaseSensitiveSort;
	bool DirectoriesFirst;
	int ModalMode;
	int PluginCommand;
	string strPluginParam;

private:
	string strDragName;
};

class dummy_panel : public Panel
{
	virtual void Update(int Mode) override {};
	virtual void UpdateKeyBar() override {}
};