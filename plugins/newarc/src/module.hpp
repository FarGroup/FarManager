#pragma once
#include <plugin.hpp>

#ifdef __cplusplus
  #define MY_EXTERN_C extern "C"
#else
  #define MY_EXTERN_C extern
#endif

#define MY_DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
  MY_EXTERN_C const GUID name = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }


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
#define AM_START_OPERATION		2
#define AM_PROCESS_FILE			3
#define AM_PROCESS_DATA			4

#define OPERATION_EXTRACT		1
#define OPERATION_ADD			2
#define OPERATION_DELETE		3

#define PASSWORD_RESET	0
#define PASSWORD_LIST	1
#define PASSWORD_FILE	2


#define OS_FLAG_TOTALSIZE	1
#define OS_FLAG_TOTALFILES	2

struct OperationStructPlugin {
	DWORD dwFlags;
	unsigned __int64 uTotalSize;
	unsigned __int64 uTotalFiles;
};

struct ArchivePassword {
	DWORD dwBufferSize;
	char *lpBuffer;
};

typedef LONG_PTR (__stdcall *ARCHIVECALLBACK) (int nMsg, int nParam1, LONG_PTR nParam2);

struct ProcessFileStruct {
	PluginPanelItem *pItem;
	const char *lpDestFileName;
};

#define AIF_CRYPTED		1
#define AIF_SOLID		2

struct ArchiveItemInfo {
	DWORD dwFlags;
	PluginPanelItem pi;
};

#define AFF_SUPPORT_INTERNAL_EXTRACT	1
#define AFF_SUPPORT_INTERNAL_TEST		2
#define AFF_SUPPORT_INTERNAL_ADD		4
#define AFF_SUPPORT_INTERNAL_DELETE		8
#define AFF_SUPPORT_INTERNAL_CREATE		16
#define AFF_SUPPORT_INTERNAL_CONFIG		32

struct ArchiveFormatInfo {
	GUID uid;
	DWORD dwFlags;
	const char *lpName;
	const char *lpDefaultExtention;
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

	int nFormats;
	HANDLE hResult;
};

struct QueryArchiveFormatStruct {
	DWORD dwStructSize;

	int nFormat;
	HANDLE hResult;
};

struct GetDefaultCommandStruct {
	DWORD dwStructSize;

	GUID uid;
	int nCommand;

	char *lpCommand;

	bool bResult;
};

struct GetArchiveFormatStruct {
	DWORD dwStructSize;

	HANDLE hArchive;

	GUID uid;
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

struct AddStruct {
	DWORD dwStructSize;

	HANDLE hArchive;

	const char *lpSourcePath;
	const char *lpCurrentPath;
	PluginPanelItem *pItems;
	int nItemsNumber;
	bool bResult;
};

struct CreateArchiveStruct {
	DWORD dwStructSize;

	GUID uid;
	const char *lpFileName;

	HANDLE hResult;
};

struct ConfigureFormatStruct {
	GUID uid;
	char *lpResult;
};


#define NOTIFY_EXTERNAL_ADD_START		1
#define NOTIFY_EXTERNAL_ADD_END			2
#define NOTIFY_EXTERNAL_DELETE_START	3
#define NOTIFY_EXTERNAL_DELETE_END		4
#define NOTIFY_EXTERNAL_EXTRACT_START	5
#define NOTIFY_EXTERNAL_EXTRACT_END		6

struct NotifyStruct {
	DWORD dwStructSize;

	HANDLE hArchive;
	HANDLE hPanel;

	int nEvent;
	void *pEventData;
};


typedef void* (__stdcall *ALLOCATE) (DWORD dwBytes);
typedef void (__stdcall *FREE) (void *pBlock);

struct Helpers {
	ALLOCATE Allocate;
	FREE Free;
};

struct StartupInfo {
	PluginStartupInfo Info;
	Helpers HF;
};


#define FID_INITIALIZE			 1	//param - StartupInfo
#define FID_FINALIZE			 2	//param - NULL
#define FID_QUERYARCHIVE    	 3	//param - QueryArchiveStruct
#define FID_QUERYARCHIVEFORMAT 4	//param - QueryArchiveFormatStruct
#define FID_QUERYARCHIVEEND    5
#define FID_FINALIZEARCHIVE		 6	//param - hArchive (to change to FinalizeArchiveStruct!!!)
#define FID_GETDEFAULTCOMMAND	 7	//param - GetDefaultCommandStruct
#define FID_GETARCHIVEFORMAT	 8	//param - GetArchiveFormatStruct
#define FID_EXTRACT				 9	//param - ExtractStruct
#define FID_OPENARCHIVE			 10
#define FID_CLOSEARCHIVE		 11
#define FID_GETARCHIVEITEM		12
#define FID_TEST				13
#define FID_GETARCHIVEPLUGININFO	14	//param - ArchivePluginInfo
#define FID_DELETE				15 //param - DeleteStruct
#define FID_ADD                 16 //param - AddStruct
#define FID_CREATEARCHIVE    	17 //param - CreateArchiveStruct
#define FID_NOTIFY				18 //param - NotifyStruct
#define FID_CONFIGUREFORMAT		19 //param - ConfigureFormatStruct

#ifdef __cplusplus
extern "C" {
#endif

int __stdcall PluginEntry (int nFunctionID, void *pParams);

#ifdef __cplusplus
}
#endif
