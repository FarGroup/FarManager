#ifndef FILELIST_HPP_825FE8AE_1E34_4DFD_B167_2D6A121B1777
#define FILELIST_HPP_825FE8AE_1E34_4DFD_B167_2D6A121B1777
#pragma once

/*
filelist.hpp

Файловая панель - общие функции
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

#include "panel.hpp"
#include "dizlist.hpp"
#include "hilight.hpp"
#include "filesystemwatcher.hpp"
#include "plugins.hpp"

class FileFilter;
class Plugin;

using content_data_ptr = std::unique_ptr<std::unordered_map<string, string>>;

class FileListItem
{
public:
	NONCOPYABLE(FileListItem);
	MOVABLE(FileListItem);

	FileListItem();
	FileListItem(const PluginPanelItem& pi);

	bool IsNumberOfLinksRead() const;
	DWORD NumberOfLinks(const FileList* Owner) const;

	bool IsNumberOfStreamsRead() const;
	DWORD NumberOfStreams(const FileList* Owner) const;

	bool IsStreamsSizeRead() const;
	unsigned long long StreamsSize(const FileList* Owner) const;

	bool IsOwnerRead() const;
	const string& Owner(const FileList* Owner) const;

	bool IsContentDataRead() const;
	const content_data_ptr& ContentData(const FileList* Owner) const;

	// KEEP ALIGNED!
	FILETIME CreationTime{};
	FILETIME AccessTime{};
	FILETIME WriteTime{};
	FILETIME ChangeTime{};

	unsigned long long FileSize{};
	unsigned long long AllocationSize{};

	unsigned long long UserFlags{};
	UserDataItem UserData{};

	DWORD FileAttr{};
	DWORD ReparseTag{};

	mutable const HighlightFiles::highlight_item* Colors{};

	wchar_t** CustomColumnData{};
	size_t CustomColumnNumber{};
	size_t Position{};

	int SortGroup{};
	DWORD CRC32{};

	const wchar_t *DizText{};

	bool Selected{};
	bool PrevSelected{};
	char ShowFolderSize{};

	string strName;
	string strShortName;

	struct values
	{
		template<class T>
		static T uninitialised(const T&) { return -2; }
		template<class T>
		static T unknown(const T&) { return -1; }
	};

private:
	mutable string m_Owner;
	mutable DWORD m_NumberOfLinks = values::uninitialised(m_NumberOfLinks);
	mutable DWORD m_NumberOfStreams = values::uninitialised(m_NumberOfStreams);
	mutable unsigned long long m_StreamsSize = values::uninitialised(m_StreamsSize);
	mutable content_data_ptr m_ContentData;
};

struct PluginsListItem
{
	NONCOPYABLE(PluginsListItem);
	MOVABLE(PluginsListItem);

	PluginsListItem(plugin_panel* hPlugin, const string& HostFile, int Modified, int PrevViewMode, panel_sort PrevSortMode, bool PrevSortOrder, bool PrevNumericSort, bool PrevCaseSensitiveSort, bool PrevDirectoriesFirst, const PanelViewSettings& PrevViewSettings):
		m_Plugin(hPlugin),
		m_HostFile(HostFile),
		m_Modified(Modified),
		m_PrevViewMode(PrevViewMode),
		m_PrevSortMode(PrevSortMode),
		m_PrevSortOrder(PrevSortOrder),
		m_PrevNumericSort(PrevNumericSort),
		m_PrevCaseSensitiveSort(PrevCaseSensitiveSort),
		m_PrevDirectoriesFirst(PrevDirectoriesFirst),
		m_PrevViewSettings(PrevViewSettings.clone())
	{}

	plugin_panel* m_Plugin;
	string m_HostFile;
	int m_Modified;
	int m_PrevViewMode;
	panel_sort m_PrevSortMode;
	bool m_PrevSortOrder;
	bool m_PrevNumericSort;
	bool m_PrevCaseSensitiveSort;
	bool m_PrevDirectoriesFirst;
	PanelViewSettings m_PrevViewSettings;
};

enum OPENFILEPLUGINTYPE: int;

class FileList:public Panel
{
	struct private_tag
	{
	};

public:
	static file_panel_ptr create(window_ptr Owner);
	FileList(private_tag, window_ptr Owner);
	virtual ~FileList() override;

	virtual bool ProcessKey(const Manager::Key& Key) override;
	virtual bool ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent) override;
	virtual long long VMProcess(int OpCode, void* vParam = nullptr, long long iParam = 0) override;
	virtual void MoveToMouse(const MOUSE_EVENT_RECORD *MouseEvent) override;
	virtual void Update(int Mode) override;
	virtual bool UpdateIfChanged(bool Idle) override;
	virtual void UpdateIfRequired() override;
	virtual bool SendKeyToPlugin(DWORD Key, bool Pred = false) override;
	virtual void StartFSWatcher(bool got_focus = false, bool check_time = true) override;
	virtual void StopFSWatcher() override;
	virtual void SortFileList(bool KeepPosition) override;
	virtual void SetViewMode(int ViewMode) override;
	virtual void SetSortMode(panel_sort SortMode, bool KeepOrder = false) override;
	virtual void SetCustomSortMode(int SortMode, sort_order Order = SO_AUTO, bool InvertByDefault = false) override;
	virtual void ChangeSortOrder(bool Reverse) override;
	virtual void ChangeNumericSort(bool Mode) override;
	virtual void ChangeCaseSensitiveSort(bool Mode) override;
	virtual void ChangeDirectoriesFirst(bool Mode) override;
	virtual bool SetCurDir(const string& NewDir, bool ClosePanel, bool IsUpdated = true) override;
	virtual panel_sort GetPrevSortMode() const override;
	virtual bool GetPrevSortOrder() const override;
	virtual int GetPrevViewMode() const override;
	virtual bool GetPrevNumericSort() const override;
	virtual bool GetPrevCaseSensitiveSort() const override;
	virtual bool GetPrevDirectoriesFirst() const override;
	virtual bool GetFileName(string &strName, int Pos, DWORD &FileAttr) const override;
	virtual int GetCurrentPos() const override;
	virtual bool FindPartName(const string& Name, int Next, int Direct = 1) override;
	virtual bool GetPlainString(string& Dest, int ListPos) const override;
	virtual bool GoToFile(long idxItem) override;
	virtual bool GoToFile(const string& Name, bool OnlyPartName = false) override;
	virtual long FindFile(const string& Name, bool OnlyPartName = false) override;
	virtual bool IsSelected(const string& Name) override;
	virtual bool IsSelected(size_t idxItem) override;
	virtual long FindFirst(const string& Name) override;
	virtual long FindNext(int StartPos, const string& Name) override;
	virtual void UpdateViewPanel() override;
	virtual void CompareDir() override;
	virtual void ClearSelection() override;
	virtual void SaveSelection() override;
	virtual void RestoreSelection() override;
	virtual void EditFilter() override;
	virtual bool FileInFilter(size_t idxItem) override;
	virtual bool FilterIsEnabled() override;
	virtual void ReadDiz(PluginPanelItem *ItemList = nullptr, int ItemLength = 0, DWORD dwFlags = 0) override;
	virtual void DeleteDiz(const string& Name, const string& ShortName) override;
	virtual void FlushDiz() override;
	virtual void GetDizName(string &strDizName) const override;
	virtual void CopyDiz(const string& Name, const string& ShortName, const string& DestName, const string& DestShortName, DizList *DestDiz) override;
	virtual bool IsDizDisplayed() const override;
	virtual bool IsColumnDisplayed(int Type) const override;
	virtual int GetColumnsCount() const override
	{
		return m_Columns;
	}
	virtual void SetReturnCurrentFile(bool Mode) override;
	virtual void GetOpenPanelInfo(OpenPanelInfo *Info) const override;
	virtual void SetPluginMode(plugin_panel* hPlugin, const string& PluginFile, bool SendOnFocus = false) override;
	virtual size_t GetSelCount() const override;
	virtual bool GetSelName(string *strName, DWORD &FileAttr, string *strShortName = nullptr, os::FAR_FIND_DATA *fde = nullptr) override;
	virtual void UngetSelName() override;
	virtual void ClearLastGetSelection() override;
	virtual unsigned long long GetLastSelectedSize() const override;
	virtual plugin_panel* GetPluginHandle() const override;
	virtual size_t GetRealSelCount() const override;
	virtual void SetPluginModified() override;
	virtual bool ProcessPluginEvent(int Event, void *Param) override;
	virtual void RefreshTitle() override;
	virtual size_t GetFileCount() const override;
	virtual void UpdateKeyBar() override;
	virtual void IfGoHome(wchar_t Drive) override;
	virtual bool GetSelectedFirstMode() const override
	{
		return SelectedFirst;
	}

	const FileListItem* GetItem(size_t Index) const;
	const FileListItem* GetLastSelectedItem() const;

	plugin_panel* OpenFilePlugin(const string& FileName, int PushPrev, OPENFILEPLUGINTYPE Type);
	void ProcessHostFile();
	bool GetPluginInfo(PluginInfo *PInfo) const;
	void PluginGetPanelInfo(PanelInfo &Info);
	size_t PluginGetPanelItem(int ItemNumber, FarGetPluginPanelItem *Item);
	size_t PluginGetSelectedPanelItem(int ItemNumber, FarGetPluginPanelItem *Item);
	void PluginGetColumnTypesAndWidths(string& strColumnTypes, string& strColumnWidths) const;
	void PluginBeginSelection();
	void PluginSetSelection(int ItemNumber, bool Selection);
	void PluginClearSelection(int SelectedItemNumber);
	void PluginEndSelection();
	bool PluginPanelHelp(const plugin_panel* hPlugin) const;
	void ResetLastUpdateTime()
	{
		LastUpdateTime = 0;
	}
	string GetPluginPrefix() const;

	size_t FileListToPluginItem2(const FileListItem& fi, FarGetPluginPanelItem* pi) const;
	static bool FileNameToPluginItem(const string& Name, class PluginPanelItemHolder& pi);
	void FileListToPluginItem(const FileListItem& fi, PluginPanelItemHolder& pi) const;
	static bool IsModeFullScreen(int Mode);

	struct PrevDataItem;

protected:
	virtual void ClearAllItem() override;

private:
	friend class FileListItem;

	class list_data;

	bool HardlinksSupported() const
	{
		return m_HardlinksSupported;
	}
	bool StreamsSupported() const
	{
		return m_StreamsSupported;
	}
	const string& GetComputerName() const
	{
		return m_ComputerName;
	}
	content_data_ptr GetContentData(const string& Item) const;

	virtual void SetSelectedFirstMode(bool Mode) override;
	virtual void DisplayObject() override;
	virtual bool GetCurName(string &strName, string &strShortName) const override;
	virtual bool GetCurBaseName(string &strName, string &strShortName) const override;

	void ApplySortMode(panel_sort Mode);
	void ToBegin();
	void ToEnd();
	void MoveCursor(int offset);
	void MoveCursorAndShow(int offset);
	void Scroll(int offset);
	void CorrectPosition();
	void ShowFileList(bool Fast = true);
	void ShowList(int ShowStatus, int StartColumn);
	void SetShowColor(int Position, bool FileColor = true) const;
	FarColor GetShowColor(int Position, bool FileColor = true) const;
	void ShowSelectedSize();
	void ShowTotalSize(const OpenPanelInfo &Info);
	bool ConvertName(const wchar_t *SrcName, string &strDest, int MaxLength, unsigned long long RightAlign, int ShowStatus, DWORD dwFileAttr) const;
	void Select(FileListItem& SelItem, bool Selection);
	long SelectFiles(int Mode, const wchar_t *Mask = nullptr);
	void ProcessEnter(bool EnableExec, bool SeparateWindow, bool EnableAssoc, bool RunAs, OPENFILEPLUGINTYPE Type);
	// ChangeDir возвращает false, eсли не смогла выставить заданный путь
	bool ChangeDir(const string& NewDir,bool ResolvePath=false, bool IsUpdated=true, const UserDataItem* DataItem = nullptr, OPENFILEPLUGINTYPE Type=OFP_NORMAL);
	void CountDirSize(bool IsRealNames);
	void ReadFileNames(int KeepSelection, int UpdateEvenIfPanelInvisible, int DrawMessage);
	void UpdatePlugin(int KeepSelection, int UpdateEvenIfPanelInvisible);
	void MoveSelection(list_data& From, list_data& To);
	void PushPlugin(plugin_panel* hPlugin, const string& HostFile);
	bool PopPlugin(int EnableRestoreViewMode);
	void PopPrevData(const string& DefaultName, bool Closed, bool UsePrev, bool Position, bool SetDirectorySuccess);
	void CopyFiles(bool bMoved = false);
	void CopyNames(bool FillPathName, bool UNC);
	void SelectSortMode();
	bool ApplyCommand();
	void DescribeFiles();
	std::vector<PluginPanelItem> CreatePluginItemList(bool AddTwoDot = true);
	plugin_panel* OpenPluginForFile(const string& FileName, DWORD FileAttr, OPENFILEPLUGINTYPE Type);
	int PreparePanelView(PanelViewSettings *PanelView);
	int PrepareColumnWidths(std::vector<column>& Columns, bool FullScreen, bool StatusLine);
	void PrepareViewSettings(int ViewMode);
	void PluginDelete();
	void PutDizToPlugin(FileList *DestPanel, const std::vector<PluginPanelItem>& ItemList, int Delete, int Move, DizList *SrcDiz);
	void PluginGetFiles(const wchar_t **DestPath, int Move);
	void PluginToPluginFiles(int Move);
	void PluginHostGetFiles();
	void PluginPutFilesToNew();
	int PluginPutFilesToAnother(int Move, panel_ptr AnotherPanel);
	void ProcessPluginCommand();
	void PluginClearSelection(const std::vector<PluginPanelItem>& ItemList);
	void ProcessCopyKeys(int Key);
	void ReadSortGroups(bool UpdateFilterCurrentTime = true);
	int ProcessOneHostFile(const FileListItem* Item);
	void HighlightBorder(int Level, int ListPos) const;
	void InitFSWatcher(bool CheckTree);
	bool IsColumnDisplayed(std::function<bool(const column&)> Compare) const;
	void UpdateHeight();
	enum direction
	{
		up, down
	};
	void MoveSelection(direction Direction);

	static void DeletePluginItemList(std::vector<PluginPanelItem> &ItemList);
	static void FillParentPoint(FileListItem& Item, size_t CurFilePos, const FILETIME* Times = nullptr);

	std::unique_ptr<FileFilter> m_Filter;
	DizList Diz;
	bool DizRead{};
	/* $ 09.11.2001 IS
		 Открывающий и закрывающий символ, которые используются для показа
		 имени, которое не помещается в панели. По умолчанию - фигурные скобки.
	*/
	wchar_t openBracket[2]{}, closeBracket[2]{};

	string strOriginalCurDir;
	string strPluginDizName;

	class list_data
	{
	public:
		NONCOPYABLE(list_data);
		MOVABLE(list_data);

		using value_type = FileListItem;

		list_data() {}
		~list_data() { clear(); }

		void initialise(plugin_panel* ph) { clear(); m_Plugin = ph; }

		void clear();
		decltype(auto) size() const { return Items.size(); }
		decltype(auto) empty() const { return Items.empty(); }
		decltype(auto) begin() const { return Items.begin(); }
		decltype(auto) end() const { return Items.end(); }
		decltype(auto) begin() { return Items.begin(); }
		decltype(auto) end() { return Items.end(); }
		decltype(auto) cbegin() { return Items.cbegin(); }
		decltype(auto) cend() { return Items.cend(); }
		decltype(auto) front() const { return Items.front(); }
		decltype(auto) back() const { return Items.back(); }
		decltype(auto) front() { return Items.front(); }
		decltype(auto) back() { return Items.back(); }
		decltype(auto) operator[](size_t Index) const { return Items[Index]; }
		decltype(auto) operator[](size_t Index) { return Items[Index]; }
		decltype(auto) data() const { return Items.data(); }
		decltype(auto) resize(size_t Size) { return Items.resize(Size); }
		decltype(auto) reserve(size_t Size) { return Items.reserve(Size); }
		template<typename... args>
		decltype(auto) emplace_back(args&&... Args) { return Items.emplace_back(std::forward<args>(Args)...); }
		template<typename T>
		decltype(auto) push_back(T&& Value) { return Items.push_back(std::forward<T>(Value)); }
	private:
		std::vector<FileListItem> Items;
		plugin_panel* m_Plugin{};
	}
	m_ListData;
	plugin_panel* m_hPlugin{};
	std::list<PrevDataItem> PrevDataList;
	std::list<PluginsListItem> PluginsList;
	FileSystemWatcher FSWatcher;
	long UpperFolderTopFile{}, LastCurFile{ -1 };
	bool ReturnCurrentFile{};
	size_t m_SelFileCount{}; // both files and directories
	size_t m_SelDirCount{}; // directories only
	long GetSelPosition{}, LastSelPosition{ -1 };
	size_t m_TotalFileCount{}; // files only
	size_t m_TotalDirCount{}; // directories only
	unsigned long long SelFileSize{};
	unsigned long long TotalFileSize{};
	unsigned long long FreeDiskSize = -1;
	clock_t LastUpdateTime{};
	int m_Height{};
	int m_Columns{};
	int ColumnsInGlobal{};
	int LeftPos{};
	int ShiftSelection{ -1 };
	bool MouseSelection{};
	bool SelectedFirst{};
	bool empty{}; // указывает на полностью пустую колонку
	bool AccessTimeUpdateRequired{};
	bool UpdateRequired{};
	int UpdateRequiredMode{};
	int UpdateDisabled{};
	bool SortGroupsRead{};
	int InternalProcessKey{};
	long CacheSelIndex{ -1 }, CacheSelPos{};
	long CacheSelClearIndex{ -1 }, CacheSelClearPos{};
	wchar_t CustomSortIndicator[2]{};
	mutable OpenPanelInfo m_CachedOpenPanelInfo{};
	bool m_HardlinksSupported{};
	bool m_StreamsSupported{};
	string m_ComputerName;
	std::vector<string> m_ContentNames;
	std::vector<const wchar_t*> m_ContentNamesPtrs;
	mutable std::vector<const wchar_t*> m_ContentValues;
	std::vector<Plugin*> m_ContentPlugins;
};

#endif // FILELIST_HPP_825FE8AE_1E34_4DFD_B167_2D6A121B1777
