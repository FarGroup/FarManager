#ifndef TREELIST_HPP_B2FDA185_E801_437B_A9D7_F4D3CE6D40A4
#define TREELIST_HPP_B2FDA185_E801_437B_A9D7_F4D3CE6D40A4
#pragma once

/*
treelist.hpp

Tree panel
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

// Platform:
#include "platform.fwd.hpp"

// Common:

// External:

//----------------------------------------------------------------------------

enum
{
	MODALTREE_ACTIVE  =1,
	MODALTREE_PASSIVE =2,
	MODALTREE_FREE    =3
};


class TreeList final: public Panel
{
	struct private_tag { explicit private_tag() = default; };

public:
	struct TreeItem
	{
		NONCOPYABLE(TreeItem);
		MOVABLE(TreeItem);

		string strName;
		std::vector<int> Last;
		size_t Depth;

		TreeItem():
			Last(MAX_PATH/2),
			Depth(0)
		{
		}

		explicit TreeItem(string Name):
			strName(std::move(Name)),
			Last(MAX_PATH/2),
			Depth(0)
		{
		}

		explicit(false) operator string_view() const { return strName; }
	};

	static tree_panel_ptr create(window_ptr Owner, int ModalMode = 0);
	TreeList(private_tag, window_ptr Owner, int ModalMode);
	~TreeList() override;

	bool ProcessKey(const Manager::Key& Key) override;
	bool ProcessMouse(const MOUSE_EVENT_RECORD *MouseEvent) override;
	bool GoToFile(string_view Name, bool OnlyPartName = false) override;
	bool FindPartName(string_view Name, int Next, int Direct = 1) override;
	void Update(int Mode) override;
	const string& GetCurDir() const override;

	void SetRootDir(string_view NewRootDir);
	void ProcessEnter();
	int GetExitCode() const { return m_ExitCode; }
	const TreeItem* GetItem(size_t Index) const;

	static void AddTreeName(string_view Name);
	static void DelTreeName(string_view Name);
	static void RenTreeName(string_view SrcName, string_view DestName);
	static void ReadSubTree(string_view Path);
	static void ClearCache();
	static void ReadCache(string_view TreeRoot);
	static void FlushCache();

private:
	long long VMProcess(int OpCode, void* vParam = nullptr, long long iParam = 0) override;
	bool SetCurDir(string_view NewDir, bool ClosePanel, bool IsUpdated = true, bool Silent = false) override;
	bool GetCurName(string &strName, string &strShortName) const override;
	void UpdateViewPanel() override;
	void MoveToMouse(const MOUSE_EVENT_RECORD *MouseEvent) override;
	bool GetPlainString(string& Dest, int ListPos) const override;
	bool GoToFile(long idxItem) override;
	long FindFile(string_view Name, bool OnlyPartName = false) override;
	long FindFirst(string_view Name) override;
	long FindNext(int StartPos, string_view Name) override;
	size_t GetFileCount() const override { return m_ListData.size(); }
	bool GetFileName(string &strName, int Pos, os::fs::attributes& FileAttr) const override;
	void RefreshTitle() override;
	string GetTitle() const override;
	void OnFocusChange(bool Get) override;
	void UpdateKeyBar() override;
	int GetCurrentPos() const override;
	bool GetSelName(string *Name, string *ShortName = nullptr, os::fs::find_data *fd = nullptr) override;
	void DisplayObject() override;
	size_t GetSelCount() const override;

	bool ReadTree();
	void DisplayTree(bool Fast);
	void DisplayTreeName(string_view Name, size_t Pos) const;
	void ToBegin();
	void ToEnd();
	void Up(int Count);
	void Down(int Count);
	void Scroll(int Count);
	void CorrectPosition();
	bool FillLastData();
	bool SetDirPosition(string_view NewDir);
	void GetRoot();
	panel_ptr GetRootPanel();
	void SyncDir();
	void SaveTreeFile() const;
	bool ReadTreeFile();
	int GetNextNavPos() const;
	int GetPrevNavPos() const;
	bool SaveState();
	bool RestoreState();

	std::vector<TreeItem> m_ListData;
	std::vector<TreeItem> m_SavedListData;
	const string m_Empty; // bugbug
	string m_Root;
	size_t m_WorkDir{};
	size_t m_SavedWorkDir{};
	long m_GetSelPosition{};
	int m_ExitCode{1}; // актуально только для дерева, вызванного из копира!
	bool m_ReadingTree{};
};

#endif // TREELIST_HPP_B2FDA185_E801_437B_A9D7_F4D3CE6D40A4
