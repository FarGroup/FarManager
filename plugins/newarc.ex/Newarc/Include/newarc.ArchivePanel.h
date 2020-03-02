#pragma once
#include "newarc.h"

class ArchivePanel {

private:

	ArchiveModuleManager* m_pManager;

	Archive* m_pArchive;
	Array<ArchiveFormat*> m_pFormats;

	bool m_bMultiVolume;

	bool m_bFirstTime;

	string m_strFileName;
	string m_strPathInArchive;
	string m_strPanelTitle;

	string m_strLastDestPath;

	bool m_bPasswordSet;
	string m_strPassword;
	string m_strAdditionalCommandLine;

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
	
	int pPutFiles(
			const PluginPanelItem *PanelItem, 
			int ItemsNumber, 
			int Move, 
#ifdef UNICODE
			const wchar_t* SrcPath,
#endif
			int OpMode
			);

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
	int OnEnterStage(int nStage);
	int OnQueryPassword(int nMode, PasswordStruct* pPS);
	int OnProcessFile(ProcessFileStruct* pfs);
	int OnProcessData(ProcessDataStruct* pDS);
	int OnReportError(ReportErrorStruct* pRS);
	int OnNeedVolume(VolumeStruct* pVS);

	int OnFileAlreadyExists(OverwriteStruct* pOS);

private:

	bool GetCommand(int nCommand, string& strCommand);

	int Extract(const ArchiveItemArray& items, const TCHAR* lpDestDiskPath, bool bWithoutPath);
	int Delete(const ArchiveItemArray& items);
	int AddFiles(const ArchiveItemArray& items, const TCHAR* lpSourceDiskPath, const TCHAR* lpConfig);
	int Test(const ArchiveItemArray& items);
	int MakeDirectory(const TCHAR* lpDirectory);
};
