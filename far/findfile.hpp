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

// Internal:
#include "datetime.hpp"
#include "plugin.hpp"

// Platform:
#include "platform.concurrency.hpp"
#include "platform.fs.hpp"

// Common:

// External:

//----------------------------------------------------------------------------

enum FINDAREA
{
	FINDAREA_ALL,
	FINDAREA_ALL_BUTNETWORK,
	FINDAREA_INPATH,
	FINDAREA_ROOT,
	FINDAREA_FROM_CURRENT,
	FINDAREA_CURRENT_ONLY,
	FINDAREA_SELECTED,

	FINDAREA_COUNT
};

class Dialog;
class plugin_panel;
struct THREADPARAM;
class InterThreadData;
class filemasks;
class FileFilter;
struct FindListItem;
class delayed_deleter;

class FindFiles: noncopyable
{
public:
	// Список архивов. Если файл найден в архиве, то FindList->ArcIndex указывает сюда.
	struct ArcListItem
	{
		string strArcName;
		plugin_panel* hPlugin; // Plugin handle
		unsigned long long Flags; // OpenPanelInfo.Flags
		string strRootPath; // Root path in plugin after opening.
	};
public:
	FindFiles();
	~FindFiles();

	const std::unique_ptr<filemasks>& GetFileMask() const { return FileMaskForFindFile; }
	const std::unique_ptr<FileFilter>& GetFilter() const { return Filter; }
	static bool IsWordDiv(wchar_t symbol);
	// BUGBUG
	void AddMenuRecord(Dialog* Dlg, string_view FullName, const os::fs::find_data& FindData, void* Data, FARPANELITEMFREECALLBACK FreeData, ArcListItem* Arc);

	enum type2
	{
		data,
		push,
		pop
	};

	struct AddMenuData
	{
		type2 m_Type{ data };
		FindFiles* m_Owner{};
		Dialog* m_Dlg{};
		string m_FullName;
		os::fs::find_data m_FindData;
		void* m_Data{};
		FARPANELITEMFREECALLBACK m_FreeData{};
		ArcListItem* m_Arc{};

		AddMenuData() = default;
		explicit AddMenuData(type2 Type): m_Type(Type) {}
		AddMenuData(string_view FullName, const os::fs::find_data& FindData, void* Data, FARPANELITEMFREECALLBACK FreeData, ArcListItem* Arc):
			m_FullName(FullName),
			m_FindData(FindData),
			m_Data(Data),
			m_FreeData(FreeData),
			m_Arc(Arc)
		{
		}
	};

	std::unique_ptr<InterThreadData> itd;
	os::synced_queue<AddMenuData> m_Messages;

	// BUGBUG
	void Lock() { PluginCS.lock(); }
	void Unlock() { PluginCS.unlock(); }
	[[nodiscard]]
	auto ScopedLock() { return make_raii_wrapper(this, &FindFiles::Lock, &FindFiles::Unlock); }

private:
	string &PrepareDriveNameStr(string &strSearchFromRoot) const;
	void AdvancedDialog() const;
	intptr_t MainDlgProc(Dialog* Dlg, intptr_t Msg, intptr_t Param1, void* Param2);
	intptr_t FindDlgProc(Dialog* Dlg, intptr_t Msg, intptr_t Param1, void* Param2);
	void OpenFile(const string& strSearchFileName, int OpenKey, const FindListItem* FindItem, Dialog* Dlg) const;
	bool FindFilesProcess();
	void ProcessMessage(const AddMenuData& Data);
	void SetPluginDirectory(string_view DirName, const plugin_panel* hPlugin, bool UpdatePanel, const UserDataItem *UserData);
	bool GetPluginFile(struct ArcListItem const* ArcItem, const os::fs::find_data& FindData, const string& DestPath, string &strResultName, const UserDataItem* UserData);

	static intptr_t AdvancedDlgProc(Dialog* Dlg, intptr_t Msg, intptr_t Param1, void* Param2);

	// BUGBUG
	bool AnySetFindList{};
	bool CmpCase{};
	bool WholeWords{};
	bool SearchInArchives{};
	bool SearchHex{};
	bool NotContaining{};
	bool UseFilter{};
	bool FindFoldersChanged{};
	bool SearchFromChanged{};
	bool FindPositionChanged{};
	bool Finalized{};
	bool PluginMode{};
	FINDAREA SearchMode{ FINDAREA_ALL };
	int favoriteCodePages{};
	uintptr_t CodePage{ CP_DEFAULT };
	struct FindListItem* FindExitItem{};
	string strFindMask;
	string strFindStr;
	std::unique_ptr<filemasks> FileMaskForFindFile;
	std::unique_ptr<FileFilter> Filter;

	std::unique_ptr<delayed_deleter> m_DelayedDeleter;

	int m_FileCount{};
	int m_DirCount{};
	int m_LastFoundNumber{};

	os::critical_section PluginCS;

	time_check m_TimeCheck;
	// BUGBUG
	class background_searcher* m_Searcher{};
	std::exception_ptr m_ExceptionPtr;
	std::stack<string> m_LastDir;
	string m_LastDirName;
	Dialog* m_ResultsDialogPtr{};
	bool m_EmptyArc{};
	os::event m_MessageEvent;
};

#endif // FINDFILE_HPP_8601893C_E4B7_4EC6_A79F_9C6E491FF5ED
