#pragma once
#include "newarc.h"

#define PROCESS_CANCEL 0
#define PROCESS_SKIP 1
#define PROCESS_OVERWRITE 2
#define PROCESS_OVERWRITE_ALL 3
#define PROCESS_SKIP_ALL 4
#define PROCESS_UNKNOWN 100

class OperationDialog {

private:	
	
	COORD m_Coord;
	string m_strPanelTitle;

public:

	void SetFileName(bool bTotal, const TCHAR* lpFileName);
	void SetIndicator(bool bTotal, double dRatio);
	void Show(const TCHAR* lpTitle);
};

class OperationErrorList {

private:

	PointerArray<TCHAR*> m_pErrors;

public:

	void AddError(const TCHAR* lpFileName);
	void Show();
};

struct OperationStructEx {

	OperationDialog Dlg;
	OperationErrorList ErrorList;

	int nMode;

	int nOperation;
	bool bFirstFile;

	unsigned __int64 uFileSize; 
	unsigned __int64 uProcessedSize; 
	unsigned __int64 uTotalSize;
	unsigned __int64 uTotalProcessedSize;
	unsigned __int64 uTotalFiles;
	unsigned __int64 uTotalProcessedFiles;

	const ArchiveItem* pCurrentItem;

	//XPERIMENTAL
	int overwrite;
	//XPERIMENTAL

	void Clear()
	{
		nMode = 0;
		nOperation = 0;
		//strOldTitle = NULL;
		pCurrentItem = NULL;

		bFirstFile = false;

		uFileSize = 0;
		uProcessedSize = 0;
		uTotalSize = 0;
		uTotalProcessedSize = 0;
		uTotalFiles = 0;
		uTotalProcessedFiles = 0;

		overwrite = PROCESS_UNKNOWN;
	}

};


class Archive {

private:

	GUID m_uid;

	HANDLE m_hArchive;

	ArchiveModule *m_pModule;
	ArchiveFormat *m_pFormat;

	string m_strFileName;
	string m_strPathInArchive;
	string m_strPassword;

	const ArchiveItem *m_pCurrentItem;

	FILETIME m_ArchiveLastAccessTime;
	FILETIME m_ArchiveLastWriteTime;

	DWORD m_dwArchiveFileSizeLow;

	ArchiveTree* _tree;
	ArchiveTree* _current;

public:

	HANDLE GetHandle() const; 

	Archive(HANDLE hArchive, ArchiveFormat* pFormat, const TCHAR *lpFileName);
	~Archive();

	const GUID& GetUID() const ;
	const TCHAR* GetFileName() const;

	bool QueryCapability(DWORD dwFlags) const;

	ArchiveFormat* GetFormat();
	ArchiveModule* GetModule();
	ArchivePlugin* GetPlugin();

	bool ReadArchiveItems(bool bForce = false);
	void FreeArchiveItems();

	void GetArchiveTreeItems(Array<ArchiveTreeNode*>& items, bool bRecursive);

	bool OpenArchive(int nMode);
	void CloseArchive();

	bool StartOperation(int nOperation, bool bInternal);
	void EndOperation(int nOperation, bool bInternal);

	int GetArchiveItem(ArchiveItem* pItem);
	bool FreeArchiveItem(ArchiveItem* pItem);

	int GetArchiveInfo(const ArchiveInfoItem** pItems);

	bool SetCurrentDirectory(const TCHAR* lpPathInArchive);
	const TCHAR* GetCurrentDirectory();

	void SetPassword(const TCHAR* lpPassword);

	int Extract(const ArchiveItemArray& items, const TCHAR *lpDestDiskPath, bool bWithoutPath);
	int Delete(const ArchiveItemArray& items);
	int AddFiles(const ArchiveItemArray& items, const TCHAR *lpSourceDiskPath);
	int Test(const ArchiveItemArray& items);

	bool MakeDirectory(const TCHAR* lpDirectory);

	bool GetDefaultCommand(int nCommand, string& strCommand, bool& bEnabledByDefault);
	bool GetCommand(int nCommand, string& strCommand);

	bool ExecuteCommand(
			const ArchiveItemArray& items, 
			int nCommand, 
			const TCHAR* lpCurrentDiskPath = NULL,
			const TCHAR* lpAdditionalCommandLine = NULL,
			bool bHideOutput = false
			);

private:

	bool WasUpdated();
	void FreeArchiveItemsHelper(ArchiveTree* tree); //private

};
