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

class FindFiles: noncopyable
{
public:
	FindFiles();
	~FindFiles();

private:
	void InitInFileSearch();
	void ReleaseInFileSearch();
	string &PrepareDriveNameStr(string &strSearchFromRoot);
	void AdvancedDialog();
	intptr_t MainDlgProc(Dialog* Dlg, intptr_t Msg, intptr_t Param1, void* Param2);
	template<class T, class Pred>
	int FindStringBMH(const T* searchBuffer, size_t searchBufferCount, size_t findStringCount, Pred p) const;
	int FindStringBMH(const wchar_t* searchBuffer, size_t searchBufferCount) const;
	int FindStringBMH(const unsigned char* searchBuffer, size_t searchBufferCount) const;
	bool LookForString(const string& Name);
	bool IsFileIncluded(PluginPanelItem* FileItem, const string& FullName, DWORD FileAttr, const string &strDisplayName);
	intptr_t FindDlgProc(Dialog* Dlg, intptr_t Msg, intptr_t Param1, void* Param2);
	void AddMenuRecord(Dialog* Dlg,const string& FullName, const api::FAR_FIND_DATA& FindData, void* Data, FARPANELITEMFREECALLBACK FreeData);
	void AddMenuRecord(Dialog* Dlg,const string& FullName, PluginPanelItem& FindData);
	void DoPreparePluginList(Dialog* Dlg, bool Internal);
	void ArchiveSearch(Dialog* Dlg, const string& ArcName);
	void DoScanTree(Dialog* Dlg, const string& strRoot);
	void ScanPluginTree(Dialog* Dlg, PluginHandle* hPlugin, UINT64 Flags, int& RecurseLevel);
	void DoPrepareFileList(Dialog* Dlg);
	unsigned int ThreadRoutine(THREADPARAM* Param);
	bool FindFilesProcess();

	static intptr_t AdvancedDlgProc(Dialog* Dlg, intptr_t Msg, intptr_t Param1, void* Param2);
	static bool IsWordDiv(const wchar_t symbol);
	static void SetPluginDirectory(const string& DirName, PluginHandle* hPlugin, bool UpdatePanel = false, UserDataItem *UserData = nullptr);
	static bool GetPluginFile(struct ArcListItem* ArcItem, const api::FAR_FIND_DATA& FindData, const string& DestPath, string &strResultName, UserDataItem *UserData);

private:
	// BUGBUG
	bool AnySetFindList;
	bool CmpCase;
	bool WholeWords;
	bool SearchInArchives;
	bool SearchHex;
	bool NotContaining;
	bool UseFilter;

	bool m_Autodetection;
	bool InFileSearchInited;
	bool FindFoldersChanged;
	bool SearchFromChanged;
	bool FindPositionChanged;
	bool Finalized;
	bool PluginMode;
	FINDAREA SearchMode;
	int favoriteCodePages;
	std::vector<char> readBufferA;
	std::vector<wchar_t> readBuffer;
	const wchar_t *findString;
	std::string hexFindString;
	uintptr_t CodePage;
	std::vector<size_t> skipCharsTable;
	UINT64 SearchInFirst;
	struct FindListItem* FindExitItem;
	string strFindMask;
	string strFindStr;
	string strLastDirName;
	string strPluginSearchPath;
	string findStringBuffer;
	std::unique_ptr<filemasks> FileMaskForFindFile;
	std::unique_ptr<FileFilter> Filter;
	std::unique_ptr<IndeterminateTaskBar> TB;
	std::unique_ptr<InterThreadData> itd;
	struct CodePageInfo;
	std::list<CodePageInfo> m_CodePages;
	CriticalSection PluginCS;
	Event PauseEvent;
	Event StopEvent;
	time_check m_TimeCheck;
};
