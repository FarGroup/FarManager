#pragma once
#include "newarc.h"

struct InternalArchiveItemInfo {
	ArchiveItemInfo ItemInfo;
	char *lpPassword;
};


struct InternalEditorInfo {
	char *lpFileName;
	char *lpFolder;
	char *lpPassword;
	int nEditorID;
	FILETIME ftTime;
	HANDLE hFile;
};

class ArchivePanel {

public:

	Archive *m_pArchive;

	Archive **m_pArchives;
	int m_nArchivesCount;

	bool m_bFirstTime;

	char *m_lpCurrentFolder;
	char *m_lpPanelTitle;

	InternalArchiveItemInfo *m_pArchiveFiles;

	int m_nArchiveFilesCount;
	int m_nArrayCount;

	Collection <InternalEditorInfo*> m_Editors;

	int m_nCurrentMode;

public:

	ArchivePanel (Archive **pArchives, int nArchivesCount);
	~ArchivePanel ();

	bool __stdcall ReadArchive (bool bSilent);

	int __stdcall pGetFindData (PluginPanelItem **pPanelItem, int *pItemsNumber, int OpMode);
	void __stdcall pFreeFindData (PluginPanelItem *pPanelItem, int nItemsNumber);

	void __stdcall pClosePlugin ();
	void __stdcall pGetOpenPluginInfo (OpenPluginInfo *pInfo);

	int __stdcall pSetDirectory (const char *Dir, int nOpMode);

	int __stdcall pGetFiles (PluginPanelItem *PanelItem, int ItemsNumber, int Move, char *DestPath, int OpMode);
	int __stdcall pPutFilesNew (Archive *pArchive, PluginPanelItem *PanelItem, int ItemsNumber, int Move, int OpMode);
	int __stdcall pPutFiles (PluginPanelItem *PanelItem, int ItemsNumber, int Move, int OpMode);
	int __stdcall pDeleteFiles (PluginPanelItem *PanelItem,	int ItemsNumber, int OpMode);

	int __stdcall pProcessHostFile (PluginPanelItem *PanelItem, int ItemsNumber, int OpMode);

	int __stdcall pProcessKey (int nKey, dword dwControlState);

    void __stdcall pExecuteCommand (
    		int nCommand,
    		const char *lpArchivePassword,
    		const char *lpAdditionalCommandLine,
			PluginPanelItem *pItems,
			int nItemsNumber
			);
};
