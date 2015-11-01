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

#include "synchro.hpp"
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
struct PluginHandle;
struct THREADPARAM;
class IndeterminateTaskBar;
class InterThreadData;
class filemasks;
class FileFilter;
struct FindListItem;

class FindFiles: noncopyable
{
public:
	FindFiles();
	~FindFiles();

	const std::unique_ptr<filemasks>& GetFileMask() const { return FileMaskForFindFile; }
	const std::unique_ptr<FileFilter>& GetFilter() const { return Filter; }
	static bool IsWordDiv(const wchar_t symbol);
	// BUGBUG
	void AddMenuRecord(Dialog* Dlg, const string& FullName, string& strLastDirName, const os::FAR_FIND_DATA& FindData, void* Data, FARPANELITEMFREECALLBACK FreeData);

private:
	string &PrepareDriveNameStr(string &strSearchFromRoot);
	void AdvancedDialog();
	intptr_t MainDlgProc(Dialog* Dlg, intptr_t Msg, intptr_t Param1, void* Param2);
	intptr_t FindDlgProc(Dialog* Dlg, intptr_t Msg, intptr_t Param1, void* Param2);
	void OpenFile(string strSearchFileName, int key, const FindListItem* FindItem, Dialog* Dlg);
	bool FindFilesProcess();

	static intptr_t AdvancedDlgProc(Dialog* Dlg, intptr_t Msg, intptr_t Param1, void* Param2);
	static void SetPluginDirectory(const string& DirName, PluginHandle* hPlugin, bool UpdatePanel = false, UserDataItem *UserData = nullptr);
	static bool GetPluginFile(struct ArcListItem* ArcItem, const os::FAR_FIND_DATA& FindData, const string& DestPath, string &strResultName, UserDataItem *UserData);

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

public:
	std::unique_ptr<InterThreadData> itd;

	// BUGBUG
	void Lock() { PluginCS.lock(); }
	void Unlock() { PluginCS.unlock(); }
	typedef raii_wrapper<FindFiles*, void (FindFiles::*)(), void (FindFiles::*)()> scoped_lock;
	scoped_lock ScopedLock() { return scoped_lock(this, &FindFiles::Lock, &FindFiles::Unlock); }

private:
	CriticalSection PluginCS;

	time_check m_TimeCheck;
	// BUGBUG
	class background_searcher* m_Searcher;
};

class background_searcher: noncopyable
{
public:
	background_searcher(FindFiles* Owner,
		const string& FindString,
		FINDAREA SearchMode,
		uintptr_t CodePage,
		UINT64 SearchInFirst,
		bool AnySetFindList,
		bool CmpCase,
		bool WholeWords,
		bool SearchInArchives,
		bool SearchHex,
		bool NotContaining,
		bool UseFilter
	);

	unsigned int ThreadRoutine(THREADPARAM* Param);
	void Pause() const { PauseEvent.Reset(); }
	void Resume() const { PauseEvent.Set(); }
	void Stop() const { StopEvent.Set(); }
	bool Stopped() const { return StopEvent.Signaled(); }

private:
	void InitInFileSearch();
	void ReleaseInFileSearch();

	template<class T, class Pred>
	int FindStringBMH(const T* searchBuffer, size_t searchBufferCount, size_t findStringCount, Pred p) const;
	int FindStringBMH(const wchar_t* searchBuffer, size_t searchBufferCount) const;
	int FindStringBMH(const unsigned char* searchBuffer, size_t searchBufferCount) const;
	bool LookForString(const string& Name);
	bool IsFileIncluded(PluginPanelItem* FileItem, const string& FullName, DWORD FileAttr, const string &strDisplayName);
	void DoPrepareFileList(Dialog* Dlg);
	void DoPreparePluginList(Dialog* Dlg, bool Internal);
	void ArchiveSearch(Dialog* Dlg, const string& ArcName);
	void DoScanTree(Dialog* Dlg, const string& strRoot);
	void ScanPluginTree(Dialog* Dlg, PluginHandle* hPlugin, UINT64 Flags, int& RecurseLevel);
	void AddMenuRecord(Dialog* Dlg, const string& FullName, string& strLastDirName, PluginPanelItem& FindData);


	FindFiles* m_Owner;

	std::vector<char> readBufferA;
	std::vector<wchar_t> readBuffer;
	std::vector<char> hexFindString;
	std::vector<size_t> skipCharsTable;
	struct CodePageInfo;
	std::list<CodePageInfo> m_CodePages;
	string findStringBuffer;
	string strPluginSearchPath;
	string strLastDirName;

	const wchar_t *findString;

	bool InFileSearchInited;
	bool m_Autodetection;

	const string strFindStr;
	const FINDAREA SearchMode;
	const uintptr_t CodePage;
	const UINT64 SearchInFirst;
	const bool AnySetFindList;
	const bool CmpCase;
	const bool WholeWords;
	const bool SearchInArchives;
	const bool SearchHex;
	const bool NotContaining;
	const bool UseFilter;

	Event PauseEvent;
	Event StopEvent;
};