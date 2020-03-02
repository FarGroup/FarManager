#pragma once

typedef HANDLE (__stdcall *PLUGINOPENARCHIVE)(tOpenArchiveData* ArchiveData);
typedef HANDLE (__stdcall *PLUGINOPENARCHIVEW)(tOpenArchiveDataW* ArchiveData);

typedef int (__stdcall *PLUGINREADHEADER)(HANDLE hArcData, tHeaderData* HeaderData);
typedef int (__stdcall *PLUGINREADHEADEREX)(HANDLE hArcData, tHeaderDataEx* HeaderData); 
typedef int (__stdcall *PLUGINREADHEADEREXW)(HANDLE hArcData, tHeaderDataExW* HeaderData); 

typedef int (__stdcall *PLUGINPROCESSFILE)(HANDLE hArcData, int Operation, const char* DestPath, const char* DestName);
typedef int (__stdcall *PLUGINPROCESSFILEW)(HANDLE hArcData, int Operation, const wchar_t* DestPath, const wchar_t* DestName);

typedef int (__stdcall *PLUGINPACKFILES)(const char* PackedFile, const char* SubPath, const char* SrcPath, const char* AddList, int Flags);
typedef int (__stdcall *PLUGINPACKFILESW)(const wchar_t* PackedFile, const wchar_t* SubPath, const wchar_t* SrcPath, const wchar_t* AddList, int Flags);

typedef int (__stdcall *PLUGINDELETEFILES)(const char* PackedFile, const char* DeleteList);
typedef int (__stdcall *PLUGINDELETEFILESW)(const wchar_t* PackedFile, const wchar_t* DeleteList);

typedef void (__stdcall *PLUGINSETCHANGEVOLPROC)(HANDLE hArcData, tChangeVolProc pChangeVolProc);
typedef void (__stdcall *PLUGINSETCHANGEVOLPROCW)(HANDLE hArcData, tChangeVolProcW pChangeVolProc);

typedef void (__stdcall *PLUGINSETPROCESSDATAPROC)(HANDLE hArcData, tProcessDataProc pProcessDataProc);
typedef void (__stdcall *PLUGINSETPROCESSDATAPROCW)(HANDLE hArcData, tProcessDataProcW pProcessDataProc);

typedef BOOL (__stdcall *PLUGINCANYOUHANDLETHISFILE)(const char* FileName);
typedef BOOL (__stdcall *PLUGINCANYOUHANDLETHISFILEW)(const wchar_t* FileName);

//no unicode
typedef int (__stdcall *PLUGINCLOSEARCHIVE)(HANDLE hArcData);
typedef void (__stdcall *PLUGINCONFIGUREPACKER)(HWND Parent, HINSTANCE DllInstance);
typedef int (__stdcall *PLUGINGETPACKERCAPS)();
typedef void (__stdcall *PLUGINPACKSETDEFAULTPARAMS)(PackDefaultParamStruct* dps);

#define ARCHIVE_OPERATION_ADD 1
#define ARCHIVE_OPERATION_DELETE 2

class WcxPlugin {

private:

	GUID m_uid;

	HMODULE m_hModule;

	ArchiveFormatInfo* m_pFormatInfo;

	string m_strModuleName;

	PLUGINOPENARCHIVE m_pfnOpenArchive;
	PLUGINOPENARCHIVEW m_pfnOpenArchiveW;

	PLUGINREADHEADER m_pfnReadHeader;
	PLUGINREADHEADEREX m_pfnReadHeaderEx;
	PLUGINREADHEADEREXW m_pfnReadHeaderExW;

	PLUGINPROCESSFILE m_pfnProcessFile;
	PLUGINPROCESSFILEW m_pfnProcessFileW;

	PLUGINPACKFILES m_pfnPackFiles;
	PLUGINPACKFILESW m_pfnPackFilesW;
	PLUGINDELETEFILES m_pfnDeleteFiles;
	PLUGINDELETEFILESW m_pfnDeleteFilesW;

	PLUGINSETCHANGEVOLPROC m_pfnSetChangeVolProc;
	PLUGINSETCHANGEVOLPROCW m_pfnSetChangeVolProcW;
	PLUGINSETPROCESSDATAPROC m_pfnSetProcessDataProc;
	PLUGINSETPROCESSDATAPROCW m_pfnSetProcessDataProcW;

	PLUGINCANYOUHANDLETHISFILE m_pfnCanYouHandleThisFile;
	PLUGINCANYOUHANDLETHISFILEW m_pfnCanYouHandleThisFileW;
	
	PLUGINCLOSEARCHIVE m_pfnCloseArchive;
	PLUGINCONFIGUREPACKER m_pfnConfigurePacker;
	PLUGINGETPACKERCAPS m_pfnGetPackerCaps;
	PLUGINPACKSETDEFAULTPARAMS m_pfnPackSetDefaultParams;

public:

	WcxPlugin(const GUID& uid);
	~WcxPlugin ();

	bool Load(const TCHAR *lpModuleName);

	const GUID& GetUID();
	const TCHAR* GetModuleName();

	unsigned int GetNumberOfFormats();
	const ArchiveFormatInfo* GetFormats();

	int QueryArchive(const TCHAR* lpFileName, Array<ArchiveQueryResult*>& result);
	
	WcxArchive* OpenCreateArchive(
				const GUID& uidFormat, 
				const TCHAR* lpFileName, 
				HANDLE hCallback, 
				ARCHIVECALLBACK pfnCallback,
				bool bCreate
				);

	void CloseArchive(WcxArchive* pArchive);
//wcx

	HANDLE OpenArchive(const TCHAR* lpFileName, int nOpMode);
	void CloseArchive(HANDLE hArchive);

	void SetCallbacks(
			HANDLE hArchive,
			tProcessDataProcW pfnProcessDataProcW,
			tProcessDataProc pfnProcessDataProc,
			tChangeVolProcW pfnSetChangeVolProcW,
			tChangeVolProc pfnSetChangeVolProc
			);

	__int64 CreateFileList(const ArchiveItem* pItems, int nItemsNumber, int nOperation, void** pResult);

	int GetArchiveItem(HANDLE hArchive, ArchiveItem* pItem);
	int ProcessFile(HANDLE hArchive, int OpMode, const TCHAR* lpDestPath, const TCHAR* lpDestName);

	int DeleteFiles(const TCHAR* lpPackedFile, void* pNamesList/*const ArchiveItem* pItems, int nItemsNumber*/);
	int PackFiles(const TCHAR* lpPackedFile, const TCHAR* SubPath, const TCHAR* SrcPath, void* pNamesList/*const ArchiveItem* pItems, int nItemsNumber*/, int Flags);

	int GetPackerCaps();
	void ConfigurePacker();

private:

	int ConvertResult(int nResult);
};
