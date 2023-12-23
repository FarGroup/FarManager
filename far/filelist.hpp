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

// Internal:
#include "panel.hpp"
#include "dizlist.hpp"
#include "filesystemwatcher.hpp"
#include "plugin.hpp"

// Platform:
#include "platform.fs.hpp"

// Common:
#include "common/function_ref.hpp"
#include "common/string_utils.hpp"

// External:

//----------------------------------------------------------------------------

class multifilter;
class Plugin;
class plugin_item_list;

namespace highlight
{
	class element;
}

using content_data = unordered_string_map<string>;

class FileListItem: public os::fs::find_data
{
public:
	NONCOPYABLE(FileListItem);
	MOVABLE(FileListItem);

	FileListItem();
	explicit FileListItem(const PluginPanelItem& pi);

	bool IsNumberOfLinksRead() const;
	DWORD NumberOfLinks(const FileList* Owner) const;

	bool IsNumberOfStreamsRead() const;
	DWORD NumberOfStreams(const FileList* Owner) const;

	bool IsStreamsSizeRead() const;
	unsigned long long StreamsSize(const FileList* Owner) const;

	bool IsOwnerRead() const;
	const string& Owner(const FileList* Owner) const;

	bool IsContentDataRead() const;
	const std::unique_ptr<content_data>& ContentData(const FileList* Owner) const;

	const string& AlternateOrNormal(bool Alternate) const;

	unsigned long long UserFlags{};
	UserDataItem UserData{};

	mutable const highlight::element* Colors{};

	std::vector<const wchar_t*> CustomColumns;

	size_t Position{};

	int SortGroup{};
	DWORD CRC32{};

	const wchar_t *DizText{};
	bool DeleteDiz{};

	bool Selected{};
	bool PrevSelected{};
	char ShowFolderSize{};

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
	mutable std::unique_ptr<content_data> m_ContentData;
};

enum OPENFILEPLUGINTYPE: int;

class FileList final: public Panel
{
	struct private_tag { explicit private_tag() = default; };

public:
	static file_panel_ptr create(window_ptr Owner);
	FileList(private_tag, window_ptr Owner);
	~FileList() override;

