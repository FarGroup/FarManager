#pragma once
#include <windows.h>

#ifdef __cplusplus
  #define MY_EXTERN_C extern "C"
#else
  #define MY_EXTERN_C extern
#endif

#define MY_DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
  MY_EXTERN_C const GUID name = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }

#define RESULT_ERROR   0
#define RESULT_SUCCESS 1
#define RESULT_PARTIAL 2
#define RESULT_CANCEL  3

//ModuleEntry result
#define NAERROR_SUCCESS         0
#define NAERROR_NOTIMPLEMENTED  1

#define E_SUCCESS			0
#define E_EOF				1
#define E_BROKEN			2
#define E_READ_ERROR		3
#define E_UNEXPECTED_EOF	4

#define COMMAND_EXTRACT               0
#define COMMAND_EXTRACT_WITHOUT_PATH  1
#define COMMAND_TEST                  2
#define COMMAND_DELETE                3
#define COMMAND_ARCHIVE_COMMENT       4
#define COMMAND_FILE_COMMENT          5
#define COMMAND_CONVERT_TO_SFX        6
#define COMMAND_LOCK                  7
#define COMMAND_ADD_RECOVERY_RECORD   8
#define COMMAND_RECOVER               9
#define COMMAND_ADD                  10

#define MAX_COMMANDS 11

#define AIF_CRYPTED 1 //NOT USED
#define AIF_SOLID   2 //NOT USED

struct ArchiveItem
{
	DWORD dwFlags;

	DWORD dwFileAttributes;
	FILETIME ftCreationTime;
	FILETIME ftLastAccessTime;
	FILETIME ftLastWriteTime;
	unsigned __int64 nFileSize;
	unsigned __int64 nPackSize;

	const TCHAR* lpFileName;
	const TCHAR* lpAlternateFileName;

	DWORD dwCRC32;

	DWORD_PTR UserData; //for internal transitions
};



#define AM_NEED_PASSWORD   1
#define AM_START_OPERATION 2
#define AM_ENTER_STAGE     3
#define AM_PROCESS_FILE    4
#define AM_PROCESS_DATA    5
#define AM_REPORT_ERROR    6
#define AM_NEED_VOLUME     7

#define OPERATION_LIST      1
#define OPERATION_EXTRACT   2
#define OPERATION_ADD       3
#define OPERATION_DELETE    4
#define OPERATION_TEST      5
#define OPERATION_OTHER   100

#define STAGE_EXTRACTING 1
#define STAGE_ADDING     2
#define STAGE_DELETING   3
#define STAGE_TESTING    4
#define STAGE_SKIPPING   5
#define STAGE_UPDATING   6

#define OS_FLAG_TOTALSIZE                       1
#define OS_FLAG_TOTALFILES                      2
#define OS_FLAG_SUPPORT_SINGLE_FILE_PROGRESS    4

struct StartOperationStruct {
	DWORD dwFlags;
	unsigned __int64 uTotalSize;
	unsigned __int64 uTotalFiles;
};

#define PASSWORD_RESET       0
#define PASSWORD_LIST        1
#define PASSWORD_FILE        2
#define PASSWORD_COMPRESSION 3

struct PasswordStruct {
	DWORD dwBufferSize;
	TCHAR* lpBuffer;
};

struct VolumeStruct {
	const TCHAR* lpSuggestedName;

	DWORD dwBufferSize;
	TCHAR* lpBuffer;
};

#define PROGRESS_PROCESSED_DIFF	1  //UNSUPPORTED
#define PROGRESS_PROCESSED_SIZE	2  //UNSUPPORTED
#define PROGRESS_PERCENTS		3  //UNSUPPORTED
#define PROGRESS_DETAILS		4

struct ProcessDataStruct {

	int nMode; 

	union {
		unsigned __int64 uProcessedDiff;
		unsigned __int64 uProcessedSize;
		struct {
			char cPercents;
			char cTotalPercents;
		};
		struct {
			unsigned __int64 uProcessedBytesFile;
			unsigned __int64 uTotalBytesFile;
			unsigned __int64 uProcessedBytesTotal;
			unsigned __int64 uTotalBytes;
		};
	};
};

