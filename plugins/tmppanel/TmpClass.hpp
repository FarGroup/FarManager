#ifndef TMPCLASS_HPP_691D4071_CABF_4159_B82A_BE97B4C8DB04
#define TMPCLASS_HPP_691D4071_CABF_4159_B82A_BE97B4C8DB04
#pragma once

/*
TMPCLASS.HPP

Temporary panel plugin class header file

*/

#include "TmpPanel.hpp"

#include <span>

class TmpPanel
{
public:
	NONCOPYABLE(TmpPanel);

	explicit TmpPanel(string_view HostFile = {});

	int GetFindData(PluginPanelItem*& pPanelItem, size_t& pItemsNumber, OPERATION_MODES OpMode);
	void GetOpenPanelInfo(OpenPanelInfo& Info);
	int SetDirectory(const wchar_t* Dir, OPERATION_MODES OpMode);

	bool PutFiles(std::span<const PluginPanelItem> Files, const wchar_t* SrcPath, OPERATION_MODES OpMode);
	HANDLE BeginPutFiles();
	void CommitPutFiles(HANDLE RestoreScreen, bool Success);
	bool PutDirectoryContents(const wchar_t* Path);
	bool PutOneFile(const string& SrcPath, const PluginPanelItem& PanelItem);
	bool PutOneFile(const string& FilePath);

	int SetFindList(std::span<const PluginPanelItem> Files);
	int ProcessEvent(intptr_t Event, void* Param);
	bool ProcessKey(const INPUT_RECORD* Rec);
	void IfOptCommonPanel();
	void clear();

private:
	void RemoveDups();
	void RemoveEmptyItems();
	void UpdateItems(bool ShowOwners, bool ShowLinks);
	void ProcessRemoveKey();
	void ProcessSaveListKey();
	void ProcessPanelSwitchMenu();
	void SwitchToPanel(size_t NewPanelIndex);
	void FindSearchResultsPanel();
	void SaveListFile(const string& Path);
	string IsCurrentFileCorrect();

	PluginPanel m_LocalPanel, * m_Panel{ &m_LocalPanel };
	string m_HostFile, m_Title;
	bool
		m_LastOwnersRead{},
		m_LastLinksRead{};
	bool m_UpdateNotNeeded{};
};

#endif // TMPCLASS_HPP_691D4071_CABF_4159_B82A_BE97B4C8DB04
