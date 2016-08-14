#ifndef PANEL_HPP_FFA15B35_5546_4AA9_84B2_B60D8AA904C7
#define PANEL_HPP_FFA15B35_5546_4AA9_84B2_B60D8AA904C7
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
#include "panelfwd.hpp"

class DizList;

struct column
{
	unsigned long long type;
	int width;
	int width_type;
	string title;
};

struct PanelViewSettings
{
	NONCOPYABLE(PanelViewSettings);
	TRIVIALLY_MOVABLE(PanelViewSettings);

	PanelViewSettings(): Flags() {}

	PanelViewSettings clone() const
	{
		PanelViewSettings result;
		result.PanelColumns = PanelColumns;
		result.StatusColumns = StatusColumns;
		result.Name = Name;
		result.Flags = Flags;
		return result;
	}

	std::vector<column> PanelColumns;
	std::vector<column> StatusColumns;
	string Name;
	unsigned __int64 Flags;
};

enum
{
	PVS_NONE                  = 0,
	PVS_FULLSCREEN            = bit(0),
	PVS_ALIGNEXTENSIONS       = bit(1),
	PVS_FOLDERALIGNEXTENSIONS = bit(2),
	PVS_FOLDERUPPERCASE       = bit(3),
	PVS_FILELOWERCASE         = bit(5),
	PVS_FILEUPPERTOLOWERCASE  = bit(6),
};

enum {VIEW_0=0,VIEW_1,VIEW_2,VIEW_3,VIEW_4,VIEW_5,VIEW_6,VIEW_7,VIEW_8,VIEW_9};

enum {UPDATE_KEEP_SELECTION=1,UPDATE_SECONDARY=2,UPDATE_IGNORE_VISIBLE=4,UPDATE_DRAW_MESSAGE=8};

enum class panel_type
{
	FILE_PANEL,
	TREE_PANEL,
	QVIEW_PANEL,
	INFO_PANEL
};

enum class panel_mode
{
	NORMAL_PANEL,
	PLUGIN_PANEL
};

enum class panel_sort
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

	COUNT
};

enum sort_order
{
	SO_AUTO,
	SO_KEEPCURRENT,
	SO_DIRECT,
	SO_REVERSE,
};

class VMenu2;
class Edit;
struct PanelMenuItem;
class Viewer;
struct PluginHandle;
class FilePanels;

class Panel: public ScreenObject, public std::enable_shared_from_this<Panel>
{
public:
	virtual ~Panel();

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
	virtual void ReadDiz(PluginPanelItem *ItemList=nullptr,int ItemLength=0, DWORD dwFlags=0) {}
	virtual void DeleteDiz(const string& Name,const string& ShortName) {}
	virtual void GetDizName(string &strDizName) const {}
	virtual void FlushDiz() {}
	virtual void CopyDiz(const string& Name,const string& ShortName,const string& DestName, const string& DestShortName,DizList *DestDiz) {}
	virtual bool IsDizDisplayed() const { return false; }
	virtual bool IsColumnDisplayed(int Type) const {return false;}
	virtual int GetColumnsCount() const { return 1;}
	virtual void SetReturnCurrentFile(int Mode) {}
	virtual void QViewDelTempName() {}
	virtual void GetOpenPanelInfo(OpenPanelInfo *Info) const {}
	virtual void SetPluginMode(PluginHandle* hPlugin,const string& PluginFile,bool SendOnFocus=false) {}
	virtual void SetPluginModified() {}
	virtual int ProcessPluginEvent(int Event,void *Param) {return FALSE;}
	virtual PluginHandle* GetPluginHandle() const {return nullptr;}
	virtual void RefreshTitle();
	virtual string GetTitle() const;
	virtual __int64 VMProcess(int OpCode,void *vParam=nullptr,__int64 iParam=0) override;
	virtual int SendKeyToPlugin(DWORD Key,bool Pred=false) {return FALSE;}
	virtual bool SetCurDir(const string& NewDir,bool ClosePanel,bool IsUpdated=true);
	virtual void ChangeDirToCurrent();
	virtual const string& GetCurDir() const;
	virtual size_t GetSelCount() const { return 0; }
	virtual size_t GetRealSelCount() const {return 0;}
	virtual int GetSelName(string *strName, DWORD &FileAttr, string *ShortName = nullptr, os::FAR_FIND_DATA *fd = nullptr) { return FALSE; }
	virtual void UngetSelName() {}
	virtual void ClearLastGetSelection() {}
	virtual unsigned __int64 GetLastSelectedSize() const { return -1; }
	virtual int GetCurName(string &strName, string &strShortName) const;
	virtual int GetCurBaseName(string &strName, string &strShortName) const;
	virtual int GetFileName(string &strName, int Pos, DWORD &FileAttr) const { return FALSE; }
	virtual int GetCurrentPos() const {return 0;}