#define PROCESS_CANCEL    0
#define PROCESS_SKIP      1
#define PROCESS_OVERWRITE 2

struct ProcessFileStruct {
	const TCHAR* lpDestFileName;
	const ArchiveItem* pItem;
};

#define DELETE_ERROR_UNKNOWN	100
#define ADD_ERROR_UNKNOWN		200

#define EXTRACT_ERROR_UNKNOWN	1
#define EXTRACT_ERROR_CRC		2
#define EXTRACT_ERROR_DATA		3

struct ReportErrorStruct {
	const ArchiveItem* pItem;
	int nError;
};

typedef LONG_PTR (__stdcall *ARCHIVECALLBACK)(HANDLE hPlugin, int nMsg, int nParam1, LONG_PTR nParam2);

struct ArchiveQueryResult {
	GUID uidPlugin;
	GUID uidFormat;
};

#define AFF_SUPPORT_INTERNAL_EXTRACT       1
#define AFF_SUPPORT_INTERNAL_TEST          2
#define AFF_SUPPORT_INTERNAL_ADD           4
#define AFF_SUPPORT_INTERNAL_DELETE        8
#define AFF_SUPPORT_INTERNAL_CREATE       16
#define AFF_SUPPORT_CONFIG_GENERAL        32
#define AFF_SUPPORT_CONFIG_CREATE         64
#define AFF_SUPPORT_CONFIG_DELETE        128
#define AFF_SUPPORT_CONFIG_EXTRACT       256
#define AFF_SUPPORT_CONFIG_TEST          512
#define AFF_SUPPORT_CONFIG_ADD          1024
#define AFF_SUPPORT_DEFAULT_COMMANDS    2048
#define AFF_NEED_EXTERNAL_NOTIFICATIONS 4096

struct ArchiveFormatInfo {
	DWORD dwStructVersion;

	GUID uid; //format uid

	DWORD dwFlags;

	const TCHAR* lpName;
	const TCHAR* lpDefaultExtention;

	const TCHAR* lpDescription;
};

#define APF_SUPPORT_SINGLE_FORMAT_QUERY 1

struct ArchivePluginInfo {
	DWORD dwStructVersion;
	
	GUID uid;
	DWORD dwFlags;

	const TCHAR* lpModuleName;

	unsigned int uFormats;
	const ArchiveFormatInfo* pFormats;
};


struct ArchiveModuleVersion {
	unsigned char Major;
	unsigned char Minor;
	unsigned short Build;
};


#define AMF_SUPPORT_SINGLE_PLUGIN_QUERY 1

struct ArchiveModuleInfo {
	DWORD dwStructVersion;

	GUID uid; //module uid
	DWORD dwFlags;

	const TCHAR* lpDescription;
	const TCHAR* lpAuthor;

	ArchiveModuleVersion Version;

	unsigned int uPlugins;
	const ArchivePluginInfo* pPlugins;
};

struct GetArchiveFormatStruct {
	DWORD dwStructVersion;

	HANDLE hArchive;
	GUID uid;

	bool bResult;
};


#define QUERY_FLAG_FORMAT_UID_READY	1  //format uid already known
#define QUERY_FLAG_PLUGIN_UID_READY 2
#define QUERY_FLAG_MORE_ARCHIVES	4  //has more archive formats to query


struct QueryArchiveStruct {
	DWORD dwStructVersion;

	GUID uidFormat;
	GUID uidPlugin;

	const TCHAR *lpFileName;

	const unsigned char *pBuffer;
	DWORD dwBufferSize;

	DWORD dwFlags;

	bool bResult;
};


struct GetDefaultCommandStruct {
	DWORD dwStructVersion;

	GUID uidPlugin;
	GUID uidFormat;

	int nCommand;

	const TCHAR* lpCommand;
	bool bEnabled;

	bool bResult;
};

struct ExtractStruct {
	DWORD dwStructVersion;

	HANDLE hArchive;
	
	const ArchiveItem *pItems;
	unsigned int uItemsNumber;
	
	const TCHAR *lpDestPath;
	const TCHAR *lpCurrentPath;

