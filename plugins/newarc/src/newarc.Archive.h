#pragma once
#include "newarc.h"

class ArchivePlugin;

class Archive {

public:

	HANDLE m_hArchive;

	ArchivePlugin *m_pPlugin;

	char *m_lpFileName;
	char *m_lpListPassword;

	char *m_lpLastUsedPassword;

public:

	bool m_bFirstFile;

	unsigned __int64 m_uTotalSize;
	unsigned __int64 m_uTotalSize2;
	unsigned __int64 m_uFullSize;

	PluginPanelItem *m_pCurrentItem;

	FILETIME m_ArchiveLastAccessTime;
	dword m_dwArchiveFileSizeLow;

	PBYTE m_pCallbackThunk;

	int m_nMode;

public:

	Archive (ArchivePlugin *pPlugin, const char *lpFileName, HANDLE hArchive);
	~Archive ();

	bool WasUpdated ();

	bool pOpenArchive (int nMode);
	void pCloseArchive ();

	int pGetArchiveFormatType ();

	char *pGetArchiveFormatName ();

	int pGetArchiveItem (ArchiveItemInfo *pItem);

	bool pExtract (PluginPanelItem *pItems, int nItemsNumber, const char *lpDestPath, const char *lpCurrentPath);

	bool pGetDefaultCommand (int nCommand, char *lpCommand);

	void Finalize ();

friend class ArchivePlugin;

private:

	int __stdcall ArchiveCallback (int nMsg, int nParam, int nParam2);
};