	bool ProcessKey(const Manager::Key& Key) override;
	bool ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent) override;
	long long VMProcess(int OpCode, void* vParam = nullptr, long long iParam = 0) override;
	void MoveToMouse(const MOUSE_EVENT_RECORD *MouseEvent) override;
	void Update(int Mode) override;
	void UpdateIfChanged(bool Changed = false) override;
	void UpdateIfRequired() override;
	bool SendKeyToPlugin(DWORD Key, bool Pred = false) override;
	void StopFSWatcher() override;
	void SortFileList(bool KeepPosition) override;
	void SetViewMode(int Mode) override;
	void SetSortMode(panel_sort Mode, bool KeepOrder = false) override;
	void SetCustomSortMode(panel_sort Mode, sort_order Order, bool InvertByDefault) override;
	void ChangeSortOrder(bool Reverse) override;
	void ChangeDirectoriesFirst(bool Mode) override;
	void OnSortingChange() override;
	void InitCurDir(string_view CurDir) override;
	bool SetCurDir(string_view NewDir, bool ClosePanel, bool IsUpdated = true, bool Silent = false) override;
	panel_sort GetPrevSortMode() const override;
	bool GetPrevSortOrder() const override;
	int GetPrevViewMode() const override;
	bool GetPrevDirectoriesFirst() const override;
	bool GetFileName(string &strName, int Pos, os::fs::attributes& FileAttr) const override;
	const unordered_string_set* GetFilteredExtensions() const override;
	int GetCurrentPos() const override;
	bool FindPartName(string_view Name, int Next, int Direct = 1) override;
	bool GetPlainString(string& Dest, int ListPos) const override;
	bool GoToFile(long idxItem) override;
	bool GoToFile(string_view Name, bool OnlyPartName = false) override;
	long FindFile(string_view Name, bool OnlyPartName = false) override;
	bool IsSelected(string_view Name) override;
	bool IsSelected(size_t idxItem) override;
	long FindFirst(string_view Name) override;
	long FindNext(int StartPos, string_view Name) override;
	void UpdateViewPanel() override;
	void CompareDir() override;
	void ClearSelection() override;
	void SaveSelection() override;
	void RestoreSelection() override;
	void EditFilter() override;
	bool FileInFilter(size_t idxItem) override;
	bool FilterIsEnabled() override;
	void ReadDiz(std::span<PluginPanelItem> Items = {}) override;
	void DeleteDiz(string_view Name, string_view ShortName) override;
	void FlushDiz() override;
	string GetDizName() const override;
	string_view GetDescription(string_view Name, string_view ShortName, long long FileSize) const;
	void CopyDiz(string_view Name, string_view ShortName, string_view DestName, string_view DestShortName, DizList *DestDiz) override;
	bool IsDizDisplayed() const override;
	bool IsColumnDisplayed(column_type Type) const override;
	int GetColumnsCount() const override;
	void SetReturnCurrentFile(bool Mode) override;
	void GetOpenPanelInfo(OpenPanelInfo *Info) const override;
	void SetPluginMode(std::unique_ptr<plugin_panel>&& PluginPanel, string_view PluginFile, bool SendOnFocus = false) override;
	size_t GetSelCount() const override;
	bool GetSelName(string *strName, string *strShortName = nullptr, os::fs::find_data *fd = nullptr) override;
	void ClearLastGetSelection() override;
	plugin_panel* GetPluginHandle() const override;
	size_t GetRealSelCount() const override;
	void SetPluginModified() override;
	bool ProcessPluginEvent(int Event, void *Param) override;
	void RefreshTitle() override;
	size_t GetFileCount() const override;
	void UpdateKeyBar() override;
	void GoHome(string_view Drive) override;
	bool GetSelectedFirstMode() const override;
	void on_swap() override;
	void dispose() override;

	const FileListItem* GetItem(size_t Index) const;
	const FileListItem* GetLastSelectedItem() const;

	plugin_panel* OpenFilePlugin(const string& FileName, int PushPrev, OPENFILEPLUGINTYPE Type, bool* StopProcessing = nullptr);
	void PushFilePlugin(const string& FileName, std::unique_ptr<plugin_panel>&& hNewPlugin);
	void SetAndUpdateFilePlugin(const string& FileName, std::unique_ptr<plugin_panel>&& hNewPlugin);

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
	string GetPluginPrefix() const;

	size_t FileListToPluginItem2(const FileListItem& fi, FarGetPluginPanelItem* gpi) const;
	static bool FileNameToPluginItem(string_view Name, class PluginPanelItemHolder& pi);
	void FileListToPluginItem(const FileListItem& fi, PluginPanelItemHolder& Holder) const;
	static bool IsModeFullScreen(int Mode);
	void background_update();

	struct PrevDataItem;

protected:
	void ClearAllItem() override;