	//bool bResult;
	int nResult; //NEW!!!
};

struct OpenCreateArchiveStruct {
	DWORD dwStructVersion;

	GUID uidFormat;
	GUID uidPlugin;

	HANDLE hCallback;
	ARCHIVECALLBACK pfnCallback;

	const TCHAR* lpFileName;

	bool bCreate;

	HANDLE hResult;
};


struct OperationStruct {
	DWORD dwStructVersion;

	HANDLE hArchive;

	int nOperation;
	bool bInternal;

	void* pOperationData;

	bool bResult;
};

struct CloseArchiveStruct {
	DWORD dwStructVersion;

	GUID uidPlugin;
	HANDLE hArchive;
};

struct GetArchiveItemStruct {
	DWORD dwStructVersion;

	HANDLE hArchive;
	ArchiveItem* pItem;

	int nResult;
};

struct FreeArchiveItemStruct {
	DWORD dwStructVersion;

	HANDLE hArchive;
	ArchiveItem* pItem;

	bool bResult;
};


struct TestStruct {
	DWORD dwStructVersion;

	HANDLE hArchive;

	const ArchiveItem* pItems;
	unsigned int uItemsNumber;

	int nResult;
};

struct DeleteStruct {
	DWORD dwStructVersion;

	HANDLE hArchive;
	
	const ArchiveItem* pItems;
	unsigned int uItemsNumber;
	
	int nResult;
};

struct AddStruct {
	DWORD dwStructVersion;

	HANDLE hArchive;

	const TCHAR* lpSourcePath;
	const TCHAR* lpCurrentPath;
	
	const ArchiveItem* pItems;
	unsigned int uItemsNumber;

	const TCHAR* pConfig;
	
	int nResult;
};

struct ArchiveInfoItem {
	const TCHAR* lpName;
	const TCHAR* lpValue;
};

struct ArchiveInfoStruct {
	DWORD dwStructVersion;

	HANDLE hArchive;

	bool bMultiVolume; //archive has several volumes

	int nInfoItems;
	const ArchiveInfoItem* pInfo;

	bool bResult;
};

struct ArchiveFileInfoStruct {
	DWORD dwStructVersion;

	HANDLE hArchive;
	const ArchiveItem* pItem;

	int nInfoItems;
	const ArchiveInfoItem* pInfo;

	bool bResult;
};


struct ConfigureStruct { //to change
	DWORD dwStructVersion;
};

struct ConfigureFormatStruct {
	DWORD dwStructVersion;

	int nOperation;

	GUID uidFormat;
	GUID uidPlugin;

	const TCHAR* lpConfig;
	TCHAR* lpResult;

	bool bResult;
};

struct FreeConfigResultStruct {
	TCHAR* lpResult;
};


struct StartupInfo {
	PluginStartupInfo Info;
};


#define FID_INITIALIZE            1
#define FID_FINALIZE              2
#define FID_QUERYARCHIVE          3
#define FID_FREEARCHIVEITEM       4
#define FID_GETDEFAULTCOMMAND     5
#define FID_GETARCHIVEFORMAT      6
#define FID_EXTRACT               7
#define FID_OPENARCHIVE           8
#define FID_CREATEARCHIVE         9
#define FID_STARTOPERATION       10
#define FID_ENDOPERATION         11
#define FID_CLOSEARCHIVE         12
#define FID_GETARCHIVEITEM       13
#define FID_TEST                 14
#define FID_GETARCHIVEMODULEINFO 15
#define FID_DELETE               16
#define FID_ADD                  17
#define FID_CONFIGURE            18
#define FID_CONFIGUREFORMAT      19
#define FID_FREECONFIGRESULT     20
#define FID_GETARCHIVEINFO       21
#define FID_GETARCHIVEFILEINFO   22

//XPERIMENTAL

struct OverwriteStruct {
	const ArchiveItem* pItem;
	const TCHAR* lpFileName;
	bool bExtract;
};

//XPERIMENTAL END


#ifdef __cplusplus
extern "C" {
#endif

int __stdcall ModuleEntry(int nFunctionID, void *pParams);

#ifdef __cplusplus
}
#endif
