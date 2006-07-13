#pragma once
#include <plugin.hpp>

#define _M(id) (char*)Info.GetMsg (Info.ModuleNumber, id)

#define E_SUCCESS			0
#define E_EOF				1
#define E_BROKEN			2
#define E_READ_ERROR		3
#define E_UNEXPECTED_EOF	4

#define COMMAND_EXTRACT					0
#define COMMAND_EXTRACT_WITHOUT_PATH	1
#define COMMAND_TEST					2
#define COMMAND_DELETE					3
#define COMMAND_ARCHIVE_COMMENT			4
#define COMMAND_FILE_COMMENT			5
#define COMMAND_CONVERT_TO_SFX			6
#define COMMAND_LOCK					7
#define COMMAND_ADD_RECOVERY_RECORD		8
#define COMMAND_RECOVER					9
#define COMMAND_ADD						10

#define OM_LIST		1
#define OM_EXTRACT	2
#define OM_TEST		3
#define OM_ADD		4

#define AM_NEED_PASSWORD		1
#define AM_START_EXTRACT_FILE	2
#define AM_PROCESS_DATA			4

#define PASSWORD_RESET	0
#define PASSWORD_LIST	1
#define PASSWORD_FILE	2

struct ArchivePassword {
	DWORD dwBufferSize;
	char *lpBuffer;
};

typedef int (__stdcall *ARCHIVECALLBACK) (int nMsg, int nParam1, int nParam2);

#define AIF_CRYPTED		1
#define AIF_SOLID		2

struct ArchiveItemInfo {
	DWORD dwFlags;
	PluginPanelItem pi;
};

#define AFF_SUPPORT_INTERNAL_EXTRACT	1
#define AFF_SUPPORT_INTERNAL_TEST		2
#define AFF_SUPPORT_INTERNAL_ADD		4
#define AFF_SUPPORT_INTERNAL_DELETE		5

struct ArchiveFormatInfo {
	DWORD dwFlags;
	char *lpName;
	char *lpDefaultExtention;
};

struct ArchivePluginInfo {
	int nFormats;
	ArchiveFormatInfo *pFormatInfo;
};

/*struct ArchivePluginInfo {
	int nFormatTypes;
	char **pFormatNames;
};*/

#define NAERROR_SUCCESS			0
#define NAERROR_NOTIMPLEMENTED	1
#define NAERROR_INTERNAL			2
#define NAERROR_MEMORYALLOCATION	3

struct QueryArchiveStruct {
	DWORD dwStructSize;

	const char *lpFileName;
	const char *lpBuffer;
	DWORD dwBufferSize;

	HANDLE hResult;
};

struct GetDefaultCommandStruct {
	DWORD dwStructSize;

	int nFormat;
	int nCommand;

	char *lpCommand;

	bool bResult;
};

struct GetArchiveFormatStruct {
	DWORD dwStructSize;

	HANDLE hArchive;

	int nFormat;
};

struct ExtractStruct {
	DWORD dwStructSize;

	HANDLE hArchive;
	PluginPanelItem *pItems;
	int nItemsNumber;
	const char *lpDestPath;
	const char *lpCurrentPath;

	bool bResult;
};

struct OpenArchiveStruct {
	DWORD dwStructSize;

	HANDLE hArchive;
	int nMode;
	ARCHIVECALLBACK pfnCallback;

	bool bResult;
};

struct CloseArchiveStruct {
	DWORD dwStructSize;

	HANDLE hArchive;
};

struct GetArchiveItemStruct {
	DWORD dwStructSize;

	HANDLE hArchive;
	ArchiveItemInfo *pItem;

	int nResult;
};

struct TestStruct {
	DWORD dwStructSize;

	HANDLE hArchive;
	PluginPanelItem *pItems;
	int nItemsNumber;

	bool bResult;
};

struct DeleteStruct {
	DWORD dwStructSize;

	HANDLE hArchive;
	PluginPanelItem *pItems;
	int nItemsNumber;
	bool bResult;
};

#define FID_INITIALIZE			 1	//param - PluginStartupInfo
#define FID_FINALIZE			 2	//param - NULL
#define FID_QUERYARCHIVE    	 3	//param - QueryArchiveStruct
#define FID_FINALIZEARCHIVE		 4	//param - hArchive (to change to FinalizeArchiveStruct!!!)
#define FID_GETDEFAULTCOMMAND	 5	//param - GetDefaultCommandStruct
#define FID_GETARCHIVEFORMAT	 6	//param - GetArchiveFormatStruct
#define FID_EXTRACT				 7	//param - ExtractStruct
#define FID_OPENARCHIVE			 8
#define FID_CLOSEARCHIVE		 9
#define FID_GETARCHIVEITEM		10
#define FID_TEST				11
#define FID_GETARCHIVEPLUGININFO	12	//param - ArchivePluginInfo
#define FID_DELETE				13 //param - DeleteStruct

#ifdef __cplusplus
extern "C" {
#endif

int __stdcall PluginEntry (int nFunctionID, void *pParams);

#ifdef __cplusplus
}
#endif