private:
	friend class FileListItem;

	class list_data;

	void SetSelectedFirstMode(bool Mode) override;
	void DisplayObject() override;
	bool GetCurName(string &strName, string &strShortName) const override;
	bool GetCurBaseName(string &strName, string &strShortName) const override;

	bool HardlinksSupported() const;
	bool StreamsSupported() const;
	const string& GetComputerName() const;
	std::unique_ptr<content_data> GetContentData(string_view Item) const;
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
	bool ConvertName(string_view SrcName, string &strDest, size_t MaxLength, unsigned long long RightAlign, int ShowStatus, os::fs::attributes FileAttr) const;
	void Select(FileListItem& SelItem, bool Selection);
	long SelectFiles(int Mode, string_view Mask = {});
	void ProcessEnter(bool EnableExec, bool SeparateWindow, bool EnableAssoc, bool RunAs, OPENFILEPLUGINTYPE Type);
	// ChangeDir возвращает false, eсли не смогла выставить заданный путь
	bool ChangeDir(string_view NewDir, bool IsParent, bool ResolvePath, bool IsUpdated, const UserDataItem* DataItem, OPENFILEPLUGINTYPE OfpType, bool Silent);
	bool ChangeDir(string_view NewDir, bool IsParent);
	void CountDirSize(bool IsRealNames);
	void ReadFileNames(bool KeepSelection, bool UpdateEvenIfPanelInvisible);
	void UpdatePlugin(bool KeepSelection, bool UpdateEvenIfPanelInvisible);
	void MoveSelection(list_data& From, list_data& To);
	void PushPlugin(std::unique_ptr<plugin_panel>&& hPlugin, string_view HostFile);
	bool PopPlugin(int EnableRestoreViewMode);
	void PopPrevData(string_view DefaultName, bool Closed, bool UsePrev, bool Position, bool SetDirectorySuccess);
	void CopyFiles(bool Move);
	void CopyNames(bool FillPathName, bool UNC);
	void SelectSortMode();
	bool ApplyCommand();
	void DescribeFiles();

	plugin_item_list CreatePluginItemList();
	std::unique_ptr<plugin_panel> OpenPluginForFile(const string& FileName, os::fs::attributes FileAttr, OPENFILEPLUGINTYPE Type, bool* StopProcessing = nullptr);
	void PreparePanelView();
	void PrepareColumnWidths(std::vector<column>& Columns, bool FullScreen) const;
	void PrepareStripes(const std::vector<column>& Columns);
	void PrepareViewSettings(int ViewMode);
	void PluginDelete();
	void PutDizToPlugin(FileList *DestPanel, std::span<PluginPanelItem> ItemList, bool Delete, bool Move, DizList *SrcDiz) const;
	void PluginGetFiles(const string& DestPath, bool Move);
	void PluginToPluginFiles(bool Move);
	void PluginHostGetFiles();
	void PluginPutFilesToNew();
	int PluginPutFilesToAnother(bool Move, panel_ptr AnotherPanel);
	void PluginClearSelection(std::span<PluginPanelItem> ItemList);
	void ProcessCopyKeys(unsigned Key);
	void ReadSortGroups(bool UpdateFilterCurrentTime = true);
	int ProcessOneHostFile(const FileListItem* Item);
	void HighlightBorder(int Level, int ListPos) const;
	void InitFSWatcher(bool CheckTree);
	bool IsColumnDisplayed(function_ref<bool(const column&)> Compare) const;
	void UpdateHeight();
	enum direction
	{
		up, down
	};
	void MoveSelection(direction Direction);

	static void FillParentPoint(FileListItem& Item);

	std::unique_ptr<multifilter> m_Filter;
	DizList Diz;
	bool DizRead{};
	/* $ 09.11.2001 IS
		 Открывающий и закрывающий символ, которые используются для показа
		 имени, которое не помещается в панели. По умолчанию - фигурные скобки.
	*/
	wchar_t openBracket[2]{L'{'}, closeBracket[2]{L'}'};

	string strOriginalCurDir;
	string strPluginDizName;

	class list_data
	{
	public:
		NONCOPYABLE(list_data);
		MOVE_CONSTRUCTIBLE(list_data);

		using value_type = FileListItem;

		list_data() = default;
		~list_data() { clear(); }

		list_data& operator=(list_data&& rhs) noexcept;

		void initialise(plugin_panel* ph) { clear(); m_Plugin = ph; }

		void clear();
		decltype(auto) size() const { return Items.size(); }
		decltype(auto) empty() const { return Items.empty(); }
		decltype(auto) begin() const { return Items.begin(); }
		decltype(auto) end() const { return Items.end(); }
		decltype(auto) begin() { return Items.begin(); }
		decltype(auto) end() { return Items.end(); }
		decltype(auto) cbegin() const { return Items.cbegin(); }
		decltype(auto) cend() const { return Items.cend(); }
		decltype(auto) front() const { return Items.front(); }
		decltype(auto) back() const { return Items.back(); }
		decltype(auto) front() { return Items.front(); }
		decltype(auto) back() { return Items.back(); }
		decltype(auto) operator[](size_t Index) const { return Items[Index]; }
		decltype(auto) operator[](size_t Index) { return Items[Index]; }
		decltype(auto) data() const { return Items.data(); }
		decltype(auto) resize(size_t Size) { return Items.resize(Size); }
		decltype(auto) reserve(size_t Size) { return Items.reserve(Size); }
		decltype(auto) emplace_back(auto&&... Args) { return Items.emplace_back(FWD(Args)...); }
		decltype(auto) push_back(auto&& Value) { return Items.push_back(FWD(Value)); }
	private:
		std::vector<FileListItem> Items;
		plugin_panel* m_Plugin{};
	};

	// All these shenanigans are only to prevent the background re-read and invalidation of list_data while we're still using it.

	// Consider the following scenario:
	// - The user starts a lengthy operation, e.g. a directory scan
	// - We iterate through list_data and do things with each item
	// - Suddenly a wild Elevation Dialog appears, because one of those things needed it
	// - While the user reads the dialog and considers what to do next, some other process changes something in the same directory
	// - Suddenly a wild filesystem notification appears and tells the FileList to re-read the directory as soon as possible
	// - The user closes the dialog, the window manager tells all the windows to redraw, the FileList re-reads the directory
	// - The control finally returns to the list_data iteration
	// - Suddenly a wild read access violation appears, because the referenced data is gone

	// So now we 'lock' the data before referencing it, and FileList postpones updates if it's locked.
	// TODO: Remove and re-expose m_ListData if we ever come up with a better background update logic or make elevation safer.
	class list_data_lock
	{
	public:
		auto& get() { return m_ListData; }
		auto& get() const { return m_ListData; }

		void lock() const { ++m_Lock; }
		void unlock() const { --m_Lock; }

		bool locked() const { return m_Lock != 0; }

	//private:
		list_data m_ListData;
		mutable size_t m_Lock{};
	}
	m_ListData_DoNotTouchDirectly;

	template<typename T>
	class data_lock_ptr
	{
	public:
		NONCOPYABLE(data_lock_ptr);

		explicit data_lock_ptr(T& DataLock):
			m_DataLock(&DataLock)
		{
			m_DataLock->lock();
		}

		~data_lock_ptr()
		{
			m_DataLock->unlock();
		}

		auto& operator*() const &
		{
			return m_DataLock->get();
		}

		auto const& operator*() const && = delete;

	private:
		T* m_DataLock;
	};

	data_lock_ptr<list_data_lock> lock_data() { return data_lock_ptr(m_ListData_DoNotTouchDirectly); }
	data_lock_ptr<list_data_lock const> lock_data() const { return data_lock_ptr(m_ListData_DoNotTouchDirectly); }
	bool is_data_locked() const { return m_ListData_DoNotTouchDirectly.locked(); }

	// These should be safe(ish) to call any time
	bool is_data_empty() const { return m_ListData_DoNotTouchDirectly.get().empty(); }
	size_t data_size() const { return m_ListData_DoNotTouchDirectly.get().size(); }

	std::list<PrevDataItem> PrevDataList;
	struct PluginsListItem;
	std::list<std::shared_ptr<PluginsListItem>> PluginsList;
	std::shared_ptr<PluginsListItem> m_ExpiringPluginPanel{};
	std::optional<FileSystemWatcher> FSWatcher;
	bool m_UpdatePending{};
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
	int m_Height{};
	int m_Stripes{}; // Stripe is a logical column representing one list item == group of columns repeated across the list
	int m_ColumnsInStripe{}; // number of columns (item attributes) in a stripe
	int LeftPos{};
	int ShiftSelection{ -1 };
	bool MouseSelection{};
	bool SelectedFirst{};
	bool empty{}; // указывает на полностью пустую колонку
	bool AccessTimeUpdateRequired{};
	bool UpdateRequired{};
	bool m_KeepSelection{};
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
	int m_InsideGetFindData{};
	unordered_string_set m_FilteredExtensions;
	std::weak_ptr<PluginsListItem> GetPluginItem() const;

	class background_updater;
	std::unique_ptr<background_updater> m_BackgroundUpdater;
};

#endif // FILELIST_HPP_825FE8AE_1E34_4DFD_B167_2D6A121B1777
