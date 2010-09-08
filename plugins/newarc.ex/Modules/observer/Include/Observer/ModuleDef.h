#ifndef _MODULE_DEF_H_
#define _MODULE_DEF_H_

#define MODULE_EXPORT __stdcall

// Extract progress callbacks
typedef int (CALLBACK *ExtractStartFunc)(HANDLE);
typedef int (CALLBACK *ExtractProgressFunc)(HANDLE);
typedef void (CALLBACK *ExtractEndFunc)(HANDLE);

struct ExtractProcessCallbacks
{
	HANDLE signalContext;
	ExtractProgressFunc FileProgress;
};

struct ProgressContext
{
	char szFilePath[MAX_PATH];
	wchar_t wszFilePath[MAX_PATH];

	int nCurrentFileNumber;
	int nTotalFiles;
	__int64 nProcessedBytes;
	__int64 nTotalSize;
	int nCurrentFileProgress;

	ProgressContext()
	{
		memset(szFilePath, 0, MAX_PATH);
		nCurrentFileNumber = 0;
		nTotalFiles = 0;
		nTotalSize = 0;
		nProcessedBytes = 0;
		nCurrentFileProgress = 0;
	}
};

#define STORAGE_FORMAT_NAME_MAX_LEN 16
#define STORAGE_PARAM_MAX_LEN 64

struct StorageGeneralInfo
{
	wchar_t Format[STORAGE_FORMAT_NAME_MAX_LEN];
	wchar_t Compression[STORAGE_PARAM_MAX_LEN];
	wchar_t Comment[STORAGE_PARAM_MAX_LEN];
	FILETIME Created;
};

struct ExtractOperationParams 
{
	int item;
	int flags;
	const wchar_t* destFilePath;
	ExtractProcessCallbacks callbacks;
};

typedef int (MODULE_EXPORT *LoadSubModuleFunc)(const wchar_t*);
typedef int (MODULE_EXPORT *OpenStorageFunc)(const wchar_t*, INT_PTR**, StorageGeneralInfo*);
typedef void (MODULE_EXPORT *CloseStorageFunc)(INT_PTR*);
typedef int (MODULE_EXPORT *GetItemFunc)(INT_PTR*, int, LPWIN32_FIND_DATAW, wchar_t*, size_t);
typedef int (MODULE_EXPORT *ExtractFunc)(INT_PTR*, ExtractOperationParams params);

// Item retrieval result
#define GET_ITEM_ERROR 0
#define GET_ITEM_OK 1
#define GET_ITEM_NOMOREITEMS 2

// Extract operation flags
#define SEP_ASKOVERWRITE 1

// Extract result
#define SER_SUCCESS 0
#define SER_ERROR_WRITE 1
#define SER_ERROR_READ 2
#define SER_ERROR_SYSTEM 3
#define SER_USERABORT 4

// Extract error reasons
#define EER_NOERROR 0
#define EER_READERROR 1
#define EER_WRITEERROR 2
#define EER_TARGETEXISTS 3

// Extract error reactions
#define EEN_ABORT 1
#define EEN_RETRY 2
#define EEN_SKIP 3
#define EEN_SKIPALL 4
#define EEN_CONTINUE 5
#define EEN_CONTINUESILENT 6

#endif
