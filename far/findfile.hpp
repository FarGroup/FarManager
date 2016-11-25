#ifndef FINDFILE_HPP_8601893C_E4B7_4EC6_A79F_9C6E491FF5ED
#define FINDFILE_HPP_8601893C_E4B7_4EC6_A79F_9C6E491FF5ED
#pragma once

/*
findfile.hpp

Поиск (Alt-F7)
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

#include "notification.hpp"
#include "datetime.hpp"

enum FINDAREA
{
	FINDAREA_ALL,
	FINDAREA_ALL_BUTNETWORK,
	FINDAREA_INPATH,
	FINDAREA_ROOT,
	FINDAREA_FROM_CURRENT,
	FINDAREA_CURRENT_ONLY,
	FINDAREA_SELECTED,
};

class Dialog;
class plugin_panel;
struct THREADPARAM;
class IndeterminateTaskBar;
class InterThreadData;
class filemasks;
class FileFilter;
struct FindListItem;

class FindFiles: noncopyable
{
public:
	// Список архивов. Если файл найден в архиве, то FindList->ArcIndex указывает сюда.
	struct ArcListItem
	{
		string strArcName;
		plugin_panel* hPlugin; // Plugin handle
		UINT64 Flags; // OpenPanelInfo.Flags
		string strRootPath; // Root path in plugin after opening.
	};
public:
	FindFiles();
	~FindFiles();

	const std::unique_ptr<filemasks>& GetFileMask() const { return FileMaskForFindFile; }
	const std::unique_ptr<FileFilter>& GetFilter() const { return Filter; }
	static bool IsWordDiv(const wchar_t symbol);
	// BUGBUG
	void AddMenuRecord(Dialog* Dlg, const string& FullName, const os::FAR_FIND_DATA& FindData, void* Data, FARPANELITEMFREECALLBACK FreeData, ArcListItem* Arc);

	enum type2
	{
		data,
		push,
		pop
	};

	struct AddMenuData
	{
		type2 m_Type;
		FindFiles* m_Owner;
		Dialog* m_Dlg;
		string m_FullName;
		os::FAR_FIND_DATA m_FindData;
		void* m_Data;
		FARPANELITEMFREECALLBACK m_FreeData;
		ArcListItem* m_Arc;

		AddMenuData() = default;
		AddMenuData(type2 Type): m_Type(Type) {}
		AddMenuData(const string& FullName, const os::FAR_FIND_DATA& FindData, void* Data, FARPANELITEMFREECALLBACK FreeData, ArcListItem* Arc):
			m_Type(data),
			m_FullName(FullName),
			m_FindData(FindData),
			m_Data(Data),
			m_FreeData(FreeData),
			m_Arc(Arc)
		{
		}
	};

private:
	string &PrepareDriveNameStr(string &strSearchFromRoot) const;
	void AdvancedDialog();
	intptr_t MainDlgProc(Dialog* Dlg, intptr_t Msg, intptr_t Param1, void* Param2);
	intptr_t FindDlgProc(Dialog* Dlg, intptr_t Msg, intptr_t Param1, void* Param2);
	void OpenFile(string strSearchFileName, int key, const FindListItem* FindItem, Dialog* Dlg) const;
	bool FindFilesProcess();
	void ProcessMessage(const AddMenuData& Data);
	void SetPluginDirectory(const string& DirName, plugin_panel* hPlugin, bool UpdatePanel = false, UserDataItem *UserData = nullptr);
	bool GetPluginFile(struct ArcListItem* ArcItem, const os::FAR_FIND_DATA& FindData, const string& DestPath, string &strResultName, UserDataItem *UserData);

	static intptr_t AdvancedDlgProc(Dialog* Dlg, intptr_t Msg, intptr_t Param1, void* Param2);

	// BUGBUG
	bool AnySetFindList;
	bool CmpCase;
	bool WholeWords;
	bool SearchInArchives;
	bool SearchHex;
	bool NotContaining;
	bool UseFilter;

	bool FindFoldersChanged;
	bool SearchFromChanged;
	bool FindPositionChanged;
	bool Finalized;
	bool PluginMode;
	FINDAREA SearchMode;
	int favoriteCodePages;
	uintptr_t CodePage;
	UINT64 SearchInFirst;
	struct FindListItem* FindExitItem;
	string strFindMask;
	string strFindStr;
	std::unique_ptr<filemasks> FileMaskForFindFile;
	std::unique_ptr<FileFilter> Filter;
	std::unique_ptr<IndeterminateTaskBar> TB;

	class delayed_deleter;
	std::list<delayed_deleter> m_DelayedDeleters;

	int m_FileCount{};
	int m_DirCount{};
	int m_LastFoundNumber{};

public:
	std::unique_ptr<InterThreadData> itd;

	SyncedQueue<AddMenuData> m_Messages;

	// BUGBUG
	void Lock() { PluginCS.lock(); }
	void Unlock() { PluginCS.unlock(); }
	auto ScopedLock() { return make_raii_wrapper(this, &FindFiles::Lock, &FindFiles::Unlock); }

private:
	CriticalSection PluginCS;

	time_check m_TimeCheck;
	// BUGBUG
	class background_searcher* m_Searcher;
	std::exception_ptr m_ExceptionPtr;
	std::stack<string> m_LastDir;
	string m_LastDirName;
	Dialog* m_ResultsDialogPtr;
	bool m_EmptyArc;
	Event m_MessageEvent;
};

class background_searcher: noncopyable
{
public:
	background_searcher(FindFiles* Owner,
		const string& FindString,
		FINDAREA SearchMode,
		uintptr_t CodePage,
		UINT64 SearchInFirst,
		bool CmpCase,
		bool WholeWords,
		bool SearchInArchives,
		bool SearchHex,
		bool NotContaining,
		bool UseFilter,
		bool PluginMode
	);

	void Search();
	void Pause() const { PauseEvent.Reset(); }
	void Resume() const { PauseEvent.Set(); }
	void Stop() const;
	bool Stopped() const { return StopEvent.Signaled(); }


	auto ExceptionPtr() const { return m_ExceptionPtr; }

private:
	void InitInFileSearch();
	void ReleaseInFileSearch();

	int FindStringBMH(const wchar_t* searchBuffer, size_t searchBufferCount) const;
	int FindStringBMH(const unsigned char* searchBuffer, size_t searchBufferCount) const;
	bool LookForString(const string& Name);
	bool IsFileIncluded(PluginPanelItem* FileItem, const string& FullName, DWORD FileAttr, const string &strDisplayName);
	void DoPrepareFileList();
	void DoPreparePluginList(bool Internal);
	void ArchiveSearch(const string& ArcName);
	void DoScanTree(const string& strRoot);
	void ScanPluginTree(plugin_panel* hPlugin, UINT64 Flags, int& RecurseLevel);
	void AddMenuRecord(const string& FullName, PluginPanelItem& FindData) const;

	FindFiles* const m_Owner;
	const string m_EventName;

	std::vector<char> readBufferA;
	std::vector<wchar_t> readBuffer;
	std::vector<char> hexFindString;
	std::vector<size_t> skipCharsTable;
	struct CodePageInfo;
	std::list<CodePageInfo> m_CodePages;
	string findStringBuffer;
	string strPluginSearchPath;

	const wchar_t *findString;

	bool InFileSearchInited;
	bool m_Autodetection;

	const string strFindStr;
	const FINDAREA SearchMode;
	const uintptr_t CodePage;
	const UINT64 SearchInFirst;
	const bool CmpCase;
	const bool WholeWords;
	const bool SearchInArchives;
	const bool SearchHex;
	const bool NotContaining;
	const bool UseFilter;
	const bool m_PluginMode;

	Event PauseEvent;
	Event StopEvent;

	std::exception_ptr m_ExceptionPtr;
};
#endif // FINDFILE_HPP_8601893C_E4B7_4EC6_A79F_9C6E491FF5ED
