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

class FileFilter;

struct FileListItem:public Panel::panelitem
{
	char Selected;
	char PrevSelected;
	char ShowFolderSize;
	char ShortNamePresent;
	HighlightDataColor Colors;

	DWORD NumberOfLinks;
	DWORD NumberOfStreams;
	UINT64 UserFlags;
	void* UserData;
	FARPANELITEMFREECALLBACK Callback;

	size_t Position;
	int SortGroup;
	wchar_t *DizText;
	char DeleteDiz;
	string strOwner;
	wchar_t **CustomColumnData;
	size_t CustomColumnNumber;
	DWORD CRC32;

	//BUGBUG!!
	DWORD FileAttr;
	FILETIME CreationTime;
	FILETIME AccessTime;
	FILETIME WriteTime;
	FILETIME ChangeTime;

	unsigned __int64 FileSize;
	unsigned __int64 AllocationSize;
	unsigned __int64 StreamsSize;

	string strShortName;

	DWORD ReparseTag;

	string strCustomData;

	~FileListItem();

private:
	// no construction outside of FileList
	FileListItem():
		Selected(0),
		PrevSelected(0),
		ShowFolderSize(0),
		ShortNamePresent(0),
		Colors(),
		NumberOfLinks(0),
		NumberOfStreams(0),
		UserFlags(0),
		UserData(nullptr),
		Callback(nullptr),
		Position(0),
		SortGroup(0),
		DizText(nullptr),
		DeleteDiz(0),
		CustomColumnData(nullptr),
		CustomColumnNumber(0),
		CRC32(0),
		FileAttr(0),
		CreationTime(),
		AccessTime(),
		WriteTime(),
		ChangeTime(),
		FileSize(0),
		AllocationSize(0),
		StreamsSize(0),
		ReparseTag(0)
	{
	}
	friend class FileList;
};

struct PluginsListItem
{
	HANDLE hPlugin;
	string strHostFile;
	string strPrevOriginalCurDir;
	int Modified;
	int PrevViewMode;
	int PrevSortMode;
	int PrevSortOrder;
	bool PrevNumericSort;
	bool PrevCaseSensitiveSort;
	bool PrevDirectoriesFirst;
	PanelViewSettings PrevViewSettings;
};

ENUM(OPENFILEPLUGINTYPE);

class FileList:public Panel
{
public:
	FileList();

