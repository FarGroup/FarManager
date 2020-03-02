#include "7z.h"

typedef unsigned int (__stdcall *CREATEOBJECT) (const GUID *, const GUID *, void **);
typedef HRESULT (__stdcall *GETHANDLERPROPERTY) (PROPID propID, PROPVARIANT *value);
typedef HRESULT (__stdcall *GETHANDLERPROPERTY2) (unsigned int formatIndex, PROPID propID, PROPVARIANT *value);
typedef HRESULT (__stdcall *GETNUMBEROFFORMATS) (unsigned int *numFormats);

typedef LONG_PTR (__stdcall *STARTOPERATION)(HANDLE hContext, int nOperation, unsigned __int64 uTotalSize, unsigned __int64 uTotalFiles);
typedef LONG_PTR (__stdcall *PROCESSFILE)(HANDLE hContext, const ArchiveItem* pItems, const TCHAR* lpDestName);
typedef LONG_PTR (__stdcall *PROCESSDATA)(HANDLE hContext, int nSize);
typedef LONG_PTR (__stdcall *PASSWORDOPERATION)(HANDLE hContext, int nType, TCHAR* pBuffer, DWORD dwBufferSize);

struct FormatPosition {
	const GUID* puid;
	int position;
};

class SevenZipPlugin {

private:

	GUID m_uid;

	HMODULE m_hModule;

	CREATEOBJECT m_pfnCreateObject;
	GETHANDLERPROPERTY m_pfnGetHandlerProperty;
	GETHANDLERPROPERTY2 m_pfnGetHandlerProperty2;
	GETNUMBEROFFORMATS m_pfnGetNumberOfFormats;

	string m_strModuleName;

	Array<ArchiveFormatInfo> m_pFormatInfo;

public:

	SevenZipPlugin(const GUID& uid, const TCHAR* lpModuleName);
	virtual ~SevenZipPlugin();

	bool Load();

	const GUID& GetUID();
	const TCHAR* GetModuleName();

	unsigned int GetNumberOfFormats();
	ArchiveFormatInfo* GetFormats();

	int QueryArchives(const unsigned char* pBuffer, DWORD dwBufferSize, const TCHAR* lpFileName, Array<ArchiveQueryResult*>& result);
	int QueryArchive(const unsigned char* pBuffer, DWORD dwBufferSize, const GUID& uid, const TCHAR* lpFileName, Array<ArchiveQueryResult*>& result); 
	
	SevenZipArchive* OpenCreateArchive(
			const GUID& uid, 
			const TCHAR* lpFileName, 
			HANDLE hCallback, 
			ARCHIVECALLBACK pfnCallback, 
			bool bCreate
			);

	bool GetDefaultCommand(const GUID& uid, int nCommand, const TCHAR** ppCommand);

	void CloseArchive(SevenZipArchive* pArchive);

	bool CreateObject(const GUID& uidFormat, const GUID& uidInterface, void** pObject);
//	void Configure(const GUID& uid);
};