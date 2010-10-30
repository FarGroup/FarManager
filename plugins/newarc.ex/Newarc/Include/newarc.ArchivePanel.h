#pragma once
#include "newarc.h"

class ArchivePanel {

private:

	ArchiveModuleManager* m_pManager;

	Archive* m_pArchive;
	Array<ArchiveFormat*> m_pFormats;

	bool m_bFirstTime;

	string m_strFileName;
	string m_strPathInArchive;
	string m_strPanelTitle;

	bool m_bPasswordSet;
	string m_strPassword;
	string m_strAdditionalCommandLine;

	ArchiveItemArray m_pArchiveFiles;

	InfoPanelLine* m_pArchiveInfo;
	int m_nArchiveInfoItems;

	OperationStructEx m_OS;

	string m_strShortcutData;

public:

	ArchivePanel(ArchiveModuleManager* pManager, const TCHAR* lpFileName);
	~ArchivePanel();

	Array<ArchiveFormat*>& GetFormats();

	int pGetFindData(PluginPanelItem **pPanelItem, int *pItemsNumber, int OpMode);
	void pFreeFindData(PluginPanelItem *pPanelItem, int nItemsNumber);

	void pClosePlugin();
	void pGetOpenPluginInfo(OpenPluginInfo *pInfo);

	int pSetDirectory(const TCHAR* lpDirectory, int nOpMode);
	int pMakeDirectory(const TCHAR* lpDirectory, int nOpMode);

	int pGetFiles(const PluginPanelItem *PanelItem, int ItemsNumber, int Move, const TCHAR *DestPath, int OpMode);
	int pPutFiles(const PluginPanelItem *PanelItem, int ItemsNumber, int Move, int OpMode);
	int pDeleteFiles(const PluginPanelItem *PanelItem, int ItemsNumber, int OpMode);

	int pProcessHostFile(const PluginPanelItem *PanelItem, int ItemsNumber, int OpMode);
	int pProcessKey(int nKey, DWORD dwControlState);

	void GetArchiveItemsToProcess(
			const PluginPanelItem *pPanelItems,
			int nItemsNumber,
			ArchiveItemArray &items
			);

	void GetPanelItemsToProcess(
			const PluginPanelItem *pPanelItems,
			int nItemsNumber,
			ArchiveItemArray &items
			);

private:

	void Update();

private:

	static LONG_PTR __stdcall Callback(HANDLE hArchive, int nMsg, int nParam, LONG_PTR nParam2);



private:

	int OnStartOperation(int nOperation, StartOperationStruct* pSO);
	int OnQueryPassword(int nMode, PasswordStruct* pPS);
	int OnProcessFile(ProcessFileStruct* pfs);
	int OnProcessData(ProcessDataStruct* pDS);
	int OnReportError(ReportErrorStruct* pRS);

	int OnFileAlreadyExists(OverwriteStruct* pOS);

private:

	int Extract(const ArchiveItemArray& items, const TCHAR *lpDestDiskPath, bool bWithoutPath);
	bool Delete(const ArchiveItemArray& items);
	bool AddFiles(const ArchiveItemArray& items, const TCHAR *lpSourceDiskPath);
	bool Test(const ArchiveItemArray& items);
};