	virtual int ProcessKey(int Key) override;
	virtual int ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent) override;
	virtual __int64 VMProcess(int OpCode,void *vParam=nullptr,__int64 iParam=0) override;
	virtual void MoveToMouse(const MOUSE_EVENT_RECORD *MouseEvent) override;
	virtual void SetFocus() override;
	virtual void Update(int Mode) override;
	virtual int UpdateIfChanged(panel_update_mode UpdateMode) override;
	virtual void UpdateIfRequired() override;
	virtual int SendKeyToPlugin(DWORD Key,bool Pred=false) override;
	virtual void StartFSWatcher(bool got_focus=false, bool check_time=true) override;
	virtual void StopFSWatcher() override;
	virtual void SortFileList(int KeepPosition) override;
	virtual void SetViewMode(int ViewMode) override;
	virtual void SetSortMode(int SortMode) override;
	virtual void SetCustomSortMode(int SortMode, bool InvertByDefault, const wchar_t* Indicators) override;
	virtual void ChangeSortOrder(int NewOrder) override;
	virtual void ChangeNumericSort(bool Mode) override;
	virtual void ChangeCaseSensitiveSort(bool Mode) override;
	virtual void ChangeDirectoriesFirst(bool Mode) override;
	virtual bool SetCurDir(const string& NewDir,bool ClosePanel,bool IsUpdated=true) override;
	virtual int GetPrevSortMode() override;
	virtual int GetPrevSortOrder() override;
	virtual int GetPrevViewMode() override;
	virtual bool GetPrevNumericSort() override;
	virtual bool GetPrevCaseSensitiveSort() override;
	virtual bool GetPrevDirectoriesFirst() override;
	virtual int GetFileName(string &strName,int Pos,DWORD &FileAttr) override;
	virtual int GetCurrentPos() const override;
	virtual int FindPartName(const string& Name,int Next,int Direct=1,int ExcludeSets=0) override;
	virtual bool GetPlainString(string& Dest,int ListPos) override;
	virtual int GoToFile(long idxItem) override;
	virtual int GoToFile(const string& Name,BOOL OnlyPartName=FALSE) override;
	virtual long FindFile(const string& Name,BOOL OnlyPartName=FALSE) override;
	virtual int IsSelected(const string& Name) override;
	virtual int IsSelected(size_t idxItem) override;
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
	virtual void ReadDiz(PluginPanelItem *ItemList=nullptr,int ItemLength=0, DWORD dwFlags=0) override;
	virtual void DeleteDiz(const string& Name, const string& ShortName) override;
	virtual void FlushDiz() override;
	virtual void GetDizName(string &strDizName) override;
	virtual void CopyDiz(const string& Name, const string& ShortName, const string& DestName, const string& DestShortName, DizList *DestDiz) override;
	virtual bool IsDizDisplayed() override;
	virtual bool IsColumnDisplayed(int Type) override;
	virtual int GetColumnsCount() override { return Columns;}
	virtual void SetReturnCurrentFile(int Mode) override;
	virtual void GetOpenPanelInfo(OpenPanelInfo *Info) override;
	virtual void SetPluginMode(HANDLE hPlugin,const string& PluginFile,bool SendOnFocus=false) override;
	virtual size_t GetSelCount() override;
	virtual int GetSelName(string *strName,DWORD &FileAttr,string *strShortName=nullptr,FAR_FIND_DATA *fde=nullptr) override;
	virtual void UngetSelName() override;
	virtual void ClearLastGetSelection() override;
	virtual unsigned __int64 GetLastSelectedSize() override;
	virtual const FileListItem* GetLastSelectedItem() const override;
	virtual HANDLE GetPluginHandle() override;
	virtual size_t GetRealSelCount() override;
	virtual void SetPluginModified() override;
	virtual int ProcessPluginEvent(int Event,void *Param) override;
	virtual void SetTitle() override;
	virtual size_t GetFileCount() override {return ListData.size();}
	virtual const FileListItem* GetItem(size_t Index) const override;
	virtual void UpdateKeyBar() override;
	virtual void IfGoHome(wchar_t Drive) override;

	void SetSortMode0(int SortMode);
	HANDLE OpenFilePlugin(const string* FileName,int PushPrev, OPENFILEPLUGINTYPE Type);
	long FindFile(const char *Name,BOOL OnlyPartName=FALSE);
	void ProcessHostFile();
	bool GetPluginInfo(PluginInfo *PInfo);
	void PluginGetPanelInfo(PanelInfo &Info);
	size_t PluginGetPanelItem(int ItemNumber,FarGetPluginPanelItem *Item);
	size_t PluginGetSelectedPanelItem(int ItemNumber,FarGetPluginPanelItem *Item);
	void PluginGetColumnTypesAndWidths(string& strColumnTypes,string& strColumnWidths);
	void PluginBeginSelection();
	void PluginSetSelection(int ItemNumber,bool Selection);
	void PluginClearSelection(int SelectedItemNumber);
	void PluginEndSelection();
	int PluginPanelHelp(HANDLE hPlugin);
	bool CreateFullPathName(const string& Name,const string& ShortName,DWORD FileAttr, string &strDest,int UNC,int ShortNameAsIs=TRUE);
	void ResetLastUpdateTime() {LastUpdateTime = 0;}

	static void SetFilePanelModes();
	static void SavePanelModes();
	static void ReadPanelModes();
	static size_t FileListToPluginItem2(FileListItem *fi,FarGetPluginPanelItem *pi);
	static int FileNameToPluginItem(const string& Name,PluginPanelItem *pi);
	static void FileListToPluginItem(const FileListItem *fi,PluginPanelItem *pi);
	static void PluginToFileListItem(PluginPanelItem *pi,FileListItem *fi);
	static bool IsModeFullScreen(int Mode);
	static string &AddPluginPrefix(FileList *SrcPanel,string &strPrefix);

protected:
	virtual void ClearAllItem() override;

