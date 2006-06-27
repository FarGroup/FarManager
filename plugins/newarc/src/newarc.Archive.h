#pragma once
#include "newarc.h"


struct OperationStruct {
	int nOperation;
	bool bFirstFile;

	unsigned __int64 uFileSize; 
	unsigned __int64 uProcessedSize; 
	unsigned __int64 uTotalSize;
	unsigned __int64 uTotalProcessedSize;
	unsigned __int64 uTotalFiles;
	unsigned __int64 uTotalProcessedFiles;
};


class ArchivePlugin;

class Archive {

public:

	HANDLE m_hArchive;

	ArchivePlugin *m_pPlugin;

	char *m_lpFileName;
	char *m_lpListPassword;

	char *m_lpLastUsedPassword;

public:

//	bool m_bFirstFile;

	OperationStruct m_OS;
	PluginPanelItem *m_pCurrentItem;

	FILETIME m_ArchiveLastAccessTime;
	dword m_dwArchiveFileSizeLow;

	PBYTE m_pCallbackThunk;

	const ArchiveFormatInfo *m_pInfo;
	int m_nMode;

public:

	Archive (ArchivePlugin *pPlugin, const char *lpFileName, HANDLE hArchive);
	~Archive ();

	bool WasUpdated ();

	bool pOpenArchive (int nMode);
	void pCloseArchive ();

	int pGetArchiveItem (ArchiveItemInfo *pItem);

	bool pExtract (PluginPanelItem *pItems, int nItemsNumber, const char *lpDestPath, const char *lpCurrentPath);
	bool pDelete (PluginPanelItem *pItems, int nItemsNumber);
	bool pAddFiles (const char *lpSourcePath, const char *lpCurrentPath, PluginPanelItem *pItems, int nItemsNumber);

	void pNotify (HANDLE hPanel, int nEvent, void *pEventData);

	bool pGetDefaultCommand (int nCommand, char *lpCommand);

	void Finalize ();

friend class ArchivePlugin;

private:

	int __stdcall ArchiveCallback (int nMsg, int nParam, int nParam2);

	int __stdcall OnStartOperation (int nOperation, OperationStructPlugin *pOS);
	int __stdcall OnQueryPassword (int nMode, ArchivePassword *pPassword);
	int __stdcall OnProcessFile (PluginPanelItem *item, const char *lpDestName);
	int __stdcall OnProcessData (unsigned int uDataSize);
};
