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

class plugin_panel;
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
	MOVABLE(PanelViewSettings);

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
	unsigned long long Flags;
};

enum panel_view_settings_flags
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

enum
{
	UPDATE_KEEP_SELECTION = bit(0),
	UPDATE_SECONDARY      = bit(1),
	UPDATE_IGNORE_VISIBLE = bit(2),
	UPDATE_DRAW_MESSAGE   = bit(3),
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
class FilePanels;

class Panel: public ScreenObject, public std::enable_shared_from_this<Panel>
{
public:
	~Panel() override;

	void Show() override;
	void Hide() override;
	void DisplayObject() override {}
	void ShowConsoleTitle() override;
	long long VMProcess(int OpCode, void* vParam = nullptr, long long iParam = 0) override;

	// TODO: make empty methods pure virtual, move empty implementations to dummy_panel class
	virtual void CloseFile() {}
	virtual void UpdateViewPanel() {}
	virtual void CompareDir() {}
	virtual void MoveToMouse(const MOUSE_EVENT_RECORD *MouseEvent) {}
	virtual void ClearSelection() {}
	virtual void SaveSelection() {}
	virtual void RestoreSelection() {}
	virtual void SortFileList(bool KeepPosition) {}
	virtual void EditFilter() {}
	virtual bool FileInFilter(size_t idxItem) {return true;}
	virtual bool FilterIsEnabled() {return false;}
	virtual void ReadDiz(PluginPanelItem *ItemList=nullptr,int ItemLength=0, DWORD dwFlags=0) {}
	virtual void DeleteDiz(const string& Name,const string& ShortName) {}
	virtual string GetDizName() const { return {}; }
	virtual void FlushDiz() {}
	virtual void CopyDiz(const string& Name,const string& ShortName,const string& DestName, const string& DestShortName,DizList *DestDiz) {}
	virtual bool IsDizDisplayed() const { return false; }
	virtual bool IsColumnDisplayed(int Type) const {return false;}
	virtual int GetColumnsCount() const { return 1;}
	virtual void SetReturnCurrentFile(bool Mode) {}
	virtual void QViewDelTempName() {}
	virtual void GetOpenPanelInfo(OpenPanelInfo *Info) const {}
	virtual void SetPluginMode(std::unique_ptr<plugin_panel>&& hPlugin,const string& PluginFile,bool SendOnFocus=false) {}
	virtual void SetPluginModified() {}
	virtual bool ProcessPluginEvent(int Event,void *Param) {return false;}
	virtual plugin_panel* GetPluginHandle() const {return nullptr;}
	virtual void RefreshTitle();
	virtual string GetTitle() const;
	virtual bool SendKeyToPlugin(DWORD Key,bool Pred=false) {return false;}
	virtual bool SetCurDir(const string& NewDir,bool ClosePanel,bool IsUpdated=true);
	virtual void ChangeDirToCurrent();
	virtual const string& GetCurDir() const;
	virtual size_t GetSelCount() const { return 0; }
	virtual size_t GetRealSelCount() const {return 0;}
	virtual bool GetSelName(string *strName, DWORD &FileAttr, string *ShortName = nullptr, os::fs::find_data *fd = nullptr) { return false; }
	virtual void ClearLastGetSelection() {}
	virtual bool GetCurName(string &strName, string &strShortName) const;
	virtual bool GetCurBaseName(string &strName, string &strShortName) const;
	virtual bool GetFileName(string &strName, int Pos, DWORD &FileAttr) const { return false; }
	virtual int GetCurrentPos() const {return 0;}
	virtual bool IsFocused() const;
	virtual void OnFocusChange(bool Get);
	virtual void Update(int Mode) = 0;
	virtual bool UpdateIfChanged(bool Idle) {return false;}
	virtual void UpdateIfRequired() {}
	virtual void StartFSWatcher(bool got_focus=false, bool check_time=true) {}
	virtual void StopFSWatcher() {}
	virtual bool FindPartName(const string& Name,int Next,int Direct=1) {return false;}
	virtual bool GetPlainString(string& Dest, int ListPos) const { return false; }
	virtual bool GoToFile(long idxItem) {return true;}
	virtual bool GoToFile(string_view Name, bool OnlyPartName = false) {return true;}
	virtual long FindFile(string_view Name, bool OnlyPartName = false) {return -1;}
	virtual bool IsSelected(const string& Name) {return false;}
	virtual bool IsSelected(size_t indItem) {return false;}
	virtual long FindFirst(const string& Name) {return -1;}
	virtual long FindNext(int StartPos, const string& Name) {return -1;}
	virtual void SetSelectedFirstMode(bool) {}
	virtual bool GetSelectedFirstMode() const { return false; }
	virtual void SetViewMode(int ViewMode);
	virtual int GetPrevViewMode() const {return m_PrevViewMode;}
	virtual panel_sort GetPrevSortMode() const { return m_SortMode; }
	virtual bool GetPrevSortOrder() const { return m_ReverseSortOrder; }
	virtual bool GetPrevDirectoriesFirst() const { return m_DirectoriesFirst; }
	virtual void ChangeDirectoriesFirst(bool Mode) { SetDirectoriesFirst(Mode); }
	virtual void OnSortingChange() {}
	virtual void SetSortMode(panel_sort Mode, bool KeepOrder = false) { m_SortMode = Mode; }
	virtual void SetCustomSortMode(int SortMode, sort_order Order = SO_AUTO, bool InvertByDefault = false) {}
	virtual void ChangeSortOrder(bool Reverse) {SetSortOrder(Reverse);}
	virtual void IfGoHome(wchar_t Drive) {}
	virtual void UpdateKeyBar() = 0;
	virtual size_t GetFileCount() const { return 0; }
	virtual Viewer* GetViewer() {return nullptr;}
	virtual Viewer* GetById(int ID) { return nullptr;}