private:
	virtual ~FileList();
	virtual void SetSelectedFirstMode(bool Mode) override;
	virtual bool GetSelectedFirstMode() override {return SelectedFirst;}
	virtual void DisplayObject() override;
	virtual int GetCurName(string &strName, string &strShortName) override;
	virtual int GetCurBaseName(string &strName, string &strShortName) override;

	void DeleteListData(std::vector<std::unique_ptr<FileListItem>>& ListData);
	void Up(int Count);
	void Down(int Count);
	void Scroll(int Count);
	void CorrectPosition();
	void ShowFileList(int Fast);
	void ShowList(int ShowStatus,int StartColumn);
	void SetShowColor(int Position, int ColorType=HIGHLIGHTCOLORTYPE_FILE);
	const FarColor GetShowColor(int Position, int ColorType);
	void ShowSelectedSize();
	void ShowTotalSize(const OpenPanelInfo &Info);
	int ConvertName(const wchar_t *SrcName, string &strDest, int MaxLength, unsigned __int64 RightAlign, int ShowStatus, DWORD dwFileAttr);
	void Select(FileListItem *SelPtr,int Selection);
	long SelectFiles(int Mode,const wchar_t *Mask=nullptr);
	void ProcessEnter(bool EnableExec,bool SeparateWindow, bool EnableAssoc, bool RunAs, OPENFILEPLUGINTYPE Type);
	// ChangeDir возвращает false, eсли не смогла выставить заданный путь
	bool ChangeDir(const string& NewDir,bool ResolvePath=false,bool IsUpdated=true,const FileListItem *CurPtr=nullptr);
	void CountDirSize(UINT64 PluginFlags);
	void ReadFileNames(int KeepSelection, int UpdateEvenIfPanelInvisible, int DrawMessage);
	void UpdatePlugin(int KeepSelection, int UpdateEvenIfPanelInvisible);
	void MoveSelection(std::vector<std::unique_ptr<FileListItem>>& From, std::vector<std::unique_ptr<FileListItem>>& To);
	void PushPlugin(HANDLE hPlugin,const string& HostFile);
	int PopPlugin(int EnableRestoreViewMode);
	void PopPrevData(const string& DefaultName,bool Closed,bool UsePrev,bool Position,bool SetDirectorySuccess);
	void CopyFiles(bool bMoved=false);
	void CopyNames(bool FillPathName, bool UNC);
	void SelectSortMode();
	bool ApplyCommand();
	void DescribeFiles();
	std::vector<PluginPanelItem> CreatePluginItemList(bool AddTwoDot=TRUE);
	void DeletePluginItemList(std::vector<PluginPanelItem> &ItemList);
	HANDLE OpenPluginForFile(const string* FileName,DWORD FileAttr, OPENFILEPLUGINTYPE Type);
	int PreparePanelView(PanelViewSettings *PanelView);
	int PrepareColumnWidths(std::vector<column>& Columns, bool FullScreen, bool StatusLine);
	void PrepareViewSettings(int ViewMode, const OpenPanelInfo *PlugInfo);
	void PluginDelete();
	void PutDizToPlugin(FileList *DestPanel,std::vector<PluginPanelItem>& ItemList, int Delete, int Move, DizList *SrcDiz, DizList *DestDiz);
	void PluginGetFiles(const wchar_t **DestPath,int Move);
	void PluginToPluginFiles(int Move);
	void PluginHostGetFiles();
	void PluginPutFilesToNew();
	int PluginPutFilesToAnother(int Move,Panel *AnotherPanel);
	void ProcessPluginCommand();
	void PluginClearSelection(const std::vector<PluginPanelItem>& ItemList);
	void ProcessCopyKeys(int Key);
	void ReadSortGroups(bool UpdateFilterCurrentTime=true);
	void AddParentPoint(FileListItem *CurPtr, size_t CurFilePos, const FILETIME* Times=nullptr, const string& Owner = string());
	int ProcessOneHostFile(const std::vector<std::unique_ptr<FileListItem>>::const_iterator Idx);
	void HighlightBorder(int Level, int ListPos);
	void InitFSWatcher(bool CheckTree);

	struct PrevDataItem
	{
		PrevDataItem(const string& rhsPrevName, std::vector<std::unique_ptr<FileListItem>>&& rhsPrevListData, int rhsPrevTopFile)
		{
			strPrevName = rhsPrevName;
			std::swap(PrevListData, rhsPrevListData);
			PrevTopFile = rhsPrevTopFile;
		}

		PrevDataItem(PrevDataItem&& rhs)
		{
			std::swap(strPrevName, rhs.strPrevName);
			std::swap(PrevListData, rhs.PrevListData);
			PrevTopFile = rhs.PrevTopFile;
		}

		string strPrevName;
		std::vector<std::unique_ptr<FileListItem>> PrevListData;
		int PrevTopFile;
	};

	std::unique_ptr<FileFilter> Filter;
	DizList Diz;
	int DizRead;
	/* $ 09.11.2001 IS
	     Открывающий и закрывающий символ, которые используются для показа
	     имени, которое не помещается в панели. По умолчанию - фигурные скобки.
	*/
	wchar_t openBracket[2], closeBracket[2];

	string strOriginalCurDir;
	string strPluginDizName;
	std::vector<std::unique_ptr<FileListItem>> ListData;
	HANDLE hPlugin;
	std::list<PrevDataItem> PrevDataList;
	std::list<PluginsListItem> PluginsList;
	FileSystemWatcher FSWatcher;
	long UpperFolderTopFile,LastCurFile;
	long ReturnCurrentFile;
	size_t SelFileCount;
	long GetSelPosition,LastSelPosition;
	size_t TotalFileCount;
	unsigned __int64 SelFileSize;
	unsigned __int64 TotalFileSize;
	unsigned __int64 FreeDiskSize;
	clock_t LastUpdateTime;
	int Height,Columns;

	int ColumnsInGlobal;

	int LeftPos;
	int ShiftSelection;
	int MouseSelection;
	bool SelectedFirst;
	int empty; // указывает на полностью пустую колонку
	int AccessTimeUpdateRequired;

	int UpdateRequired,UpdateRequiredMode,UpdateDisabled;
	int SortGroupsRead;
	int InternalProcessKey;

	long CacheSelIndex,CacheSelPos;
	long CacheSelClearIndex,CacheSelClearPos;

	wchar_t CustomSortIndicator[2];
};
