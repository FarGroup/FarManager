/*
TMPCLASS.HPP

Temporary panel plugin class header file

*/

#ifndef __TMPCLASS_HPP__
#define __TMPCLASS_HPP__

#define REMOVE_FLAG 1

class TmpPanel
{
	private:
		PluginPanelItem *TmpPanelItem;
		size_t TmpItemsNumber;
		int LastOwnersRead;
		int LastLinksRead;
		int UpdateNotNeeded;
		wchar_t* HostFile;

	private:
		void RemoveDups();
		void RemoveEmptyItems();
		void UpdateItems(int ShowOwners,int ShowLinks);
		int IsOwnersDisplayed(LPCTSTR ColumnTypes);
		int IsLinksDisplayed(LPCTSTR ColumnTypes);
		void ProcessRemoveKey();
		void ProcessSaveListKey();
		void ProcessPanelSwitchMenu();
		void SwitchToPanel(int NewPanelIndex);
		void FindSearchResultsPanel();
		void SaveListFile(const wchar_t *Path);
		bool IsCurrentFileCorrect(wchar_t **pCurFileName);

	public:
		TmpPanel(const wchar_t *pHostFile=nullptr);
		~TmpPanel();

	public:
		int PanelIndex;
		//int OpenFrom;

	public:
		int GetFindData(PluginPanelItem **pPanelItem,size_t *pItemsNumber,const OPERATION_MODES OpMode);
		void GetOpenPanelInfo(struct OpenPanelInfo *Info);
		int SetDirectory(const wchar_t *Dir,const OPERATION_MODES OpMode);

		int PutFiles(struct PluginPanelItem *PanelItem,size_t ItemsNumber,int Move,const wchar_t *SrcPath,const OPERATION_MODES OpMode);
		HANDLE BeginPutFiles();
		void CommitPutFiles(HANDLE hRestoreScreen, int Success);
		int PutDirectoryContents(const wchar_t* Path);
		int PutOneFile(const wchar_t* SrcPath, PluginPanelItem &PanelItem);
		int PutOneFile(const wchar_t* FilePath);

		int SetFindList(const struct PluginPanelItem *PanelItem,size_t ItemsNumber);
		int ProcessEvent(intptr_t Event,void *Param);
		int ProcessKey(const INPUT_RECORD *Rec);
		void IfOptCommonPanel(void);

		static bool GetFileInfoAndValidate(const wchar_t *FilePath, PluginPanelItem* FindData, int Any);
};

#endif /* __TMPCLASS_HPP__ */