	static void exclude_sets(string& mask);

	panel_mode GetMode() const { return m_PanelMode; }
	void SetMode(panel_mode Mode) { m_PanelMode = Mode; }
	int GetModalMode() const { return m_ModalMode; }
	int GetViewMode() const { return m_ViewMode; }
	void SetPrevViewMode(int PrevViewMode) {m_PrevViewMode=PrevViewMode;}
	panel_sort GetSortMode() const { return m_SortMode; }
	bool GetDirectoriesFirst() const { return m_DirectoriesFirst; }
	void SetDirectoriesFirst(bool Mode) { m_DirectoriesFirst = Mode != 0; }
	bool GetSortOrder() const { return m_ReverseSortOrder; }
	void SetSortOrder(bool Reverse) {m_ReverseSortOrder = Reverse;}
	bool GetSortGroups() const { return m_SortGroups; }
	void SetSortGroups(bool Mode) {m_SortGroups=Mode;}
	bool GetShowShortNamesMode() const { return m_ShowShortNames; }
	void SetShowShortNamesMode(bool Mode) {m_ShowShortNames=Mode;}
	void InitCurDir(const string& CurDir);
	bool ExecShortcutFolder(int Pos);
	bool ExecShortcutFolder(string strShortcutFolder, const GUID& PluginGuid, const string& strPluginFile, const string& strPluginData, bool CheckType, bool TryClosest = true, bool Silent = false);
	bool SaveShortcutFolder(int Pos) const;
	int SetPluginCommand(int Command,int Param1,void* Param2);
	bool ProcessMouseDrag(const MOUSE_EVENT_RECORD* MouseEvent);
	bool IsMouseInClientArea(const MOUSE_EVENT_RECORD *MouseEvent) const;
	panel_type GetType() const {return m_Type;}
	void SetUpdateMode(int Mode) {m_EnableUpdate=Mode;}
	bool MakeListFile(string &strListFileName,bool ShortNames,const string& Modifers);
	bool SetCurPath();
	bool NeedUpdatePanel(const Panel *AnotherPanel) const;
	bool IsFullScreen() const { return (m_ViewSettings.Flags & PVS_FULLSCREEN) != 0; }
	void SetFullScreen() { m_ViewSettings.Flags |= PVS_FULLSCREEN; }
	string CreateFullPathName(string_view Name, bool Directory, bool UNC, bool ShortNameAsIs = true) const;
	FilePanels* Parent() const;

	static void EndDrag();

	int ProcessingPluginCommand = 0;

	auto enum_selected()
	{
		using value_type = os::fs::find_data;
		return make_inline_enumerator<value_type>([this](const bool Reset, value_type& Value)
		{
			DWORD Attributes;

			if (Reset)
				GetSelName(nullptr, Attributes);

			string Name;
			return GetSelName(&Name, Attributes, nullptr, &Value);
		});
	}

	bool get_first_selected(os::fs::find_data& Value)
	{
		DWORD Attributes;
		GetSelName(nullptr, Attributes);
		string Name;
		return GetSelName(&Name, Attributes, nullptr, &Value);
	}

protected:
	explicit Panel(window_ptr Owner);
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
	bool SetPluginDirectory(const string& strDirectory, bool Silent);

	static void DragMessage(int X,int Y,int Move);

protected:
	PanelViewSettings m_ViewSettings;
	string m_CurDir;
	panel_type m_Type = panel_type::FILE_PANEL;
	int m_EnableUpdate = TRUE;
	panel_mode m_PanelMode = panel_mode::NORMAL_PANEL;
	panel_sort m_SortMode = panel_sort::UNSORTED;
	bool m_ReverseSortOrder = false;
	bool m_SortGroups = false;
	int m_PrevViewMode = VIEW_3;
	int m_ViewMode = 0;
	int m_CurTopFile = 0;
	int m_CurFile = 0;
	bool m_ShowShortNames = false;
	bool m_DirectoriesFirst = true;
	int m_ModalMode = 0;
	int m_PluginCommand = -1;
	string m_PluginParam;
	string m_Title;

private:
	string strDragName;
};

class dummy_panel : public Panel
{
public:
	explicit dummy_panel(window_ptr Owner):
		Panel(std::move(Owner))
	{
	}

private:
	void Update(int Mode) override {};
	void UpdateKeyBar() override {}
};

#endif // PANEL_HPP_FFA15B35_5546_4AA9_84B2_B60D8AA904C7