	virtual bool IsFocused() const;
	virtual void OnFocusChange(bool Get);

	virtual void Update(int Mode) = 0;
	virtual bool UpdateIfChanged(bool Idle) {return false;}
	virtual void UpdateIfRequired() {}
	virtual void StartFSWatcher(bool got_focus=false, bool check_time=true) {}
	virtual void StopFSWatcher() {}
	virtual int FindPartName(const string& Name,int Next,int Direct=1) {return FALSE;}
	virtual bool GetPlainString(string& Dest, int ListPos) const { return false; }
	virtual int GoToFile(long idxItem) {return TRUE;}
	virtual int GoToFile(const string& Name,BOOL OnlyPartName=FALSE) {return TRUE;}
	virtual long FindFile(const string& Name,BOOL OnlyPartName=FALSE) {return -1;}
	virtual int IsSelected(const string& Name) {return FALSE;}
	virtual int IsSelected(size_t indItem) {return FALSE;}
	virtual long FindFirst(const string& Name) {return -1;}
	virtual long FindNext(int StartPos, const string& Name) {return -1;}
	virtual void SetSelectedFirstMode(bool) {}
	virtual bool GetSelectedFirstMode() const { return false; }
	virtual void SetViewMode(int ViewMode);
	virtual int GetPrevViewMode() const {return m_PrevViewMode;}
	virtual panel_sort GetPrevSortMode() const { return m_SortMode; }
	virtual bool GetPrevSortOrder() const { return m_ReverseSortOrder; }
	virtual bool GetPrevNumericSort() const { return m_NumericSort; }
	virtual void ChangeNumericSort(bool Mode) { SetNumericSort(Mode); }
	virtual bool GetPrevCaseSensitiveSort() const { return m_CaseSensitiveSort; }
	virtual void ChangeCaseSensitiveSort(bool Mode) {SetCaseSensitiveSort(Mode);}
	virtual bool GetPrevDirectoriesFirst() const { return m_DirectoriesFirst; }
	virtual void ChangeDirectoriesFirst(bool Mode) { SetDirectoriesFirst(Mode); }
	virtual void SetSortMode(panel_sort Mode, bool KeepOrder = false) { m_SortMode = Mode; }
	virtual void SetCustomSortMode(int SortMode, sort_order Order = SO_AUTO, bool InvertByDefault = false) {}
	virtual void ChangeSortOrder(bool Reverse) {SetSortOrder(Reverse);}
	virtual void IfGoHome(wchar_t Drive) {}
	virtual void UpdateKeyBar() = 0;
	virtual size_t GetFileCount() const { return 0; }
	virtual void Hide() override;
	virtual void Show() override;
	virtual void DisplayObject() override {}
	virtual Viewer* GetViewer(void) {return nullptr;}
	virtual Viewer* GetById(int ID) {(void)ID; return nullptr;}
	virtual void ShowConsoleTitle() override;

	static void exclude_sets(string& mask);

	panel_mode GetMode() const { return m_PanelMode; }
	void SetMode(panel_mode Mode) { m_PanelMode = Mode; }
	int GetModalMode() const { return m_ModalMode; }
	int GetViewMode() const { return m_ViewMode; }
	void SetPrevViewMode(int PrevViewMode) {m_PrevViewMode=PrevViewMode;}
	panel_sort GetSortMode() const { return m_SortMode; }
	bool GetNumericSort() const { return m_NumericSort; }
	void SetNumericSort(bool Mode) { m_NumericSort = Mode; }
	bool GetCaseSensitiveSort() const { return m_CaseSensitiveSort; }
	void SetCaseSensitiveSort(bool Mode) {m_CaseSensitiveSort = Mode;}
	bool GetDirectoriesFirst() const { return m_DirectoriesFirst; }
	void SetDirectoriesFirst(bool Mode) { m_DirectoriesFirst = Mode != 0; }
	bool GetSortOrder() const { return m_ReverseSortOrder; }
	void SetSortOrder(bool Reverse) {m_ReverseSortOrder = Reverse;}
	bool GetSortGroups() const { return m_SortGroups; }
	void SetSortGroups(bool Mode) {m_SortGroups=Mode;}
	bool GetShowShortNamesMode() const { return m_ShowShortNames; }
	void SetShowShortNamesMode(bool Mode) {m_ShowShortNames=Mode;}
	void InitCurDir(const string& CurDir);
	bool ExecShortcutFolder(int Pos, bool raw=false);
	bool ExecShortcutFolder(string& strShortcutFolder, const GUID& PluginGuid, const string& strPluginFile, const string& strPluginData, bool CheckType, bool TryClosest = true, bool Silent = false);
	bool SaveShortcutFolder(int Pos, bool Add) const;
	int SetPluginCommand(int Command,int Param1,void* Param2);
	int ProcessMouseDrag(const MOUSE_EVENT_RECORD *MouseEvent);
	bool IsMouseInClientArea(const MOUSE_EVENT_RECORD *MouseEvent) const;
	panel_type GetType() const {return m_Type;}
	void SetUpdateMode(int Mode) {m_EnableUpdate=Mode;}
	bool MakeListFile(string &strListFileName,bool ShortNames,const string& Modifers);
	int SetCurPath();
	BOOL NeedUpdatePanel(const Panel *AnotherPanel) const;
	bool IsFullScreen() const { return (m_ViewSettings.Flags & PVS_FULLSCREEN) != 0; }
	void SetFullScreen() { m_ViewSettings.Flags |= PVS_FULLSCREEN; }
	bool CreateFullPathName(const string& Name, const string& ShortName, DWORD FileAttr, string &strDest, int UNC, int ShortNameAsIs = TRUE) const;
	FilePanels* Parent() const;

	static void EndDrag();

	int ProcessingPluginCommand;

protected:
	Panel(window_ptr Owner);
	virtual void ClearAllItem(){}

	void FastFind(const Manager::Key& FirstKey);
	void DrawSeparator(int Y) const;
	void ShowScreensCount() const;
	string GetTitleForDisplay() const;

	static bool IsDragging();

private:
	struct ShortcutInfo
	{
		string ShortcutFolder;
		string PluginFile;
		string PluginData;
		GUID PluginGuid;
	};
	bool GetShortcutInfo(ShortcutInfo& Info) const;

	static void DragMessage(int X,int Y,int Move);

protected:
	PanelViewSettings m_ViewSettings;
	string m_CurDir;
	panel_type m_Type;
	int m_EnableUpdate;
	panel_mode m_PanelMode;
	panel_sort m_SortMode;
	bool m_ReverseSortOrder;
	bool m_SortGroups;
	int m_PrevViewMode;
	int m_ViewMode;
	int m_CurTopFile;
	int m_CurFile;
	bool m_ShowShortNames;
	bool m_NumericSort;
	bool m_CaseSensitiveSort;
	bool m_DirectoriesFirst;
	int m_ModalMode;
	int m_PluginCommand;
	string m_PluginParam;
	string m_Title;

private:
	string strDragName;
};

class dummy_panel : public Panel
{
public:
	dummy_panel(window_ptr Owner): Panel(Owner){}

private:
	virtual void Update(int Mode) override {};
	virtual void UpdateKeyBar() override {}
};

#endif // PANEL_HPP_FFA15B35_5546_4AA9_84B2_B60D8AA904C7
